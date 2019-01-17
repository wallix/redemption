/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Vnc encoder module for Hextile bitmaps
*/

#include "mod/vnc/encoder/hextile.hpp"
#include "cxx/diagnostic.hpp"

#include "core/buf64k.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/vnc/vnc_verbose.hpp"
#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "utils/bitmap.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/update_lock.hpp"


//    7.6.5   Hextile Encoding

//    Hextile is a variation on the RRE idea. Rectangles are split up into 16x16 tiles,
// allowing the dimensions of the subrectangles to be specified in 4 bits each, 16 bits in total.

// The rectangle is split into tiles starting at the top left going in left-to-right,
// top-to-bottom order. The encoded contents of the tiles simply follow one another in the
// predetermined order. If the width of the whole rectangle is not an exact multiple of 16
// then the width of the last tile in each row will be correspondingly smaller. Similarly
// if the height of the whole rectangle is not an exact multiple of 16 then the height of
// each tile in the final row will also be smaller.

//    Each tile is either encoded as raw pixel data, or as a variation on RRE.

// Each tile has a background pixel value, as before. The background pixel value does
// not need to be explicitly specified for a given tile if it is the same as the background
// of the previous tile. However the background pixel value may not be carried over if the previous tile was raw.

// If all of the subrectangles of a tile have the same pixel value, this can be specified
// once as a foreground pixel value for the whole tile. As with the background, the
// foreground pixel value can be left unspecified, meaning it is carried over from the
// previous tile.

// The foreground pixel value may not be carried over if the previous tile
// was raw or had the SubrectsColored bit set.

// It may, however, be carried over from a previous tile with the AnySubrects bit clear,
// as long as that tile itself carried over a valid foreground from its previous tile.

//    So the data consists of each tile encoded in order.
// Each tile begins with a subencoding type byte, which is a mask made up of a number of bits:

//    No. of bytes      Type
//    1                  U8

//    subencoding-mask:
//      1   Raw
//      2   BackgroundSpecified
//      4   ForegroundSpecified
//      8   AnySubrects
//      16  SubrectsColoured

//    If the Raw bit is set then the other bits are irrelevant;
// width * height pixel values follow (where width and height are the width
// and height of the tile).

// Otherwise the other bits in the mask are as follows:

//    BackgroundSpecified

//        If set, a pixel value follows which specifies the background colour for this tile:
//        No. of bytes      Type          Description
//        bytesPerPixel     PIXEL       background-pixel-value

//        The first non-raw tile in a rectangle must have this bit set.
// If this bit isn't set then the background is the same as the last tile.

//    ForegroundSpecified

//        If set, a pixel value follows which specifies the foreground colour
//        to be used for all subrectangles in this tile:

//        No. of bytes      Type        Description
//        bytesPerPixel     PIXEL   foreground-pixel-value

//        If this bit is set then the SubrectsColoured bit must be zero

//    AnySubrects

//        If set, a single byte follows giving the number of subrectangles following:
//        No. of bytes      Type        Description
//        1                 U8      number-of-subrectangles

//        If not set, there are no subrectangles (i.e. the whole tile is just solid
//        background colour).

//    SubrectsColoured

//        If set then each subrectangle is preceded by a pixel value giving the colour of
//        that subrectangle, so a subrectangle is:

//        No. of bytes      Type        Description
//        bytesPerPixel     PIXEL   subrect-pixel-value
//        1                 U8      x-and-y-position
//        1                 U8      width-and-height

//        If not set, all subrectangles are the same colour, the foreground colour; if the
// ForegroundSpecified bit wasn't set then the foreground is the same as the last tile.

// A subrectangle is:
//        No. of bytes 	Type 	Description
//        1     U8 	x-and-y-position
//        1     U8 	width-and-height

//    The position and size of each subrectangle is specified in two bytes,
// x-and-y-position and width-and-height. The most-significant four bits
// of x-and-y-position specify the X position, the least-significant specify
// the Y position. The most-significant four bits of width-and-height specify
// the width minus one, the least-significant specify the height minus one.

namespace VNC
{
namespace Encoder
{
namespace
{
    class Hextile
    {
        const BitsPerPixel bpp;
        const uint8_t Bpp;
        // rectangle we are refreshing
        const Rect r;
        // current tile
        Rect tile;
        // cursor used for next tile
        size_t cx_remain;
        size_t cy_remain;
        uint32_t fgPixel;
        uint32_t bgPixel;

        enum
        {
            hextileRaw = 1,
            hextileBackgroundSpecified = 2,
            hextileForegroundSpecified = 4,
            hextileAnySubrects = 8,
            hextileSubrectsColoured = 16,
        };

        VNCVerbose verbose;

    public:
        Hextile(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect, VNCVerbose verbose)
        : bpp(bpp), Bpp(safe_int(Bpp)), r(rect)
        , tile(Rect(this->r.x, this->r.y,
            std::min<size_t>(this->r.cx, 16),
            std::min<size_t>(this->r.cy, 16)))
        , cx_remain{0}
        , cy_remain{0}
        , fgPixel{0}
        , bgPixel{0}
        , verbose(verbose)
        {
            // remaining part of rect to draw, including current tile
            this->cx_remain = this->r.cx;
            this->cy_remain = this->r.cy;
        }

        // return is EncoderState::Exit if the Encoder has finished working (can be reset or deleted),
        // return is EncoderState::NeedMoreData if the encoder is waiting for more data
        EncoderState operator()(Buf64k & buf, gdi::GraphicApi & drawable)
        {
            if (this->r.isempty())
            {
                if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                    LOG(LOG_INFO, "Hextile::hexTileraw Encoder done (empty)");
                }
                return EncoderState::Exit;
            }

            size_t last_remaining = 0;
            while (buf.remaining()){
                if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                    LOG(LOG_INFO, "Rect=%s Tile = %s cx_remain=%zu, cy_remain=%zu", this->r, this->tile, this->cx_remain, this->cy_remain);
                }
                if (buf.remaining() == last_remaining){
                    LOG(LOG_ERR, "Hextile Stalled: Rect=%s Tile = %s cx_remain=%zu, cy_remain=%zu", this->r, this->tile, this->cx_remain, this->cy_remain);
                    assert(buf.remaining() != last_remaining);
                    throw Error(ERR_VNC_HEXTILE_PROTOCOL);
                }
                last_remaining = buf.remaining();

                Parse parser(buf.av().data());

                uint8_t tileType = parser.in_uint8();

                if (tileType & hextileRaw){
                    size_t raw_length = this->tile.cx * this->tile.cy * this->Bpp;
                    if (buf.remaining() < raw_length + 1){
                        if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                            LOG(LOG_INFO, "Hextile::hexTileraw need more data %zu (has %u)", raw_length+1, buf.remaining());
                        }
                        return EncoderState::NeedMoreData;
                    }
                    const uint8_t * raw(buf.av().data()+1);
                    this->draw_tile(raw, drawable);
                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "consumed raw tile_bytes %zu", raw_length+1);
                    }
                    buf.advance(raw_length + 1);
                    if (not this->next_tile()){
                        if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                            LOG(LOG_INFO, "Hextile::hexTileraw Encoder done (raw)");
                        }
                        return EncoderState::Exit;
                    }
                    continue;
                }
                // Keep a 16x16 tiledata buffer for the current tile

                const size_t type_bytes        = 1;
                const size_t any_subrect_bytes = ((tileType & hextileAnySubrects)!=0)?1:0;
                const size_t hextile_bg_bytes  = ((tileType & hextileBackgroundSpecified)!=0)?this->Bpp:0;
                const size_t hextile_fg_bytes  = ((tileType & hextileForegroundSpecified)!=0)?this->Bpp:0;

                const size_t header_bytes = type_bytes + any_subrect_bytes + hextile_bg_bytes + hextile_fg_bytes;
                if (buf.remaining() < header_bytes){
                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "Hextile::hexTileraw need more data %zu (has %hu)", header_bytes, buf.remaining());
                    }
                    return EncoderState::NeedMoreData;
                }

                if (tileType & hextileBackgroundSpecified){
                    this->bgPixel = parser.in_bytes_le(this->Bpp);
                }

                if (tileType & hextileForegroundSpecified){
                    this->fgPixel = parser.in_bytes_le(this->Bpp);
                }

                uint8_t nSubRects = 0;
                if (tileType & hextileAnySubrects) {
                    nSubRects = parser.in_uint8();
                }

                const size_t subrects_bytes = nSubRects * (2 +((tileType & hextileSubrectsColoured)?this->Bpp:0));
                const size_t tile_bytes = header_bytes + subrects_bytes;

                if (buf.remaining() < tile_bytes){
                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "Hextile::hexTileraw need more data %zu (has %u)", tile_bytes, buf.remaining());
                    }
                    return EncoderState::NeedMoreData; // finished decoding
                }

                uint8_t tile_data[16*16*4];

                memcpy(&tile_data[0*this->Bpp], &this->bgPixel, this->Bpp);
                memcpy(&tile_data[1*this->Bpp], tile_data,  this->Bpp);
                memcpy(&tile_data[2*this->Bpp], tile_data, 2*this->Bpp);
                memcpy(&tile_data[4*this->Bpp], tile_data, 4*this->Bpp);
                memcpy(&tile_data[8*this->Bpp], tile_data, 8*this->Bpp);
                memcpy(&tile_data[16*this->Bpp], tile_data, 16*this->Bpp);
                memcpy(&tile_data[32*this->Bpp], tile_data, 32*this->Bpp);
                memcpy(&tile_data[64*this->Bpp], tile_data, 64*this->Bpp);
                memcpy(&tile_data[128*this->Bpp], tile_data, 128*this->Bpp);

                if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                    LOG(LOG_INFO, "Rect=%s Tile = %s cx_remain=%zu, cy_remain=%zu", this->r, this->tile, this->cx_remain, this->cy_remain);
                }

                for (size_t q = 0 ; q < nSubRects ; q++){

                    if (tileType & hextileSubrectsColoured){
                        this->fgPixel = parser.in_bytes_le(this->Bpp);
                    }
                    const uint8_t xy = parser.in_uint8();
                    const uint8_t wh = parser.in_uint8();

                    const uint8_t x = ((xy >> 4) & 0xF);
                    const uint8_t y = (xy & 0xF);
                    const uint8_t w = ((wh >> 4) & 0xF) + 1;
                    const uint8_t h = (wh & 0xF) + 1;

                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "Hextile::subrect (%d, %d, %d, %d) color=%u", x, y, w, h, this->fgPixel);
                    }

                    if (x + w > 16 || y + h > 16) {
                        LOG(LOG_ERR, "Hextile::subrect (%d, %d, %d, %d) : bad subrect coordinates", x, y, w, h);
                        if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                            LOG(LOG_INFO, "Hextile encoding type=%.2x data=%u", tileType, buf.remaining());
                            LOG(LOG_INFO, "Rect=%s Tile = %s cx_remain=%zu, cy_remain=%zu", this->r, this->tile, this->cx_remain, this->cy_remain);
                            hexdump(buf.av().data(), std::min<uint16_t>(buf.remaining(), 1024));
                        }
                        throw Error(ERR_VNC_HEXTILE_PROTOCOL);
                    }
                    uint8_t * ptr = &tile_data[(y * this->tile.cx + x) * this->Bpp];
                    for (uint8_t qx = 0 ; qx < w ; qx++) {
                        memcpy(&ptr[qx*this->Bpp], &this->fgPixel, this->Bpp);
                    }
                    for (uint8_t qy = 1 ; qy < h ; qy++) {
                        memcpy(&ptr[this->tile.cx * qy * this->Bpp], ptr, w * this->Bpp);
                    }
                }

                this->draw_tile(tile_data, drawable);
                if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                    LOG(LOG_INFO, "consumed tile_bytes %zu", tile_bytes);
                }
                buf.advance(tile_bytes);


                if (not this->next_tile()){
                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "Hextile::hexTileraw Encoder done");
                    }
                    return EncoderState::Exit;
                }
            }
            if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                LOG(LOG_INFO, "Hextile::hexTileraw need more data (has %hu)", buf.remaining());
            }
            return EncoderState::NeedMoreData; // finished decoding
        }

        void draw_tile(const uint8_t * raw, gdi::GraphicApi & drawable)
        {
            update_lock<gdi::GraphicApi> lock(drawable);
            const Bitmap bmp(raw, this->tile.cx, this->tile.cy, this->bpp, Rect(0, 0, this->tile.cx, this->tile.cy));
            const RDPMemBlt cmd(0, this->tile, 0xCC, 0, 0, 0);
            drawable.draw(cmd, this->tile, bmp);
        }

        // return false if there is no next tile any more
        bool next_tile()
        {
            if (this->cx_remain <= 16){
                if (this->cy_remain <= 16){
                    return false;
                }
                this->cx_remain = this->r.cx;
                this->cy_remain -= 16;
                this->tile = Rect(this->r.x, this->tile.y + 16,
                                  std::min<size_t>(16, this->cx_remain),
                                  std::min<size_t>(16, this->cy_remain));
                return true;
            }
            this->cx_remain -= 16;
            this->tile.x += 16;
            this->tile.cx = std::min<size_t>(16, this->cx_remain);
            return true;
        }

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function")
        Rect current_tile() const
        {
            return this->tile;
        }
        REDEMPTION_DIAGNOSTIC_POP
    };
} // namespace

Encoder hextile_encoder(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect, VNCVerbose verbose)
{
  return Encoder(Hextile{bpp, Bpp, rect, verbose});
}

} // namespace Encoder
} // namespace VNC

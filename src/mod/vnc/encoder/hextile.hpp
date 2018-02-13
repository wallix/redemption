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

#pragma once

#include "utils/log.hpp"
#include "utils/verbose_flags.hpp"
#include "mod/vnc/vnc_verbose.hpp"

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

namespace VNC {
    namespace Encoder {
        class Hextile : public EncoderApi {
             const uint8_t bpp;
             const uint8_t Bpp;
             const Rect r;
             // current tile
             Rect tile;
             // cursor used for next tile
             size_t cx_remain;
             size_t cy_remain;
             uint8_t tile_data[16*16*4];
             uint8_t tileType;
             uint8_t nSubRects;
             uint32_t fgPixel;
             uint32_t bgPixel;

            enum {
                hextileRaw = 1,
                hextileBackgroundSpecified = 2,
                hextileForegroundSpecified = 4,
                hextileAnySubrects = 8,
                hextileSubrectsColoured = 16,
            };

        public:
             VNCVerbose verbose;

            Hextile(uint8_t bpp, uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, VNCVerbose verbose)
                 : bpp(bpp), Bpp(Bpp), r(x, y, cx, cy)
                 , tile(Rect(this->r.x, this->r.y, 
                      std::min<size_t>(this->r.cx, 16), 
                      std::min<size_t>(this->r.cy, 16)))
                 , cx_remain{0}
                 , cy_remain{0}
                 , tileType{0}
                 , nSubRects{0}
                 , fgPixel{0}
                 , bgPixel{0}
                 , verbose(verbose)
            {
                 this->cx_remain = (this->r.cx < 16)?this->r.cx:this->r.cx-16;
                 this->cy_remain = (this->r.cy < 16)?0:this->r.cy-16;
            }
            
            virtual ~Hextile(){}

            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            EncoderState consume(Buf64k & buf, gdi::GraphicApi & drawable) override
            {
                while (buf.remaining()){
                    Parse parser(buf.av().data());
                    if (!this->nSubRects){
                        this->tileType = parser.in_uint8();
                        if (tileType & hextileRaw){
                            size_t raw_length = this->tile.cx * this->tile.cy * this->Bpp;
                            if (buf.remaining() < raw_length + 1){
                                break;
                            }
                            this->draw_tile(parser.in_uint8p(raw_length), drawable);
                            this->next_tile();
                            buf.advance(raw_length + 1);
                        }
                        // Keep a 16x16 tiledata buffer for the current tile
                        if (this->tileType & hextileBackgroundSpecified){
                            this->bgPixel = parser.in_bytes_le(this->Bpp);
                            buf.advance(this->Bpp);
                        }

                        if (this->tileType & hextileForegroundSpecified){
                            this->fgPixel = parser.in_bytes_le(this->Bpp);
                            buf.advance(this->Bpp);
                        }

                        if (this->tileType & hextileAnySubrects) {
                            this->nSubRects = parser.in_uint8();
                            buf.advance(1);
                        }
                        uint8_t * ptr = &this->tile_data[0];
                        for (uint8_t h = 0 ; h < 16 ; h++) {
                            for (uint8_t w = 0 ; h < 16 ; w++) {
                                memcpy(ptr, &this->bgPixel, this->Bpp); 
                                ptr += this->Bpp;
                            }
                        }
                    }
                    else {
                        if (tileType & hextileSubrectsColoured){
                            this->fgPixel = parser.in_bytes_le(this->Bpp);
                            buf.advance(this->Bpp);
                        }
                        uint8_t xy = parser.in_uint8();
                        uint8_t wh = parser.in_uint8();
                        buf.advance(2);

                        int x = ((xy >> 4) & 15);
                        int y = (xy & 15);
                        int w = ((wh >> 4) & 15) + 1;
                        int h = (wh & 15) + 1;
                        if (x + w > 16 || y + h > 16) {
                            throw Error(ERR_VNC_ZRLE_PROTOCOL);
                        }
                        uint8_t * ptr = &this->tile_data[y * 16 * this->Bpp + x];
                        uint8_t rowoffset = 16 - w;
                        while (h-- > 0) {
                            while (w-- > 0) {
                                memcpy(ptr, &this->fgPixel, this->Bpp); ptr += Bpp;
                            }
                            ptr += rowoffset * this->Bpp;
                        }
                        this->nSubRects--;
                        if (!this->nSubRects){
                            this->draw_tile(this->tile_data, drawable);
                            this->next_tile();
                        }
                    }
                }
                return EncoderState::NeedMoreData; // finished decoding
            }

            void draw_tile(const uint8_t * raw, gdi::GraphicApi & drawable)
            {            
                update_lock<gdi::GraphicApi> lock(drawable);
                const Bitmap bmp(raw, this->tile.cx, this->tile.cy, this->bpp, Rect(0, 0, 16, 16));
                const RDPMemBlt cmd(0, this->tile, 0xCC, 0, 0, 0);
                drawable.draw(cmd, this->tile, bmp);
            }
            
            // return false if there is no next tile any more
            bool next_tile()
            {
                if (this->cy_remain == 0){
                    return false;
                }
                this->tile.cy = std::min<size_t>(this->cy_remain, 16);

                if (this->cx_remain <= 16){
                    this->tile.x = this->r.x;
                    this->tile.cx = this->cx_remain;
                    this->cx_remain = this->tile.cx;
                    this->tile.y += 16;
                    if (this->cy_remain < 16){
                        this->tile.cy = this->cy_remain;
                        this->cy_remain = 0;
                    }
                    this->tile.cy = 16;
                    this->cy_remain -= 16;
                    return true;
                }
                this->tile.x += 16;
                this->tile.cx = 16;
                this->cx_remain -= 16;
                return true;
            }
 
        };    
    } // namespace encoder
} // namespace VNC

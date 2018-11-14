/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Vnc module for zrle encoder
*/

#include "mod/vnc/encoder/zrle.hpp"
#include "cxx/diagnostic.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/buf64k.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/bitmap.hpp"
#include "utils/colors.hpp"
#include "utils/hexdump.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/update_lock.hpp"


namespace VNC
{
namespace Encoder
{
namespace
{
    class Zrle
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

        Zdecompressor<> & zd;

        enum class ZrleState {
            Header,
            Data,
            Exit
        } state = ZrleState::Header;
        uint32_t zlib_compressed_data_length = 0;
        std::vector<uint8_t> accumulator;
        std::vector<uint8_t> accumulator_uncompressed;

        VNCVerbose verbose;

    public:
        Zrle(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect, Zdecompressor<>& zd, VNCVerbose verbose)
        : bpp(bpp), Bpp(safe_int(Bpp)), r(rect)
        , tile(Rect(this->r.x, this->r.y,
            std::min<size_t>(this->r.cx, 64),
            std::min<size_t>(this->r.cy, 64)))
        , cx_remain{r.cx}
        , cy_remain{r.cy}
        , zd(zd)
        , verbose(verbose)
        {
            if (bool(this->verbose & VNCVerbose::zrle_trace)) {
                LOG(LOG_INFO, "New VNC::Encoder::ZRLE %d (%d %d %u %u)", bpp, r.x, r.y, r.cx, r.cy);
            }
        }

        EncoderState operator()(Buf64k & buf, gdi::GraphicApi & drawable)
        {
            if (this->r.isempty())
            {
                if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                    LOG(LOG_INFO, "Hextile::zrle Encoder done (empty rect)");
                }
                return EncoderState::Exit;
            }

            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "zrle consuming data  %zu", buf.av().size());
            }
//                hexdump_d(buf.av().data(), buf.av().size());
            switch (this->state) {
            case ZrleState::Header:
            {
                const size_t sz = sizeof(uint32_t);
                if (buf.remaining() < sz){
                    return EncoderState::NeedMoreData;
                }
                this->zlib_compressed_data_length = Parse(buf.av().data()).in_uint32_be();
                this->accumulator.clear();
                this->accumulator.reserve(this->zlib_compressed_data_length);
                this->accumulator_uncompressed.reserve(200000);
                this->accumulator_uncompressed.clear();
                if (bool(this->verbose & VNCVerbose::zrle_trace)) {
                    LOG(LOG_INFO, "ZRLE: consuming header data %zu bytes", sz);
                    hexdump_d(buf.av().data(), sz);
                }
                buf.advance(sz);
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "VNC Encoding: ZRLE, compressed length = %u remaining=%hu", this->zlib_compressed_data_length, buf.remaining());
                }
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "Zrle::EncoderReady::zrle remaining data  %zu", buf.av().size());
                }
                this->state = ZrleState::Data;
                return EncoderState::Ready;
            }
            case ZrleState::Data:
            {
                if (this->accumulator.size() + buf.remaining() < this->zlib_compressed_data_length)
                {
                    auto av = buf.av(buf.remaining());
                    this->accumulator.insert(this->accumulator.end(), av.begin(), av.end());
                    if (bool(this->verbose & VNCVerbose::zrle_trace)) {
                        LOG(LOG_INFO, "ZRLE: consuming compressed data %hu bytes", buf.remaining());
                        hexdump_d(buf.av().data(), buf.remaining());
                    }
                    buf.advance(buf.remaining());
                    if (bool(this->verbose & VNCVerbose::basic_trace)) {
                        LOG(LOG_INFO, "Zrle::Encoder::NeedMoreData::zrle remaining data  %zu", buf.av().size());
                    }
                    return EncoderState::NeedMoreData;
                }
                size_t interesting_part = this->zlib_compressed_data_length - this->accumulator.size();
                auto av = buf.av(interesting_part);
                this->accumulator.insert(this->accumulator.end(), av.begin(), av.end());
                if (bool(this->verbose & VNCVerbose::zrle_trace)) {
                    LOG(LOG_INFO, "ZRLE: consuming end of compressed data block : %zu bytes", interesting_part);
                    hexdump_d(buf.av().data(), interesting_part);
                }
                buf.advance(interesting_part);

                size_t data_ready = 0;
                const size_t step = 32768;
                for (size_t q = 0 ; q < this->zlib_compressed_data_length ;
                                    q += this->zd.update(&this->accumulator[q], std::min(step, this->zlib_compressed_data_length-q))){
                    if (this->zd.full()) {
                        data_ready += this->zd.flush_ready(this->accumulator_uncompressed);
                    }
                }
                while (!this->zd.finish() && this->zd.available()){
                    data_ready += this->zd.flush_ready(this->accumulator_uncompressed);
                }

                InStream zlib_uncompressed_data_stream(this->accumulator_uncompressed.data(), data_ready);
                this->accumulator.clear();

                this->lib_framebuffer_update_zrle(zlib_uncompressed_data_stream, drawable);

                this->state = ZrleState::Exit;
                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                    LOG(LOG_INFO, "Zrle::Encoder::Exit remaining data  %zu", buf.av().size());
                }
                return EncoderState::Exit;
            }
            default:
                LOG(LOG_ERR, "Unexpected state in ZrleEncoder (%u), should not happen", static_cast<unsigned>(this->state));
                break;
            }

            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "Zrle::Encoder Error remaining data  %zu", buf.av().size());
            }

            LOG(LOG_ERR, "VNC Encoding: ZRLE, unexpected encoding stream exit");
            throw Error(ERR_VNC_ZRLE_PROTOCOL);
        }

        // this one is used only in tests to check decompression behavior without compression layer
        // injection of compressed data in tests raise troubles because it depends of all the past history.

        // We are currently providing complete Streams encoding all the expected tiling rects
        // We can predict the number of expected rects (64x64 tiling of external rect is determinist)
        // But we can't predict the size of each individual rect before decoding.
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function")
        void rle_test_bypass(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            this->lib_framebuffer_update_zrle(uncompressed_data_buffer, drawable);
        }
        REDEMPTION_DIAGNOSTIC_POP

    private:
    //    7.6.9   ZRLE Encoding
    //    =====================

    //    ZRLE stands for Zlib [3] Run-Length Encoding, and combines zlib compression, tiling, palettisation and run-length encoding.
    //    On the wire, the rectangle begins with a 4-byte length field, and is followed by that many bytes of zlib-compressed data.
    //    A single zlib "stream" object is used for a given RFB protocol connection, so that ZRLE rectangles must be encoded and
    //    decoded strictly in order.

    //        | No. of bytes | Type     | Description |
    //        +--------------+----------+-------------+
    //        | 4            | U32      |  length     |
    //        +--------------+----------+-------------+
    //        | length       | U8 array | zlibData    |
    //        +--------------+----------+-------------+

    //    The zlibData when uncompressed represents tiles of 64x64 pixels in left-to-right, top-to-bottom order, similar to hextile.
    //    If the width of the rectangle is not an exact multiple of 64 then the width of the last tile in each row is smaller, and
    //    if the height of the rectangle is not an exact multiple of 64 then the height of each tile in the final row is smaller.

    //    ZRLE makes use of a new type CPIXEL (compressed pixel). This is the same as a PIXEL for the agreed pixel format, except
    //    where true-colour-flag is non-zero, bits-per-pixel is 32, depth is 24 or less and all of the bits making up the red, green
    //    and blue intensities fit in either the least significant 3 bytes or the most significant 3 bytes. In this case a CPIXEL is
    //    only 3 bytes long, and contains the least significant or the most significant 3 bytes as appropriate. bytesPerCPixel is the
    //    number of bytes in a CPIXEL.

    //    Note that for the corner case where bits-per-pixel is 32 and depth is 16 or less (this is a corner case, since the client
    //    is much better off using 16 or even 8 bits-per-pixels) a CPIXEL is still 3 bytes long. By convention, the three least
    //    significant bytes are used when both the three least and the three most significant bytes would cover the used bits.

    //    Each tile begins with a subencoding type byte. The top bit of this byte is set if the tile has been run-length encoded,
    //    clear otherwise. The bottom seven bits indicate the size of the palette used: zero means no palette, one means that the
    //    tile is of a single colour, 2 to 127 indicate a palette of that size. The possible values of subencoding are:

    //    0:  Raw pixel data. width * height pixel values follow (where width and height are the width and height of the tile):

    //        |  No. of bytes                   |  Type        |  Description |
    //        +---------------------------------+--------------+--------------+
    //        | width * height * bytesPerCPixel | CPIXEL array |    pixels    |
    //        ----------------------------------------------------------------+

    //    1:  A solid tile consisting of a single colour. The pixel value follows:

    //        |  No. of bytes  |  Type  |  Description     |
    //        +----------------+--------+------------------+
    //        | bytesPerCPixel | CPIXEL |    pixelValue    |
    //        +----------------+--------+------------------+

    //    2 to 16

    //        Packed palette types. Followed by the palette, consisting of paletteSize (=*subencoding*) pixel values. Then the packed
    //    pixels follow, each pixel represented as a bit field yielding an index into the palette (0 meaning the first palette entry).
    //    For paletteSize 2, a 1-bit field is used, for paletteSize 3 or 4 a 2-bit field is used and for paletteSize from 5 to 16 a
    //    4-bit field is used. The bit fields are packed into bytes, the most significant bits representing the leftmost pixel (i.e. big
    //     endian). For tiles not a multiple of 8, 4 or 2 pixels wide (as appropriate), padding bits are used to align each row to an
    //    exact number of bytes.

    //        |  No. of bytes                |  Type        |  Description     |
    //        +------------------------------+--------------+------------------+
    //        | paletteSize * bytesPerCPixel | CPIXEL array |    palette       |
    //        +------------------------------+--------------+------------------+
    //        |                            m |     U8 array |    packedPixels  |
    //        +------------------------------+--------------+------------------+

    //        where m is the number of bytes representing the packed pixels.

    //        For paletteSize of 2 this is floor((width + 7) / 8) * height,
    //        for paletteSize of 3 or 4 this is floor((width + 3) / 4) * height,
    //        for paletteSize of 5 to 16 this is floor((width + 1) / 2) * height.

    //    17 to 127
    //        Unused (no advantage over palette RLE).

    //    128

    //        Plain RLE. Consists of a number of runs, repeated until the tile is done. Runs may continue from the end of one row to
    //    the beginning of the next. Each run is a represented by a single pixel value followed by the length of the run. The length
    //    is represented as one or more bytes. The length is calculated as one more than the sum of all the bytes representing the
    //    length. Any byte value other than 255 indicates the final byte. So for example length 1 is represented as [0], 255 as [254],
    //    256 as [255,0], 257 as [255,1], 510 as [255,254], 511 as [255,255,0] and so on.

    //       | No. of bytes   | Type     | [Value] |      Description         |
    //       +----------------+----------+---------+--------------------------+
    //       | bytesPerCPixel | CPIXEL   |         |       pixelValue         |
    //       +----------------+----------+---------+--------------------------+
    //       |             r  | U8 array |   255   |                          |
    //       +----------------+----------+---------+--------------------------+
    //       |             1  |     U8   |         | (runLength - 1) % 255    |
    //       +----------------+----------+---------+--------------------------+

    //        Where r is floor((runLength - 1) / 255).

    //    129
    //        Unused.

    //    130 to 255

    //        Palette RLE. Followed by the palette, consisting of paletteSize = (subencoding - 128) pixel values:
    //        No. of bytes |Type |Description
    //        paletteSize * bytesPerCPixel |CPIXEL array |palette

    //        Then as with plain RLE, consists of a number of runs, repeated until the tile is done. A run of length one is
    //         represented simply by a palette index:

    //        | No. of bytes |Type | Description  |
    //        +--------------+-----+--------------+
    //        |            1 |  U8 | paletteIndex |
    //        +--------------+-----+--------------+

    //        A run of length more than one is represented by a palette index with the top bit set, followed by the length of the
    //        run as for plain RLE.

    //        | No. of bytes |     Type | [Value] |     Description       |
    //        +--------------+----------+---------+-----------------------+
    //        |            1 |       U8 |         | paletteIndex + 128    |
    //        +--------------+----------+---------+-----------------------+
    //        |            r | U8 array |     255 |                       |
    //        +--------------+----------+---------+-----------------------+
    //        |            1 |       U8 |         | (runLength - 1) % 255 |
    //        +--------------+----------+---------+-----------------------+

    //        Where r is floor((runLength - 1) / 255).

    private:
        void lib_framebuffer_update_zrle(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            while (uncompressed_data_buffer.in_remain())
            {
                if (bool(this->verbose & VNCVerbose::zrle_encoder)){
                    LOG(LOG_INFO, "lib_framebuffer_update_zrle remaining %zu", uncompressed_data_buffer.in_remain());
                    LOG(LOG_INFO, "Rect=%s Tile = %s cx_remain=%zu, cy_remain=%zu", this->r, this->tile, this->cx_remain, this->cy_remain);
                }

                uint8_t   subencoding = uncompressed_data_buffer.in_uint8();

                switch (subencoding) {
                case 0:
                    this->rawTile(uncompressed_data_buffer, drawable);
                break;
                case 1:
                    this->solidTile(uncompressed_data_buffer, drawable);
                break;
                case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
                case 10: case 11: case 12: case 13: case 14: case 15: case 16:
                    if (bool(this->verbose & VNCVerbose::zrle_encoder)){
                        LOG(LOG_INFO, "lib_framebuffer_update_zrle:: subencoding =%u", subencoding);
                        hexdump_d(uncompressed_data_buffer.get_current(), 1024);
                    }
                    this->packedPalette(subencoding, uncompressed_data_buffer, drawable);
                break;
                            case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
                case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
                case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
                case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
                case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
                case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
                case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
                case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
                case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
                case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
                case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
                case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
                case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
                case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
                case 129:
                // 17 to 127 Unused (no advantage over palette RLE).
                // 129 Unused
                {
                    LOG(LOG_ERR, "VNC Encoding: ZRLE, unused subencoding %u", subencoding);
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                case 128:
                    if (bool(this->verbose & VNCVerbose::zrle_encoder)){
                        LOG(LOG_INFO, "lib_framebuffer_update_zrle:: subencoding =%u", subencoding);
                        hexdump_d(uncompressed_data_buffer.get_current(), 1024);
                    }
                    this->plainRLE(uncompressed_data_buffer, drawable);
                    break;
                default: // 130 to 255
                    if (bool(this->verbose & VNCVerbose::zrle_encoder)){
                        LOG(LOG_INFO, "lib_framebuffer_update_zrle:: subencoding =%u", subencoding);
                        hexdump_d(uncompressed_data_buffer.get_current(), 1024);
                    }
                    this->paletteRLE(subencoding, uncompressed_data_buffer, drawable);
                    break;
                } // switch subencoding

                if (not this->next_tile()){
                    if (bool(this->verbose & VNCVerbose::hextile_encoder)){
                        LOG(LOG_INFO, "Hextile::hexTileraw Encoder done (raw) %zu", uncompressed_data_buffer.in_remain());
                    }
                    return;
                }
            } // while

            LOG(LOG_ERR, "Compressed VNC::zrle stream truncated : not enough compressed data");
            throw Error(ERR_VNC_ZRLE_PROTOCOL);
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
//                LOG(LOG_INFO, "Previous tile: rect=%s remain=(%d, %d) tile=%s", this->r, this->cx_remain, this->cy_remain, this->tile);

            if (this->cx_remain <= 64){
                if (this->cy_remain <= 64){
//                        LOG(LOG_INFO, "rect=%s remain=(%d, %d) tile=%s", this->r, this->cx_remain, this->cy_remain, this->tile);
                    return false;
                }
                this->cx_remain = this->r.cx;
                this->cy_remain -= 64;
                this->tile = Rect(this->r.x, this->tile.y + 64,
                                std::min<size_t>(64, this->cx_remain),
                                std::min<size_t>(64, this->cy_remain));
//                    LOG(LOG_INFO, "rect=%s remain=(%d, %d) tile=%s", this->r, this->cx_remain, this->cy_remain, this->tile);
                return true;
            }
            this->cx_remain -= 64;
            this->tile.x += 64;
            this->tile.cx = std::min<size_t>(64, this->cx_remain);
//                LOG(LOG_INFO, "rect=%s remain=(%d, %d) tile=%s", this->r, this->cx_remain, this->cy_remain, this->tile);
            return true;
        }


        // 0:  Raw pixel data. width * height pixel values follow (where width and height are the width and height of the tile):

        // |  No. of bytes                   |  Type        |  Description |
        // +---------------------------------+--------------+--------------+
        // | width * height * bytesPerCPixel | CPIXEL array |    pixels    |
        // +---------------------------------+--------------+--------------+

        void rawTile(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding: ZRLE, Raw pixel data");
            }

            const uint16_t tile_data_length = this->tile.cx * this->tile.cy * this->Bpp;

            if (uncompressed_data_buffer.in_remain() < tile_data_length)
            {
                LOG(LOG_ERR, "Compressed VNC::zrle stream truncated (1)");
                throw Error(ERR_VNC_ZRLE_PROTOCOL);
            }

            const uint8_t * tile_data_p = uncompressed_data_buffer.in_uint8p(tile_data_length);

            this->draw_tile(tile_data_p, drawable);
        }

        //    1:  A solid tile consisting of a single colour. The pixel value follows:

        //        |  No. of bytes  |  Type  |  Description     |
        //        +----------------+--------+------------------+
        //        | bytesPerCPixel | CPIXEL |    pixelValue    |
        //        +----------------+--------+------------------+
        void solidTile(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding: ZRLE, Solid tile (single color)");
            }
            const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(this->Bpp);

            auto const color_context= gdi::ColorCtx::depth16();
            auto pixel_color = RDPColor::from((cpixel_pattern[1]<<8)+cpixel_pattern[0]);
            const RDPOpaqueRect cmd(this->tile, pixel_color);
            drawable.draw(cmd, this->tile, color_context);

        }

        //  2 to 16 Packed palette types.

        // Followed by the palette, consisting of paletteSize (=*subencoding*) pixel values.
        // Then the packed pixels follow, each pixel represented as a bit field yielding an index into the palette
        // (0 meaning the first palette entry).
        //  For paletteSize 2, a 1-bit field is used,
        //  for paletteSize 3 or 4 a 2-bit field is used
        //  for paletteSize from 5 to 16 a 4-bit field is used.

        // The bit fields are packed into bytes, the most significant bits representing the leftmost pixel
        // (i.e. big endian).

        // For tiles not a multiple of 8, 4 or 2 pixels wide (as appropriate), padding bits are used to align
        // each row to an exact number of bytes.

        //        |  No. of bytes                |  Type        |  Description     |
        //        +------------------------------+--------------+------------------+
        //        | paletteSize * bytesPerCPixel | CPIXEL array |    palette       |
        //        +------------------------------+--------------+------------------+
        //        |                            m |     U8 array |    packedPixels  |
        //        +------------------------------+--------------+------------------+

        //        where m is the number of bytes representing the packed pixels.

        //        For paletteSize of 2 this is floor((width + 7) / 8) * height,
        //        for paletteSize of 3 or 4 this is floor((width + 3) / 4) * height,
        //        for paletteSize of 5 to 16 this is floor((width + 1) / 2) * height.

        // Note by CGR: what to do for invalid pattern referencing undefined palette color ?
        // We could either use some arbitrary Color or raise some VNC Error
        // As compatibility goes, we will LOG the error but accept the data and draw it as black

        void packedPalette(uint8_t subencoding, InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding: ZRLE, Packed palette types, palette size=%d", subencoding);
            }

            uint8_t         tile_data[64*64*4];    // max raw tile size with 32 bpp
            const uint8_t * tile_data_p = tile_data;

            const uint16_t   palette_size  = subencoding * this->Bpp;

            if (uncompressed_data_buffer.in_remain() < palette_size){
                LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (missing palette)");
                throw Error(ERR_VNC_ZRLE_PROTOCOL);
            }

            const uint8_t * palette = uncompressed_data_buffer.in_uint8p(palette_size);

            uint8_t pixels_per_byte = (subencoding>4)?2:(subencoding>2)?4:8;
            size_t line_bytes_width = (subencoding>4)?((this->tile.cx+1)>>1)
                                    : (subencoding>2)?((this->tile.cx+3)>>2)
                                    : (this->tile.cx+7)>>3;

            size_t packed_pixels_length = line_bytes_width * this->tile.cy;

            if (uncompressed_data_buffer.in_remain() < packed_pixels_length)
            {
                LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (missing palette data)");
                throw Error(ERR_VNC_ZRLE_PROTOCOL);
            }
            const uint8_t * packed_pixels = uncompressed_data_buffer.in_uint8p(packed_pixels_length);
            for (size_t y = 0 ; y < this->tile.cy ; y++){
                size_t x = 0;
                for (size_t i = 0 ; i < line_bytes_width ; i++){
                    uint8_t current_byte = packed_pixels[i];
                    for(uint8_t q = 0 ; q < pixels_per_byte ; q++){
                        if (x >= this->tile.cx) break;
                        uint8_t palette_index = 0;
                        if (subencoding > 4){ // 5 .. 16
//                               LOG(LOG_INFO, "subencoding=%u q=%u", subencoding, q);
                            switch (q){
                            case 0:
                                palette_index = current_byte >> 4;
                            break;
                            case 1:
                                palette_index = current_byte & 0xF;
                            break;
                            }
                        }
                        else if (subencoding > 2){ // 3 or 4
//                               LOG(LOG_INFO, "subencoding=%u q=%u", subencoding, q);
                            switch (q){
                                case 0:
                                    palette_index = (current_byte >> 6) & 3;
                                break;
                                case 1:
                                    palette_index = (current_byte >> 4) & 3;
                                break;
                                case 2:
                                    palette_index = (current_byte >> 2) & 3;
                                break;
                                case 3:
                                    palette_index = current_byte & 3;
                                break;
                            }
                        }
                        else { // 2
//                               LOG(LOG_INFO, "subencoding=%u q=%u", subencoding, q);
                            switch (q){
                                case 0:
                                    palette_index = (current_byte >> 7) & 1;
                                    break;
                                case 1:
                                    palette_index = (current_byte >> 6) & 1;
                                    break;
                                case 2:
                                    palette_index = (current_byte >> 5) & 1;
                                    break;
                                case 3:
                                    palette_index = (current_byte >> 4) & 1;
                                    break;
                                case 4:
                                    palette_index = (current_byte >> 3) & 1;
                                    break;
                                case 5:
                                    palette_index = (current_byte >> 2) & 1;
                                    break;
                                case 6:
                                    palette_index = (current_byte >> 1) & 1;
                                    break;
                                case 7:
                                    palette_index = current_byte & 1;
                                    break;
                            }
                        }
                        if (palette_index >= subencoding){
                            LOG(LOG_WARNING, "VNC::zrle uncompressed stream palette entry overflow) limit=%u palette_index=%u", subencoding, palette_index);
                            palette_index = 0;
                        }
                        memcpy(tile_data+(y*this->tile.cx+x)*this->Bpp, &palette[palette_index*this->Bpp], this->Bpp);
                        x++;
                    }
                }
                packed_pixels += line_bytes_width;
            }

            this->draw_tile(tile_data_p, drawable);
        }

        //  128   Plain RLE. Consists of a number of runs, repeated until the tile is done. Runs may continue from the end of one row to
        //    the beginning of the next. Each run is a represented by a single pixel value followed by the length of the run. The length
        //    is represented as one or more bytes. The length is calculated as one more than the sum of all the bytes representing the
        //    length. Any byte value other than 255 indicates the final byte. So for example length 1 is represented as [0], 255 as [254],
        //    256 as [255,0], 257 as [255,1], 510 as [255,254], 511 as [255,255,0] and so on.

        //       | No. of bytes   | Type     | [Value] |      Description         |
        //       +----------------+----------+---------+--------------------------+
        //       | bytesPerCPixel | CPIXEL   |         |       pixelValue         |
        //       +----------------+----------+---------+--------------------------+
        //       |             r  | U8 array |   255   |                          |
        //       +----------------+----------+---------+--------------------------+
        //       |             1  |     U8   |         | (runLength - 1) % 255    |
        //       +----------------+----------+---------+--------------------------+

        //        Where r is floor((runLength - 1) / 255).
        void plainRLE(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding: ZRLE, Plain RLE");
            }

            uint8_t    tile_data[4*64*64];    // max size with 32 bpp
            uint8_t  * tmp_tile_data = tile_data;
            for (;;){
                if (uncompressed_data_buffer.in_remain() < this->Bpp+1u){
                    LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (plainRLE)");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(this->Bpp);
                size_t length = uncompressed_data_buffer.in_uint8() + 1;
                if (length == 256){ // multi bytes length
                    length -= 1;
                    for(;;){
                        if (uncompressed_data_buffer.in_remain() < 1){
                            LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (plainRLE) truncated length");
                            throw Error(ERR_VNC_ZRLE_PROTOCOL);
                        }
                        uint8_t tmp = uncompressed_data_buffer.in_uint8();
                        length += tmp + 1;
                        if (tmp != 255){
                            break;
                        }
                        length = length - 1;
                    }
                }
                for (size_t i = 0 ; i < length ; i++){
                    if (tmp_tile_data == tile_data + this->tile.cx * this->tile.cy * this->Bpp){
                        LOG(LOG_ERR, "VNC::zrle uncompressed stream, plainRLE out of bound");
                        throw Error(ERR_VNC_ZRLE_PROTOCOL);
                    }
                    memcpy(tmp_tile_data, cpixel_pattern, this->Bpp);
                    tmp_tile_data+=this->Bpp;
                }
                if (tmp_tile_data == tile_data + this->tile.cx * this->tile.cy * this->Bpp){
                    break;
                }
            }

            this->draw_tile(tile_data, drawable);
        }

        //    130 to 255
        //        Palette RLE.

        //        Followed by the palette, consisting of paletteSize = (subencoding - 128) pixel values:

        //        | No. of bytes                 | Type         | Description  |
        //        +------------------------------+--------------+--------------+
        //        | paletteSize * bytesPerCPixel | CPIXEL array | palette      |
        //        +------------------------------+--------------+--------------+

        //        Then as with plain RLE, consists of a number of runs, repeated until the tile is done. A run of length one is
        //         represented simply by a palette index:

        //        | No. of bytes |Type | Description  |
        //        +--------------+-----+--------------+
        //        |            1 |  U8 | paletteIndex |
        //        +--------------+-----+--------------+

        //        A run of length more than one is represented by a palette index with the top bit set, followed by the length of the
        //        run as for plain RLE.

        //        | No. of bytes |     Type | [Value] |     Description       |
        //        +--------------+----------+---------+-----------------------+
        //        |            1 |       U8 |         | paletteIndex + 128    |
        //        +--------------+----------+---------+-----------------------+
        //        |            r | U8 array |     255 |                       |
        //        +--------------+----------+---------+-----------------------+
        //        |            1 |       U8 |         | (runLength - 1) % 255 |
        //        +--------------+----------+---------+-----------------------+

        //        Where r is floor((runLength - 1) / 255).

        void paletteRLE(uint8_t subencoding, InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
        {
            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                LOG(LOG_INFO, "VNC Encoding: ZRLE, Palette RLE");
            }

            const uint16_t   palette_size  = (subencoding & 0x7F) * this->Bpp;

            if (uncompressed_data_buffer.in_remain() < palette_size){
                LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (missing palette)");
                throw Error(ERR_VNC_ZRLE_PROTOCOL);
            }

            const uint8_t * palette = uncompressed_data_buffer.in_uint8p(palette_size);

            uint8_t    tile_data[4*64*64];    // max size with 32 bpp
            uint8_t  * tmp_tile_data = tile_data;
            for (;;){
                if (uncompressed_data_buffer.in_remain() < 1){
                    LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (paletteRLE)");
                    throw Error(ERR_VNC_ZRLE_PROTOCOL);
                }
                uint8_t palette_index = uncompressed_data_buffer.in_uint8();
                size_t length = 1;
                if ((palette_index & 0x80) != 0){
                    palette_index &= 0x7F;
                    if (uncompressed_data_buffer.in_remain() < 1){
                        LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated, length missing (paletteRLE)");
                        throw Error(ERR_VNC_ZRLE_PROTOCOL);
                    }
                    length = uncompressed_data_buffer.in_uint8() + 1;
                    if (length == 256){ // multi bytes length
                        length = length - 1;
                        for(;;){
                            if (uncompressed_data_buffer.in_remain() < 1){
                                LOG(LOG_ERR, "VNC::zrle uncompressed stream truncated (paletteRLE) truncated length");
                                throw Error(ERR_VNC_ZRLE_PROTOCOL);
                            }
                            uint8_t tmp = uncompressed_data_buffer.in_uint8();
                            length += tmp + 1;
                            if (tmp != 255){
                                break;
                            }
                            length = length - 1;
                        }
                    }
                }
//                    LOG(LOG_INFO, "paletteIndex=%u length=%u", palette_index, length);
                for (size_t i = 0 ; i < length ; i++){
                    if (tmp_tile_data == tile_data + this->tile.cx * this->tile.cy * this->Bpp){
                        LOG(LOG_ERR, "VNC::zrle uncompressed stream, plainRLE out of bound");
                        throw Error(ERR_VNC_ZRLE_PROTOCOL);
                    }
                    memcpy(tmp_tile_data, palette+palette_index*this->Bpp, this->Bpp);
                    tmp_tile_data+=this->Bpp;
                }
                if (tmp_tile_data == tile_data + this->tile.cx * this->tile.cy * this->Bpp){
                    break;
                }
            }
            this->draw_tile(tile_data, drawable);
        }
    }; // class Zrle
} // namespace

Encoder zrle_encoder(BitsPerPixel bpp, BytesPerPixel Bpp, Rect rect, Zdecompressor<>& zd, VNCVerbose verbose)
{
    return Encoder(Zrle{bpp, Bpp, rect, zd, verbose});
}

} // namespace Encoder
} // namespace VNC


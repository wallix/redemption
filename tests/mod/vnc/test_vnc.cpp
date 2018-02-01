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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define RED_TEST_MODULE TestVNC
#include "system/redemption_unit_tests.hpp"
#include "test_only/fake_graphic.hpp"
#include "test_only/check_sig.hpp"

#include "core/client_info.hpp"
#include "core/font.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/transport/test_transport.hpp"

RED_AUTO_TEST_CASE(TestFillEncodingTypesBuffer)
{
    struct testable_mod_vnc : mod_vnc {
        static void testable_fill_encoding_types_buffer(
                const char * encodings, OutStream & stream,
                uint16_t & number_of_encodings) {
            fill_encoding_types_buffer(encodings, stream, number_of_encodings,
                 VNCVerbose{});
        }
    };

    {
        StaticOutStream <512> stream;
        uint16_t number_of_encodings = 0;
        testable_mod_vnc::testable_fill_encoding_types_buffer("16,2,0,1,-239",
            stream, number_of_encodings);
        RED_CHECK(!memcmp(stream.get_data(),
                            "\x00\x00\x00\x10\x00\x00\x00\x02"
                            "\x00\x00\x00\x00\x00\x00\x00\x01"
                            "\xFF\xFF\xFF\x11",
                            20));
        RED_CHECK_EQUAL(number_of_encodings, 5);
    }

    {
        StaticOutStream <512> stream;
        uint16_t number_of_encodings = 0;
        testable_mod_vnc::testable_fill_encoding_types_buffer(
            "\t16 , 2 , 0 , 1 , -239 ", stream, number_of_encodings);
        RED_CHECK(!memcmp(stream.get_data(),
                            "\x00\x00\x00\x10\x00\x00\x00\x02"
                            "\x00\x00\x00\x00\x00\x00\x00\x01"
                            "\xFF\xFF\xFF\x11",
                            20));
        RED_CHECK_EQUAL(number_of_encodings, 5);
    }
}


RED_AUTO_TEST_CASE(TestVncMouse)
{
    char data[] =
        "\x05\x00\x00\x0a\x00\x0a"                          // move 10, 10
        "\x05\x08\x00\x0a\x00\x0a\x05\x00\x00\x0a\x00\x0a"  // scrool up
        "\x05\x01\x00\x0a\x00\x0a"                          // up left click
        "\x05\x03\x00\x0a\x00\x0a"                          // up right click
        "\x05\x02\x00\x0a\x00\x0a"                          // down left click
        "\x05\x02\x00\x0f\x00\x11"                          // move 15, 17
        "\x05\x00\x00\x0f\x00\x12"                          // down right click + move 15, 18
    ;
    CheckTransport t(data, sizeof(data)-1);
    mod_vnc::Mouse mouse;
    mouse.move(t, 10, 10);
    mouse.scroll(t, 8);
    mouse.click(t, 10, 10, 1, 1);
    mouse.click(t, 10, 10, 2, 1);
    mouse.click(t, 10, 10, 1, 0);
    mouse.move(t, 15, 17);
    mouse.click(t, 15, 18, 2, 0);
}


namespace VNC {
    namespace Encoder {
        enum class State
        {
            Header,
            Encoding,
            Data,
            ZrleData,
            RreData,
            Exit
        };

        class Zrle {
            const uint8_t Bpp;
            const size_t x;
            const size_t y;
            const size_t cx;
            const size_t cy;

            Zdecompressor<> & zd;

            enum class ZrleState {
                Header,
                Data,
                Exit
            } state;
            uint32_t zlib_compressed_data_length;
            std::vector<uint8_t> accumulator;
            std::vector<uint8_t> accumulator_uncompressed;


        public:

            VNCVerbose verbose;

            Zrle(uint8_t Bpp, size_t x, size_t y, size_t cx, size_t cy, Zdecompressor<> & zd, VNCVerbose verbose) 
                : Bpp(Bpp), x(x), y(y), cx(cx), cy(cy)
                , zd(zd)
                , state(ZrleState::Header)
                , zlib_compressed_data_length(0), accumulator{}, accumulator_uncompressed{}
                , verbose(verbose)
            {
            }

            // return is true if the Encoder has finished working (can be reset or deleted),
            // return is false if the encoder is waiting for more data
            bool consume(Buf64k & buf, gdi::GraphicApi & drawable)
            {
                LOG(LOG_INFO, "consuming buffer '%u bytes'", buf.remaining());
                switch (this->state) {
                case ZrleState::Header:
                {
                    const size_t sz = sizeof(uint32_t);
                    if (buf.remaining() < sz){ return false; }
                    this->zlib_compressed_data_length = Parse(buf.av().data()).in_uint32_be();
                    this->accumulator.clear();
                    this->accumulator.reserve(this->zlib_compressed_data_length);
                    this->accumulator_uncompressed.reserve(200000);
                    this->accumulator_uncompressed.clear();
                    buf.advance(sz);
                    if (bool(this->verbose & VNCVerbose::basic_trace))
                    {
                        LOG(LOG_INFO, "VNC Encoding: ZRLE, compressed length = %u remaining=%zu", this->zlib_compressed_data_length, buf.remaining());
                    }
                    this->state = ZrleState::Data;
                }
                break;
                case ZrleState::Data:
                {
                    LOG(LOG_INFO, "read_data_zrle_data %zu", buf.remaining());
//                    hexdump_d(buf.av().data(), buf.remaining());

                    LOG(LOG_INFO, "accumulator has %zu", this->accumulator.size());
                    if (this->accumulator.size() + buf.remaining() < this->zlib_compressed_data_length)
                    {
                        auto av = buf.av(buf.remaining());
                        this->accumulator.insert(this->accumulator.end(), av.begin(), av.end());
                        buf.advance(buf.remaining());
                        return false;
                    }
                    size_t interesting_part = this->zlib_compressed_data_length - this->accumulator.size();
                    auto av = buf.av(interesting_part);
                    this->accumulator.insert(this->accumulator.end(), av.begin(), av.end());
                    buf.advance(interesting_part);

                    LOG(LOG_INFO, "Got enough data for compressed zrle %u", this->zlib_compressed_data_length);

//                    hexdump_d(this->accumulator.data(), this->zlib_compressed_data_length);

                    ZRLEUpdateContext2 zrle_update_context(this->verbose);

                    LOG(LOG_INFO, "zrle_update_context Bpp=%u x=%zu cx=%zu cx_remain=%zu, cy_remain=%zu tile_x=%zu tile_y=%zu", 
                        Bpp, x, cx, cx, cy, x, y);

                    zrle_update_context.Bpp       = this->Bpp;
                    zrle_update_context.x         = this->x;
                    zrle_update_context.cx        = this->cx;
                    zrle_update_context.cx_remain = this->cx;
                    zrle_update_context.cy_remain = this->cy;
                    zrle_update_context.tile_x    = this->x;
                    zrle_update_context.tile_y    = this->y;

                    size_t data_ready = 0;
                    size_t consumed = 0;
                    while (this->zlib_compressed_data_length > 0){
                        // TODO: see in Zdecompresssor class, ensure some exception is raised if decompressor fails
                        size_t res = this->zd.update(this->accumulator.data() + consumed, this->zlib_compressed_data_length);
                        consumed += res;
                        // TODO: check we made progress
                        this->zlib_compressed_data_length -= res;

                        if (data_ready + this->zd.available() > this->accumulator_uncompressed.capacity()){
                            // if we don't have room enough, make room
                            this->accumulator_uncompressed.reserve(this->accumulator_uncompressed.capacity()+65536);
                        }
                        data_ready += this->zd.flush_ready(&this->accumulator_uncompressed[data_ready], this->accumulator_uncompressed.capacity() - data_ready);
                    }
                    // TODO: I should be able to merge that loop with the previous one
                    while (this->zd.available()){
                        if (data_ready + this->zd.available() > this->accumulator_uncompressed.capacity()){
                            // if we don't have room enough, make room
                            this->accumulator_uncompressed.reserve(this->accumulator_uncompressed.capacity()+65536);
                        }
                        data_ready += zd.flush_ready(&this->accumulator_uncompressed[data_ready], this->accumulator_uncompressed.capacity()-data_ready);
                    }

                    InStream zlib_uncompressed_data_stream(this->accumulator_uncompressed.data(), data_ready);

                    LOG(LOG_INFO, "read_data_zrle_data data_ready %zu zlib_compressed_data_length", data_ready);

                    zrle_update_context.lib_framebuffer_update_zrle(
                        zlib_uncompressed_data_stream,
                        drawable
                    );

                    this->accumulator.clear();
                    this->state = ZrleState::Exit;
                    return true;
                }
                default:
                    LOG(LOG_INFO, "Unexpected case, should not happen");
                    break;
                }
                return false;
            }

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

            struct ZRLEUpdateContext2
            {
                uint8_t Bpp;

                uint16_t x;
                uint16_t cx;

                uint16_t cx_remain;
                uint16_t cy_remain;

                uint16_t tile_x;
                uint16_t tile_y;

                StaticOutStream<16384> data_remain;

                VNCVerbose verbose;

                ZRLEUpdateContext2(VNCVerbose verbose) : verbose(verbose) {}

                bool is_first_membelt = true;

                void draw_tile(Rect rect, const uint8_t * raw, gdi::GraphicApi & drawable)
                {
                    const uint16_t TILE_CX = 32;
                    const uint16_t TILE_CY = 32;

                    for (int y = 0; y < rect.cy ; y += TILE_CY) {
                        uint16_t cy = std::min(TILE_CY, uint16_t(rect.cy - y));

                        for (int x = 0; x < rect.cx ; x += TILE_CX) {
                            uint16_t cx = std::min(TILE_CX, uint16_t(rect.cx - x));

                            const Rect src_tile(x, y, cx, cy);
                            // TODO: fix here magic number 16 is vnc.bpp
                            const Bitmap tiled_bmp(raw, rect.cx, rect.cy, 16, src_tile);
                            const Rect dst_tile(rect.x + x, rect.y + y, cx, cy);
                            const RDPMemBlt cmd2(0, dst_tile, 0xCC, 0, 0, 0);
                            /// NOTE force resize cliping with rdesktop...
                            if (this->is_first_membelt && dst_tile.cx != 1 && dst_tile.cy != 1) {
                                drawable.draw(cmd2, Rect(dst_tile.x,dst_tile.y,1,1), tiled_bmp);
                                this->is_first_membelt = false;
                            }
                            drawable.draw(cmd2, dst_tile, tiled_bmp);
                        }
                    }
                }

                void lib_framebuffer_update_zrle(InStream & uncompressed_data_buffer, gdi::GraphicApi & drawable)
                {
                    LOG(LOG_INFO, "lib_framebuffer_update_zrle %zu", uncompressed_data_buffer.in_remain());

                    uint8_t         tile_data[2*16384];    // max size with 16 bpp
                    uint8_t const * remaining_data        = nullptr;
                    uint16_t        remaining_data_length = 0;

                    try
                    {
                        while (uncompressed_data_buffer.in_remain())
                        {
                            uint16_t tile_cx = std::min<uint16_t>(this->cx_remain, 64);
                            uint16_t tile_cy = std::min<uint16_t>(this->cy_remain, 64);

                            const uint8_t * tile_data_p = tile_data;

                            uint16_t tile_data_length = tile_cx * tile_cy * this->Bpp;
                            if (tile_data_length > sizeof(tile_data))
                            {
                                LOG(LOG_ERR,
                                    "VNC Encoding: ZRLE, tile buffer too small (%zu < %" PRIu16 ")",
                                    sizeof(tile_data), tile_data_length);
                                throw Error(ERR_BUFFER_TOO_SMALL);
                            }

                            remaining_data        = uncompressed_data_buffer.get_current();
                            remaining_data_length = uncompressed_data_buffer.in_remain();

                            uint8_t   subencoding = uncompressed_data_buffer.in_uint8();

                            if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                LOG(LOG_INFO, "VNC Encoding: ZRLE, subencoding = %u",  subencoding);
                            }

                            switch (subencoding) {
                            case 0:
                            {
                                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                    LOG(LOG_INFO, "VNC Encoding: ZRLE, Raw pixel data");
                                }

                                if (uncompressed_data_buffer.in_remain() < tile_data_length)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                tile_data_p = uncompressed_data_buffer.in_uint8p(tile_data_length);
                            }
                            break;
                            //    1:  A solid tile consisting of a single colour. The pixel value follows:

                            //        |  No. of bytes  |  Type  |  Description     |
                            //        +----------------+--------+------------------+
                            //        | bytesPerCPixel | CPIXEL |    pixelValue    |
                            //        +----------------+--------+------------------+
                            case 1:
                            {
                                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                    LOG(LOG_INFO,
                                        "VNC Encoding: ZRLE, Solid tile (single color)");
                                }

                                if (uncompressed_data_buffer.in_remain() < this->Bpp)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(this->Bpp);

                                uint8_t * tmp_tile_data = tile_data;

                                for (int i = 0; i < tile_cx; i++, tmp_tile_data += this->Bpp)
                                    memcpy(tmp_tile_data, cpixel_pattern, this->Bpp);

                                uint16_t line_size = tile_cx * this->Bpp;

                                for (int i = 1; i < tile_cy; i++, tmp_tile_data += line_size)
                                    memcpy(tmp_tile_data, tile_data, line_size);
                            }
                            break;
                            case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
                            case 10: case 11: case 12: case 13: case 14: case 15: case 16:
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
                            {
                                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                    LOG(LOG_INFO,
                                        "VNC Encoding: ZRLE, Packed palette types, "
                                            "palette size=%d",
                                        subencoding);
                                }

                                const uint8_t  * palette;
                                const uint8_t    palette_count = subencoding;
                                const uint16_t   palette_size  = palette_count * this->Bpp;

                                if (uncompressed_data_buffer.in_remain() < palette_size)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                palette = uncompressed_data_buffer.in_uint8p(palette_size);

                                uint16_t   packed_pixels_length;

                                if (palette_count == 2)
                                {
                                    packed_pixels_length = (tile_cx + 7) / 8 * tile_cy;
                                }
                                else if ((palette_count == 3) || (palette_count == 4))
                                {
                                    packed_pixels_length = (tile_cx + 3) / 4 * tile_cy;
                                }
                                else// if ((palette_count >= 5) && (palette_count <= 16))
                                {
                                    packed_pixels_length = (tile_cx + 1) / 2 * tile_cy;
                                }

                                if (uncompressed_data_buffer.in_remain() < packed_pixels_length)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                const uint8_t * packed_pixels = uncompressed_data_buffer.in_uint8p(packed_pixels_length);

                                uint8_t * tmp_tile_data = tile_data;

                                uint16_t  tile_data_length_remain = tile_data_length;

                                uint8_t         pixel_remain         = tile_cx;
                                const uint8_t * packed_pixels_remain = packed_pixels;
                                uint8_t         current              = 0;
                                uint8_t         index                = 0;

                                uint8_t palette_index;

                                while (tile_data_length_remain >= this->Bpp)
                                {
                                    pixel_remain--;

                                    if (!index)
                                    {
                                        current = *packed_pixels_remain;
                                        packed_pixels_remain++;
                                    }

                                    if (palette_count == 2)
                                    {
                                        palette_index = (current & 0x80) >> 7;
                                        current <<= 1;
                                        index++;

                                        if (!pixel_remain || (index > 7))
                                        {
                                            index = 0;
                                        }
                                    }
                                    else if ((palette_count == 3) || (palette_count == 4))
                                    {
                                        palette_index = (current & 0xC0) >> 6;
                                        current <<= 2;
                                        index++;

                                        if (!pixel_remain || (index > 3))
                                        {
                                            index = 0;
                                        }
                                    }
                                    else// if ((palette_count >= 5) && (palette_count <= 16))
                                    {
                                        palette_index = (current & 0xF0) >> 4;
                                        current <<= 4;
                                        index++;

                                        if (!pixel_remain || (index > 1))
                                        {
                                            index = 0;
                                        }
                                    }

                                    if (!pixel_remain)
                                    {
                                        pixel_remain = tile_cx;
                                    }

                                    const uint8_t * cpixel_pattern = palette + palette_index * this->Bpp;

                                    memcpy(tmp_tile_data, cpixel_pattern, this->Bpp);

                                    tmp_tile_data           += this->Bpp;
                                    tile_data_length_remain -= this->Bpp;
                                }
                            }
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
                            //    17 to 127
                            //        Unused (no advantage over palette RLE).
                            {
                                LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                                throw Error(ERR_VNC_ZRLE_PROTOCOL);
                            }
                            break;
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
                            case 128:
                            {
                                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                    LOG(LOG_INFO, "VNC Encoding: ZRLE, Plain RLE");
                                }

                                uint16_t   tile_data_length_remain = tile_data_length;

                                uint16_t   run_length    = 0;
                                uint8_t  * tmp_tile_data = tile_data;

                                while (tile_data_length_remain >= this->Bpp)
                                {

                                    if (uncompressed_data_buffer.in_remain() < this->Bpp)
                                    {
                                        throw Error(ERR_VNC_NEED_MORE_DATA);
                                    }

                                    const uint8_t * cpixel_pattern = uncompressed_data_buffer.in_uint8p(this->Bpp);

                                    run_length = 1;

                                    while (true)
                                    {
                                        if (uncompressed_data_buffer.in_remain() < 1)
                                        {
                                            throw Error(ERR_VNC_NEED_MORE_DATA);
                                        }

                                        uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                                        run_length += byte_value;

                                        if (byte_value != 255)
                                            break;
                                    }

                                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                                    while ((tile_data_length_remain >= this->Bpp) && run_length)
                                    {
                                        memcpy(tmp_tile_data, cpixel_pattern, this->Bpp);

                                        tmp_tile_data           += this->Bpp;
                                        tile_data_length_remain -= this->Bpp;

                                        run_length--;
                                    }
                                }

                                // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                                assert(!run_length);
                                assert(!tile_data_length_remain);
                            }
                            break;
                            case 129:
                            //    129
                            //        Unused.
                            {
                                LOG(LOG_ERR, "VNC Encoding: ZRLE, unused");
                                throw Error(ERR_VNC_ZRLE_PROTOCOL);
                            }
                            break;
                            default:
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
                            {
                                if (bool(this->verbose & VNCVerbose::basic_trace)) {
                                    LOG(LOG_INFO, "VNC Encoding: ZRLE, Palette RLE");
                                }

                                const uint8_t  * palette;
                                const uint8_t    palette_count = subencoding - 128;
                                const uint16_t   palette_size  = palette_count * this->Bpp;

                                if (uncompressed_data_buffer.in_remain() < palette_size)
                                {
                                    throw Error(ERR_VNC_NEED_MORE_DATA);
                                }

                                palette = uncompressed_data_buffer.in_uint8p(palette_size);

                                uint16_t   tile_data_length_remain = tile_data_length;

                                uint16_t   run_length    = 0;
                                uint8_t  * tmp_tile_data = tile_data;

                                while (tile_data_length_remain >= this->Bpp)
                                {
                                    if (uncompressed_data_buffer.in_remain() < 1)
                                    {
                                        throw Error(ERR_VNC_NEED_MORE_DATA);
                                    }

                                    uint8_t         palette_index  = uncompressed_data_buffer.in_uint8();
                                    const uint8_t * cpixel_pattern = palette + (palette_index & 0x7F) * this->Bpp;

                                    run_length = 1;

                                    if (palette_index & 0x80)
                                    {
                                        while (true)
                                        {
                                            if (uncompressed_data_buffer.in_remain() < 1)
                                            {
                                                throw Error(ERR_VNC_NEED_MORE_DATA);
                                            }

                                            uint8_t byte_value = uncompressed_data_buffer.in_uint8();
                                            run_length += byte_value;

                                            if (byte_value != 255)
                                                break;
                                        }
                                    }

                                    // LOG(LOG_INFO, "VNC Encoding: ZRLE, run length=%u", run_length);

                                    while ((tile_data_length_remain >= this->Bpp) && run_length)
                                    {
                                        memcpy(tmp_tile_data, cpixel_pattern, this->Bpp);

                                        tmp_tile_data           += this->Bpp;
                                        tile_data_length_remain -= this->Bpp;

                                        run_length--;
                                    }
                                }

                                // LOG(LOG_INFO, "VNC Encoding: ZRLE, run_length=%u", run_length);

                                assert(!run_length);
                                assert(!tile_data_length_remain);
                            }
                            break;
                            } // switch subencoding

                            {            
                                update_lock<gdi::GraphicApi> lock(drawable);
                                this->draw_tile(Rect(this->tile_x, this->tile_y,
                                                     tile_cx, tile_cy),
                                                tile_data_p, drawable);
                            }

                            this->cx_remain -= tile_cx;
                            this->tile_x    += tile_cx;

                            if (!this->cx_remain)
                            {
                                this->cx_remain =  this->cx;
                                this->cy_remain -= tile_cy;

                                this->tile_x =  this->x;
                                this->tile_y += tile_cy;
                            }
                        } // while
                    } // try
                    catch (Error const& e)
                    {
                        if (e.id != ERR_VNC_NEED_MORE_DATA){
                            throw;
                        }
                        else
                        {
                            this->data_remain.out_copy_bytes(remaining_data, remaining_data_length);
                        }
                    }
                }
            }; // struct ZrleUpdateContext
        }; // class Zrle
    } // namespace Encoder
} // namespace VNC

#include "vnc_zrle_slice1_0_34.hpp"

RED_AUTO_TEST_CASE(TestZrle)
{
    Zdecompressor<> zd;

    const uint8_t rect1_header[] = {/* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x22, 0x00, 0x00, 0x00, 0x10,};
    const uint8_t compressed_len1[] = { 0x00, 0x00, 0xff, 0xad};
    const_byte_array datas[5] = {
         make_array_view(rect1_header),
         make_array_view(compressed_len1),
         make_array_view(slice1_0_34_p1),
         make_array_view(slice1_0_34_p2),
         make_array_view(slice1_0_34_p3)
    };
    Buf64k buf;
    class BlockWrap : public PartialReaderAPI
    {
        const_byte_array & t;
        size_t pos;
    public:
        BlockWrap(const_byte_array & t) : t(t), pos(0) {}
        
        size_t partial_read(byte_ptr buffer, size_t len) override
        {
            const size_t available = this->t.size() - this->pos;
            if (len >= available){
                std::memcpy(&buffer[0], &this->t[this->pos], available);
                this->pos += available;
                return available;
            }
            std::memcpy(&buffer[0], &this->t[this->pos], len);
            this->pos += len;
            return len;
        }
        bool empty() const {
            return this->t.size() == this->pos;
        }
    };

    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1920;
    info.height = 34;

    FakeGraphic drawable(info, 20);


    auto state = VNC::Encoder::State::Encoding;
    VNC::Encoder::Zrle * encoder = nullptr;
    for (auto t: datas){
        BlockWrap bw(t);
                
        while (!bw.empty()){
            buf.read_from(bw);
            switch (state){
                default:
                    LOG(LOG_INFO, "ignoring %zu\n", buf.av().size());
                    buf.advance(buf.av().size());
                    break;
                case VNC::Encoder::State::Encoding:
                {
                    const size_t sz = 12;
                    if (buf.remaining() < sz){ break; /* need more data */ }
                    InStream stream(buf.av(sz));
                    uint16_t x = stream.in_uint16_be();
                    uint16_t y = stream.in_uint16_be();
                    uint16_t cx = stream.in_uint16_be();
                    uint16_t cy = stream.in_uint16_be();
                    int32_t encoding = stream.in_sint32_be();
                    encoder = new VNC::Encoder::Zrle(2 /* Bpp */, x, y, cx, cy, zd, VNCVerbose::basic_trace);
                    LOG(LOG_INFO, "Encoding: (%u, %u, %u, %u) : %d", x, y, cx, cy, encoding);
                    buf.advance(sz);
                    // Post Assertion: we have an encoder
                    state = VNC::Encoder::State::Data;
                    break;
                }
                case VNC::Encoder::State::Data:
                {
                        // Pre Assertion: we have an encoder
                        if (encoder->consume(buf, drawable)){
                            // consume returns true if encoder is finished (ready to be resetted)
                            state = VNC::Encoder::State::Exit;
                            LOG(LOG_INFO, "End of encoder");
                        }
                }
                break;
            }
        }
        LOG(LOG_INFO, "All data consumed");
        drawable.save_to_png("vnc_first_len.png");
        char message[4096] = {};
        if (!redemption_unit_test__::check_sig(drawable.gd, message,
                                "\xd6\x38\xee\x6a\xa7\x49\x9e\x06\xa3\x6d\x08\xd1\xf3\x82\x8d\x63\xad\x23\x9d\x2f")){
            LOG(LOG_INFO, "signature mismatch: %s", message);
            BOOST_CHECK(false);
        }
    }
}



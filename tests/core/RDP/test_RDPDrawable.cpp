/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestRDPDrawable
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/transport/test_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"
#include "utils/bitmap_shrink.hpp"
#include "utils/drawable.hpp"
#include "utils/fileutils.hpp"
#include "utils/png.hpp"
#include "utils/bitmap.hpp"
#include "utils/rect.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/timestamp_tracer.hpp"

#include <chrono>


// #define STRINGIFY_I(x) #x
// #define STRINGIFY(x) STRINGIFY_I(x)
// #define DUMP_PNG(prefix, data) dump_png24(prefix STRINGIFY(__LINE__) ".png", data, true)

inline void server_add_char(
    GlyphCache & gly_cache, uint8_t cacheId, uint16_t cacheIndex
  , int16_t offset, int16_t baseline, uint16_t width, uint16_t height, const uint8_t * data)
{
    FontChar fi(offset, baseline, width, height, 0);
    memcpy(fi.data.get(), data, fi.datasize());
    // LOG(LOG_INFO, "cacheId=%u cacheIndex=%u", cacheId, cacheIndex);
    gly_cache.set_glyph(std::move(fi), cacheId, cacheIndex);
}

inline void process_glyphcache(GlyphCache & gly_cache, InStream & stream) {
    const uint8_t cacheId = stream.in_uint8();
    const uint8_t nglyphs = stream.in_uint8();
    for (uint8_t i = 0; i < nglyphs; i++) {
        const uint16_t cacheIndex = stream.in_uint16_le();
        const int16_t  offset     = stream.in_sint16_le();
        const int16_t  baseline   = stream.in_sint16_le();
        const uint16_t width      = stream.in_uint16_le();
        const uint16_t height     = stream.in_uint16_le();

        const unsigned int   datasize = (height * nbbytes(width) + 3) & ~3;
        const uint8_t      * data     = stream.in_uint8p(datasize);

        server_add_char(gly_cache, cacheId, cacheIndex, offset, baseline, width, height, data);
    }
}

RED_AUTO_TEST_CASE(TestGraphicGlyphIndex)
{
    uint16_t width = 1024;
    uint16_t height = 768;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);

    auto const color_cxt = gdi::ColorCtx::depth24();

    gd.draw(RDPOpaqueRect(screen_rect, encode_color24()(BLACK)), screen_rect, color_cxt);

    GlyphCache gly_cache;

    {
        uint8_t glyph_cache_data[] = {
/* 0000 */ 0x07, 0x06, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x07, 0x00, 0x08, 0x00, 0xf8, 0xcc, 0xc6, 0xc6,  // ................
/* 0010 */ 0xc6, 0xc6, 0xcc, 0xf8, 0x01, 0x00, 0x00, 0x00, 0xf7, 0xff, 0x06, 0x00, 0x09, 0x00, 0x18, 0x30,  // ...............0
/* 0020 */ 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0xc0, 0x7c, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xfa, 0xff,  // .x....|.........
/* 0030 */ 0x0a, 0x00, 0x06, 0x00, 0xfb, 0x80, 0xcc, 0xc0, 0xcc, 0xc0, 0xcc, 0xc0, 0xcc, 0xc0, 0xcc, 0xc0,  // ................
/* 0040 */ 0x03, 0x00, 0x00, 0x00, 0xfa, 0xff, 0x06, 0x00, 0x06, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0xcc, 0x7c,  // ..........x.|..|
/* 0050 */ 0x18, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfa, 0xff, 0x04, 0x00, 0x06, 0x00, 0xd0, 0xf0, 0xc0, 0xc0,  // ................
/* 0060 */ 0xc0, 0xc0, 0x18, 0x00, 0x05, 0x00, 0x00, 0x00, 0xfa, 0xff, 0x06, 0x00, 0x06, 0x00, 0x78, 0xcc,  // ..............x.
/* 0070 */ 0xfc, 0xc0, 0xc0, 0x7c, 0x18, 0x00, 0x09, 0x1b, 0xeb, 0x03, 0x38, 0x07, 0x03, 0x01, 0xd4, 0xd0,  // ...|......8.....
/* 0080 */ 0xc8, 0x16, 0x00, 0xea, 0x02, 0x4d, 0x00, 0xf7, 0x02, 0x16, 0x00, 0xf5, 0x02, 0x13, 0x00, 0x00,  // .....M..........
/* 0090 */ 0x01, 0x08, 0x02, 0x07, 0x03, 0x0b, 0x04, 0x07, 0x04, 0x05, 0x05, 0x05, 0x04, 0x07, 0xff, 0x00,  // ................
/* 00a0 */ 0x10,                                               // .
        };
        InStream stream(glyph_cache_data);
        process_glyphcache(gly_cache, stream);

        Rect rect_bk(22, 746, 56, 14);
        Rect rect_op(0, 0, 1, 1);
        RDPBrush brush;
        RDPGlyphIndex glyph_index(7,                            // cacheId
                                  3,                            // flAccel
                                  0,                            // ulCharInc
                                  1,                            // fOpRedundant
                                  encode_color24()(BGRColor(0x000000)),           // BackColor
                                  encode_color24()(BGRColor(0xc8d0d4)),           // ForeColor
                                  rect_bk,                      // Bk
                                  rect_op,                      // Op
                                  RDPBrush(),                   // Brush
                                  22,                           // X
                                  757,                          // Y
                                  19,                           // cbData
                                  byte_ptr_cast(                // rgbData
                                          "\x00\x00\x01\x08"
                                          "\x02\x07\x03\x0b"
                                          "\x04\x07\x04\x05"
                                          "\x05\x05\x04\x07"
                                          "\xff\x00\x10"
                                      )
                                 );

        Rect rect_clip(0, 0, 1024, 768);
        gd.draw(glyph_index, rect_clip, color_cxt, gly_cache);

        RDPGlyphIndex glyph_index_1(7,                              // cacheId
                                    3,                              // flAccel
                                    0,                              // ulCharInc
                                    1,                              // fOpRedundant
                                    encode_color24()(BGRColor(0x000000)),             // BackColor
                                    encode_color24()(BGRColor(0xc8d0d4)),             // ForeColor
                                    rect_bk,                        // Bk
                                    rect_op,                        // Op
                                    RDPBrush(),                     // Brush
                                    22,                             // X
                                    757,                            // Y
                                    3,                              // cbData
                                    byte_ptr_cast("\xfe\x00\x00")   // rgbData
                                   );

        gd.draw(glyph_index, rect_clip, color_cxt, gly_cache);
    }

    RED_CHECK_SIG(gd, "\xd8\xf7\x6e\xf5\xd1\xe6\x4a\x05\x56\x0a\x21\x42\xa4\x27\x73\x5a\xce\x67\xf6\xb3");

    // uncomment to see result in png file
    //DUMP_PNG("test_glyph_000_", gd);
}

RED_AUTO_TEST_CASE(TestPolyline)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    auto const color_cxt = gdi::ColorCtx::depth24();
    gd.draw(RDPOpaqueRect(screen_rect, encode_color24()(WHITE)), screen_rect, color_cxt);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), encode_color24()(BLACK)), screen_rect, color_cxt);

    constexpr std::size_t array_size = 1024;
    uint8_t array[array_size];
    OutStream deltaPoints(array, array_size);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(20);

    deltaPoints.out_sint16_le(160);
    deltaPoints.out_sint16_le(0);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(-50);
    deltaPoints.out_sint16_le(50);

    deltaPoints.out_sint16_le(0);
    deltaPoints.out_sint16_le(-30);

    deltaPoints.out_sint16_le(-160);
    deltaPoints.out_sint16_le(0);

    InStream dp(array, deltaPoints.get_offset());

    gd.draw(RDPPolyline(158, 230, 0x06, 0, encode_color24()(WHITE), 7, dp), screen_rect, color_cxt);

    RED_CHECK_SIG(gd, "\x32\x60\x8b\x02\xb9\xa2\x83\x27\x0f\xa9\x67\xef\x3c\x2e\xa0\x25\x69\x16\x02\x2b");

    // uncomment to see result in png file
    //DUMP_PNG("/tmp/test_polyline_000_", gd);
}

RED_AUTO_TEST_CASE(TestMultiDstBlt)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    auto const color_cxt = gdi::ColorCtx::depth24();
    gd.draw(RDPOpaqueRect(screen_rect, encode_color24()(WHITE)), screen_rect, color_cxt);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), encode_color24()(GREEN)), screen_rect, color_cxt);

    StaticOutStream<1024> deltaRectangles;

    deltaRectangles.out_sint16_le(100);
    deltaRectangles.out_sint16_le(100);
    deltaRectangles.out_sint16_le(10);
    deltaRectangles.out_sint16_le(10);

    for (int i = 0; i < 19; i++) {
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
    }

    InStream deltaRectangles_in(deltaRectangles.get_bytes());

    gd.draw(RDPMultiDstBlt(100, 100, 200, 200, 0x55, 20, deltaRectangles_in), screen_rect);

    RED_CHECK_SIG(gd, "\x3d\x83\xd7\x7e\x0b\x3e\xf4\xd1\x53\x50\x33\x94\x1e\x11\x46\x9c\x60\x76\xd7\x0a");

    // uncomment to see result in png file
    //DUMP_PNG("/tmp/test_multidstblt_000_", gd);
}

RED_AUTO_TEST_CASE(TestMultiOpaqueRect)
{
    // Create a simple capture image and dump it to file
    uint16_t width = 640;
    uint16_t height = 480;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height);
    auto const color_cxt = gdi::ColorCtx::depth24();
    gd.draw(RDPOpaqueRect(screen_rect, encode_color24()(WHITE)), screen_rect, color_cxt);
    gd.draw(RDPOpaqueRect(screen_rect.shrink(5), encode_color24()(GREEN)), screen_rect, color_cxt);

    StaticOutStream<1024> deltaRectangles;

    deltaRectangles.out_sint16_le(100);
    deltaRectangles.out_sint16_le(100);
    deltaRectangles.out_sint16_le(10);
    deltaRectangles.out_sint16_le(10);

    for (int i = 0; i < 19; i++) {
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
        deltaRectangles.out_sint16_le(10);
    }

    InStream deltaRectangles_in(deltaRectangles.get_bytes());

    gd.draw(RDPMultiOpaqueRect(100, 100, 200, 200, encode_color24()(BLACK), 20, deltaRectangles_in), screen_rect, color_cxt);

    RED_CHECK_SIG(gd, "\x1d\x52\x8e\x03\x43\xc8\x99\x8d\xeb\x51\xa6\x23\x91\x24\xab\x8c\xa4\xcc\xf0\xc8");

    // uncomment to see result in png file
    //DUMP_PNG("/tmp/test_multiopaquerect_000_", gd);
}


const char expected_red[] =
    /* 0000 */ "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"                                 //.PNG....
    /* 0000 */ "\x00\x00\x00\x0d\x49\x48\x44\x52"                                 //....IHDR
    /* 0000 */ "\x00\x00\x03\x20\x00\x00\x02\x58\x08\x02\x00\x00\x00"             //... ...X.....
    /* 0000 */ "\x15\x14\x15\x27"                                                 //...'
    /* 0000 */ "\x00\x00\x0a\xa9\x49\x44\x41\x54"                                 //....IDAT
    /* 0000 */ "\x78\x9c\xed\xd6\xc1\x09\x00\x20\x10\xc0\x30\x75\xff\x9d\xcf\x25" //x...... ..0u...%
    /* 0010 */ "\x0a\x82\x24\x13\xf4\xd9\x3d\x0b\x00\x80\xd2\x79\x1d\x00\x00\xf0" //..$...=....y....
    /* 0020 */ "\x1b\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //.....3X..1....3X
    /* 0030 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0040 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0050 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0060 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0070 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0080 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0090 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 00a0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 00b0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 00c0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 00d0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 00e0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 00f0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0100 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0110 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0120 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0130 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0140 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0150 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0160 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0170 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0180 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0190 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 01a0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 01b0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 01c0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 01d0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 01e0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 01f0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0200 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0210 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0220 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0230 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0240 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0250 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0260 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0270 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0280 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0290 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 02a0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 02b0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 02c0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 02d0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 02e0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 02f0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0300 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0310 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0320 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0330 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0340 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0350 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0360 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0370 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0380 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0390 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 03a0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 03b0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 03c0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 03d0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 03e0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 03f0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0400 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0410 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0420 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0430 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0440 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0450 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0460 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0470 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0480 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0490 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 04a0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 04b0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 04c0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 04d0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 04e0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 04f0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0500 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0510 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0520 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0530 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0540 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0550 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0560 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0570 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0580 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0590 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 05a0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 05b0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 05c0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 05d0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 05e0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 05f0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0600 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0610 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0620 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0630 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0640 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0650 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0660 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0670 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0680 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0690 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 06a0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 06b0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 06c0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 06d0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 06e0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 06f0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0700 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0710 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0720 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0730 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0740 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0750 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0760 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0770 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0780 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0790 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 07a0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 07b0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 07c0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 07d0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 07e0 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 07f0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0800 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0810 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0820 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0830 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0840 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0850 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0860 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0870 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0880 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0890 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 08a0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 08b0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 08c0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 08d0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 08e0 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 08f0 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0900 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0910 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0920 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0930 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0940 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0950 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0960 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0970 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0980 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0990 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 09a0 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 09b0 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 09c0 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 09d0 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 09e0 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 09f0 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0a00 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0a10 */ "\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00" //X..1....3X..1...
    /* 0a20 */ "\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83" //.3X..1....3X..1.
    /* 0a30 */ "\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00" //...3X..1....3X..
    /* 0a40 */ "\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58" //1....3X..1....3X
    /* 0a50 */ "\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10" //..1....3X..1....
    /* 0a60 */ "\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05" //3X..1....3X..1..
    /* 0a70 */ "\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31" //..3X..1....3X..1
    /* 0a80 */ "\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33\x58\x00" //....3X..1....3X.
    /* 0a90 */ "\x00\x31\x83\x05\x00\x10\x33\x58\x00\x00\x31\x83\x05\x00\x10\x33" //.1....3X..1....3
    /* 0aa0 */ "\x58\x00\x00\xb1\x0b\xbb\xfd\x05\xaf"                             //X........
    /* 0000 */ "\x0d\x9d\x5e\xa4"                                                 //..^.
    /* 0000 */ "\x00\x00\x00\x00\x49\x45\x4e\x44"                                 //....IEND
    /* 0000 */ "\xae\x42\x60\x82"                                                 //.B`.
    ;

RED_AUTO_TEST_CASE(TestTransportPngOneRedScreen)
{
    // This is how the expected raw PNG (a big flat RED 800x600 screen)
    // will look as a string

    RDPDrawable d(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), encode_color24()(RED));
    d.draw(cmd, screen_rect, color_cxt);
    TestTransport trans("", 0, expected_red, sizeof(expected_red)-1);
    dump_png24(trans, d.impl(), true);
}

RED_AUTO_TEST_CASE(TestImageCapturePngOneRedScreen)
{
    CheckTransport trans(expected_red, sizeof(expected_red)-1);
    RDPDrawable drawable(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), encode_color24()(RED));
    drawable.draw(cmd, screen_rect, color_cxt);
    dump_png24(trans, drawable, true);
}

RED_AUTO_TEST_CASE(TestImageCaptureToFilePngOneRedScreen)
{
    const char * filename = "test.png";
    size_t len = strlen(filename);
    char path[1024];
    memcpy(path, filename, len);
    path[len] = 0;
    int fd = ::creat(path, 0777);
    RED_REQUIRE_NE(fd, -1);

    OutFileTransport trans(unique_fd{fd});
    RDPDrawable drawable(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), encode_color24()(RED));
    drawable.draw(cmd, screen_rect, color_cxt);
    dump_png24(trans, drawable, true);
    trans.disconnect(); // close file before checking size
    RED_CHECK_EQUAL(2786, filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestImageCaptureToFilePngBlueOnRed)
{
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "test", ".png", groupid, ReportError{});
    RDPDrawable drawable(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    Rect screen_rect(0, 0, 800, 600);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), encode_color24()(RED));
    drawable.draw(cmd, screen_rect, color_cxt);
    dump_png24(trans, drawable, true);

    RDPOpaqueRect cmd2(Rect(50, 50, 100, 50), encode_color24()(BLUE));
    drawable.draw(cmd2, screen_rect, color_cxt);
    trans.next();

    dump_png24(trans, drawable, true);
    trans.next();

    const char * filename;

    filename = trans.seqgen()->get(0);
    RED_CHECK_EQUAL(2786, ::filesize(filename));
    ::unlink(filename);
    filename = trans.seqgen()->get(1);
    RED_CHECK_EQUAL(2806, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestOneRedScreen)
{
    struct timeval now;
    now.tv_sec = 1000;
    now.tv_usec = 0;

    Rect screen_rect(0, 0, 800, 600);
    const int groupid = 0;
    struct CleanupTransport : OutFilenameSequenceTransport {
        CleanupTransport()
        : OutFilenameSequenceTransport(
            FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION,
            "./", "xxxtest", ".png", groupid, ReportError{})
        {}

        ~CleanupTransport() {
            for(unsigned until_num = this->get_seqno() + 1u; this->num_start < until_num; ++this->num_start) {
                ::unlink(this->seqgen()->get(this->num_start));
            }
        }

        bool next() override {
            if (this->png_limit && this->get_seqno() >= this->png_limit) {
                // unlink may fail, for instance if file does not exist, just don't care
                ::unlink(this->seqgen()->get(this->get_seqno() - this->png_limit));
            }
            return OutFilenameSequenceTransport::next();
        }

        unsigned num_start = 0;
        unsigned png_limit = 3;
    } trans;
    RDPDrawable drawable(800, 600);

    class ImageCaptureLocal
    {
    private:
        Transport & trans;
        Drawable & drawable;
        uint8_t save_mouse[3072];
        int save_mouse_x = 0;
        int save_mouse_y = 0;
        int mouse_cursor_pos_x;
        int mouse_cursor_pos_y;
    public:
        bool dont_show_mouse_cursor;
    private:
        const DrawablePointer * current_pointer;
        DrawablePointer dynamic_pointer;
        DrawablePointer default_pointer;

        TimestampTracer timestamp_tracer;

    public:
        ImageCaptureLocal(Drawable & drawable, Transport & trans)
        : trans(trans)
        , drawable(drawable)
        , mouse_cursor_pos_x(800 / 2)
        , mouse_cursor_pos_y(600 / 2)
        , dont_show_mouse_cursor(false)
        , current_pointer(&this->default_pointer)
        , timestamp_tracer(gdi::get_mutable_image_view(drawable))
        {}

        std::chrono::microseconds do_snapshot(
            const timeval & now, int x, int y, bool ignore_frame_in_timeval
        ) {
            (void)x;
            (void)y;
            (void)ignore_frame_in_timeval;
            using std::chrono::microseconds;
            if (!this->dont_show_mouse_cursor && this->current_pointer){
                this->save_mouse_x = this->mouse_cursor_pos_x;
                this->save_mouse_y = this->mouse_cursor_pos_y;
                this->drawable.trace_mouse(this->current_pointer, this->mouse_cursor_pos_x, this->mouse_cursor_pos_y, this->save_mouse);
            }
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->timestamp_tracer.trace(ptm);
            this->flush();
            this->trans.next();
            this->timestamp_tracer.clear();
            if (!this->dont_show_mouse_cursor && this->current_pointer){
                this->drawable.clear_mouse(this->current_pointer, this->save_mouse_x, this->save_mouse_y, this->save_mouse);
            }
            return microseconds::zero();
        }

        void flush() {
            dump_png24(this->trans, this->drawable, true);
        }
    };

    ImageCaptureLocal consumer(drawable.impl(), trans);

    consumer.dont_show_mouse_cursor = true;

    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), encode_color24()(RED));
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(cmd, screen_rect, color_cxt);

    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(1)));

    bool ignore_frame_in_timeval = false;

    now.tv_sec++; consumer.do_snapshot(now, 0, 0, ignore_frame_in_timeval);

    RED_CHECK_EQUAL(3052, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(1)));

    now.tv_sec++; consumer.do_snapshot(now, 0, 0, ignore_frame_in_timeval);

    RED_CHECK_EQUAL(3052, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(3061, ::filesize(trans.seqgen()->get(1)));
    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(2)));

    now.tv_sec++; consumer.do_snapshot(now, 0, 0, ignore_frame_in_timeval);

    RED_CHECK_EQUAL(3052, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(3061, ::filesize(trans.seqgen()->get(1)));
    RED_CHECK_EQUAL(3057, ::filesize(trans.seqgen()->get(2)));
    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(3)));

    now.tv_sec++; consumer.do_snapshot(now, 0, 0, ignore_frame_in_timeval);

    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(3061, ::filesize(trans.seqgen()->get(1)));
    RED_CHECK_EQUAL(3057, ::filesize(trans.seqgen()->get(2)));
    RED_CHECK_EQUAL(3059, ::filesize(trans.seqgen()->get(3)));
    RED_CHECK_EQUAL(-1, ::filesize(trans.seqgen()->get(4)));

    for (unsigned i = 1; i <= 3; ++i) {
        unlink(trans.seqgen()->get(i));
    }
}

RED_AUTO_TEST_CASE(TestSmallImage)
{
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "sample", ".png", groupid, ReportError{});
    Rect scr(0, 0, 20, 10);
    RDPDrawable drawable(20, 10);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(RED)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(5, 5, 10, 3), encode_color24()(BLUE)), scr, color_cxt);
    drawable.draw(RDPOpaqueRect(Rect(10, 0, 1, 10), encode_color24()(WHITE)), scr, color_cxt);
    dump_png24(trans, drawable, true);
    trans.next();
    const char * filename = trans.seqgen()->get(0);
    RED_CHECK_EQUAL(107, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestScaleImage)
{
    const int width = 800;
    const int height = 600;
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "test_scale", ".png", groupid, ReportError{});
    Rect scr(0, 0, width, height);
    RDPDrawable drawable(scr.cx, scr.cy);

    {
        const char * filename = FIXTURES_PATH "/win2008capture10.png";
        // TODO "Add ability to write image to file or read image from file in RDPDrawable"
        read_png24(filename, gdi::get_mutable_image_view(drawable));
    }

    // TODO: zooming should be managed by some dedicated Drawable
    unsigned zoom_factor = 50;
    unsigned scaled_width = (((drawable.width() * zoom_factor) / 100) + 3) & 0xFFC;
    unsigned scaled_height = (drawable.height() * zoom_factor) / 100;
    std::unique_ptr<uint8_t[]> scaled_buffer = nullptr;
    scaled_buffer.reset(new uint8_t[scaled_width * scaled_height * 3]);

    // Zoom 50
    scale_data(
        scaled_buffer.get(), drawable.data(),
        scaled_width, drawable.width(),
        scaled_height, drawable.height(),
        drawable.rowsize());

    ::dump_png24(
        trans, scaled_buffer.get(),
        scaled_width, scaled_height,
        scaled_width * 3, false);
    trans.next();

    // TODO "check this: BGR/RGB problem i changed 8176 to 8162 to fix test"
    const char * filename = trans.seqgen()->get(0);
    RED_CHECK_EQUAL(8162, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestBogusBitmap)
{
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "bogus", ".png", groupid, ReportError{});
    Rect scr(0, 0, 800, 600);
    RDPDrawable drawable(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);

    uint8_t source64x64[] = {
// MIX_SET 60 remaining=932 bytes pix=0
/* 0000 */ 0xc0, 0x2c, 0xdf, 0xff,                                      // .,..
// COPY 6 remaining=931 bytes pix=60
/* 0000 */ 0x86, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,           // .............
// FOM_SET 3006 remaining=913 bytes pix=66
/* 0000 */ 0xf7, 0xbe, 0x0b, 0x01, 0xF0, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // ................
/* 0010 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0020 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0030 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0040 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0050 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0060 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0070 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0080 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0090 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00a0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00b0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00c0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00d0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 00e0 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 00f0 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0100 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0110 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0120 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0130 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0140 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0150 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ....QDDDDDDDDDDD
/* 0160 */ 0x44, 0x44, 0x44, 0x44, 0x04, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDDD............
/* 0170 */ 0x11, 0x11, 0x11, 0x11, 0x51, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x04,           // ....QDDDDDDD.

// BICOLOR 64 remaining=532 bytes pix=3072
/* 0000 */ 0xe0, 0x10, 0x55, 0xad, 0x35, 0xad,                                // ..U.5.
// COPY 63 remaining=530 bytes pix=3136
/* 0000 */ 0x80, 0x1f, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0010 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0020 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0030 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0040 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0050 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0060 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
/* 0070 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff,  // ................
// COLOR 65 remaining=400 bytes pix=3199
/* 0000 */ 0x60, 0x21, 0xdf, 0xff,                                      // `!..
// FOM 387 remaining=396 bytes pix=3264
/* 0000 */ 0xf2, 0x83, 0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // ................
/* 0010 */ 0x11, 0x11, 0x11, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,  // ...DDDDDDDDDDDDD
/* 0020 */ 0x44, 0x44, 0x44, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // DDD.............
/* 0030 */ 0x11, 0x11, 0x11, 0x04,                                      // ....
// COPY 3 remaining=347 bytes pix=3651
/* 0000 */ 0x83, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,                             // .......
// FOM 122 remaining=338 bytes pix=3654
/* 0000 */ 0x40, 0x79, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,  // @y..............
/* 0010 */ 0x11, 0x01,                                            // ..
// COPY 99 remaining=321 bytes pix=3776
/* 0000 */ 0x80, 0x43, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .C..............
/* 0010 */ 0xdf, 0xff, 0xde, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 0030 */ 0xdf, 0xff, 0xde, 0xff, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0040 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0050 */ 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0060 */ 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0070 */ 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0080 */ 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,  // ................
/* 0090 */ 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff,  // ................
/* 00a0 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 00b0 */ 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00,  // ................
/* 00c0 */ 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0x00, 0x00,                          // ........
// FILL 9 remaining=122 bytes pix=3875
/* 0000 */ 0x09,                                               // .
// FOM 63 remaining=119 bytes pix=3884
/* 0000 */ 0x40, 0x3e, 0x11, 0x11, 0x41, 0x44, 0x04, 0x40, 0x00, 0x44,                    // @>..AD.@.D
// COPY 15 remaining=111 bytes pix=3947
/* 0000 */ 0x8f, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde,  // ................
/* 0010 */ 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,     // ...............
// FOM 24 remaining=79 bytes pix=3962
/* 0000 */ 0x43, 0x11, 0x11, 0x11,                                      // C...
// COPY 8 remaining=76 bytes pix=3986
/* 0000 */ 0x88, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xff, 0x00, 0x00, 0xdf,  // ................
/* 0010 */ 0xff,                                               // .
// FOM 57 remaining=57 bytes pix=3994
/* 0000 */ 0x40, 0x38, 0x01, 0x10, 0x11, 0x11, 0x51, 0x44, 0x44, 0x00,                    // @8....QDD.
// COPY 3 remaining=49 bytes pix=4051
/* 0000 */ 0x83, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff,                             // .......
// FILL 10 remaining=42 bytes pix=4054
/* 0000 */ 0x0a,                                               // .
// FILL 12 remaining=41 bytes pix=4064
// magic mix

/* 0000 */ 0x0c,                                               // .
// COPY 20 remaining=40 bytes pix=4075
/* 0000 */ 0x94, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf,  // ................
/* 0010 */ 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf,  // ................
/* 0020 */ 0xff, 0xde, 0xff, 0xdf, 0xff, 0xdf, 0xff, 0xdf, 0xff,                       // .........
    }
    ;

    Bitmap bloc64x64(BitsPerPixel{16}, BitsPerPixel{16}, nullptr, 64, 64, source64x64, sizeof(source64x64), true );
    drawable.draw(RDPMemBlt(0, Rect(100, 100, bloc64x64.cx(), bloc64x64.cy()), 0xCC, 0, 0, 0), scr, bloc64x64);

    Bitmap good16(BitsPerPixel{24}, bloc64x64);
    drawable.draw(RDPMemBlt(0, Rect(200, 200, good16.cx(), good16.cy()), 0xCC, 0, 0, 0), scr, good16);

    StaticOutStream<8192> stream;
    good16.compress(BitsPerPixel{24}, stream);
    Bitmap bogus(BitsPerPixel{24}, BitsPerPixel{24}, nullptr, 64, 64, stream.get_data(), stream.get_offset(), true);
    drawable.draw(RDPMemBlt(0, Rect(300, 100, bogus.cx(), bogus.cy()), 0xCC, 0, 0, 0), scr, bogus);

//     dump_png24("/tmp/test_bmp.png", drawable, true);

    dump_png24(trans, drawable, true);
    trans.next();
    const char * filename = trans.seqgen()->get(0);
    RED_CHECK_EQUAL(4094, ::filesize(filename));
    ::unlink(filename);
}

RED_AUTO_TEST_CASE(TestBogusBitmap2)
{
    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "bogus", ".png", groupid, ReportError{});
    Rect scr(0, 0, 800, 600);
    RDPDrawable drawable(800, 600);
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(scr, encode_color24()(GREEN)), scr, color_cxt);

    uint8_t source32x1[] =
//MemBlt Primary Drawing Order (0x0D)
//memblt(id=0 idx=15 x=448 y=335 cx=32 cy=1)
//Cache Bitmap V2 (Compressed) Secondary Drawing Order (0x05)
//update_read_cache_bitmap_v2_order
//update_read_cache_bitmap_v2_order id=0
//update_read_cache_bitmap_v2_order flags=8 CBR2_NO_BITMAP_COMPRESSION_HDR
//update_read_cache_bitmap_v2_order width=32 height=1
//update_read_cache_bitmap_v2_order Index=16
//rledecompress width=32 height=1 cbSrcBuffer=58

//-- /* BMP Cache compressed V2 */
//-- COPY1 5
//-- MIX 1
//-- COPY1 7
//-- MIX 1
//-- COPY1 2
//-- MIX_SET 4
//-- COPY1 9
//-- MIX_SET 3

           "\x85\xf8\xff\x2b\x6a\x6c\x12\x8d\x12\x79\x14"
           "\x21"
           "\x87\x15\xff\x2b\x42\x4b\x12\x4c\x12\x6c\x12\x4c\x12\x38\x14"
           "\x21"
           "\x82\x32\xfe\x6c\x12"
           "\xc4\x8d\x12"
           "\x89\x6d\x12\x4c\x12\x1f\x6e\xff\xff\x2a\xb4\x2b\x12\x6d\x12\xae\x12\xcf\x1a"
           "\xc3\xef\x1a"
    ;

    Bitmap bloc32x1(BitsPerPixel{16}, BitsPerPixel{16}, nullptr, 32, 1, source32x1, sizeof(source32x1)-1, true);
    drawable.draw(RDPMemBlt(0, Rect(100, 100, bloc32x1.cx(), bloc32x1.cy()), 0xCC, 0, 0, 0), scr, bloc32x1);

    dump_png24(trans, drawable, true);
    trans.next();
    const char * filename = trans.seqgen()->get(0);
    RED_CHECK_EQUAL(2913, ::filesize(filename));
    ::unlink(filename);
}

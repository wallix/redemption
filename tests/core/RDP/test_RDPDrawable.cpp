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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRDPDrawable
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "RDP/RDPDrawable.hpp"

#include "RDP/caches/glyphcache.hpp"

#include "log.hpp"
#include "check_sig.hpp"
#include "png.hpp"
#include "rect.hpp"
#include "stream.hpp"

inline bool check_sig(RDPDrawable & data, char * message, const char * shasig)
{
    return check_sig(data.data(), data.height(), data.rowsize(), message, shasig);
}

inline void dump_png(const char * prefix, const Drawable & data)
{
    char tmpname[128];
    sprintf(tmpname, "%sXXXXXX.png", prefix);
    int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
    FILE * f = fdopen(fd, "wb");
    ::dump_png24(f, data.data(), data.width(), data.height(), data.rowsize(), true);
    ::fclose(f);
}

inline void server_add_char( GlyphCache & gly_cache, uint8_t cacheId, uint16_t cacheIndex
                    , int16_t offset, int16_t baseline
                    , uint16_t width, uint16_t height, const uint8_t * data)
{
    FontChar fi(offset, baseline, width, height, 0);
    memcpy(fi.data.get(), data, fi.datasize());


LOG(LOG_INFO, "cacheId=%u cacheIndex=%u", cacheId, cacheIndex);
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

BOOST_AUTO_TEST_CASE(TestDrawGlyphIndex)
{
    uint16_t width = 1024;
    uint16_t height = 768;
    Rect screen_rect(0, 0, width, height);
    RDPDrawable gd(width, height, 24);

    gd.draw(RDPOpaqueRect(screen_rect, BLACK), screen_rect);

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
                                  0x000000,                     // BackColor
                                  0xc8d0d4,                     // ForeColor
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
        gd.draw(glyph_index, rect_clip, &gly_cache);

        RDPGlyphIndex glyph_index_1(7,                              // cacheId
                                    3,                              // flAccel
                                    0,                              // ulCharInc
                                    1,                              // fOpRedundant
                                    0x000000,                       // BackColor
                                    0xc8d0d4,                       // ForeColor
                                    rect_bk,                        // Bk
                                    rect_op,                        // Op
                                    RDPBrush(),                     // Brush
                                    22,                             // X
                                    757,                            // Y
                                    3,                              // cbData
                                    byte_ptr_cast("\xfe\x00\x00")   // rgbData
                                   );

        gd.draw(glyph_index, rect_clip, &gly_cache);
    }

    char message[1024];
    if (!check_sig(gd, message,
                   "\xd8\xf7\x6e\xf5\xd1\xe6\x4a\x05\x56\x0a"
                   "\x21\x42\xa4\x27\x73\x5a\xce\x67\xf6\xb3"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // uncomment to see result in png file
    //dump_png("test_glyph_000_", gd.impl());
}

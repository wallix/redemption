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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap class (mostly tests of compression/decompression)

*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConvertBitmap
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/unit_test.hpp>

#include"log.hpp"
#define LOGNULL

#include"bitmap.hpp"
#include"colors.hpp"
#include"ssl_calls.hpp"
#include"png.hpp"


// to see last result file, remove unlink
// and do something like:
// eog `ls -1tr /tmp/test_* | tail -n 1`
// (or any other variation you like)

void dump_png(const char * prefix, const Bitmap & bmp)
{
    char tmpname[128];
    sprintf(tmpname, "%sXXXXXX.png", prefix);
    int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
    FILE * f = fdopen(fd, "wb");
//    ::dump_png24(f, bmp.data_bitmap, bmp.bmp_size / (bmp.cy*nbbytes(bmp.original_bpp)), bmp.cy, bmp.bmp_size / bmp.cy);
    ::dump_png24(f, bmp.data(), 4, 3, 12);

    ::fclose(f);
}

BOOST_AUTO_TEST_CASE(TestConvertBitmap)
{
    BGRPalette palette332;
    init_palette332(palette332);

    const uint8_t source_bpp = 16;
    const uint16_t cx = 2;
    const uint16_t cy = 3;
    const uint8_t data[] = {
        0xFF, 0xFF,   0xFF, 0xFF,
        0xFF, 0xFF,   0xFF, 0xFF,
        0xFF, 0xFF,   0xFF, 0xFF,
    };

    Bitmap bmp16(source_bpp, &palette332, cx, cy, data, cx * nbbytes(source_bpp) * cy, false);
    BOOST_CHECK_EQUAL(24, bmp16.bmp_size);
    BOOST_CHECK_EQUAL(4, bmp16.line_size);
    BOOST_CHECK_EQUAL(cx, bmp16.cx);
    BOOST_CHECK_EQUAL(cy, bmp16.cy);
    BOOST_CHECK_EQUAL(16, bmp16.original_bpp);

    uint16_t target_bpp = 24;
    Bitmap bmp24(target_bpp, bmp16);
    BOOST_CHECK_EQUAL(36, bmp24.bmp_size);
    BOOST_CHECK_EQUAL(6, bmp24.line_size);
    BOOST_CHECK_EQUAL(cx, bmp24.cx);
    BOOST_CHECK_EQUAL(cy, bmp24.cy);
    BOOST_CHECK_EQUAL(24, bmp24.original_bpp);

    const uint8_t * outbuf = bmp24.data();

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[1]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[2]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[3]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[4]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[5]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

    // ---------------------------------

    BOOST_CHECK_EQUAL(0xFF, outbuf[12]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[13]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[14]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[15]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[16]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[17]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

    // ---------------------------------

    BOOST_CHECK_EQUAL(0xFF, outbuf[24]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[25]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[26]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[27]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[28]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[29]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);

}

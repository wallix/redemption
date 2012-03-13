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
#include <iostream>
#include <sstream>
#include <string>

#include "bitmap.hpp"
#include "colors.hpp"
#include <algorithm>
#include "ssl_calls.hpp"
#include "png.hpp"


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
    ::dump_png24(f, bmp.data_bitmap, 4, 3, 12);

    ::fclose(f);
}

BOOST_AUTO_TEST_CASE(TestConvertBitmap)
{
    BGRPalette palette332;
    init_palette332(palette332);

    uint8_t outbuf[65536];
    const uint8_t source_bpp = 16;
    const uint16_t cx = 2;
    const uint16_t cy = 3;
    const uint8_t data[] = {
        0xFF, 0xFF,   0xFF, 0xFF,
        0xFF, 0xFF,   0xFF, 0xFF,
        0xFF, 0xFF,   0xFF, 0xFF,
    };
    Bitmap bmp(source_bpp, &palette332, cx, cy, data, cx * nbbytes(source_bpp) * cy, false, false);
    const uint16_t target_bpp = 24;
    bmp.convert_data_bitmap(target_bpp, outbuf);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[1]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[2]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[3]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[4]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[5]);


    BOOST_CHECK_EQUAL(0xFF, outbuf[6]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[7]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[8]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[9]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[10]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[11]);


    BOOST_CHECK_EQUAL(0xFF, outbuf[12]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[13]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[14]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[15]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[16]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[17]);

    BOOST_CHECK_EQUAL(target_bpp, 24);
    BOOST_CHECK_EQUAL(bmp.cx, 2);
    BOOST_CHECK_EQUAL(bmp.cy, 3);

    Bitmap bmp2(target_bpp, &palette332, bmp.cx, bmp.cy, outbuf, bmp.cx * nbbytes(target_bpp) * bmp.cy, false, false);

    BOOST_CHECK_EQUAL(bmp2.original_bpp, 24);
    BOOST_CHECK_EQUAL(bmp2.cx, 2);
    BOOST_CHECK_EQUAL(bmp2.cy, 3);

    {
        const uint8_t * outbuf = bmp2.data_bitmap;
        BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[1]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[2]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[3]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[4]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[5]);

        BOOST_CHECK_EQUAL(0, outbuf[6]);
        BOOST_CHECK_EQUAL(0, outbuf[7]);
        BOOST_CHECK_EQUAL(0, outbuf[8]);

        BOOST_CHECK_EQUAL(0, outbuf[9]);
        BOOST_CHECK_EQUAL(0, outbuf[10]);
        BOOST_CHECK_EQUAL(0, outbuf[11]);


        BOOST_CHECK_EQUAL(0xFF, outbuf[12]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[13]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[14]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[15]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[16]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[17]);

        BOOST_CHECK_EQUAL(0, outbuf[18]);
        BOOST_CHECK_EQUAL(0, outbuf[19]);
        BOOST_CHECK_EQUAL(0, outbuf[20]);

        BOOST_CHECK_EQUAL(0, outbuf[21]);
        BOOST_CHECK_EQUAL(0, outbuf[22]);
        BOOST_CHECK_EQUAL(0, outbuf[23]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[24]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[25]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[26]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[27]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[28]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[29]);

        BOOST_CHECK_EQUAL(0, outbuf[30]);
        BOOST_CHECK_EQUAL(0, outbuf[31]);
        BOOST_CHECK_EQUAL(0, outbuf[32]);

        BOOST_CHECK_EQUAL(0, outbuf[33]);
        BOOST_CHECK_EQUAL(0, outbuf[34]);
        BOOST_CHECK_EQUAL(0, outbuf[35]);
    }

    BOOST_CHECK_EQUAL(8, bmp2.line_size);
    BOOST_CHECK_EQUAL(36, bmp2.bmp_size);
   BOOST_CHECK_EQUAL(2, bmp2.cx);
   BOOST_CHECK_EQUAL(3, bmp2.cy);
   BOOST_CHECK_EQUAL(24, bmp2.original_bpp);

    dump_png("/tmp/test_convert_000_", bmp2);
}


BOOST_AUTO_TEST_CASE(TestConvertBitmap24to16)
{
    BGRPalette palette332;
    init_palette332(palette332);

    uint8_t outbuf[65536];
    const uint8_t source_bpp = 24;
    const uint16_t cx = 2;
    const uint16_t cy = 3;
    const uint8_t data[] = {
        0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF,
    };
    Bitmap bmp(source_bpp, &palette332, cx, cy, data, cx * nbbytes(source_bpp) * cy, false, false);

    const uint16_t target_bpp = 16;
    bmp.convert_data_bitmap(target_bpp, outbuf);
    BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[1]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[2]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[3]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[4]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[5]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[6]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[7]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[8]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[9]);

    BOOST_CHECK_EQUAL(0xFF, outbuf[10]);
    BOOST_CHECK_EQUAL(0xFF, outbuf[11]);

    BOOST_CHECK_EQUAL(target_bpp, 16);
    BOOST_CHECK_EQUAL(bmp.cx, 2);
    BOOST_CHECK_EQUAL(bmp.cy, 3);

    Bitmap bmp2(target_bpp, &palette332, bmp.cx, bmp.cy, outbuf, bmp.cx * nbbytes(target_bpp) * bmp.cy, false, false);

    BOOST_CHECK_EQUAL(bmp2.original_bpp, 16);
    BOOST_CHECK_EQUAL(bmp2.cx, 2);
    BOOST_CHECK_EQUAL(bmp2.cy, 3);

    {
        const uint8_t * outbuf = bmp2.data_bitmap;
        BOOST_CHECK_EQUAL(0xFF, outbuf[0]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[1]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[2]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[3]);

        BOOST_CHECK_EQUAL(0, outbuf[4]);
        BOOST_CHECK_EQUAL(0, outbuf[5]);

        BOOST_CHECK_EQUAL(0, outbuf[6]);
        BOOST_CHECK_EQUAL(0, outbuf[7]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[8]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[9]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[10]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[11]);

        BOOST_CHECK_EQUAL(0, outbuf[12]);
        BOOST_CHECK_EQUAL(0, outbuf[13]);

        BOOST_CHECK_EQUAL(0, outbuf[14]);
        BOOST_CHECK_EQUAL(0, outbuf[15]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[16]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[17]);

        BOOST_CHECK_EQUAL(0xFF, outbuf[18]);
        BOOST_CHECK_EQUAL(0xFF, outbuf[19]);

        BOOST_CHECK_EQUAL(0, outbuf[20]);
        BOOST_CHECK_EQUAL(0, outbuf[21]);

        BOOST_CHECK_EQUAL(0, outbuf[22]);
        BOOST_CHECK_EQUAL(0, outbuf[23]);
    }

    BOOST_CHECK_EQUAL(4, bmp2.line_size);
    BOOST_CHECK_EQUAL(24, bmp2.bmp_size);
   BOOST_CHECK_EQUAL(2, bmp2.cx);
   BOOST_CHECK_EQUAL(3, bmp2.cy);
   BOOST_CHECK_EQUAL(16, bmp2.original_bpp);

}

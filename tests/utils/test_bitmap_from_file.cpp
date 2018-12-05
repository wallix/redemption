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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 021249, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE Test_Bitmap_With_PNG
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/bitmap_from_file.hpp"


RED_AUTO_TEST_CASE(TestBitmapOpenFiles)
{
    {
        const char * filename = FIXTURES_PATH "/jhlhjsdlkfhlkasd.png";
        Bitmap bmp = bitmap_from_file_impl(filename);
        RED_REQUIRE(!bmp.is_valid());
    }
    {
        const char * filename = FIXTURES_PATH "/replay.wrm";
        Bitmap bmp = bitmap_from_file_impl(filename);
        RED_REQUIRE(!bmp.is_valid());
    }
    {
        const char * filename = FIXTURES_PATH "/xrdp24b.jpg";
        Bitmap bmp = bitmap_from_file_impl(filename);
        RED_REQUIRE(!bmp.is_valid());
    }
    {
        const char * filename = FIXTURES_PATH "/xrdp24b.bmp";
        Bitmap bmp = bitmap_from_file_impl(filename);
        RED_REQUIRE(bmp.is_valid());
        RED_CHECK_EQUAL(bmp.cx(), 256);
        RED_CHECK_EQUAL(bmp.cy(), 150);
        RED_CHECK_EQUAL(bmp.bpp(), BitsPerPixel{24});
    }
    {
        const char * filename = FIXTURES_PATH "/xrdp24b.png";
        Bitmap bmp = bitmap_from_file_impl(filename);
        RED_REQUIRE(bmp.is_valid());
        RED_CHECK_EQUAL(bmp.cx(), 256);
        RED_CHECK_EQUAL(bmp.cy(), 150);
        RED_CHECK_EQUAL(bmp.bpp(), BitsPerPixel{24});
    }
}

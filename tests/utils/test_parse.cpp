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
   Copyright (C) Wallix 2014
   Author(s): Christophe Grosjean

   Unit test for char parse class

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/parse.hpp"

RED_AUTO_TEST_CASE(TestParse_0)
{
    uint8_t buffer[] = { 0x10, 0xFF, 0xFF, 0x11, 0x12, 0x13, 0x14 };
    Parse data(buffer);
    RED_CHECK_EQUAL(0x10, data.in_uint8());
    RED_CHECK_EQUAL(0xFF, data.in_uint8());
    RED_CHECK_EQUAL(-1, data.in_sint8());
}

RED_AUTO_TEST_CASE(TestParse_in_DEP)
{
    uint8_t buffer[] = {
        0x10,  // 16
        0x20,  // 32
        0x3F,  // 63
        0x7F,  // -1
        0x40,  // -64
        0x80, 0x00, // 0
        0x00, // 0
        0xC0, 0x00,
        0xDF, 0xFF,
        0xE0, 0x00,
        0xBF, 0xFF,
        0x00, // 0
        0xFF, 0xFF};
    Parse data(buffer);
    RED_CHECK_EQUAL(16, data.in_DEP());
    RED_CHECK_EQUAL(32, data.in_DEP());
    RED_CHECK_EQUAL(63, data.in_DEP());
    RED_CHECK_EQUAL(-1, data.in_DEP());
    RED_CHECK_EQUAL(-64, data.in_DEP());
    RED_CHECK_EQUAL(0, data.in_DEP());
    RED_CHECK_EQUAL(0, data.in_DEP());
    RED_CHECK_EQUAL(-16384, data.in_DEP());
    RED_CHECK_EQUAL(-8193, data.in_DEP());
    RED_CHECK_EQUAL(-8192, data.in_DEP());
    RED_CHECK_EQUAL(16383, data.in_DEP());
    RED_CHECK_EQUAL(0, data.in_DEP());
    RED_CHECK_EQUAL(-1, data.in_DEP());
}

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

#define RED_TEST_MODULE TestParse
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

RED_AUTO_TEST_CASE(TestParse_1)
{
    RED_CHECK_EQUAL(Parse(byte_ptr_cast("2281701377")).ulong_from_cstr(), 0x88000001);
}


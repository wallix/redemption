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

   Unit test to out_per_bstream object
   Using lib boost functions, some tests need to be added
*/

#define RED_TEST_MODULE TestOutPerStream
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/stream.hpp"
#include "core/RDP/out_per_bstream.hpp"


RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_large)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    out_per_integer(stream, 0x12345678);
    RED_CHECK_MEM_AC(stream.get_bytes(), "\x04\x12\x34\x56\x78");
}

RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_large2)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    out_per_integer(stream, 0x00345678);
    RED_CHECK_MEM_AC(stream.get_bytes(), "\x04\x00\x34\x56\x78");
}


RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_medium)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    out_per_integer(stream, 0x1234);
    RED_CHECK_MEM_AC(stream.get_bytes(), "\x02\x12\x34");
}

RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_small)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    out_per_integer(stream, 0x12);
    RED_CHECK_MEM_AC(stream.get_bytes(), "\x01\x12");
}

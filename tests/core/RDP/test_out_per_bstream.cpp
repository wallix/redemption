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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "utils/stream.hpp"
#include "core/RDP/out_per_bstream.hpp"

RED_AUTO_TEST_CASE(TestOutPerStream)
{
    // test we can create a Stream object
    uint8_t buf[3];
    OutPerStream out_per_stream(buf);

    RED_CHECK(out_per_stream.get_capacity() == 3);
    RED_CHECK(out_per_stream.get_data());
    RED_CHECK(out_per_stream.get_data() == out_per_stream.get_current());
}


RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_large)
{
    // test we can create a Stream object
    StaticOutPerStream<256> stream;
    stream.out_per_integer(0x12345678);
    RED_CHECK_EQUAL(5, stream.get_offset());
    RED_CHECK(0 == memcmp(stream.get_data(), "\x04\x12\x34\x56\x78", stream.get_offset()));
}

RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_large2)
{
    // test we can create a Stream object
    StaticOutPerStream<256> stream;
    stream.out_per_integer(0x00345678);
    RED_CHECK_EQUAL(5, stream.get_offset());
    RED_CHECK(0 == memcmp(stream.get_data(), "\x04\x00\x34\x56\x78", stream.get_offset()));
}


RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_medium)
{
    // test we can create a Stream object
    StaticOutPerStream<256> stream;
    stream.out_per_integer(0x1234);
    RED_CHECK_EQUAL(3, stream.get_offset());
    RED_CHECK(0 == memcmp(stream.get_data(), "\x02\x12\x34", stream.get_offset()));
}

RED_AUTO_TEST_CASE(TestOutPerStream_per_integer_small)
{
    // test we can create a Stream object
    StaticOutPerStream<256> stream;
    stream.out_per_integer(0x12);
    RED_CHECK_EQUAL(2, stream.get_offset());
    RED_CHECK(0 == memcmp(stream.get_data(), "\x01\x12", stream.get_offset()));
}

#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc.hpp"


/* TODO For now we are testing out_per primitive through gcc object,
 * true unit test should do that at a lower level
 * for every out_per primitive defined */
RED_AUTO_TEST_CASE(Test_gcc_write_conference_create_request)
{
    const char gcc_user_data[] =
    "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x05\x00\x04\x01\xCA\x03\xAA"
    "\x09\x04\x00\x00\xCE\x0E\x00\x00\x45\x00\x4c\x00\x54\x00\x4f\x00"
    "\x4e\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x32\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
    "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\xCA\x01\x00\x00\x00\x00\x00\x18\x00\x07\x00"
    "\x01\x00\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00"
    "\x38\x00\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00"
    "\x37\x00\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\xC0\x0C\x00\x0D\x00\x00\x00"
    "\x00\x00\x00\x00\x02\xC0\x0C\x00\x1B\x00\x00\x00\x00\x00\x00\x00"
    "\x03\xC0\x2C\x00\x03\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00"
    "\x00\x00\x80\x80\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00\xA0\xC0"
    "\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0"
    ;

    const char gcc_conference_create_request_expected[] =
    // conference_create_request_header
    "\x00\x05\x00\x14\x7C\x00\x01"
    "\x81\x2A\x00\x08\x00\x10\x00\x01\xC0"
    "\x00\x44\x75\x63\x61"
    "\x81\x1c" // User data length
    // header
    "\x01\xc0\xd8\x00\x04\x00\x08\x00\x00\x05\x00\x04\x01\xCA\x03\xAA"
    "\x09\x04\x00\x00\xCE\x0E\x00\x00\x45\x00\x4c\x00\x54\x00\x4f\x00"
    "\x4e\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x32\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
    "\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x01\xCA\x01\x00\x00\x00\x00\x00\x18\x00\x07\x00"
    "\x01\x00\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00"
    "\x38\x00\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00"
    "\x37\x00\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    "\x00\x00\x00\x00\x00\x00\x00\x00\x04\xC0\x0C\x00\x0D\x00\x00\x00"
    "\x00\x00\x00\x00\x02\xC0\x0C\x00\x1B\x00\x00\x00\x00\x00\x00\x00"
    "\x03\xC0\x2C\x00\x03\x00\x00\x00\x72\x64\x70\x64\x72\x00\x00\x00"
    "\x00\x00\x80\x80\x63\x6c\x69\x70\x72\x64\x72\x00\x00\x00\xA0\xC0"
    "\x72\x64\x70\x73\x6e\x64\x00\x00\x00\x00\x00\xc0";


    constexpr std::size_t sz = sizeof(gcc_conference_create_request_expected)-1; // -1 to ignore final 0
    TestTransport t(
        "", 0,
        gcc_conference_create_request_expected, sz - (sizeof(gcc_user_data) - 1));

    StaticOutPerStream<65536> gcc_header;
    GCC::Create_Request_Send(gcc_header, sizeof(gcc_user_data)-1);
    t.send(gcc_header.get_data(), gcc_header.get_offset());

    InStream in_stream(gcc_conference_create_request_expected, sz);
    RED_CHECK_NO_THROW(GCC::Create_Request_Recv{in_stream});

//    RED_CHECK(t.get_status());
}


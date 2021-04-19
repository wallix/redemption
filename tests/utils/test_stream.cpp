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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to stream object
   Using lib boost functions, some tests need to be added
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"


RED_AUTO_TEST_CASE(TestOutStream_Create)
{
    // test we can create a Stream object
    uint8_t buf[3];
    OutStream out_per_stream(buf);

    RED_CHECK(out_per_stream.get_capacity() == 3u);
    RED_CHECK(out_per_stream.get_data());
    RED_CHECK(out_per_stream.get_data() == out_per_stream.get_current());
}

RED_AUTO_TEST_CASE(TestStream_uint8)
{
    // test reading of 8 bits data from Stream signed or unsigned is working

    InStream s("\1\xFE\xFD\4\5"_av);
    // 5 characters are availables
    RED_CHECK(s.in_check_rem(5));
    // but not 6...
    RED_CHECK(!s.in_check_rem(6));

    // we read one unsigned char, it is 1
    RED_CHECK_EQUAL(1, s.in_uint8());
    // now we have only 4 char left
    RED_CHECK(s.in_check_rem(4));
    RED_CHECK(!s.in_check_rem(5));

    // we read one unsigned char, it is 0xFE (254)
    RED_CHECK_EQUAL(254, s.in_uint8());
    // now we have only 3 char left
    RED_CHECK(s.in_check_rem(3));
    RED_CHECK(!s.in_check_rem(4));

    // we read one signed char, it is 0xFD (-3)
    RED_CHECK_EQUAL(-3, s.in_sint8());
    // now we have only 3 char left
    RED_CHECK(s.in_check_rem(2));
    RED_CHECK(!s.in_check_rem(3));

    // we peek a byte (as many time as we want it will always return the same byte)
    RED_CHECK_EQUAL(4, s.peek_uint8());
    RED_CHECK_EQUAL(4, s.peek_uint8());
    RED_CHECK_EQUAL(4, s.peek_uint8());

    RED_CHECK_EQUAL(4, s.in_sint8());
    RED_CHECK_EQUAL(5, s.in_sint8());
    // now the buffer is empty
    RED_CHECK(!s.in_remain());
}

RED_AUTO_TEST_CASE(TestStream_uint16)
{
    // test reading of 16 bits data from Stream signed or unsigned
    // with any endiannessis working. The way functions are written
    // target endianness is care of automagically.
    // (the + operator does the job).

    InStream s("\1\0\xFE\xFF\xFF\xFD\xFF\xFC\xFB\xFF\0\1"_av);

    uint8_t const * oldp = s.get_current();
    // 12 characters are availables
    RED_CHECK(s.in_check_rem(12));
    // but not 13...
    RED_CHECK(!s.in_check_rem(13));

    RED_CHECK_EQUAL(s.in_uint16_le(), 1);
    RED_CHECK_EQUAL(oldp+2, s.get_current());

    RED_CHECK_EQUAL(s.in_sint16_le(), -2); // FFFE == -2
    RED_CHECK_EQUAL(s.in_sint16_be(), -3); // FFFD == -3

    RED_CHECK_EQUAL(s.in_uint16_be(), 0xFFFC);
    RED_CHECK_EQUAL(s.in_uint16_le(), 0xFFFB);

    RED_CHECK_EQUAL(s.in_sint16_be(), 1);

    // empty is OK
    RED_CHECK(!s.in_remain());
}

RED_AUTO_TEST_CASE(TestStream_uint32)
{
    // test reading of 32 bits unsigned data from Stream
    // with any endiannessis working. The way functions are written
    // target endianness is taken care of automagically.
    // (the + operator does the job).

    InStream s("\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF"_av);

    uint8_t const * oldp = s.get_current();

    RED_CHECK_EQUAL(s.in_uint32_le(), 1u);
    RED_CHECK_EQUAL(oldp+4, s.get_current());

    RED_CHECK_EQUAL(s.in_uint32_be(), 0xFFFFFFFEu);
    RED_CHECK_EQUAL(s.in_uint32_be(), 1u);
    RED_CHECK_EQUAL(s.in_uint32_le(), 0xFFFFFFFCu);

    // empty is OK
    RED_CHECK(!s.in_remain());
}

RED_AUTO_TEST_CASE(TestStream_uint64)
{
    // test reading of 64 bits unsigned data from Stream
    // with any endiannessis working. The way functions are written
    // target endianness is taken care of automagically.
    // (the + operator does the job).

    auto data = "\1\0\0\0\0\0\0\0\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\0\0\0\0\0\0\0\1\xFC\xFF\xFF\xFF\xFF\xFF\xFF\xFF"_av;
    InStream s(data);

    uint8_t const * oldp = s.get_current();

    RED_CHECK_EQUAL(s.in_uint64_le(), 1u);
    RED_CHECK_EQUAL(oldp+8, s.get_current());

    RED_CHECK_EQUAL(s.in_uint64_be(), 0xFFFFFFFFFFFFFFFEull);
    RED_CHECK_EQUAL(s.in_uint64_be(), 1ull);
    RED_CHECK_EQUAL(s.in_uint64_le(), 0xFFFFFFFFFFFFFFFCull);

    // empty is OK
    RED_CHECK(!s.in_remain());

    uint8_t out_data[32];
    OutStream out(out_data);
    out.out_uint64_be(1LL);
    out.out_uint64_le(0xFFEECCLL);
    s = InStream(out_data);
    RED_CHECK_EQUAL(s.in_uint64_be(), 1ull);
    RED_CHECK_EQUAL(s.in_uint64_le(), 0xFFEECCull);
    RED_CHECK(s.get_current() == out_data + 16);
}

RED_AUTO_TEST_CASE(TestStream_in_skip_bytes)
{
    // test use of skip_bytes that skip a given number of bytes

    InStream s("\0\1\2\3\4\5\6\7\x8\x9\xA\xB\xC\xD"_av);

    uint8_t const * oldp = s.get_current();

    auto v = s.in_skip_bytes(10);
    RED_CHECK_EQUAL(v.size(), 10);
    RED_CHECK_EQUAL(oldp+10, s.get_current());
    RED_CHECK_EQUAL(s.in_uint8(), 0x0A);

    s.in_skip_bytes(3);

    // empty is OK
    RED_CHECK(!s.in_remain());
}

RED_AUTO_TEST_CASE(TestStream_out_Stream)
{
    // use output primitives to write to a Stream

    StaticOutStream<100> s;

    uint8_t const * oldp = s.get_current();

    s.out_uint8(10);
    s.out_uint16_le(1 + (2 << 8));
    s.out_uint16_be(3 + (4 << 8));
    s.out_uint32_le((1 << 24) + (2 << 16) + (3 << 8) + 4);
    s.out_uint32_be((5 << 24) + (6 << 16) + (7 << 8) + 8);

    RED_CHECK_EQUAL(oldp+13, s.get_current());
    RED_CHECK_EQUAL((s.get_data())[0], 10);

    RED_CHECK_EQUAL((s.get_data())[1], 1);
    RED_CHECK_EQUAL((s.get_data())[2], 2);

    RED_CHECK_EQUAL((s.get_data())[3], 4);
    RED_CHECK_EQUAL((s.get_data())[4], 3);

    RED_CHECK_EQUAL((s.get_data())[5], 4);
    RED_CHECK_EQUAL((s.get_data())[6], 3);
    RED_CHECK_EQUAL((s.get_data())[7], 2);
    RED_CHECK_EQUAL((s.get_data())[8], 1);

    RED_CHECK_EQUAL((s.get_data())[9], 5);
    RED_CHECK_EQUAL((s.get_data())[10], 6);
    RED_CHECK_EQUAL((s.get_data())[11], 7);
    RED_CHECK_EQUAL((s.get_data())[12], 8);

    RED_CHECK("\xA\1\2\4\3\4\3\2\1\5\6\7\x8"_av == s.get_produced_bytes());

    // underflow because end is not yet moved at p
    RED_CHECK(s.tailroom());
}

RED_AUTO_TEST_CASE(TestStream_Stream_Compatibility)
{
    StaticOutStream<512> stream;

    RED_CHECK_EQUAL(512u,   stream.get_capacity());

    stream.out_copy_bytes("0123456789", 10);

    RED_CHECK_EQUAL(502u,   stream.tailroom());
    RED_CHECK( stream.has_room(502));
    RED_CHECK(not stream.has_room(503));
}

RED_AUTO_TEST_CASE(TestStream_2BUE)
{
    StaticOutStream<256> stream;

    stream.out_2BUE(0x1A1B);

    InStream in_stream(stream.get_produced_bytes());

    RED_CHECK_EQUAL(0x1A1Bu, in_stream.in_2BUE());
}

RED_AUTO_TEST_CASE(TestStream_4BUE)
{
    StaticOutStream<256> stream;

    stream.out_4BUE(0x001A1B1C);

    InStream in_stream(stream.get_produced_bytes());

    RED_CHECK_EQUAL(0x001A1B1Cu, in_stream.in_4BUE());
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
        0xFF, 0xFF,
        0xFF, 0xBF
    };
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
    RED_CHECK_EQUAL(-65, data.in_DEP());
}

RED_AUTO_TEST_CASE(TestStream_out_DEP)
{
    StaticOutStream<256> stream;

    uint8_t expected[] = {
        0x10,  // 16
        0x20,  // 32
        0x3F,  // 63
        0x7F,  // -1
        0xFF, 0xBF,  // -65
        0x40,  // -64
        0x41,  // -63
        0x61,  // -31
        0x00, // 0
        0xC0, 0x00,
        0xDF, 0xFF,
        0xE0, 0x00,
        0xBF, 0xFF,
        0xBF, 0xFF,
    };

    stream.out_DEP(16); // 0x10
    stream.out_DEP(32); // 0x20
    stream.out_DEP(63); // 0x3F
    stream.out_DEP(-1); // 0x7F
    stream.out_DEP(-65); // 0xFF, 0xBF
    stream.out_DEP(-64); // 0x40
    stream.out_DEP(-63); // 0x41
    stream.out_DEP(-31); // 0x61
    stream.out_DEP(0); // 0
    stream.out_DEP(-16384); // 0xC0, 0x00
    stream.out_DEP(-8193); // 0xDF, 0xFF
    stream.out_DEP(-8192); // 0xE0, 0x00
    stream.out_DEP(16383); // 0xBF, 0xFF 0x1011 0x1111
    // Actually the example below is invalid, the function only apply
    // in range -16384 .. 16383 (15 bits)
    // -16385 gave the same output as +16383
    stream.out_DEP(-16385); // 0xBF, 0xFF

    RED_CHECK(stream.get_produced_bytes() == make_array_view(expected));
    InStream in_stream(stream.get_produced_bytes());

    RED_CHECK(16 == in_stream.in_DEP()); // 0x10
    RED_CHECK(32 == in_stream.in_DEP()); // 0x20
    RED_CHECK(63 == in_stream.in_DEP()); // 0x3F
    RED_CHECK(-1 == in_stream.in_DEP()); // 0x7F
    RED_CHECK(-65 == in_stream.in_DEP()); // 0xFF, 0xBF
    RED_CHECK(-64 == in_stream.in_DEP()); // 0x40
    RED_CHECK(-63 == in_stream.in_DEP()); // 0x41
    RED_CHECK(-31 == in_stream.in_DEP()); // 0x61
    RED_CHECK(0 == in_stream.in_DEP()); // 0
    RED_CHECK(-16384 == in_stream.in_DEP()); // 0xC0, 0x00
    RED_CHECK(-8193 == in_stream.in_DEP()); // 0xDF, 0xFF
    RED_CHECK(-8192 == in_stream.in_DEP()); // 0xE0, 0x00
    RED_CHECK(16383 == in_stream.in_DEP()); // 0xBF, 0xFF 0x1011 0x1111
    RED_CHECK(16383 == in_stream.in_DEP()); // 0xBF, 0xFF 0x1011 0x1111
}

RED_AUTO_TEST_CASE(TestStream_in_out_DEP)
{
    for (int i = -16384; i < 16383; i++) {
        StaticOutStream<256> stream;

        stream.out_DEP(i);

        InStream in_stream(stream.get_produced_bytes());

        RED_CHECK(i == in_stream.in_DEP());
    }
}

RED_AUTO_TEST_CASE(TestStream_sint32)
{
    {
        auto tmp = "\xFE\xFD\xFC\xFF"_av;
        InStream ss_min_val(tmp);
        RED_CHECK_EQUAL(int32_t(0xFFFCFDFE), ss_min_val.in_sint32_le());
        StaticOutStream<4> fs_min_val;
        fs_min_val.out_sint32_le(0xFFFCFDFE);
        RED_CHECK(fs_min_val.get_produced_bytes() == tmp);
    }

    {
        auto tmp = "\xFC\xFD\xFE\x7F"_av;
        InStream ss_max_val(tmp);
        RED_CHECK_EQUAL(0x7FFEFDFC, ss_max_val.in_sint32_le());
        StaticOutStream<4> fs_max_val;
        fs_max_val.out_sint32_le(0x7FFEFDFC);
        RED_CHECK(fs_max_val.get_produced_bytes() == tmp);
    }

}

RED_AUTO_TEST_CASE(TestOutSInt64Le)
{
    {
        const int64_t int64_test = -5000000000LLU;

        StaticOutStream<8> stream;
        stream.out_sint64_le(int64_test);

        RED_CHECK(stream.get_produced_bytes() == "\x00\x0e\xfa\xd5\xfe\xff\xff\xff"_av);
    }

    {
        StaticOutStream<8> stream;
        stream.out_sint64_le(0);

        RED_CHECK(stream.get_produced_bytes() == "\0\0\0\0\0\0\0\0"_av);
    }

    {
        const int64_t int64_test = 10000000000LLU;

        StaticOutStream<8> stream;
        stream.out_sint64_le(int64_test);

        RED_CHECK(stream.get_produced_bytes() == "\x00\xe4\x0b\x54\x02\x00\x00\x00"_av);
    }
}

RED_AUTO_TEST_CASE(TestInSInt64Le)
{
    {
        InStream stream("\x00\x44\x5f\x9a\xfe\xff\xff\xff"_av);
        RED_CHECK_EQUAL(stream.in_sint64_le(), int64_t(-6000000000LLU));
    }

    {
        InStream stream("\x00\x00\x00\x00\x00\x00\x00\x00"_av);
        RED_CHECK_EQUAL(stream.in_sint64_le(), 0);
    }

    {
        InStream stream("\x00\x78\x41\xcb\x02\x00\x00\x00"_av);
        RED_CHECK_EQUAL(stream.in_sint64_le(), int64_t(12000000000LLU));
    }
}

RED_AUTO_TEST_CASE(TestStreamAt)
{
    StaticOutStream<12> stream;
    stream.out_copy_bytes("abcde"_av);

    RED_CHECK(stream.get_produced_bytes() == "abcde"_av);
    stream.stream_at(1).out_uint8('x');
    RED_CHECK(stream.get_produced_bytes() == "axcde"_av);
}


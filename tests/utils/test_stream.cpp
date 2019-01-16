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

#define RED_TEST_MODULE TestStream
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"


RED_AUTO_TEST_CASE(TestOutStream_Create)
{
    // test we can create a Stream object
    uint8_t buf[3];
    OutStream out_per_stream(buf);

    RED_CHECK_EQ(out_per_stream.get_capacity(), 3u);
    RED_CHECK(out_per_stream.get_data());
    RED_CHECK_EQ(out_per_stream.get_data(), out_per_stream.get_current());
}

RED_AUTO_TEST_CASE(TestStream_uint8)
{
    // test reading of 8 bits data from Stream signed or unsigned is working

    char buf[] = "\1\xFE\xFD\4\5";
    InStream s(buf, sizeof(buf)-1);
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

    char buf[] = "\1\0\xFE\xFF\xFF\xFD\xFF\xFC\xFB\xFF\0\1";
    InStream s(buf, sizeof(buf)-1);

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

    char data[] = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    InStream s(data, sizeof(data)-1);

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

    char data[] = "\1\0\0\0\0\0\0\0\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\0\0\0\0\0\0\0\1\xFC\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    InStream s(data, sizeof(data)-1);

    uint8_t const * oldp = s.get_current();

    RED_CHECK_EQUAL(s.in_uint64_le(), 1u);
    RED_CHECK_EQUAL(oldp+8, s.get_current());

    RED_CHECK_EQUAL(s.in_uint64_be(), 0xFFFFFFFFFFFFFFFEull);
    RED_CHECK_EQUAL(s.in_uint64_be(), 1ull);
    RED_CHECK_EQUAL(s.in_uint64_le(), 0xFFFFFFFFFFFFFFFCull);

    // empty is OK
    RED_CHECK(!s.in_remain());

    OutStream out(data);
    out.out_uint64_be(1LL);
    out.out_uint64_le(0xFFEECCLL);
    s.rewind();
    RED_CHECK_EQUAL(s.in_uint64_be(), 1ull);
    RED_CHECK_EQUAL(s.in_uint64_le(), 0xFFEECCull);
    RED_CHECK(s.get_current() == oldp + 16);
}

RED_AUTO_TEST_CASE(TestStream_in_uint8p)
{
    // test in buffer access to some block of data
    // in_uint8p returns a pointer to current beginning of buffer
    // and advance by some given amount of characters.

    char data[] = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    InStream s(data, sizeof(data)-1);

    uint8_t const * oldp = s.get_current();

    RED_CHECK_EQUAL(s.in_uint8p(8), oldp);
    RED_CHECK_EQUAL(oldp+8, s.get_current());

    RED_CHECK_EQUAL(s.in_uint8p(8), oldp+8);
    RED_CHECK_EQUAL(oldp+16, s.get_current());

    // empty is OK
    RED_CHECK(!s.in_remain());
}

RED_AUTO_TEST_CASE(TestStream_in_skip_bytes)
{
    // test use of skip_bytes that skip a given number of bytes

    char data[] = "\0\1\2\3\4\5\6\7\x8\x9\xA\xB\xC\xD";
    InStream s(data, sizeof(data)-1);

    uint8_t const * oldp = s.get_current();

    s.in_skip_bytes(10);
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

    RED_CHECK(!memcmp("\xA\1\2\4\3\4\3\2\1\5\6\7\x8", s.get_data(), 13));

    // underflow because end is not yet moved at p
    RED_CHECK(s.tailroom());
}

RED_AUTO_TEST_CASE(TestStream_in_unistr)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    uint8_t data[] = { 'r', 0, 'e', 0, 's', 0, 'u', 0, 'l', 0, 't', 0, 0, 0 };
    stream.out_copy_bytes(data, sizeof(data));
    stream.out_uint32_le(-1); // just to put a padding after usefull data

    InStream in_stream(stream.get_bytes());
    uint8_t result[256];
    in_stream.in_uni_to_ascii_str(result, sizeof(data), sizeof(result));
    RED_CHECK_EQUAL(14u, in_stream.get_offset());
    RED_CHECK_EQUAL('r', result[0]);
    RED_CHECK_EQUAL('e', result[1]);
    RED_CHECK_EQUAL('s', result[2]);
    RED_CHECK_EQUAL('u', result[3]);
    RED_CHECK_EQUAL('l', result[4]);
    RED_CHECK_EQUAL('t', result[5]);
    RED_CHECK_EQUAL(0, result[6]);
}

RED_AUTO_TEST_CASE(TestStream_in_unistr_2)
{
    // test we can create a Stream object
    StaticOutStream<256> stream;
    uint8_t data[] = { 'r', 0, 0xE9, 0, 's', 0, 'u', 0, 'l', 0, 't', 0, 0, 0 };
    stream.out_copy_bytes(data, sizeof(data));
    stream.out_uint32_le(-1); // just to put a padding after usefull data

    InStream in_stream(stream.get_bytes());
    uint8_t result[256];
    in_stream.in_uni_to_ascii_str(result, sizeof(data), sizeof(result));
    RED_CHECK_EQUAL(14u, in_stream.get_offset());
    RED_CHECK_EQUAL('r', result[0]);
    RED_CHECK_EQUAL(0xC3, result[1]);
    RED_CHECK_EQUAL(0xA9, result[2]);
    RED_CHECK_EQUAL('s', result[3]);
    RED_CHECK_EQUAL('u', result[4]);
    RED_CHECK_EQUAL('l', result[5]);
    RED_CHECK_EQUAL('t', result[6]);
    RED_CHECK_EQUAL(0, result[7]);
}

RED_AUTO_TEST_CASE(TestStream_Stream_Compatibility)
{
    StaticOutStream<512> stream;

    RED_CHECK_EQUAL(512u,   stream.get_capacity());

    stream.out_copy_bytes("0123456789", 10);

    RED_CHECK_EQUAL(502u,   stream.tailroom());
    RED_CHECK_EQUAL(true,  stream.has_room(502));
    RED_CHECK_EQUAL(false, stream.has_room(503));
}

RED_AUTO_TEST_CASE(TestStream_2BUE)
{
    StaticOutStream<256> stream;

    stream.out_2BUE(0x1A1B);

    //hexdump_d(stream.get_data(), stream.size());

    InStream in_stream(stream.get_bytes());

    RED_CHECK_EQUAL(0x1A1Bu, in_stream.in_2BUE());
}

RED_AUTO_TEST_CASE(TestStream_4BUE)
{
    StaticOutStream<256> stream;

    stream.out_4BUE(0x001A1B1C);

    //hexdump_d(stream.get_data(), stream.size());

    InStream in_stream(stream.get_bytes());

    RED_CHECK_EQUAL(0x001A1B1Cu, in_stream.in_4BUE());
}

RED_AUTO_TEST_CASE(TestStream_sint32)
{
    {
        const uint8_t tmp[4] = {0xFE, 0xFD, 0xFC, 0xFF};
        InStream ss_min_val(tmp);
        RED_CHECK_EQUAL(static_cast<int32_t>(0xFFFCFDFE), ss_min_val.in_sint32_le());
        uint8_t vartmp[4] = {};
        OutStream fs_min_val(vartmp);
        fs_min_val.out_sint32_le(0xFFFCFDFE);
        RED_CHECK_MEM_AA(vartmp, tmp);
    }

    {
        const uint8_t tmp[4] = {0xFC, 0xFD, 0xFE, 0x7F};
        InStream ss_max_val(tmp);
        RED_CHECK_EQUAL(0x7FFEFDFC, ss_max_val.in_sint32_le());
        uint8_t vartmp[4] = {};
        OutStream fs_max_val(vartmp);
        fs_max_val.out_sint32_le(0x7FFEFDFC);
        RED_CHECK_MEM_AA(tmp, vartmp);
    }

}

RED_AUTO_TEST_CASE(TestOutSInt64Le)
{
    {
        const int64_t int64_test = -5000000000LLU;

        const unsigned char data_test[] = { 0x00, 0x0e, 0xfa, 0xd5, 0xfe, 0xff, 0xff, 0xff };

        StaticOutStream<8> stream;
        stream.out_sint64_le(int64_test);

        RED_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }

    {
        const int64_t int64_test = 0LLU;

        const unsigned char data_test[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        StaticOutStream<8> stream;
        stream.out_sint64_le(int64_test);

        RED_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }

    {
        const int64_t int64_test = 10000000000LLU;

        const unsigned char data_test[] = { 0x00, 0xe4, 0x0b, 0x54, 0x02, 0x00, 0x00, 0x00 };

        StaticOutStream<8> stream;
        stream.out_sint64_le(int64_test);

        RED_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }
}

RED_AUTO_TEST_CASE(TestInSInt64Le)
{
    {
        int64_t i64_original = -6000000000LLU;

        uint8_t data_test[] = { 0x00, 0x44, 0x5f, 0x9a, 0xfe, 0xff, 0xff, 0xff };

        InStream stream(data_test, sizeof(data_test));

        RED_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }

    {
        int64_t i64_original = 0LLU;

        uint8_t data_test[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        InStream stream(data_test, sizeof(data_test));

        RED_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }

    {
        int64_t i64_original = 12000000000LLU;

        uint8_t data_test[] = { 0x00, 0x78, 0x41, 0xcb, 0x02, 0x00, 0x00, 0x00 };

        InStream stream(data_test, sizeof(data_test));

        RED_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }
}

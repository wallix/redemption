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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestStream
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "stream.hpp"
#include "test_transport.hpp"

BOOST_AUTO_TEST_CASE(TestStreamInitWithSize)
{
    // test we can create a Stream object
    Stream * s = new BStream(1000);
    BOOST_CHECK(s);

    BOOST_CHECK(s->get_capacity() == 1000);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream)
{
    // test we can create a Stream object
    Stream * s = new BStream();
    BOOST_CHECK(s);

    BOOST_CHECK(s->get_capacity() == AUTOSIZE);

    s->init(8192);
    BOOST_CHECK(s->get_capacity() == 8192);
    BOOST_CHECK(s->get_data());
    BOOST_CHECK(s->get_data() == s->p);
    BOOST_CHECK(s->get_data() == s->end);

    // and we can destroy Stream.
    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_uint8)
{
    // test reading of 8 bits data from Stream signed or unsigned is working

    Stream * s = new BStream(10);
    memcpy(s->get_data(), reinterpret_cast<const uint8_t*>("\1\xFE\xFD\4\5"), 5);
    s->end += 5;
    // 5 characters are availables
    BOOST_CHECK(s->in_check_rem(5));
    // but not 6...
    BOOST_CHECK(!s->in_check_rem(6));

    // we read one unsigned char, it is 1
    BOOST_CHECK_EQUAL(1, s->in_uint8());
    // now we have only 4 char left
    BOOST_CHECK(s->in_check_rem(4));
    BOOST_CHECK(!s->in_check_rem(5));

    // we read one unsigned char, it is 0xFE (254)
    BOOST_CHECK_EQUAL(254, s->in_uint8());
    // now we have only 3 char left
    BOOST_CHECK(s->in_check_rem(3));
    BOOST_CHECK(!s->in_check_rem(4));

    // we read one signed char, it is 0xFD (-3)
    BOOST_CHECK_EQUAL(-3, s->in_sint8());
    // now we have only 3 char left
    BOOST_CHECK(s->in_check_rem(2));
    BOOST_CHECK(!s->in_check_rem(3));

    // we peek a byte (as many time as we want it will always return the same byte)
    BOOST_CHECK_EQUAL(4, s->peek_uint8());
    BOOST_CHECK_EQUAL(4, s->peek_uint8());
    BOOST_CHECK_EQUAL(4, s->peek_uint8());

    BOOST_CHECK_EQUAL(4, s->in_sint8());
    BOOST_CHECK_EQUAL(5, s->in_sint8());
    // now the buffer is empty
    BOOST_CHECK(s->check_end());

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_uint16)
{
    // test reading of 16 bits data from Stream signed or unsigned
    // with any endiannessis working. The way functions are written
    // target endianness is care of automagically.
    // (the + operator does the job).

    Stream * s = new BStream(100);
    const char * data = "\1\0\xFE\xFF\xFF\xFD\xFF\xFC\xFB\xFF\0\1";
    memcpy(s->get_data(), reinterpret_cast<const uint8_t*>(data), 12);
    s->end += 12;

    uint8_t * oldp = s->p;
    // 12 characters are availables
    BOOST_CHECK(s->in_check_rem(12));
    // but not 13...
    BOOST_CHECK(!s->in_check_rem(13));

    BOOST_CHECK_EQUAL(s->in_uint16_le(), 1);
    BOOST_CHECK_EQUAL(oldp+2, s->p);

    BOOST_CHECK_EQUAL(s->in_sint16_le(), -2); // FFFE == -2
    BOOST_CHECK_EQUAL(s->in_sint16_be(), -3); // FFFD == -3

    BOOST_CHECK_EQUAL(s->in_uint16_be(), 0xFFFC);
    BOOST_CHECK_EQUAL(s->in_uint16_le(), 0xFFFB);

    BOOST_CHECK_EQUAL(s->in_sint16_be(), 1);

    // empty is OK
    BOOST_CHECK(s->p == s->end);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_uint32)
{
    // test reading of 32 bits unsigned data from Stream
    // with any endiannessis working. The way functions are written
    // target endianness is taken care of automagically.
    // (the + operator does the job).

    Stream * s = new BStream(100);
    const char * data = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    memcpy(s->get_data(), reinterpret_cast<const uint8_t*>(data), 16);
    s->end += 16;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint32_le(), 1);
    BOOST_CHECK_EQUAL(oldp+4, s->p);

    BOOST_CHECK_EQUAL(s->in_uint32_be(), 0xFFFFFFFE);
    BOOST_CHECK_EQUAL(s->in_uint32_be(), 1);
    BOOST_CHECK_EQUAL(s->in_uint32_le(), 0xFFFFFFFC);

    // empty is OK
    BOOST_CHECK(s->p == s->end);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_uint64)
{
    // test reading of 64 bits unsigned data from Stream
    // with any endiannessis working. The way functions are written
    // target endianness is taken care of automagically.
    // (the + operator does the job).

    Stream * s = new BStream(100);
    const char * data = "\1\0\0\0\0\0\0\0\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE\0\0\0\0\0\0\0\1\xFC\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
    memcpy(s->get_data(), reinterpret_cast<const uint8_t*>(data), 32);
    s->end += 32;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint64_le(), 1LL);
    BOOST_CHECK_EQUAL(oldp+8, s->p);

    BOOST_CHECK_EQUAL(s->in_uint64_be(), 0xFFFFFFFFFFFFFFFELL);
    BOOST_CHECK_EQUAL(s->in_uint64_be(), 1LL);
    BOOST_CHECK_EQUAL(s->in_uint64_le(), 0xFFFFFFFFFFFFFFFCLL);

    // empty is OK
    BOOST_CHECK(s->p == s->end);

    s->p = oldp;
    s->out_uint64_be(1LL);
    s->out_uint64_le(0xFFEECCLL);
    s->p = oldp;
    BOOST_CHECK_EQUAL(s->in_uint64_be(), 1LL);
    BOOST_CHECK_EQUAL(s->in_uint64_le(), 0xFFEECCLL);
    BOOST_CHECK(s->p == oldp + 16);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_in_uint8p)
{
    // test in buffer access to some block of data
    // in_uint8p returns a pointer to current beginning of buffer
    // and advance by some given amount of characters.

    Stream * s = new BStream(100);
    const char * data = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    memcpy(s->get_data(), reinterpret_cast<const uint8_t*>(data), 16);
    s->end += 16;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint8p(8), oldp);
    BOOST_CHECK_EQUAL(oldp+8, s->p);

    BOOST_CHECK_EQUAL(s->in_uint8p(8), oldp+8);
    BOOST_CHECK_EQUAL(oldp+16, s->p);

    // empty is OK
    BOOST_CHECK(s->p == s->end);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_in_skip_bytes)
{
    // test use of skip_bytes that skip a given number of bytes

    BStream s(100);
    const char * data = "\0\1\2\3\4\5\6\7\x8\x9\xA\xB\xC\xD";
    memcpy(s.get_data(), reinterpret_cast<const uint8_t*>(data), 14);
    s.end += 14;

    uint8_t * oldp = s.p;

    s.in_skip_bytes(10);
    BOOST_CHECK_EQUAL(oldp+10, s.p);
    BOOST_CHECK_EQUAL(s.in_uint8(), 0x0A);

    s.in_skip_bytes(3);

    // empty is OK
    BOOST_CHECK(s.p == s.end);
}

BOOST_AUTO_TEST_CASE(TestStream_out_Stream)
{
    // use output primitives to write to a Stream

    Stream * s = new BStream(100);

    uint8_t * oldp = s->p;

    s->out_uint8(10);
    s->out_uint16_le(1 + (2 << 8));
    s->out_uint16_be(3 + (4 << 8));
    s->out_uint32_le((1 << 24) + (2 << 16) + (3 << 8) + 4);
    s->out_uint32_be((5 << 24) + (6 << 16) + (7 << 8) + 8);

    BOOST_CHECK_EQUAL(oldp+13, s->p);
    BOOST_CHECK_EQUAL((s->get_data())[0], 10);

    BOOST_CHECK_EQUAL((s->get_data())[1], 1);
    BOOST_CHECK_EQUAL((s->get_data())[2], 2);

    BOOST_CHECK_EQUAL((s->get_data())[3], 4);
    BOOST_CHECK_EQUAL((s->get_data())[4], 3);

    BOOST_CHECK_EQUAL((s->get_data())[5], 4);
    BOOST_CHECK_EQUAL((s->get_data())[6], 3);
    BOOST_CHECK_EQUAL((s->get_data())[7], 2);
    BOOST_CHECK_EQUAL((s->get_data())[8], 1);

    BOOST_CHECK_EQUAL((s->get_data())[9], 5);
    BOOST_CHECK_EQUAL((s->get_data())[10], 6);
    BOOST_CHECK_EQUAL((s->get_data())[11], 7);
    BOOST_CHECK_EQUAL((s->get_data())[12], 8);

    BOOST_CHECK(!memcmp("\xA\1\2\4\3\4\3\2\1\5\6\7\x8", s->get_data(), 13));

    // underflow because end is not yet moved at p
    BOOST_CHECK(s->p > s->end);

    // move end to where p is now, hence between data and end we have
    // the data we wish to export.
    s->mark_end();

    // no more underflow
    BOOST_CHECK(s->p == s->end);

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_in_unistr)
{
    // test we can create a Stream object
    BStream stream(1024);
    uint8_t data[] = { 'r', 0, 'e', 0, 's', 0, 'u', 0, 'l', 0, 't', 0, 0, 0 };
    stream.out_copy_bytes(data, sizeof(data));
    stream.out_uint32_le(-1); // just to put a padding after usefull data
    stream.mark_end();

    stream.p = stream.get_data();
    uint8_t result[256];
    stream.in_uni_to_ascii_str(result, sizeof(data), sizeof(result));
    BOOST_CHECK_EQUAL(14, stream.get_offset());
    BOOST_CHECK_EQUAL('r', result[0]);
    BOOST_CHECK_EQUAL('e', result[1]);
    BOOST_CHECK_EQUAL('s', result[2]);
    BOOST_CHECK_EQUAL('u', result[3]);
    BOOST_CHECK_EQUAL('l', result[4]);
    BOOST_CHECK_EQUAL('t', result[5]);
    BOOST_CHECK_EQUAL(0, result[6]);
}

BOOST_AUTO_TEST_CASE(TestStream_in_unistr_2)
{
    // test we can create a Stream object
    BStream stream(1024);
    uint8_t data[] = { 'r', 0, 0xE9, 0, 's', 0, 'u', 0, 'l', 0, 't', 0, 0, 0 };
    stream.out_copy_bytes(data, sizeof(data));
    stream.out_uint32_le(-1); // just to put a padding after usefull data
    stream.mark_end();

    stream.p = stream.get_data();
    uint8_t result[256];
    stream.in_uni_to_ascii_str(result, sizeof(data), sizeof(result));
    BOOST_CHECK_EQUAL(14, stream.get_offset());
    BOOST_CHECK_EQUAL('r', result[0]);
    BOOST_CHECK_EQUAL(0xC3, result[1]);
    BOOST_CHECK_EQUAL(0xA9, result[2]);
    BOOST_CHECK_EQUAL('s', result[3]);
    BOOST_CHECK_EQUAL('u', result[4]);
    BOOST_CHECK_EQUAL('l', result[5]);
    BOOST_CHECK_EQUAL('t', result[6]);
    BOOST_CHECK_EQUAL(0, result[7]);
}

BOOST_AUTO_TEST_CASE(TestStream_BStream_Compatibility)
{
    BStream stream(512);

    BOOST_CHECK_EQUAL(512,   stream.get_capacity());

    stream.out_copy_bytes("0123456789", 10);

    BOOST_CHECK_EQUAL(502,   stream.tailroom());
    BOOST_CHECK_EQUAL(true,  stream.has_room(502));
    BOOST_CHECK_EQUAL(false, stream.has_room(503));
}

BOOST_AUTO_TEST_CASE(TestStream_HStream)
{
    HStream stream(512, 1024);

    BOOST_CHECK_EQUAL(512, stream.get_capacity());
    BOOST_CHECK_EQUAL(512, stream.tailroom());
    BOOST_CHECK_EQUAL(0, stream.size());

    const uint8_t data1[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    stream.out_copy_bytes(data1, 10);
    stream.mark_end();

    BOOST_CHECK_EQUAL(512, stream.get_capacity());
    BOOST_CHECK_EQUAL(502, stream.tailroom());
    BOOST_CHECK_EQUAL(10, stream.size());


    const uint8_t header3[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g'};
    stream.copy_to_head(header3, 7);

    BOOST_CHECK_EQUAL(519, stream.get_capacity());
    BOOST_CHECK_EQUAL(502, stream.tailroom());
    BOOST_CHECK_EQUAL(17, stream.size());

    const uint8_t header2[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G'};
    stream.copy_to_head(header2, 7);

    BOOST_CHECK_EQUAL(526, stream.get_capacity());
    BOOST_CHECK_EQUAL(502, stream.tailroom());
    BOOST_CHECK_EQUAL(24, stream.size());

    const uint8_t header1[] = { '#', '*', '!', '+', '-', '_'};
    stream.copy_to_head(header1, 6);

    BOOST_CHECK_EQUAL(532, stream.get_capacity());
    BOOST_CHECK_EQUAL(502, stream.tailroom());
    BOOST_CHECK_EQUAL(30, stream.size());

    BOOST_CHECK_EQUAL(true,  stream.has_room(480));
    BOOST_CHECK_EQUAL(true, stream.has_room(502));

    stream.mark_end();


    const uint8_t expected[] = { '#', '*', '!', '+', '-', '_',
                               'A', 'B', 'C', 'D', 'E', 'F', 'G',
                               'a', 'b', 'c', 'd', 'e', 'f', 'g',
                               '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    CheckTransport ct(reinterpret_cast<const char *>(expected), sizeof(expected));

    BOOST_CHECK_EQUAL(30, stream.size());

    ct.send(stream);

    BOOST_CHECK_EQUAL(true, ct.get_status());


    stream.init(2048);

    BOOST_CHECK_EQUAL(2048,  stream.tailroom());
    BOOST_CHECK_EQUAL(true,  stream.has_room(2048));
    BOOST_CHECK_EQUAL(false, stream.has_room(2049));
}

BOOST_AUTO_TEST_CASE(TestStream_2BUE)
{
    BStream stream(256);

    stream.out_2BUE(0x1A1B);
    stream.mark_end();

    //hexdump_d(stream.get_data(), stream.size());

    stream.rewind();

    BOOST_CHECK_EQUAL(0x1A1B, stream.in_2BUE());
}

BOOST_AUTO_TEST_CASE(TestStream_4BUE)
{
    BStream stream(256);

    stream.out_4BUE(0x001A1B1C);
    stream.mark_end();

    //hexdump_d(stream.get_data(), stream.size());

    stream.rewind();

    BOOST_CHECK_EQUAL(0x001A1B1C, stream.in_4BUE());
}

BOOST_AUTO_TEST_CASE(TestStream_sint32)
{
    const int32_t const_min_val = -2147483648;

    StaticStream ss_min_val(reinterpret_cast<const uint8_t *>(&const_min_val), sizeof(const_min_val));

    BOOST_CHECK_EQUAL(const_min_val, ss_min_val.in_sint32_le());


    int32_t min_val = 0;

    FixedSizeStream fs_min_val(reinterpret_cast<uint8_t *>(&min_val), sizeof(min_val));

    fs_min_val.out_sint32_le(const_min_val);

    BOOST_CHECK_EQUAL(const_min_val, min_val);



    const int32_t const_max_val = 2147483647;

    StaticStream ss_max_val(reinterpret_cast<const uint8_t *>(&const_max_val), sizeof(const_max_val));

    BOOST_CHECK_EQUAL(const_max_val, ss_max_val.in_sint32_le());


    int32_t max_val = 0;

    FixedSizeStream fs_max_val(reinterpret_cast<uint8_t *>(&max_val), sizeof(max_val));

    fs_max_val.out_sint32_le(const_max_val);

    BOOST_CHECK_EQUAL(const_max_val, max_val);




    const int32_t const_null_val = 0;

    StaticStream ss_null_val(reinterpret_cast<const uint8_t *>(&const_null_val), sizeof(const_null_val));

    BOOST_CHECK_EQUAL(const_null_val, ss_null_val.in_sint32_le());


    int32_t null_val = 0;

    FixedSizeStream fs_null_val(reinterpret_cast<uint8_t *>(&null_val), sizeof(null_val));

    fs_null_val.out_sint32_le(const_null_val);

    BOOST_CHECK_EQUAL(const_null_val, null_val);



    const int32_t const_negative_val = -32768;

    StaticStream ss_negative_val(reinterpret_cast<const uint8_t *>(&const_negative_val), sizeof(const_negative_val));

    BOOST_CHECK_EQUAL(const_negative_val, ss_negative_val.in_sint32_le());


    int32_t negative_val = 0;

    FixedSizeStream fs_negative_val(reinterpret_cast<uint8_t *>(&negative_val), sizeof(negative_val));

    fs_negative_val.out_sint32_le(const_negative_val);

    BOOST_CHECK_EQUAL(const_negative_val, negative_val);



    const int32_t const_positive_val = 32767;

    StaticStream ss_positive_val(reinterpret_cast<const uint8_t *>(&const_positive_val), sizeof(const_positive_val));

    BOOST_CHECK_EQUAL(const_positive_val, ss_positive_val.in_sint32_le());


    int32_t positive_val = 0;

    FixedSizeStream fs_positive_val(reinterpret_cast<uint8_t *>(&positive_val), sizeof(positive_val));

    fs_positive_val.out_sint32_le(const_positive_val);

    BOOST_CHECK_EQUAL(const_positive_val, positive_val);
}

BOOST_AUTO_TEST_CASE(TestOutSInt64Le)
{
    {
        const int64_t int64_test = -5000000000LLU;

        const unsigned char data_test[] = { 0x00, 0x0e, 0xfa, 0xd5, 0xfe, 0xff, 0xff, 0xff };

        BStream stream(8);
        stream.out_sint64_le(int64_test);

        BOOST_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }

    {
        const int64_t int64_test = 0LLU;

        const unsigned char data_test[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        BStream stream(8);
        stream.out_sint64_le(int64_test);

        BOOST_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }

    {
        const int64_t int64_test = 10000000000LLU;

        const unsigned char data_test[] = { 0x00, 0xe4, 0x0b, 0x54, 0x02, 0x00, 0x00, 0x00 };

        BStream stream(8);
        stream.out_sint64_le(int64_test);

        BOOST_CHECK(!memcmp(stream.get_data(), data_test, sizeof(data_test)));
    }
}

BOOST_AUTO_TEST_CASE(TestInSInt64Le)
{
    {
        int64_t i64_original = -6000000000LLU;

        uint8_t data_test[] = { 0x00, 0x44, 0x5f, 0x9a, 0xfe, 0xff, 0xff, 0xff };

        FixedSizeStream stream(data_test, sizeof(data_test));

        BOOST_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }

    {
        int64_t i64_original = 0LLU;

        uint8_t data_test[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        FixedSizeStream stream(data_test, sizeof(data_test));

        BOOST_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }

    {
        int64_t i64_original = 12000000000LLU;

        uint8_t data_test[] = { 0x00, 0x78, 0x41, 0xcb, 0x02, 0x00, 0x00, 0x00 };

        FixedSizeStream stream(data_test, sizeof(data_test));

        BOOST_CHECK_EQUAL(stream.in_sint64_le(), i64_original);
    }
}

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

//#define LOGNULL
#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "testtransport.hpp"

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
    memcpy(s->get_data(), (uint8_t*)"\1\xFE\xFD\4\5", 5);
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
    memcpy(s->get_data(), (uint8_t*)data, 12);
    s->end += 12;

    uint8_t * oldp = s->p;
    // 12 characters are availables
    BOOST_CHECK(s->in_check_rem(12));
    // but not 13...
    BOOST_CHECK(!s->in_check_rem(13));

    BOOST_CHECK_EQUAL(s->in_uint16_le(), 1);
    BOOST_CHECK_EQUAL((unsigned long)oldp+2, ((unsigned long)s->p));

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
    memcpy(s->get_data(), (uint8_t*)data, 16);
    s->end += 16;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint32_le(), 1);
    BOOST_CHECK_EQUAL((unsigned long)oldp+4, ((unsigned long)s->p));

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
    memcpy(s->get_data(), (uint8_t*)data, 32);
    s->end += 32;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint64_le(), 1LL);
    BOOST_CHECK_EQUAL((unsigned long)oldp+8, ((unsigned long)s->p));

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
    memcpy(s->get_data(), (uint8_t*)data, 16);
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
    memcpy(s.get_data(), (uint8_t*)data, 14);
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

    BOOST_CHECK_EQUAL(502,   stream.room());
    BOOST_CHECK_EQUAL(true,  stream.has_room(502));
    BOOST_CHECK_EQUAL(false, stream.has_room(503));
}

BOOST_AUTO_TEST_CASE(TestStream_HStream)
{
    HStream stream(512, 1024);

    BOOST_CHECK_EQUAL(512, stream.get_capacity());
    BOOST_CHECK_EQUAL(512, stream.room());

    stream.out_copy_bytes("0123456789", 10);
    stream.copy_to_head("abcdefg", 7);
    stream.copy_to_head("ABCDEFG", 7);
    stream.copy_to_head("#*?!+-_", 7);

    BOOST_CHECK_EQUAL(481,   stream.room());
    BOOST_CHECK_EQUAL(true,  stream.has_room(481));
    BOOST_CHECK_EQUAL(false, stream.has_room(482));

    stream.mark_end();

    const char * data = "#*?!+-_ABCDEFGabcdefg0123456789";

    CheckTransport ct(data, strlen(data));

    BOOST_CHECK_EQUAL(31, stream.size());

    ct.send(stream);

    BOOST_CHECK_EQUAL(true, ct.get_status());


    stream.init(2048);

    BOOST_CHECK_EQUAL(2048,  stream.room());
    BOOST_CHECK_EQUAL(true,  stream.has_room(2048));
    BOOST_CHECK_EQUAL(false, stream.has_room(2049));
}

BOOST_AUTO_TEST_CASE(TestStream_RedemptionBStream)
{
    redemption::BStream stream(1024, 32768);

    BOOST_CHECK_EQUAL(0,     stream.get_size());
    BOOST_CHECK_EQUAL(32768, stream.get_body_capacity());
    BOOST_CHECK_EQUAL(true,  stream.has_body_room(32768));
    BOOST_CHECK_EQUAL(false, stream.has_body_room(32769));

    stream.out_uint8(127);

    BOOST_CHECK_EQUAL(true,  stream.has_body_room(32767));
    BOOST_CHECK_EQUAL(false, stream.has_body_room(32768));
    BOOST_CHECK_EQUAL(1,     stream.in_remain());
    BOOST_CHECK_EQUAL(true,  stream.in_check_remain(1));
    BOOST_CHECK_EQUAL(false, stream.in_check_remain(2));

    BOOST_CHECK_EQUAL(127,   stream.in_uint8());
    BOOST_CHECK_EQUAL(0,     stream.in_remain());
    BOOST_CHECK_EQUAL(true,  stream.in_check_remain(0));
    BOOST_CHECK_EQUAL(false, stream.in_check_remain(1));
}

BOOST_AUTO_TEST_CASE(TestStream_RedemptionSubStream)
{
    redemption::BStream stream(1024, 32768);

    redemption::SubStream sub_stream(stream, 0, 16);

    redemption::InStream  & in_stream  = sub_stream;
    redemption::OutStream & out_stream = sub_stream;

    BOOST_CHECK_EQUAL(0,     sub_stream.get_size());
    BOOST_CHECK_EQUAL(16,    sub_stream.get_body_capacity());
    BOOST_CHECK_EQUAL(true,  sub_stream.has_body_room(16));
    BOOST_CHECK_EQUAL(false, sub_stream.has_body_room(17));

    sub_stream.out_uint8(127);

    BOOST_CHECK_EQUAL(true,  sub_stream.has_body_room(15));
    BOOST_CHECK_EQUAL(false, sub_stream.has_body_room(16));
    BOOST_CHECK_EQUAL(1,     sub_stream.in_remain());
    BOOST_CHECK_EQUAL(true,  sub_stream.in_check_remain(1));
    BOOST_CHECK_EQUAL(false, sub_stream.in_check_remain(2));

    BOOST_CHECK_EQUAL(127,   sub_stream.in_uint8());
    BOOST_CHECK_EQUAL(0,     sub_stream.in_remain());
    BOOST_CHECK_EQUAL(true,  sub_stream.in_check_remain(0));
    BOOST_CHECK_EQUAL(false, sub_stream.in_check_remain(1));

    stream.inc_write_ptr(16);

    BOOST_CHECK_EQUAL(16,    stream.get_size());
    BOOST_CHECK_EQUAL(true,  stream.in_check_remain(16));
}

BOOST_AUTO_TEST_CASE(TestStream_RedemptionInOutStream)
{
    redemption::BStream stream(65536);

    redemption::SubStream sub_stream(stream, 0, 16);

    redemption::OutStream & out_stream = sub_stream;

    BOOST_CHECK_EQUAL(0,     out_stream.get_head_capacity());
    BOOST_CHECK_EQUAL(16,    out_stream.get_body_capacity());

    out_stream.out_uint8(127);

    BOOST_CHECK_EQUAL(true,  sub_stream.has_body_room(15));
    BOOST_CHECK_EQUAL(false, sub_stream.has_body_room(16));
    BOOST_CHECK_EQUAL(1,     sub_stream.in_remain());
    BOOST_CHECK_EQUAL(true,  sub_stream.in_check_remain(1));
    BOOST_CHECK_EQUAL(false, sub_stream.in_check_remain(2));

    stream.inc_write_ptr(16);

    BOOST_CHECK_EQUAL(16,    stream.get_size());
    BOOST_CHECK_EQUAL(true,  stream.in_check_remain(16));

    BOOST_CHECK_EQUAL(127,   stream.in_uint8());
    BOOST_CHECK_EQUAL(15,    stream.in_remain());
    BOOST_CHECK_EQUAL(true,  stream.in_check_remain(15));
    BOOST_CHECK_EQUAL(false, stream.in_check_remain(16));

    redemption::InStream & in_stream = sub_stream;

    BOOST_CHECK_EQUAL(1,     in_stream.in_remain());
}

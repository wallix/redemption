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

   Unit test to stream object
   Using lib boost functions, some tests need to be added
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestStream
#include <boost/test/auto_unit_test.hpp>
#include "stream.hpp"

BOOST_AUTO_TEST_CASE(TestStreamInitWithSize)
{
    // test we can create a Stream object
    Stream * s = new Stream(1000);
    BOOST_CHECK(s);

    BOOST_CHECK(s->capacity == 1000);

    delete s;
}


BOOST_AUTO_TEST_CASE(TestStream)
{
    // test we can create a Stream object
    Stream * s = new Stream();
    BOOST_CHECK(s);

    BOOST_CHECK(s->capacity == AUTOSIZE);

    s->init(8192);
    BOOST_CHECK(s->capacity == 8192);
    BOOST_CHECK(s->data);
    BOOST_CHECK(s->data == s->p);
    BOOST_CHECK(s->data == s->end);

    // buffer is empty but no underflow check ok
    BOOST_CHECK(s->check());

    // add 100 characters to buffer
    s->end += 100;
    // now there is character in buffer
    BOOST_CHECK(s->check());

    // we are not at the end of buffer
    BOOST_CHECK(!s->check_end());

    // there is more than 50 characters
    BOOST_CHECK(s->check_rem(50));

    // there is 100 characters
    BOOST_CHECK(s->check_rem(100));

    // there is not 101 characters
    BOOST_CHECK(!s->check_rem(101));

    s->p += 100;
    // Now we are at the end of buffer
    BOOST_CHECK(s->check_end());

    // still no buffer underflow
    BOOST_CHECK(s->check());

    s->p += 1;
    // now we have a buffer underflow
    BOOST_CHECK(!s->check());

    // and we can destroy Stream.
    delete s;

}

BOOST_AUTO_TEST_CASE(TestStream_uint8)
{
    // test reading of 8 bits data from Stream signed or unsigned is working

    Stream * s = new Stream(10);
    memcpy(s->data, (uint8_t*)"\1\xFE\xFD\4\5", 5);
    s->end += 5;
    // 5 characters are availables
    BOOST_CHECK(s->check_rem(5));
    // but not 6...
    BOOST_CHECK(!s->check_rem(6));

    // we read one unsigned char, it is 1
    BOOST_CHECK_EQUAL(1, s->in_uint8());
    // now we have only 4 char left
    BOOST_CHECK(s->check_rem(4));
    BOOST_CHECK(!s->check_rem(5));

    // we read one unsigned char, it is 0xFE (254)
    BOOST_CHECK_EQUAL(254, s->in_uint8());
    // now we have only 3 char left
    BOOST_CHECK(s->check_rem(3));
    BOOST_CHECK(!s->check_rem(4));

    // we read one signed char, it is 0xFD (-3)
    BOOST_CHECK_EQUAL(-3, s->in_sint8());
    // now we have only 3 char left
    BOOST_CHECK(s->check_rem(2));
    BOOST_CHECK(!s->check_rem(3));

    BOOST_CHECK_EQUAL(4, s->in_sint8());
    BOOST_CHECK_EQUAL(5, s->in_sint8());
    // now the buffer is empty
    BOOST_CHECK(s->check_end());

    // empty is OK
    BOOST_CHECK(s->check());
    // we read past the end of buffer
    s->in_sint8();
    // underflow is not OK
    BOOST_CHECK(!s->check());

    delete s;

}

BOOST_AUTO_TEST_CASE(TestStream_uint16)
{
    // test reading of 16 bits data from Stream signed or unsigned
    // with any endiannessis working. The way functions are written
    // target endianness is care of automagically.
    // (the + operator does the job).

    Stream * s = new Stream(100);
    const char * data = "\1\0\xFE\xFF\xFF\xFD\xFF\xFC\xFB\xFF\0\1";
    memcpy(s->data, (uint8_t*)data, 12);
    s->end += 12;

    uint8_t * oldp = s->p;
    // 12 characters are availables
    BOOST_CHECK(s->check_rem(12));
    // but not 13...
    BOOST_CHECK(!s->check_rem(13));

    BOOST_CHECK_EQUAL(s->in_uint16_le(), 1);
    BOOST_CHECK_EQUAL((unsigned long)oldp+2, ((unsigned long)s->p));

    BOOST_CHECK_EQUAL(s->in_sint16_le(), -2); // FFFE == -2
    BOOST_CHECK_EQUAL(s->in_sint16_be(), -3); // FFFD == -3

    BOOST_CHECK_EQUAL(s->in_uint16_be(), 0xFFFC);
    BOOST_CHECK_EQUAL(s->in_uint16_le(), 0xFFFB);

    BOOST_CHECK_EQUAL(s->in_sint16_be(), 1);

    // empty is OK
    BOOST_CHECK(s->check());
    // we read past the end of buffer
    s->in_sint16_le();
    // underflow is not OK
    BOOST_CHECK(!s->check());

    delete s;
}


BOOST_AUTO_TEST_CASE(TestStream_uint32)
{
    // test reading of 32 bits unsigned data from Stream
    // with any endiannessis working. The way functions are written
    // target endianness is taken care of automagically.
    // (the + operator does the job).

    Stream * s = new Stream(100);
    const char * data = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    memcpy(s->data, (uint8_t*)data, 16);
    s->end += 16;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint32_le(), 1);
    BOOST_CHECK_EQUAL((unsigned long)oldp+4, ((unsigned long)s->p));

    BOOST_CHECK_EQUAL(s->in_uint32_be(), 0xFFFFFFFE);
    BOOST_CHECK_EQUAL(s->in_uint32_be(), 1);
    BOOST_CHECK_EQUAL(s->in_uint32_le(), 0xFFFFFFFC);

    // empty is OK
    BOOST_CHECK(s->check());
    // we read past the end of buffer
    s->in_uint32_le();
    // underflow is not OK
    BOOST_CHECK(!s->check());

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_in_uint8p)
{
    // test in buffer access to some block of data
    // in_uint8p returns a pointer to current beginning of buffer
    // and advance by some given amount of characters.

    Stream * s = new Stream(100);
    const char * data = "\1\0\0\0\xFF\xFF\xFF\xFE\0\0\0\1\xFC\xFF\xFF\xFF";
    memcpy(s->data, (uint8_t*)data, 16);
    s->end += 16;

    uint8_t * oldp = s->p;

    BOOST_CHECK_EQUAL(s->in_uint8p(8), oldp);
    BOOST_CHECK_EQUAL(oldp+8, s->p);

    BOOST_CHECK_EQUAL(s->in_uint8p(8), oldp+8);
    BOOST_CHECK_EQUAL(oldp+16, s->p);

    // empty is OK
    BOOST_CHECK(s->check());
    // we read past the end of buffer
    s->in_uint8p(2);
    // underflow is not OK
    BOOST_CHECK(!s->check());

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_skip_uint8)
{
    // test use of skip_uint8 that skip a given number of bytes

    Stream * s = new Stream(100);
    const char * data = "\0\1\2\3\4\5\6\7\x8\x9\xA\xB\xC\xD";
    memcpy(s->data, (uint8_t*)data, 14);
    s->end += 14;

    uint8_t * oldp = s->p;

    s->skip_uint8(10);
    BOOST_CHECK_EQUAL(oldp+10, s->p);
    BOOST_CHECK_EQUAL(s->in_uint8(), 0x0A);

    s->skip_uint8(3);

    // empty is OK
    BOOST_CHECK(s->check());
    // we read past the end of buffer
    s->skip_uint8(1);
    // underflow is not OK
    BOOST_CHECK(!s->check());

    delete s;
}

BOOST_AUTO_TEST_CASE(TestStream_out_Stream)
{
    // use output primitives to write to a Stream

    Stream * s = new Stream(100);

    uint8_t * oldp = s->p;

    s->out_uint8(10);
    s->out_uint16_le(1 + (2 << 8));
    s->out_uint16_be(3 + (4 << 8));
    s->out_uint32_le((1 << 24) + (2 << 16) + (3 << 8) + 4);
    s->out_uint32_be((5 << 24) + (6 << 16) + (7 << 8) + 8);

    BOOST_CHECK_EQUAL(oldp+13, s->p);
    BOOST_CHECK_EQUAL(s->data[0], 10);

    BOOST_CHECK_EQUAL(s->data[1], 1);
    BOOST_CHECK_EQUAL(s->data[2], 2);

    BOOST_CHECK_EQUAL(s->data[3], 4);
    BOOST_CHECK_EQUAL(s->data[4], 3);

    BOOST_CHECK_EQUAL(s->data[5], 4);
    BOOST_CHECK_EQUAL(s->data[6], 3);
    BOOST_CHECK_EQUAL(s->data[7], 2);
    BOOST_CHECK_EQUAL(s->data[8], 1);

    BOOST_CHECK_EQUAL(s->data[9], 5);
    BOOST_CHECK_EQUAL(s->data[10], 6);
    BOOST_CHECK_EQUAL(s->data[11], 7);
    BOOST_CHECK_EQUAL(s->data[12], 8);

    BOOST_CHECK(!memcmp("\xA\1\2\4\3\4\3\2\1\5\6\7\x8", s->data, 13));

    // underflow because end is not yet moved at p
    BOOST_CHECK(!s->check());

    // move end to where p is now, hence between data and end we have
    // the data we wish to export.
    s->mark_end();

    // no more underflow
    BOOST_CHECK(s->check());

    delete s;
}

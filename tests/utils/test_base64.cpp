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
   Author(s): Christophe Grosjean, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Tests on Base64 Converter

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBase64
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "base64.hpp"
#include "stream.hpp"
#include "genrandom.hpp"
#include <stdio.h>

BOOST_AUTO_TEST_CASE(TestEncoding)
{

    Base64 b64converter;

    unsigned char input[] = "Hi!";
    unsigned char output[64];

    size_t res = b64converter.encode(output, 64, input, sizeof(input)-1);


    LOG(LOG_INFO, "input: %s, output:%s\n", input, output);
    BOOST_CHECK_EQUAL(res, 4);
    BOOST_CHECK_EQUAL(std::string("SGkh"), std::string((const char*)output));

}

BOOST_AUTO_TEST_CASE(TestDecoding)
{
    Base64 b64converter;

    unsigned char input[] = "dGVzdCBzb21lIHRleHQ=";
    unsigned char output[64];

    size_t res = b64converter.decode(output, 64, input, sizeof(input)-1);
    BOOST_CHECK_EQUAL(res, 14);
    BOOST_CHECK_EQUAL(std::string("test some text"),
                      std::string((const char*)output));


    BStream streamin(256);

    streamin.out_uint8(0xFF);
    streamin.out_uint8(0xFD);
    streamin.out_uint8(0X15);
    streamin.out_uint8(0x12);
    streamin.out_uint8(0x00);
    streamin.out_uint8(0x91);
    streamin.out_uint8(0x44);
    streamin.out_uint8(0x4A);

    unsigned char output2[512];
    res = b64converter.encode(output2, sizeof(output2), streamin.get_data(), streamin.get_offset());
    BOOST_CHECK_EQUAL(res, 12);
    int i = 0;
    BOOST_CHECK_EQUAL(output2[i++], '/');
    BOOST_CHECK_EQUAL(output2[i++], '/');


    unsigned char output3[256];
    res = b64converter.decode(output3, sizeof(output3), output2, res);

    BOOST_CHECK_EQUAL(res, 8);
    i = 0;
    unsigned char * p = streamin.get_data();
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);
    BOOST_CHECK_EQUAL(output3[i++], *p++);

}


BOOST_AUTO_TEST_CASE(TestRandom)
{
    Base64 b64converter;

    LCGRandom rand(0xA3F45);
    BStream streamin(256);

    uint32_t randsize = rand.rand32() % 64;
    unsigned int i = 0;
    for (i = 0; i < randsize; i++)
        streamin.out_uint32_le(rand.rand32());

    size_t res;
    unsigned char output[512];
    res = b64converter.encode(output, sizeof(output), streamin.get_data(), streamin.get_offset());

    unsigned char output2[256];
    res = b64converter.decode(output2, sizeof(output2), output, res);
    BOOST_CHECK_EQUAL(res, randsize * 4);
    unsigned char * p = streamin.get_data();
    for (i = 0; i < randsize;)
        BOOST_CHECK_EQUAL(output2[i++], *p++);
}

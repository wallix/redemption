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

    unsigned char input[] = "Hi!Hi!";
    unsigned char output[64];

    size_t res = b64converter.encode(output, sizeof(output), input, sizeof(input)-1);

    BOOST_CHECK_EQUAL(res, 8);
    output[res] = 0;
    BOOST_CHECK_EQUAL(std::string("SGkhSGkh"), std::string((const char *)output));


    unsigned char input2[] = "test un texte";
    unsigned char output2[64];
    size_t res2 = b64converter.encode(output2, sizeof(output2), input2, sizeof(input2)-1);

    BOOST_CHECK_EQUAL(res2, 20);
    output2[res2] = 0;
    BOOST_CHECK_EQUAL(std::string("dGVzdCB1biB0ZXh0ZQ=="),
                      std::string((const char *)output2));

    unsigned char input3[] = "test some text";
    unsigned char output3[64];
    size_t res3 = b64converter.encode(output3, sizeof(output3), input3, sizeof(input3)-1);

    BOOST_CHECK_EQUAL(res3, 20);
    output3[res2] = 0;
    BOOST_CHECK_EQUAL(std::string("dGVzdCBzb21lIHRleHQ="),
                      std::string((const char *)output3));
}

BOOST_AUTO_TEST_CASE(TestDecoding)
{
    Base64 b64converter;

    unsigned char input[] = "dGVzdCBzb21lIHRleHQ=";
    unsigned char output[64];

    size_t res = b64converter.decode(output, 64, input, sizeof(input)-1);
    BOOST_CHECK_EQUAL(res, 14);
    output[res] = 0;
    BOOST_CHECK_EQUAL(std::string("test some text"),
                      std::string((const char*)output));

    unsigned char input2[] = "dGVzdCB1biB0ZXh0ZQ==";
    unsigned char output2[64];

    size_t res2 = b64converter.decode(output2, 64, input2, sizeof(input2)-1);
    BOOST_CHECK_EQUAL(res2, 13);
    output2[res2] = 0;
    BOOST_CHECK_EQUAL(std::string("test un texte"),
                      std::string((const char*)output2));

    unsigned char input3[] = "SGkhSGkh";
    unsigned char output3[64];

    size_t res3 = b64converter.decode(output3, 64, input3, sizeof(input3)-1);
    BOOST_CHECK_EQUAL(res2, 13);
    output3[res3] = 0;
    BOOST_CHECK_EQUAL(std::string("Hi!Hi!"),
                      std::string((const char*)output3));


}

BOOST_AUTO_TEST_CASE(TestRandom)
{
    Base64 b64converter;

    LCGRandom rand(0xA3F451);
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

void testrandom(uint32_t seed) {
    Base64 b64converter;

    LCGRandom rand(seed);
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

BOOST_AUTO_TEST_CASE(TestManyRandom)
{
    LCGRandom rand(0x123ABC);
    for (unsigned i = 0xF; i < 0xFFFF; i++) {
        testrandom(rand.rand32()*i);
    }
}

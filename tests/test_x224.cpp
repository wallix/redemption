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
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestX224
#include <boost/test/auto_unit_test.hpp>

#include "stream.hpp"
#include "transport.hpp"
#include "RDP/x224.hpp"


BOOST_AUTO_TEST_CASE(TestReceive_CR_TPDU)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", 11);
    X224In tpdu(&t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(11, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::CR_TPDU, tpdu.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(6, tpdu.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL(11, stream.get_offset(0));
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
}

BOOST_AUTO_TEST_CASE(TestReceive_CC_TPDU)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", 11);
    X224In tpdu(&t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(11, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::CC_TPDU, tpdu.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(6, tpdu.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL(stream.p, stream.data+11);
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
}


BOOST_AUTO_TEST_CASE(TestReceive_DR_TPDU)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x01", 11);
    X224In tpdu(&t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(11, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::DR_TPDU, tpdu.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(6, tpdu.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL(1, tpdu.tpdu_hdr.reason);
    BOOST_CHECK_EQUAL(stream.p, stream.data+11);
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
}


BOOST_AUTO_TEST_CASE(TestReceive_DT_TPDU)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0B\x02\xF0\x80\x12\x34\x56\x78", 11);
    X224In tpdu(&t, stream);
    // tpkt header is OK
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(11, tpdu.tpkt.len);

    // X224 header is OK
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::DT_TPDU, tpdu.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(2, tpdu.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL(0x80, tpdu.tpdu_hdr.eot);

    // stream points to user data to read
    BOOST_CHECK_EQUAL(0x12, stream.p[0]);
    BOOST_CHECK_EQUAL(0x34, stream.p[1]);
    BOOST_CHECK_EQUAL(0x56, stream.p[2]);
    BOOST_CHECK_EQUAL(0x78, stream.p[3]);
    BOOST_CHECK_EQUAL(stream.p, stream.data+7);
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
}

BOOST_AUTO_TEST_CASE(TestReceive_ER_TPDU)
{
    Stream stream;
    GeneratorTransport t("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", 13);
    X224In tpdu(&t, stream);
    // tpkt header is OK
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(13, tpdu.tpkt.len);

    // X224 header is OK
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::ER_TPDU, tpdu.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(8, tpdu.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL(2, tpdu.tpdu_hdr.reject_cause);

    BOOST_CHECK_EQUAL(13, stream.get_offset(0));
    BOOST_CHECK_EQUAL(stream.end, stream.data+tpdu.tpkt.len);
}



BOOST_AUTO_TEST_CASE(TestSend_CR_TPDU)
{
    Stream stream(65536);
    memset(stream.data, 0, 65536);

    GeneratorTransport t("", 0); // used as /dev/null
    X224Out tpdu(X224Packet::CR_TPDU, stream);
    tpdu.end();
    BOOST_CHECK_EQUAL(stream.get_offset(0), stream.data[2]*256+stream.data[3]);
    // tpkt header
    BOOST_CHECK_EQUAL(0x03, stream.data[0]); // version 3
    BOOST_CHECK_EQUAL(0x00, stream.data[1]);
    BOOST_CHECK_EQUAL(0x00, stream.data[2]); // len 11
    BOOST_CHECK_EQUAL(0x0B, stream.data[3]); //

    // CR_TPDU
    BOOST_CHECK_EQUAL(0x06, stream.data[4]); // LI
    BOOST_CHECK_EQUAL(0xE0, stream.data[5]); // CR_TPDU code
    BOOST_CHECK_EQUAL(0x00, stream.data[6]); // DST-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[7]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[8]); // SRC-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[9]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[10]);  // CLASS OPTION
    tpdu.send(&t);
}

BOOST_AUTO_TEST_CASE(TestSend_CC_TPDU)
{
    Stream stream(65536);
    memset(stream.data, 0, 65536);

    GeneratorTransport t("", 0); // used as /dev/null
    X224Out tpdu(X224Packet::CC_TPDU, stream);
    tpdu.end();
    BOOST_CHECK_EQUAL(stream.get_offset(0), stream.data[2]*256+stream.data[3]);
    // tpkt header
    BOOST_CHECK_EQUAL(0x03, stream.data[0]); // version 3
    BOOST_CHECK_EQUAL(0x00, stream.data[1]);
    BOOST_CHECK_EQUAL(0x00, stream.data[2]); // len 11
    BOOST_CHECK_EQUAL(0x0B, stream.data[3]); //

    // CC_TPDU
    BOOST_CHECK_EQUAL(0x06, stream.data[4]); // LI
    BOOST_CHECK_EQUAL(0xD0, stream.data[5]); // CC_TPDU code
    BOOST_CHECK_EQUAL(0x00, stream.data[6]); // DST-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[7]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[8]); // SRC-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[9]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[10]);  // CLASS OPTION
    tpdu.send(&t);
}

BOOST_AUTO_TEST_CASE(TestSend_DR_TPDU)
{
    Stream stream(65536);
    memset(stream.data, 0, 65536);

    GeneratorTransport t("", 0); // used as /dev/null
    X224Out tpdu(X224Packet::DR_TPDU, stream);
    tpdu.end();
    BOOST_CHECK_EQUAL(stream.get_offset(0), stream.data[2]*256+stream.data[3]);
    // tpkt header
    BOOST_CHECK_EQUAL(0x03, stream.data[0]); // version 3
    BOOST_CHECK_EQUAL(0x00, stream.data[1]);
    BOOST_CHECK_EQUAL(0x00, stream.data[2]); // len 11
    BOOST_CHECK_EQUAL(0x0B, stream.data[3]); //

    // DR_TPDU
    BOOST_CHECK_EQUAL(0x06, stream.data[4]); // LI
    BOOST_CHECK_EQUAL(0x80, stream.data[5]); // DR_TPDU code
    BOOST_CHECK_EQUAL(0x00, stream.data[6]); // DST-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[7]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[8]); // SRC-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[9]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[10]);  // REASON (0 = Not specified)
    tpdu.send(&t);
}

BOOST_AUTO_TEST_CASE(TestSend_ER_TPDU)
{
    Stream stream(65536);
    memset(stream.data, 0, 65536);

    GeneratorTransport t("", 0); // used as /dev/null
    X224Out tpdu(X224Packet::ER_TPDU, stream);
    tpdu.end();
    BOOST_CHECK_EQUAL(stream.get_offset(0), stream.data[2]*256+stream.data[3]);
    // tpkt header
    BOOST_CHECK_EQUAL(0x03, stream.data[0]); // version 3
    BOOST_CHECK_EQUAL(0x00, stream.data[1]);
    BOOST_CHECK_EQUAL(0x00, stream.data[2]); // len 11
    BOOST_CHECK_EQUAL(0x0B, stream.data[3]); //

    // ER_TPDU
    BOOST_CHECK_EQUAL(0x06, stream.data[4]); // LI
    BOOST_CHECK_EQUAL(0x70, stream.data[5]); // ER_TPDU code
    BOOST_CHECK_EQUAL(0x00, stream.data[6]); // DST-REF
    BOOST_CHECK_EQUAL(0x00, stream.data[7]); //
    BOOST_CHECK_EQUAL(0x00, stream.data[8]); // Reject Cause : Unspecified
    BOOST_CHECK_EQUAL(0xC1, stream.data[9]); // Invalid TPDU Code
    BOOST_CHECK_EQUAL(0x00, stream.data[10]);  // Parameter Length 0
    tpdu.send(&t);
}

BOOST_AUTO_TEST_CASE(TestSend_DT_TPDU)
{
    Stream stream(65536);
    memset(stream.data, 0, 65536);
    GeneratorTransport t("", 0); // used as /dev/null
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    //------------ Here stream points to where user must write it's data if any
    stream.out_uint8(0x12);
    stream.out_uint8(0x34);
    stream.out_uint8(0x56);
    stream.out_uint8(0x78);
    tpdu.end();
    BOOST_CHECK_EQUAL(stream.get_offset(0), stream.data[2]*256+stream.data[3]);
    // tpkt header
    BOOST_CHECK_EQUAL(0x03, stream.data[0]); // version 3
    BOOST_CHECK_EQUAL(0x00, stream.data[1]);
    BOOST_CHECK_EQUAL(0x00, stream.data[2]); // len 11
    BOOST_CHECK_EQUAL(0x0B, stream.data[3]); //

    // DT_TPDU
    BOOST_CHECK_EQUAL(0x02, stream.data[4]); // LI
    BOOST_CHECK_EQUAL(0xF0, stream.data[5]); // DT_TPDU code
    BOOST_CHECK_EQUAL(0x80, stream.data[6]); // EOT

    // USER DATA
    BOOST_CHECK_EQUAL(0x12, stream.data[7]);
    BOOST_CHECK_EQUAL(0x34, stream.data[8]);
    BOOST_CHECK_EQUAL(0x56, stream.data[9]);
    BOOST_CHECK_EQUAL(0x78, stream.data[10]);

    tpdu.send(&t);
}



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

//#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "RDP/x224.hpp"


BOOST_AUTO_TEST_CASE(TestReceive_CR_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", 11);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::CR_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)11, (size_t)fac_x224.length);
    
    X224::CR_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(11, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::CR_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(6, x224.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL(0, strlen(x224.cookie));
    BOOST_CHECK_EQUAL(0, x224.rdp_neg_type);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}

BOOST_AUTO_TEST_CASE(TestSend_CR_TPDU)
{
    BStream stream(256); 
    X224::CR_TPDU_Send x224(stream, "", 0, 0, 0);
    BOOST_CHECK_EQUAL(11, stream.size());
    BOOST_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", stream.data, 11));
}

BOOST_AUTO_TEST_CASE(TestReceive_CR_TPDU_with_factory_TLS_Negotiation_packet)
{
    size_t tpkt_len = 55;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x37\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
        , tpkt_len);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::CR_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL(tpkt_len, (size_t)fac_x224.length);
    
    X224::CR_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(tpkt_len, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::CR_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(0x32, x224.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL(0, strcmp("Cookie: mstshash=administrateur@qa\x0D\x0A", x224.cookie));
    BOOST_CHECK_EQUAL((uint8_t)X224::RDP_NEG_REQ, x224.rdp_neg_type);
    BOOST_CHECK_EQUAL(0, x224.rdp_neg_flags);
    BOOST_CHECK_EQUAL(8, x224.rdp_neg_length);
    BOOST_CHECK_EQUAL((uint32_t)X224::RDP_NEG_PROTOCOL_TLS, x224.rdp_neg_code);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}

BOOST_AUTO_TEST_CASE(TestSend_CR_TPDU_TLS_Negotiation_packet)
{
    BStream stream(256); 
    X224::CR_TPDU_Send(stream,
            "Cookie: mstshash=administrateur@qa\x0D\x0A", 
            X224::RDP_NEG_REQ, 0, X224::RDP_NEG_PROTOCOL_TLS);
    BOOST_CHECK_EQUAL(55, stream.size());
    BOOST_CHECK_EQUAL(0, memcmp(
/* 0000 */ "\x03\x00\x00\x37\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
    , stream.data, 55));
}

BOOST_AUTO_TEST_CASE(TestReceive_CC_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", 11);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::CC_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)11, (size_t)fac_x224.length);
    
    X224::CC_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(11, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::CC_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(6, x224.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL(0, x224.rdp_neg_type);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}


BOOST_AUTO_TEST_CASE(TestSend_CC_TPDU)
{
    BStream stream(256); 
    X224::CC_TPDU_Send x224(stream, 0, 0, 0);
    BOOST_CHECK_EQUAL(11, stream.size());
    BOOST_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", stream.data, 11));
}

BOOST_AUTO_TEST_CASE(TestReceive_CC_TPDU_TLS_with_factory)
{
    size_t tpkt_len = 19;
    GeneratorTransport t("\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00", tpkt_len);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::CC_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)tpkt_len, (size_t)fac_x224.length);
    
    X224::CC_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(tpkt_len, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::CC_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(0, x224.tpdu_hdr.dst_ref);
    BOOST_CHECK_EQUAL(0, x224.tpdu_hdr.src_ref);
    BOOST_CHECK_EQUAL(14, x224.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL((uint8_t)X224::RDP_NEG_RESP, x224.rdp_neg_type);
    BOOST_CHECK_EQUAL(0, x224.rdp_neg_flags);
    BOOST_CHECK_EQUAL(8, x224.rdp_neg_length);
    BOOST_CHECK_EQUAL((uint32_t)X224::RDP_NEG_PROTOCOL_TLS, x224.rdp_neg_code);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}

BOOST_AUTO_TEST_CASE(TestSend_CC_TPDU_TLS)
{
    BStream stream(256); 
    X224::CC_TPDU_Send x224(stream, X224::RDP_NEG_RESP, 0, X224::RDP_NEG_PROTOCOL_TLS);
    BOOST_CHECK_EQUAL(19, stream.size());
    BOOST_CHECK_EQUAL(0, 
        memcmp("\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00", stream.data, 19));
}

BOOST_AUTO_TEST_CASE(TestReceive_DR_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x00", 11);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::DR_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)11, (size_t)fac_x224.length);
    
    X224::DR_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(11, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::DR_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL((uint8_t)X224::REASON_NOT_SPECIFIED, x224.tpdu_hdr.reason);
    BOOST_CHECK_EQUAL(6, x224.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}

BOOST_AUTO_TEST_CASE(TestSend_DR_TPDU)
{
    BStream stream(256); 
    X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
    BOOST_CHECK_EQUAL(11, stream.size());
    BOOST_CHECK_EQUAL(0, 
        memcmp("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x00", stream.data, 11));
}

BOOST_AUTO_TEST_CASE(TestReceive_ER_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", 13);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::ER_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)13, (size_t)fac_x224.length);
    
    X224::ER_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(13, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::ER_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(8, x224.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL((uint8_t)X224::REASON_INVALID_TPDU_TYPE, x224.tpdu_hdr.reject_cause);
    BOOST_CHECK_EQUAL(0xC1, x224.tpdu_hdr.invalid_tpdu_var);
    BOOST_CHECK_EQUAL(2, x224.tpdu_hdr.invalid_tpdu_vl);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(x224._header_size, stream.size());
}

BOOST_AUTO_TEST_CASE(TestSend_ER_TPDU)
{
    BStream stream(256);
    uint8_t invalid[2] = {0x06, 0x22};
    X224::ER_TPDU_Send x224(stream, X224::REASON_INVALID_TPDU_TYPE, 2, invalid);
    BOOST_CHECK_EQUAL(13, stream.size());
    BOOST_CHECK_EQUAL(0, 
        memcmp("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", stream.data, 13));
}

BOOST_AUTO_TEST_CASE(TestReceive_DT_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0C\x02\xF0\x80\x12\x34\x56\x78\x9A", 12);

    BStream stream(65536);
    X224::RecvFactory fac_x224(t, stream);
    BOOST_CHECK_EQUAL((uint8_t)X224::DT_TPDU, fac_x224.type);
    BOOST_CHECK_EQUAL((size_t)12, (size_t)fac_x224.length);
    
    X224::DT_TPDU_Recv x224(t, stream, fac_x224.length);

    BOOST_CHECK_EQUAL(3, x224.tpkt.version);
    BOOST_CHECK_EQUAL(12, x224.tpkt.len);
    BOOST_CHECK_EQUAL((uint8_t)X224::DT_TPDU, x224.tpdu_hdr.code);
    BOOST_CHECK_EQUAL(2, x224.tpdu_hdr.LI);

    BOOST_CHECK_EQUAL(stream.size(), x224.tpkt.len);
    BOOST_CHECK_EQUAL(7, x224._header_size);
    BOOST_CHECK_EQUAL(x224._header_size + x224.payload_size, stream.size());

    BOOST_CHECK_EQUAL(5, x224.payload_size);
}


BOOST_AUTO_TEST_CASE(TestSend_DT_TPDU)
{
    BStream payload(256);
    payload.out_copy_bytes("\x12\x34\x56\x78\x9A", 5);
    payload.end = payload.p;

    size_t payload_len = payload.end - payload.data;
    BStream stream(256);
    X224::DT_TPDU_Send x224(stream, payload_len);

    BOOST_CHECK_EQUAL(7, stream.size());
    BOOST_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0C\x02\xF0\x80", stream.data, 7));
    BOOST_CHECK_EQUAL(5, payload_len);
    BOOST_CHECK_EQUAL(0, memcmp("\x12\x34\x56\x78\x9A", payload.data, 5));

    
    CheckTransport t("\x03\x00\x00\x0C\x02\xF0\x80\x12\x34\x56\x78\x9A", 12);
    t.send(stream.data, stream.size());
    t.send(payload.data, payload_len);
    BOOST_CHECK_EQUAL(true, t.status);
}

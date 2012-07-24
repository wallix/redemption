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
#define BOOST_TEST_MODULE TestMCS
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "RDP/mcs.hpp"


BOOST_AUTO_TEST_CASE(TestReceive_MCSPDU_CONNECT_INITIAL_with_factory)
{
    size_t payload_length = 369;
    GeneratorTransport t(
/* 0000 */                             "\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" //       .e..l.... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" //.....0..."...... |
/* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" //................ |
/* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" //.0.............. |
/* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" //........ ...0... |
/* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" //................ |
/* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" //................ |
/* 0070 */ "\x07\x00\x05\x00\x14\x7c\x00\x01\x80\xfe\x00\x08\x00\x10\x00\x01" //.....|.......... |
/* 0080 */ "\xc0\x00\x44\x75\x63\x61\x80\xf0\x01\xc0\xd8\x00\x04\x00\x08\x00" //..Duca.......... |
/* 0090 */ "\x00\x04\x00\x03\x01\xca\x03\xaa\x0c\x04\x00\x00\x28\x0a\x00\x00" //............(... |
/* 00a0 */ "\x31\x00\x39\x00\x35\x00\x2d\x00\x31\x00\x33\x00\x32\x00\x2d\x00" //1.9.5.-.1.3.2.-. |
/* 00b0 */ "\x32\x00\x30\x00\x33\x00\x2d\x00\x32\x00\x31\x00\x32\x00\x00\x00" //2.0.3.-.2.1.2... |
/* 00c0 */ "\x04\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00d0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00e0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 00f0 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0100 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xca\x01\x00" //................ |
/* 0110 */ "\x00\x00\x00\x00\x10\x00\x07\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0120 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0130 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0140 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
/* 0150 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00" //................ |
/* 0160 */ "\x04\xc0\x0c\x00\x0d\x00\x00\x00\x00\x00\x00\x00\x02\xc0\x0c\x00" //................ |
/* 0170 */ "\x03\x00\x00\x00\x00\x00\x00\x00"                                 //........ |
    , payload_length);

    BStream payload(65536);
    t.recv(&payload.end, payload_length);

    MCS::RecvFactory fac_mcs(payload, MCS::BER_ENCODING);
    BOOST_CHECK_EQUAL((uint8_t)MCS::MCSPDU_CONNECT_INITIAL, fac_mcs.type);

    MCS::CONNECT_INITIAL_PDU_Recv mcs(payload, payload_length, MCS::BER_ENCODING);

    BOOST_CHECK_EQUAL(101, mcs.tag);
    BOOST_CHECK_EQUAL(369, mcs.tag_len + 5);
 
    BOOST_CHECK_EQUAL(34, mcs.targetParameters.maxChannelIds);
    BOOST_CHECK_EQUAL(2, mcs.targetParameters.maxUserIds);
    BOOST_CHECK_EQUAL(0, mcs.targetParameters.maxTokenIds);
    BOOST_CHECK_EQUAL(1, mcs.targetParameters.numPriorities);
    BOOST_CHECK_EQUAL(0, mcs.targetParameters.minThroughput);
    BOOST_CHECK_EQUAL(1, mcs.targetParameters.maxHeight);
    BOOST_CHECK_EQUAL(65535, mcs.targetParameters.maxMCSPDUsize);
    BOOST_CHECK_EQUAL(2, mcs.targetParameters.protocolVersion);            

    BOOST_CHECK_EQUAL(1, mcs.minimumParameters.maxChannelIds);
    BOOST_CHECK_EQUAL(1, mcs.minimumParameters.maxUserIds);
    BOOST_CHECK_EQUAL(1, mcs.minimumParameters.maxTokenIds);
    BOOST_CHECK_EQUAL(1, mcs.minimumParameters.numPriorities);
    BOOST_CHECK_EQUAL(0, mcs.minimumParameters.minThroughput);
    BOOST_CHECK_EQUAL(1, mcs.minimumParameters.maxHeight);
    BOOST_CHECK_EQUAL(1056, mcs.minimumParameters.maxMCSPDUsize);
    BOOST_CHECK_EQUAL(2, mcs.minimumParameters.protocolVersion);

    BOOST_CHECK_EQUAL(65535, mcs.maximumParameters.maxChannelIds);
    BOOST_CHECK_EQUAL(64535, mcs.maximumParameters.maxUserIds);
    BOOST_CHECK_EQUAL(65535, mcs.maximumParameters.maxTokenIds);
    BOOST_CHECK_EQUAL(1, mcs.maximumParameters.numPriorities);
    BOOST_CHECK_EQUAL(0, mcs.maximumParameters.minThroughput);
    BOOST_CHECK_EQUAL(1, mcs.maximumParameters.maxHeight);
    BOOST_CHECK_EQUAL(65535, mcs.maximumParameters.maxMCSPDUsize);
    BOOST_CHECK_EQUAL(2, mcs.maximumParameters.protocolVersion);

    BOOST_CHECK_EQUAL(1, mcs.len_callingDomainSelector);
    BOOST_CHECK_EQUAL(0, memcmp("\x01", mcs.callingDomainSelector, 1));

    BOOST_CHECK_EQUAL(1, mcs.len_calledDomainSelector);
    BOOST_CHECK_EQUAL(0, memcmp("\x01", mcs.calledDomainSelector, 1));

    BOOST_CHECK_EQUAL(true, mcs.upwardFlag);

    BOOST_CHECK_EQUAL(106, mcs.header_size); // everything up to USER_DATA
    BOOST_CHECK_EQUAL(263, mcs.payload_size); // USER_DATA (after len)
    BOOST_CHECK_EQUAL(mcs.payload_size, payload.end - payload.data - mcs.header_size);
}

BOOST_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_INITIAL)
{
    BStream stream(1024);
    size_t payload_length = 263;
    MCS::CONNECT_INITIAL_Send mcs(stream, payload_length, MCS::BER_ENCODING);
    BOOST_CHECK_EQUAL(106, stream.end - stream.data);

    const char * expected = 
/* 0000 */                             "\x7f\x65\x82\x01\x6c\x04\x01\x01\x04" //       .e..l.... |
/* 0010 */ "\x01\x01\x01\x01\xff\x30\x1a\x02\x01\x22\x02\x01\x02\x02\x01\x00" //.....0..."...... |
/* 0020 */ "\x02\x01\x01\x02\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01" //................ |
/* 0030 */ "\x02\x30\x19\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02\x01\x01\x02" //.0.............. |
/* 0040 */ "\x01\x00\x02\x01\x01\x02\x02\x04\x20\x02\x01\x02\x30\x1f\x02\x03" //........ ...0... |
/* 0050 */ "\x00\xff\xff\x02\x02\xfc\x17\x02\x03\x00\xff\xff\x02\x01\x01\x02" //................ |
/* 0060 */ "\x01\x00\x02\x01\x01\x02\x03\x00\xff\xff\x02\x01\x02\x04\x82\x01" //................ |
/* 0070 */ "\x07" //.....|.......... |
    ;

    BOOST_CHECK_EQUAL(0, memcmp(expected, stream.data, 106));
}

BOOST_AUTO_TEST_CASE(TestReceive_MCSPDU_CONNECT_RESPONSE_with_factory)
{
    size_t payload_length = 93;
    GeneratorTransport t(
 /* 0000 */             "\x7f\x66\x5a\x0a\x01\x00\x02\x01\x00\x30\x1a\x02\x01" //....fZ......0... |
 /* 0010 */ "\x22\x02\x01\x03\x02\x01\x00\x02\x01\x01\x02\x01\x00\x02\x01\x01" //"............... |
 /* 0020 */ "\x02\x03\x00\xff\xf8\x02\x01\x02\x04\x36\x00\x05\x00\x14\x7c\x00" //.........6....|. |
 /* 0030 */ "\x01\x2a\x14\x76\x0a\x01\x01\x00\x01\xc0\x00\x4d\x63\x44\x6e\x20" //.*.v.......McDn  |
 /* 0040 */ "\x01\x0c\x0c\x00\x04\x00\x08\x00\x01\x00\x00\x00\x03\x0c\x08\x00" //................ |
 /* 0050 */ "\xeb\x03\x00\x00\x02\x0c\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ |
   , payload_length);

    BStream payload(65536);
    t.recv(&payload.end, payload_length);

    MCS::RecvFactory fac_mcs(payload, MCS::BER_ENCODING);
    BOOST_CHECK_EQUAL((uint8_t)MCS::MCSPDU_CONNECT_RESPONSE, fac_mcs.type);

    MCS::CONNECT_RESPONSE_PDU_Recv mcs(payload, payload_length, MCS::BER_ENCODING);
 
    BOOST_CHECK_EQUAL(102, mcs.tag);
    BOOST_CHECK_EQUAL(90, mcs.tag_len);

    BOOST_CHECK_EQUAL(0, mcs.result);
    BOOST_CHECK_EQUAL(0, mcs.connectId);

    BOOST_CHECK_EQUAL(34, mcs.domainParameters.maxChannelIds);
    BOOST_CHECK_EQUAL(3, mcs.domainParameters.maxUserIds);
    BOOST_CHECK_EQUAL(0, mcs.domainParameters.maxTokenIds);
    BOOST_CHECK_EQUAL(1, mcs.domainParameters.numPriorities);
    BOOST_CHECK_EQUAL(0, mcs.domainParameters.minThroughput);
    BOOST_CHECK_EQUAL(1, mcs.domainParameters.maxHeight);
    BOOST_CHECK_EQUAL(65528, mcs.domainParameters.maxMCSPDUsize);
    BOOST_CHECK_EQUAL(2, mcs.domainParameters.protocolVersion);            

    BOOST_CHECK_EQUAL(54, mcs.payload_size);
    BOOST_CHECK_EQUAL(39, mcs.header_size);
}

BOOST_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_RESPONSE)
{
    BStream stream(1024);
    size_t payload_size = 54;
    size_t header_size = 39;
    MCS::CONNECT_RESPONSE_Send mcs(stream, payload_size, MCS::BER_ENCODING);
    BOOST_CHECK_EQUAL(header_size, stream.end - stream.data);

    const char * expected = 
    "\x7f\x66" // BER_TAG_MCS_CONNECT_RESPONSE
    "\x5a"     // LEN = payload_size + header_size
        // Result
        "\x0a"     // Stream::BER_TAG_RESULT
        "\x01"     // LEN RESULT
        "\x00"     // RESULT VALUE
        // ConnectId
        "\x02"     // BER_TAG_INTEGER
        "\x01"     // LEN
        "\x00"     // ConnectId value
        // DomainParameters
        "\x30"     // BER_TAG_MCS_DOMAIN_PARAMS
        "\x1a"     // LEN
            // DomainParameters::maxChannelIds = 34
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x22" // VALUE
            // DomainParameters::maxUserIds = 3
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x03" // VALUE
            // DomainParameters::maximumTokenIds = 0
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x00" // VALUE
            // DomainParameters::numPriorities = 1
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x01" // VALUE
            // DomainParameters::minThroughput = 0
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x00" // VALUE
            // DomainParameters::maxHeight = 1
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x01" // VALUE
            // DomainParameters::maxMCSPDUsize = 65528
            "\x02" // BER_TAG_INTEGER
            "\x03" // LEN
            "\x00\xff\xf8" // VALUE
            // DomainParameters::protocolVersion = 2
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x02" // VALUE
        // UserData
        "\x04" // BER_TAG_OCTET_STRING
        "\x36" // PAYLOAD LEN
    ;

    BOOST_CHECK_EQUAL(0, memcmp(expected, stream.data, header_size));
}

BOOST_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_RESPONSE_large_payload)
{
    BStream stream(2048);
    size_t payload_size = 1024;
    size_t header_size = 43;
    try {
        MCS::CONNECT_RESPONSE_Send mcs(stream, payload_size, MCS::BER_ENCODING);
    }
    catch (...) {
    };
    BOOST_CHECK_EQUAL(header_size, stream.end - stream.data);

    const char * expected = 
    "\x7f\x66" // BER_TAG_MCS_CONNECT_RESPONSE
    "\x82\x04\x26"     // LEN = payload_size + header_size
        // Result
        "\x0a"     // Stream::BER_TAG_RESULT
        "\x01"     // LEN RESULT
        "\x00"     // RESULT VALUE
        // ConnectId
        "\x02"     // BER_TAG_INTEGER
        "\x01"     // LEN
        "\x00"     // ConnectId value
        // DomainParameters
        "\x30"     // BER_TAG_MCS_DOMAIN_PARAMS
        "\x1a"     // LEN
            // DomainParameters::maxChannelIds = 34
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x22" // VALUE
            // DomainParameters::maxUserIds = 3
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x03" // VALUE
            // DomainParameters::maximumTokenIds = 0
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x00" // VALUE
            // DomainParameters::numPriorities = 1
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x01" // VALUE
            // DomainParameters::minThroughput = 0
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x00" // VALUE
            // DomainParameters::maxHeight = 1
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x01" // VALUE
            // DomainParameters::maxMCSPDUsize = 65528
            "\x02" // BER_TAG_INTEGER
            "\x03" // LEN
            "\x00\xff\xf8" // VALUE
            // DomainParameters::protocolVersion = 2
            "\x02" // BER_TAG_INTEGER
            "\x01" // LEN
            "\x02" // VALUE
        // UserData
        "\x04" // BER_TAG_OCTET_STRING
        "\x82\x04\x00" // PAYLOAD LEN
    ;

    BOOST_CHECK_EQUAL(0, memcmp(expected, stream.data, header_size));
}


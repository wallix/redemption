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
   Author(s): Christophe Grosjean

   Unit test to Mcs PDU coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestMCS
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/stream.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/RDP/mcs.hpp"

RED_AUTO_TEST_CASE(TestReceive_MCSPDU_CONNECT_INITIAL_with_factory)
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

    uint8_t payload_buf[65536];
    InStream payload(payload_buf, payload_length);
    {
        auto end = payload_buf;
        t.recv_boom(end, payload_length);
    }

    MCS::CONNECT_INITIAL_PDU_Recv mcs(payload, MCS::BER_ENCODING);

    RED_CHECK_EQUAL(101, mcs.tag);
    RED_CHECK_EQUAL(369, mcs.tag_len + 5);

    RED_CHECK_EQUAL(34, mcs.targetParameters.maxChannelIds);
    RED_CHECK_EQUAL(2, mcs.targetParameters.maxUserIds);
    RED_CHECK_EQUAL(0, mcs.targetParameters.maxTokenIds);
    RED_CHECK_EQUAL(1, mcs.targetParameters.numPriorities);
    RED_CHECK_EQUAL(0, mcs.targetParameters.minThroughput);
    RED_CHECK_EQUAL(1, mcs.targetParameters.maxHeight);
    RED_CHECK_EQUAL(65535, mcs.targetParameters.maxMCSPDUsize);
    RED_CHECK_EQUAL(2, mcs.targetParameters.protocolVersion);

    RED_CHECK_EQUAL(1, mcs.minimumParameters.maxChannelIds);
    RED_CHECK_EQUAL(1, mcs.minimumParameters.maxUserIds);
    RED_CHECK_EQUAL(1, mcs.minimumParameters.maxTokenIds);
    RED_CHECK_EQUAL(1, mcs.minimumParameters.numPriorities);
    RED_CHECK_EQUAL(0, mcs.minimumParameters.minThroughput);
    RED_CHECK_EQUAL(1, mcs.minimumParameters.maxHeight);
    RED_CHECK_EQUAL(1056, mcs.minimumParameters.maxMCSPDUsize);
    RED_CHECK_EQUAL(2, mcs.minimumParameters.protocolVersion);

    RED_CHECK_EQUAL(65535, mcs.maximumParameters.maxChannelIds);
    RED_CHECK_EQUAL(64535, mcs.maximumParameters.maxUserIds);
    RED_CHECK_EQUAL(65535, mcs.maximumParameters.maxTokenIds);
    RED_CHECK_EQUAL(1, mcs.maximumParameters.numPriorities);
    RED_CHECK_EQUAL(0, mcs.maximumParameters.minThroughput);
    RED_CHECK_EQUAL(1, mcs.maximumParameters.maxHeight);
    RED_CHECK_EQUAL(65535, mcs.maximumParameters.maxMCSPDUsize);
    RED_CHECK_EQUAL(2, mcs.maximumParameters.protocolVersion);

    RED_CHECK_EQUAL(1, mcs.len_callingDomainSelector);
    RED_CHECK_EQUAL(0, memcmp("\x01", mcs.callingDomainSelector, 1));

    RED_CHECK_EQUAL(1, mcs.len_calledDomainSelector);
    RED_CHECK_EQUAL(0, memcmp("\x01", mcs.calledDomainSelector, 1));

    RED_CHECK_EQUAL(true, mcs.upwardFlag);

    RED_CHECK_EQUAL(106, mcs._header_size); // everything up to USER_DATA
    RED_CHECK_EQUAL(263, mcs.payload.in_remain()); // USER_DATA (after len)
    RED_CHECK_EQUAL(mcs.payload.get_capacity(), payload.get_capacity() - mcs._header_size);
}


RED_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_INITIAL)
{
    StaticOutStream<1024> stream;
    size_t payload_length = 263;
    MCS::CONNECT_INITIAL_Send mcs(stream, payload_length, MCS::BER_ENCODING);
    RED_CHECK_EQUAL(106, stream.get_offset());

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

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), 106));
}

RED_AUTO_TEST_CASE(TestReceive_MCSPDU_CONNECT_RESPONSE_with_factory)
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

    uint8_t buf[65536];
    auto end = buf;
    t.recv_boom(end, payload_length);

    InStream payload(buf, payload_length);
    MCS::CONNECT_RESPONSE_PDU_Recv mcs(payload, MCS::BER_ENCODING);

    RED_CHECK_EQUAL(102, mcs.tag);
    RED_CHECK_EQUAL(90, mcs.tag_len);

    RED_CHECK_EQUAL(0, mcs.result);
    RED_CHECK_EQUAL(0, mcs.connectId);

    RED_CHECK_EQUAL(34, mcs.domainParameters.maxChannelIds);
    RED_CHECK_EQUAL(3, mcs.domainParameters.maxUserIds);
    RED_CHECK_EQUAL(0, mcs.domainParameters.maxTokenIds);
    RED_CHECK_EQUAL(1, mcs.domainParameters.numPriorities);
    RED_CHECK_EQUAL(0, mcs.domainParameters.minThroughput);
    RED_CHECK_EQUAL(1, mcs.domainParameters.maxHeight);
    RED_CHECK_EQUAL(65528, mcs.domainParameters.maxMCSPDUsize);
    RED_CHECK_EQUAL(2, mcs.domainParameters.protocolVersion);

    RED_CHECK_EQUAL(54, mcs.payload.get_capacity());
    RED_CHECK_EQUAL(39, payload_length - mcs.payload.get_capacity());
}

RED_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_RESPONSE)
{
    StaticOutStream<1024> stream;
    size_t payload_size = 54;
    size_t header_size = 39;
    MCS::CONNECT_RESPONSE_Send mcs(stream, payload_size, MCS::BER_ENCODING);
    RED_CHECK_EQUAL(header_size, stream.get_offset());

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

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), header_size));
}

RED_AUTO_TEST_CASE(TestSend_MCSPDU_CONNECT_RESPONSE_large_payload)
{
    StaticOutStream<2048> stream;
    size_t payload_size = 1024;
    size_t header_size = 43;
    RED_CHECK_NO_THROW(MCS::CONNECT_RESPONSE_Send(stream, payload_size, MCS::BER_ENCODING));
    RED_CHECK_EQUAL(header_size, stream.get_offset());

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

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), header_size));
}

RED_AUTO_TEST_CASE(TestSend_ErectDomainRequest)
{
    StaticOutStream<1024> stream;
    size_t length = 5;
    int subheight = 0;
    int subinterval = 0;
    MCS::ErectDomainRequest_Send mcs(stream, subheight, subinterval, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x04"  // ErectDomainRequest * 4
        "\x01"  // subHeight len
        "\x00"  // subHeight
        "\x01"  // subInterval len
        "\x00"  // subInterval
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_ErectDomainRequest)
{
    size_t length = 5;
    GeneratorTransport t(
        "\x04"  // ErectDomainRequest * 4
        "\x01"  // subHeight len
        "\x00"  // subHeight
        "\x01"  // subInterval len
        "\x00"  // subInterval
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::ErectDomainRequest_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(MCS::MCSPDU_ErectDomainRequest , mcs.type);
    RED_CHECK_EQUAL(0, mcs.subHeight);
    RED_CHECK_EQUAL(0, mcs.subInterval);
}

RED_AUTO_TEST_CASE(TestSend_DisconnectProviderUltimatum)
{
    StaticOutStream<1024> stream;
    size_t length = 2;
    MCS::DisconnectProviderUltimatum_Send mcs(stream, MCS::RN_DOMAIN_DISCONNECTED, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x20"  // DisconnectProviderUltimatum * 4
        "\x00"  // reason
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_DisconnectProviderUltimatum)
{
    size_t length = 2;
    GeneratorTransport t(
        "\x20"  // DisconnectProviderUltimatum * 4
        "\x00"  // reason
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::DisconnectProviderUltimatum_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_DisconnectProviderUltimatum), mcs.type);
    RED_CHECK_EQUAL(0, mcs.reason);
}


RED_AUTO_TEST_CASE(TestSend_AttachUserRequest)
{
    StaticOutStream<1024> stream;
    size_t length = 1;
    MCS::AttachUserRequest_Send mcs(stream, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x28"  // AttachUserRequest * 4
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_AttachUserRequest)
{
    size_t length = 1;
    GeneratorTransport t(
        "\x28"  // AttachUserRequest * 4
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::AttachUserRequest_Recv mcs(stream, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_AttachUserRequest), mcs.type);
}

RED_AUTO_TEST_CASE(TestSend_AttachUserConfirm_without_userid)
{
    StaticOutStream<1024> stream;
    size_t length = 2;
    MCS::AttachUserConfirm_Send mcs(stream, MCS::RT_SUCCESSFUL, false, 0, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x2C"  //  AttachUserConfirm * 4
        "\x00"
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_AttachUserConfirm_without_userid)
{
    size_t length = 2;
    GeneratorTransport t(
        "\x2C"  // AttachUserConfirm * 4
        "\x00"
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::AttachUserConfirm_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_AttachUserConfirm), mcs.type);
    RED_CHECK_EQUAL(0 , mcs.result);
    RED_CHECK_EQUAL(false , mcs.initiator_flag);
}

RED_AUTO_TEST_CASE(TestSend_AttachUserConfirm_with_userid)
{
    StaticOutStream<1024> stream;
    size_t length = 4;
    MCS::AttachUserConfirm_Send mcs(stream, MCS::RT_SUCCESSFUL, true, 1, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x2E"  //  AttachUserConfirm * 4
        "\x00"
        "\x00\x01"
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_AttachUserConfirm_with_userid)
{
    size_t length = 4;
    GeneratorTransport t(
        "\x2E"  // AttachUserConfirm * 4
        "\x00"
        "\x00\x01"
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::AttachUserConfirm_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_AttachUserConfirm), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint8_t>(0) , mcs.result);
    RED_CHECK_EQUAL(true , mcs.initiator_flag);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1) , mcs.initiator);
}

RED_AUTO_TEST_CASE(TestSend_ChannelJoinRequest)
{
    StaticOutStream<1024> stream;
    size_t length = 5;
    MCS::ChannelJoinRequest_Send mcs(stream, 3, 1004, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x38"  // ChannelJoinRequest * 4
        "\x00\x03" // userId = 3
        "\x03\xec" // channelId = 1004
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_ChannelJoinRequest)
{
    size_t length = 5;
    GeneratorTransport t(
        "\x38"  // ChannelJoinRequest * 4
        "\x00\x03" // userId = 3
        "\x03\xec" // channelId = 1004
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);

    MCS::ChannelJoinRequest_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_ChannelJoinRequest), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint16_t>(3), mcs.initiator);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004), mcs.channelId);
}

RED_AUTO_TEST_CASE(TestSend_ChannelJoinConfirm)
{
    StaticOutStream<1024> stream;
    size_t length = 8;
    MCS::ChannelJoinConfirm_Send mcs(stream, MCS::RT_SUCCESSFUL, 3, 1004, true, 1004, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x3E"  // ChannelJoinConfirm * 4
        "\x00"  // result RT_SUCCESSFUL
        "\x00\x03" // userId = 3
        "\x03\xec" // requested = 1004
        "\x03\xec" // channelId = 1004
    ;

    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_ChannelJoinConfirm)
{
    size_t length = 8;
    GeneratorTransport t(
        "\x3E"  // ChannelJoinConfirm * 4
        "\x00"  // result RT_SUCCESSFUL
        "\x00\x03" // userId = 3
        "\x03\xec" // requested = 1004
        "\x03\xec" // channelId = 1004
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::ChannelJoinConfirm_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_ChannelJoinConfirm), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::RT_SUCCESSFUL), mcs.result);
    RED_CHECK_EQUAL(static_cast<uint16_t>(3) , mcs.initiator);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004) , mcs.requested);
    RED_CHECK_EQUAL(true , mcs.channelId_flag);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004) , mcs.channelId);
}

RED_AUTO_TEST_CASE(TestSend_SendDataRequest)
{
    StaticOutStream<1024> stream;
    size_t length = 8;
    MCS::SendDataRequest_Send mcs(stream, 3, 1004, 1, 3, 379, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x64"  // SendDataRequest * 4
        "\x00\x03" // userid  = 3
        "\x03\xec" // channel = 1005
        "\x70"     // high priority, segmentation end
        "\x81\x7b" // len 379
    ;
    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_SendDataRequest)
{
    size_t length = 8 + 379;
    GeneratorTransport t(
        "\x64"  // SendDataRequest * 4
        "\x00\x03" // userid  = 3
        "\x03\xec" // channel = 1005
        "\x70"     // high priority, segmentation end
        "\x81\x7b" // len 379

        // 379 bytes
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::SendDataRequest_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_SendDataRequest), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint16_t>(3), mcs.initiator);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004) , mcs.channelId);
    RED_CHECK_EQUAL(static_cast<uint8_t>(1) , mcs.dataPriority);
    RED_CHECK_EQUAL(static_cast<uint8_t>(3) , mcs.segmentation);
    RED_CHECK_EQUAL(static_cast<uint16_t>(379) , mcs.payload.get_capacity());
}

RED_AUTO_TEST_CASE(TestSend_SendDataIndication)
{
    StaticOutStream<1024> stream;
    size_t length = 8;
    MCS::SendDataIndication_Send mcs(stream, 3, 1004, 1, 3, 379, MCS::PER_ENCODING);
    RED_CHECK_EQUAL(length, stream.get_offset());

    const char * expected =
        "\x68"  // SendDataIndication * 4
        "\x00\x03" // userid  = 3
        "\x03\xec" // channel = 1005
        "\x70"     // high priority, segmentation end
        "\x81\x7b" // len 379
    ;
    RED_CHECK_EQUAL(0, memcmp(expected, stream.get_data(), length));
}

RED_AUTO_TEST_CASE(TestRecv_SendDataIndication)
{
    size_t length = 8 + 379;
    GeneratorTransport t(
        "\x68"  // SendDataIndication * 4
        "\x00\x03" // userid  = 3
        "\x03\xec" // channel = 1005
        "\x70"     // high priority, segmentation end
        "\x81\x7b" // len 379

        // 379 bytes
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::SendDataIndication_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_SendDataIndication), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint16_t>(3), mcs.initiator);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004) , mcs.channelId);
    RED_CHECK_EQUAL(static_cast<uint8_t>(1) , mcs.dataPriority);
    RED_CHECK_EQUAL(static_cast<uint8_t>(3) , mcs.segmentation);
    RED_CHECK_EQUAL(static_cast<uint16_t>(379) , mcs.payload.get_capacity());
}


RED_AUTO_TEST_CASE(TestRecv_SendDataIndication2)
{
    size_t length = 8 + 363;
    GeneratorTransport t(
        "\x68"      // SendDataIndication * 4
        "\x00\x01"  // userid  = 1
        "\x03\xec"  // channel = 1004
        "\x70"      // high priority, segmentation end
        "\x81\x6b"  // len 363

        // 363 bytes
                                               "\x00\x00\x10\x00\x67\x01\x11" //.h....p.k....g..
/* 0010 */ "\x00\xea\x03\xea\x03\x01\x00\x04\x00\x51\x01\x52\x44\x50\x00\x0d" //.........Q.RDP..
/* 0020 */ "\x00\x00\x00\x09\x00\x08\x00\xea\x03\xf3\xe2\x01\x00\x18\x00\x01" //................
/* 0030 */ "\x00\x03\x00\x00\x02\x00\x00\x00\x00\x1d\x04\x00\x00\x00\x00\x00" //................
/* 0040 */ "\x00\x01\x01\x14\x00\x08\x00\x02\x00\x00\x00\x16\x00\x28\x00\x01" //.............(..
/* 0050 */ "\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\xa8\xb6\x98\xee\x7c" //...............|
/* 0060 */ "\xc2\x84\x80\x00\x00\x00\x00\x18\xb7\x98\xee\x4c\xce\x84\x80\x00" //...........L....
/* 0070 */ "\x00\x00\x00\x0e\x00\x04\x00\x02\x00\x1c\x00\x10\x00\x01\x00\x01" //................
/* 0080 */ "\x00\x01\x00\x20\x03\x58\x02\x00\x00\x01\x00\x01\x00\x00\x00\x01" //... .X..........
/* 0090 */ "\x00\x00\x00\x03\x00\x58\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.....X..........
/* 00a0 */ "\x00\x00\x00\x00\x00\x00\x00\x40\x42\x0f\x00\x01\x00\x14\x00\x00" //.......@B.......
/* 00b0 */ "\x00\x01\x00\x00\x00\x22\x00\x01\x01\x01\x01\x01\x00\x00\x01\x01" //....."..........
/* 00c0 */ "\x01\x01\x01\x00\x00\x00\x01\x01\x01\x01\x01\x01\x01\x01\x00\x01" //................
/* 00d0 */ "\x01\x01\x01\x00\x00\x00\x00\xa1\x06\x00\x00\x40\x42\x0f\x00\x40" //...........@B..@
/* 00e0 */ "\x42\x0f\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x08\x00\x06" //B...............
/* 00f0 */ "\x00\x00\x00\x12\x00\x08\x00\x01\x00\x00\x00\x08\x00\x0a\x00\x01" //................
/* 0100 */ "\x00\x19\x00\x19\x00\x0d\x00\x58\x00\x35\x00\x00\x00\xa1\x06\x00" //.......X.5......
/* 0110 */ "\x00\x40\x42\x0f\x00\x0c\xb6\x98\xee\x81\xfa\x89\xf4\x00\x20\x17" //.@B........... .
/* 0120 */ "\xe1\x86\xbc\x8a\xf4\x40\xb6\x98\xee\x04\x00\x00\x00\x4c\x64\xf3" //.....@.......Ld.
/* 0130 */ "\xe2\x08\x60\xf3\xe2\x01\x00\x00\x00\x08\x60\xf3\xe2\x00\x00\x00" //..`.......`.....
/* 0140 */ "\x00\x38\xb6\x98\xee\x78\xa5\x8a\xf4\x08\x60\xf3\xe2\x2c\xb6\x98" //.8...x....`..,..
/* 0150 */ "\xee\x00\x00\x00\x00\x08\x00\x0a\x00\x01\x00\x19\x00\x17\x00\x08" //................
/* 0160 */ "\x00\x00\x00\x00\x00\x18\x00\x0b\x00\x00\x00\x00\x00\x00\x00\x00" //................
/* 0170 */ "\x00\x00\x00\x00"                                                 //....
   , length);
    uint8_t buf[1024];
    auto end = buf;
    t.recv_boom(end, length);

    InStream stream(buf, length);
    MCS::SendDataIndication_Recv mcs(stream, MCS::PER_ENCODING);

    RED_CHECK_EQUAL(static_cast<uint8_t>(MCS::MCSPDU_SendDataIndication), mcs.type);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1),                             mcs.initiator);
    RED_CHECK_EQUAL(static_cast<uint16_t>(1004),             mcs.channelId);
    RED_CHECK_EQUAL(static_cast<uint8_t>(1),                 mcs.dataPriority);
    RED_CHECK_EQUAL(static_cast<uint8_t>(3),                 mcs.segmentation);
    RED_CHECK_EQUAL(static_cast<uint16_t>(363),              mcs.payload.get_capacity());
}

RED_AUTO_TEST_CASE(TestRecv_NotImplemented)
{
    StaticInStream<1024> istream;
    StaticOutStream<1024> ostream;
    int const ERR_MCS = static_cast<int>(::ERR_MCS);

    RED_CHECK_EXCEPTION_ERROR_ID(MCS::PlumbDomainIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::PlumbDomainIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeChannelRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeChannelRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeChannelsConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeChannelsConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeTokensRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeTokensRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeTokensRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::MergeTokensConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::PurgeTokensIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::PurgeTokensIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::RejectMCSPDUUltimatum_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::RejectMCSPDUUltimatum_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::DetachUserRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::DetachUserRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::DetachUserIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::DetachUserIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelLeaveRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelLeaveRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelConveneRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelConveneRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelConveneConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelConveneConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelDisbandRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelDisbandRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelDisbandIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelDisbandIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelAdmitRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelAdmitRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelAdmitIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelAdmitIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelExpelRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelExpelRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelExpelIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::ChannelExpelIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::UniformSendDataRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::UniformSendDataRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::UniformSendDataIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::UniformSendDataIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGrabRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGrabRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGrabConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGrabConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenInhibitRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenInhibitRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenInhibitConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenInhibitConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveResponse_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveResponse_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenGiveConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenPleaseRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenPleaseRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenPleaseIndication_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenPleaseIndication_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenReleaseRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenReleaseRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenReleaseConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenReleaseConfirm_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenTestRequest_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenTestRequest_Recv mcs(istream, MCS::PER_ENCODING), ERR_MCS);
    RED_CHECK_EXCEPTION_ERROR_ID(MCS::TokenTestConfirm_Send mcs(ostream, MCS::PER_ENCODING), ERR_MCS);
}

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

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define RED_TEST_MODULE TestX224
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "test_only/transport/test_transport.hpp"
#include "core/RDP/x224.hpp"

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_Correlation_Info)
{
    size_t tpkt_len = 85;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x55\x50\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...UP......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x6a\x62\x62\x65" //e: mstshash=jbbe
/* 0020 */ "\x72\x74\x68\x65\x6c\x69\x6e\x0d\x0a"                             //rthelin..
           "\x01\x08\x08\x00\x0b\x00\x00" //.......
/* 0030 */ "\x00"
               "\x06\x00\x24\x00\x75\xcc\x9f\xac\x96\xa5\x41\x82\xbd\x1c\x2d" //...$.u.....A...-
/* 0040 */ "\x63\x52\xc7\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //cR..............
/* 0050 */ "\x00\x00\x00\x00\x00" //.....
        , tpkt_len);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);

    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(tpkt_len, fac_x224.length);

    X224::CR_TPDU_Recv x224(stream, false, true);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(tpkt_len, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(0x50, x224.tpdu_hdr.LI);

    RED_CHECK_EQUAL("Cookie: mstshash=jbberthelin\x0D\x0A", x224.cookie);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::RDP_NEG_REQ), x224.rdp_neg_type);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CORRELATION_INFO_PRESENT), x224.rdp_neg_flags);
    RED_CHECK_EQUAL(8, x224.rdp_neg_length);
    RED_CHECK_EQUAL(static_cast<uint32_t>(X224::PROTOCOL_TLS | X224::PROTOCOL_HYBRID | X224::PROTOCOL_HYBRID_EX), x224.rdp_neg_requestedProtocols);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestReceive_RecvFactory_Short_TPKT)
{
    GeneratorTransport t("\x03\x00\x00\x02\x06\x10\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory(t, &end, array_size), ERR_X224);
    t.disable_remaining_error();
}

RED_AUTO_TEST_CASE(TestReceive_RecvFactory_Unknown_TPDU)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\x10\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory(t, &end, array_size), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_no_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    X224::CR_TPDU_Recv x224(stream, false);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(11, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(6, x224.tpdu_hdr.LI);
    RED_CHECK_EQUAL(0, strlen(x224.cookie));
    RED_CHECK_EQUAL(0, x224.rdp_neg_type);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_overfull_stream)
{
    GeneratorTransport t("\x03\x00\x00\x0C\x06\xE0\x00\x00\x00\x00\x00\x00", 12);

    // stream is too small to hold received data
    constexpr size_t array_size = 4;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory fac_x224(t, &end, array_size), ERR_X224);
    t.disable_remaining_error();
}

RED_AUTO_TEST_CASE(TestReceive_TPDU_truncated_header)
{
    GeneratorTransport t("\x03\x00\x00\x0A\x06\xE0\x00\x00\x00", 10);

    constexpr size_t array_size = 20;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EXCEPTION_ERROR_ID(X224::CR_TPDU_Recv x224(stream, false), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_Wrong_opcode)
{
    GeneratorTransport t("\x03\x00\x00\x0C\x06\xF0\x00\x00\x00\x00\x00\x00", 12);

    constexpr size_t array_size = 20;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EXCEPTION_ERROR_ID(X224::CR_TPDU_Recv(stream, false), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_truncated_header)
{
    size_t tpkt_len = 55;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x35\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
        , tpkt_len - 2);

    constexpr size_t array_size = 100;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EXCEPTION_ERROR_ID(X224::CR_TPDU_Recv(stream, false), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_NEG_REQ_MISSING)
{
    size_t tpkt_len = 55;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x37\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x00" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
        , tpkt_len);

    constexpr size_t array_size = 100;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EXCEPTION_ERROR_ID(X224::CR_TPDU_Recv(stream, false), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_trailing_data)
{
    size_t tpkt_len = 55;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x37\x30\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
        , tpkt_len);

    constexpr size_t array_size = 100;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EXCEPTION_ERROR_ID(X224::CR_TPDU_Recv(stream, false), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(size_t(11), fac_x224.length);

    X224::CR_TPDU_Recv x224(stream, false);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(11, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(6, x224.tpdu_hdr.LI);
    RED_CHECK_EQUAL(0, strlen(x224.cookie));
    RED_CHECK_EQUAL(0, x224.rdp_neg_type);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestSend_CR_TPDU)
{
    StaticOutStream<256> stream;
    X224::CR_TPDU_Send x224(stream, "", 0, 0, 0);
    RED_CHECK_EQUAL(11, stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0B\x06\xE0\x00\x00\x00\x00\x00", stream.get_data(), 11));
}

RED_AUTO_TEST_CASE(TestReceive_CR_TPDU_with_factory_TLS_Negotiation_packet)
{
    size_t tpkt_len = 55;
    GeneratorTransport t(
/* 0000 */ "\x03\x00\x00\x37\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
        , tpkt_len);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(tpkt_len, fac_x224.length);

    X224::CR_TPDU_Recv x224(stream, false);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(tpkt_len, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CR_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(0x32, x224.tpdu_hdr.LI);

    RED_CHECK_EQUAL("Cookie: mstshash=administrateur@qa\x0D\x0A", x224.cookie);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::RDP_NEG_REQ), x224.rdp_neg_type);
    RED_CHECK_EQUAL(0, x224.rdp_neg_flags);
    RED_CHECK_EQUAL(8, x224.rdp_neg_length);
    RED_CHECK_EQUAL(static_cast<uint32_t>(X224::PROTOCOL_TLS), x224.rdp_neg_requestedProtocols);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}




RED_AUTO_TEST_CASE(TestSend_CR_TPDU_TLS_Negotiation_packet)
{

    StaticOutStream<256> stream;
    X224::CR_TPDU_Send(stream,
            "Cookie: mstshash=administrateur@qa\x0D\x0A",
            X224::RDP_NEG_REQ, 0, X224::PROTOCOL_TLS);
    RED_CHECK_EQUAL(55, stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(
/* 0000 */ "\x03\x00\x00\x37\x32\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...72......Cooki |
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x61\x64\x6d\x69" //e: mstshash=admi |
/* 0020 */ "\x6e\x69\x73\x74\x72\x61\x74\x65\x75\x72\x40\x71\x61\x0d\x0a\x01" //nistrateur@qa... |
/* 0030 */ "\x00\x08\x00\x01\x00\x00\x00"                                     //....... |
    , stream.get_data(), 55));
}


RED_AUTO_TEST_CASE(TestSend_CR_TPDU_TLS_Negotiation_packet_forge)
{
    StaticOutStream<256> stream;
    X224::CR_TPDU_Send(stream, "", X224::RDP_NEG_REQ, 0, X224::PROTOCOL_TLS);
    RED_CHECK_MEM(stream.get_bytes(),
        "\x03\x00\x00\x13\x0e\xe0\x00\x00\x00\x00\x00\x01\x00\x08\x00\x01\x00\x00\x00"_av);
}


RED_AUTO_TEST_CASE(TestReceive_CC_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<int>(X224::CC_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(11u, fac_x224.length);

    X224::CC_TPDU_Recv x224(stream);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(11, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CC_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(6, x224.tpdu_hdr.LI);
    RED_CHECK_EQUAL(0, x224.rdp_neg_type);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestReceive_CC_TPDU_wrong_opcode)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xC0\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory fac_x224(t, &end, array_size), ERR_X224);
}

RED_AUTO_TEST_CASE(TestSend_CC_TPDU)
{
    StaticOutStream<256> stream;
    X224::CC_TPDU_Send x224(stream, 0, 0, 0);
    RED_CHECK_EQUAL(11, stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0B\x06\xD0\x00\x00\x00\x00\x00", stream.get_data(), 11));
}

RED_AUTO_TEST_CASE(TestReceive_CC_TPDU_TLS_with_factory)
{
    size_t tpkt_len = 19;
    GeneratorTransport t("\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00", tpkt_len);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<int>(X224::CC_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(tpkt_len, fac_x224.length);

    X224::CC_TPDU_Recv x224(stream);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(tpkt_len, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::CC_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(0, x224.tpdu_hdr.dst_ref);
    RED_CHECK_EQUAL(0, x224.tpdu_hdr.src_ref);
    RED_CHECK_EQUAL(14, x224.tpdu_hdr.LI);

    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::RDP_NEG_RSP), x224.rdp_neg_type);
    RED_CHECK_EQUAL(0, x224.rdp_neg_flags);
    RED_CHECK_EQUAL(8, x224.rdp_neg_length);
    RED_CHECK_EQUAL(static_cast<uint32_t>(X224::PROTOCOL_TLS), x224.rdp_neg_code);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestSend_CC_TPDU_TLS)
{
    StaticOutStream<256> stream;
    X224::CC_TPDU_Send x224(stream, X224::RDP_NEG_RSP, 0, X224::PROTOCOL_TLS);
    RED_CHECK_EQUAL(19, stream.get_offset());
    RED_CHECK_EQUAL(0,
        memcmp("\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00", stream.get_data(), 19));
}

RED_AUTO_TEST_CASE(TestReceive_DR_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<int>(X224::DR_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(11u, fac_x224.length);

    X224::DR_TPDU_Recv x224(stream);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(11, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::DR_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::REASON_NOT_SPECIFIED), x224.tpdu_hdr.reason);
    RED_CHECK_EQUAL(6, x224.tpdu_hdr.LI);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestReceive_DR_TPDU_wrong_opcode)
{
    GeneratorTransport t("\x03\x00\x00\x0B\x06\xC0\x00\x00\x00\x00\x00", 11);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory fac_x224(t, &end, array_size), ERR_X224);
}

RED_AUTO_TEST_CASE(TestSend_DR_TPDU)
{
    StaticOutStream<256> stream;
    X224::DR_TPDU_Send x224(stream, X224::REASON_NOT_SPECIFIED);
    RED_CHECK_EQUAL(11, stream.get_offset());
    RED_CHECK_EQUAL(0,
        memcmp("\x03\x00\x00\x0B\x06\x80\x00\x00\x00\x00\x00", stream.get_data(), 11));
}

RED_AUTO_TEST_CASE(TestReceive_ER_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", 13);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::ER_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(13u, fac_x224.length);

    X224::ER_TPDU_Recv x224(stream);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(13, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::ER_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(8, x224.tpdu_hdr.LI);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::REASON_INVALID_TPDU_TYPE), x224.tpdu_hdr.reject_cause);
    RED_CHECK_EQUAL(0xC1, x224.tpdu_hdr.invalid_tpdu_var);
    RED_CHECK_EQUAL(2, x224.tpdu_hdr.invalid_tpdu_vl);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(x224._header_size, stream.get_capacity());
}

RED_AUTO_TEST_CASE(TestSend_ER_TPDU)
{
    StaticOutStream<256> stream;
    uint8_t invalid[2] = {0x06, 0x22};
    X224::ER_TPDU_Send x224(stream, X224::REASON_INVALID_TPDU_TYPE, 2, invalid);
    RED_CHECK_EQUAL(13, stream.get_offset());
    RED_CHECK_EQUAL(0,
        memcmp("\x03\x00\x00\x0D\x08\x70\x00\x00\x02\xC1\x02\x06\x22", stream.get_data(), 13));
}


RED_AUTO_TEST_CASE(TestReceive_ER_TPDU_wrong_opcode)
{
    GeneratorTransport t("\x03\x00\x00\x0D\x08\xC0\x00\x00\x02\xC1\x02\x06\x22", 13);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory fac_x224(t, &end, array_size), ERR_X224);
}

RED_AUTO_TEST_CASE(TestReceive_DT_TPDU_with_factory)
{
    GeneratorTransport t("\x03\x00\x00\x0C\x02\xF0\x80\x12\x34\x56\x78\x9A", 12);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    X224::RecvFactory fac_x224(t, &end, array_size);
    InStream stream(array, end - array);
    RED_CHECK_EQUAL(static_cast<int>(X224::DT_TPDU), fac_x224.type);
    RED_CHECK_EQUAL(12u, fac_x224.length);

    X224::DT_TPDU_Recv x224(stream);

    RED_CHECK_EQUAL(3, x224.tpkt.version);
    RED_CHECK_EQUAL(12, x224.tpkt.len);
    RED_CHECK_EQUAL(static_cast<uint8_t>(X224::DT_TPDU), x224.tpdu_hdr.code);
    RED_CHECK_EQUAL(2, x224.tpdu_hdr.LI);

    RED_CHECK_EQUAL(stream.get_capacity(), x224.tpkt.len);
    RED_CHECK_EQUAL(7, x224._header_size);
    RED_CHECK_EQUAL(x224._header_size + x224.payload.get_capacity(), stream.get_capacity());

    RED_CHECK_EQUAL(5, x224.payload.get_capacity());
}

RED_AUTO_TEST_CASE(TestReceive_DT_TPDU_wrong_opcode)
{
    GeneratorTransport t("\x03\x00\x00\x0C\x02\xC0\x80\x12\x34\x56\x78\x9A", 12);

    constexpr size_t array_size = AUTOSIZE;
    uint8_t array[array_size];
    uint8_t * end = array;
    RED_CHECK_EXCEPTION_ERROR_ID(X224::RecvFactory fac_x224(t, &end, array_size), ERR_X224);
}

RED_AUTO_TEST_CASE(TestSend_DT_TPDU)
{
    StaticOutStream<256> payload;
    payload.out_copy_bytes("\x12\x34\x56\x78\x9A", 5);

    size_t payload_len = payload.get_offset();
    StaticOutStream<256> stream;
    X224::DT_TPDU_Send x224(stream, payload_len);

    RED_CHECK_EQUAL(7, stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp("\x03\x00\x00\x0C\x02\xF0\x80", stream.get_data(), 7));
    RED_CHECK_EQUAL(5, payload_len);
    RED_CHECK_EQUAL(0, memcmp("\x12\x34\x56\x78\x9A", payload.get_data(), 5));

    CheckTransport t("\x03\x00\x00\x0C\x02\xF0\x80\x12\x34\x56\x78\x9A", 12);
    t.send(stream.get_bytes());
    t.send(payload.get_data(), payload_len);
}

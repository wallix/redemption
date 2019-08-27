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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/nla/credssp.hpp"

#include "test_only/check_sig.hpp"


RED_AUTO_TEST_CASE(TestBERInteger)
{
    StaticOutStream<2048> s;
    int res;
    uint32_t value;

    res = BER::write_integer(s, 114178754);
    RED_CHECK_EQUAL(res, 6);


    InStream in_s(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK(res);

    RED_CHECK_EQUAL(value, 114178754);

    s.rewind();

    res = BER::write_integer(s, 1);
    RED_CHECK_EQUAL(res, 3);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK(res);

    RED_CHECK_EQUAL(value, 1);

    s.rewind();

    res = BER::write_integer(s, 52165);
    RED_CHECK_EQUAL(res, 5);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK(res);

    RED_CHECK_EQUAL(value, 52165);

    s.rewind();

    res = BER::write_integer(s, 0x0FFF);
    RED_CHECK_EQUAL(res, 4);

    in_s = InStream(s.get_bytes());
    res = BER::read_integer(in_s, value);
    RED_CHECK(res);

    RED_CHECK_EQUAL(value, 0x0FFF);

    s.rewind();
}

//RED_AUTO_TEST_CASE(TestBERBool)
//{
//    BStream s(2048);
//    int res;
//    bool value;
//
//    BER::write_bool(s, true);
//    s.mark_end();
//    s.rewind();
//    res = BER::read_bool(s, value);
//    RED_CHECK(res);
//    RED_CHECK(value);
//
//    s.rewind();
//
//    BER::write_bool(s, false);
//    s.mark_end();
//    s.rewind();
//    res = BER::read_bool(s, value);
//    RED_CHECK(res);
//    RED_CHECK(not value);
//
//    s.rewind();
//
//}

RED_AUTO_TEST_CASE(TestBEROctetString)
{
    StaticOutStream<2048> s;
    int res;
    int value;
    const uint8_t oct_str[] =
        { 1, 2, 3, 4, 5, 6 , 7 };

    BER::write_octet_string(s, oct_str, 7);
    InStream in_s(s.get_bytes());
    res = BER::read_octet_string_tag(in_s, value);
    RED_CHECK(res);
    RED_CHECK_EQUAL(value, 7);

    s.rewind();
}


RED_AUTO_TEST_CASE(TestBERContextual)
{
    StaticOutStream<2048> s;
    int res;
    int value;
    uint8_t tag = 0x06;

    auto v = BER::mkContextualFieldHeader(3, tag);
    s.out_copy_bytes(v);

    InStream in_s(s.get_bytes());
    res = BER::read_contextual_tag(in_s, tag, value, true);
    RED_CHECK(res);
    RED_CHECK_EQUAL(value, 3);
}

RED_AUTO_TEST_CASE(TestTSRequest)
{

    // ===== NTLMSSP_NEGOTIATE =====
    constexpr static uint8_t packet[] = {
        0x30, 0x37, 0xa0, 0x03, 0x02, 0x01, 0x03, 0xa1,
        0x30, 0x30, 0x2e, 0x30, 0x2c, 0xa0, 0x2a, 0x04,
        0x28, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50,
        0x00, 0x01, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08,
        0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x05, 0x01, 0x28, 0x0a, 0x00, 0x00, 0x00,
        0x0f
    };

    InStream in_s(packet);
    uint32_t error_code = 0;
    TSRequest ts_req = recvTSRequest(in_s, error_code, 3);

    RED_CHECK_EQUAL(ts_req.version, 3);

    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x28);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);

    StaticOutStream<65536> to_send;

    auto v = emitTSRequest(ts_req, error_code);
    to_send.out_copy_bytes(v);

    RED_CHECK_EQUAL(to_send.get_offset(), 0x37 + 2);

    RED_CHECK_SIG_FROM(to_send, packet);

    // ===== NTLMSSP_CHALLENGE =====
    constexpr static uint8_t packet2[] = {
        0x30, 0x81, 0x94, 0xa0, 0x03, 0x02, 0x01, 0x03,
        0xa1, 0x81, 0x8c, 0x30, 0x81, 0x89, 0x30, 0x81,
        0x86, 0xa0, 0x81, 0x83, 0x04, 0x81, 0x80, 0x4e,
        0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x38,
        0x00, 0x00, 0x00, 0x35, 0x82, 0x8a, 0xe2, 0x26,
        0x6e, 0xcd, 0x75, 0xaa, 0x41, 0xe7, 0x6f, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
        0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x06,
        0x01, 0xb0, 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x57,
        0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00, 0x02,
        0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e,
        0x00, 0x37, 0x00, 0x01, 0x00, 0x08, 0x00, 0x57,
        0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00, 0x04,
        0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e,
        0x00, 0x37, 0x00, 0x03, 0x00, 0x08, 0x00, 0x77,
        0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x07,
        0x00, 0x08, 0x00, 0xa9, 0x8d, 0x9b, 0x1a, 0x6c,
        0xb0, 0xcb, 0x01, 0x00, 0x00, 0x00, 0x00
    };

    in_s = InStream(packet2);
    uint32_t error_code2 = 0;
    TSRequest ts_req2 = recvTSRequest(in_s, error_code2, 3);

    RED_CHECK_EQUAL(ts_req2.version, 3);

    RED_CHECK_EQUAL(ts_req2.negoTokens.size(), 0x80);
    RED_CHECK_EQUAL(ts_req2.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req2.pubKeyAuth.size(), 0);

    StaticOutStream<65536> to_send2;

    auto v2 = emitTSRequest(ts_req2, error_code2);
    to_send2.out_copy_bytes(v2);

    RED_CHECK_EQUAL(to_send2.get_offset(), 0x94 + 3);

    RED_CHECK_SIG_FROM(to_send2, packet2);

    // ===== NTLMSSP_AUTH =====
    constexpr static uint8_t packet3[] = {
        0x30, 0x82, 0x02, 0x41, 0xa0, 0x03, 0x02, 0x01,
        0x03, 0xa1, 0x82, 0x01, 0x12, 0x30, 0x82, 0x01,
        0x0e, 0x30, 0x82, 0x01, 0x0a, 0xa0, 0x82, 0x01,
        0x06, 0x04, 0x82, 0x01, 0x02, 0x4e, 0x54, 0x4c,
        0x4d, 0x53, 0x53, 0x50, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x18, 0x00, 0x18, 0x00, 0x6a, 0x00, 0x00,
        0x00, 0x70, 0x00, 0x70, 0x00, 0x82, 0x00, 0x00,
        0x00, 0x08, 0x00, 0x08, 0x00, 0x48, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x10, 0x00, 0x50, 0x00, 0x00,
        0x00, 0x0a, 0x00, 0x0a, 0x00, 0x60, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x10, 0x00, 0xf2, 0x00, 0x00,
        0x00, 0x35, 0x82, 0x88, 0xe2, 0x05, 0x01, 0x28,
        0x0a, 0x00, 0x00, 0x00, 0x0f, 0x77, 0x00, 0x69,
        0x00, 0x6e, 0x00, 0x37, 0x00, 0x75, 0x00, 0x73,
        0x00, 0x65, 0x00, 0x72, 0x00, 0x6e, 0x00, 0x61,
        0x00, 0x6d, 0x00, 0x65, 0x00, 0x57, 0x00, 0x49,
        0x00, 0x4e, 0x00, 0x58, 0x00, 0x50, 0x00, 0xa0,
        0x98, 0x01, 0x10, 0x19, 0xbb, 0x5d, 0x00, 0xf6,
        0xbe, 0x00, 0x33, 0x90, 0x20, 0x34, 0xb3, 0x47,
        0xa2, 0xe5, 0xcf, 0x27, 0xf7, 0x3c, 0x43, 0x01,
        0x4a, 0xd0, 0x8c, 0x24, 0xb4, 0x90, 0x74, 0x39,
        0x68, 0xe8, 0xbd, 0x0d, 0x2b, 0x70, 0x10, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3,
        0x83, 0xa2, 0x1c, 0x6c, 0xb0, 0xcb, 0x01, 0x47,
        0xa2, 0xe5, 0xcf, 0x27, 0xf7, 0x3c, 0x43, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x57,
        0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00, 0x01,
        0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e,
        0x00, 0x37, 0x00, 0x04, 0x00, 0x08, 0x00, 0x77,
        0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x03,
        0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e,
        0x00, 0x37, 0x00, 0x07, 0x00, 0x08, 0x00, 0xa9,
        0x8d, 0x9b, 0x1a, 0x6c, 0xb0, 0xcb, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb1,
        0xd2, 0x45, 0x42, 0x0f, 0x37, 0x9a, 0x0e, 0xe0,
        0xce, 0x77, 0x40, 0x10, 0x8a, 0xda, 0xba, 0xa3,
        0x82, 0x01, 0x22, 0x04, 0x82, 0x01, 0x1e, 0x01,
        0x00, 0x00, 0x00, 0x91, 0x5e, 0xb0, 0x6e, 0x72,
        0x82, 0x53, 0xae, 0x00, 0x00, 0x00, 0x00, 0x27,
        0x29, 0x73, 0xa9, 0xfa, 0x46, 0x17, 0x3c, 0x74,
        0x14, 0x45, 0x2a, 0xd1, 0xe2, 0x92, 0xa1, 0xc6,
        0x0a, 0x30, 0xd4, 0xcc, 0xe0, 0x92, 0xf6, 0xb3,
        0x20, 0xb3, 0xa0, 0xf1, 0x38, 0xb1, 0xf4, 0xe5,
        0x96, 0xdf, 0xa1, 0x65, 0x5b, 0xd6, 0x0c, 0x2a,
        0x86, 0x99, 0xcc, 0x72, 0x80, 0xbd, 0xe9, 0x19,
        0x1f, 0x42, 0x53, 0xf6, 0x84, 0xa3, 0xda, 0x0e,
        0xec, 0x10, 0x29, 0x15, 0x52, 0x5c, 0x77, 0x40,
        0xc8, 0x3d, 0x44, 0x01, 0x34, 0xb6, 0x0a, 0x75,
        0x33, 0xc0, 0x25, 0x71, 0xd3, 0x25, 0x38, 0x3b,
        0xfc, 0x3b, 0xa8, 0xcf, 0xba, 0x2b, 0xf6, 0x99,
        0x0e, 0x5f, 0x4e, 0xa9, 0x16, 0x2b, 0x52, 0x9f,
        0xbb, 0x76, 0xf8, 0x03, 0xfc, 0x11, 0x5e, 0x36,
        0x83, 0xd8, 0x4c, 0x9a, 0xdc, 0x9d, 0x35, 0xe2,
        0xc8, 0x63, 0xa9, 0x3d, 0x07, 0x97, 0x52, 0x64,
        0x54, 0x72, 0x9e, 0x9a, 0x8c, 0x56, 0x79, 0x4a,
        0x78, 0x91, 0x0a, 0x4c, 0x52, 0x84, 0x5a, 0x4a,
        0xb8, 0x28, 0x0b, 0x2f, 0xe6, 0x89, 0x7d, 0x07,
        0x3b, 0x7b, 0x6e, 0x22, 0xcc, 0x4c, 0xff, 0xf4,
        0x10, 0x96, 0xf2, 0x27, 0x29, 0xa0, 0x76, 0x0d,
        0x4c, 0x7e, 0x7a, 0x42, 0xe4, 0x1e, 0x6a, 0x95,
        0x7d, 0x4c, 0xaf, 0xdb, 0x86, 0x49, 0x5c, 0xbf,
        0xc2, 0x65, 0xb6, 0xf2, 0xed, 0xae, 0x8d, 0x57,
        0xed, 0xf0, 0xd4, 0xcb, 0x7a, 0xbb, 0x23, 0xde,
        0xe3, 0x43, 0xea, 0xb1, 0x02, 0xe3, 0xb4, 0x96,
        0xe9, 0xe7, 0x48, 0x69, 0xb0, 0xaa, 0xec, 0x89,
        0x38, 0x8b, 0xc2, 0xbd, 0xdd, 0xf7, 0xdf, 0xa1,
        0x37, 0xe7, 0x34, 0x72, 0x7f, 0x91, 0x10, 0x14,
        0x73, 0xfe, 0x32, 0xdc, 0xfe, 0x68, 0x2b, 0xc0,
        0x08, 0xdf, 0x05, 0xf7, 0xbd, 0x46, 0x33, 0xfb,
        0xc9, 0xfc, 0x89, 0xaa, 0x5d, 0x25, 0x49, 0xc8,
        0x6e, 0x86, 0xee, 0xc2, 0xce, 0xc4, 0x8e, 0x85,
        0x9f, 0xe8, 0x30, 0xb3, 0x86, 0x11, 0xd5, 0xb8,
        0x34, 0x4a, 0xe0, 0x03, 0xe5
    };

    in_s = InStream(packet3);
    uint32_t error_code3 = 0;
    TSRequest ts_req3 = recvTSRequest(in_s, error_code3, 3);

    RED_CHECK_EQUAL(ts_req3.version, 3);

    RED_CHECK_EQUAL(ts_req3.negoTokens.size(), 0x102);
    RED_CHECK_EQUAL(ts_req3.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req3.pubKeyAuth.size(), 0x11e);

    StaticOutStream<65536> to_send3;

    auto v3 = emitTSRequest(ts_req3, error_code3);
    to_send3.out_copy_bytes(v3);


    RED_CHECK_EQUAL(to_send3.get_offset(), 0x241 + 4);

    RED_CHECK_SIG_FROM(to_send3, packet3);

    // ===== PUBKEYAUTH =====
    constexpr static uint8_t packet4[] = {
        0x30, 0x82, 0x01, 0x2b, 0xa0, 0x03, 0x02, 0x01,
        0x03, 0xa3, 0x82, 0x01, 0x22, 0x04, 0x82, 0x01,
        0x1e, 0x01, 0x00, 0x00, 0x00, 0xc9, 0x88, 0xfc,
        0xf1, 0x11, 0x68, 0x2c, 0x72, 0x00, 0x00, 0x00,
        0x00, 0xc7, 0x51, 0xf4, 0x71, 0xd3, 0x9f, 0xb6,
        0x50, 0xbe, 0xa8, 0xf6, 0x20, 0x77, 0xa1, 0xfc,
        0xdd, 0x8e, 0x02, 0xf0, 0xa4, 0x6b, 0xba, 0x3f,
        0x9d, 0x65, 0x9d, 0xab, 0x4a, 0x95, 0xc9, 0xb4,
        0x38, 0x03, 0x87, 0x04, 0xb1, 0xfe, 0x42, 0xec,
        0xfa, 0xfc, 0xaa, 0x85, 0xf1, 0x31, 0x2d, 0x26,
        0xcf, 0x63, 0xfd, 0x62, 0x36, 0xcf, 0x56, 0xc3,
        0xfb, 0xf6, 0x36, 0x9b, 0xe5, 0xb2, 0xe7, 0xce,
        0xcb, 0xe1, 0x82, 0xb2, 0x89, 0xff, 0xdd, 0x87,
        0x5e, 0xd3, 0xd8, 0xff, 0x2e, 0x16, 0x35, 0xad,
        0xdb, 0xda, 0xc9, 0xc5, 0x81, 0xad, 0x48, 0xf1,
        0x8b, 0x76, 0x3d, 0x74, 0x34, 0xdf, 0x80, 0x6b,
        0xf3, 0x68, 0x6d, 0xf6, 0xec, 0x5f, 0xbe, 0xea,
        0xb7, 0x6c, 0xea, 0xe4, 0xeb, 0xe9, 0x17, 0xf9,
        0x4e, 0x0d, 0x79, 0xd5, 0x82, 0xdd, 0xb7, 0xdc,
        0xcd, 0xfc, 0xbb, 0xf1, 0x0b, 0x9b, 0xe9, 0x18,
        0xe7, 0xb3, 0xb3, 0x8b, 0x40, 0x82, 0xa0, 0x9d,
        0x58, 0x73, 0xda, 0x54, 0xa2, 0x2b, 0xd2, 0xb6,
        0x41, 0x60, 0x8a, 0x64, 0xf2, 0xa2, 0x59, 0x64,
        0xcf, 0x27, 0x1a, 0xe6, 0xb5, 0x1a, 0x0e, 0x0e,
        0xe1, 0x14, 0xef, 0x26, 0x68, 0xeb, 0xc8, 0x49,
        0xe2, 0x66, 0xbb, 0x11, 0x71, 0x49, 0xad, 0x7e,
        0xae, 0xde, 0xa8, 0x78, 0xfd, 0x64, 0x51, 0xd8,
        0x18, 0x01, 0x11, 0xc0, 0x8d, 0x3b, 0xec, 0x40,
        0x2b, 0x1f, 0xc5, 0xa4, 0x45, 0x1e, 0x07, 0xae,
        0x5a, 0xd8, 0x1c, 0xab, 0xdf, 0x89, 0x96, 0xdc,
        0xdc, 0x29, 0xd8, 0x30, 0xdb, 0xbf, 0x48, 0x2a,
        0x42, 0x27, 0xc2, 0x50, 0xac, 0xf9, 0x02, 0xd1,
        0x20, 0x12, 0xdd, 0x50, 0x22, 0x09, 0x44, 0xac,
        0xe0, 0x22, 0x1f, 0x66, 0x64, 0xec, 0xfa, 0x2b,
        0xb8, 0xcd, 0x43, 0x3a, 0xce, 0x40, 0x74, 0xe1,
        0x34, 0x81, 0xe3, 0x94, 0x47, 0x6f, 0x49, 0x01,
        0xf8, 0xb5, 0xfc, 0xd0, 0x75, 0x80, 0xc6, 0x35,
        0xac, 0xc0, 0xfd, 0x1b, 0xb5, 0xa2, 0xd3
    };

    in_s = InStream(packet4);
    uint32_t error_code4 = 0;
    TSRequest ts_req4 = recvTSRequest(in_s, error_code4, 3);

    RED_CHECK_EQUAL(ts_req4.version, 3);

    RED_CHECK_EQUAL(ts_req4.negoTokens.size(), 0);
    RED_CHECK_EQUAL(ts_req4.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req4.pubKeyAuth.size(), 0x11e);

    StaticOutStream<65536> to_send4;

    auto v4 = emitTSRequest(ts_req4, error_code4);
    to_send4.out_copy_bytes(v4);

    RED_CHECK_EQUAL(to_send4.get_offset(), 0x12b + 4);

    RED_CHECK_SIG_FROM(to_send4, packet4);

    // ===== AUTHINFO =====
    constexpr static uint8_t packet5[] = {
        0x30, 0x5a, 0xa0, 0x03, 0x02, 0x01, 0x03, 0xa2,
        0x53, 0x04, 0x51, 0x01, 0x00, 0x00, 0x00, 0xb3,
        0x2c, 0x3b, 0xa1, 0x36, 0xf6, 0x55, 0x71, 0x01,
        0x00, 0x00, 0x00, 0xa8, 0x85, 0x7d, 0x11, 0xef,
        0x92, 0xa0, 0xd6, 0xff, 0xee, 0xa1, 0xae, 0x6d,
        0xc5, 0x2e, 0x4e, 0x65, 0x50, 0x28, 0x93, 0x75,
        0x30, 0xe1, 0xc3, 0x37, 0xeb, 0xac, 0x1f, 0xdd,
        0xf3, 0xe0, 0x92, 0xf6, 0x21, 0xbc, 0x8f, 0xa8,
        0xd4, 0xe0, 0x5a, 0xa6, 0xff, 0xda, 0x09, 0x50,
        0x24, 0x0d, 0x8f, 0x8f, 0xf4, 0x92, 0xfe, 0x49,
        0x2a, 0x13, 0x52, 0xa6, 0x52, 0x75, 0x50, 0x8d,
        0x3e, 0xe9, 0x6b, 0x57
    };
//    "\x30\x08\xa0\x03\x02\x01\x03\xa2\x53\x04\x51\x01"
//    "\x00\x00\x00\xb3\x2c\x3b\xa1\x36\xf6\x55\x71\x01"
//    "\x00\x00\x00\xa8\x85\x7d\x11\xef\x92\xa0\xd6\xff"
//    "\xee\xa1\xae\x6d\xc5\x2e\x4e\x65\x50\x28\x93\x75"
//    "\x30\xe1\xc3\x37\xeb\xac\x1f\xdd\xf3\xe0\x92\xf6"
//    "\x21\xbc\x8f\xa8\xd4\xe0\x5a\xa6\xff\xda\x09\x50"
//    "\x24\x0d\x8f\x8f\xf4\x92\xfe\x49\x2a\x13\x52\xa6"
//    "\x52\x75\x50\x8d\x3e\xe9\x6b\x57"
//
//    "\x30\x5a\xa0\x03\x02\x01\x03\xa2\x53\x04\x51\x01"
//    "\x00\x00\x00\xb3\x2c\x3b\xa1\x36\xf6\x55\x71\x01"
//    "\x00\x00\x00\xa8\x85\x7d\x11\xef\x92\xa0\xd6\xff"
//    "\xee\xa1\xae\x6d\xc5\x2e\x4e\x65\x50\x28\x93\x75"
//    "\x30\xe1\xc3\x37\xeb\xac\x1f\xdd\xf3\xe0\x92\xf6"
//    "\x21\xbc\x8f\xa8\xd4\xe0\x5a\xa6\xff\xda\x09\x50"
//    "\x24\x0d\x8f\x8f\xf4\x92\xfe\x49\x2a\x13\x52\xa6"
//    "\x52\x75\x50\x8d\x3e\xe9\x6b\x57"

    in_s = InStream(packet5);
    uint32_t error_code5 = 0;

    TSRequest ts_req5 = recvTSRequest(in_s, error_code5, 3);

    RED_CHECK_EQUAL(ts_req5.version, 3);

    RED_CHECK_EQUAL(ts_req5.negoTokens.size(), 0);
    RED_CHECK_EQUAL(ts_req5.authInfo.size(), 0x51);
    RED_CHECK_EQUAL(ts_req5.pubKeyAuth.size(), 0);

    StaticOutStream<65536> to_send5;

    auto v5 = emitTSRequest(ts_req5, error_code5);
    to_send5.out_copy_bytes(v5);

    RED_CHECK_EQUAL(to_send5.get_offset(), 0x5c);

    RED_CHECK_SIG_FROM(to_send5, packet5);

    // RED_CHECK_MEM(to_send5.get_bytes(), packet5);
}


RED_AUTO_TEST_CASE(TestTSCredentialsPassword)
{

    std::vector<uint8_t> domain = {'f', 'l', 'a', 't', 'l', 'a', 'n', 'd', 0};
    std::vector<uint8_t> user = { 's', 'q', 'u', 'a', 'r', 'e', 0};
    std::vector<uint8_t> pass = {'h', 'y', 'p', 'e', 'r', 'c', 'u', 'b', 'e', 0};


    TSCredentials ts_cred(domain, user, pass);

    StaticOutStream<65536> s;

    int emited = emitTSCredentials(s, ts_cred);
    RED_CHECK_EQUAL(s.get_offset(), *(s.get_data() + 1) + 2);
    RED_CHECK_EQUAL(s.get_offset(), emited);

    TSCredentials ts_cred_received;

    InStream in_s(s.get_bytes());
    ts_cred_received.recv(in_s);

    RED_CHECK_EQUAL(ts_cred_received.credType, 1);
    RED_CHECK_EQUAL(ts_cred_received.passCreds.domainName, domain);
    RED_CHECK_EQUAL(ts_cred_received.passCreds.userName, user);
    RED_CHECK_EQUAL(ts_cred_received.passCreds.password, pass);


    std::vector<uint8_t> domain2 = {'s','o','m','e','w','h','e','r','e', 0};
    std::vector<uint8_t> user2 = {'s', 'o', 'm', 'e', 'o', 'n', 'e', 0};
    std::vector<uint8_t> pass2 = {'s', 'o', 'm', 'e', 'p', 'a', 's', 's', 0};

    ts_cred.set_credentials_from_av(domain2, user2, pass2);
    RED_CHECK_EQUAL(ts_cred.passCreds.domainName, domain2);
    RED_CHECK_EQUAL(ts_cred.passCreds.userName,user2);
    RED_CHECK_EQUAL(ts_cred.passCreds.password,pass2);
}

RED_AUTO_TEST_CASE(TestTSCredentialsSmartCard)
{

    std::vector<uint8_t> pin = {'3', '6', '1', '5', 0};
    std::vector<uint8_t> userHint = {'a', 'k', 'a', 0};
    std::vector<uint8_t> domainHint = {'g', 'r', 'a', 'n', 'd', 'p', 'a', 'r', 'c', 0};

    uint8_t cardName[] = "passepartout";
    uint8_t readerName[] = "acrobat";
    uint8_t containerName[] = "docker";
    uint8_t cspName[] = "what";
    uint32_t keySpec = 32;

    TSCredentials ts_cred(pin, userHint, domainHint,
                          keySpec, 
                          {cardName, sizeof(cardName)},
                          {readerName, sizeof(readerName)},
                          {containerName, sizeof(containerName)},
                          {cspName, sizeof(cspName)});

    StaticOutStream<65536> s;

    int emited = emitTSCredentials(s, ts_cred);
    RED_CHECK_EQUAL(s.get_offset(), *(s.get_data() + 1) + 2);
    RED_CHECK_EQUAL(s.get_offset(), emited);

    TSCredentials ts_cred_received;

    InStream in_s(s.get_bytes());
    ts_cred_received.recv(in_s);

    RED_CHECK_EQUAL(ts_cred_received.credType, 2);
    RED_CHECK_EQUAL(ts_cred_received.smartcardCreds.pin, pin);
    RED_CHECK_EQUAL(ts_cred_received.smartcardCreds.userHint, userHint);
    RED_CHECK_EQUAL(ts_cred_received.smartcardCreds.domainHint, domainHint);
    RED_CHECK_EQUAL(ts_cred_received.smartcardCreds.cspData.keySpec, keySpec);
    
    RED_CHECK_SMEM(ts_cred_received.smartcardCreds.cspData.cardName, bytes_view({cardName, sizeof(cardName)}));
    RED_CHECK_SMEM(ts_cred_received.smartcardCreds.cspData.readerName, bytes_view({readerName, sizeof(readerName)}));
    RED_CHECK_SMEM(ts_cred_received.smartcardCreds.cspData.containerName, bytes_view({containerName, sizeof(containerName)}));
    RED_CHECK_SMEM(ts_cred_received.smartcardCreds.cspData.cspName, bytes_view({cspName, sizeof(cspName)}));
}

// Traffic Captured on WIN2012

//TSRequest hexdump ---------------------------------
//TSRequest ts_request_header -----------------------
///* 0000 */ "\x30\x37"                                                         // 07
//TSRequest version_field ---------------------------
///* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
//TSRequest nego_tokens_header ----------------------
///* 0000 */ "\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04\x28"                         // .00.0,.*.(
//TSRequest auth_info_header ------------------------
//TSRequest pub_key_auth_header ---------------------
//TSRequest nonce -----------------------------------
//TSRequest full dump--------------------------------
///* 0000 */ "\x30\x37\xa0\x03\x02\x01\x06\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" // 07......00.0,.*.
///* 0010 */ "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" // (NTLMSSP........
///* 0020 */ "\xe2\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00" // .....(.......(..
///* 0030 */ "\x00\x06\x01\xb1\x1d\x00\x00\x00\x0f"                             // .........
//TSRequest hexdump -DONE----------------------------

//TSRequest hexdump ---------------------------------
//TSRequest ts_request_header -----------------------
///* 0000 */ "\x30\x82\x02\x07"                                                 // 0...
//TSRequest version_field ---------------------------
///* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
//TSRequest nego_tokens_header ----------------------
///* 0000 */ "\xa1\x82\x01\xa6\x30\x82\x01\xa2\x30\x82\x01\x9e\xa0\x82\x01\x9a" // ....0...0.......
///* 0010 */ "\x04\x82\x01\x96"                                                 // ....
//TSRequest auth_info_header ------------------------
//TSRequest pub_key_auth_header ---------------------
///* 0000 */ "\xa3\x32\x04\x30"                                                 // .2.0
//TSRequest nonce -----------------------------------
///* 0000 */ "\xa5\x22\x04\x20"                                                 // .". 
//TSRequest full dump--------------------------------
///* 0000 */ "\x30\x82\x02\x07\xa0\x03\x02\x01\x06\xa1\x82\x01\xa6\x30\x82\x01" // 0............0..
///* 0010 */ "\xa2\x30\x82\x01\x9e\xa0\x82\x01\x9a\x04\x82\x01\x96\x4e\x54\x4c" // .0...........NTL
///* 0020 */ "\x4d\x53\x53\x50\x00\x03\x00\x00\x00\x18\x00\x18\x00\x58\x00\x00" // MSSP.........X..
///* 0030 */ "\x00\xd6\x00\xd6\x00\x70\x00\x00\x00\x00\x00\x00\x00\x46\x01\x00" // .....p.......F..
///* 0040 */ "\x00\x34\x00\x34\x00\x46\x01\x00\x00\x0c\x00\x0c\x00\x7a\x01\x00" // .4.4.F.......z..
///* 0050 */ "\x00\x10\x00\x10\x00\x86\x01\x00\x00\x35\xa2\x88\xe2\x06\x01\xb1" // .........5......
///* 0060 */ "\x1d\x00\x00\x00\x0f\x92\x73\x54\x02\xb9\x9b\x85\xde\xf4\x29\xbe" // ......sT......).
///* 0070 */ "\xf6\x80\x44\x50\x87\xcc\x86\x71\x15\xf8\x03\xd9\x5c\x7a\x9d\x92" // ..DP...q.....z..
///* 0080 */ "\x1a\x79\x07\xf8\x05\xfb\x3c\x66\xcb\x13\xb8\xa8\x2a\xc1\x19\x36" // .y....<f....*..6
///* 0090 */ "\x1b\x64\x98\x63\x4a\xf9\xb7\x15\x97\xf8\xb4\x9a\x6c\x01\x01\x00" // .d.cJ.......l...
///* 00a0 */ "\x00\x00\x00\x00\x00\x4d\x8c\x05\x00\xda\xf3\x53\x5d\xfb\x3c\x66" // .....M.....S].<f
///* 00b0 */ "\xcb\x13\xb8\xa8\x2a\x00\x00\x00\x00\x02\x00\x10\x00\x50\x00\x52" // ....*........P.R
///* 00c0 */ "\x00\x4f\x00\x58\x00\x59\x00\x4b\x00\x44\x00\x43\x00\x01\x00\x14" // .O.X.Y.K.D.C....
///* 00d0 */ "\x00\x57\x00\x49\x00\x4e\x00\x4b\x00\x44\x00\x43\x00\x32\x00\x30" // .W.I.N.K.D.C.2.0
///* 00e0 */ "\x00\x31\x00\x32\x00\x04\x00\x18\x00\x70\x00\x72\x00\x6f\x00\x78" // .1.2.....p.r.o.x
///* 00f0 */ "\x00\x79\x00\x6b\x00\x64\x00\x63\x00\x2e\x00\x6c\x00\x61\x00\x62" // .y.k.d.c...l.a.b
///* 0100 */ "\x00\x03\x00\x2e\x00\x57\x00\x49\x00\x4e\x00\x4b\x00\x44\x00\x43" // .....W.I.N.K.D.C
///* 0110 */ "\x00\x32\x00\x30\x00\x31\x00\x32\x00\x2e\x00\x70\x00\x72\x00\x6f" // .2.0.1.2...p.r.o
///* 0120 */ "\x00\x78\x00\x79\x00\x6b\x00\x64\x00\x63\x00\x2e\x00\x6c\x00\x61" // .x.y.k.d.c...l.a
///* 0130 */ "\x00\x62\x00\x05\x00\x18\x00\x70\x00\x72\x00\x6f\x00\x78\x00\x79" // .b.....p.r.o.x.y
///* 0140 */ "\x00\x6b\x00\x64\x00\x63\x00\x2e\x00\x6c\x00\x61\x00\x62\x00\x07" // .k.d.c...l.a.b..
///* 0150 */ "\x00\x08\x00\x73\xd2\x2d\x78\x95\x52\xd5\x01\x00\x00\x00\x00\x00" // ...s.-x.R.......
///* 0160 */ "\x00\x00\x00\x41\x00\x64\x00\x6d\x00\x69\x00\x6e\x00\x69\x00\x73" // ...A.d.m.i.n.i.s
///* 0170 */ "\x00\x74\x00\x72\x00\x61\x00\x74\x00\x6f\x00\x72\x00\x40\x00\x70" // .t.r.a.t.o.r.@.p
///* 0180 */ "\x00\x72\x00\x6f\x00\x78\x00\x79\x00\x6b\x00\x64\x00\x63\x00\x2e" // .r.o.x.y.k.d.c..
///* 0190 */ "\x00\x6c\x00\x61\x00\x62\x00\x63\x00\x67\x00\x72\x00\x74\x00\x68" // .l.a.b.c.g.r.t.h
///* 01a0 */ "\x00\x63\x00\xdb\x08\x14\xb1\x24\x15\x4c\xc8\xfc\x6e\x86\xc4\x61" // .c.....$.L..n..a
///* 01b0 */ "\x59\xa0\xba\xa3\x32\x04\x30\x01\x00\x00\x00\x12\xd4\x49\x10\x49" // Y...2.0......I.I
///* 01c0 */ "\x9d\xcc\xe8\x00\x00\x00\x00\xe3\x21\xd7\x43\x36\xf2\xb3\x4a\xe2" // ........!.C6..J.
///* 01d0 */ "\x89\x74\xc2\x79\x6b\x6f\x09\xd6\xd6\x33\x56\x3c\x87\x8d\x01\x26" // .t.yko...3V<...&
///* 01e0 */ "\x7e\xf0\x97\x0c\xa4\xfa\x6a\xa5\x22\x04\x20\x6e\xdd\x7c\xd5\xa1" // ~.....j.". n.|..
///* 01f0 */ "\x98\x18\xb8\x90\xec\x89\x2b\xc4\x6f\xde\x8b\x36\x85\xea\xee\x8f" // ......+.o..6....
///* 0200 */ "\x20\xb8\x58\x2b\x3e\xd7\x31\x36\x9a\x15\x1c"                     //  .X+>.16...
//TSRequest hexdump -DONE----------------------------

//TSRequest hexdump ---------------------------------
//TSRequest ts_request_header -----------------------
///* 0000 */ "\x30\x81\xa4"                                                     // 0..
//TSRequest version_field ---------------------------
///* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
//TSRequest nego_tokens_header ----------------------
//TSRequest auth_info_header ------------------------
///* 0000 */ "\xa2\x79\x04\x77"                                                 // .y.w
//TSRequest pub_key_auth_header ---------------------
//TSRequest nonce -----------------------------------
///* 0000 */ "\xa5\x22\x04\x20"                                                 // .". 
//TSRequest full dump--------------------------------
///* 0000 */ "\x30\x81\xa4\xa0\x03\x02\x01\x06\xa2\x79\x04\x77\x01\x00\x00\x00" // 0........y.w....
///* 0010 */ "\x84\x63\xdd\x0b\x27\x86\x08\x74\x01\x00\x00\x00\xc8\x39\xfc\x78" // .c..'..t.....9.x
///* 0020 */ "\xb8\x2f\x6c\x16\xcf\xce\x52\x24\x67\x2d\x50\x68\xcd\x4c\x4d\x3b" // ./l...R$g-Ph.LM;
///* 0030 */ "\xf1\x83\x97\x89\x14\x4b\xd4\x2c\xe7\x28\xe9\xf7\x95\xbf\x1a\x80" // .....K.,.(......
///* 0040 */ "\xde\x56\x7a\x50\x5b\xdc\xcd\xc5\x37\xae\xcf\x2c\x89\x9b\x02\xea" // .VzP[...7..,....
///* 0050 */ "\x95\xa0\x66\x07\x83\x39\xd7\xae\x04\x5e\x8b\x70\xc1\xf3\x0c\xdb" // ..f..9...^.p....
///* 0060 */ "\xba\x61\x89\xe1\x14\xb9\xeb\xb8\x3f\x54\x8e\x9e\xfe\xe4\x0d\x9d" // .a......?T......
///* 0070 */ "\x58\x02\xf5\xb2\x83\xe0\xe9\x22\xa0\x5b\x3d\x1c\x2f\xaa\x69\x31" // X......".[=./.i1
///* 0080 */ "\x30\x87\x7f\xa5\x22\x04\x20\x6e\xdd\x7c\xd5\xa1\x98\x18\xb8\x90" // 0...". n.|......
///* 0090 */ "\xec\x89\x2b\xc4\x6f\xde\x8b\x36\x85\xea\xee\x8f\x20\xb8\x58\x2b" // ..+.o..6.... .X+
///* 00a0 */ "\x3e\xd7\x31\x36\x9a\x15\x1c"                                     // >.16...
//TSRequest hexdump -DONE----------------------------

// Traffic captured on Win 2008

// TSRequest hexdump ---------------------------------
// TSRequest ts_request_header -----------------------
// /* 0000 */ "\x30\x37"                                                         // 07
// TSRequest version_field ---------------------------
// /* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
// TSRequest nego_tokens_header ----------------------
// /* 0000 */ "\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04\x28"                         // .00.0,.*.(
// TSRequest auth_info_header ------------------------
// TSRequest pub_key_auth_header ---------------------
// TSRequest nonce -----------------------------------
// TSRequest full dump--------------------------------
// /* 0000 */ "\x30\x37\xa0\x03\x02\x01\x06\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" // 07......00.0,.*.
// /* 0010 */ "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" // (NTLMSSP........
// /* 0020 */ "\xe2\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00" // .....(.......(..
// /* 0030 */ "\x00\x06\x01\xb1\x1d\x00\x00\x00\x0f"                             // .........
// TSRequest hexdump -DONE----------------------------

//dpproxy: INFO (18423/18423) -- TSRequest hexdump ---------------------------------
// TSRequest ts_request_header -----------------------
// /* 0000 */ "\x30\x82\x01\xd9"                                                 // 0...
// TSRequest version_field ---------------------------
// /* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
// TSRequest nego_tokens_header ----------------------
// /* 0000 */ "\xa1\x82\x01\x78\x30\x82\x01\x74\x30\x82\x01\x70\xa0\x82\x01\x6c" // ...x0..t0..p...l
// /* 0010 */ "\x04\x82\x01\x68"                                                 // ...h
// TSRequest auth_info_header ------------------------
// TSRequest pub_key_auth_header ---------------------
// /* 0000 */ "\xa3\x32\x04\x30"                                                 // .2.0
// TSRequest nonce -----------------------------------
// /* 0000 */ "\xa5\x22\x04\x20"                                                 // .". 
// TSRequest full dump--------------------------------
// /* 0000 */ "\x30\x82\x01\xd9\xa0\x03\x02\x01\x06\xa1\x82\x01\x78\x30\x82\x01" // 0...........x0..
// /* 0010 */ "\x74\x30\x82\x01\x70\xa0\x82\x01\x6c\x04\x82\x01\x68\x4e\x54\x4c" // t0..p...l...hNTL
// /* 0020 */ "\x4d\x53\x53\x50\x00\x03\x00\x00\x00\x18\x00\x18\x00\x58\x00\x00" // MSSP.........X..
// /* 0030 */ "\x00\xc8\x00\xc8\x00\x70\x00\x00\x00\x00\x00\x00\x00\x38\x01\x00" // .....p.......8..
// /* 0040 */ "\x00\x14\x00\x14\x00\x38\x01\x00\x00\x0c\x00\x0c\x00\x4c\x01\x00" // .....8.......L..
// /* 0050 */ "\x00\x10\x00\x10\x00\x58\x01\x00\x00\x35\xa2\x88\xe2\x06\x01\xb1" // .....X...5......
// /* 0060 */ "\x1d\x00\x00\x00\x0f\x4b\x7f\xb3\xbb\x4e\xf2\x16\xbb\xdb\x04\x23" // .....K...N.....#
// /* 0070 */ "\xc7\x46\x88\x0d\x02\xf1\x4c\xf7\x1d\x01\x88\x83\xca\xbc\x8d\x84" // .F....L.........
// /* 0080 */ "\xc8\x9f\xcc\x68\xfe\x60\xe0\x22\x5d\xa6\x21\xa3\x7d\x5f\xfd\x28" // ...h.`."].!.}_.(
// /* 0090 */ "\x4e\xbc\x94\x87\x5f\x1f\x74\xea\x9c\xba\x08\x96\x69\x01\x01\x00" // N..._.t.....i...
// /* 00a0 */ "\x00\x00\x00\x00\x00\x29\xd9\x03\x00\xd4\xf4\x53\x5d\x60\xe0\x22" // .....).....S]`."
// /* 00b0 */ "\x5d\xa6\x21\xa3\x7d\x00\x00\x00\x00\x02\x00\x1e\x00\x52\x00\x44" // ].!.}........R.D
// /* 00c0 */ "\x00\x50\x00\x2d\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f\x00\x57" // .P.-.W.I.N.D.O.W
// /* 00d0 */ "\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x01\x00\x1e\x00\x52" // .S.-.D.E.V.....R
// /* 00e0 */ "\x00\x44\x00\x50\x00\x2d\x00\x57\x00\x49\x00\x4e\x00\x44\x00\x4f" // .D.P.-.W.I.N.D.O
// /* 00f0 */ "\x00\x57\x00\x53\x00\x2d\x00\x44\x00\x45\x00\x56\x00\x04\x00\x1e" // .W.S.-.D.E.V....
// /* 0100 */ "\x00\x72\x00\x64\x00\x70\x00\x2d\x00\x77\x00\x69\x00\x6e\x00\x64" // .r.d.p.-.w.i.n.d
// /* 0110 */ "\x00\x6f\x00\x77\x00\x73\x00\x2d\x00\x64\x00\x65\x00\x76\x00\x03" // .o.w.s.-.d.e.v..
// /* 0120 */ "\x00\x1e\x00\x72\x00\x64\x00\x70\x00\x2d\x00\x77\x00\x69\x00\x6e" // ...r.d.p.-.w.i.n
// /* 0130 */ "\x00\x64\x00\x6f\x00\x77\x00\x73\x00\x2d\x00\x64\x00\x65\x00\x76" // .d.o.w.s.-.d.e.v
// /* 0140 */ "\x00\x07\x00\x08\x00\xba\xe5\x1a\x0d\x96\x52\xd5\x01\x00\x00\x00" // ..........R.....
// /* 0150 */ "\x00\x00\x00\x00\x00\x43\x00\x68\x00\x72\x00\x69\x00\x73\x00\x74" // .....C.h.r.i.s.t
// /* 0160 */ "\x00\x6f\x00\x70\x00\x68\x00\x65\x00\x63\x00\x67\x00\x72\x00\x74" // .o.p.h.e.c.g.r.t
// /* 0170 */ "\x00\x68\x00\x63\x00\x99\xce\x7c\xaa\xa9\x9b\xfd\x8c\x9e\xcb\x77" // .h.c...|.......w
// /* 0180 */ "\xd7\x82\x5c\x8e\xf1\xa3\x32\x04\x30\x01\x00\x00\x00\x09\xba\xc8" // ......2.0.......
// /* 0190 */ "\x08\xd3\xca\xf7\x82\x00\x00\x00\x00\x3b\x2f\x94\x91\xc2\x80\xcc" // .........;/.....
// /* 01a0 */ "\x2a\x66\xc3\xf7\xfe\x61\xcd\x05\xca\x51\xe7\x09\x97\xd4\x85\x01" // *f...a...Q......
// /* 01b0 */ "\x1d\xe4\x09\x31\x01\xc4\xd6\x22\x87\xa5\x22\x04\x20\xd7\x04\xc8" // ...1..."..". ...
// /* 01c0 */ "\x19\x0e\x48\x50\x16\x61\x06\xfb\xdb\x55\x7c\x2a\x73\xca\x23\x01" // ..HP.a...U|*s.#.
// /* 01d0 */ "\x98\x45\x1c\xcf\x9c\x80\x8f\xce\xa1\x39\x63\xda\xd1"             // .E.......9c..
// TSRequest hexdump -DONE----------------------------

//dpproxy: INFO (18423/18423) -- TSRequest ts_request_header -----------------------
// /* 0000 */ "\x30\x81\x86"                                                     // 0..
// TSRequest version_field ---------------------------
// /* 0000 */ "\xa0\x03\x02\x01\x06"                                             // .....
// TSRequest nego_tokens_header ----------------------
// TSRequest auth_info_header ------------------------
// /* 0000 */ "\xa2\x5b\x04\x59"                                                 // .[.Y
// TSRequest pub_key_auth_header ---------------------
// TSRequest nonce -----------------------------------
// /* 0000 */ "\xa5\x22\x04\x20"                                                 // .". 
// TSRequest full dump--------------------------------
// /* 0000 */ "\x30\x81\x86\xa0\x03\x02\x01\x06\xa2\x5b\x04\x59\x01\x00\x00\x00" // 0........[.Y....
// /* 0010 */ "\x7b\x23\x5c\x9b\x4b\x2b\xb2\x93\x01\x00\x00\x00\x2f\x3e\xdf\xf9" // {#..K+....../>..
// /* 0020 */ "\xa0\x85\xc3\x0d\xd4\x7a\xbd\xf4\x95\xda\xe3\x41\xc0\x75\xd3\xf0" // .....z.....A.u..
// /* 0030 */ "\x91\x8f\xcc\x19\xe3\x0a\x8f\xf1\x65\x96\x8a\x87\x26\x93\x43\xd1" // ........e...&.C.
// /* 0040 */ "\x7e\xfa\x21\xe3\xdb\xee\x3d\xff\x3c\x80\x22\x96\x14\xca\xc8\x8b" // ~.!...=.<.".....
// /* 0050 */ "\xbf\x81\x6c\x62\x29\x7b\x69\xee\x7d\xc8\x9f\x90\x99\xd9\xf4\x3b" // ..lb){i.}......;
// /* 0060 */ "\xa5\xcf\x14\x2b\xf1\xa5\x22\x04\x20\xd7\x04\xc8\x19\x0e\x48\x50" // ...+..". .....HP
// /* 0070 */ "\x16\x61\x06\xfb\xdb\x55\x7c\x2a\x73\xca\x23\x01\x98\x45\x1c\xcf" // .a...U|*s.#..E..
// /* 0080 */ "\x9c\x80\x8f\xce\xa1\x39\x63\xda\xd1"                             // .....9c..
// TSRequest hexdump -DONE----------------------------

// TSPasswordCreds hexdump ---------------------------
// TSPasswordCreds ts_password_creds_header ----------
// TSPasswordCreds domain name header ----------------------
// /* 0000 */ "\xa0\x02\x04\x00"                                                 // ....
// TSPasswordCreds user name header ------------------------
// /* 0000 */ "\xa1\x36\x04\x34"                                                 // .6.4
// /* 0000 */ "\x41\x00\x64\x00\x6d\x00\x69\x00\x6e\x00\x69\x00\x73\x00\x74\x00" // A.d.m.i.n.i.s.t.
// /* 0010 */ "\x72\x00\x61\x00\x74\x00\x6f\x00\x72\x00\x40\x00\x70\x00\x72\x00" // r.a.t.o.r.@.p.r.
// /* 0020 */ "\x6f\x00\x78\x00\x79\x00\x6b\x00\x64\x00\x63\x00\x2e\x00\x6c\x00" // o.x.y.k.d.c...l.
// /* 0030 */ "\x61\x00\x62\x00"                                                 // a.b.
// TSPasswordCreds password header -------------------------
// /* 0000 */ "\xa2\x1c\x04\x1a"                                                 // ....
// /* 0000 */ "\x53\x00\x65\x00\x63\x00\x75\x00\x72\x00\x65\x00\x4c\x00\x69\x00" // S.e.c.u.r.e.L.i.
// /* 0010 */ "\x6e\x00\x75\x00\x78\x00\x21\x00\x32\x00"                         // n.u.x.!.2.
// TSPasswordCreds full dump--------------------------------
// /* 0000 */ "\x30\x5a\xa0\x02\x04\x00\xa1\x36\x04\x34\x41\x00\x64\x00\x6d\x00" // 0Z.....6.4A.d.m.
// /* 0010 */ "\x69\x00\x6e\x00\x69\x00\x73\x00\x74\x00\x72\x00\x61\x00\x74\x00" // i.n.i.s.t.r.a.t.
// /* 0020 */ "\x6f\x00\x72\x00\x40\x00\x70\x00\x72\x00\x6f\x00\x78\x00\x79\x00" // o.r.@.p.r.o.x.y.
// /* 0030 */ "\x6b\x00\x64\x00\x63\x00\x2e\x00\x6c\x00\x61\x00\x62\x00\xa2\x1c" // k.d.c...l.a.b...
// /* 0040 */ "\x04\x1a\x53\x00\x65\x00\x63\x00\x75\x00\x72\x00\x65\x00\x4c\x00" // ..S.e.c.u.r.e.L.
// /* 0050 */ "\x69\x00\x6e\x00\x75\x00\x78\x00\x21\x00\x32\x00"                 // i.n.u.x.!.2.
// TSPasswordCreds hexdump -DONE----------------------------




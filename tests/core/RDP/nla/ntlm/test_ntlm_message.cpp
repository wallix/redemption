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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/lcg_random.hpp"

#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "utils/hexdump.hpp"

#include "core/RDP/nla/ntlm_message.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"
#include "utils/sugar/byte_ptr.hpp"

#include "test_only/test_framework/sig.hpp"


RED_AUTO_TEST_CASE(TestAvPair)
{
    NtlmAvPairList listAvPair;

    RED_CHECK_EQUAL(listAvPair.size(), 0);
    size_t packet_length = (sizeof(NTLM_AV_ID) + sizeof(uint16_t)) * (listAvPair.size()+1);
    for (auto & avp: listAvPair) { packet_length += avp.data.size(); }
    RED_CHECK_EQUAL(packet_length, 4);

    auto tartempion = "NomDeDomaine\0"_av;

    auto NtlmAddToAvPairList = [](NTLM_AV_ID avId, bytes_view data, NtlmAvPairList & list) -> void
    {
        for (auto & avp: list) {
            if (avp.id == avId){
                avp.data.assign(data.data(), data.data()+data.size());
                return;
            }
        }
        list.push_back({avId, std::vector<uint8_t>(data.data(), data.data()+data.size())});
    };

    NtlmAddToAvPairList(MsvAvNbDomainName, tartempion, listAvPair);

    RED_CHECK_EQUAL(listAvPair.size(), 1);
    packet_length = (sizeof(NTLM_AV_ID) + sizeof(uint16_t)) * (listAvPair.size()+1);
    for (auto & avp: listAvPair) { packet_length += avp.data.size(); }
    RED_CHECK_EQUAL(packet_length, 21);

    StaticOutStream<65535> stream;

    for (auto & avp: listAvPair) {
        stream.out_uint16_le(avp.id);
        stream.out_uint16_le(avp.data.size());
        stream.out_copy_bytes(avp.data);
    }
    stream.out_uint16_le(MsvAvEOL);
    stream.out_uint16_le(0);

    packet_length = (sizeof(NTLM_AV_ID) + sizeof(uint16_t)) * (listAvPair.size()+1);
    for (auto & avp: listAvPair) { packet_length += avp.data.size(); }
    RED_CHECK_EQUAL(packet_length, stream.get_offset());

    // LOG(LOG_INFO, "Av Pair List : %zu elements {", listAvPair.size());
    // for (auto & avp: listAvPair) {
    //     LOG(LOG_INFO, "\tAvId: 0x%02X, AvLen : %u,", avp.id, unsigned(avp.data.size()));
    //     hexdump_c(avp.data);
    // }
    // LOG(LOG_INFO, "}");
}


RED_AUTO_TEST_CASE(TestAvPairRecv)
{
    const uint8_t TargetInfo[] = {
        0x02, 0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00,
        0x4e, 0x00, 0x37, 0x00, 0x01, 0x00, 0x08, 0x00,
        0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00,
        0x04, 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00,
        0x6e, 0x00, 0x37, 0x00, 0x03, 0x00, 0x08, 0x00,
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00,
        0x07, 0x00, 0x08, 0x00, 0xa9, 0x8d, 0x9b, 0x1a,
        0x6c, 0xb0, 0xcb, 0x01, 0x00, 0x00, 0x00, 0x00
    };
    NtlmAvPairList avpairlist;
    InStream stream(TargetInfo);

    for (std::size_t i = 0; i < AV_ID_MAX; ++i) {
        auto id = stream.in_uint16_le();
        auto length = stream.in_uint16_le();
        if (id == MsvAvEOL) {
            // ASSUME last element is MsvAvEOL
            stream.in_skip_bytes(length);
            break;
        }
        avpairlist.push_back({static_cast<NTLM_AV_ID>(id), stream.in_copy_bytes_as_vector(length)});
    }
}

RED_AUTO_TEST_CASE(TestChallenge)
{
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

    TSRequest ts_req = recvTSRequest(make_array_view(packet2), true);

    RED_CHECK(ts_req.version == 6);
    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x80);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);

    StaticOutStream<65536> to_send2;

    ts_req.version = 3;

    RED_CHECK_EQUAL(to_send2.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized,
                           true);
    to_send2.out_copy_bytes(v);

    RED_CHECK_EQUAL(to_send2.get_offset(), 0x94 + 3);

    RED_CHECK_SIG_A(to_send2.get_produced_bytes(), ut::sig(make_array_view(packet2)).bytes());

    NTLMChallengeMessage ChallengeMsg;

    hexdump_c(ts_req.negoTokens.data(), ts_req.negoTokens.size());
    // ChallengeMsg.recv(ts_req.negoTokens);

    ChallengeMsg = recvNTLMChallengeMessage(ts_req.negoTokens);

    RED_CHECK_EQUAL(ChallengeMsg.negoFlags.flags, 0xe28a8235);
    //ChallengeMsg.negoFlags.log();

    RED_CHECK_EQUAL(ChallengeMsg.TargetName.buffer.size(), 8);
    RED_CHECK_EQUAL(ChallengeMsg.TargetName.bufferOffset, 56);
    RED_CHECK(ChallengeMsg.TargetName.buffer == "\x57\x00\x49\x00\x4e\x00\x37\x00"_av);
    // hexdump_c(ChallengeMsg.TargetName.buffer.ostream.get_data(),
    //           ChallengeMsg.TargetName.buffer.ostream.size());
    RED_CHECK_EQUAL(ChallengeMsg.TargetInfo.buffer.size(), 64);
    RED_CHECK_EQUAL(ChallengeMsg.TargetInfo.bufferOffset, 64);
    RED_CHECK(ChallengeMsg.TargetInfo.buffer ==
        "\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00"
        "\x57\x00\x49\x00\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00"
        "\x6e\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00"
        "\x07\x00\x08\x00\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00"_av
    );
    // hexdump_c(ChallengeMsg.TargetInfo.buffer.ostream.get_data(),
    //           ChallengeMsg.TargetInfo.buffer.ostream.size());
    InStream servChall(ChallengeMsg.serverChallenge);
    uint64_t servchallengeinteger = servChall.in_uint64_le();
    RED_CHECK_EQUAL(servchallengeinteger, 8063485858206805542LL);

    // ChallengeMsg.version.print();

    //ChallengeMsg.AvPairList.log();

    // // hexdump_c(to_send2.get_data(), to_send2.size());

    auto target_info = emitTargetInfo(ChallengeMsg.AvPairList);

    uint32_t negoFlags = NTLMSSP_NEGOTIATE_56
                  | NTLMSSP_NEGOTIATE_KEY_EXCH
                  | NTLMSSP_NEGOTIATE_128
                  | NTLMSSP_NEGOTIATE_VERSION
                  | NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY
                  | NTLMSSP_TARGET_TYPE_SERVER
                  | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                  | NTLMSSP_NEGOTIATE_NTLM
                  | NTLMSSP_NEGOTIATE_SEAL
                  | NTLMSSP_NEGOTIATE_SIGN
                  | NTLMSSP_REQUEST_TARGET
                  | NTLMSSP_NEGOTIATE_UNICODE;

    logNtlmFlags(negoFlags);

    NtlmVersion ntlm_version;
    auto raw_ntlm_version = emitNtlmVersion(
                                ntlm_version.ProductMajorVersion,
                                ntlm_version.ProductMinorVersion,
                                ntlm_version.ProductBuild,
                                ntlm_version.NtlmRevisionCurrent);
    auto challenge = emitNTLMChallengeMessage(ChallengeMsg.TargetName.buffer,ChallengeMsg.serverChallenge, negoFlags, raw_ntlm_version, target_info);

    NTLMChallengeMessage ChallengeMsgDuplicate;

    ChallengeMsgDuplicate = recvNTLMChallengeMessage(challenge);

    RED_CHECK_EQUAL(ChallengeMsgDuplicate.negoFlags.flags, 3792339509);
    // ChallengeMsgDuplicate.negoFlags.print();

    RED_CHECK_EQUAL(ChallengeMsgDuplicate.TargetName.buffer.size(), 8);
    RED_CHECK_EQUAL(ChallengeMsgDuplicate.TargetName.bufferOffset, 56);

    RED_CHECK_EQUAL(ChallengeMsgDuplicate.TargetInfo.buffer.size(), 64);
    RED_CHECK_EQUAL(ChallengeMsgDuplicate.TargetInfo.bufferOffset, 64);
    InStream servChall2(ChallengeMsgDuplicate.serverChallenge);
    uint64_t servchallengeinteger2 = servChall2.in_uint64_le();
    RED_CHECK_EQUAL(servchallengeinteger2, 8063485858206805542LL);

    // ChallengeMsgDuplicate.AvPairList.print();
}

RED_AUTO_TEST_CASE(TestNegotiate)
{
    // ===== NTLMSSP_NEGOTIATE =====
    uint8_t packet[] = {
        0x30, 0x37, 0xa0, 0x03, 0x02, 0x01, 0x03, 0xa1,
        0x30, 0x30, 0x2e, 0x30, 0x2c, 0xa0, 0x2a, 0x04,
        0x28, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50,
        0x00, 0x01, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08,
        0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x05, 0x01, 0x28, 0x0a, 0x00, 0x00, 0x00,
        0x0f
    };

    TSRequest ts_req = recvTSRequest(make_array_view(packet), true);

    RED_CHECK_EQUAL(ts_req.version, 6);

    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x28);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);

    ts_req.version = 3;

    StaticOutStream<65536> to_send;

    RED_CHECK_EQUAL(to_send.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized,
                           true);
    to_send.out_copy_bytes(v);

    RED_CHECK_EQUAL(to_send.get_offset(), 0x37 + 2);

    RED_CHECK_SIG_A(to_send.get_produced_bytes(), ut::sig(make_array_view(packet)).bytes());

    auto NegoMsg = recvNTLMNegotiateMessage(ts_req.negoTokens);


    RED_CHECK_EQUAL(NegoMsg.negoFlags.flags, 0xe20882b7);
    // NegoMsg.negoFlags.print();

    RED_CHECK_EQUAL(NegoMsg.DomainName.buffer.size(), 0);
    RED_CHECK_EQUAL(NegoMsg.DomainName.bufferOffset, 0);
    // hexdump_c(NegoMsg.DomainName.Buffer.get_data(),
    //           NegoMsg.DomainName.Buffer.size());
    // uint8_t domainname_match[] = "";
    // RED_CHECK_EQUAL(memcmp(domainname_match,
    //                          NegoMsg.DomainName.Buffer.get_data(),
    //                          NegoMsg.DomainName.len),
    //                   0);

    RED_CHECK_EQUAL(NegoMsg.Workstation.buffer.size(), 0);
    RED_CHECK_EQUAL(NegoMsg.Workstation.bufferOffset, 0);
    // uint8_t workstation_match[] = "";
    // RED_CHECK_EQUAL(memcmp(workstation_match,
    //                          NegoMsg.Workstation.Buffer.get_data(),
    //                          NegoMsg.Workstation.len),
    //                   0);
    // hexdump_c(NegoMsg.DomainName.Buffer.get_data(),
    //           NegoMsg.Workstation.Buffer.size());

    // NegoMsg.version.log();

    // // hexdump_c(to_send.get_data(), to_send.size());
    // BStream outMsg;
    // NegoMsg.emit(outMsg);
    // hexdump_c(outMsg.get_data(), outMsg.size());


}



// // ===== PUBKEYAUTH =====
// uint8_t packet4[] = {
//     0x30, 0x82, 0x01, 0x2b, 0xa0, 0x03, 0x02, 0x01,
//     0x02, 0xa3, 0x82, 0x01, 0x22, 0x04, 0x82, 0x01,
//     0x1e, 0x01, 0x00, 0x00, 0x00, 0xc9, 0x88, 0xfc,
//     0xf1, 0x11, 0x68, 0x2c, 0x72, 0x00, 0x00, 0x00,
//     0x00, 0xc7, 0x51, 0xf4, 0x71, 0xd3, 0x9f, 0xb6,
//     0x50, 0xbe, 0xa8, 0xf6, 0x20, 0x77, 0xa1, 0xfc,
//     0xdd, 0x8e, 0x02, 0xf0, 0xa4, 0x6b, 0xba, 0x3f,
//     0x9d, 0x65, 0x9d, 0xab, 0x4a, 0x95, 0xc9, 0xb4,
//     0x38, 0x03, 0x87, 0x04, 0xb1, 0xfe, 0x42, 0xec,
//     0xfa, 0xfc, 0xaa, 0x85, 0xf1, 0x31, 0x2d, 0x26,
//     0xcf, 0x63, 0xfd, 0x62, 0x36, 0xcf, 0x56, 0xc3,
//     0xfb, 0xf6, 0x36, 0x9b, 0xe5, 0xb2, 0xe7, 0xce,
//     0xcb, 0xe1, 0x82, 0xb2, 0x89, 0xff, 0xdd, 0x87,
//     0x5e, 0xd3, 0xd8, 0xff, 0x2e, 0x16, 0x35, 0xad,
//     0xdb, 0xda, 0xc9, 0xc5, 0x81, 0xad, 0x48, 0xf1,
//     0x8b, 0x76, 0x3d, 0x74, 0x34, 0xdf, 0x80, 0x6b,
//     0xf3, 0x68, 0x6d, 0xf6, 0xec, 0x5f, 0xbe, 0xea,
//     0xb7, 0x6c, 0xea, 0xe4, 0xeb, 0xe9, 0x17, 0xf9,
//     0x4e, 0x0d, 0x79, 0xd5, 0x82, 0xdd, 0xb7, 0xdc,
//     0xcd, 0xfc, 0xbb, 0xf1, 0x0b, 0x9b, 0xe9, 0x18,
//     0xe7, 0xb3, 0xb3, 0x8b, 0x40, 0x82, 0xa0, 0x9d,
//     0x58, 0x73, 0xda, 0x54, 0xa2, 0x2b, 0xd2, 0xb6,
//     0x41, 0x60, 0x8a, 0x64, 0xf2, 0xa2, 0x59, 0x64,
//     0xcf, 0x27, 0x1a, 0xe6, 0xb5, 0x1a, 0x0e, 0x0e,
//     0xe1, 0x14, 0xef, 0x26, 0x68, 0xeb, 0xc8, 0x49,
//     0xe2, 0x66, 0xbb, 0x11, 0x71, 0x49, 0xad, 0x7e,
//     0xae, 0xde, 0xa8, 0x78, 0xfd, 0x64, 0x51, 0xd8,
//     0x18, 0x01, 0x11, 0xc0, 0x8d, 0x3b, 0xec, 0x40,
//     0x2b, 0x1f, 0xc5, 0xa4, 0x45, 0x1e, 0x07, 0xae,
//     0x5a, 0xd8, 0x1c, 0xab, 0xdf, 0x89, 0x96, 0xdc,
//     0xdc, 0x29, 0xd8, 0x30, 0xdb, 0xbf, 0x48, 0x2a,
//     0x42, 0x27, 0xc2, 0x50, 0xac, 0xf9, 0x02, 0xd1,
//     0x20, 0x12, 0xdd, 0x50, 0x22, 0x09, 0x44, 0xac,
//     0xe0, 0x22, 0x1f, 0x66, 0x64, 0xec, 0xfa, 0x2b,
//     0xb8, 0xcd, 0x43, 0x3a, 0xce, 0x40, 0x74, 0xe1,
//     0x34, 0x81, 0xe3, 0x94, 0x47, 0x6f, 0x49, 0x01,
//     0xf8, 0xb5, 0xfc, 0xd0, 0x75, 0x80, 0xc6, 0x35,
//     0xac, 0xc0, 0xfd, 0x1b, 0xb5, 0xa2, 0xd3
// };

// LOG(LOG_INFO, "=================================\n");
// s.init(sizeof(packet4));
// s.out_copy_bytes(packet4, sizeof(packet4));
// s.mark_end();
// s.rewind();
// get_sig(s, sig);

// TSRequest ts_req4(s);

// RED_CHECK_EQUAL(ts_req4.version, 2);

// RED_CHECK_EQUAL(ts_req4.negoTokens.size(), 0);
// RED_CHECK_EQUAL(ts_req4.authInfo.size(), 0);
// RED_CHECK_EQUAL(ts_req4.pubKeyAuth.size(), 0x11e);

// BStream to_send4;

// RED_CHECK_EQUAL(to_send4.size(), 0);
// ts_req4.emit(to_send4);

// RED_CHECK_EQUAL(to_send4.size(), 0x12b + 4);

// if (!check_sig(to_send4, message, sig)){
//     RED_CHECK_MESSAGE(false, message);
// }

// // hexdump_c(to_send4.get_data(), to_send4.size());


// // ===== AUTHINFO =====
// uint8_t packet5[] = {
//     0x30, 0x5a, 0xa0, 0x03, 0x02, 0x01, 0x02, 0xa2,
//     0x53, 0x04, 0x51, 0x01, 0x00, 0x00, 0x00, 0xb3,
//     0x2c, 0x3b, 0xa1, 0x36, 0xf6, 0x55, 0x71, 0x01,
//     0x00, 0x00, 0x00, 0xa8, 0x85, 0x7d, 0x11, 0xef,
//     0x92, 0xa0, 0xd6, 0xff, 0xee, 0xa1, 0xae, 0x6d,
//     0xc5, 0x2e, 0x4e, 0x65, 0x50, 0x28, 0x93, 0x75,
//     0x30, 0xe1, 0xc3, 0x37, 0xeb, 0xac, 0x1f, 0xdd,
//     0xf3, 0xe0, 0x92, 0xf6, 0x21, 0xbc, 0x8f, 0xa8,
//     0xd4, 0xe0, 0x5a, 0xa6, 0xff, 0xda, 0x09, 0x50,
//     0x24, 0x0d, 0x8f, 0x8f, 0xf4, 0x92, 0xfe, 0x49,
//     0x2a, 0x13, 0x52, 0xa6, 0x52, 0x75, 0x50, 0x8d,
//     0x3e, 0xe9, 0x6b, 0x57
// };
// LOG(LOG_INFO, "=================================\n");
// s.init(sizeof(packet5));
// s.out_copy_bytes(packet5, sizeof(packet5));
// s.mark_end();
// s.rewind();
// get_sig(s, sig);

// TSRequest ts_req5(s);

// RED_CHECK_EQUAL(ts_req5.version, 2);

// RED_CHECK_EQUAL(ts_req5.negoTokens.size(), 0);
// RED_CHECK_EQUAL(ts_req5.authInfo.size(), 0x51);
// RED_CHECK_EQUAL(ts_req5.pubKeyAuth.size(), 0);

// BStream to_send5;

// RED_CHECK_EQUAL(to_send5.size(), 0);
// ts_req5.emit(to_send5);

// RED_CHECK_EQUAL(to_send5.size(), 0x5a + 2);

// if (!check_sig(to_send5, message, sig)){
//     RED_CHECK_MESSAGE(false, message);
// }

// // hexdump_c(to_send5.get_data(), to_send5.size());

RED_AUTO_TEST_CASE(TestAuthenticate)
{
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

    TSRequest ts_req = recvTSRequest(make_array_view(packet3), true);

    RED_CHECK_EQUAL(ts_req.version, 6);
    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x102);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0x11e);

    ts_req.version = 3;

    StaticOutStream<65536> to_send3;
    RED_CHECK_EQUAL(to_send3.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized,
                           true);
    to_send3.out_copy_bytes(v);


    RED_CHECK_EQUAL(to_send3.get_offset(), 0x241 + 4);

    RED_CHECK_SIG_A(to_send3.get_produced_bytes(), ut::sig(make_array_view(packet3)).bytes());

    // hexdump_c(to_send3.get_data(), to_send3.size());

    NTLMAuthenticateMessage AuthMsg;
    RED_CHECK_EQUAL(AuthMsg.has_mic, true);
    // AuthMsg.recv(ts_req.negoTokens);

    AuthMsg = recvNTLMAuthenticateMessage(ts_req.negoTokens, true);

    RED_CHECK_EQUAL(AuthMsg.negoFlags.flags, 0xE2888235);
    // AuthMsg.negoFlags.print();

    RED_CHECK_EQUAL(AuthMsg.LmChallengeResponse.buffer.size(), 24);
    RED_CHECK_EQUAL(AuthMsg.LmChallengeResponse.bufferOffset, 106);
    RED_CHECK_EQUAL(AuthMsg.NtChallengeResponse.buffer.size(), 112);
    RED_CHECK_EQUAL(AuthMsg.NtChallengeResponse.bufferOffset, 130);
    RED_CHECK_EQUAL(AuthMsg.DomainName.buffer.size(), 8);
    RED_CHECK_EQUAL(AuthMsg.DomainName.bufferOffset, 72);
    RED_CHECK_EQUAL(AuthMsg.UserName.buffer.size(), 16);
    RED_CHECK_EQUAL(AuthMsg.UserName.bufferOffset, 80);
    RED_CHECK_EQUAL(AuthMsg.Workstation.buffer.size(), 10);
    RED_CHECK_EQUAL(AuthMsg.Workstation.bufferOffset, 96);
    RED_CHECK_EQUAL(AuthMsg.EncryptedRandomSessionKey.buffer.size(), 16);
    RED_CHECK_EQUAL(AuthMsg.has_mic, false);

    // LmChallengeResponse
    // LOG(LOG_INFO, "Lm Response . Response ===========\n");
    // hexdump_c(lmResponse.Response, 16);
    RED_CHECK(lmv2_response(AuthMsg.LmChallengeResponse.buffer) ==
        "\xa0\x98\x01\x10\x19\xbb\x5d\x00\xf6\xbe\x00\x33\x90\x20\x34\xb3"_av);

    // LOG(LOG_INFO, "Lm Response . ClientChallenge ===========\n");
    // hexdump_c(lmResponse.ClientChallenge, 8);
    RED_CHECK(lmv2_client_challenge(AuthMsg.LmChallengeResponse.buffer) ==
        "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43"_av);


    // NtChallengeResponse
    NTLMv2_Response ntResponse;
    InStream in_stream2(AuthMsg.NtChallengeResponse.buffer);
    ntResponse.recv(in_stream2);

    // LOG(LOG_INFO, "Nt Response . Response ===========\n");
    // hexdump_c(ntResponse.Response, 16);
    RED_CHECK(make_array_view(ntResponse.Response) ==
        "\x01\x4a\xd0\x8c\x24\xb4\x90\x74\x39\x68\xe8\xbd\x0d\x2b\x70\x10"_av);

    RED_CHECK_EQUAL(ntResponse.Challenge.RespType, 1);
    RED_CHECK_EQUAL(ntResponse.Challenge.HiRespType, 1);
    // LOG(LOG_INFO, "Nt Response . Challenge . Timestamp ===========\n");
    // hexdump_c(ntResponse.Challenge.Timestamp, 8);
    RED_CHECK(make_array_view(ntResponse.Challenge.Timestamp) ==
        "\xc3\x83\xa2\x1c\x6c\xb0\xcb\x01"_av);

    // LOG(LOG_INFO, "Nt Response . Challenge . ClientChallenge ===========\n");
    // hexdump_c(ntResponse.Challenge.ClientChallenge, 8);
    RED_CHECK(make_array_view(ntResponse.Challenge.ClientChallenge) ==
        "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43"_av);

    // LOG(LOG_INFO, "Nt Response . Challenge . AvPairList ===========\n");
    // ntResponse.Challenge.AvPairList.print();

    // Domain Name
    // LOG(LOG_INFO, "Domain Name ===========\n");
    // hexdump_c(AuthMsg.DomainName.Buffer.get_data(), AuthMsg.DomainName.Buffer.size());
    RED_CHECK(make_array_view(AuthMsg.DomainName.buffer) == "\x77\x00\x69\x00\x6e\x00\x37\x00"_av);

    // User Name
    // LOG(LOG_INFO, "User Name ===========\n");
    // hexdump_c(AuthMsg.UserName.Buffer.get_data(), AuthMsg.UserName.Buffer.size());
    RED_CHECK(make_array_view(AuthMsg.UserName.buffer) ==
        "\x75\x00\x73\x00\x65\x00\x72\x00\x6e\x00\x61\x00\x6d\x00\x65\x00"_av);

    // Work Station
    // LOG(LOG_INFO, "Work Station ===========\n");
    // hexdump_c(AuthMsg.Workstation.buffer);
    RED_CHECK(make_array_view(AuthMsg.Workstation.buffer) ==
        "\x57\x00\x49\x00\x4e\x00\x58\x00\x50\x00"_av);

    // Encrypted Random Session Key
    // LOG(LOG_INFO, "Encrypted Random Session Key ===========\n");
    // hexdump_c(AuthMsg.EncryptedRandomSessionKey.Buffer.get_data(),
    //           AuthMsg.EncryptedRandomSessionKey.Buffer.size());
    RED_CHECK(make_array_view(AuthMsg.EncryptedRandomSessionKey.buffer) ==
        "\xb1\xd2\x45\x42\x0f\x37\x9a\x0e\xe0\xce\x77\x40\x10\x8a\xda\xba"_av);

    StaticOutStream<65635> tosend;
    size_t mic_offset;
    auto auth_message = emitNTLMAuthenticateMessage(AuthMsg.negoFlags.flags,
                        NtlmVersion(),
                        AuthMsg.LmChallengeResponse.buffer,
                        AuthMsg.NtChallengeResponse.buffer,
                        AuthMsg.DomainName.buffer,
                        AuthMsg.UserName.buffer,
                        AuthMsg.Workstation.buffer,
                        AuthMsg.EncryptedRandomSessionKey.buffer,
                        AuthMsg.has_mic,
                        mic_offset);

    NTLMAuthenticateMessage AuthMsgDuplicate;

    AuthMsgDuplicate = recvNTLMAuthenticateMessage(auth_message, true);

    RED_CHECK_EQUAL(AuthMsgDuplicate.negoFlags.flags, 0xE2888235);
    logNtlmFlags(AuthMsgDuplicate.negoFlags.flags);

    RED_CHECK_EQUAL(AuthMsgDuplicate.LmChallengeResponse.buffer.size(), 24);
    RED_CHECK_EQUAL(AuthMsgDuplicate.LmChallengeResponse.bufferOffset, 72);
    RED_CHECK_EQUAL(AuthMsgDuplicate.NtChallengeResponse.buffer.size(), 112);
    RED_CHECK_EQUAL(AuthMsgDuplicate.NtChallengeResponse.bufferOffset, 96);
    RED_CHECK_EQUAL(AuthMsgDuplicate.DomainName.buffer.size(), 8);
    RED_CHECK_EQUAL(AuthMsgDuplicate.DomainName.bufferOffset, 208);
    RED_CHECK_EQUAL(AuthMsgDuplicate.UserName.buffer.size(), 16);
    RED_CHECK_EQUAL(AuthMsgDuplicate.UserName.bufferOffset, 216);
    RED_CHECK_EQUAL(AuthMsgDuplicate.Workstation.buffer.size(), 0);
    RED_CHECK_EQUAL(AuthMsgDuplicate.Workstation.bufferOffset, 232);
    RED_CHECK_EQUAL(AuthMsgDuplicate.EncryptedRandomSessionKey.buffer.size(), 16);
    RED_CHECK_EQUAL(AuthMsgDuplicate.EncryptedRandomSessionKey.bufferOffset, 232);
}




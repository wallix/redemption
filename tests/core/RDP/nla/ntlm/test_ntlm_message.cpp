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
#include "utils/translation.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"
#include "utils/sugar/byte_ptr.hpp"
#include "utils/difftimeval.hpp"

#include "test_only/check_sig.hpp"

#include <memory>
#include <functional>

struct SEC_WINNT_AUTH_IDENTITY
{
    // kerberos only
    //@{
    char princname[256];
    char princpass[256];
    //@}
    // ntlm only
    //@{
    private:
    std::vector<uint8_t> User;
    std::vector<uint8_t> Domain;
    public:
    std::vector<uint8_t> Password;
    //@}

    public:
    SEC_WINNT_AUTH_IDENTITY()
    {
        this->princname[0] = 0;
        this->princpass[0] = 0;
    }

    void user_init_copy(bytes_view av)
    {
        this->User.assign(av.data(), av.data()+av.size());
    }

    void domain_init_copy(bytes_view av)
    {
        this->Domain.assign(av.data(), av.data()+av.size());
    }

    bool is_empty_user_domain(){
        return (this->User.size() == 0) && (this->Domain.size() == 0);
    }

    bytes_view get_password_utf16_av() const
    {
        return this->Password;
    }

    bytes_view get_user_utf16_av() const
    {
        return this->User;
    }

    bytes_view get_domain_utf16_av() const
    {
        return this->Domain;
    }

    void SetUserFromUtf8(const uint8_t * user)
    {
        if (user) {
            size_t user_len = UTF8Len(user);
            this->User = std::vector<uint8_t>(user_len * 2);
            UTF8toUTF16({user, strlen(char_ptr_cast(user))}, this->User.data(), user_len * 2);
        }
        else {
            this->User.clear();
        }
    }

    void SetDomainFromUtf8(const uint8_t * domain)
    {
        if (domain) {
            size_t domain_len = UTF8Len(domain);
            this->Domain = std::vector<uint8_t>(domain_len * 2);
            UTF8toUTF16({domain, strlen(char_ptr_cast(domain))}, this->Domain.data(), domain_len * 2);
        }
        else {
            this->Domain.clear();
        }
    }

    void SetPasswordFromUtf8(const uint8_t * password)
    {
        if (password) {
            size_t password_len = UTF8Len(password);
            this->Password = std::vector<uint8_t>(password_len * 2);
            UTF8toUTF16({password, strlen(char_ptr_cast(password))}, this->Password.data(), password_len * 2);
        }
        else {
            this->Password.clear();
        }
    }

    void SetKrbAuthIdentity(const uint8_t * user, const uint8_t * pass)
    {
        auto copy = [](char (&arr)[256], uint8_t const* data){
            if (data) {
                const char * p = char_ptr_cast(data);
                const size_t length = p ? strnlen(p, 255) : 0;
                memcpy(arr, data, length);
                arr[length] = 0;
            }
        };

        copy(this->princname, user);
        copy(this->princpass, pass);
    }

    void clear()
    {
        this->User.clear();
        this->Domain.clear();
        this->Password.clear();
    }

    void CopyAuthIdentity(bytes_view user_utf16_av, bytes_view domain_utf16_av, bytes_view password_utf16_av)
    {
        this->User.assign(user_utf16_av.data(),user_utf16_av.data()+user_utf16_av.size());
        this->Domain.assign(domain_utf16_av.data(),domain_utf16_av.data()+domain_utf16_av.size());
        this->Password.assign(password_utf16_av.data(),password_utf16_av.data()+password_utf16_av.size());
    }

};


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

    LOG(LOG_INFO, "Av Pair List : %zu elements {", listAvPair.size());
    for (auto & avp: listAvPair) {
        LOG(LOG_INFO, "\tAvId: 0x%02X, AvLen : %u,", avp.id, unsigned(avp.data.size()));
        hexdump_c(avp.data);
    }
    LOG(LOG_INFO, "}");
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

    TSRequest ts_req = recvTSRequest(make_array_view(packet2), 3);

    RED_CHECK_EQUAL(ts_req.version, 3);
    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x80);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);

    StaticOutStream<65536> to_send2;

    RED_CHECK_EQUAL(to_send2.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized);
    to_send2.out_copy_bytes(v);

    RED_CHECK_EQUAL(to_send2.get_offset(), 0x94 + 3);

    RED_CHECK_SIG_FROM(to_send2, packet2);

    NTLMChallengeMessage ChallengeMsg;

    hexdump_c(ts_req.negoTokens.data(), ts_req.negoTokens.size());
    // ChallengeMsg.recv(ts_req.negoTokens);

    ChallengeMsg = recvNTLMChallengeMessage(ts_req.negoTokens);

    RED_CHECK_EQUAL(ChallengeMsg.negoFlags.flags, 0xe28a8235);
    //ChallengeMsg.negoFlags.log();

    RED_CHECK_EQUAL(ChallengeMsg.TargetName.buffer.size(), 8);
    RED_CHECK_EQUAL(ChallengeMsg.TargetName.bufferOffset, 56);
    RED_CHECK_MEM(ChallengeMsg.TargetName.buffer,
        "\x57\x00\x49\x00\x4e\x00\x37\x00"_av
    );
    // hexdump_c(ChallengeMsg.TargetName.buffer.ostream.get_data(),
    //           ChallengeMsg.TargetName.buffer.ostream.size());
    RED_CHECK_EQUAL(ChallengeMsg.TargetInfo.buffer.size(), 64);
    RED_CHECK_EQUAL(ChallengeMsg.TargetInfo.bufferOffset, 64);
    RED_CHECK_MEM(ChallengeMsg.TargetInfo.buffer,
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

    StaticOutStream<65535> tosend;
    EmitNTLMChallengeMessage(tosend, ChallengeMsg);

    NTLMChallengeMessage ChallengeMsgDuplicate;

    ChallengeMsgDuplicate = recvNTLMChallengeMessage(tosend.get_bytes());

    RED_CHECK_EQUAL(ChallengeMsgDuplicate.negoFlags.flags, 0xE28A8235);
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

    TSRequest ts_req = recvTSRequest(make_array_view(packet), 3);

    RED_CHECK_EQUAL(ts_req.version, 3);

    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x28);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);


    StaticOutStream<65536> to_send;

    RED_CHECK_EQUAL(to_send.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized);
    to_send.out_copy_bytes(v);

    RED_CHECK_EQUAL(to_send.get_offset(), 0x37 + 2);

    RED_CHECK_SIG_FROM(to_send, packet);

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

    TSRequest ts_req = recvTSRequest(make_array_view(packet3),3);

    RED_CHECK_EQUAL(ts_req.version, 3);
    RED_CHECK_EQUAL(ts_req.negoTokens.size(), 0x102);
    RED_CHECK_EQUAL(ts_req.authInfo.size(), 0);
    RED_CHECK_EQUAL(ts_req.error_code, 0);
    RED_CHECK_EQUAL(ts_req.pubKeyAuth.size(), 0x11e);

    StaticOutStream<65536> to_send3;
    RED_CHECK_EQUAL(to_send3.get_offset(), 0);
    auto v = emitTSRequest(ts_req.version,
                           ts_req.negoTokens,
                           ts_req.authInfo,
                           ts_req.pubKeyAuth,
                           ts_req.error_code,
                           ts_req.clientNonce.clientNonce,
                           ts_req.clientNonce.initialized);
    to_send3.out_copy_bytes(v);


    RED_CHECK_EQUAL(to_send3.get_offset(), 0x241 + 4);

    RED_CHECK_SIG_FROM(to_send3, packet3);

    // hexdump_c(to_send3.get_data(), to_send3.size());

    NTLMAuthenticateMessage AuthMsg;
    RED_CHECK_EQUAL(AuthMsg.has_mic, true);
    // AuthMsg.recv(ts_req.negoTokens);

    InStream token({ts_req.negoTokens.data(), ts_req.negoTokens.size()});
    recvNTLMAuthenticateMessage(token, AuthMsg);

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
    RED_CHECK_MEM_AA(lmv2_response(AuthMsg.LmChallengeResponse.buffer),
                    "\xa0\x98\x01\x10\x19\xbb\x5d\x00\xf6\xbe\x00\x33\x90\x20\x34\xb3"_av);

    // LOG(LOG_INFO, "Lm Response . ClientChallenge ===========\n");
    // hexdump_c(lmResponse.ClientChallenge, 8);
    RED_CHECK_MEM_AA(lmv2_client_challenge(AuthMsg.LmChallengeResponse.buffer),
                    "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43"_av);


    // NtChallengeResponse
    NTLMv2_Response ntResponse;
    InStream in_stream2(AuthMsg.NtChallengeResponse.buffer);
    ntResponse.recv(in_stream2);

    // LOG(LOG_INFO, "Nt Response . Response ===========\n");
    // hexdump_c(ntResponse.Response, 16);
    RED_CHECK_MEM_AA(ntResponse.Response, "\x01\x4a\xd0\x8c\x24\xb4\x90\x74\x39\x68\xe8\xbd\x0d\x2b\x70\x10"_av);

    RED_CHECK_EQUAL(ntResponse.Challenge.RespType, 1);
    RED_CHECK_EQUAL(ntResponse.Challenge.HiRespType, 1);
    // LOG(LOG_INFO, "Nt Response . Challenge . Timestamp ===========\n");
    // hexdump_c(ntResponse.Challenge.Timestamp, 8);
    RED_CHECK_MEM_AA(ntResponse.Challenge.Timestamp, "\xc3\x83\xa2\x1c\x6c\xb0\xcb\x01"_av);

    // LOG(LOG_INFO, "Nt Response . Challenge . ClientChallenge ===========\n");
    // hexdump_c(ntResponse.Challenge.ClientChallenge, 8);
    RED_CHECK_MEM_AA(ntResponse.Challenge.ClientChallenge, "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43"_av);

    // LOG(LOG_INFO, "Nt Response . Challenge . AvPairList ===========\n");
    // ntResponse.Challenge.AvPairList.print();

    // Domain Name
    // LOG(LOG_INFO, "Domain Name ===========\n");
    // hexdump_c(AuthMsg.DomainName.Buffer.get_data(), AuthMsg.DomainName.Buffer.size());
    RED_CHECK_MEM(AuthMsg.DomainName.buffer,
        "\x77\x00\x69\x00\x6e\x00\x37\x00"_av
    );

    // User Name
    // LOG(LOG_INFO, "User Name ===========\n");
    // hexdump_c(AuthMsg.UserName.Buffer.get_data(), AuthMsg.UserName.Buffer.size());
    RED_CHECK_MEM(AuthMsg.UserName.buffer,
        "\x75\x00\x73\x00\x65\x00\x72\x00\x6e\x00\x61\x00\x6d\x00\x65\x00"_av
    );

    // Work Station
    // LOG(LOG_INFO, "Work Station ===========\n");
    // hexdump_c(AuthMsg.Workstation.buffer);
    RED_CHECK_MEM(AuthMsg.Workstation.buffer, "\x57\x00\x49\x00\x4e\x00\x58\x00\x50\x00"_av
    );

    // Encrypted Random Session Key
    // LOG(LOG_INFO, "Encrypted Random Session Key ===========\n");
    // hexdump_c(AuthMsg.EncryptedRandomSessionKey.Buffer.get_data(),
    //           AuthMsg.EncryptedRandomSessionKey.Buffer.size());
    RED_CHECK_MEM(AuthMsg.EncryptedRandomSessionKey.buffer,
        "\xb1\xd2\x45\x42\x0f\x37\x9a\x0e\xe0\xce\x77\x40\x10\x8a\xda\xba"_av
    );

    StaticOutStream<65635> tosend;
    size_t mic_offset;
    auto auth_message = emitNTLMAuthenticateMessage(AuthMsg.negoFlags.flags,
                        AuthMsg.LmChallengeResponse.buffer,
                        AuthMsg.NtChallengeResponse.buffer,
                        AuthMsg.DomainName.buffer,
                        AuthMsg.UserName.buffer,
                        AuthMsg.Workstation.buffer,
                        AuthMsg.EncryptedRandomSessionKey.buffer,
                        AuthMsg.has_mic,
                        mic_offset);
    tosend.out_copy_bytes(auth_message);

    NTLMAuthenticateMessage AuthMsgDuplicate;

    InStream in_tosend(tosend.get_bytes());
    recvNTLMAuthenticateMessage(in_tosend, AuthMsgDuplicate);

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


class NTLMContext
{
public:
    TimeObj & timeobj;
    Random & rand;
private:
    const bool server = false;
    const bool NTLMv2 = true;
    bool UseMIC{};
public:
    NtlmState state = NTLM_STATE_INITIAL;

private:
    uint8_t MachineID[32];
    const bool SendVersionInfo = true;
    const bool confidentiality = true;

    using array16 = uint8_t[16];

public:
    SslRC4 SendRc4Seal {};
    SslRC4 RecvRc4Seal {};
    array16* SendSigningKey = nullptr;
    array16* RecvSigningKey = nullptr;
private:
    // TODO unused
    array16* SendSealingKey = nullptr;
    array16* RecvSealingKey = nullptr;

public:
    uint32_t NegotiateFlags = 0;

public:
    //int LmCompatibilityLevel;
    bool SendWorkstationName = true;
public:
    std::vector<uint8_t> Workstation;
    std::vector<uint8_t> ServicePrincipalName;
    SEC_WINNT_AUTH_IDENTITY identity{};

    // bool SendSingleHostData;
    // NTLM_SINGLE_HOST_DATA SingleHostData;
    NTLMNegotiateMessage NEGOTIATE_MESSAGE{};
    NTLMChallengeMessage CHALLENGE_MESSAGE{};
    NTLMAuthenticateMessage AUTHENTICATE_MESSAGE{};

private:
    NtlmVersion version;
public:
    std::vector<uint8_t> SavedNegotiateMessage;
    std::vector<uint8_t> SavedChallengeMessage;
    std::vector<uint8_t> SavedAuthenticateMessage;

    uint8_t Timestamp[8]{};
public:
    uint8_t ChallengeTimestamp[8]{};
public:
    array_challenge ServerChallenge{};
private:
    array_challenge ClientChallenge{};
public:
    array_md5 SessionBaseKey = {};
private:
    //uint8_t KeyExchangeKey[16];
    //uint8_t RandomSessionKey[16];
public:
    uint8_t ExportedSessionKey[16]{};
    uint8_t EncryptedRandomSessionKey[16]{};
    uint8_t ClientSigningKey[16]{};
    uint8_t ClientSealingKey[16]{};
    uint8_t ServerSigningKey[16]{};
    uint8_t ServerSealingKey[16]{};
    uint8_t MessageIntegrityCheck[SslMd5::DIGEST_LENGTH];
    // uint8_t NtProofStr[16];

    const bool verbose;

public:
    explicit NTLMContext(bool is_server, Random & rand, TimeObj & timeobj, bool verbose = false)
        : timeobj(timeobj)
        , rand(rand)
        , server(is_server)
        , UseMIC(this->NTLMv2/* == true*/)
        //, LmCompatibilityLevel(3)
        , ServicePrincipalName(0)
        , SavedNegotiateMessage(0)
        , SavedChallengeMessage(0)
        , SavedAuthenticateMessage(0)
        //, KeyExchangeKey()
        //, RandomSessionKey()
        //, SendSingleHostData(false)
        , verbose(verbose)
    {
        memset(this->MachineID, 0xAA, sizeof(this->MachineID));
        memset(this->MessageIntegrityCheck, 0x00, sizeof(this->MessageIntegrityCheck));

        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Init");
    }

    NTLMContext(NTLMContext const &) = delete;
    NTLMContext& operator = (NTLMContext const &) = delete;

    /**
     * Generate RandomSessionKey (16-byte nonce).
     */

    // all strings are in unicode utf16
    void NTOWFv2_FromHash(array_view_const_u8 hash,
                          array_view_const_u8 user,
                          array_view_const_u8 domain,
                          uint8_t (&buff)[SslMd5::DIGEST_LENGTH]) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext NTOWFv2 Hash");
        SslHMAC_Md5 hmac_md5(hash);

        auto unique_userup = std::make_unique<uint8_t[]>(user.size());
        uint8_t * userup = unique_userup.get();
        memcpy(userup, user.data(), user.size());
        UTF16Upper(userup, user.size());
        hmac_md5.update({userup, user.size()});
        unique_userup.reset();

        // hmac_md5.update({user, user_size});
        hmac_md5.update(domain);
        hmac_md5.final(buff);
    }

    // all strings are in unicode utf16
    void hash_password(array_view_const_u8 pass, uint8_t (&hash)[SslMd4::DIGEST_LENGTH]) {
        SslMd4 md4;
        md4.update(pass);
        md4.final(hash);
    }

    // all strings are in unicode utf16
    void NTOWFv2(array_view_const_u8 pass,
                 array_view_const_u8 user,
                 array_view_const_u8 domain,
                 array_view_u8 buff) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext NTOWFv2");
        SslMd4 md4;
        uint8_t md4password[SslMd4::DIGEST_LENGTH] = {};

        md4.update(pass);
        md4.final(md4password);

        SslHMAC_Md5 hmac_md5(make_array_view(md4password));

        auto unique_userup = std::make_unique<uint8_t[]>(user.size());
        uint8_t * userup = unique_userup.get();
        memcpy(userup, user.data(), user.size());
        UTF16Upper(userup, user.size());
        hmac_md5.update({userup, user.size()});
        unique_userup.reset();

        uint8_t tmp_md5[SslMd5::DIGEST_LENGTH] = {};

        userup = nullptr;
        hmac_md5.update(domain);
        hmac_md5.final(tmp_md5);
        // TODO: check if buff_size is SslMd5::DIGEST_LENGTH
        // if it is so no need to use a temporary variable
        // and copy digest afterward.
        memset(buff.data(), 0, buff.size());
        memcpy(buff.data(), tmp_md5, std::min(buff.size(), size_t(SslMd5::DIGEST_LENGTH)));
    }

    // all strings are in unicode utf16
    void LMOWFv2(array_view_const_u8 pass,
                 array_view_const_u8 user,
                 array_view_const_u8 domain,
                 array_view_u8 buff) {
        NTOWFv2(pass, user, domain, buff);
    }

    // client method
    // ntlmv2_compute_response_from_challenge generates :
    // - timestamp
    // - client challenge
    // - NtChallengeResponse
    // - LmChallengeResponse
    // all strings are in unicode utf16
    void ntlmv2_compute_response_from_challenge(array_view_const_u8 pass,
                                                array_view_const_u8 user,
                                                array_view_const_u8 domain) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute response from challenge");

        uint8_t ResponseKeyNT[16] = {};
        uint8_t ResponseKeyLM[16] = {};
        this->NTOWFv2(pass, user, domain, make_array_view(ResponseKeyNT));
        this->LMOWFv2(pass, user, domain, make_array_view(ResponseKeyLM));

        // struct NTLMv2_Client_Challenge = temp
        // temp = { 0x01, 0x01, Z(6), Time, ClientChallenge, Z(4), ServerName , Z(4) }
        // Z(n) = { 0x00, ... , 0x00 } n times
        // ServerName = AvPairs received in Challenge message
        auto & AvPairsStream = this->CHALLENGE_MESSAGE.TargetInfo.buffer;
        // BStream AvPairsStream;
        // this->CHALLENGE_MESSAGE.AvPairList.emit(AvPairsStream);
        size_t temp_size = 1 + 1 + 6 + 8 + 8 + 4 + AvPairsStream.size() + 4;
        if (this->verbose) {
            LOG(LOG_INFO, "NTLMContext Compute response: AvPairs size %zu", AvPairsStream.size());
            LOG(LOG_INFO, "NTLMContext Compute response: temp size %zu", temp_size);
        }

        auto unique_temp = std::make_unique<uint8_t[]>(temp_size);
        uint8_t* temp = unique_temp.get();
        memset(temp, 0, temp_size);
        temp[0] = 0x01;
        temp[1] = 0x01;
        // compute ClientTimeStamp
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext TimeStamp");
        uint8_t ZeroTimestamp[8] = {};

        if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0) {
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
        }
        else {
            const timeval tv = this->timeobj.get_time();
            OutStream out_stream(this->Timestamp);
            out_stream.out_uint32_le(tv.tv_usec);
            out_stream.out_uint32_le(tv.tv_sec);
        }
        // compute ClientChallenge (nonce(8))
        // /* ClientChallenge is used in computation of LMv2 and NTLMv2 responses */
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Generate Client Challenge");
        this->rand.random(this->ClientChallenge.data(), this->ClientChallenge.size());
        memcpy(&temp[1+1+6], this->Timestamp, 8);
        memcpy(&temp[1+1+6+8], this->ClientChallenge.data(), this->ClientChallenge.size());
        memcpy(&temp[1+1+6+8+8+4], AvPairsStream.data(), AvPairsStream.size());

        // NtProofStr = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                       Concat(ServerChallenge, temp))

        uint8_t NtProofStr[SslMd5::DIGEST_LENGTH] = {};
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute response: NtProofStr");
        SslHMAC_Md5 hmac_md5resp(make_array_view(ResponseKeyNT));

        this->ServerChallenge = this->CHALLENGE_MESSAGE.serverChallenge;
        hmac_md5resp.update(this->ServerChallenge);
        hmac_md5resp.update({temp, temp_size});
        hmac_md5resp.final(NtProofStr);

        // NtChallengeResponse = Concat(NtProofStr, temp)

        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute response: NtChallengeResponse");
        auto & NtChallengeResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        // BStream & NtChallengeResponse = this->BuffNtChallengeResponse;
        NtChallengeResponse.assign(NtProofStr, NtProofStr + sizeof(NtProofStr));
        NtChallengeResponse.insert(std::end(NtChallengeResponse), temp, temp+temp_size);

        LOG_IF(this->verbose, LOG_INFO, "Compute response: NtChallengeResponse Ready");

        unique_temp.reset();

        LOG_IF(this->verbose, LOG_INFO, "Compute response: temp buff successfully deleted");
        // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
        //                                         Concat(ServerChallenge, ClientChallenge))
        // LmChallengeResponse.ChallengeFromClient = ClientChallenge
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute response: LmChallengeResponse");
        auto & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
        // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
        SslHMAC_Md5 hmac_md5lmresp(make_array_view(ResponseKeyLM));
        hmac_md5lmresp.update(this->ServerChallenge);
        hmac_md5lmresp.update(this->ClientChallenge);
        uint8_t LCResponse[SslMd5::DIGEST_LENGTH] = {};
        hmac_md5lmresp.final(LCResponse);

        LmChallengeResponse.assign(LCResponse, LCResponse+SslMd5::DIGEST_LENGTH);
        LmChallengeResponse.insert(std::end(LmChallengeResponse), this->ClientChallenge.data(), this->ClientChallenge.data()+this->ClientChallenge.size());

        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute response: SessionBaseKey");
        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        this->SessionBaseKey = HmacMd5(make_array_view(ResponseKeyNT), {NtProofStr, sizeof(NtProofStr)});
    }

    // client method for authenticate message
    void ntlm_encrypt_random_session_key() {
        // EncryptedRandomSessionKey = RC4K(KeyExchangeKey, ExportedSessionKey)
        // ExportedSessionKey = NONCE(16) (random 16bytes number)
        // KeyExchangeKey = SessionBaseKey
        // EncryptedRandomSessionKey = RC4K(SessionBaseKey, NONCE(16))

        // generate NONCE(16) exportedsessionkey
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Encrypt RandomSessionKey");
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Generate Exported Session Key");
        this->rand.random(this->ExportedSessionKey, 16);

        SslRC4 rc4;
        rc4.set_key({this->SessionBaseKey.data(), 16});
        rc4.crypt(16, this->ExportedSessionKey, this->EncryptedRandomSessionKey);

        auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
        AuthEncryptedRSK.assign(this->EncryptedRandomSessionKey, this->EncryptedRandomSessionKey+16);
    }
    // server method to decrypt exported session key from authenticate message with
    // session base key computed with Responses.
    void ntlm_decrypt_exported_session_key() {
        auto & AuthEncryptedRSK = this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer;
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Decrypt RandomSessionKey");
        memcpy(this->EncryptedRandomSessionKey, AuthEncryptedRSK.data(),
               AuthEncryptedRSK.size());
               
        SslRC4 rc4;
        rc4.set_key({this->SessionBaseKey.data(), 16});
        rc4.crypt(16, this->EncryptedRandomSessionKey, this->ExportedSessionKey);
    }

    /**
     * Generate signing key.\n
     * @msdn{cc236711}
     * @param sign_magic Sign magic string
     * @param signing_key Destination signing key
     */

    void ntlm_generate_signing_key(array_view_const_u8 sign_magic, uint8_t (&signing_key)[SslMd5::DIGEST_LENGTH])
    {
        SslMd5 md5sign;
        md5sign.update({this->ExportedSessionKey, 16});
        md5sign.update(sign_magic);
        md5sign.final(signing_key);
    }


    /**
     * Generate client signing key (ClientSigningKey).\n
     * @msdn{cc236711}
     */

    void ntlm_generate_client_signing_key()
    {
        this->ntlm_generate_signing_key(make_array_view(client_sign_magic),
                                        this->ClientSigningKey);
    }

    /**
     * Generate server signing key (ServerSigningKey).\n
     * @msdn{cc236711}
     */

    void ntlm_generate_server_signing_key()
    {
        this->ntlm_generate_signing_key(make_array_view(server_sign_magic),
                                        this->ServerSigningKey);
    }


    /**
     * Generate sealing key.\n
     * @msdn{cc236712}
     * @param seal_magic Seal magic string
     * @param sealing_key Destination sealing key
     */

    void ntlm_generate_sealing_key(array_view_const_u8 seal_magic, uint8_t (&sealing_key)[SslMd5::DIGEST_LENGTH])
    {
        SslMd5 md5seal;
        md5seal.update(make_array_view(this->ExportedSessionKey));
        md5seal.update(seal_magic);
        md5seal.final(sealing_key);
    }

    /**
     * Generate client sealing key (ClientSealingKey).\n
     * @msdn{cc236712}
     */

    void ntlm_generate_client_sealing_key()
    {
        ntlm_generate_sealing_key(make_array_view(client_seal_magic), this->ClientSealingKey);
    }

    /**
     * Generate server sealing key (ServerSealingKey).\n
     * @msdn{cc236712}
     */

    void ntlm_generate_server_sealing_key()
    {
        ntlm_generate_sealing_key(make_array_view(server_seal_magic), this->ServerSealingKey);
    }

    void ntlm_compute_MIC() {
        SslHMAC_Md5 hmac_md5resp(make_array_view(this->ExportedSessionKey));
        hmac_md5resp.update(this->SavedNegotiateMessage);
        hmac_md5resp.update(this->SavedChallengeMessage);
        hmac_md5resp.update(this->SavedAuthenticateMessage);
        hmac_md5resp.final(this->MessageIntegrityCheck);
    }


    // all strings are in unicode utf16
    //void ntlm_compute_lm_v2_response(const uint8_t * pass,   size_t pass_size,
    //                                 const uint8_t * user,   size_t user_size,
    //                                 const uint8_t * domain, size_t domain_size)
    //{
    //    uint8_t ResponseKeyLM[16] = {};
    //    this->LMOWFv2(pass, pass_size, user, user_size, domain, domain_size,
    //            ResponseKeyLM, sizeof(ResponseKeyLM));
    //    // LmChallengeResponse.Response = HMAC_MD5(LMOWFv2(password, user, userdomain),
    //    //                                         Concat(ServerChallenge, ClientChallenge))
    //    // LmChallengeResponse.ChallengeFromClient = ClientChallenge
    //    BStream & LmChallengeResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
    //    // BStream & LmChallengeResponse = this->BuffLmChallengeResponse;
    //    SslHMAC_Md5 hmac_md5lmresp(make_array_view(ResponseKeyLM));
    //    LmChallengeResponse.reset();
    //    hmac_md5lmresp.update({this->ServerChallenge, 8});
    //    hmac_md5lmresp.update({this->ClientChallenge, 8});
    //    uint8_t LCResponse[SslMd5::DIGEST_LENGTH] = {};
    //    hmac_md5lmresp.final(LCResponse);
    //    LmChallengeResponse.out_copy_bytes(LCResponse, 16);
    //    LmChallengeResponse.out_copy_bytes(this->ClientChallenge, 8);
    //    LmChallengeResponse.mark_end();
    //}



    /**
     * Initialize RC4 stream cipher states for sealing.
     */

    void ntlm_init_rc4_seal_states()
    {
        if (this->server) {
            this->SendSigningKey = &this->ServerSigningKey;
            this->RecvSigningKey = &this->ClientSigningKey;
            this->SendSealingKey = &this->ClientSealingKey;
            this->RecvSealingKey = &this->ServerSealingKey;
        }
        else {
            this->SendSigningKey = &this->ClientSigningKey;
            this->RecvSigningKey = &this->ServerSigningKey;
            this->SendSealingKey = &this->ServerSealingKey;
            this->RecvSealingKey = &this->ClientSealingKey;
        }
        this->SendRc4Seal.set_key(make_array_view(*this->RecvSealingKey));
        this->RecvRc4Seal.set_key(make_array_view(*this->SendSealingKey));
    }

    // server check nt response
    bool ntlm_check_nt_response_from_authenticate(array_view_const_u8 hash) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Check NtResponse");
        auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        // LOG(LOG_INFO, "tmp size = %u", temp_size);
        uint8_t NtProofStr_from_msg[16] = {};
        memcpy(NtProofStr_from_msg, AuthNtResponse.data(), 16);

        uint8_t NtProofStr[SslMd5::DIGEST_LENGTH] = {};
        uint8_t ResponseKeyNT[16] = {};
        // LOG(LOG_INFO, "NTLM CHECK NT RESPONSE FROM AUTHENTICATE");
        // LOG(LOG_INFO, "UserName size = %u", UserName.size());
        // LOG(LOG_INFO, "DomainName size = %u", DomainName.size());
        // LOG(LOG_INFO, "hash size = %u", hash_size);

        this->NTOWFv2_FromHash(hash, UserName, DomainName, ResponseKeyNT);
        // LOG(LOG_INFO, "ResponseKeyNT");
        // hexdump_c(ResponseKeyNT, sizeof(ResponseKeyNT));
        SslHMAC_Md5 hmac_md5resp(make_array_view(ResponseKeyNT));
        hmac_md5resp.update(this->ServerChallenge);
        hmac_md5resp.update({AuthNtResponse.data()+16, AuthNtResponse.size() - 16});
        hmac_md5resp.final(NtProofStr);

        return !memcmp(NtProofStr, NtProofStr_from_msg, 16);
//        return true;
    }

    // Server check lm response
    bool ntlm_check_lm_response_from_authenticate(array_view_const_u8 hash) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Check LmResponse");
        auto & AuthLmResponse = this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        size_t lm_response_size = AuthLmResponse.size(); // should be 24
        if (lm_response_size != 24) {
            return false;
        }
        uint8_t response[16] = {};
        memcpy(response, AuthLmResponse.data(), 16);
        memcpy(this->ClientChallenge.data(), AuthLmResponse.data()+16, 8);

        uint8_t compute_response[SslMd5::DIGEST_LENGTH] = {};
        uint8_t ResponseKeyLM[16] = {};
        this->NTOWFv2_FromHash(hash, UserName, DomainName, ResponseKeyLM);

        SslHMAC_Md5 hmac_md5resp(make_array_view(ResponseKeyLM));
        hmac_md5resp.update(this->ServerChallenge);
        hmac_md5resp.update(this->ClientChallenge);
        hmac_md5resp.final(compute_response);

        return !memcmp(response, compute_response, 16);
    }

    // server compute Session Base Key
    void ntlm_compute_session_base_key(array_view_const_u8 hash) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Compute Session Base Key");
        auto & AuthNtResponse = this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;
        auto & DomainName = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        auto & UserName = this->AUTHENTICATE_MESSAGE.UserName.buffer;
        uint8_t NtProofStr[16] = {};
        memcpy(NtProofStr, AuthNtResponse.data(), 16);
        uint8_t ResponseKeyNT[16] = {};
        this->NTOWFv2_FromHash(hash, UserName, DomainName, ResponseKeyNT);
        // SessionBaseKey = HMAC_MD5(NTOWFv2(password, user, userdomain),
        //                           NtProofStr)
        this->SessionBaseKey = HmacMd5(make_array_view(ResponseKeyNT), {NtProofStr, sizeof(NtProofStr)});
    }


    // server method
    bool ntlm_check_nego() {
        uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        uint32_t const mask = NTLMSSP_REQUEST_TARGET
                            | NTLMSSP_NEGOTIATE_NTLM
                            | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                            | NTLMSSP_NEGOTIATE_UNICODE;
        if ((negoFlag & mask) != mask) {
            return false;
        }
        this->NegotiateFlags = negoFlag;
        return true;
    }

    void ntlm_set_negotiate_flags() {
        uint32_t & negoFlag = this->NegotiateFlags;
        if (this->NTLMv2) {
            negoFlag |= NTLMSSP_NEGOTIATE_56;
            negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
            negoFlag |= NTLMSSP_NEGOTIATE_LM_KEY;
            negoFlag |= NTLMSSP_NEGOTIATE_OEM;
        }

        negoFlag |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        negoFlag |= NTLMSSP_NEGOTIATE_128;
        negoFlag |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
        negoFlag |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        negoFlag |= NTLMSSP_NEGOTIATE_NTLM;
        negoFlag |= NTLMSSP_NEGOTIATE_SIGN;
        negoFlag |= NTLMSSP_REQUEST_TARGET;
        negoFlag |= NTLMSSP_NEGOTIATE_UNICODE;

        if (this->confidentiality) {
            negoFlag |= NTLMSSP_NEGOTIATE_SEAL;
        }

        if (this->SendVersionInfo) {
            negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
        }

        if (negoFlag & NTLMSSP_NEGOTIATE_VERSION) {
            // this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_5;
            // this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            // this->version.ProductBuild        = 2600;
            // this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
            this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->version.ProductBuild        = 7601;
            this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
        }

        this->NegotiateFlags = negoFlag;
        this->NEGOTIATE_MESSAGE.negoFlags.flags = negoFlag;
    }

    void ntlm_set_negotiate_flags_auth() {
        uint32_t negoFlag = 0;
        if (this->NTLMv2) {
            negoFlag |= NTLMSSP_NEGOTIATE_56;
            if (this->SendVersionInfo) {
                negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
            }
        }

        if (this->UseMIC) {
            negoFlag |= NTLMSSP_NEGOTIATE_TARGET_INFO;
        }
        if (this->SendWorkstationName) {
            negoFlag |= NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED;
        }
        if (this->confidentiality) {
            negoFlag |= NTLMSSP_NEGOTIATE_SEAL;
        }
        if (this->CHALLENGE_MESSAGE.negoFlags.flags & NTLMSSP_NEGOTIATE_KEY_EXCH) {
            negoFlag |= NTLMSSP_NEGOTIATE_KEY_EXCH;
        }
        negoFlag |= NTLMSSP_NEGOTIATE_128;
        negoFlag |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
        negoFlag |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        negoFlag |= NTLMSSP_NEGOTIATE_NTLM;
        negoFlag |= NTLMSSP_NEGOTIATE_SIGN;
        negoFlag |= NTLMSSP_REQUEST_TARGET;
        negoFlag |= NTLMSSP_NEGOTIATE_UNICODE;

        // if (this->SendVersionInfo) {
        //     negoFlag |= NTLMSSP_NEGOTIATE_VERSION;
        // }

        if (negoFlag & NTLMSSP_NEGOTIATE_VERSION) {
            // this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_5;
            // this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            // this->version.ProductBuild        = 2600;
            // this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
            this->version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->version.ProductBuild        = 7601;
            this->version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;

        }

        this->NegotiateFlags = negoFlag;
        this->AUTHENTICATE_MESSAGE.negoFlags.flags = negoFlag;
    }



    // server method
    // TODO COMPLETE
    void ntlm_construct_challenge_target_info() {
    }



    // CLIENT BUILD NEGOTIATE
    void ntlm_client_build_negotiate() {
        if (this->server) {
            return;
        }
        this->ntlm_set_negotiate_flags();
        this->NEGOTIATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;
        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            // this->NEGOTIATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_5;
            // this->NEGOTIATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            // this->NEGOTIATE_MESSAGE.version.ProductBuild        = 2600;
            // this->NEGOTIATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
            this->NEGOTIATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->NEGOTIATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->NEGOTIATE_MESSAGE.version.ProductBuild        = 7601;
            this->NEGOTIATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED) {
            this->Workstation = this->NEGOTIATE_MESSAGE.Workstation.buffer;
        }

        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED) {
            auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
            auto domain_av = this->identity.get_domain_utf16_av();
            domain.assign(domain_av.data(), domain_av.data()+domain_av.size());
        }

        this->state = NTLM_STATE_CHALLENGE;
    }

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    void ntlm_server_build_challenge() {
        if (!this->server) {
            return;
        }

        uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        uint32_t const mask = NTLMSSP_REQUEST_TARGET
                            | NTLMSSP_NEGOTIATE_NTLM
                            | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                            | NTLMSSP_NEGOTIATE_UNICODE;
        if ((negoFlag & mask) != mask) {
            LOG(LOG_ERR, "ERROR CHECK NEGO FLAGS");
        }
        this->NegotiateFlags = negoFlag;

        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Generate Server Challenge");
        this->rand.random(this->ServerChallenge.data(), this->ServerChallenge.size());
        this->CHALLENGE_MESSAGE.serverChallenge = this->ServerChallenge;
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext TimeStamp");
        uint8_t ZeroTimestamp[8] = {};

        if (memcmp(ZeroTimestamp, this->ChallengeTimestamp, 8) != 0) {
            memcpy(this->Timestamp, this->ChallengeTimestamp, 8);
        }
        else {
            const timeval tv = this->timeobj.get_time();
            OutStream out_stream(this->Timestamp);
            out_stream.out_uint32_le(tv.tv_usec);
            out_stream.out_uint32_le(tv.tv_sec);
        }

        std::vector<uint8_t> win7{ 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00 };
        std::vector<uint8_t> upwin7{ 0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00 };

        auto & list = this->CHALLENGE_MESSAGE.AvPairList;
        list.push_back({MsvAvNbComputerName, upwin7});
        list.push_back({MsvAvNbDomainName, upwin7});
        list.push_back({MsvAvDnsComputerName, win7});
        list.push_back({MsvAvDnsDomainName, win7});
        list.push_back({MsvAvTimestamp, std::vector<uint8_t>(this->Timestamp, this->Timestamp+sizeof(this->Timestamp))});

        this->CHALLENGE_MESSAGE.negoFlags.flags = this->NegotiateFlags;
        if (this->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
            this->CHALLENGE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->CHALLENGE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->CHALLENGE_MESSAGE.version.ProductBuild        = 7601;
            this->CHALLENGE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
        }

        this->state = NTLM_STATE_AUTHENTICATE;
    }

    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE
    // all strings are in unicode utf16
    void ntlm_client_build_authenticate(array_view_const_u8 password,
                                        array_view_const_u8 userName,
                                        array_view_const_u8 userDomain,
                                        array_view_const_u8 workstation) {
        if (this->server) {
            return;
        }
        this->ntlmv2_compute_response_from_challenge(password, userName, userDomain);
        this->ntlm_encrypt_random_session_key();
        this->ntlm_generate_client_signing_key();
        this->ntlm_generate_client_sealing_key();
        this->ntlm_generate_server_signing_key();
        this->ntlm_generate_server_sealing_key();
        this->ntlm_init_rc4_seal_states();
        this->ntlm_set_negotiate_flags_auth();
        // this->AUTHENTICATE_MESSAGE.negoFlags.flags = this->NegotiateFlags;

        uint32_t flag = this->AUTHENTICATE_MESSAGE.negoFlags.flags;
        if (flag & NTLMSSP_NEGOTIATE_VERSION) {
            this->AUTHENTICATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
            this->AUTHENTICATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
            this->AUTHENTICATE_MESSAGE.version.ProductBuild        = 7601;
            this->AUTHENTICATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
        }

        if (!(flag & NTLMSSP_NEGOTIATE_KEY_EXCH)) {
            // If flag is not set, encryted session key buffer is not send
            this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer.clear();
        }
        if (flag & NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED) {
            this->AUTHENTICATE_MESSAGE.Workstation.buffer.assign(workstation.data(), workstation.data() + workstation.size());
        }

        //flag |= NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED;
        auto & domain = this->AUTHENTICATE_MESSAGE.DomainName.buffer;
        domain.assign(userDomain.data(),userDomain.data()+userDomain.size());

        this->AUTHENTICATE_MESSAGE.UserName.buffer
            .assign(userName.data(),userName.data()+userName.size());

        // this->AUTHENTICATE_MESSAGE.version.ntlm_get_version_info();

        this->state = NTLM_STATE_FINAL;
    }

    void ntlm_server_fetch_hash(uint8_t (&hash)[SslMd4::DIGEST_LENGTH]) {
        // TODO get password hash from DC or find ourself
        // LOG(LOG_INFO, "MARK %u, %u, %u",
        //     this->identity.User.size(),
        //     this->identity.Domain.size(),
        //     this->identity.Password.size());

        auto password_av = this->identity.get_password_utf16_av();
        if (password_av.size() > 0) {
            // password is available
            this->hash_password(password_av, hash);
        }
    }

    // SERVER PROCEED RESPONSE CHECKING
    SEC_STATUS ntlm_server_proceed_authenticate(const uint8_t (&hash)[16]) {
        if (!this->server) {
            return SEC_E_INTERNAL_ERROR;
        }
        if (!this->ntlm_check_nt_response_from_authenticate(make_array_view(hash))) {
            LOG(LOG_ERR, "NT RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        if (!this->ntlm_check_lm_response_from_authenticate(make_array_view(hash))) {
            LOG(LOG_ERR, "LM RESPONSE NOT MATCHING STOP AUTHENTICATE");
            return SEC_E_LOGON_DENIED;
        }
        // SERVER COMPUTE SHARED KEY WITH CLIENTtests/core/RDP/nla/ntlm/test_ntlm.cpp
        this->ntlm_compute_session_base_key(make_array_view(hash));
        this->ntlm_decrypt_exported_session_key();

        this->ntlm_generate_client_signing_key();
        this->ntlm_generate_client_sealing_key();
        this->ntlm_generate_server_signing_key();
        this->ntlm_generate_server_sealing_key();
        this->ntlm_init_rc4_seal_states();
        if (this->UseMIC) {
            this->ntlm_compute_MIC();
            if (0 != memcmp(this->MessageIntegrityCheck, this->AUTHENTICATE_MESSAGE.MIC, 16)) {
                LOG(LOG_ERR, "MIC NOT MATCHING STOP AUTHENTICATE");
                hexdump_c(this->MessageIntegrityCheck, 16);
                hexdump_c(this->AUTHENTICATE_MESSAGE.MIC, 16);
                return SEC_E_MESSAGE_ALTERED;
            }
        }
        this->state = NTLM_STATE_FINAL;
        return SEC_I_COMPLETE_NEEDED;
    }

    void ntlm_SetContextServicePrincipalName(array_view_const_char pszTargetName) {
        // CHECK UTF8 or UTF16 (should store in UTF16)
        if (!pszTargetName.empty()) {
            size_t host_len = UTF8Len(pszTargetName.data());
            this->ServicePrincipalName = std::vector<uint8_t>(host_len * 2);
            UTF8toUTF16(pszTargetName, this->ServicePrincipalName.data(), host_len * 2);
        }
        else {
            this->ServicePrincipalName.clear();
        }
    }


    // READ WRITE FUNCTIONS
    SEC_STATUS write_negotiate(std::vector<uint8_t>& output_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Write Negotiate");
        this->ntlm_client_build_negotiate();
        auto negotiate = emitNTLMNegotiateMessage();
        output_buffer = negotiate;
        this->SavedNegotiateMessage = std::move(negotiate);
        this->state = NTLM_STATE_CHALLENGE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS read_negotiate(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Read Negotiate");
        this->NEGOTIATE_MESSAGE = recvNTLMNegotiateMessage(input_buffer);

        uint32_t const negoFlag = this->NEGOTIATE_MESSAGE.negoFlags.flags;
        uint32_t const mask = NTLMSSP_REQUEST_TARGET
                            | NTLMSSP_NEGOTIATE_NTLM
                            | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                            | NTLMSSP_NEGOTIATE_UNICODE;
        if ((negoFlag & mask) != mask) {
            return SEC_E_INVALID_TOKEN;
        }
        this->NegotiateFlags = negoFlag;
        this->SavedNegotiateMessage = this->NEGOTIATE_MESSAGE.raw_bytes;

        this->state = NTLM_STATE_CHALLENGE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS write_challenge(std::vector<uint8_t>& output_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Write Challenge");
        this->ntlm_server_build_challenge();
        StaticOutStream<65535> out_stream;
        EmitNTLMChallengeMessage(out_stream, this->CHALLENGE_MESSAGE);
        output_buffer.assign(out_stream.get_bytes().data(),out_stream.get_bytes().data()+out_stream.get_offset());

        this->SavedChallengeMessage.assign(out_stream.get_bytes().data(),out_stream.get_bytes().data()+out_stream.get_offset());

        this->state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS read_challenge(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Read Challenge");
        this->CHALLENGE_MESSAGE = recvNTLMChallengeMessage(input_buffer);
        this->SavedChallengeMessage = this->CHALLENGE_MESSAGE.raw_bytes;

        this->state = NTLM_STATE_AUTHENTICATE;
        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS write_authenticate(std::vector<uint8_t>& output_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Write Authenticate");
        auto password_av = this->identity.get_password_utf16_av();
        auto user_av = this->identity.get_user_utf16_av();
        auto domain_av = this->identity.get_domain_utf16_av();

        this->ntlm_client_build_authenticate(password_av, user_av, domain_av,
                                             this->Workstation);

        size_t mic_offset = 0;
        auto auth_message = emitNTLMAuthenticateMessage(
            this->AUTHENTICATE_MESSAGE.negoFlags.flags,
            this->AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer,
            this->AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer,
            this->AUTHENTICATE_MESSAGE.DomainName.buffer,
            this->AUTHENTICATE_MESSAGE.UserName.buffer,
            this->AUTHENTICATE_MESSAGE.Workstation.buffer,
            this->AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer,
            this->UseMIC,
            mic_offset);

        if (this->UseMIC) {
            this->SavedAuthenticateMessage = auth_message;
            this->ntlm_compute_MIC();
            memcpy(auth_message.data()+mic_offset, this->MessageIntegrityCheck, 16);
        }

        output_buffer = auth_message;
        return SEC_I_COMPLETE_NEEDED;
    }

    SEC_STATUS read_authenticate(array_view_const_u8 input_buffer) {
        LOG_IF(this->verbose, LOG_INFO, "NTLMContext Read Authenticate");
        InStream in_stream(input_buffer);
        recvNTLMAuthenticateMessage(in_stream, this->AUTHENTICATE_MESSAGE);
        if (this->AUTHENTICATE_MESSAGE.has_mic) {
            this->UseMIC = true;
            this->SavedAuthenticateMessage = std::vector<uint8_t>(in_stream.get_offset());
            constexpr std::size_t null_data_sz = 16;
            uint8_t const null_data[null_data_sz]{0u};
            auto const p = in_stream.get_data();
            std::size_t offset = 0u;
            memcpy(this->SavedAuthenticateMessage.data()+offset, p + offset, this->AUTHENTICATE_MESSAGE.PayloadOffset);
            offset += this->AUTHENTICATE_MESSAGE.PayloadOffset;
            memcpy(this->SavedAuthenticateMessage.data()+offset, null_data, null_data_sz);
            offset += null_data_sz;
            memcpy(this->SavedAuthenticateMessage.data()+offset, p + offset, in_stream.get_offset() - offset);
        }

        this->identity.user_init_copy(this->AUTHENTICATE_MESSAGE.UserName.buffer);
        this->identity.domain_init_copy(this->AUTHENTICATE_MESSAGE.DomainName.buffer);

        if (this->identity.is_empty_user_domain()){
            LOG(LOG_ERR, "ANONYMOUS User not allowed");
            return SEC_E_LOGON_DENIED;
        }

        return SEC_I_CONTINUE_NEEDED;
    }

    SEC_STATUS check_authenticate() {
        uint8_t hash[16];
        this->ntlm_server_fetch_hash(hash);
        return this->ntlm_server_proceed_authenticate(hash);
    }
};


RED_AUTO_TEST_CASE(TestNtlmContext)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj, true);
    // context.init();
    context.ntlm_set_negotiate_flags();
    // context.hardcoded_tests = true;

    // NtlmNegotiateFlags ntlm_nego_flag;

    // ntlm_nego_flag.flags = context.ConfigFlags;
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_56;  // W
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_VERSION;  // T
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_LM_KEY;  // G
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_SEAL;  // E
    // ntlm_nego_flag.flags |= NTLMSSP_NEGOTIATE_OEM;  // B

    // hexdump_c((uint8_t*)&ntlm_nego_flag.flags, 4);

    auto nego_string =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08\xe2"
        /* 0010 */ "\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00\x00"
        /* 0020 */ "\x05\x01\x28\x0a\x00\x00\x00\x0f"_av;

    auto challenge_string =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x08\x00\x08\x00"
        /* 0010 */ "\x38\x00\x00\x00\x35\x82\x8a\xe2\x26\x6e\xcd\x75\xaa\x41\xe7\x6f"
        /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00"
        /* 0030 */ "\x06\x01\xb0\x1d\x00\x00\x00\x0f\x57\x00\x49\x00\x4e\x00\x37\x00"
        /* 0040 */ "\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00"
        /* 0050 */ "\x57\x00\x49\x00\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00"
        /* 0060 */ "\x6e\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00"
        /* 0070 */ "\x07\x00\x08\x00\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00"_av;

    context.NEGOTIATE_MESSAGE = recvNTLMNegotiateMessage(nego_string);

    context.CHALLENGE_MESSAGE = recvNTLMChallengeMessage(challenge_string);

    const uint8_t password[] = {
        // 0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        // 0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    const uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    // const uint8_t userUpper[] = {
    //     // 0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x4e, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x45, 0x00
    // };
    const uint8_t userDomain[] = {
        // 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        // 0x69, 0x00, 0x6e, 0x00
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };



    context.ntlmv2_compute_response_from_challenge(make_array_view(password),
                                                   make_array_view(userName),
                                                   make_array_view(userDomain));

    auto & LmChallengeResponse = context.AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer;
    RED_CHECK_MEM(LmChallengeResponse,
        /* 0000 */ "\x11\x1b\x69\x4b\xdb\x30\x53\x91\xef\x94\x8b\x20\x83\xbd\x07\x43" //..iK.0S.... ...C
        /* 0010 */ "\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d"_av                                 //.l....0.
    );
    auto & NtChallengeResponse = context.AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer;

    RED_CHECK_MEM(NtChallengeResponse,
        "\x54\x01\x2a\xc9\x4e\x20\x30\x7d\xed\x6a\xcf\xb8\x6b\xb0\x45\xc5" //T.*.N 0}.j..k.E. !
        "\x01\x01\x00\x00\x00\x00\x00\x00\x67\x95\x0e\x5a\x4e\x56\x76\xd6" //........g..ZNVv. !
        "\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\x00\x00\x00\x00\x02\x00\x08\x00" //.l....0......... !
        "\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00\x57\x00\x49\x00" //W.I.N.7.....W.I. !
        "\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00" //N.7.....w.i.n.7. !
        "\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00" //....w.i.n.7..... !
        "\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00\x00\x00\x00\x00"_av //....l........... !
    );
    RED_CHECK_MEM_AA(
        context.SessionBaseKey,
        "\x1b\x76\xfd\xe3\x46\x77\x60\x04\x39\x7a\x47\x8a\x60\x92\x0c\x4c"_av
    );

    context.ntlm_encrypt_random_session_key();

    RED_CHECK_MEM_AA(
        context.EncryptedRandomSessionKey,
        "\x5a\xcb\x6c\xba\x58\x07\xb5\xd4\xf4\x61\x65\xfb\xb0\x9a\xe7\xc6"_av
    );

    context.ntlm_generate_client_signing_key();
    RED_CHECK_MEM_AA(
        context.ClientSigningKey,
        /* 0000 */ "\xb6\x18\x61\x5b\xdb\x97\x6c\x62\xfd\xd5\x72\xab\x37\x24\xd1\x38"_av //..a[..lb..r.7$.8
    );
    context.ntlm_generate_client_sealing_key();
    RED_CHECK_MEM_AA(
        context.ClientSealingKey,
        /* 0000 */ "\x02\x46\xea\x18\xc8\xba\x71\xf3\xc1\x06\xb9\xf0\x54\x37\x44\x01"_av //.F....q.....T7D.
    );
    context.ntlm_generate_server_signing_key();
    RED_CHECK_MEM_AA(
        context.ServerSigningKey,
      /* 0000 */ "\x56\x66\xbd\xc3\x82\xda\xb7\x70\x08\x36\xb3\xed\xcd\x67\x8b\x5a"_av //Vf.....p.6...g.Z
    );
    context.ntlm_generate_server_sealing_key();
    RED_CHECK_MEM_AA(
        context.ServerSealingKey,
        /* 0000 */ "\x19\x3a\x3f\x24\x89\x27\xd3\x8b\x4b\xf5\x63\x2d\xa4\xc2\xb2\x78"_av //.:?$.'..K.c-...x
    );
}
RED_AUTO_TEST_CASE(TestNTOWFv2)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj);
    uint8_t buff[16];

    constexpr auto password = "Password"_av;
    constexpr auto user = "User"_av;
    constexpr auto domain = "Domain"_av;

    uint8_t upassword[password.size() * 2];
    uint8_t uuser[user.size() * 2];
    uint8_t udomain[domain.size() * 2];
    UTF8toUTF16(password, upassword, sizeof(upassword));
    UTF8toUTF16(user, uuser, sizeof(uuser));
    UTF8toUTF16(domain, udomain, sizeof(udomain));

    context.NTOWFv2(make_array_view(upassword),
                    make_array_view(uuser),
                    make_array_view(udomain),
                    make_array_view(buff));
    RED_CHECK_MEM_AA(buff, "\x0c\x86\x8a\x40\x3b\xfd\x7a\x93\xa3\x00\x1e\xf2\x2e\xf0\x2e\x3f"_av);
}

RED_AUTO_TEST_CASE(TestSetters)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context(false, rand, timeobj);
    // context.init();

    auto work = "Carpe Diem"_av;
    auto spn = "Sustine et abstine"_av;

    RED_CHECK_EQUAL(context.Workstation.size(), 0);

    array_view_const_char workstation = work;
    // CHECK UTF8 or UTF16 (should store in UTF16)
    context.SendWorkstationName = !workstation.empty();
    context.Workstation.clear();
    if (context.SendWorkstationName) {
        size_t host_len = UTF8Len(workstation.data());
        context.Workstation.resize(host_len*2);
        UTF8toUTF16(workstation, context.Workstation.data(), host_len * 2);
    }

    RED_CHECK_EQUAL(context.Workstation.size(), work.size() * 2);
    // TODO TEST bad test
    RED_CHECK(memcmp(work.data(), context.Workstation.data(), work.size()+1));

    RED_CHECK_EQUAL(context.ServicePrincipalName.size(), 0);
    context.ntlm_SetContextServicePrincipalName(spn);
    RED_CHECK_EQUAL(context.ServicePrincipalName.size(), spn.size() * 2);
    // TODO TEST bad test
    RED_CHECK(memcmp(spn.data(), context.ServicePrincipalName.data(), spn.size()+1));

}


RED_AUTO_TEST_CASE(TestNtlmNewtest)
{
    RED_CHECK_EQUAL(1,1);
}



RED_AUTO_TEST_CASE(TestNtlmScenario)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext client_context(false, rand, timeobj, true);
    NTLMContext server_context(true, rand, timeobj, true);
    server_context.SessionBaseKey = {};
    client_context.SessionBaseKey = {};


    const uint8_t password[] = {
        0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00,
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    const uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    // const uint8_t userUpper[] = {
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
    //     0x4e, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x45, 0x00
    // };
    const uint8_t userDomain[] = {
        0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        0x69, 0x00, 0x6e, 0x00,
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };

    const uint8_t workstation[] = {
        0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x58, 0x00, 0x50, 0x00
    };

    // Initialization
    bool result;

    // CLIENT BUILDS NEGOTIATE
    client_context.ntlm_set_negotiate_flags();
    client_context.NEGOTIATE_MESSAGE.negoFlags.flags = client_context.NegotiateFlags;
    if (client_context.NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
        client_context.NEGOTIATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
        client_context.NEGOTIATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
        client_context.NEGOTIATE_MESSAGE.version.ProductBuild        = 7601;
        client_context.NEGOTIATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
    }

    // send NEGOTIATE MESSAGE
    auto negotiate = emitNTLMNegotiateMessage();
    server_context.NEGOTIATE_MESSAGE = recvNTLMNegotiateMessage(negotiate);

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    uint32_t const negoFlag = server_context.NEGOTIATE_MESSAGE.negoFlags.flags;
    uint32_t const mask = NTLMSSP_REQUEST_TARGET
                        | NTLMSSP_NEGOTIATE_NTLM
                        | NTLMSSP_NEGOTIATE_ALWAYS_SIGN
                        | NTLMSSP_NEGOTIATE_UNICODE;
    if ((negoFlag & mask) != mask) {
        result = false;
    }
    else {
        server_context.NegotiateFlags = negoFlag;
        result = true;
    }
    RED_CHECK(result);
    LOG_IF(server_context.verbose, LOG_INFO, "NTLMContext Generate Server Challenge");
    server_context.rand.random(server_context.ServerChallenge.data(), server_context.ServerChallenge.size());

    server_context.ServerChallenge = server_context.CHALLENGE_MESSAGE.serverChallenge;

    LOG_IF(server_context.verbose, LOG_INFO, "NTLMContext TimeStamp");
    uint8_t ZeroTimestamp[8] = {};

    if (memcmp(ZeroTimestamp, server_context.ChallengeTimestamp, 8) != 0) {
        memcpy(server_context.Timestamp, server_context.ChallengeTimestamp, 8);
    }
    else {
        const timeval tv = server_context.timeobj.get_time();
        OutStream out_stream(server_context.Timestamp);
        out_stream.out_uint32_le(tv.tv_usec);
        out_stream.out_uint32_le(tv.tv_sec);
    }


    uint8_t win7[] =  {0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00};
    uint8_t upwin7[] = {0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00};
    
    auto clone_av_to_vector = [](bytes_view data) -> std::vector<uint8_t> {
        return std::vector<uint8_t>(data.data(), data.data()+data.size());
    };

    auto & list = server_context.CHALLENGE_MESSAGE.AvPairList;
    list.push_back({MsvAvNbComputerName, clone_av_to_vector(buffer_view(upwin7))});
    list.push_back({MsvAvNbDomainName, clone_av_to_vector(buffer_view(upwin7))});
    list.push_back({MsvAvDnsComputerName, clone_av_to_vector(buffer_view(win7))});
    list.push_back({MsvAvDnsDomainName, clone_av_to_vector(buffer_view(win7))});
    list.push_back({MsvAvTimestamp, clone_av_to_vector(buffer_view(server_context.Timestamp))});

    server_context.CHALLENGE_MESSAGE.negoFlags.flags = server_context.NegotiateFlags;
    if (server_context.NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION) {
        server_context.NEGOTIATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
        server_context.NEGOTIATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
        server_context.NEGOTIATE_MESSAGE.version.ProductBuild        = 7601;
        server_context.NEGOTIATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
    }

    // send CHALLENGE MESSAGE
    uint8_t server_to_client_buf[65535];
    OutStream out_server_to_client(server_to_client_buf);

    EmitNTLMChallengeMessage(out_server_to_client, server_context.CHALLENGE_MESSAGE);
    client_context.CHALLENGE_MESSAGE = recvNTLMChallengeMessage(out_server_to_client.get_bytes());

    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE

    client_context.ntlmv2_compute_response_from_challenge(make_array_view(password),
                                                          make_array_view(userName),
                                                          make_array_view(userDomain));

    client_context.ntlm_encrypt_random_session_key();
    client_context.ntlm_generate_client_signing_key();
    client_context.ntlm_generate_client_sealing_key();
    client_context.ntlm_generate_server_signing_key();
    client_context.ntlm_generate_server_sealing_key();
    client_context.AUTHENTICATE_MESSAGE.negoFlags.flags = client_context.NegotiateFlags;

    uint32_t const flag = client_context.AUTHENTICATE_MESSAGE.negoFlags.flags;
    if (flag & NTLMSSP_NEGOTIATE_VERSION) {
        client_context.NEGOTIATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
        client_context.NEGOTIATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
        client_context.NEGOTIATE_MESSAGE.version.ProductBuild        = 7601;
        client_context.NEGOTIATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;
    }

    if (flag & NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED) {
        client_context.AUTHENTICATE_MESSAGE.Workstation.buffer.assign(workstation, workstation + sizeof(workstation));
    }

    auto & domain = client_context.AUTHENTICATE_MESSAGE.DomainName.buffer;
    domain.assign(userDomain, userDomain + sizeof(userDomain));

    auto & user = client_context.AUTHENTICATE_MESSAGE.UserName.buffer;
    user.assign(userName, userName + sizeof(userName));

    client_context.AUTHENTICATE_MESSAGE.version.ProductMajorVersion = WINDOWS_MAJOR_VERSION_6;
    client_context.AUTHENTICATE_MESSAGE.version.ProductMinorVersion = WINDOWS_MINOR_VERSION_1;
    client_context.AUTHENTICATE_MESSAGE.version.ProductBuild        = 7601;
    client_context.AUTHENTICATE_MESSAGE.version.NtlmRevisionCurrent = NTLMSSP_REVISION_W2K3;


    RED_CHECK_EQUAL(client_context.AUTHENTICATE_MESSAGE.has_mic, true);
    // send AUTHENTICATE MESSAGE
    StaticOutStream<65535> out_client_to_server;
    size_t mic_offset = 0;
    auto auth_message = emitNTLMAuthenticateMessage(
                        client_context.AUTHENTICATE_MESSAGE.negoFlags.flags,
                        client_context.AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer,
                        client_context.AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer,
                        client_context.AUTHENTICATE_MESSAGE.DomainName.buffer,
                        client_context.AUTHENTICATE_MESSAGE.UserName.buffer,
                        client_context.AUTHENTICATE_MESSAGE.Workstation.buffer,
                        client_context.AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer,
                        client_context.AUTHENTICATE_MESSAGE.has_mic,
                        mic_offset);
    memcpy(auth_message.data()+mic_offset, client_context.AUTHENTICATE_MESSAGE.MIC, 16);

    
    InStream in_client_to_server(auth_message);
    recvNTLMAuthenticateMessage(in_client_to_server, server_context.AUTHENTICATE_MESSAGE);

    // SERVER PROCEED RESPONSE CHECKING
    uint8_t hash[16] = {};
    server_context.hash_password(make_array_view(password), hash);

    result = server_context.ntlm_check_nt_response_from_authenticate(make_array_view(hash));
    RED_CHECK(result);
    result = server_context.ntlm_check_lm_response_from_authenticate(make_array_view(hash));
    RED_CHECK(result);
    // SERVER COMPUTE SHARED KEY WITH CLIENT
    server_context.ntlm_compute_session_base_key(make_array_view(hash));
    server_context.ntlm_decrypt_exported_session_key();

    server_context.ntlm_generate_client_signing_key();
    server_context.ntlm_generate_client_sealing_key();
    server_context.ntlm_generate_server_signing_key();
    server_context.ntlm_generate_server_sealing_key();

    RED_CHECK_MEM_AA(server_context.SessionBaseKey, client_context.SessionBaseKey);
    RED_CHECK_MEM_AA(server_context.ExportedSessionKey, client_context.ExportedSessionKey);
    RED_CHECK_MEM_AA(server_context.ClientSigningKey, client_context.ClientSigningKey);
    RED_CHECK_MEM_AA(server_context.ClientSealingKey, client_context.ClientSealingKey);
    RED_CHECK_MEM_AA(server_context.ServerSigningKey, client_context.ServerSigningKey);
    RED_CHECK_MEM_AA(server_context.ServerSealingKey, client_context.ServerSealingKey);
}


RED_AUTO_TEST_CASE(TestNtlmScenario2)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext client_context(false, rand, timeobj);
    NTLMContext server_context(true, rand, timeobj);

    const uint8_t password[] = {
        0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00,
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    uint8_t userName[] = {
        0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00,
        0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    };
    const uint8_t userDomain[] = {
        0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        0x69, 0x00, 0x6e, 0x00,
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };

    const uint8_t workstation[] = {
        0x57, 0x00, 0x49, 0x00, 0x4e, 0x00, 0x58, 0x00, 0x50, 0x00
    };

    // Initialization

    // CLIENT BUILDS NEGOTIATE
    client_context.ntlm_client_build_negotiate();

    // send NEGOTIATE MESSAGE
    auto negotiate = emitNTLMNegotiateMessage();

    client_context.SavedNegotiateMessage = negotiate;

    InStream in_client_to_server(negotiate);
    server_context.NEGOTIATE_MESSAGE = recvNTLMNegotiateMessage(negotiate);
    server_context.SavedNegotiateMessage = server_context.NEGOTIATE_MESSAGE.raw_bytes;

    // SERVER RECV NEGOTIATE AND BUILD CHALLENGE
    server_context.ntlm_server_build_challenge();

    // send CHALLENGE MESSAGE
    uint8_t server_to_client_buf[65535];
    OutStream out_server_to_client(server_to_client_buf);

    EmitNTLMChallengeMessage(out_server_to_client, server_context.CHALLENGE_MESSAGE);
    server_context.SavedChallengeMessage = std::vector<uint8_t>(out_server_to_client.get_offset());
    memcpy(server_context.SavedChallengeMessage.data(),
           out_server_to_client.get_data(), out_server_to_client.get_offset());

    InStream in_server_to_client(out_server_to_client.get_bytes());
    client_context.CHALLENGE_MESSAGE = recvNTLMChallengeMessage(out_server_to_client.get_bytes());
    client_context.SavedChallengeMessage = client_context.CHALLENGE_MESSAGE.raw_bytes;

    // CLIENT RECV CHALLENGE AND BUILD AUTHENTICATE

    client_context.ntlm_client_build_authenticate(make_array_view(password),
                                                  make_array_view(userName),
                                                  make_array_view(userDomain),
                                                  make_array_view(workstation));

    // send AUTHENTICATE MESSAGE
    uint8_t client_to_server_buf[65535];
    size_t mic_offset = 0;
    auto auth_message = emitNTLMAuthenticateMessage(client_context.AUTHENTICATE_MESSAGE.negoFlags.flags,
                    client_context.AUTHENTICATE_MESSAGE.LmChallengeResponse.buffer,
                    client_context.AUTHENTICATE_MESSAGE.NtChallengeResponse.buffer,
                    client_context.AUTHENTICATE_MESSAGE.DomainName.buffer,
                    client_context.AUTHENTICATE_MESSAGE.UserName.buffer,
                    client_context.AUTHENTICATE_MESSAGE.Workstation.buffer,
                    client_context.AUTHENTICATE_MESSAGE.EncryptedRandomSessionKey.buffer,
                    client_context.AUTHENTICATE_MESSAGE.has_mic,
                    mic_offset);

    client_context.SavedAuthenticateMessage = auth_message;
    client_context.ntlm_compute_MIC();
    memcpy(client_context.AUTHENTICATE_MESSAGE.MIC, client_context.MessageIntegrityCheck, 16);
    memcpy(auth_message.data()+mic_offset, client_context.MessageIntegrityCheck, 16);

    OutStream out_client_to_server(client_to_server_buf);
    out_client_to_server.out_copy_bytes(auth_message);
    
    in_client_to_server = InStream(out_client_to_server.get_bytes());
    recvNTLMAuthenticateMessage(in_client_to_server, server_context.AUTHENTICATE_MESSAGE);
    if (server_context.AUTHENTICATE_MESSAGE.has_mic) {
        memset(client_to_server_buf +
               server_context.AUTHENTICATE_MESSAGE.PayloadOffset, 0, 16);
        server_context.SavedAuthenticateMessage = std::vector<uint8_t>(in_client_to_server.get_offset());
        memcpy(server_context.SavedAuthenticateMessage.data(),
               in_client_to_server.get_data(), in_client_to_server.get_offset());
    }

    // SERVER PROCEED RESPONSE CHECKING
    uint8_t hash[16] = {};
    server_context.hash_password(make_array_view(password), hash);
    server_context.ntlm_server_proceed_authenticate(hash);

    RED_CHECK_MEM_AA(server_context.SessionBaseKey, client_context.SessionBaseKey);
    RED_CHECK_MEM_AA(server_context.ExportedSessionKey, client_context.ExportedSessionKey);
    RED_CHECK_MEM_AA(server_context.ClientSigningKey, client_context.ClientSigningKey);
    RED_CHECK_MEM_AA(server_context.ClientSealingKey, client_context.ClientSealingKey);
    RED_CHECK_MEM_AA(server_context.ServerSigningKey, client_context.ServerSigningKey);
    RED_CHECK_MEM_AA(server_context.ServerSealingKey, client_context.ServerSealingKey);
    RED_CHECK_MEM_AA(client_context.MessageIntegrityCheck, server_context.MessageIntegrityCheck);
}



RED_AUTO_TEST_CASE(TestWrittersReaders)
{
    LCGRandom rand(0);
    LCGTime timeobj;

    NTLMContext context_write(false, rand, timeobj, true);
    context_write.NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED;
    context_write.NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED;
    NTLMContext context_read(true, rand, timeobj, true);
    SEC_STATUS status;

    std::vector<uint8_t> nego;
    status = context_write.write_negotiate(nego);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_CHALLENGE);
    status = context_read.read_negotiate(nego);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_read.state, NTLM_STATE_CHALLENGE);

    std::vector<uint8_t> chal;
    status = context_write.write_challenge(chal);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_AUTHENTICATE);
    status = context_read.read_challenge(chal);
    RED_CHECK_EQUAL(status, SEC_I_CONTINUE_NEEDED);
    RED_CHECK_EQUAL(context_read.state, NTLM_STATE_AUTHENTICATE);

    std::vector<uint8_t> auth;
    status = context_write.write_authenticate(auth);
    RED_CHECK_EQUAL(status, SEC_I_COMPLETE_NEEDED);
    RED_CHECK_EQUAL(context_write.state, NTLM_STATE_FINAL);
    status = context_read.read_authenticate(auth);
    RED_CHECK_EQUAL(status, SEC_E_LOGON_DENIED);
}






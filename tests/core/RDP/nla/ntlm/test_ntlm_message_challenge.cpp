/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestNtlmMessageChallenge
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "RDP/nla/credssp.hpp"
#include "RDP/nla/ntlm/ntlm_message_challenge.hpp"

#include "ssl_calls.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestChallenge)
{
    BStream s;
    // ===== NTLMSSP_CHALLENGE =====
    uint8_t packet2[] = {
        0x30, 0x81, 0x94, 0xa0, 0x03, 0x02, 0x01, 0x02,
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


    LOG(LOG_INFO, "=================================\n");
    s.init(sizeof(packet2));
    s.out_copy_bytes(packet2, sizeof(packet2));
    s.mark_end();
    s.rewind();

    uint8_t sig[20];
    get_sig(s, sig, sizeof(sig));

    TSRequest ts_req2(s);

    BOOST_CHECK_EQUAL(ts_req2.version, 2);

    BOOST_CHECK_EQUAL(ts_req2.negoTokens.size(), 0x80);
    BOOST_CHECK_EQUAL(ts_req2.authInfo.size(), 0);
    BOOST_CHECK_EQUAL(ts_req2.pubKeyAuth.size(), 0);

    BStream to_send2;

    BOOST_CHECK_EQUAL(to_send2.size(), 0);
    ts_req2.emit(to_send2);

    BOOST_CHECK_EQUAL(to_send2.size(), 0x94 + 3);

    char message[1024];
    if (!check_sig(to_send2, message, (const char *)sig)){
        BOOST_CHECK_MESSAGE(false, message);
    }

    NTLMChallengeMessage ChallengeMsg;

    ChallengeMsg.recv(ts_req2.negoTokens);

    BOOST_CHECK_EQUAL(ChallengeMsg.negoFlags.flags, 0xe28a8235);
    ChallengeMsg.negoFlags.print();

    BOOST_CHECK_EQUAL(ChallengeMsg.TargetName.len, 8);
    BOOST_CHECK_EQUAL(ChallengeMsg.TargetName.bufferOffset, 56);
    hexdump_c(ChallengeMsg.TargetName.Buffer.get_data(),
              ChallengeMsg.TargetName.Buffer.size());
    BOOST_CHECK_EQUAL(ChallengeMsg.TargetInfo.len, 64);
    BOOST_CHECK_EQUAL(ChallengeMsg.TargetInfo.bufferOffset, 64);
    hexdump_c(ChallengeMsg.TargetInfo.Buffer.get_data(),
              ChallengeMsg.TargetInfo.Buffer.size());
    BOOST_CHECK_EQUAL(ChallengeMsg.serverChallenge, 8063485858206805542LL);

    ChallengeMsg.version.print();

    ChallengeMsg.AvPairList.print();

    // // hexdump_c(to_send2.get_data(), to_send2.size());


    {
        LOG(LOG_INFO, "=================================\n");
        SslMd5 md5;
        BStream buff;
        buff.out_uint64_be(ChallengeMsg.serverChallenge);
        buff.mark_end();
        md5.update(buff.get_data(), buff.size());
        uint8_t sigmd5[24];
        md5.final(sigmd5, sizeof(sigmd5));
        hexdump_c(sigmd5, sizeof(sigmd5));
    }


}

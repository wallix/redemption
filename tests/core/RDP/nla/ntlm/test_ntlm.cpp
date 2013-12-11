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
#define BOOST_TEST_MODULE TestNtlm
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#define DISABLE_RANDOM_TESTS
#include "RDP/nla/ntlm/ntlm.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestNtlmContext)
{

    NTLMContext context;

    uint8_t nego_string[] =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08\xe2"
        /* 0010 */ "\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00\x00"
        /* 0020 */ "\x05\x01\x28\x0a\x00\x00\x00\x0f";

    uint8_t challenge_string[] =
        /* 0000 */ "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x08\x00\x08\x00"
        /* 0010 */ "\x38\x00\x00\x00\x35\x82\x8a\xe2\x26\x6e\xcd\x75\xaa\x41\xe7\x6f"
        /* 0020 */ "\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00"
        /* 0030 */ "\x06\x01\xb0\x1d\x00\x00\x00\x0f\x57\x00\x49\x00\x4e\x00\x37\x00"
        /* 0040 */ "\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00"
        /* 0050 */ "\x57\x00\x49\x00\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00"
        /* 0060 */ "\x6e\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00"
        /* 0070 */ "\x07\x00\x08\x00\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00";

    BStream s;
    s.out_copy_bytes(nego_string, sizeof(nego_string) - 1);
    s.mark_end();
    s.rewind();

    context.server = false;
    context.NEGOTIATE_MESSAGE.recv(s);

    s.reset();
    s.out_copy_bytes(challenge_string, sizeof(challenge_string) - 1);
    s.mark_end();
    s.rewind();
    context.CHALLENGE_MESSAGE.recv(s);

    const uint8_t password[] = {
        // 0x50, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        // 0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
        0x70, 0x00, 0x61, 0x00, 0x73, 0x00, 0x73, 0x00,
        0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00
    };
    // uint8_t user[] = {
    //     0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00,
    //     0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00
    // };
    const uint8_t userUpper[] = {
        // 0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00
        0x55, 0x00, 0x53, 0x00, 0x45, 0x00, 0x52, 0x00,
        0x4e, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x45, 0x00
    };
    const uint8_t userDomain[] = {
        // 0x44, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x61, 0x00,
        // 0x69, 0x00, 0x6e, 0x00
        0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00
    };

    context.ntlmv2_compute_response_from_challenge(password, sizeof(password),
                                                   userUpper, sizeof(userUpper),
                                                   userDomain, sizeof(userDomain));

    BStream & LmChallengeResponse = context.AUTHENTICATE_MESSAGE.LmChallengeResponse.Buffer;
    // LOG(LOG_INFO, "==== LmChallengeResponse =====");
    // hexdump_c(LmChallengeResponse.get_data(), LmChallengeResponse.size());
    BOOST_CHECK_EQUAL(memcmp("\xa0\x98\x01\x10\x19\xbb\x5d\x00"
                             "\xf6\xbe\x00\x33\x90\x20\x34\xb3"
                             "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43",
                             LmChallengeResponse.get_data(),
                             LmChallengeResponse.size()),
                      0);
    BStream & NtChallengeResponse = context.AUTHENTICATE_MESSAGE.NtChallengeResponse.Buffer;
    // LOG(LOG_INFO, "==== NtChallengeResponse =====");
    // hexdump_c(NtChallengeResponse.get_data(), NtChallengeResponse.size());
    BOOST_CHECK_EQUAL(memcmp("\x01\x4a\xd0\x8c\x24\xb4\x90\x74\x39\x68\xe8\xbd\x0d\x2b\x70\x10"
                             "\x01\x01\x00\x00\x00\x00\x00\x00\xc3\x83\xa2\x1c\x6c\xb0\xcb\x01"
                             "\x47\xa2\xe5\xcf\x27\xf7\x3c\x43\x00\x00\x00\x00\x02\x00\x08\x00"
                             "\x57\x00\x49\x00\x4e\x00\x37\x00\x01\x00\x08\x00\x57\x00\x49\x00"
                             "\x4e\x00\x37\x00\x04\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00"
                             "\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00"
                             "\xa9\x8d\x9b\x1a\x6c\xb0\xcb\x01\x00\x00\x00\x00\x00\x00\x00\x00",
                             NtChallengeResponse.get_data(),
                             NtChallengeResponse.size()),
                      0);

    // LOG(LOG_INFO, "==== SessionBaseKey =====");
    // hexdump_c(context.SessionBaseKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\x6e\xf1\x6b\x79\x88\xf2\x3d\x7e\x54\x2a\x1a\x38\x4e\xa0\x6b\x52",
                             context.SessionBaseKey,
                             16),
                      0);

    context.ntlm_encrypt_random_session_key();

    // LOG(LOG_INFO, "==== EncryptedRandomSessionKey =====");
    // hexdump_c(context.EncryptedRandomSessionKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\xb1\xd2\x45\x42\x0f\x37\x9a\x0e\xe0\xce\x77\x40\x10\x8a\xda\xba",
                             context.EncryptedRandomSessionKey,
                             16),
                      0);

    context.ntlm_generate_client_signing_key();
    // LOG(LOG_INFO, "==== ClientSigningKey =====");
    // hexdump_c(context.ClientSigningKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\xbf\x5e\x42\x76\x55\x68\x38\x97\x45\xd3\xb4\x9f\x5e\x2f\xbc\x89",
                             context.ClientSigningKey,
                             16),
                      0);
    context.ntlm_generate_client_sealing_key();
    // LOG(LOG_INFO, "==== ClientSealingKey =====");
    // hexdump_c(context.ClientSealingKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\xca\x41\xcd\x08\x48\x07\x22\x6e\x0d\x84\xc3\x88\xa5\x07\xa9\x73",
                             context.ClientSealingKey,
                             16),
                      0);
    context.ntlm_generate_server_signing_key();
    // LOG(LOG_INFO, "==== ServerSigningKey =====");
    // hexdump_c(context.ServerSigningKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\x9b\x3b\x64\x89\xda\x84\x52\x17\xd5\xc2\x6e\x90\x16\x3b\x42\x11",
                             context.ServerSigningKey,
                             16),
                      0);
    context.ntlm_generate_server_sealing_key();
    // LOG(LOG_INFO, "==== ServerSealingKey =====");
    // hexdump_c(context.ServerSealingKey, 16);
    BOOST_CHECK_EQUAL(memcmp("\x14\xb7\x1d\x06\x2c\x68\x2e\xad\x4b\x0e\x95\x23\x70\x91\x98\x90",
                             context.ServerSealingKey,
                             16),
                      0);
}

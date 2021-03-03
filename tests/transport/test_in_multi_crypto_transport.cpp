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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "transport/in_multi_crypto_transport.hpp"

constexpr auto is_encrypted = InCryptoTransport::EncryptionMode::Encrypted;
constexpr auto is_not_encrypted = InCryptoTransport::EncryptionMode::NotEncrypted;

RED_AUTO_TEST_CASE(TestInMultiCryptoTransport1)
{
    CryptoContext cctx;
    InMultiCryptoTransport t(
        {
            FIXTURES_PATH "/sample.mwrm",
            FIXTURES_PATH "/sample_v2.mwrm",
        },
        cctx, is_not_encrypted
    );

    char buffer[300];
    RED_TEST(t.atomic_read({buffer, 164}) == Transport::Read::Ok);
    RED_TEST(array_view(buffer, 164) ==
R"(800 600
0

./tests/fixtures/sample0.wrm 1352304810 1352304870
./tests/fixtures/sample1.wrm 1352304870 1352304930
./tests/fixtures/sample2.wrm 1352304930 1352304990
)"_av);
    RED_TEST(t.atomic_read({buffer, 239}) == Transport::Read::Ok);
    RED_TEST(array_view(buffer, 239) ==
R"(v2
800 600
nochecksum


./tests/fixtures/sample0.wrm 1 2 3 4 5 6 7 8 1352304810 1352304870
./tests/fixtures/sample1.wrm 9 8 10 11 12 13 14 15 1352304870 1352304930
./tests/fixtures/sample2.wrm 16 17 18 19 20 21 22 23 1352304930 1352304990
)"_av);
    RED_TEST(t.atomic_read({buffer, 1}) == Transport::Read::Eof);
}

RED_AUTO_TEST_CASE(TestInMultiCryptoTransport2)
{
    uint8_t hmac_key[] = {
        0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
        0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
        0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
        0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };

    auto trace_fn = [](uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme) {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2 };
        static_assert(sizeof(trace_key) == MD_HASH::DIGEST_LENGTH );
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    };

    CryptoContext cctx;
    cctx.set_hmac_key(hmac_key);
    cctx.set_get_trace_key_cb(trace_fn);
    cctx.set_master_derivator(cstr_array_view(
        "toto@10.10.43.13,Administrateur@QA@cible,"
        "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm"
    ));
    InMultiCryptoTransport t(
        {
            FIXTURES_PATH "/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
            FIXTURES_PATH "/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        },
        cctx, is_encrypted
    );

    auto expected = R"(v2
800 600
checksum


/var/wab/recorded/rdp/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335-000000.wrm 163032 33056 1001 1001 65030 89 1455816632 1455816632 1455816611 1455816633 056c10b7bd80a87287336dee6e431d815606a1f9f0e637120722e30c2c8cd777 f3c5362bc347f8b44a1d9163dd68ed99c1ed58c2d328d1a94a077d7658ca667c
)"_av;

    char buffer[512];
    RED_TEST(t.atomic_read({buffer, 344}) == Transport::Read::Ok);
    RED_TEST(array_view(buffer, 344) == expected);
    buffer[0] = '_';
    RED_TEST(t.atomic_read({buffer, 344}) == Transport::Read::Ok);
    RED_TEST(array_view(buffer, 344) == expected);
    RED_TEST(t.atomic_read({buffer, 1}) == Transport::Read::Eof);
}

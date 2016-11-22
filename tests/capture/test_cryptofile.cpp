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
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
// #define LOGPRINT

#include "capture/cryptofile.hpp"


BOOST_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey)
{
    unsigned char expected_hmac_key[]
             = { 0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
                 0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
                 0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
                 0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c
               };

    Inifile ini;
    ini.set<cfg::crypto::key0>(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    );
    ini.set<cfg::crypto::key1>(
         "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
         "\x49\x21\x57\x87\x47\x74\x08\x8a"
         "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
         "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
    );
    CryptoContext cctx(ini);
    cctx.get_master_key();
//    hexdump_c(cctx.get_hmac_key(), HMAC_KEY_LENGTH);
    BOOST_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), 32));
}


BOOST_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey2)
{
    unsigned char expected_hmac_key[] = {
         0x86U, 0x41U, 0x05U, 0x58U, 0xc4U, 0x95U, 0xccU, 0x4eU,
         0x49U, 0x21U, 0x57U, 0x87U, 0x47U, 0x74U, 0x08U, 0x8aU,
         0x33U, 0xb0U, 0x2aU, 0xb8U, 0x65U, 0xccU, 0x38U, 0x41U,
         0x20U, 0xfeU, 0xc2U, 0xc9U, 0xb8U, 0x72U, 0xc8U, 0x2cU
         };

    Inifile ini;
    ini.set<cfg::crypto::key0>(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8");
    ini.set<cfg::crypto::key1>(
         "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
         "\x49\x21\x57\x87\x47\x74\x08\x8a"
         "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
         "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
    );

    CryptoContext cctx(ini);
    cctx.get_master_key();
    BOOST_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), HMAC_KEY_LENGTH));
}

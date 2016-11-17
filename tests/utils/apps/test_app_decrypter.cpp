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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAppDecrypter
#include "system/redemption_unit_tests.hpp"


// #define LOGPRINT
#define LOGNULL

#include <fcntl.h>

#include <iostream>

#include "main/version.hpp"
#include "system/ssl_calls.hpp"
#include "utils/apps/app_decrypter.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include "utils/genrandom.hpp"

#include <new>


#include "utils/fdbuf.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "utils/apps/cryptofile.hpp"

#ifdef IN_IDE_PARSER
#define FIXTURES_PATH ""
#endif

extern "C" {
    int hmac_fn(char * buffer);

    int hmac_fn(char * buffer)
    {
        uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        memcpy(buffer, hmac_key, SslSha256::DIGEST_LENGTH);
        return 0;
    }

    int trace_fn(char * base, int len, char * buffer);

    int trace_fn(char * base, int len, char * buffer)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        uint8_t trace_key[SslSha256::DIGEST_LENGTH] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2
         };
        memset(buffer, 0, len);
        memcpy(buffer, trace_key, std::min(
                static_cast<size_t>(len),
                static_cast<size_t>(SslSha256::DIGEST_LENGTH)));
        return 0;
    }
}

// tests/fixtures/verifier/recorded/toto@10.10.43.13\,Administrateur@QA@cible\,20160218-181658\,wab-5-0-0.yourdomain\,7681.mwrm

// python tools/decrypter.py -i tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -o decrypted.out

BOOST_AUTO_TEST_CASE(TestDecrypterEncryptedData)
{
    Inifile ini;
    ini.set<cfg::debug::config>(false);
    CryptoContext cctx(ini);
    cctx.set_get_hmac_key_cb(hmac_fn);
    cctx.set_get_trace_key_cb(trace_fn);

    char const * argv[] {
        "decrypter.py",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
            "toto@10.10.43.13,Administrateur@QA@cible,"
            "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
        "-o",
            "decrypted.out",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    try {
        res = app_decrypter(argc, argv
            , "ReDemPtion VERifier " VERSION ".\n"
            "Copyright (C) Wallix 2010-2016.\n"
            "Christophe Grosjean, Raphael Zhou."
            , cctx);
    } catch (const Error & e) {
        printf("verify failed: with id=%d\n", e.id);
    }
    BOOST_CHECK_EQUAL(0, unlink("decrypted.out"));
    BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestDecrypterClearData)
{
    Inifile ini;
    ini.set<cfg::debug::config>(false);
    CryptoContext cctx(ini);
    cctx.set_get_hmac_key_cb(hmac_fn);
    cctx.set_get_trace_key_cb(trace_fn);

    char const * argv[] {
        "decrypter.py",
        "-i",
            FIXTURES_PATH "/verifier/recorded/"
                "toto@10.10.43.13,Administrateur@QA@cible"
            ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
        "-o",
            "decrypted.2.out",
        "--verbose",
            "10",
    };
    int argc = sizeof(argv)/sizeof(char*);

    int res = -1;
    try {
        res = app_decrypter(argc, argv
            , "ReDemPtion VERifier " VERSION ".\n"
            "Copyright (C) Wallix 2010-2016.\n"
            "Christophe Grosjean, Raphael Zhou."
            , cctx);
    } catch (const Error & e) {
        printf("verify failed: with id=%d\n", e.id);
    }
    BOOST_CHECK_EQUAL(0, res);
}

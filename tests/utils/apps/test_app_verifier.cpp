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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAppVerifier
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGPRINT
//#define LOGNULL

#include <fcntl.h>

#include <iostream>

#include "system/ssl_calls.hpp"
#include "utils/apps/app_verifier.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include "utils/genrandom.hpp"

#include <new>

#include "utils/fdbuf.hpp"
#include "utils/sugar/local_fd.hpp"
#include "transport/out_meta_sequence_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "transport/cryptofile.hpp"

#ifdef IN_IDE_PARSER
#define FIXTURES_PATH ""
#endif

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

BOOST_AUTO_TEST_CASE(TestReverseIterators)
{

    // Show how to extract filename even if it cvontains spaces
    // the idea is that the final fields are fixed, henceforth
    // we can skip these fields by looking for spaces from end of line
    // once filename is found we can manage other fields as usual.
    // no need to search backward.

    char line[256] = {};
    const char * str = "ff fff sssss eeeee hhhhh HHHHH\n";
    int len = strlen(str);
    memcpy(line, str, len+1);
    typedef std::reverse_iterator<char*> reverse_iterator;

    reverse_iterator first(line);
    reverse_iterator last(line + len);

    reverse_iterator space4 = std::find(last, first, ' ');
    printf("space1=%s\n", &space4[0]);
    space4++;
    reverse_iterator space3 = std::find(space4, first, ' ');
    printf("space3=%s\n", &space3[0]);
    space3++;
    reverse_iterator space2 = std::find(space3, first, ' ');
    printf("space2=%s\n", &space2[0]);
    space2++;
    reverse_iterator space1 = std::find(space2, first, ' ');
    printf("space1=%s\n", &space1[0]);
    space1++;
    int filename_len = first-space1;
    printf("filename_len = %d\n", filename_len);

    char filename[128];
    memcpy(filename, line, filename_len);
    
    BOOST_CHECK(0 == memcmp("ff fff", filename, filename_len));
    printf("filename=%s\n", filename);

}

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
    const std::string test_mwrm_path = "./";
    const std::string test_file_name = "TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/
    Inifile ini;
    ini.set<cfg::crypto::key0>(
        "\x00\x01\x02\x03\x04\x05\x06\x07"
        "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
        "\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
    );
    ini.set<cfg::crypto::key1>("12345678901234567890123456789012");
    LCGRandom rnd(0);

    CryptoContext cctx(rnd, ini);

    uint8_t hmac_key[32] = {};

    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    unsigned char tmp_derivation1[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated2[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation1, HASH_DERIVATOR, DERIVATOR_LENGTH);
    memcpy(tmp_derivation1 + DERIVATOR_LENGTH, cctx.get_master_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation1, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated2);
    memcpy(hmac_key, derivated2, HMAC_KEY_LENGTH);

    OpenSSL_add_all_digests();

    // Any iv key would do, we are checking round trip
    unsigned char iv[32] = {
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
    };

    unsigned char derivator[DERIVATOR_LENGTH];

    size_t len = 0;
    const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(test_file_name.c_str(), len));
    SslSha256 sha256;
    sha256.update(base, len);
    uint8_t tmp[SHA256_DIGEST_LENGTH];
    sha256.final(tmp, SHA256_DIGEST_LENGTH);
    memcpy(derivator, tmp, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    unsigned char tmp_derivation2[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
    unsigned char derivated1[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
    memcpy(tmp_derivation2, derivator, DERIVATOR_LENGTH);
    memcpy(tmp_derivation2 + DERIVATOR_LENGTH, cctx.get_master_key(), CRYPTO_KEY_LENGTH);
    SHA256(tmp_derivation2, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated1);
    memcpy(trace_key, derivated1, HMAC_KEY_LENGTH);

    std::string full_test_file_name = test_mwrm_path + test_file_name;
    int system_fd = open(full_test_file_name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", full_test_file_name.c_str());
        BOOST_CHECK(false);
    }

    struct crypto_file
    {
      transfil::encrypt_filter encrypt;
      io::posix::fdbuf file;

      crypto_file(int fd)
      : file(fd)
      {}
    } * cf_struct = new (std::nothrow) crypto_file(system_fd);

    if (cf_struct) {
        if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, &cctx, iv)) {
            delete cf_struct;
            cf_struct = nullptr;
            close(system_fd);
        }
    }

    BOOST_CHECK(cf_struct);

    const char    * data     = "Indentation Settings determine the size of the tab stops, "
                               "and control whether the tab key should insert tabs or spaces.";
    const size_t    data_len = strlen(data);
    int             res;
    unsigned char   hash[HASH_LEN];


    for (int i = 0; i < 256; i ++) {
        res = cf_struct->encrypt.write(cf_struct->file, const_cast<char *>(data), data_len);
        BOOST_CHECK_EQUAL(data_len, res);
    }

    res = cf_struct->encrypt.close(cf_struct->file, hash, cctx.get_hmac_key());
    delete cf_struct;

    BOOST_CHECK_EQUAL(0, res);

    std::string const test_full_mwrm_filename = test_mwrm_path + test_file_name;
    {
        FileChecker check(test_full_mwrm_filename);
        check.check_hash_sha256(cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()), hash, HASH_LEN / 2, true);
        BOOST_CHECK_EQUAL(false, check.failed);
    }

    {
        FileChecker check(test_full_mwrm_filename);
        check.check_hash_sha256(cctx.get_hmac_key(), sizeof(cctx.get_hmac_key()),
                                hash + (HASH_LEN / 2), HASH_LEN / 2, false);
        BOOST_CHECK_EQUAL(false, check.failed);
    }

    unlink(full_test_file_name.c_str());
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */

// python tools/verifier.py -i toto@10.10.43.13\,Administrateur@QA@cible\,20160218-183009\,wab-5-0-0.yourdomain\,7335.mwrm --hash-path tests/fixtures/verifier/hash/ --mwrm-path tests/fixtures/verifier/recorded/ --verbose 10

extern "C" {
    int hmac_fn(char * buffer)
    {
        uint8_t hmac_key[32] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        memcpy(buffer, hmac_key, 32);
        return 0;
    }

    int trace_fn(char * base, int len, char * buffer)
    {
        // in real uses actual trace_key is derived from base and some master key
        uint8_t trace_key[32] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2
         };
        memcpy(buffer, trace_key, 32);
        return 0;
    }
}

BOOST_AUTO_TEST_CASE(TestVerifierEncryptedData)
{
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        char const * argv[] = {
            "verifier.py",
            "-i",
                "toto@10.10.43.13,Administrateur@QA@cible,"
                "20160218-183009,wab-5-0-0.yourdomain,7335.mwrm",
            "--hash-path",
                FIXTURES_PATH "/verifier/hash",
            "--mwrm-path",
                FIXTURES_PATH "/verifier/recorded",
            "--verbose",
                "10",
        };
        int argc = sizeof(argv)/sizeof(char*);

        int res = -1;
        try {
            res = app_verifier(ini,
                argc, argv
              , "ReDemPtion VERifier " VERSION ".\n"
                "Copyright (C) Wallix 2010-2016.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("verify ok\n");
            }
            else {
                printf("verify failed\n");
            }
        } catch (const Error & e) {
            printf("verify failed: with id=%d\n", e.id);
        }
        BOOST_CHECK_EQUAL(0, res);
}

BOOST_AUTO_TEST_CASE(TestVerifierClearData)
{
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        char const * argv[] {
            "verifier.py",
            "-i",
                "toto@10.10.43.13,Administrateur@QA@cible"
                ",20160218-181658,wab-5-0-0.yourdomain,7681.mwrm",
            "--hash-path",
                FIXTURES_PATH "/verifier/hash/",
            "--mwrm-path",
                FIXTURES_PATH "/verifier/recorded/",
            "--verbose",
                "10",
        };
        int argc = sizeof(argv)/sizeof(char*);


        int res = -1;
        try {
            res = app_verifier(ini,
                argc, argv
              , "ReDemPtion VERifier " VERSION ".\n"
                "Copyright (C) Wallix 2010-2016.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("verify ok\n");
            }
            else {
                printf("verify failed\n");
            }
        } catch (const Error & e) {
            printf("verify failed: with id=%d\n", e.id);
        }
        BOOST_CHECK_EQUAL(0, res);
}


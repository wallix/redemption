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
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT

#include <fcntl.h>

#include <iostream>

#include "ssl_calls.hpp"
#include "apps/app_verifier.hpp"

#include "ccryptofile.h"

#ifdef HASH_LEN
#undef HASH_LEN
#endif  // #ifdef HASH_LEN
#define HASH_LEN 64

int libc_read(int fd, char *buf, unsigned int count)
{
   return read(fd, buf, count);
}   /* int libc_read(int fd, char *buf, unsigned int count) */

BOOST_AUTO_TEST_CASE(TestVerifierReadLine)
{
   BStream opaque_stream(32)    ;
   int     opaque_data       = 1;

   int fd = open("./tests/fixtures/sample.txt", O_RDONLY);
   BOOST_CHECK(fd != -1);

   if (fd != -1) {
      char line[256];

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("S_IRWXO\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("    00007 others have read, write and execute permission\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("S_IROTH\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("    00004 others have read permission\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("S_IWOTH\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("    00002 others have write permission\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("S_IXOTH\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) > 0);
      BOOST_CHECK_EQUAL(std::string(line), std::string("\n"));

      BOOST_CHECK(read_line<int>(fd, libc_read, opaque_stream, opaque_data, line, sizeof(line)) == 0);

      close(fd);
   }
}   /* BOOST_AUTO_TEST_CASE(TestVerifierReadLine) */

BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash)
{
    const char * test_file_name = "./TestCheckFileHash";

    /************************
    * Manage encryption key *
    ************************/
    CryptoContext cctx = {
        // hmac_key
        {
        },
        // Crypto key
        {
         0,  1,  2,  3,  4,  5,  6,  7,
         8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31
        },
    };

    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };
    BOOST_CHECK(0 == compute_hmac(cctx.hmac_key, cctx.crypto_key, HASH_DERIVATOR));
    OpenSSL_add_all_digests();

    // Any iv key would do, we are checking round trip
    unsigned char iv[32] = {
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
    };

    unsigned char derivator[DERIVATOR_LENGTH];
    get_derivator(test_file_name, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    if (compute_hmac(trace_key, cctx.crypto_key, derivator) == -1){
        BOOST_CHECK(false);
    }

    int system_fd = open(test_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", test_file_name);
        BOOST_CHECK(false);
    }

    crypto_file * cf_struct = crypto_open_write(system_fd, trace_key, &cctx, iv);
    if (!cf_struct){
        close(system_fd);
    }

    BOOST_CHECK(cf_struct);

    const char    * data     = "Indentation Settings determine the size of the tab stops, "
                               "and control whether the tab key should insert tabs or spaces.";
    const size_t    data_len = strlen(data);
    int             res;
    unsigned char   hash[HASH_LEN];


    for (int i = 0; i < 256; i ++) {
        res = crypto_write(cf_struct, const_cast<char *>(data), data_len);

        BOOST_CHECK_EQUAL(data_len, res);
    }

    res = crypto_close(cf_struct, hash, cctx.hmac_key);

    BOOST_CHECK_EQUAL(0, res);

    StaticStream _4kb_hash(hash, HASH_LEN / 2);
    StaticStream full_hash(hash + (HASH_LEN / 2), HASH_LEN / 2);

    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.hmac_key, sizeof(cctx.hmac_key),
                                                   _4kb_hash.get_data(), _4kb_hash.size(), 4096));
    BOOST_CHECK_EQUAL(true, check_file_hash_sha256(test_file_name, cctx.hmac_key, sizeof(cctx.hmac_key),
                                                   full_hash.get_data(), full_hash.size(), 0));

    unlink(test_file_name);
}   /* BOOST_AUTO_TEST_CASE(TestVerifierCheckFileHash) */

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6\n";

    int result;

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(1, result);

    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp/replay-008795-000000.wrm"),
        std::string(reinterpret_cast<const char *>(file_name.get_data())));
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo1)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6";

    int result;

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(1, result);

    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp/replay-008795-000000.wrm"),
        std::string(reinterpret_cast<const char *>(file_name.get_data())));
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo2)
{
    const char * line = "";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(0, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo3)
{
    const char * line = "\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(0, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo4)
{
    const char * line = " \n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo5)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "  "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);

    BOOST_CHECK_EQUAL(1, result);

    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp/replay-008795-000000.wrm"),
        std::string(reinterpret_cast<const char *>(file_name.get_data())));
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo6)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c777291 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo7)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo8)
{
    const char * line = "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo9)
{
    const char * line = " "
                        "1367316138 1367316189 "
                        "8da78365b5421e706a987af4cc3e18dbca1911a7de8341d825570c68c7772913 "
                        "d21918de7764c62cfef5f32326d76afc14558eedd21f4926484ad2dcfa804ba6";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo10)
{
    const char * line = "/var/wab/recorded/rdp/replay-008795-000000.wrm "
                        "1367316138 1367316189\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

BOOST_AUTO_TEST_CASE(TestVerifierExtractFileInfo11)
{
    const char * line = "TestVerifierExtractFileInfo11"
                        "TestVerifierExtractFileInfo11"
                        "TestVerifierExtractFileInfo11\n";

    BStream file_name(1024);
    BStream _4kb_hash(HASH_LEN);
    BStream full_hash(HASH_LEN);

    int result = extract_file_info(line, file_name, _4kb_hash, full_hash);
    BOOST_CHECK_EQUAL(-1, result);
}

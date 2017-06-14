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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE Testscytale
#include "system/redemption_unit_tests.hpp"

// #define LOGPRINT
#define LOGNULL
#include "utils/log.hpp"

#include "main/scytale.hpp"

extern "C" {
    inline
    int hmac_fn(uint8_t * buffer)
    {
        // 86410558C495CC4E492157874774088A33B02AB865CC384120FEC2C9B872C82C
        uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
            0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
            0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
            0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
            0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c,
        };
        memcpy(buffer, hmac_key, sizeof(hmac_key));
        return 0;
    }

    inline
    int trace_fn(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 611FD4CDE595B7FDA65038FCD886514F597E8E9081F6F4489C7741510F530EE8
        uint8_t trace_key[SslSha256::DIGEST_LENGTH] = {
            0x61, 0x1f, 0xd4, 0xcd, 0xe5, 0x95, 0xb7, 0xfd,
            0xa6, 0x50, 0x38, 0xfc, 0xd8, 0x86, 0x51, 0x4f,
            0x59, 0x7e, 0x8e, 0x90, 0x81, 0xf6, 0xf4, 0x48,
            0x9c, 0x77, 0x41, 0x51, 0x0f, 0x53, 0x0e, 0xe8,
        };
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

inline uint8_t const * bytes(char const * p)
{
    return reinterpret_cast<uint8_t const *>(p);
}

RED_AUTO_TEST_CASE(Testscytale)
{
    const char * finalname = "encrypted.txt";
    const char * hash_finalname = "hash_encrypted.txt";
    ::unlink(finalname);
    ::unlink(hash_finalname);

    // Writer
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new_with_test_random(with_encryption, with_checksum, finalname, &hmac_fn, &trace_fn);
        RED_CHECK_NE(handle, nullptr);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        RED_CHECK_EQ(scytale_writer_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_EQ(scytale_writer_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

        scytale_writer_delete(handle);
    }

    // Reader
    {
        auto handle = scytale_reader_new(finalname, &hmac_fn, &trace_fn, 0, 0);
        RED_CHECK_NE(scytale_reader_open(handle, finalname, finalname), -1);
        RED_CHECK_NE(handle, nullptr);

        uint8_t buf[31];

        size_t total = 0;
        while (total < sizeof(buf)) {
            int res = scytale_reader_read(handle, &buf[total], 10);
            RED_REQUIRE_GT(res, 0);
            total += size_t(res);
        }
        RED_CHECK_MEM_C(byte_array(buf, 31), "We write, and again, and so on.");
        RED_CHECK_EQ(scytale_reader_close(handle), 0);

        RED_CHECK_EQ(scytale_reader_error_message(handle), "No error");

        RED_CHECK_EQ(scytale_reader_hash(handle, finalname), 0);

        RED_CHECK_EQ(scytale_reader_qhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_EQ(scytale_reader_fhashhex(handle), "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        scytale_reader_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);
}

RED_AUTO_TEST_CASE(TestscytaleWriteUseRandom)
{
    const char * finalname = "encrypted.txt";
    const char * hash_finalname = "hash_encrypted.txt";

    HashHexArray qhash;
    HashHexArray fhash;

    // Writer with udev random
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname, &hmac_fn, &trace_fn);
        RED_CHECK_NE(handle, nullptr);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        memcpy(qhash, scytale_writer_qhashhex(handle), sizeof(qhash));
        memcpy(fhash, scytale_writer_fhashhex(handle), sizeof(fhash));
        RED_CHECK_NE(qhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
        RED_CHECK_NE(fhash, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

        scytale_writer_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);

    // Writer with udev random
    {
        int with_encryption = 1; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        auto * handle = scytale_writer_new(with_encryption, with_checksum, finalname,  &hmac_fn, &trace_fn);
        RED_CHECK_NE(handle, nullptr);
        RED_CHECK_EQ(scytale_writer_open(handle, finalname, hash_finalname, 0), 0);

        RED_CHECK_EQ(scytale_writer_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(scytale_writer_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(scytale_writer_close(handle), 0);

        auto qhash2 = scytale_writer_qhashhex(handle);
        auto fhash2 = scytale_writer_fhashhex(handle);
        RED_CHECK_NE(qhash2, qhash);
        RED_CHECK_NE(fhash2, fhash);

        RED_CHECK_EQ(scytale_writer_error_message(handle), "No error");

        scytale_writer_delete(handle);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
    RED_CHECK_EQ(::unlink(hash_finalname), 0);
}

RED_AUTO_TEST_CASE(TestscytaleError)
{
    auto handle_w = scytale_writer_new(1, 1, "/", &hmac_fn, &trace_fn);
    RED_CHECK_EQ(scytale_writer_open(handle_w, "/", "/", 0), -1);
    RED_CHECK_NE(scytale_writer_error_message(handle_w), "No error");

    auto handle_r = scytale_reader_new("/", &hmac_fn, &trace_fn, 0, 0);
    RED_CHECK_EQ(scytale_reader_open(handle_r, "/", "/"), -1);
    RED_CHECK_NE(scytale_reader_error_message(handle_r), "No error");

    RED_CHECK_EQ(scytale_reader_hash(handle_r, "/"), -1);
    RED_CHECK_NE(scytale_reader_error_message(handle_r), "No error");

    scytale_writer_delete(handle_w);
    scytale_reader_delete(handle_r);

    RED_CHECK_EQ(scytale_writer_write(nullptr, bytes("We write, "), 10), -1);
    RED_CHECK_EQ(scytale_writer_close(nullptr), -1);
    RED_CHECK_NE(scytale_writer_error_message(nullptr), "No error");

    uint8_t buf[12];
    RED_CHECK_EQ(scytale_reader_read(nullptr, buf, 10), -1);
    RED_CHECK_EQ(scytale_reader_close(nullptr), -1);
    RED_CHECK_NE(scytale_reader_error_message(nullptr), "No error");


}

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

#define RED_TEST_MODULE TestRedcryptofile
#include "system/redemption_unit_tests.hpp"

//#define LOGPRINT
#define LOGNULL
#include "utils/log.hpp"

#include "main/redcryptofile.hpp"

extern "C" {
    inline
    int hmac_fn(char * buffer)
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
    int trace_fn(char * base, int len, char * buffer, unsigned oldscheme)
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

RED_AUTO_TEST_CASE(TestRedCryptofile)
{
    OpenSSL_add_all_digests();

    const char * finalname = "./encrypted.txt";

    // Writer
    {
        int with_encryption = 0; // int used as boolean 0 false, true otherwise
        int with_checksum = 1;   // int used as boolean 0 false, true otherwise

        HashHexArray qhashhex {};
        HashHexArray fhashhex {};

        auto * handle = redcryptofile_new_writer(with_encryption, with_checksum, &hmac_fn, &trace_fn);
        RED_CHECK_NE(handle, nullptr);
        RED_CHECK_EQ(redcryptofile_open_writer(handle, finalname), 0);

        RED_CHECK_EQ(redcryptofile_write(handle, bytes("We write, "), 10), 10);
        RED_CHECK_EQ(redcryptofile_write(handle, bytes("and again, "), 11), 11);
        RED_CHECK_EQ(redcryptofile_write(handle, bytes("and so on."), 10), 10);

        RED_CHECK_EQ(redcryptofile_close_writer(handle, qhashhex, fhashhex), 0);

//        RED_CHECK_EQ(qhashhex, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
//        RED_CHECK_EQ(fhashhex, "2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5");
    }

    // Reader
    {
        auto * handle = redcryptofile_open_reader(finalname, &hmac_fn, &trace_fn);
        RED_CHECK_NE(handle, nullptr);

        uint8_t buf[31];

        size_t total = 0; 
        while (total < sizeof(buf)) {
            int res = redcryptofile_read(handle, &buf[total], 10);
            LOG(LOG_INFO, "%d", res);
            BOOST_CHECK(res > 0);
            total += size_t(res);
        }
        RED_CHECK_MEM_C(bytes_array(buf, 31), "We write, and again, and so on.");
        RED_CHECK_EQ(redcryptofile_close_reader(handle), 0);
    }

    RED_CHECK_EQ(::unlink(finalname), 0);
}

RED_AUTO_TEST_CASE(TestRedCryptofileError)
{
    LOG(LOG_INFO, "TestRedCryptofileError");
    LOG(LOG_INFO, "Errors below are expected this is the purpose of the test");
    auto handle = redcryptofile_new_writer(1, 1, &hmac_fn, &trace_fn);
    RED_CHECK_EQ(redcryptofile_open_writer(handle, "/"), -1);
    RED_CHECK_EQ(redcryptofile_open_reader("/", &hmac_fn, &trace_fn), nullptr);

    HashHexArray qhashhex {};
    HashHexArray fhashhex {};
    RED_CHECK_EQ(redcryptofile_write(nullptr, bytes("We write, "), 10), -1);
    RED_CHECK_EQ(redcryptofile_close_writer(nullptr, qhashhex, fhashhex), -1);
    uint8_t buf[12];
    RED_CHECK_EQ(redcryptofile_read(nullptr, buf, 10), -1);
    RED_CHECK_EQ(redcryptofile_close_reader(nullptr), -1);
    LOG(LOG_INFO, "TestRedCryptofileError done");
}

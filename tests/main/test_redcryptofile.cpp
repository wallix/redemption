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

// #define LOGPRINT
#define LOGNULL
#include "utils/log.hpp"

#include "main/redcryptofile.hpp"

extern "C" {
    int hmac_fn(char * buffer)
    {
        // E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE
        uint8_t hmac_key[SslSha256::DIGEST_LENGTH] = {
            0xe3, 0x8d, 0xa1, 0x5e, 0x50, 0x1e, 0x4f, 0x6a,
            0x01, 0xef, 0xde, 0x6c, 0xd9, 0xb3, 0x3a, 0x3f,
            0x2b, 0x41, 0x72, 0x13, 0x1e, 0x97, 0x5b, 0x4c,
            0x39, 0x54, 0x23, 0x14, 0x43, 0xae, 0x22, 0xae };
        memcpy(buffer, hmac_key, SslSha256::DIGEST_LENGTH);
        return 0;
    }

    int trace_fn(char * base, int len, char * buffer, unsigned oldscheme)
    {
        // in real uses actual trace_key is derived from base and some master key
        (void)base;
        (void)len;
        (void)oldscheme;
        // 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2
        uint8_t trace_key[SslSha256::DIGEST_LENGTH] = {
            0x56, 0x3e, 0xb6, 0xe8, 0x15, 0x8f, 0x0e, 0xed,
            0x2e, 0x5f, 0xb6, 0xbc, 0x28, 0x93, 0xbc, 0x15,
            0x27, 0x0d, 0x7e, 0x78, 0x15, 0xfa, 0x80, 0x4a,
            0x72, 0x3e, 0xf4, 0xfb, 0x31, 0x5f, 0xf4, 0xb2
         };
        memcpy(buffer, trace_key, sizeof(trace_key));
        return 0;
    }
}

RED_AUTO_TEST_CASE(TestRedCryptofileWriter)
{
    OpenSSL_add_all_digests();
    int with_encryption = 0; // int used as boolean 0 false, true otherwise
    int with_checksum = 0;   // int used as boolean 0 false, true otherwise

    uint8_t qhashhex[MD_HASH::DIGEST_LENGTH * 2]{};
    uint8_t fhashhex[MD_HASH::DIGEST_LENGTH * 2]{};

    const char * finalname = "./encrypted.txt";

    RedCryptoHandle * handle = redcryptofile_open_writer(with_encryption, with_checksum, finalname, trace_fn, hmac_fn);
    RED_CHECK_NE(handle, nullptr);
    RED_CHECK_EQ(redcryptofile_write(handle, "We write, ", 10), 10);
    RED_CHECK_EQ(redcryptofile_write(handle, "and again, ", 11), 11);
    RED_CHECK_EQ(redcryptofile_write(handle, "and so on.", 10), 10);
    RED_CHECK_EQ(redcryptofile_close_writer(handle, qhashhex, fhashhex), 0);
}

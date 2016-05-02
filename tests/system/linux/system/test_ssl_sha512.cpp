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
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestXXX
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
// #define LOGNULL

#include <stdio.h>
#include "system/ssl_calls.hpp"

BOOST_AUTO_TEST_CASE(TestSslHmacSHA512)
{
    const uint8_t key[] = "key";
    // const uint8_t key[] = "";
    SslHMAC_Sha256 hmac(key, sizeof(key)-1);

    const uint8_t msg[] = "The quick brown fox jumps over the lazy dog";
    // const uint8_t msg[] = "";
    hmac.update(msg, sizeof(msg)-1);

    uint8_t sig[SHA512_DIGEST_LENGTH];
    hmac.final(sig, SHA512_DIGEST_LENGTH);

    BOOST_CHECK_EQUAL(SHA512_DIGEST_LENGTH, 32);

    BOOST_CHECK_EQUAL(memcmp(sig,
                             "\xf7\xbc\x83\xf4\x30\x53\x84\x24\xb1\x32\x98\xe6\xaa\x6f\xb1\x43"
                             "\xef\x4d\x59\xa1\x49\x46\x17\x59\x97\x47\x9d\xbc\x2d\x1a\x3c\xd8",
                             SHA512_DIGEST_LENGTH),
                      0);
    // hexdump96_c(sigstream.get_data(), sigstream.size());

}


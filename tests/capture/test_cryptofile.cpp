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

#define RED_TEST_MODULE TestVerifier
#include "system/redemption_unit_tests.hpp"

//#define LOGNULL
#define LOGPRINT

#include "capture/cryptofile.hpp"

RED_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey)
{
    unsigned char expected_master_key[] {
        0x61, 0x1f, 0xd4, 0xcd, 0xe5, 0x95, 0xb7, 0xfd,
        0xa6, 0x50, 0x38, 0xfc, 0xd8, 0x86, 0x51, 0x4f,
        0x59, 0x7e, 0x8e, 0x90, 0x81, 0xf6, 0xf4, 0x48,
        0x9c, 0x77, 0x41, 0x51, 0x0f, 0x53, 0x0e, 0xe8,
    };

    unsigned char expected_hmac_key[] {
        0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
        0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
        0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
        0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c,
    };

    CryptoContext cctx;
    cctx.set_master_key(expected_master_key);
    cctx.set_hmac_key(expected_hmac_key);

    RED_CHECK(0 == memcmp(expected_master_key, cctx.get_master_key(), 32));
    RED_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), 32));
}

namespace {
    bool visited_trace_key = false;
}
inline int trace_key_cb(uint8_t const * base, int len, uint8_t * /*buffer*/, unsigned /*oldscheme*/)
{
    RED_CHECK_EQ(reinterpret_cast<char const *>(base), "abcd.mwrm");
    RED_CHECK_EQ(len, 9);
    visited_trace_key = true;
    return 0;
}

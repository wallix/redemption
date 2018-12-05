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
#include "test_only/test_framework/redemption_unit_tests.hpp"


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
    struct Observer
    {
        bool visited = false;
        std::string key;
        void reset()
        {
            this->visited = false;
            this->key.clear();
        }
    };
    Observer g_trace_key_ob;

    int trace_key_cb(uint8_t const * base, int len, uint8_t * /*buffer*/, unsigned /*oldscheme*/)
    {
        g_trace_key_ob.key.assign(const_byte_ptr(base), len);
        g_trace_key_ob.visited = true;
        return 0;
    }
}


RED_AUTO_TEST_CASE(TestNormalizeDerivedKey)
{
    CryptoContext cctx;
    cctx.set_get_trace_key_cb(trace_key_cb);
    cctx.set_master_derivator(cstr_array_view("abcd.mwrm"));

    uint8_t trace_key[CRYPTO_KEY_LENGTH];

    cctx.old_encryption_scheme = true;

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcde.mwrm"));
    RED_CHECK(g_trace_key_ob.visited);
    RED_CHECK_EQ(g_trace_key_ob.key, "abcde.mwrm");

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcdef.log"));
    RED_CHECK(g_trace_key_ob.visited);
    RED_CHECK_EQ(g_trace_key_ob.key, "abcdef.mwrm");

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcdefghi"));
    RED_CHECK(g_trace_key_ob.visited);
    RED_CHECK_EQ(g_trace_key_ob.key, "abcdefghi");

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcdefghi.xxx"));
    RED_CHECK(g_trace_key_ob.visited);
    RED_CHECK_EQ(g_trace_key_ob.key, "abcdefghi.xxx");

    cctx.old_encryption_scheme = false;

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcdefg.log"));
    RED_CHECK(g_trace_key_ob.visited);
    RED_CHECK_EQ(g_trace_key_ob.key, "abcd.mwrm");

    g_trace_key_ob.reset();
    cctx.get_derived_key(trace_key, cstr_array_view("abcdefgh.log"));
    RED_CHECK(!g_trace_key_ob.visited);
}

RED_AUTO_TEST_CASE(TestSetMasterDerivator)
{
    CryptoContext cctx;
    auto abc = cstr_array_view("abc");
    auto abcd = cstr_array_view("abcd");
    cctx.set_master_derivator(abc);
    RED_CHECK_NO_THROW(cctx.set_master_derivator(abc));
    RED_CHECK_EXCEPTION_ERROR_ID(cctx.set_master_derivator(abcd), ERR_WRM_INVALID_INIT_CRYPT);
    RED_CHECK_NO_THROW(cctx.set_master_derivator(abc));
}


namespace
{
    bool visited_cb = false;
}

RED_AUTO_TEST_CASE(TestErrCb)
{
    CryptoContext cctx;
    cctx.set_get_hmac_key_cb(
        [](uint8_t*){ visited_cb = true; return -1; });
    cctx.set_get_trace_key_cb(
        [](uint8_t const*, int, uint8_t*, unsigned){ visited_cb = true; return -1; });
    cctx.set_master_derivator(cstr_array_view("abc"));

    visited_cb = false;
    RED_CHECK_EXCEPTION_ERROR_ID(cctx.get_hmac_key(), ERR_WRM_INVALID_INIT_CRYPT);
    RED_CHECK(visited_cb);

    visited_cb = false;
    uint8_t trace_key[CRYPTO_KEY_LENGTH];
    RED_CHECK_EXCEPTION_ERROR_ID(cctx.get_derived_key(trace_key, {}), ERR_WRM_INVALID_INIT_CRYPT);
    RED_CHECK(visited_cb);
}

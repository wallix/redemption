/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2013
 * Author(s): Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSumBuf
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH
#define LOGPRINT
#define LOGNULL

#include "buffer/checksum_buf.hpp"
#include "buffer/null_buf.hpp"
#include "detail/meta_hash.hpp"

template<size_t N>
long write(transbuf::ochecksum_buf<transbuf::null_buf> & buf, char const (&s)[N]) {
    return buf.write(s, N-1);
}

struct checksum_buf : transbuf::ochecksum_buf<transbuf::null_buf> {
    checksum_buf(CryptoContext & cctx, char const (&crypto_key)[33], Random & rnd, Inifile & ini)
    : transbuf::ochecksum_buf<transbuf::null_buf>([&]{
            memcpy(cctx.hmac_key, crypto_key, 32);
            return std::ref(cctx.hmac_key);
        }())
    {
        this->open();
    }
};

BOOST_AUTO_TEST_CASE(TestOSumBuf)
{
    Inifile ini;
    ini.set_value("crypto", "key0", 
                  "\x00\x01\x02\x03\x04\x05\x06\x07"
                  "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
                  "\x10\x11\x12\x13\x14\x15\x16\x17"
                  "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F");
    ini.set_value("crypto", "key1", "12345678901234567890123456789012");

    LCGRandom rnd(0);
    CryptoContext cctx(rnd, ini, 1);
    checksum_buf buf(cctx, "12345678901234567890123456789012", rnd, ini);
    BOOST_CHECK_EQUAL(write(buf, "ab"), 2);
    BOOST_CHECK_EQUAL(write(buf, "cde"), 3);

    detail::hash_type hash;
    buf.close(hash);

    char hash_str[detail::hash_string_len + 1];
    *detail::swrite_hash(hash_str, hash) = 0;
    BOOST_CHECK_EQUAL(
        hash_str,
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
        " 03cb482c5a6af0d37b74d0a8b1facf6a02b619068e92495f469e0098b662fe3f"
    );
}

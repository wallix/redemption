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

#define LOGNULL

#include "buffer/checksum_buf.hpp"
#include "buffer/null_buf.hpp"
#include "detail/meta_hash.hpp"

template<size_t N>
long write(transbuf::ochecksum_buf<transbuf::null_buf> & buf, char const (&s)[N]) {
    return buf.write(s, N-1);
}

using checksum_buf_base = transbuf::ochecksum_buf<transbuf::null_buf>;

struct checksum_buf : CryptoContext, checksum_buf_base {
    template<std::size_t N>
    checksum_buf(char const (&crypto_key)[N])
    : checksum_buf_base([&]{
        auto & hmac_key = this->CryptoContext::hmac_key;
        static_assert(N-1 <= sizeof(hmac_key), "");
        memcpy(hmac_key, crypto_key, N-1);
        memset(hmac_key + N - 1, 0, sizeof(hmac_key) - (N - 1));
        return std::ref(hmac_key);
    }())
    {
        this->open();
    }
};

BOOST_AUTO_TEST_CASE(TestOSumBuf)
{
    checksum_buf buf("12345678901234567890123456789012");
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

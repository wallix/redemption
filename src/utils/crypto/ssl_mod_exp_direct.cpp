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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#include "utils/crypto/ssl_mod_exp_direct.hpp"

#include <algorithm>
#include <memory>
#include <cassert>
#include <cstddef>

#ifdef __EMSCRIPTEN__
// because _mm_getcsr() and _MM_FLUSH_ZERO_ON not implemented
# include "cxx/diagnostic.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wreserved-id-macro")
# ifdef __SSE2__
#  undef __SSE2__
# endif
REDEMPTION_DIAGNOSTIC_POP
#endif
#include <boost/multiprecision/cpp_int.hpp>

/**
 * \pre  \a out_len >= \a modulus_size
 * \return  the length of the big-endian number placed at out. ~size_t{} if error
 */
std::size_t mod_exp_direct(
    uint8_t * out, std::size_t out_len,
    const uint8_t * inr, std::size_t in_len,
    const uint8_t * modulus, std::size_t modulus_size,
    const uint8_t * exponent, std::size_t exponent_size
) {
    assert(out_len >= modulus_size);
    (void)out_len;

    using int_type = boost::multiprecision::cpp_int;

    auto b256_to_bigint = [](uint8_t const * s, size_t n) {
        int_type i;
        boost::multiprecision::import_bits(i, s, s+n);
        return i;
    };

    int_type base = b256_to_bigint(inr, in_len);
    int_type exp = b256_to_bigint(exponent, exponent_size);
    int_type m = b256_to_bigint(modulus, modulus_size);

    int_type r = boost::multiprecision::powm(base, exp, m);

    auto it = boost::multiprecision::export_bits(r, out, 8);
    auto r_len = static_cast<size_t>(it - out);
    *it = 0;
    if (r_len == 1 && *out == 0) {
        r_len = 0;
    }
    return r_len;
}

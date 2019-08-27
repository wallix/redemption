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

# include "cxx/diagnostic.hpp"

#ifdef __EMSCRIPTEN__
// because _mm_getcsr() and _MM_FLUSH_ZERO_ON not implemented
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
writable_bytes_view mod_exp_direct(
    writable_bytes_view out,
    bytes_view inr,
    bytes_view modulus,
    bytes_view exponent
) {
    assert(out.size() >= modulus.size());

    using int_type = boost::multiprecision::cpp_int;

    auto b256_to_bigint = [](bytes_view s) {
        int_type i;
        boost::multiprecision::import_bits(i, s.begin(), s.end());
        return i;
    };

    int_type base = b256_to_bigint(inr);
    int_type exp = b256_to_bigint(exponent);
    int_type m = b256_to_bigint(modulus);

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wzero-as-null-pointer-constant")
    int_type r = boost::multiprecision::powm(base, exp, m);
    REDEMPTION_DIAGNOSTIC_POP

    auto it = boost::multiprecision::export_bits(r, out.data(), 8);
    auto r_len = static_cast<size_t>(it - out.data());
    *it = 0;
    if (r_len == 1 && out[0] == 0) {
        r_len = 0;
    }
    return out.first(r_len);
}

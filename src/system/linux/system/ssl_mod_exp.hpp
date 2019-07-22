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

#pragma once

#include <cstdint>
#include <cassert>

#include <openssl/bn.h>

#include "utils/sugar/bytes_view.hpp"


/**
 * \pre  \a out.size() >= \a modulus.size()
 * \return  the length of the big-endian number placed at out. ~size_t{} if error
 */
static inline bytes_view mod_exp(
    bytes_view out,
    const_bytes_view inr,
    const_bytes_view modulus,
    const_bytes_view exponent
) {
    assert(out.size() >= modulus.size());

    struct Bignum_base
    {
        Bignum_base()
          : bn(BN_new())
        {}

        Bignum_base(const_bytes_view data)
          : bn(BN_new())
        {
            BN_bin2bn(data.data(), data.size(), this->bn);
        }

        operator BIGNUM * ()
        {
            return this->bn;
        }

        ~Bignum_base()
        {
            BN_free(this->bn);
        }

    private:
        BIGNUM * bn;
    };

    Bignum_base mod(modulus);
    Bignum_base exp(exponent);
    Bignum_base x(inr);

    Bignum_base result;

    BN_CTX *ctx = BN_CTX_new();
    BN_mod_exp(result, x, exp, mod, ctx);
    BN_CTX_free(ctx);

    auto const outlen = BN_bn2bin(result, out.data());
    // BN_clear is used to destroy sensitive data
    BN_clear(x);

    return out.first(outlen);
}

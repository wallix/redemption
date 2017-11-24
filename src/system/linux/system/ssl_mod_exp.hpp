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

#include <openssl/bn.h>
#include <cstdint>
#include <cassert>

/**
 * \pre  \a out_len >= \a modulus_size
 * \return  the length of the big-endian number placed at out. ~size_t{} if error
 */
static inline size_t mod_exp(
    uint8_t * out, size_t out_len,
    const uint8_t * inr, size_t in_len,
    const uint8_t * modulus, size_t modulus_size,
    const uint8_t * exponent, size_t exponent_size
) {
    (void)out_len;
    assert(out_len >= modulus_size);

    struct Bignum_base
    {
        Bignum_base()
          : bn(BN_new())
        {}

        Bignum_base(const uint8_t * data, size_t len)
          : bn(BN_new())
        {
            BN_bin2bn(data, len, this->bn);
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

    Bignum_base mod(modulus, modulus_size);
    Bignum_base exp(exponent, exponent_size);
    Bignum_base x(inr, in_len);

    Bignum_base result;

    BN_CTX *ctx = BN_CTX_new();
    BN_mod_exp(result, x, exp, mod, ctx);
    BN_CTX_free(ctx);

    auto const outlen = BN_bn2bin(result, out);
    // BN_clear is used to destroy sensitive data
    BN_clear(x);
    return outlen;
}

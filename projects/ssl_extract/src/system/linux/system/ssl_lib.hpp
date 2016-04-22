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
   Copyright (C) Wallix 2010-2016
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan

*/

#pragma once

#include "system/ssl_calls.hpp"
#include "system/ssl_bignum.hpp"

class ssllib
{
    public:
    static void rsa_encrypt(uint8_t * out, uint32_t in_len, uint8_t * in, uint32_t modulus_size, uint8_t * modulus, uint32_t exponent_size, uint8_t * exponent)
    {
        uint8_t inr[SEC_MAX_MODULUS_SIZE];

        reverseit(modulus, modulus_size);
        reverseit(exponent, exponent_size);

        for (uint32_t i = 0; i < in_len ; i++){
            inr[in_len-1-i] = in[i];
        }

        int outlen = 0;
        {
            Bignum mod(modulus, modulus_size);
            Bignum exp(exponent, exponent_size);
            Bignum x(inr, in_len);
            Bignum y = x.mod_exp(exp, mod); 
            outlen = BN_bn2bin(&y.bn, out);
        }
        
        reverseit(out, outlen);

        if (outlen < static_cast<int>(modulus_size)){
            memset(out + outlen, 0, modulus_size - outlen);
        }
    }


    static void ssl_xxxxxx(uint8_t * client_random, 
                           uint32_t in_len, const uint8_t * in, 
                           uint32_t mod_len, const uint8_t * mod, 
                           uint32_t exp_len, const uint8_t * exp)
    {
        uint8_t l_out[64]; memset(l_out, 0, 64);
        uint8_t l_in[64];  rmemcpy(l_in, in, in_len);
        uint8_t l_mod[64]; rmemcpy(l_mod, mod, mod_len);
        uint8_t l_exp[64]; rmemcpy(l_exp, exp, exp_len);

        Bignum lmod(l_mod, mod_len);
        Bignum lexp(l_exp, exp_len);
        Bignum lin(l_in, in_len);
        Bignum lout = lin.mod_exp(lexp, lmod);
        int rv = BN_bn2bin(&lout.bn, l_out);
        if (rv <= 64) {
            reverseit(l_out, rv);
            memcpy(client_random, l_out, 64);
        }
    }


    static void sec_make_40bit(uint8_t* key)
    {
        key[0] = 0xd1;
        key[1] = 0x26;
        key[2] = 0x9e;
    }
};


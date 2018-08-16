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

#include <cstring>

#include "utils/bitfu.hpp"
#include "system/ssl_mod_exp.hpp"


enum {
    SEC_RANDOM_SIZE   = 32,
    SEC_MODULUS_SIZE  = 64,
    SEC_MAX_MODULUS_SIZE  = /*256*/512,
    SEC_PADDING_SIZE  =  8,
    SEC_EXPONENT_SIZE =  4
};


class ssllib
{
public:
    static void rsa_encrypt(uint8_t * out, uint32_t in_len, uint8_t const * in, uint32_t modulus_size, uint8_t * modulus, uint32_t exponent_size, uint8_t * exponent)
    {
        // TODO check out buffer size
        size_t out_len = 256;
        uint8_t inr[SEC_MAX_MODULUS_SIZE];

        reverseit(modulus, modulus_size);
        reverseit(exponent, exponent_size);

        for (uint32_t i = 0; i < in_len ; i++){
            inr[in_len-1-i] = in[i];
        }

        size_t outlen = mod_exp(out, out_len, inr, in_len, modulus, modulus_size, exponent, exponent_size);

        reverseit(out, outlen);

        if (outlen < modulus_size){
            memset(out + outlen, 0, modulus_size - outlen);
        }
    }

    static void ssl_xxxxxx(uint8_t * client_random,
                           uint32_t in_len, const uint8_t * in,
                           uint32_t mod_len, const uint8_t * mod,
                           uint32_t exp_len, const uint8_t * exp)
    {
        uint8_t l_out[64] = {};
        uint8_t l_in[64];  rmemcpy(l_in, in, in_len);
        uint8_t l_mod[64]; rmemcpy(l_mod, mod, mod_len);
        uint8_t l_exp[64]; rmemcpy(l_exp, exp, exp_len);

        size_t outlen = mod_exp(l_out, 64, l_in, in_len, l_mod, mod_len, l_exp, exp_len);

        if (outlen <= 64) {
            reverseit(l_out, outlen);
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


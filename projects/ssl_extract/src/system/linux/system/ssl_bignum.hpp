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
   Author(s): Christophe Grosjean

   BigNum Headers

*/

#pragma once

#include <openssl/bn.h>

class Bignum
{
public:
    BIGNUM bn;
    Bignum(unsigned long w) 
    {
        BN_init(&this->bn);
        BN_set_word(&this->bn, w);
    }
    Bignum(const uint8_t * data, size_t len) 
    {
        BN_init(&this->bn);
        BN_bin2bn(data, len, &this->bn);
    }
    Bignum(const BIGNUM * bn) 
    {
        BN_init(&this->bn);
        BN_copy(&this->bn, bn);
    }
    Bignum(const Bignum & bn) 
    {
        BN_init(&this->bn);
        BN_copy(&this->bn, &bn.bn);
    }
    ~Bignum()
    {
        BN_free(&this->bn);
    }
    
    unsigned long get_word()
    {
        return BN_get_word(&this->bn);
    }
    Bignum mod_exp(const Bignum & e, const Bignum & mod) const
    {
        BN_CTX *ctx = BN_CTX_new();
        Bignum result(0UL);
        BN_mod_exp(&result.bn, &this->bn, &e.bn, &mod.bn, ctx);
        BN_CTX_free(ctx);
        return result;
    }
    Bignum operator+(const Bignum & b) const
    {
        Bignum result(b);
        BN_add(&result.bn, &result.bn, &this->bn);
        return result;
    }
    bool operator!=(const Bignum & b) const
    {
        return BN_cmp(&this->bn, &b.bn) != 0;
    }
    bool operator==(const Bignum & b) const
    {
        return BN_cmp(&this->bn, &b.bn) == 0;
    }
};

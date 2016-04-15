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
    BIGNUM * n;
    Bignum(unsigned long w) 
    {
        this->n = BN_new();
        BN_set_word(this->n, w);
    }
    Bignum(BIGNUM * bn) 
    {
        this->n = BN_dup(bn);
    }
    ~Bignum()
    {
        BN_free(this->n);
    }  
    Bignum operator+(const Bignum & b) const
    {
        Bignum result(BN_dup(this->n));
        BN_add(result.n, result.n, b.n);
        return result;
    }
    bool operator!=(const Bignum & b) const
    {
        return BN_cmp(this->n, b.n) != 0;
    }
    bool operator==(const Bignum & b) const
    {
        return BN_cmp(this->n, b.n) == 0;
    }
};

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

class Bignum
{
public:
    Bignum(unsigned long w) 
    {
    }
    Bignum(const uint8_t * data, size_t len) 
    {
    }
    Bignum(const Bignum & bn) 
    {
    }
    ~Bignum()
    {
    }

    size_t get_bin(uint8_t * out, size_t out_len)
    {
        return out_len;
    }

    unsigned long get_word()
    {
        return 0UL;
    }
    Bignum mod_exp(const Bignum & e, const Bignum & mod) const
    {
        return Bignum(0UL);
    }
    Bignum operator+(const Bignum & b) const
    {
        return Bignum(0UL);
    }
    bool operator!=(const Bignum & b) const
    {
        return true;
    }
    bool operator==(const Bignum & b) const
    {
        return true;
    }
};

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
   Copyright (C) Wallix 2010-2014
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan

   openssl headers

   Based on xrdp and rdesktop
   Copyright (C) Jay Sorg 2004-2010
   Copyright (C) Matthew Chapman 1999-2007
*/

#pragma once

#include "openssl_crypto.hpp"

#include <cstdint>


class SslRC4
{
    RC4_KEY rc4;

public:
    SslRC4() = default;

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        RC4_set_key(&this->rc4, key_size, key);
    }

    void crypt(size_t data_size, const uint8_t * const indata, uint8_t * const outdata)
    {
        RC4(&this->rc4, data_size, indata, outdata);
    }
};

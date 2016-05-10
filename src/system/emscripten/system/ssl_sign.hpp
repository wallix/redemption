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

/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
class Sign
{
    SslSha1 sha1;
    const uint8_t * const key;
    size_t key_size;

    public:
    Sign(const uint8_t * const key, size_t key_size)
        : key(key)
        , key_size(key_size)
    {
        this->sha1.update(this->key, this->key_size);
        const uint8_t sha1const[40] = {
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
        };
        sha1.update(sha1const, 40);
    }

    void update(const uint8_t * const data, size_t data_size) {
        this->sha1.update(data, data_size);
    }

    void final(uint8_t * out, size_t out_size) {
        uint8_t shasig[20];
        this->sha1.final(shasig, 20);

        SslMd5 md5;
        md5.update(this->key, this->key_size);
        const uint8_t sigconst[48] = {
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c
        };
        md5.update(sigconst, sizeof(sigconst));
        md5.update(shasig, sizeof(shasig));
        md5.final(out, out_size);
    }
};


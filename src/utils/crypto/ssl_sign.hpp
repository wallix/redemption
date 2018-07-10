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
   Author(s): Christophe Grosjean, Clement Moroldo

*/

#pragma once

#include "system/ssl_sha1.hpp"
#include "system/ssl_md5.hpp"

#include <algorithm>


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
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
        };
        sha1.update(sha1const, 40);
    }

    void update(const uint8_t * const data, size_t data_size) {
        this->sha1.update(data, data_size);
    }

    void final(uint8_t * out, size_t out_size) {
        uint8_t shasig[SslSha1::DIGEST_LENGTH];
        this->sha1.final(shasig);

        SslMd5 md5;
        md5.update(this->key, this->key_size);
        const uint8_t sigconst[48] = {
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c
        };
        md5.update(sigconst, sizeof(sigconst));
        md5.update(shasig, sizeof(shasig));

        // TODO: check out_size provided to sign.final()
        // if it's already MD5::DIGEST_LENGTH
        // no need to provide it

        uint8_t tmp[SslMd5::DIGEST_LENGTH];
        md5.final(tmp);
        memcpy(out, tmp, std::min(out_size, static_cast<size_t>(SslMd5::DIGEST_LENGTH)));

    }
};

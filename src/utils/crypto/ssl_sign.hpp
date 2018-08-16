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
#include "utils/sugar/byte.hpp"

#include <algorithm>


/* Generate a MAC hash (5.2.3.1), using a combination of SHA1 and MD5 */
class Sign
{
    SslSha1 sha1;
    const const_byte_array key;

public:
    Sign(const_byte_array key)
        : key(key)
    {
        this->sha1.update(this->key);
        const uint8_t sha1const[40] = {
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
            0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
        };
        sha1.update(make_array_view(sha1const));
    }

    void update(const_byte_array data) {
        this->sha1.update(data);
    }

    template<std::size_t OutLen>
    void final(uint8_t (&out)[OutLen]) {
        uint8_t shasig[SslSha1::DIGEST_LENGTH];
        this->sha1.final(shasig);

        SslMd5 md5;
        md5.update(this->key);
        const uint8_t sigconst[48] = {
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c,
            0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c
        };
        md5.update(make_array_view(sigconst));
        md5.update(make_array_view(shasig));

        static_assert(OutLen <= SslMd5::DIGEST_LENGTH);

        if constexpr (OutLen == SslMd5::DIGEST_LENGTH) {
            md5.final(out);
        }
        else {
            uint8_t tmp[SslMd5::DIGEST_LENGTH];
            md5.final(tmp);
            memcpy(out, tmp, OutLen);
        }
    }
};

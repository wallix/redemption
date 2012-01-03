/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__SEC_UTILS_HPP__)
#define __SEC_UTILS_HPP__

#include "ssl_calls.hpp"

// 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
// Both SHA1 and MD5 algorithms are used.
static inline void sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
{
    uint8_t shasig[20];
    uint8_t pad[4];
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    ssllib ssl;

    for (int i = 0; i < 3; i++) {
        memset(pad, salt + i, i + 1);

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, pad, i + 1);
        ssl.sha1_update(&sha1, in, 48);
        ssl.sha1_update(&sha1, salt1, 32);
        ssl.sha1_update(&sha1, salt2, 32);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, &out[i * 16]);
    }
}

// 16-byte transformation used to generate export keys (6.2.2).
static inline void sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
{
    SSL_MD5 md5;

    ssllib ssl;

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, in, 16);
    ssl.md5_update(&md5, salt1, 32);
    ssl.md5_update(&md5, salt2, 32);
    ssl.md5_final(&md5, out);
}

#endif

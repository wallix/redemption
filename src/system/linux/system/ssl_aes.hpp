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

#include <cstdint>
#include <cstring>

#include <openssl/aes.h>


enum class AES_direction : bool {
    SSL_AES_ENCRYPT = 0,
    SSL_AES_DECRYPT = 1
};

template<std::size_t KeyLength>
class SslAes_CBC
{
    int mode; // SSL_AES_ENCRYPT or SSL_AES_DECRYPT
    AES_KEY key;

public:
    struct t_iv {
        uint8_t iv[16];
    } tiv;

    SslAes_CBC(const uint8_t key[KeyLength/8], const uint8_t (& iv)[16], AES_direction mode)
    : mode(mode == AES_direction::SSL_AES_DECRYPT ? AES_DECRYPT : AES_ENCRYPT)
    {
        (mode == AES_direction::SSL_AES_DECRYPT ? AES_set_decrypt_key : AES_set_encrypt_key)(
            key, KeyLength, &this->key
        );
        memcpy(this->tiv.iv, iv, sizeof(this->tiv.iv));
    }

    void crypt_cbc(size_t data_size, const uint8_t * const in, uint8_t * const out) {
       AES_cbc_encrypt(in, out, data_size, &this->key, this->tiv.iv, mode);
    }
};

using SslAes128_CBC = SslAes_CBC<128>;
using SslAes192_CBC = SslAes_CBC<192>;
using SslAes256_CBC = SslAes_CBC<256>;

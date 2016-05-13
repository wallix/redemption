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
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "core/error.hpp"
#include "openssl_crypto.hpp"
#include "utils/log.hpp"
#include "utils/bitfu.hpp"

class SslAes128_CBC
{
    int mode; // AES_ENCRYPT or AES_DECRYPT
    AES_KEY key;
    
    public:
    struct t_iv {
        uint8_t iv[16];
    } tiv;

    SslAes128_CBC(const uint8_t key[16], const uint8_t (& iv)[16], int mode)
        : mode(mode)
        , key([](const uint8_t key[16], int mode)-> const AES_KEY {
            AES_KEY tmp_key;
            (mode==AES_DECRYPT?AES_set_decrypt_key:AES_set_encrypt_key)(key, 128, &tmp_key);
            return std::move(tmp_key);
          }(key, mode))
        , tiv([](const uint8_t (& iv)[16])-> const t_iv {
            t_iv tmp;
            memcpy(tmp.iv, iv, sizeof(tmp.iv));
            return tmp;
          }(iv))
    {
    }

    void crypt_cbc(size_t data_size, const uint8_t * const in, uint8_t * const out) {
       AES_cbc_encrypt(in, out, data_size, &(this->key), this->tiv.iv, mode);
    }
};

class SslAes192_CBC
{
    int mode; // AES_ENCRYPT or AES_DECRYPT
    AES_KEY key;
    
    public:
    struct t_iv {
        uint8_t iv[16];
    } tiv;

    SslAes192_CBC(const uint8_t key[24], const uint8_t (& iv)[16], int mode)
        : mode(mode)
        , key([](const uint8_t key[24], int mode)-> const AES_KEY {
            AES_KEY tmp_key;
            (mode==AES_DECRYPT?AES_set_decrypt_key:AES_set_encrypt_key)(key, 192, &tmp_key);
            return std::move(tmp_key);
          }(key, mode))
        , tiv([](const uint8_t (& iv)[16])-> const t_iv {
            t_iv tmp;
            memcpy(tmp.iv, iv, sizeof(tmp.iv));
            return tmp;
          }(iv))
    {
    }

    void crypt_cbc(size_t data_size, const uint8_t * const in, uint8_t * const out) {
       AES_cbc_encrypt(in, out, data_size, &(this->key), this->tiv.iv, mode);
    }
};


class SslAes256_CBC
{
    int mode; // AES_ENCRYPT or AES_DECRYPT
    AES_KEY key;
    
    public:
    struct t_iv {
        uint8_t iv[16];
    } tiv;

    SslAes256_CBC(const uint8_t key[32], const uint8_t (& iv)[16], int mode)
        : mode(mode)
        , key([](const uint8_t key[32], int mode)-> const AES_KEY {
            AES_KEY tmp_key;
            (mode==AES_DECRYPT?AES_set_decrypt_key:AES_set_encrypt_key)(key, 256, &tmp_key);
            return std::move(tmp_key);
          }(key, mode))
        , tiv([](const uint8_t (& iv)[16])-> const t_iv {
            t_iv tmp;
            memcpy(tmp.iv, iv, sizeof(tmp.iv));
            return tmp;
          }(iv))
    {
    }

    void crypt_cbc(size_t data_size, const uint8_t * const in, uint8_t * const out) {
       AES_cbc_encrypt(in, out, data_size, &(this->key), this->tiv.iv, mode);
    }
};

class SslAES
{
    AES_KEY e_key;
    AES_KEY d_key;

    uint8_t iv;

    public:
    SslAES(){}

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        if ((key_size != 16) &&
            (key_size != 24) &&
            (key_size != 32)) {
            LOG(LOG_ERR, "Unexpected AES Key size");
            return;
        }

        AES_set_encrypt_key(key, key_size * 8, &(this->e_key));
        AES_set_decrypt_key(key, key_size * 8, &(this->d_key));
    }

    void crypt_cbc(size_t data_size, uint8_t * ivec,
                  const uint8_t * const indata, uint8_t * const outdata) {
       AES_cbc_encrypt(indata, outdata, data_size, &(this->e_key), ivec, AES_ENCRYPT);
    }

    void decrypt_cbc(size_t data_size, uint8_t * ivec,
                    const uint8_t * const indata, uint8_t * const outdata) {
       AES_cbc_encrypt(indata, outdata, data_size, &(this->d_key), ivec, AES_DECRYPT);
    }
};


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

#include "ssl_sha1.hpp"
#include "ssl_sha256.hpp"
#include "ssl_md5.hpp"
#include "ssl_md4.hpp"
#include "ssl_rc4.hpp"

#include "core/error.hpp"

#include "utils/log.hpp"
#include "utils/bitfu.hpp"
#include <strings.h>


enum {
    SEC_RANDOM_SIZE   = 32,
    SEC_MODULUS_SIZE  = 64,
    SEC_MAX_MODULUS_SIZE  = /*256*/512,
    SEC_PADDING_SIZE  =  8,
    SEC_EXPONENT_SIZE =  4
};



class SslRC4
{
    RC4_KEY rc4;

    public:
    SslRC4(){}

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        RC4_set_key(&this->rc4, key_size, key);
    }

    void crypt(size_t data_size, const uint8_t * const indata, uint8_t * const outdata){
        RC4(&this->rc4, data_size, indata, outdata);
    }
};

class SslAES
{
    AES_KEY e_key;
    AES_KEY d_key;

//    uint8_t iv;

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

class SslHMAC_Sha1
{
    HMAC_CTX hmac;

    public:
    SslHMAC_Sha1(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_sha1(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC_Sha1()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (SHA_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[SHA_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};

class SslHMAC_Sha256
{
    HMAC_CTX hmac;

    public:
    SslHMAC_Sha256(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_sha256(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC_Sha256()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (SHA256_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[SHA256_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};


class SslHMAC_Md5
{
    HMAC_CTX hmac;

    public:
    SslHMAC_Md5(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_md5(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC_Md5()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (MD5_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD5_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};


class SslHMAC
{
    HMAC_CTX hmac;

    public:

    template<typename T>
    SslHMAC(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = 0;
        res = HMAC_Init_ex(&this->hmac, key, key_size, EVP_md5(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~SslHMAC()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = 0;
        res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        unsigned int len = 0;
        int res = 0;
        if (MD5_DIGEST_LENGTH > out_data_size){
            uint8_t tmp[MD5_DIGEST_LENGTH];
            res = HMAC_Final(&this->hmac, tmp, &len);
            if (res == 0) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            memcpy(out_data, tmp, out_data_size);
            return;
        }
        res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};



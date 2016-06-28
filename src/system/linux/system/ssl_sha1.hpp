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
#include <cassert>

#include "core/error.hpp"
#include "openssl_crypto.hpp"


class SslSha1
{
    SHA_CTX sha1;

public:
    enum : unsigned { DIGEST_LENGTH = 20 };

    SslSha1()
    {
        int res = 0;
        res = SHA1_Init(&this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_INIT_FAILED);
        }
    }

    void update(const uint8_t * const data,  size_t data_size)
    {
        int res = 0;
        res = SHA1_Update(&this->sha1, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_UPDATE_FAILED);
        }
    }

    void final(uint8_t * out_data, size_t out_data_size)
    {
        assert(DIGEST_LENGTH == out_data_size);
        int res = 0;
        res = SHA1_Final(out_data, &this->sha1);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_SHA1_FINAL_FAILED);
        }
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
        if (SslSha1::DIGEST_LENGTH > out_data_size){
            uint8_t tmp[SslSha1::DIGEST_LENGTH];
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

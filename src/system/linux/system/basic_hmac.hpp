/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cstdint>
#include <cstring>

#include "core/error.hpp"
#include "openssl_crypto.hpp"

namespace detail_
{
template<const EVP_MD * (* evp)(), std::size_t DigestLength>
class basic_HMAC
{
    HMAC_CTX hmac;

public:
    basic_HMAC(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = HMAC_Init_ex(&this->hmac, key, key_size, evp(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
    }

    ~basic_HMAC()
    {
        HMAC_CTX_cleanup(&this->hmac);
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t (&out_data)[DigestLength])
    {
        unsigned int len = 0;
        int res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};

template<const EVP_MD * (* evp)(), std::size_t DigestLength>
class DelayedHMAC
{
    bool initialized = false;
    HMAC_CTX hmac;

public:
    DelayedHMAC() {}

    void init(const uint8_t * const key, size_t key_size)
    {
        HMAC_CTX_init(&this->hmac);
        int res = HMAC_Init_ex(&this->hmac, key, key_size, evp(), nullptr);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
        }
        this->initialized = true;
    }

    ~DelayedHMAC()
    {
        if (this->initialized){
            HMAC_CTX_cleanup(&this->hmac);
        }
    }

    void update(const uint8_t * const data, size_t data_size)
    {
        int res = HMAC_Update(&this->hmac, data, data_size);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
        }
    }

    void final(uint8_t (&out_data)[DigestLength])
    {
        unsigned int len = 0;
        int res = HMAC_Final(&this->hmac, out_data, &len);
        if (res == 0) {
            throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
        }
    }
};

}

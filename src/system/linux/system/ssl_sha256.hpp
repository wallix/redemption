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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan
*/

#pragma once

#include "core/error.hpp"
#include "system/basic_hmac.hpp"

#include <cstdint>
#include <cstring>
#include <cassert>

#include <openssl/sha.h>


class SslSha256_Delayed
{
    SHA256_CTX sha256;

public:
    enum : unsigned { DIGEST_LENGTH = SHA256_DIGEST_LENGTH };

    SslSha256_Delayed() = default;

    void init()
    {
        if (0 == SHA256_Init(&this->sha256)){
            throw Error(ERR_SSL_CALL_SHA256_INIT_FAILED);
        }
    }

    void update(bytes_view data)
    {
        if (0 == SHA256_Update(&this->sha256, data.as_u8p(), data.size())){
            throw Error(ERR_SSL_CALL_SHA256_UPDATE_FAILED);
        }
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH])
    {
        if (0 == SHA256_Final(out_data, &this->sha256)){
            throw Error(ERR_SSL_CALL_SHA256_FINAL_FAILED);
        }
    }

    void unchecked_final(uint8_t * out_data)
    {
        if (0 == SHA256_Final(out_data, &this->sha256)){
            throw Error(ERR_SSL_CALL_SHA256_FINAL_FAILED);
        }
    }
};

class SslSha256
{
    SslSha256_Delayed sha256;

public:
    enum : unsigned { DIGEST_LENGTH = SslSha256_Delayed::DIGEST_LENGTH };

    SslSha256()
    {
        sha256.init();
    }

    void update(bytes_view data)
    {
        sha256.update(data);
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH])
    {
        sha256.final(out_data);
    }

    void unchecked_final(uint8_t * out_data)
    {
        sha256.unchecked_final(out_data);
    }
};

using SslHMAC_Sha256 = detail_::basic_HMAC<&EVP_sha256, SslSha256::DIGEST_LENGTH>;
using SslHMAC_Sha256_Delayed = detail_::DelayedHMAC<&EVP_sha256, SslSha256::DIGEST_LENGTH>;


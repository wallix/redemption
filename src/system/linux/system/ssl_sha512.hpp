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


class SslSha512
{
    SHA512_CTX sha512;

public:
    enum : unsigned { DIGEST_LENGTH = SHA512_DIGEST_LENGTH };

    SslSha512()
    {
        if (0 == SHA512_Init(&this->sha512)){
            throw Error(ERR_SSL_CALL_SHA1_INIT_FAILED);
        }
    }

    void update(const_bytes_view data)
    {
        if (0 == SHA512_Update(&this->sha512, data.to_u8p(), data.size())){
            throw Error(ERR_SSL_CALL_SHA1_UPDATE_FAILED);
        }
    }

    void final(uint8_t (&out_data)[DIGEST_LENGTH])
    {
        if (0 == SHA512_Final(out_data, &this->sha512)){
            throw Error(ERR_SSL_CALL_SHA1_FINAL_FAILED);
        }
    }
};


using SslHMAC_Sha512 = detail_::basic_HMAC<&EVP_sha512, SslSha512::DIGEST_LENGTH>;

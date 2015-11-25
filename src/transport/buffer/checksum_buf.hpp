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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_TRANSPORT_BUFFER_CHECKSUM_BUF_HPP
#define REDEMPTION_TRANSPORT_BUFFER_CHECKSUM_BUF_HPP

#include "cryptofile.h"
#include "ssl_calls.hpp"

#include <memory>


namespace transbuf {

using std::size_t;

template<class Buf>
class ochecksum_buf
: public Buf
{
    struct HMac
    {
        HMAC_CTX hmac;
        bool initialized = false;

        HMac() = default;

        ~HMac() {
            if (this->initialized) {
                HMAC_CTX_cleanup(&this->hmac);
            }
        }

        void init(const uint8_t * const crypto_key, size_t key_len) {
            HMAC_CTX_init(&this->hmac);
            if (!HMAC_Init_ex(&this->hmac, crypto_key, key_len, EVP_sha256(), nullptr)) {
                throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
            }
            this->initialized = true;
        }

        void update(const void * const data, size_t data_size) {
            assert(this->initialized);
            if (!HMAC_Update(&this->hmac, reinterpret_cast<uint8_t const *>(data), data_size)) {
                throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
            }
        }

        void final(uint8_t (&out_data)[SHA256_DIGEST_LENGTH]) {
            assert(this->initialized);
            unsigned int len = 0;
            if (!HMAC_Final(&this->hmac, out_data, &len)) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            this->initialized = false;
        }
    };

    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    HMac hmac;
    HMac quick_hmac;
    CryptoContext * cctx;
    size_t file_size = nosize;

public:
    ochecksum_buf(CryptoContext * cctx)
    : cctx(cctx)
    {}

    template<class... Ts>
    int open(Ts && ... args)
    {
        this->hmac.init(cctx->hmac_key, sizeof(cctx->hmac_key));
        this->quick_hmac.init(cctx->hmac_key, sizeof(cctx->hmac_key));
        int ret = this->Buf::open(args...);
        this->file_size = 0;
        return ret;
    }

    ssize_t write(const void * data, size_t len)
    {
        REDASSERT(this->file_size != nosize);
        this->hmac.update(data, len);
        if (this->file_size < quick_size) {
            auto const remaining = std::min(this->file_size - quick_size, len);
            this->quick_hmac.update(data, len);
            this->file_size += remaining;
        }
        return this->Buf::write(data, len);
    }

    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
    {
        REDASSERT(this->file_size != nosize);
        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
        this->file_size = nosize;
        return this->Buf::close();
    }

    int close() {
        return this->Buf::close();
    }
};

}

#endif

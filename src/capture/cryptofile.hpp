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
 *   Copyright (C) Wallix 2010-2017
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#pragma once

#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <array>
#include <vector>
#include <iterator>
#include <algorithm>
#include <snappy-c.h>

#include "utils/log.hpp"
#include "utils/sugar/byte.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "openssl_crypto.hpp"
#include "system/ssl_sha256.hpp"

using MD_HASH = SslSha256;

enum {
    DERIVATOR_LENGTH = 8
};

extern "C" {
    typedef int get_hmac_key_prototype(uint8_t * buffer);
    typedef int get_trace_key_prototype(uint8_t const * base, int len, uint8_t * buffer, unsigned oldscheme);
}


constexpr std::size_t CRYPTO_KEY_LENGTH = MD_HASH::DIGEST_LENGTH;
constexpr std::size_t HMAC_KEY_LENGTH = MD_HASH::DIGEST_LENGTH;


class CryptoContext : noncopyable
{
    uint8_t master_key[CRYPTO_KEY_LENGTH] {};
    uint8_t hmac_key[HMAC_KEY_LENGTH] {};
    std::vector<uint8_t> master_derivator;

    get_hmac_key_prototype * get_hmac_key_cb = nullptr;
    get_trace_key_prototype * get_trace_key_cb = nullptr;

    bool master_key_loaded = false;
    bool hmac_key_loaded = false;


public:
    bool old_encryption_scheme = false;
    bool one_shot_encryption_scheme = false;

    bool with_encryption = false;
    bool with_checksum = false;

public:
    auto get_hmac_key() -> uint8_t const (&)[HMAC_KEY_LENGTH]
    {
        if (!this->hmac_key_loaded){
            if (!this->get_hmac_key_cb) {
                LOG(LOG_ERR, "CryptoContext: undefined hmac_key callback");
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }
            // if we have a callback ask key
            if (int err = this->get_hmac_key_cb(this->hmac_key)) {
                LOG(LOG_ERR, "CryptoContext: get_hmac_key_cb: callback error: %d", err);
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }
            this->hmac_key_loaded = true;
        }
        return this->hmac_key;
    }

    // for test only
    const uint8_t * get_master_key() const
    {
        assert(this->master_key_loaded);
        return this->master_key;
    }

    void set_master_derivator(const_byte_array derivator)
    {
        if ((this->master_key_loaded || this->master_derivator.size())
         && not (this->master_derivator.size() == derivator.size()
          && std::equal(derivator.begin(), derivator.end(), this->master_derivator.begin())
        )) {
            LOG(LOG_ERR, "CryptoContext: master derivator is already defined");
            throw Error(ERR_WRM_INVALID_INIT_CRYPT);
        }
        this->master_derivator.assign(derivator.begin(), derivator.end());
    }

    void set_with_encryption (bool encryption) {
        this->with_encryption = encryption;
    }

    void set_with_checksum (bool checksum) {
        this->with_checksum = checksum;
    }

    bool get_with_encryption () {
        return this->with_encryption;
    }

    bool get_with_checksum () {
        return this->with_checksum;
    }
private:
    // force extension to "mwrm" if it's .log
    static array_view_const_u8 get_normalized_derivator(
        std::unique_ptr<uint8_t[]> & normalized_derivator,
        const_byte_array derivator
    )
    {
        using reverse_iterator = std::reverse_iterator<array_view_const_u8::const_iterator>;
        reverse_iterator const first(derivator.end());
        reverse_iterator const last(derivator.begin());
        reverse_iterator const p = std::find(first, last, '.');
        constexpr auto ext = cstr_array_view(".log");
        if (derivator.end()-p.base() == ext.size() - 1
         && std::equal(
             p.base(), p.base() + ext.size() - 1,
             reinterpret_cast<uint8_t const*>(ext.data() + 1)
        )) {
            constexpr auto extmwrm = cstr_array_view(".mwrm");
            auto const prefix_len = (p == last ? derivator.end() : p.base() - 1) - derivator.begin();
            auto const new_len = prefix_len + extmwrm.size();

            normalized_derivator = std::make_unique<uint8_t[]>(new_len + 1);
            memcpy(normalized_derivator.get(), derivator.data(), prefix_len);
            memcpy(normalized_derivator.get() + prefix_len, extmwrm.data(), extmwrm.size());
            normalized_derivator[new_len] = 0;

            return array_view_const_u8{normalized_derivator.get(), new_len};
        }

        return derivator;
    }

    void load_trace_key(uint8_t (&buffer)[MD_HASH::DIGEST_LENGTH], const_byte_array derivator)
    {
        std::unique_ptr<uint8_t[]> normalized_derivator_gc;
        auto const new_derivator = get_normalized_derivator(normalized_derivator_gc, derivator);

        if (int err = this->get_trace_key_cb(
            new_derivator.data()
          , static_cast<int>(new_derivator.size())
          , buffer
          , this->old_encryption_scheme?1:0
        )) {
            LOG(LOG_ERR, "CryptoContext: get_trace_key_cb: callback error: %d", err);
            throw Error(ERR_WRM_INVALID_INIT_CRYPT);
        }
    }

public:
    void get_derived_key(uint8_t (&trace_key)[CRYPTO_KEY_LENGTH], const_byte_array derivator)
    {
        if (this->old_encryption_scheme){
            if (this->get_trace_key_cb != nullptr){
                // if we have a callback ask key
                uint8_t tmp[MD_HASH::DIGEST_LENGTH];
                this->load_trace_key(tmp, derivator);
                memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
                return;
            }
        }

        if (!this->master_key_loaded){
            if (this->get_trace_key_cb == nullptr) {
                LOG(LOG_ERR, "CryptoContext: undefined trace_key callback");
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }

            if (this->master_derivator.empty()) {
                LOG(LOG_ERR, "CryptoContext: derivator is undefined");
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }

            this->load_trace_key(this->master_key, this->master_derivator);

            this->master_key_loaded = true;
            //this->master_derivator.clear();
            //this->master_derivator.shrink_to_fit();
        }

        if (this->one_shot_encryption_scheme){
            memcpy(trace_key, this->master_key, HMAC_KEY_LENGTH);
        }
        else {
            uint8_t tmp[MD_HASH::DIGEST_LENGTH];
            {
                MD_HASH sha256;
                sha256.update(derivator.data(), derivator.size());
                sha256.final(tmp);
            }
            {
                MD_HASH sha256;
                sha256.update(tmp, DERIVATOR_LENGTH);
                sha256.update(this->master_key, CRYPTO_KEY_LENGTH);
                sha256.final(tmp);
            }
            static_assert(sizeof(trace_key) == sizeof(tmp), "");
            memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
        }
    }

    CryptoContext() = default;

    size_t unbase64(char *buffer, size_t bufsiz, const char *txt)
    {
        const uint8_t _base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned int bits = 0;
        int nbits = 0;
        char base64tbl[256];
        size_t nbytes = 0;

        memset(base64tbl, -1, sizeof base64tbl);

        for (unsigned i = 0; _base64chars[i]; i++) {
            base64tbl[_base64chars[i]] = i;
        }

        base64tbl[int('.')] = 62;
        base64tbl[int('-')] = 62;
        base64tbl[int('_')] = 63;

        while (*txt) {
            char const v = base64tbl[static_cast<uint8_t>(*txt)];
            if (v >= 0) {
                bits <<= 6;
                bits += v;
                nbits += 6;
                if (nbits >= 8) {
                    if (nbytes < bufsiz)
                        *buffer++ = (bits >> (nbits - 8));
                    nbytes++;
                    nbits -= 8;
                }
            }
            txt++;
        }

        return nbytes;
    }

    class key_data : private const_byte_array
    {
        static constexpr std::size_t key_length = CRYPTO_KEY_LENGTH;

        static_assert(sizeof(master_key) == key_length, "");
        static_assert(sizeof(hmac_key) == key_length, "");

        friend class CryptoContext;

    public:
        template<class T>
        key_data(T const & bytes32) noexcept
        : const_byte_array(bytes32)
        {
            assert(this->size() == key_length);
        }

        template<class T, std::size_t array_length>
        key_data(std::array<T, array_length> const & data) noexcept
        : const_byte_array(data.data(), data.size())
        {
            static_assert(array_length == key_length, "");
        }

        template<class T, std::size_t array_length>
        key_data(T const (& data)[array_length]) noexcept
        : const_byte_array(data, array_length)
        {
            static_assert(array_length == key_length, "");
        }
    };

    void set_master_key(key_data key) noexcept
    {
        memcpy(this->master_key, key.data(), sizeof(this->master_key));
        this->master_key_loaded = true;
    }

    void set_hmac_key(key_data key) noexcept
    {
        memcpy(this->hmac_key, key.data(), sizeof(this->hmac_key));
        this->hmac_key_loaded = true;
    }

    void set_get_hmac_key_cb(get_hmac_key_prototype * get_hmac_key_cb)
    {
        this->get_hmac_key_cb = get_hmac_key_cb;
    }

    void set_get_trace_key_cb(get_trace_key_prototype * get_trace_key_cb)
    {
        this->get_trace_key_cb = get_trace_key_cb;
    }
};

inline const EVP_CIPHER * get_cipher_and_prepare_key(
        uint8_t const (&trace_key)[CRYPTO_KEY_LENGTH],
        unsigned char key[CRYPTO_KEY_LENGTH]
) noexcept
{
    const EVP_CIPHER * cipher = ::EVP_aes_256_cbc();
    const uint8_t salt[] = { 0x39, 0x30, 0x00, 0x00, 0x31, 0xd4, 0x00, 0x00 };
    const int     nrounds = 5;
    const int i = ::EVP_BytesToKey(
        cipher, EVP_sha1(), salt, trace_key,
        CRYPTO_KEY_LENGTH, nrounds, key, nullptr
    );
    if (i != CRYPTO_KEY_LENGTH) {
        LOG(LOG_INFO, "Can't read EVP_BytesToKey");
        return nullptr;
    }
    return cipher;
}

struct CipherContext : noncopyable
{
    /// init or reinit
    void init() noexcept
    {
        this->deinit();
        ::EVP_CIPHER_CTX_init(&this->ectx);
        this->is_init = true;
    }

    void deinit() noexcept
    {
        if (this->is_init) {
            EVP_CIPHER_CTX_cleanup(&this->ectx);
            this->is_init = false;
        }
    }

    ~CipherContext()
    {
        this->deinit();
    }

    EVP_CIPHER_CTX* get_ctx() noexcept
    {
        return &this->ectx;
    }

    bool is_initialized() const noexcept
    {
        return this->is_init;
    }

private:
    EVP_CIPHER_CTX ectx; // [en|de]cryption context
    bool is_init = false;
};

struct EncryptContext
{
    /// init or reinit
    bool init(uint8_t const (&trace_key)[CRYPTO_KEY_LENGTH], uint8_t * iv) noexcept
    {
        unsigned char key[CRYPTO_KEY_LENGTH];
        const EVP_CIPHER * cipher = get_cipher_and_prepare_key(trace_key, key);

        if (!cipher) {
            return false;
        }

        this->cctx.init();
        if (::EVP_EncryptInit_ex(this->cctx.get_ctx(), cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "Can't read EVP_EncryptInit_ex");
            return false;
        }

        return true;
    }

    /**
     * \brief Encrypt \c src_buf into \c dst_buf.
     * \return encrypted output size
     */
    size_t encrypt(uint8_t const * src_buf, size_t src_sz, uint8_t * dst_buf, size_t dst_sz)
    {
        assert(this->cctx.is_initialized());
        int safe_size = dst_sz;
        int remaining_size = 0;
        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(this->cctx.get_ctx(), nullptr, nullptr, nullptr, nullptr) != 1
         || EVP_EncryptUpdate(this->cctx.get_ctx(), dst_buf, &safe_size, src_buf, src_sz) != 1
         || EVP_EncryptFinal_ex(this->cctx.get_ctx(), dst_buf + safe_size, &remaining_size) != 1) {
            LOG(LOG_ERR, "EncryptContext::encrypt");
            throw Error(ERR_SSL_CALL_FAILED);
        }
        return size_t(safe_size + remaining_size);
    }

private:
    CipherContext cctx;
};

struct DecryptContext
{
    /// init or reinit
    bool init(uint8_t const (&trace_key)[CRYPTO_KEY_LENGTH], uint8_t const * iv) noexcept
    {
        unsigned char key[CRYPTO_KEY_LENGTH];
        const EVP_CIPHER * cipher = get_cipher_and_prepare_key(trace_key, key);

        if (!cipher) {
            return false;
        }

        this->cctx.init();
        if (::EVP_DecryptInit_ex(this->cctx.get_ctx(), cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "Can't read EVP_DecryptInit_ex");
            return false;
        }

        return true;
    }

    /**
     * \brief Decrypt \c src_buf into \c dst_buf.
     * \return decrypted output size
     */
    size_t decrypt(const uint8_t * src_buf, size_t src_sz, uint8_t * dst_buf)
    {
        assert(this->cctx.is_initialized());
        int written = 0;
        int trail = 0;
        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_DecryptInit_ex(this->cctx.get_ctx(), nullptr, nullptr, nullptr, nullptr) != 1
         || EVP_DecryptUpdate(this->cctx.get_ctx(), dst_buf, &written, src_buf, src_sz) != 1
         || EVP_DecryptFinal_ex(this->cctx.get_ctx(), dst_buf + written, &trail) != 1){
            LOG(LOG_ERR, "DecryptContext::decrypt");
            throw Error(ERR_SSL_CALL_FAILED);
        }
        return size_t(written + trail);
    }

private:
    CipherContext cctx;
};

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

#include "capture/md_hash.hpp"
#include "configs/autogen/enums.hpp"
#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/sugar/byte_ptr.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "system/ssl_sha256.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <vector>

#include <cerrno>
#include <cstdio>
#include <cstdio>
#include <cstring>

#include <unistd.h>


extern "C" {
    using get_hmac_key_prototype = int (uint8_t *);
    using get_trace_key_prototype = int (const uint8_t *, int, uint8_t *, unsigned int);
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

private:
    TraceType trace_type = TraceType::localfile;

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
    [[nodiscard]] array_view<const uint8_t> get_master_key() const
    {
        assert(this->master_key_loaded);
        return make_array_view(this->master_key);
    }

    void set_master_derivator(bytes_view derivator)
    {
        if ((this->master_key_loaded || !this->master_derivator.empty())
         && not (this->master_derivator.size() == derivator.size()
           && std::equal(derivator.begin(), derivator.end(), this->master_derivator.begin())
        )) {
            LOG(LOG_ERR, "CryptoContext: master derivator is already defined");
            throw Error(ERR_WRM_INVALID_INIT_CRYPT);
        }
        this->master_derivator.assign(derivator.begin(), derivator.end());
    }

    void set_trace_type(TraceType trace_type)
    {
        this->trace_type = trace_type;
    }

    [[nodiscard]] bool get_with_encryption() const
    {
        return this->trace_type == TraceType::cryptofile;
    }

    [[nodiscard]] bool get_with_checksum() const
    {
        return this->trace_type != TraceType::localfile;
    }

private:
    void load_trace_key(uint8_t (&buffer)[MD_HASH::DIGEST_LENGTH], bytes_view derivator)
    {
        if (int err = this->get_trace_key_cb(
            derivator.data()
          , static_cast<int>(derivator.size())
          , buffer
          , this->old_encryption_scheme ? 1 : 0
        )) {
            LOG(LOG_ERR, "CryptoContext: get_trace_key_cb: callback error: %d", err);
            throw Error(ERR_WRM_INVALID_INIT_CRYPT);
        }
    }

public:
    void get_derived_key(uint8_t (&trace_key)[CRYPTO_KEY_LENGTH], bytes_view derivator)
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
                sha256.update(derivator);
                sha256.final(tmp);
            }
            {
                MD_HASH sha256;
                sha256.update({tmp, DERIVATOR_LENGTH});
                sha256.update(make_array_view(this->master_key));
                sha256.final(tmp);
            }
            static_assert(sizeof(trace_key) == sizeof(tmp));
            memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
        }
    }

    CryptoContext() = default;

    class key_data : private bytes_view
    {
        static constexpr std::size_t key_length = CRYPTO_KEY_LENGTH;

        static_assert(sizeof(master_key) == key_length);
        static_assert(sizeof(hmac_key) == key_length);

        friend class CryptoContext;

    public:
        template<class T>
        key_data(T const & bytes32) noexcept
        : bytes_view(bytes32)
        {
            assert(this->size() == key_length);
        }

        template<class T, std::size_t array_length>
        key_data(std::array<T, array_length> const & data) noexcept
        : bytes_view(data.data(), data.size())
        {
            static_assert(array_length == key_length);
        }

        template<class T, std::size_t array_length>
        key_data(T const (& data)[array_length]) noexcept
        : bytes_view(data, array_length)
        {
            static_assert(array_length == key_length);
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
        this->ectx = ::EVP_CIPHER_CTX_new();
        this->is_init = true;
    }

    void deinit() noexcept
    {
        if (this->is_init) {
            ::EVP_CIPHER_CTX_free(this->ectx);
            this->is_init = false;
        }
    }

    ~CipherContext()
    {
        this->deinit();
    }

    EVP_CIPHER_CTX* get_ctx() noexcept
    {
        return this->ectx;
    }

    [[nodiscard]] bool is_initialized() const noexcept
    {
        return this->is_init;
    }

private:
    EVP_CIPHER_CTX * ectx = nullptr; // [en|de]cryption context
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
     * \brief Encrypt \c src into \c dst.
     * \return encrypted output size
     */
    size_t encrypt(bytes_view src, writable_bytes_view dst)
    {
        assert(this->cctx.is_initialized());
        int safe_size = dst.size();
        int remaining_size = 0;
        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(this->cctx.get_ctx(), nullptr, nullptr, nullptr, nullptr) != 1
         || EVP_EncryptUpdate(this->cctx.get_ctx(), dst.data(), &safe_size, src.data(), src.size()) != 1
         || EVP_EncryptFinal_ex(this->cctx.get_ctx(), dst.data() + safe_size, &remaining_size) != 1) {
            LOG(LOG_ERR, "EncryptContext::encrypt");
            throw Error(ERR_SSL_CALL_FAILED);
        }
        return size_t(safe_size + remaining_size); /*NOLINT*/
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
     * \brief Decrypt \c src into \c dst_buf.
     * \return decrypted output size
     */
    size_t decrypt(bytes_view src, uint8_t * dst_buf)
    {
        assert(this->cctx.is_initialized());
        int written = 0;
        int trail = 0;
        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_DecryptInit_ex(this->cctx.get_ctx(), nullptr, nullptr, nullptr, nullptr) != 1
         || EVP_DecryptUpdate(this->cctx.get_ctx(), dst_buf, &written, src.data(), src.size()) != 1
         || EVP_DecryptFinal_ex(this->cctx.get_ctx(), dst_buf + written, &trail) != 1){
            if (this->enable_log_decrypt) {
                LOG(LOG_ERR, "DecryptContext::decrypt");
            }
            throw Error(ERR_SSL_CALL_FAILED);
        }
        return size_t(written + trail); /*NOLINT*/
    }

    void disable_log_decrypt(bool disable = true) noexcept /*NOLINT*/
    {
        this->enable_log_decrypt = !disable;
    }

private:
    CipherContext cctx;
    bool enable_log_decrypt = true;
};

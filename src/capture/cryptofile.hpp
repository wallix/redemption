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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

/* for HMAC calculations */
#define MD_HASH_FUNC   SHA256
#define MD_HASH_NAME   "SHA256"
#define MD_HASH_LENGTH SHA256_DIGEST_LENGTH

#include <libgen.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <array>
#include <snappy-c.h>

#include "utils/log.hpp"
#include "utils/sugar/bytes_t.hpp"
#include "utils/genrandom.hpp"
#include "openssl_crypto.hpp"
#include "system/ssl_sha256.hpp"

enum crypto_file_state {
    CF_READY = 0, // Crypto File Reading
    CF_EOF = 1,   // Crypto File reached EOF
    CF_INIT = 2,  // Crypto File nor yet initialised
};

#define MIN(x, y)               (((x) > (y) ? (y) : (x)))
#define AES_BLOCK_SIZE          16
#define WABCRYPTOFILE_MAGIC     0x4D464357
#define WABCRYPTOFILE_EOF_MAGIC 0x5743464D
#define WABCRYPTOFILE_VERSION   0x00000001

enum {
    DERIVATOR_LENGTH = 8
};

/* size of salt to protect master key */
#define MKSALT_LEN 8


#define CRYPTO_BUFFER_SIZE ((4096 * 4))

extern "C" {
    typedef int get_hmac_key_prototype(char * buffer);
    typedef int get_trace_key_prototype(char * base, int len, char * buffer, unsigned oldscheme);
}



/* 256 bits key size */
#define CRYPTO_KEY_LENGTH 32
#define HMAC_KEY_LENGTH   CRYPTO_KEY_LENGTH


class CryptoContext
{
    unsigned char master_key[CRYPTO_KEY_LENGTH] {};
    unsigned char hmac_key[HMAC_KEY_LENGTH] {};

    get_hmac_key_prototype * get_hmac_key_cb = nullptr;
    get_trace_key_prototype * get_trace_key_cb = nullptr;

    bool master_key_loaded = false;
    bool hmac_key_loaded = false;


public:
    bool old_encryption_scheme = false;


public:
    auto get_hmac_key() -> unsigned char const (&)[HMAC_KEY_LENGTH]
    {
        if (!this->hmac_key_loaded){
            if (!this->get_hmac_key_cb) {
                LOG(LOG_ERR, "CryptoContext: get_hmac_key_cb is null");
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }
            // if we have a callback ask key
            this->get_hmac_key_cb(reinterpret_cast<char*>(this->hmac_key));
            this->hmac_key_loaded = true;
        }
        return this->hmac_key;
    }

    const unsigned char * get_master_key() const
    {
        assert(this->master_key_loaded);
        return this->master_key;
    }

    void get_derived_key(uint8_t (& trace_key)[CRYPTO_KEY_LENGTH], const uint8_t * derivator, size_t derivator_len)
    {
        if (this->old_encryption_scheme){
            //LOG(LOG_INFO, "old encryption scheme derivator %.*s", static_cast<unsigned>(derivator_len), derivator);
            if (this->get_trace_key_cb != nullptr){
                // if we have a callback ask key
                uint8_t tmp[SHA256_DIGEST_LENGTH];
                this->get_trace_key_cb(
                      reinterpret_cast<char*>(const_cast<uint8_t*>(derivator))
                    , static_cast<int>(derivator_len)
                    , reinterpret_cast<char*>(tmp)
                    , this->old_encryption_scheme?1:0
                    );
                memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
                return;
            }
        }

        //LOG(LOG_INFO, "new encryption scheme derivator %.*s", static_cast<unsigned>(derivator_len), derivator);
        if (!this->master_key_loaded){
            //LOG(LOG_INFO, "first call, loading master key");
            assert(this->get_trace_key_cb);
            if (!this->get_trace_key_cb) {
                LOG(LOG_ERR, "CryptoContext: get_hmac_key_cb is null");
                throw Error(ERR_WRM_INVALID_INIT_CRYPT);
            }

            // if we have a callback ask key
            this->get_trace_key_cb(
                reinterpret_cast<char*>(const_cast<uint8_t*>(derivator))
              , static_cast<int>(derivator_len)
              , reinterpret_cast<char*>(this->master_key)
              , this->old_encryption_scheme?1:0
            );
            this->master_key_loaded = true;
        }

        uint8_t tmp[SHA256_DIGEST_LENGTH];
        {
            SslSha256 sha256;
            sha256.update(derivator, derivator_len);
            sha256.final(tmp);
        }
        {
            SslSha256 sha256;
            sha256.update(tmp, DERIVATOR_LENGTH);
            sha256.update(this->master_key, CRYPTO_KEY_LENGTH);
            sha256.final(tmp);
        }
        memcpy(trace_key, tmp, HMAC_KEY_LENGTH);
    }

    CryptoContext() = default;

    size_t unbase64(char *buffer, size_t bufsiz, const char *txt)
    {
        const unsigned char _base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
            char const v = base64tbl[static_cast<unsigned char>(*txt)];
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

    class key_data : private const_bytes_array
    {
        static constexpr std::size_t key_length = 32;

        static_assert(sizeof(master_key) == key_length, "");
        static_assert(sizeof(hmac_key) == key_length, "");

        friend class CryptoContext;

    public:
        template<class T>
        key_data(T const & bytes32) noexcept
        : const_bytes_array(bytes32)
        {
            assert(this->size() == key_length);
        }

        template<class T, std::size_t array_length>
        key_data(std::array<T, array_length> const & data) noexcept
        : const_bytes_array(data.data(), data.size())
        {
            static_assert(array_length == key_length, "");
        }

        template<class T, std::size_t array_length>
        key_data(T const (& data)[array_length]) noexcept
        : const_bytes_array(data, array_length)
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



typedef unsigned char wrmcapture_hash_type[MD_HASH_LENGTH*2];

constexpr std::size_t wrmcapture_hash_string_len = (1 + MD_HASH_LENGTH * 2) * 2;

struct ocrypto {
    uint8_t result_buffer[32768] = {};
    struct Result {
        const_bytes_array buf;
        std::size_t consumed;
        int err_code; // no error = 0

        static Result error(int err_code)
        {
            return Result{{}, 0, err_code};
        }
    };


    char           buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    EVP_MD_CTX     hctx;                    // hash context
    EVP_MD_CTX     hctx4k;                  // hash context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       file_size;               // the current file size
    char header_buf[40];

    CryptoContext & cctx;
    Random & rnd;

    ocrypto(CryptoContext & cctx, Random & rnd)
        : cctx(cctx)
        , rnd(rnd)
    {
    }

    /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
     * Return 0 on success, negative value on error
     */
    int xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
    {
        int safe_size = *dst_sz;
        int remaining_size = 0;

        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
            return -1;
        }
        if (EVP_EncryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
            return -1;
        }
        if (EVP_EncryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish encryption!\n", getpid());
            return -1;
        }
        *dst_sz = safe_size + remaining_size;
        return 0;
    }

    /* Update hash context with new data.
     * Returns 0 on success, -1 on error
     */
    int hash_update(const void * src_buf, uint32_t src_sz)
    {
        if (::EVP_DigestUpdate(&this->hctx, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
            return -1;
        }
        if (this->file_size < 4096) {
            size_t remaining_size = 4096 - this->file_size;
            size_t hashable_size = MIN(remaining_size, src_sz);
            if (::EVP_DigestUpdate(&this->hctx4k, src_buf, hashable_size) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                return -1;
            }
        }
        return 0;
    }

    int hash_finalize()
    {
        return -1;
    }


    int init_hmac(const EVP_MD *md, EVP_MD_CTX * hctx, uint8_t * key_buf, size_t blocksize, uint8_t * hash)
    {
        unsigned char tmp[MD_HASH_LENGTH];
        if (::EVP_DigestFinal_ex(hctx, tmp, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k message digest\n", ::getpid());
            tmp[0] = '\0';
            return -1;
        }

        EVP_MD_CTX mdctx;
        ::EVP_MD_CTX_init(&mdctx);
        if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestUpdate(&mdctx, tmp, MD_HASH_LENGTH) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestFinal_ex(&mdctx, hash, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
            hash[0] = '\0';
            return -1;
        }
        ::EVP_MD_CTX_cleanup(&mdctx);
        return 0;
    }

public:
    Result open(const uint8_t * derivator, size_t derivator_len)
    {
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        this->cctx.get_derived_key(trace_key, derivator, derivator_len);
        unsigned char iv[32];
        this->rnd.random(iv, 32);

        ::memset(this->buf, 0, sizeof(this->buf));
        ::memset(&this->ectx, 0, sizeof(this->ectx));
        ::memset(&this->hctx, 0, sizeof(this->hctx));
        ::memset(&this->hctx4k, 0, sizeof(this->hctx4k));
        this->pos = 0;
        this->raw_size = 0;
        this->file_size = 0;

        const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
        const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
        const int          nrounds = 5;
        unsigned char      key[32];
        const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                       trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
        if (i != 32) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
            Result::error(-1);
        }

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if (::EVP_EncryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
            Result::error(-1);
        }

        // MD stuff
        const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
        if (!md) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
            Result::error(-1);
        }

        ::EVP_MD_CTX_init(&this->hctx);
        ::EVP_MD_CTX_init(&this->hctx4k);
        if (::EVP_DigestInit_ex(&this->hctx, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
            Result::error(-1);
        }
        if (::EVP_DigestInit_ex(&this->hctx4k, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize 4k MD hash context!\n", ::getpid());
            Result::error(-1);
        }

        // HMAC: key^ipad
        const int     blocksize = ::EVP_MD_block_size(md);
        unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
        {
            if (key_buf == nullptr) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc!\n", ::getpid());
                Result::error(-1);
            }
            const std::unique_ptr<unsigned char[]> auto_free(key_buf);
            ::memset(key_buf, 0, blocksize);
            if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                unsigned char keyhash[MD_HASH_LENGTH];
                if ( ! ::MD_HASH_FUNC(cctx.get_hmac_key(), CRYPTO_KEY_LENGTH, keyhash)) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                    Result::error(-1);
                }
                ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
            }
            else {
                ::memcpy(key_buf, cctx.get_hmac_key(), CRYPTO_KEY_LENGTH);
            }
            for (int idx = 0; idx <  blocksize; idx++) {
                key_buf[idx] = key_buf[idx] ^ 0x36;
            }
            if (::EVP_DigestUpdate(&this->hctx, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                Result::error(-1);
            }
            if (::EVP_DigestUpdate(&this->hctx4k, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                Result::error(-1);
            }
        }

        // update context with previously written data
        this->header_buf[0] = WABCRYPTOFILE_MAGIC & 0xFF;
        this->header_buf[1] = (WABCRYPTOFILE_MAGIC >> 8) & 0xFF;
        this->header_buf[2] = (WABCRYPTOFILE_MAGIC >> 16) & 0xFF;
        this->header_buf[3] = (WABCRYPTOFILE_MAGIC >> 24) & 0xFF;
        this->header_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
        this->header_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
        this->header_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
        this->header_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
        ::memcpy(this->header_buf + 8, iv, 32);

        // update file_size
        this->file_size += 40;
        int err_code = this->hash_update(this->header_buf, 40);
        return {{this->header_buf, 40u}, 0u, err_code};
    }

    /* Flush procedure (compression, encryption, effective file writing)
     * Return 0 on success, negatif on error
     */
    int flush(uint8_t * buffer, size_t buflen, size_t & towrite)
    {
        // No data to flush
        if (!this->pos) {
            return 0;
        }

        // Compress
        // TODO: check this
        char compressed_buf[65536];
        //char compressed_buf[compressed_buf_sz];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->pos);
        snappy_status status = snappy_compress(this->buf, this->pos, compressed_buf, &compressed_buf_sz);

        switch (status)
        {
            case SNAPPY_OK:
                break;
            case SNAPPY_INVALID_INPUT:
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code INVALID_INPUT!\n", getpid());
                return -1;
            case SNAPPY_BUFFER_TOO_SMALL:
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                return -1;
        }

        // Encrypt
        unsigned char ciphered_buf[4 + 65536];
        //char ciphered_buf[ciphered_buf_sz];
        uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
        {
            const unsigned char * src_buf = reinterpret_cast<unsigned char*>(compressed_buf);
            if (this->xaes_encrypt(src_buf, compressed_buf_sz, ciphered_buf + 4, &ciphered_buf_sz)) {
                return -1;
            }
        }

        ciphered_buf[0] = ciphered_buf_sz & 0xFF;
        ciphered_buf[1] = (ciphered_buf_sz >> 8) & 0xFF;
        ciphered_buf[2] = (ciphered_buf_sz >> 16) & 0xFF;
        ciphered_buf[3] = (ciphered_buf_sz >> 24) & 0xFF;

        ciphered_buf_sz += 4;
        if (ciphered_buf_sz > buflen){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Encryption buffer too small\n", ::getpid());
            return -1;
        }
        ::memcpy(buffer, ciphered_buf, ciphered_buf_sz);
        towrite += ciphered_buf_sz;

        if (-1 == this->hash_update(&ciphered_buf, ciphered_buf_sz)) {
            return -1;
        }
        this->file_size += ciphered_buf_sz;

        // Reset buffer
        this->pos = 0;
        return 0;
    }

    ocrypto::Result close(unsigned char qhash[MD_HASH_LENGTH], unsigned char fhash[MD_HASH_LENGTH])
    {
        size_t buflen = sizeof(this->result_buffer);
        size_t towrite = 0;
        const uint8_t * hmac_key = this->cctx.get_hmac_key();
        int err = this->flush(this->result_buffer, buflen, towrite);
        if (err) {
            return Result::error(-1);
        }

        const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
        unsigned char tmp_buf[8] = {
            eof_magic & 0xFF,
            (eof_magic >> 8) & 0xFF,
            (eof_magic >> 16) & 0xFF,
            (eof_magic >> 24) & 0xFF,
            uint8_t(this->raw_size & 0xFF),
            uint8_t((this->raw_size >> 8) & 0xFF),
            uint8_t((this->raw_size >> 16) & 0xFF),
            uint8_t((this->raw_size >> 24) & 0xFF),
        };

        if (towrite + 8 > buflen){
           return Result::error(-1);
        }
        ::memcpy(this->result_buffer + towrite, tmp_buf, 8);
        towrite += 8;

        this->file_size += 8;

        this->hash_update(tmp_buf, 8);

        if (qhash && fhash) {
            // HMAC: MD(key^opad + MD(key^ipad))
            const EVP_MD *md = ::EVP_get_digestbyname(MD_HASH_NAME);
            if (!md) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest\n", ::getpid());
                return Result::error(-1);
            }
            const int blocksize = ::EVP_MD_block_size(md);
            unsigned char key_buf[EVP_MAX_MD_SIZE] = {};
            if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                unsigned char keyhash[MD_HASH_LENGTH];
                if ( ! ::MD_HASH_FUNC(hmac_key, CRYPTO_KEY_LENGTH, keyhash)) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key\n", ::getpid());
                    return Result::error(-1);
                }
                ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
            }
            else {
                ::memcpy(key_buf, hmac_key, CRYPTO_KEY_LENGTH);
            }
            for (int idx = 0; idx < blocksize; idx++) {
                key_buf[idx] = key_buf[idx] ^ 0x5c;
            }

//            init_hmac(md, &this->hctx4k, key_buf, blocksize, qhash);
            unsigned char tmp_qhash[MD_HASH_LENGTH];
            if (::EVP_DigestFinal_ex(&this->hctx4k, tmp_qhash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k message digest\n", ::getpid());
                tmp_qhash[0] = '\0';
                return Result::error(-1);
            }

            EVP_MD_CTX mdctx;
            ::EVP_MD_CTX_init(&mdctx);
            if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestUpdate(&mdctx, tmp_qhash, MD_HASH_LENGTH) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestFinal_ex(&mdctx, qhash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                qhash[0] = '\0';
                return Result::error(-1);
            }
            ::EVP_MD_CTX_cleanup(&mdctx);


           unsigned char tmp_fhash[MD_HASH_LENGTH];
            if (::EVP_DigestFinal_ex(&this->hctx, tmp_fhash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute message digest\n", ::getpid());
                tmp_fhash[0] = '\0';
                return Result::error(-1);
            }
 
            ::EVP_MD_CTX_init(&mdctx);
            if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestUpdate(&mdctx, tmp_fhash, MD_HASH_LENGTH) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                return Result::error(-1);
            }
            if (::EVP_DigestFinal_ex(&mdctx, fhash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                fhash[0] = '\0';
                return Result::error(-1);
            }
            ::EVP_MD_CTX_cleanup(&mdctx);
        }
        return Result{{this->result_buffer, towrite}, 0u, 0};
    }

    ocrypto::Result write(const void * data, size_t len)
    {
        size_t buflen = sizeof(this->result_buffer);
        size_t towrite = 0;
        unsigned int remaining_size = len;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->pos, remaining_size);
            // Append and update pos pointer
            ::memcpy(this->buf + this->pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
            this->pos += available_size;
            // If buffer is full, flush it to disk
            if (this->pos == CRYPTO_BUFFER_SIZE) {
                size_t tmp_towrite = 0;
                int err = this->flush(this->result_buffer + towrite, buflen - towrite, tmp_towrite);
                towrite += tmp_towrite;
                if (err) {
                    return Result::error(-1);
                }
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->raw_size += len;
        return {{this->result_buffer, towrite}, len, 0};
    }


};




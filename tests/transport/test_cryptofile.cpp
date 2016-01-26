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
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>

#include <new>

#include <snappy-c.h>
#include "fdbuf.hpp"

#include "transport/cryptofile.hpp"

namespace transfil {

    class decrypt_filter4
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       state;                   // enum crypto_file_state
        unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

    public:
        decrypt_filter4() = default;
        //: pos(0)
        //, raw_size(0)
        //, state(0)
        //, MAX_CIPHERED_SIZE(0)
        //{}

        template<class Source>
        int open(Source & src, unsigned char * trace_key)
        {
            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));

            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char tmp_buf[40];

            if (const ssize_t err = this->raw_read(src, tmp_buf, 40)) {
                return err;
            }

            // Check magic
            const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
            if (magic != WABCRYPTOFILE_MAGIC) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n",
                    ::getpid(), magic, WABCRYPTOFILE_MAGIC);
                return -1;
            }
            const int version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
            if (version > WABCRYPTOFILE_VERSION) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                    ::getpid(), version, WABCRYPTOFILE_VERSION);
                return -1;
            }

            unsigned char * const iv = tmp_buf + 8;

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
            const int          nrounds = 5;
            unsigned char      key[32];
            const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
            if (i != 32) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                return -1;
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                return -1;
            }

            return 0;
        }

        template<class Source>
        ssize_t read(Source & src, void * data, size_t len)
        {
            if (this->state & CF_EOF) {
                //printf("cf EOF\n");
                return 0;
            }

            unsigned int requested_size = len;

            while (requested_size > 0) {
                // Check how much we have decoded
                if (!this->raw_size) {
                    // Buffer is empty. Read a chunk from file
                    /*
                     i f (-1 == ::do_chunk_read*(this)) {
                         return -1;
                }
                */
                    // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                    // and it's not portable because of endianness issue => read in a buffer and decode by hand
                    unsigned char tmp_buf[4] = {};
                    if (const int err = this->raw_read(src, tmp_buf, 4)) {
                        return err;
                    }

                    uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state |= CF_EOF;
                        this->pos = 0;
                        this->raw_size = 0;
                    }
                    else {
                        if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                            return -1;
                        }
                        else {
                            uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                            //char ciphered_buf[ciphered_buf_size];
                            unsigned char ciphered_buf[65536];
                            //char compressed_buf[compressed_buf_size];
                            unsigned char compressed_buf[65536];

                            if (const ssize_t err = this->raw_read(src, ciphered_buf, ciphered_buf_size)) {
                                return err;
                            }

                            if (this->xaes_decrypt(ciphered_buf, ciphered_buf_size, compressed_buf, &compressed_buf_size)) {
                                return -1;
                            }

                            size_t chunk_size = CRYPTO_BUFFER_SIZE;
                            const snappy_status status = snappy_uncompress(reinterpret_cast<char *>(compressed_buf),
                                                                           compressed_buf_size, this->buf, &chunk_size);

                            switch (status)
                            {
                                case SNAPPY_OK:
                                    break;
                                case SNAPPY_INVALID_INPUT:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                                    return -1;
                                case SNAPPY_BUFFER_TOO_SMALL:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                                    return -1;
                                default:
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                                    return -1;
                            }

                            this->pos = 0;
                            // When reading, raw_size represent the current chunk size
                            this->raw_size = chunk_size;
                        }
                    }

                    // TODO: check that
                    if (!this->raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->raw_size - this->pos;
                // Check how much we can copy
                unsigned int copiable_size = MIN(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(static_cast<char*>(data) + (len - requested_size), this->buf + this->pos, copiable_size);
                this->pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->raw_size == this->pos) {
                    this->raw_size = 0;
                }
            }
            return len - requested_size;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        template<class Source>
        ssize_t raw_read(Source & src, void * data, size_t len)
        {
            ssize_t err = src.read(data, len);
            return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
        }

        int xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                return -1;
            }
            if (EVP_DecryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                return -1;
            }
            if (EVP_DecryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }
    };

}

namespace transfil {

    class encrypt_filter4
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        EVP_MD_CTX     hctx;                    // hash context
        EVP_MD_CTX     hctx4k;                  // hash context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       file_size;               // the current file size

    public:
        encrypt_filter4() = default;
        //: pos(0)
        //, raw_size(0)
        //, file_size(0)
        //{}

        template<class Sink>
        int open(Sink & snk, const unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
        {
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
                return -1;
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if (::EVP_EncryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
                return -1;
            }

            // MD stuff
            const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
            if (!md) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
                return -1;
            }

            ::EVP_MD_CTX_init(&this->hctx);
            ::EVP_MD_CTX_init(&this->hctx4k);
            if (::EVP_DigestInit_ex(&this->hctx, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestInit_ex(&this->hctx4k, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize 4k MD hash context!\n", ::getpid());
                return -1;
            }

            // HMAC: key^ipad
            const int     blocksize = ::EVP_MD_block_size(md);
            unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
            {
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc!\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, 0, blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(static_cast<unsigned char *>(cctx->get_hmac_key()), CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, cctx->get_hmac_key(), CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x36;
                }
                if (::EVP_DigestUpdate(&this->hctx, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&this->hctx4k, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                    return -1;
                }
            }

            // update context with previously written data
            char tmp_buf[40];
            tmp_buf[0] = WABCRYPTOFILE_MAGIC & 0xFF;
            tmp_buf[1] = (WABCRYPTOFILE_MAGIC >> 8) & 0xFF;
            tmp_buf[2] = (WABCRYPTOFILE_MAGIC >> 16) & 0xFF;
            tmp_buf[3] = (WABCRYPTOFILE_MAGIC >> 24) & 0xFF;
            tmp_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
            tmp_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
            tmp_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
            tmp_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
            ::memcpy(tmp_buf + 8, iv, 32);

            // TODO: if I suceeded writing a broken file, wouldn't it be better to remove it ?
            if (const ssize_t write_ret = this->raw_write(snk, tmp_buf, 40)){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
                return write_ret;
            }
            // update file_size
            this->file_size += 40;

            return this->xmd_update(tmp_buf, 40);
        }

        template<class Sink>
        ssize_t write(Sink & snk, const void * data, size_t len)
        {
            unsigned int remaining_size = len;
            while (remaining_size > 0) {
                // Check how much we can append into buffer
                unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->pos, remaining_size);
                // Append and update pos pointer
                ::memcpy(this->buf + this->pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
                this->pos += available_size;
                // If buffer is full, flush it to disk
                if (this->pos == CRYPTO_BUFFER_SIZE) {
                    if (this->flush(snk)) {
                        return -1;
                    }
                }
                remaining_size -= available_size;
            }
            // Update raw size counter
            this->raw_size += len;
            return len;
        }

        /* Flush procedure (compression, encryption, effective file writing)
         * Return 0 on success, negatif on error
         */
        template<class Sink>
        int flush(Sink & snk)
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
                default:
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with unknown status code (%d)!\n", getpid(), status);
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

            if (const ssize_t err = this->raw_write(snk, ciphered_buf, ciphered_buf_sz)) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
                return err;
            }
            if (-1 == this->xmd_update(&ciphered_buf, ciphered_buf_sz)) {
                return -1;
            }
            this->file_size += ciphered_buf_sz;

            // Reset buffer
            this->pos = 0;
            return 0;
        }

        template<class Sink>
        int close(Sink & snk, unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
        {
            int result = this->flush(snk);

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

            int write_ret1 = this->raw_write(snk, tmp_buf, 8);
            if (write_ret1){
                // TOOD: actual error code could help
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            }
            this->file_size += 8;

            this->xmd_update(tmp_buf, 8);

            if (hash) {
                unsigned char tmp_hash[MD_HASH_LENGTH << 1];
                if (::EVP_DigestFinal_ex(&this->hctx4k, tmp_hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[0] = '\0';
                }
                if (::EVP_DigestFinal_ex(&this->hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[MD_HASH_LENGTH] = '\0';
                }
                // HMAC: MD(key^opad + MD(key^ipad))
                const EVP_MD *md = ::EVP_get_digestbyname(MD_HASH_NAME);
                if (!md) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find MD message digest\n", ::getpid());
                    return -1;
                }
                const int     blocksize = ::EVP_MD_block_size(md);
                unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, '\0', blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(static_cast<const unsigned char *>(hmac_key), CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, hmac_key, CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x5c;
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
                if (::EVP_DigestUpdate(&mdctx, tmp_hash, MD_HASH_LENGTH) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[0] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash + MD_HASH_LENGTH, MD_HASH_LENGTH) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[MD_HASH_LENGTH] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
            }

            return result;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        template<class Sink>
        ssize_t raw_write(Sink & snk, void * data, size_t len)
        {
            ssize_t err = snk.write(data, len);
            return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
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
        int xmd_update(const void * src_buf, uint32_t src_sz)
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
    };
}



extern "C" {


/**********************************************
 *                Public API                  *
 **********************************************/

enum Crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

struct Crypto_file
{
    Crypto_type type;
    io::posix::fdbuf file;

    Crypto_file(int fd, Crypto_type t)
        : type(t)
        , file(fd)
        {}

    bool is_encrypt() const
    { return CRYPTO_ENCRYPT_TYPE == type; }

    bool is_decrypt() const
    { return CRYPTO_DECRYPT_TYPE == type; }
};

struct Crypto_file_decrypt : public Crypto_file
{
  transfil::decrypt_filter4 decrypt;
  Crypto_file_decrypt(int fd)
    : Crypto_file(fd, CRYPTO_DECRYPT_TYPE)
    {}
};

struct Crypto_file_encrypt : public Crypto_file
{
  transfil::encrypt_filter4 encrypt;
  Crypto_file_encrypt(int fd)
    : Crypto_file(fd, CRYPTO_ENCRYPT_TYPE)
    {}
};

Crypto_file * crypto_open_read(int systemfd, unsigned char * trace_key,  CryptoContext * cctx)
{
    (void)cctx;
    Crypto_file_decrypt * cf_struct = new (std::nothrow) Crypto_file_decrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->decrypt.open(cf_struct->file, trace_key)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<Crypto_file*>(cf_struct);
}

Crypto_file_encrypt * crypto_open_write(int systemfd, unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
{
    Crypto_file_encrypt * cf_struct = new (std::nothrow) Crypto_file_encrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, cctx, iv)) {
        delete cf_struct;
        return nullptr;
    }

    return cf_struct;
}

/* Flush procedure (compression, encryption, effective file writing)
 * Return 0 on success, -1 on error
 */
int crypto_flush(Crypto_file * cf)
{
    if (cf->is_decrypt()) {
        return -1;
    }
    Crypto_file_encrypt * cf_struct = reinterpret_cast<Crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.flush(cf_struct->file);
}

/* The actual read method. Read chunks until we reach requested size.
 * Return the actual size read into buf, -1 on error
 */
int crypto_read(Crypto_file * cf, char * buf, unsigned int buf_size)
{
    if (cf->is_decrypt()) {
        Crypto_file_decrypt * cf_struct = reinterpret_cast<Crypto_file_decrypt*>(cf);
        return cf_struct->decrypt.read(cf_struct->file, buf, buf_size);
    }
    return -1;
}

/* Actually appends data to Crypto_file buffer, flush if buffer gets full
 * Return the written size, -1 on error
 */
int crypto_write(Crypto_file *cf, const char * buf, unsigned int size)
{
    if (cf->is_decrypt()) {
        return -1;
    }
    Crypto_file_encrypt * cf_struct = reinterpret_cast<Crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.write(cf_struct->file, buf, size);
}

int crypto_close(Crypto_file *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
{
    int nResult = 0;
    if (cf->is_decrypt()) {
        Crypto_file_decrypt * cf_struct = reinterpret_cast<Crypto_file_decrypt*>(cf);
        delete cf_struct;
    }
    else {
        Crypto_file_encrypt * cf_struct = reinterpret_cast<Crypto_file_encrypt*>(cf);
        nResult = cf_struct->encrypt.close(cf_struct->file, hash, hmac_key);
        delete cf_struct;
    }
    return nResult;
}

} // extern "C"


BOOST_AUTO_TEST_CASE(TestDecrypt)
{
    OpenSSL_add_all_digests();

    LCGRandom rnd(0);

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));

    CryptoContext cctx(rnd, ini, 2);

    const char * file = "tests/fixtures/encrypted_video/"
        "x@10.10.43.13,qaadministrateur@win78,20131211-085926,wab2-4-0-0.yourdomain,5423.rdptrc";

    unsigned char derivator[DERIVATOR_LENGTH];
    cctx.get_derivator(file, derivator, DERIVATOR_LENGTH);

    BOOST_CHECK(0 == memcmp("\xdc\x07\x64\x92\xda\x52\xfe\xa9", derivator, DERIVATOR_LENGTH));

    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    int res = cctx.compute_hmac(trace_key, derivator);
    BOOST_CHECK(0 == res);

/*    LOG(LOG_INFO, "Dumping trace_key");*/

    BOOST_CHECK(true);
/*    hexdump_c(trace_key, CRYPTO_KEY_LENGTH);*/

    BOOST_CHECK(true);

    int system_fd = open(file, O_RDONLY, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", file);
        BOOST_CHECK(false);
    }

    BOOST_CHECK(true);

    Crypto_file * cf_struct = crypto_open_read(system_fd, trace_key, &cctx);
    BOOST_CHECK(cf_struct);

    char buf[1024];
    int sz = crypto_read(cf_struct, buf, 1024);
    BOOST_CHECK_EQUAL(118, sz);

    uint32_t i1 = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);
    uint32_t i2 = buf[4] + (buf[5]<<8) + (buf[6]<<16) + (buf[7]<<24);
    uint32_t i3 = buf[8] + (buf[9]<<8) + (buf[10]<<16) + (buf[11]<<24);

    BOOST_CHECK_EQUAL(static_cast<uint32_t>(1369971550), i1); // EPOCH: python int(time().time)
    BOOST_CHECK_EQUAL(static_cast<uint32_t>(397820), i2); // Micro-seconds: (time.time() - EPOCH) * 1000000
    BOOST_CHECK_EQUAL(static_cast<uint32_t>(106), i3);
    BOOST_CHECK(0 == memcmp(
        buf+12,
        "/var/wab/recorded/rdp/x@10.10.43.13,qaadministrateur@win78,20131211-085925,wab2-4-0-0.yourdomain,1188.mwrm",
        106));

    delete reinterpret_cast<Crypto_file_decrypt*>(cf_struct);
}

BOOST_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey)
{
    unsigned char expected_hmac_key[]
             = { 0x86, 0x41, 0x05, 0x58, 0xc4, 0x95, 0xcc, 0x4e,
                 0x49, 0x21, 0x57, 0x87, 0x47, 0x74, 0x08, 0x8a,
                 0x33, 0xb0, 0x2a, 0xb8, 0x65, 0xcc, 0x38, 0x41,
                 0x20, 0xfe, 0xc2, 0xc9, 0xb8, 0x72, 0xc8, 0x2c
               };

    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));
    LCGRandom rnd(0);
    CryptoContext cctx(rnd, ini, 2);
    cctx.get_crypto_key();
//    hexdump_c(cctx.get_hmac_key(), HMAC_KEY_LENGTH);
    BOOST_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), 32));
}


BOOST_AUTO_TEST_CASE(TestDerivationOfHmacKeyFromCryptoKey2)
{
    unsigned char expected_hmac_key[] = {
         0x86U, 0x41U, 0x05U, 0x58U, 0xc4U, 0x95U, 0xccU, 0x4eU,
         0x49U, 0x21U, 0x57U, 0x87U, 0x47U, 0x74U, 0x08U, 0x8aU,
         0x33U, 0xb0U, 0x2aU, 0xb8U, 0x65U, 0xccU, 0x38U, 0x41U,
         0x20U, 0xfeU, 0xc2U, 0xc9U, 0xb8U, 0x72U, 0xc8U, 0x2cU
         };

    LCGRandom rnd(0);
    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));
    CryptoContext cctx(rnd, ini, 2);
    cctx.get_crypto_key();
    BOOST_CHECK(0 == memcmp(expected_hmac_key, cctx.get_hmac_key(), HMAC_KEY_LENGTH));
}

BOOST_AUTO_TEST_CASE(TestCryptAndReadBack)
{
    const char * file = "/tmp/testcryptofile.tmp";
    unlink(file);

    LCGRandom rnd(0);
    Inifile ini;
    ini.set<cfg::crypto::key0>(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"));

    CryptoContext cctx(rnd, ini, 2);
    cctx.get_crypto_key();

    OpenSSL_add_all_digests();

    LCGRandom gen(0);

    unsigned char iv[32] = {};
//    dev_urandom_read(iv, 32);
    gen.random(iv, 32); // fixed random sequence to repeat test

    unsigned char derivator[DERIVATOR_LENGTH];
    cctx.get_derivator(file, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    BOOST_CHECK(0 == cctx.compute_hmac(trace_key, derivator));

    int system_fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (system_fd == -1){
        printf("failed opening=%s\n", file);
        BOOST_CHECK(false);
    }

    Crypto_file * cf_struct = crypto_open_write(system_fd, trace_key, &cctx, iv);
    if (!cf_struct){
        close(system_fd);
        unlink(file);
    }

    BOOST_CHECK(cf_struct != nullptr);

    char buf[4*8192] = {};
    gen.random(buf, sizeof(buf));
    int res1 = crypto_write(cf_struct, buf, sizeof(buf));
    BOOST_CHECK_EQUAL(res1, sizeof(buf)); // amount of data written

    gen.random(buf, sizeof(buf));
    int res2 = crypto_write(cf_struct, buf, sizeof(buf));
    BOOST_CHECK_EQUAL(res2, sizeof(buf)); // amount of data written

    {
        uint8_t hmac[32*2] = {};
        int res3 = crypto_close(cf_struct, hmac, cctx.get_hmac_key());
        BOOST_CHECK_EQUAL(res3, 0);
        unlink(file);

        uint8_t expected_hmac[] = {
            0xa1, 0x8d, 0x12, 0xaa, 0x29, 0x80, 0xbb, 0x48,
            0xa2, 0xd4, 0x99, 0x9c, 0x69, 0x4b, 0x86, 0x0e,
            0x66, 0x91, 0xea, 0x81, 0xc8, 0xd2, 0xa2, 0x67,
            0xf4, 0x18, 0xaf, 0xa5, 0x86, 0x20, 0xdb, 0x8c,
            0xb9, 0x21, 0x56, 0xd2, 0xe8, 0x28, 0x32, 0xd9,
            0x6b, 0x60, 0xad, 0x8f, 0xb5, 0xc4, 0x85, 0x20,
            0xed, 0x55, 0xbb, 0x2f, 0x5d, 0x2c, 0x4a, 0x4b,
            0x09, 0x3f, 0x51, 0xc3, 0xdf, 0x62, 0xf9, 0x6d,
        };
        BOOST_CHECK(0 == memcmp(hmac, expected_hmac, sizeof(expected_hmac)));

//       printf("hmac=\n");
//       {
//           int i = 0;
//           for (i = 0; i < sizeof(hmac) ; i++){
//                printf("0x%.2x, ", hmac[i] & 0xFF);
//                if (((i+1) % 8) == 0){
//                    printf("\n");
//                }
//           }
//       }
    }
}

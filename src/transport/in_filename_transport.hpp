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

#ifndef REDEMPTION_TRANSPORT_IN_FILENAME_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_IN_FILENAME_TRANSPORT_HPP

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>

#include "log.hpp"
#include "openssl_crypto.hpp"
#include "transport/transport.hpp"
#include "transport/cryptofile.hpp"
#include "urandom_read.hpp"

struct InFilenameTransport : public Transport
{
    int fd;
    int encryption;
    
    struct raw_t {
        char b[32768];
        int start;
        int end;

        raw_t() : start(0), end(0) {}

        uint32_t get_uint32_le(size_t offset)
        {
            return  this->b[0+offset]
                  +(this->b[1+offset]<< 8)
                  +(this->b[2+offset]<< 16)
                  +(this->b[3+offset]<< 24);
        }

        void read_min(int fd, size_t to_read, size_t min_to_read)
        {
            while ((size_t)(this->end - this->start) < min_to_read) {
                ssize_t ret = ::read(fd, &this->b[this->end], to_read - (this->end-this->start));
                if (ret <= 0){
                    if (ret < 0 && errno == EINTR){
                        continue;
                    }
                    LOG(LOG_ERR, "failed reading from file");
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
                this->end += ret;
            }
        }
    } raw;

    struct {
        uint8_t b[65536];
        int start;
        int end;
    } decrypted;

    struct {
        char b[32768];
        int start;
        int end;
    } decompressed;

    char           buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       state;                   // enum crypto_file_state


public:
    // TODO: rename that class InRedfileTransport as it is a special type of file used by redemption
    // that can be either stored as clear text or compressed/encrypted text following a special
    // purpose format defined by redemption. These files are also non-seekable to make
    // it possible to send them over wires or in non block devices..
    InFilenameTransport(CryptoContext * cctx, int fd, const uint8_t * base, size_t base_len)
        : fd(fd)
        , pos(0)
        , raw_size(0)
        , state(0)
    {
    
//        size_t base_len = 0;
//        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));
    
//        this->fd = ::open(filename, O_RDONLY);
//        if (this->fd < 0) {
//            LOG(LOG_ERR, "failed opening=%s\n", filename);
//            throw Error(ERR_TRANSPORT_OPEN_FAILED);
//        }

        this->raw.read_min(this->fd, 40, 4);
        const uint32_t magic = this->raw.get_uint32_le(0);
        this->encryption = (magic == WABCRYPTOFILE_MAGIC)?1:0;

        if (this->encryption){
            this->raw.read_min(this->fd, 40, 40);
            const int version = this->raw.get_uint32_le(4);
            if (version > WABCRYPTOFILE_VERSION) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                    ::getpid(), version, WABCRYPTOFILE_VERSION);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }


            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            unsigned char * const iv = reinterpret_cast<uint8_t *>(&this->raw.b[8]);
            this->raw.start = this->raw.end = 0;

            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            uint8_t tmp[SHA256_DIGEST_LENGTH];
            {
                SslSha256 sha256;
                sha256.update(base, base_len);
                sha256.final(tmp, SHA256_DIGEST_LENGTH);
            }
            {
                SslSha256 sha256;
                sha256.update(tmp, DERIVATOR_LENGTH);
                sha256.update(cctx->get_crypto_key(), CRYPTO_KEY_LENGTH);
                sha256.final(tmp, SHA256_DIGEST_LENGTH);
            }
            memcpy(trace_key, tmp, HMAC_KEY_LENGTH);

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const uint8_t salt[]  = { 0x39, 0x30, 0, 0, 0x31, 0xd4, 0, 0 };
            const int          nrounds = 5;
            unsigned char      key[32];
            
            // Key Derivation Algorithm
            // ------------------------

            // The key and IV is derived by concatenating D_1, D_2, etc
            // until enough data is available for the key and IV . D_i
            // is defined as:

            // D_i = HASH^count(D_(i-1) || data || salt)
            // where || denotes concatentaion, D_0 is empty, 
            // HASH is the digest algorithm in use,
            // HASH^1(data) is simply HASH (data),
            // HASH^2(data) is HASH ( HASH (data)) and so on.
            
            const int i = ::EVP_BytesToKey(cipher
                                           , ::EVP_sha1()
                                           , &salt[0]
                                           , trace_key
                                           , CRYPTO_KEY_LENGTH
                                           , nrounds
                                           , key
                                           , nullptr);
            if (i != 32) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            ::memset(&this->ectx, 0, sizeof(this->ectx));
            ::EVP_CIPHER_CTX_init(&this->ectx);
            if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }
    }

    ~InFilenameTransport()
    {
        if (-1 != this->fd) {
            ::close(this->fd);
        }
    }

    bool is_encrypted()
    {
        return this->encryption == 1;
    }

    bool disconnect() override {
        if (-1 != this->fd) {
            const int ret = ::close(this->fd);
            this->fd = -1;
            return !ret;
        }
        return !0;
    }
private:
    void do_recv(char ** pbuffer, size_t requested_size) override {
        if (this->encryption) {
            if (this->state & CF_EOF) {
                this->status = false;
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
            }

            unsigned int remaining_requested_size = requested_size;
            while (remaining_requested_size > 0) {
                // Check how much we have decoded
                if (!this->raw_size) {
                    this->raw.read_min(this->fd, 4, 4);
                    uint32_t ciphered_buf_size = this->raw.get_uint32_le(0);
                    this->raw.end = 0;

                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state |= CF_EOF;
                        this->pos = 0;
                        this->raw_size = 0;
                        break;
                    }
                    if (ciphered_buf_size > ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE) + AES_BLOCK_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        this->status = false;
                        throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                    }
                    else {
                        uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                        //char ciphered_buf[ciphered_buf_size];
                        //char compressed_buf[compressed_buf_size];
//                        unsigned char compressed_buf[65536];

                        TODO("this is blocking read, add support for timeout reading");
                        TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
                        this->raw.read_min(this->fd, ciphered_buf_size, ciphered_buf_size);

                        int safe_size = compressed_buf_size;
                        int ciph_remaining_size = 0;

                        /* allows reusing of ectx for multiple encryption cycles */
                        if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                            this->status = false;
                            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                        }
                        if (EVP_DecryptUpdate(&this->ectx,
                                              &this->decrypted.b[0],
                                              &safe_size,
                                              reinterpret_cast<uint8_t*>(&this->raw.b[0]),
                                              ciphered_buf_size) != 1){
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                            this->status = false;
                            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                        }

                        if (EVP_DecryptFinal_ex(&this->ectx,
                                                &this->decrypted.b[safe_size],
                                                &ciph_remaining_size) != 1){

                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                            this->status = false;
                            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                        }
                        compressed_buf_size = safe_size + ciph_remaining_size;
                        this->raw.end = 0;

                        size_t chunk_size = CRYPTO_BUFFER_SIZE;
                        const snappy_status status = snappy_uncompress(
                            reinterpret_cast<char *>(&this->decrypted.b[0]),
                           compressed_buf_size, this->buf, &chunk_size);

                        switch (status)
                        {
                            case SNAPPY_OK:
                                break;
                            case SNAPPY_INVALID_INPUT:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                                this->status = false;
                                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                            case SNAPPY_BUFFER_TOO_SMALL:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                                this->status = false;
                                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                            default:
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                                this->status = false;
                                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                        }

                        this->pos = 0;
                        // When reading, raw_size represent the current chunk size
                        this->raw_size = chunk_size;
                        if (!this->raw_size) { // end of file reached
                           break;
                        }

                    }
                }
                
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->raw_size - this->pos;
                // Check how much we can copy
                unsigned int copiable_size = MIN(remaining_size, remaining_requested_size);
                // Copy buffer to caller
                ::memcpy(&(*pbuffer)[requested_size - remaining_requested_size]
                        , this->buf + this->pos, copiable_size);
                this->pos      += copiable_size;
                remaining_requested_size -= copiable_size;
                // Check if we reach the end
                if (this->raw_size == this->pos) {
                    this->raw_size = 0;
                }
            }
            
            *pbuffer += requested_size - remaining_requested_size;
            this->last_quantum_received += requested_size - remaining_requested_size;
            
            if (remaining_requested_size != 0){
                this->status = false;
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
            }
        } // else encryption
        else {
            TODO("the do_recv API is annoying (need some intermediate pointer to get result), fix it => read all or raise exeception?");
            TODO("this is blocking read, add support for timeout reading");
            TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
            try {
                this->raw.read_min(this->fd, requested_size, requested_size);
                ::memcpy(&(*pbuffer)[0], this->raw.b, requested_size);
                *pbuffer += requested_size;
                this->raw.end = 0;
                this->last_quantum_received += requested_size;
            } catch (...) {
                this->status = false;
                throw;
            }
        }
    }
};

#endif

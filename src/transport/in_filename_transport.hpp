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

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>

#include "utils/log.hpp"
#include "openssl_crypto.hpp"
#include "transport/transport.hpp"
#include "capture/cryptofile.hpp"
#include "utils/urandom_read.hpp"

struct InFilenameTransport : public Transport
{
    int fd;
    int encryption;

    struct raw_t {
        uint8_t b[65536];
        size_t start;
        size_t end;

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
            while ((this->end - this->start) < min_to_read) {
                ssize_t ret = ::read(fd, &this->b[this->end], to_read - (this->end-this->start));
                if (ret <= 0){
                    if (ret < 0 && errno == EINTR){
                        continue;
                    }
                    if (ret == 0){
                        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
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
        uint8_t b[65536];
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
    InFilenameTransport(CryptoContext & cctx, int fd, const uint8_t * base, size_t base_len)
        : fd(fd)
        , pos(0)
        , raw_size(0)
        , state(0)
    {
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
            const uint8_t * const &r = &this->raw.b[8];
            const uint8_t iv[32] = {
                r[0x00], r[0x01], r[0x02], r[0x03], r[0x04],
                r[0x05], r[0x06], r[0x07], r[0x08], r[0x09],
                r[0x0A], r[0x0B], r[0x0C], r[0x0D], r[0x0E], r[0x0F],
                r[0x10], r[0x11], r[0x12], r[0x13], r[0x14],
                r[0x15], r[0x16], r[0x17], r[0x18], r[0x19],
                r[0x1A], r[0x1B], r[0x1C], r[0x1D], r[0x1E], r[0x1F],
                }
                ;
            this->raw.end = 0;

            uint8_t trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher

            cctx.get_derived_key(trace_key, base, base_len);

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
    void do_recv(uint8_t ** pbuffer, size_t requested_size) override {
        if (!this->status){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }

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

                        // TODO this is blocking read, add support for timeout reading
                        // TODO add check for O_WOULDBLOCK, as this is is blockig it would be bad
                        this->raw.read_min(this->fd, ciphered_buf_size, ciphered_buf_size);
//                        {
//                        auto p = reinterpret_cast<uint8_t*>(&this->raw.b[0]);
//                        printf("raw_size=%d %d raw=%.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n", this->raw.end, ciphered_buf_size,
//                            p[0], p[1], p[2], p[3], p[4],
//                            p[5], p[6], p[7], p[8], p[9],
//                            p[10], p[11], p[12], p[13], p[14],
//                            p[15], p[16], p[17], p[18]

//                        );
//                        }

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
                                              &this->raw.b[0],
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

            if (remaining_requested_size > 0){
                this->status = false;
            }
            return;
            //return requested_size - remaining_requested_size;
        } // else encryption
        else {
            // TODO the do_recv API is annoying (need some intermediate pointer to get result), fix it => read all or raise exeception?
            // TODO this is blocking read, add support for timeout reading
            // TODO add check for O_WOULDBLOCK, as this is is blockig it would be bad
            // TODO The best way would probably be to use the usual read API ?

            size_t end = 0;
            if (this->raw.end > 0 && this->raw.end > requested_size){
                ::memcpy(&(*pbuffer)[end], &this->raw.b[0], requested_size);
                ::memmove(&this->raw.b[0], &this->raw.b[requested_size], requested_size-this->raw.end);
                this->raw.end -= requested_size;
                this->last_quantum_received += requested_size;
                *pbuffer += requested_size;
                return;
            }
            if (this->raw.end > 0 && this->raw.end <= requested_size){
                memcpy(&(*pbuffer)[end], &this->raw.b[0], this->raw.end);
                end = this->raw.end;
                this->raw.end = 0;
            }
            while (end < requested_size) {
                ssize_t ret = ::read(fd, &(*pbuffer)[end], requested_size - end);
                if (ret <= 0){
                    if (ret < 0 && errno == EINTR){
                        continue;
                    }
                    if (ret == 0){
                        break;
                    }
                    LOG(LOG_ERR, "failed reading from file");
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
                end += ret;
            }
            *pbuffer += end;
            this->last_quantum_received += end;
            if (end != requested_size){
                this->status = false;
            }
        }
    }
};

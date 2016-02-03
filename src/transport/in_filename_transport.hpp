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
    
    char read_buffer[32768];
    int start_index;
    int end_index;

    char           buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
    uint32_t       pos;                     // current position in buf
    uint32_t       raw_size;                // the unciphered/uncompressed file size
    uint32_t       state;                   // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;


public:
    InFilenameTransport(CryptoContext * cctx, const char * filename)
        : fd(-1)
        , pos(0)
        , raw_size(0)
        , state(0)
        , MAX_CIPHERED_SIZE(0)
    {
        this->fd = ::open(filename, O_RDONLY);
        if (this->fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        unsigned char tbf[40];

        size_t rl = 40;
        while (rl > 36) {
            ssize_t ret = ::read(this->fd, &tbf[40 - rl], rl);
            if (ret <= 0){
                if (ret < 0 && errno == EINTR){
                    continue;
                }
                LOG(LOG_ERR, "failed reading magic from encrypted file=%s\n", filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            // We must exit loop or we will enter infinite loop
            rl -= ret;
        }

        const uint32_t magic = tbf[0]+(tbf[1]<<8)+(tbf[2]<<16)+(tbf[3]<<24);
                             
        if (magic != WABCRYPTOFILE_MAGIC) {
            TODO("We close and reopen because file is not encrypted and we want avoiding seek"
                "but we should keep what has already been read in a buffer instead."
                "This is important for that code to work on network sockets");
            ::close(this->fd);
            this->fd = ::open(filename, O_RDONLY);
            if (this->fd < 0) {
                LOG(LOG_ERR, "failed opening=%s\n", filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            this->encryption = 0;
        }

        if (this->encryption){
            while (rl) {
                ssize_t ret = ::read(this->fd, &tbf[40 - rl], rl);
                if (ret <= 0){
                    if (ret < 0 && errno == EINTR){
                        continue;
                    }
                    LOG(LOG_ERR, "failed reading magic from encrypted file=%s\n", filename);
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
                // We must exit loop or we will enter infinite loop
                rl -= ret;
            }

            const int version = tbf[4]+(tbf[5]<< 8)+(tbf[6]<<16)+(tbf[7]<<24);

            if (version > WABCRYPTOFILE_VERSION) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                    ::getpid(), version, WABCRYPTOFILE_VERSION);
                LOG(LOG_ERR, "failed opening=%s\n", filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));

            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

            cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == cctx->compute_hmac(trace_key, derivator)) {
                LOG(LOG_ERR, "failed opening=%s\n", filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            unsigned char * const iv = tbf + 8;

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
            const int          nrounds = 5;
            unsigned char      key[32];
            const int i = ::EVP_BytesToKey(cipher
                                           , ::EVP_sha1()
                                           , reinterpret_cast<const uint8_t*>(&salt[0])
                                           , trace_key
                                           , CRYPTO_KEY_LENGTH
                                           , nrounds
                                           , key
                                           , nullptr);
            if (i != 32) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                LOG(LOG_ERR, "failed opening=%s\n", filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                LOG(LOG_ERR, "failed opening=%s\n", filename);
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
    int decrypt_xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
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
                    unsigned char tbf[4] = {};
                    size_t rl = 4;
                    while (rl) {
                        ssize_t res = ::read(this->fd, &tbf[4 - rl], rl);
                        if (res <= 0){
                            if (res!=0 && errno == EINTR){continue;}
                            this->status = false;
                            throw Error(ERR_TRANSPORT_READ_FAILED, res==-1?errno:-1);
                        }
                        rl -= res;
                    }

                    uint32_t ciphered_buf_size = tbf[0]+(tbf[1]<<8)+(tbf[2]<<16)+(tbf[3]<< 24);

                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state |= CF_EOF;
                        this->pos = 0;
                        this->raw_size = 0;
                        break;
                    }
                    if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        this->status = false;
                        throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                    }
                    else {
                        uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                        //char ciphered_buf[ciphered_buf_size];
                        unsigned char ciphered_buf[65536];
                        //char compressed_buf[compressed_buf_size];
                        unsigned char compressed_buf[65536];

                        TODO("this is blocking read, add support for timeout reading");
                        TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
                        size_t rl = ciphered_buf_size;
                        while (rl) {
                            ssize_t res = ::read(this->fd
                                                , &ciphered_buf[ciphered_buf_size - rl]
                                                , rl);
                            if (res <= 0){
                                if (res == 0){
                                    this->status = false;
                                    throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                                }
                                if (errno == EINTR){
                                    continue;
                                }
                                // Error should still be there next time we try to read
                                if (rl != ciphered_buf_size){
                                    this->status = false;
                                    throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                                }
                                // here ret < 0
                                this->status = false;
                                throw Error(ERR_TRANSPORT_READ_FAILED, res);
                            }
                            // We must exit loop or we will enter infinite loop
                            rl -= res;
                        }

                        if (this->decrypt_xaes_decrypt(ciphered_buf, ciphered_buf_size,
                                                       compressed_buf, &compressed_buf_size)) {
                            this->status = false;
                            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
                        }

                        size_t chunk_size = CRYPTO_BUFFER_SIZE;
                        const snappy_status status = snappy_uncompress(
                            reinterpret_cast<char *>(compressed_buf),
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
            size_t rl = requested_size;
            while (rl) {
                size_t res = ::read(this->fd, *pbuffer + (requested_size - rl), rl);
                if (res <= 0){
                    // We must exit loop or we will enter infinite loop
                    if ((res == 0)
                    || ((errno!=EINTR) && (rl != requested_size))){
                        break;
                    }
                    if (errno == EINTR){
                        continue;
                    }
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                rl -= res;
            }
            *pbuffer += requested_size - rl;
            this->last_quantum_received += requested_size - rl;
            if (rl != 0){
                this->status = false;
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
            }
        }
    }
};

#endif

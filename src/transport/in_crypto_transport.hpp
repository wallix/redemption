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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan, Clément Moroldo
 */


#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "transport/transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/fileutils.hpp"
#include "utils/parse.hpp"
#include "capture/cryptofile.hpp"

class InCryptoTransport : public Transport
{
    int fd;
    bool eof;
    size_t file_len;
    size_t current_len;
    
    CryptoContext & cctx;
    char clear_data[CRYPTO_BUFFER_SIZE];  // contains either raw data from unencrypted file
                                          // or already decrypted/decompressed data
    uint32_t clear_pos;                   // current position in clear_data buf
    uint32_t raw_size;                    // the unciphered/uncompressed data available in buffer

    EVP_CIPHER_CTX ectx;                  // [en|de]cryption context
    uint32_t state;                       // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;     // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    int encryption; // encryption: 0: auto, 1: encrypted, 2: not encrypted
    bool encrypted;
    
public:
    explicit InCryptoTransport(CryptoContext & cctx, int encryption) noexcept
        : fd(-1)
        , eof(true)
        , file_len(0)
        , current_len(0)
        , cctx(cctx)
        , clear_data{}
        , clear_pos(0)
        , raw_size(0)
        , state(0)
        , MAX_CIPHERED_SIZE(0)
        , encryption(encryption)
        , encrypted(false)
    {
    } 

    ~InCryptoTransport() {
    }

    bool is_open()
    {
        return this->fd != -1;
    }

    void open(const char * pathname)
    {
        if (this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        
        struct stat sb;
        if (::stat(pathname, &sb) == 0) {
            this->file_len = sb.st_size;
        }
        
        this->fd = ::open(pathname, O_RDONLY);
        if (this->fd < 0) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        this->eof = false;


        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(pathname, base_len));

        ::memset(this->clear_data, 0, sizeof(this->clear_data));

        ::memset(&this->ectx, 0, sizeof(this->ectx));
        this->clear_pos = 0;
        this->raw_size = 0;
        this->state = 0;

        const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
        this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        // todo: we could read in clear_data, that would avoid some copying
        uint8_t data[40];
        size_t avail = 0;
        while (avail != 40) {
            ssize_t ret = ::read(this->fd, &data[avail], 40-avail);
            if (ret < 0 && errno == EINTR){
                continue;
            }
            if (ret <= 0){
                // Either read error or EOF: in both cases we are in trouble
                if (ret == 0) {
                    // if we have less than magic followed by encryption header
                    // then our file is necessarilly not encrypted.
                    this->encrypted = false;
                    // encryption requested but no encryption
                    if (this->encryption == 1){
                        this->close();
                        throw Error(ERR_TRANSPORT_READ_FAILED);
                    }
                    // copy what we have, it's not encrypted
                    this->raw_size = avail;
                    this->clear_pos = 0;
                    ::memcpy(this->clear_data, data, avail);
                    return;
                }
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            avail += ret;
        }

        if (this->encryption == 2){
            // copy what we have, it's not encrypted, don't care about magic
            this->raw_size = 40;
            this->clear_pos = 0;
            ::memcpy(this->clear_data, data, 40);
            return;
        }

        // Encrypted/Compressed file header (40 bytes)
        // -------------------------------------------
        // MAGIC: 4 bytes
        // 0x57 0x43 0x46 0x4D (WCFM)
        // VERSION: 4 bytes
        // 0x01 0x00 0x00 0x00
        // IV: 32 bytes
        // (random)

        {
            Parse p(data);
            const int magic = p.in_uint32_le();
            if (magic != 0x5743464D) {
                this->encrypted = false;
                // encryption requested but no encryption
                if (this->encryption == 1){
                    this->close();
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                // Auto: rely on magic copy what we have, it's not encrypted
                this->raw_size = 40;
                this->clear_pos = 0;
                ::memcpy(this->clear_data, data, 40);
                return;
            }
        }
        this->encrypted = true;        
        Parse p(data+4);
        const int version = p.in_uint32_le();
        if (version > WABCRYPTOFILE_VERSION) {
            // Unsupported version
            this->close();
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        // TODO: replace p.p with some array view of 32 bytes ?
        const uint8_t * const iv = p.p;
        const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
        const uint8_t salt[]  = { 0x39, 0x30, 0x00, 0x00, 0x31, 0xd4, 0x00, 0x00 };
        const int          nrounds = 5;
        unsigned char      key[32];

        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        cctx.get_derived_key(trace_key, base, base_len);

        int evp_bytes_to_key_res = ::EVP_BytesToKey(cipher, ::EVP_sha1(), salt,
                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
        if (32 != evp_bytes_to_key_res){
            this->close();
            LOG(LOG_INFO, "Can't read EVP_BytesToKey");
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        ::EVP_CIPHER_CTX_init(&this->ectx);
        if(::EVP_DecryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
            // TODO: add error management
            LOG(LOG_INFO, "Can't read EVP_DecryptInit_ex");
            this->close();
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
    }
    
    void close()
    {
        if (!this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        ::close(this->fd);
        this->fd = -1;
        this->eof = true;
    }

    bool is_eof() {
        return this->eof;
    }

private:
    void do_recv_new(uint8_t * buffer, size_t len) override 
    {
        if (this->encrypted){
            if (this->state & CF_EOF) {
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
            }

            unsigned int requested_size = len;
            while (requested_size > 0) {
                // If we do not have any clear data available read some
                if (!this->raw_size) {
                    uint8_t hlen[4] = {};
                    {
                        size_t rlen = 4;
                        while (rlen) {
                            ssize_t ret = ::read(this->fd, &hlen[4 - rlen], rlen);
                            if (ret <= 0){
                                // Unexpected EOF, we are in trouble for decompression: fatal
                                if (ret == 0){
                                    this->close();
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                if (errno == EINTR){
                                    continue;
                                }
                                // Error should still be there next time we try to read: fatal
                                this->close();
                                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                            }
                            rlen -= ret;
                        }
                    }

                    Parse p(hlen);
                    uint32_t ciphered_buf_size = p.in_uint32_le();
                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->state = CF_EOF;
                        this->clear_pos = 0;
                        this->raw_size = 0;
                        this->close();
                        break;
                    }

                    if (ciphered_buf_size > this->MAX_CIPHERED_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        this->close();
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }

                    uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;

                    //char ciphered_buf[ciphered_buf_size];
                    unsigned char ciphered_buf[65536];
                    //char compressed_buf[compressed_buf_size];
                    unsigned char compressed_buf[65536];

                    {
                        size_t rlen = ciphered_buf_size;
                        while (rlen) {
                            ssize_t ret = ::read(this->fd, &ciphered_buf[ciphered_buf_size - rlen], rlen);
                            if (ret < 0){
                                if (errno == EINTR){
                                    continue;
                                }
                                // Error should still be there next time we try to read
                                // TODO: see if we have already decrypted data
                                // error reported too early
                                this->close();
                                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                            }
                            // We must exit loop or we will enter infinite loop
                            if (ret == 0){
                                // TODO: see if we have already decrypted data
                                // error reported too early
                                this->close();
                                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                            }
                            rlen -= ret;
                        }
                    }

                    int safe_size = compressed_buf_size;
                    int remaining_size = 0;

                    /* allows reusing of ectx for multiple encryption cycles */
                    if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    if (EVP_DecryptUpdate(&this->ectx, compressed_buf, &safe_size, ciphered_buf, ciphered_buf_size) != 1){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    if (EVP_DecryptFinal_ex(&this->ectx, compressed_buf + safe_size, &remaining_size) != 1){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    compressed_buf_size = safe_size + remaining_size;

                    size_t chunk_size = CRYPTO_BUFFER_SIZE;
                    const snappy_status status = snappy_uncompress(
                            reinterpret_cast<char *>(compressed_buf),
                            compressed_buf_size, this->clear_data, &chunk_size);

                    switch (status)
                    {
                        case SNAPPY_OK:
                            break;
                        case SNAPPY_INVALID_INPUT:
                            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                        case SNAPPY_BUFFER_TOO_SMALL:
                            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                        default:
                            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }

                    this->clear_pos = 0;
                    // When reading, raw_size represent the current chunk size
                    this->raw_size = chunk_size;

                    if (!this->raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->raw_size - this->clear_pos;
                // Check how much we can copy
                unsigned int copiable_size = std::min(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(&buffer[len - requested_size], this->clear_data + this->clear_pos, copiable_size);
                this->clear_pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->raw_size == this->clear_pos) {
                    this->raw_size = 0;
                }
            }
//            return len - requested_size;
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            return;
        }
        else {
            if (this->raw_size - this->clear_pos > len){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], len);
                this->clear_pos += len;
                this->current_len += len;
                if (this->file_len <= this->current_len) {
                    this->eof = true;
                }
                return;
            }
            unsigned int remaining_len = len;
            if (this->raw_size - this->clear_pos > 0){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], this->raw_size - this->clear_pos);
                remaining_len -= this->raw_size - this->clear_pos;
                this->raw_size = 0;
                this->clear_pos = 0;
            }

            int res = -1;
            while(remaining_len){
                res = ::read(this->fd, &buffer[len - remaining_len], remaining_len);
                if (res <= 0){
                    if ((res == 0) || ((errno != EINTR) && (remaining_len != len))){
                        break;
                    }
                    if (errno == EINTR){
                        continue;
                    }
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                remaining_len -= res;
            };
            res = len - remaining_len;
            this->current_len += res;
            if (this->file_len <= this->current_len) {
                this->eof = true;
            }
            this->last_quantum_received += res;
            if (remaining_len != 0){
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
            }
        }
    }    
};

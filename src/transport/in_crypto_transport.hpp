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
#include "utils/sugar/unique_fd.hpp"

#include <memory>


class InCryptoTransport : public Transport //, public PartialIO
{
public:
    enum class EncryptionMode { Auto, Encrypted, NotEncrypted };

private:
    int fd;
    bool eof;
    size_t file_len;
    size_t current_len;

    CryptoContext & cctx;
    char clear_data[CRYPTO_BUFFER_SIZE];  // contains either raw data from unencrypted file
                                          // or already decrypted/decompressed data
    uint32_t clear_pos;                   // current position in clear_data buf
    uint32_t raw_size;                    // the unciphered/uncompressed data available in buffer

    DecryptContext ectx;
    // TODO: state to remove ? Seems to duplicate eof flag
    uint32_t state;                       // enum crypto_file_state
    unsigned int   MAX_CIPHERED_SIZE;     // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;
    EncryptionMode encryption_mode;
    bool encrypted;

public:
    explicit InCryptoTransport(CryptoContext & cctx, EncryptionMode encryption_mode) noexcept
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
    , encryption_mode(encryption_mode)
    , encrypted(false)
    {
    }

    ~InCryptoTransport()
    {
        if (this->is_open()){
            ::close(this->fd);
        }
    }

    bool is_encrypted() const
    {
        return this->encrypted;
    }

    bool is_open() const
    {
        return this->fd != -1;
    }

    struct HASH {
        uint8_t hash[MD_HASH::DIGEST_LENGTH];
    };

    const HASH qhash(const char * pathname)
    {
        SslHMAC_Sha256 hm4k(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);

        {
            int fd = ::open(pathname, O_RDONLY);
            if (fd < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            unique_fd auto_close(fd);

            constexpr std::size_t buffer_size = 4096;
            uint8_t buffer[buffer_size];
            size_t total_length = 0;
            do {
                size_t const remaining_size = buffer_size - total_length;
                ssize_t res = ::read(fd, buffer, remaining_size);
                if (res == 0) { break; }
                if (res < 0 && errno == EINTR) { continue; }
                if (res < 0) { throw Error(ERR_TRANSPORT_READ_FAILED, errno); }
                hm4k.update(buffer, res);
                total_length += res;
            } while (total_length != buffer_size);
        }

        HASH qhash;
        hm4k.final(qhash.hash);
        return qhash;
    }

    const HASH fhash(const char * pathname)
    {
        SslHMAC_Sha256 hm(this->cctx.get_hmac_key(), HMAC_KEY_LENGTH);

        {
            int fd = ::open(pathname, O_RDONLY);
            if (fd < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            unique_fd auto_close(fd);

            uint8_t buffer[4096];
            do {
                ssize_t res = ::read(fd, &buffer[0], sizeof(buffer));
                if (res == 0) { break; }
                if (res < 0 && errno == EINTR) { continue; }
                if (res < 0) { throw Error(ERR_TRANSPORT_READ_FAILED, errno); }
                hm.update(buffer, res);
            } while (1);
        }

        HASH fhash;
        hm.final(fhash.hash);
        return fhash;
    }

//    void hash(const char * pathname)
//    {
//        this->open(pathname);
//    }

    size_t partial_read(uint8_t * buffer, size_t len) __attribute__ ((warn_unused_result))
    {
        return this->do_partial_read(buffer, len);
    }

    size_t partial_read(char * buffer, size_t len) __attribute__ ((warn_unused_result))
    {
        return this->do_partial_read(reinterpret_cast<uint8_t*>(buffer), len);
    }

    void open(const char * const pathname, const_bytes_array derivator)
    {
        if (this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        this->fd = ::open(pathname, O_RDONLY);
        if (this->fd < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        this->eof = false;
        this->current_len = 0;

        ::memset(this->clear_data, 0, sizeof(this->clear_data));

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
                    if (this->encryption_mode == EncryptionMode::Encrypted){
                        this->close();
                        throw Error(ERR_TRANSPORT_READ_FAILED);
                    }
                    // copy what we have, it's not encrypted
                    this->raw_size = avail;
                    this->clear_pos = 0;
                    ::memcpy(this->clear_data, data, avail);
                    this->file_len = this->get_file_len(pathname);
                    return;
                }
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            avail += ret;
        }

        if (this->encryption_mode == EncryptionMode::NotEncrypted){
            // copy what we have, it's not encrypted, don't care about magic
            this->raw_size = 40;
            this->clear_pos = 0;
            ::memcpy(this->clear_data, data, 40);
            this->file_len = this->get_file_len(pathname);
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

        Parse p(data);
        {
            const int magic = p.in_uint32_le();
            if (magic != WABCRYPTOFILE_MAGIC) {
                this->encrypted = false;
                // encryption requested but no encryption
                if (this->encryption_mode == EncryptionMode::Encrypted){
                    this->close();
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }
                // Auto: rely on magic. Copy what we have, it's not encrypted
                this->raw_size = 40;
                this->clear_pos = 0;
                ::memcpy(this->clear_data, data, 40);
                this->file_len = this->get_file_len(pathname);
                return;
            }
        }
        this->encrypted = true;
        // check version
        {
            const uint32_t version = p.in_uint32_le();
            if (version > WABCRYPTOFILE_VERSION) {
                // Unsupported version
                this->close();
                LOG(LOG_INFO, "unsupported_version");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }

        // Read File trailer, check for magic trailer and size
        off64_t off_here = lseek64(this->fd, 0, SEEK_CUR);
        lseek64(this->fd, -8, SEEK_END);
        uint8_t trail[8] = {};
        this->raw_read(trail, 8);
        Parse pt1(&trail[0]);
        if (pt1.in_uint32_be() != WABCRYPTOFILE_MAGIC){
            // truncated file
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        Parse pt2(&trail[4]);
        this->file_len = pt2.in_uint32_le();
        lseek64(this->fd, off_here, SEEK_SET);
        // the fd is back where we was

        // TODO: replace p.with some array view of 32 bytes ?
        const uint8_t * const iv = p.p;
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        cctx.get_derived_key(trace_key, derivator);

        if (!this->ectx.init(trace_key, iv)) {
            this->close();
            throw Error(ERR_SSL_CALL_FAILED);
        }
    }

    void open(const char * const pathname)
    {
        size_t base_len = 0;
        const char * base = basename_len(pathname, base_len);
        this->open(pathname, {base, base_len});
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
    // this perform atomic read, partial read will result in exception
    void raw_read(uint8_t buffer[], const size_t len)
    {
        size_t rlen = len;
        while (rlen) {
            ssize_t ret = ::read(this->fd, &buffer[len - rlen], rlen);
            if (ret <= 0){ // unexpected EOF or error
                if (ret != 0 && errno == EINTR){
                    continue;
                }
                this->close();
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            rlen -= ret;
        }
    }

    size_t do_partial_read(uint8_t * buffer, size_t len)
    {
        if (this->eof){
            return 0;
        }
        if (this->encrypted){
            if (this->state & CF_EOF) {
                return 0;
            }

            // If we do not have any clear data available read some
            if (!this->raw_size) {

                // Read a full ciphered block at once
                uint8_t hlen[4] = {};
                this->raw_read(hlen, 4);

                Parse p(hlen);
                uint32_t enc_len = p.in_uint32_le();
                if (enc_len == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                    this->state = CF_EOF;
                    this->clear_pos = 0;
                    this->raw_size = 0;
                    this->eof = true;
                    return 0;
                }
                if (enc_len > this->MAX_CIPHERED_SIZE) { // integrity error
                    this->close();
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                // PERF allocation in loop
                std::unique_ptr<uint8_t []> enc_buf(new uint8_t[enc_len]);
                this->raw_read(&enc_buf[0], enc_len);

                // PERF allocation in loop
                std::unique_ptr<uint8_t []> pack_buf(new uint8_t[enc_len + AES_BLOCK_SIZE]);
                size_t pack_buf_size = this->ectx.decrypt(&enc_buf[0], enc_len, &pack_buf[0]);

                size_t chunk_size = CRYPTO_BUFFER_SIZE;
                const snappy_status status = snappy_uncompress(
                        reinterpret_cast<char *>(&pack_buf[0]),
                        pack_buf_size, this->clear_data, &chunk_size);

                switch (status)
                {
                    case SNAPPY_OK:
                        break;
                    case SNAPPY_INVALID_INPUT:
                    case SNAPPY_BUFFER_TOO_SMALL:
                    default:
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                this->clear_pos = 0;
                // When reading, raw_size represent the current chunk size
                this->raw_size = chunk_size;

            } // raw_size

            // Check how much we can copy
            std::size_t const copiable_size = std::min(
                len, static_cast<std::size_t>(this->raw_size - this->clear_pos)
            );
            // Copy buffer to caller
            ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], copiable_size);
            this->clear_pos += copiable_size;
            this->current_len += copiable_size;
            if (this->file_len <= this->current_len) {
                this->eof = true;
            }
            // Check if we reach the end
            if (this->raw_size == this->clear_pos) {
                this->raw_size = 0;
            }
            return copiable_size;
        }
        else {
            if (this->raw_size - this->clear_pos > len){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], len);
                this->clear_pos += len;
                this->current_len += len;
                if (this->file_len <= this->current_len) {
                    this->eof = true;
                }
                return len;
            }
            size_t remaining_len = len;
            if (this->raw_size - this->clear_pos > 0){
                ::memcpy(&buffer[0], &this->clear_data[this->clear_pos], this->raw_size - this->clear_pos);
                remaining_len -= this->raw_size - this->clear_pos;
                this->raw_size = 0;
                this->clear_pos = 0;
            }
            while(remaining_len){
                ssize_t const res = ::read(this->fd, &buffer[len - remaining_len], remaining_len);
                if (res <= 0){
                    if (res == 0) {
                        this->eof = true;
                        break;
                    }
                    if (errno == EINTR){
                        continue;
                    }
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                remaining_len -= res;
            };

            this->current_len += len;
            if (this->file_len <= this->current_len) {
                this->eof = true;
            }
            return len - remaining_len;
        }
    }

    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        size_t res;
        size_t total = 0;
        size_t remaining_len = len;
        while ((res = do_partial_read(buffer, remaining_len)) && res != remaining_len) {
            total += res;
            buffer += res;
            remaining_len -= res;
        }
        total += res;
        if (res != 0 && total != len) {
            throw Error(ERR_TRANSPORT_READ_FAILED, 0);
        }
        return total == len ? Read::Ok : Read::Eof;
    }

    std::size_t get_file_len(char const * pathname)
    {
        struct stat sb;
        if (int err = ::stat(pathname, &sb)) {
            LOG(LOG_ERR, "crypto: stat error %d", err);
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        return sb.st_size;
    }
};

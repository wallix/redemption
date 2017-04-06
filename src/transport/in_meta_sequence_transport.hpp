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
#include <cstddef>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils/fileutils.hpp"
#include "openssl_crypto.hpp"
#include "utils/log.hpp"
#include "utils/urandom_read.hpp"
#include "utils/sugar/exchange.hpp"
#include "utils/chex_to_int.hpp"
#include "capture/cryptofile.hpp"
#include "transport/transport.hpp"

namespace transbuf {
    class ofile_buf
    {
        int fd;
    public:
        ofile_buf() : fd(-1) {}
        ~ofile_buf()
        {
            this->close();
        }

        int open(const char * filename, mode_t mode)
        {
            this->close();
            this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
            return this->fd;
        }

        int close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->fd);
                this->fd = -1;
                return ret;
            }
            return 0;
        }

        ssize_t write(const void * data, size_t len)
        {
            size_t remaining_len = len;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(this->fd,
                    static_cast<const char*>(data) + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    return -1;
                }
                remaining_len -= ret;
                total_sent += ret;
            }
            return total_sent;
        }

        bool is_open() const noexcept
        { return -1 != this->fd; }

        off64_t seek(off64_t offset, int whence) const
        { return ::lseek64(this->fd, offset, whence); }

        int flush() const
        { return 0; }
    };

}

struct temporary_concat
{
    char str[1024];

    temporary_concat(const char * a, const char * b)
    {
        if (std::snprintf(this->str, sizeof(this->str), "%s%s", a, b) >= int(sizeof(this->str))) {
            throw Error(ERR_TRANSPORT);
        }
    }

    const char * c_str() const noexcept
    { return this->str; }
};

inline time_t meta_parse_sec(const char * first, const char * last)
{
    time_t sec = 0;
    for (; first != last; ++first) {
        if (*first < '0' || '9' < *first) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        unsigned old_sec = sec;
        sec *= 10;
        sec += *first - '0';
        if (old_sec > sec) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
    }
    return sec;
}

struct MetaLine
{
    char    filename[PATH_MAX + 1];
    off_t   size;
    mode_t  mode;
    uid_t   uid;
    gid_t   gid;
    dev_t   dev;
    ino_t   ino;
    time_t  mtime;
    time_t  ctime;
    time_t  start_time;
    time_t  stop_time;
    unsigned char hash1[MD_HASH::DIGEST_LENGTH];
    unsigned char hash2[MD_HASH::DIGEST_LENGTH];
};

class InMetaSequenceTransport : public Transport
{
    struct cfb_t
    {
        CryptoContext * cctx;
        int            fd;
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

        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       state;                   // enum crypto_file_state
        unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        cfb_t(CryptoContext * cctx, int encryption)
            : cctx(cctx)
            , fd(-1)
            , encryption(encryption)
        {}

        bool is_open() const noexcept
        { return -1 != this->fd; }


        int open(const char * filename)
        {
            if (-1 != this->fd) {
                ::close(this->fd);
                this->fd = -1;
            }
            this->fd = ::open(filename, O_RDONLY);
            int status = this->fd;

            if (!this->encryption) {
                return status;
            }
            if (status < 0) {
                return status;
            }

            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            size_t base_len = 0;
            const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));
            this->cctx->get_derived_key(trace_key, base, base_len);

            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));

            this->pos = 0;
            this->raw_size = 0;
            this->state = 0;
            const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
            this->MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

            unsigned char tmp_buf[40];

            if (const ssize_t err = this->raw_read(tmp_buf, 40)) {
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

        ssize_t read(void * data, size_t len)
        {
            if (this->encryption){
                if (this->state & CF_EOF) {
                    //printf("cf EOF\n");
                    return 0;
                }

                unsigned int requested_size = len;

                while (requested_size > 0) {
                    // Check how much we have decoded
                    if (!this->raw_size) {
                        unsigned char tmp_buf[4] = {};
                        if (const int err = this->raw_read(tmp_buf, 4)) {
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

                                if (const ssize_t err = this->raw_read(ciphered_buf, ciphered_buf_size)) {
                                    return err;
                                }

                                int safe_size = compressed_buf_size;
                                int remaining_size = 0;

                                /* allows reusing of ectx for multiple encryption cycles */
                                if (EVP_DecryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                                    return -1;
                                }
                                if (EVP_DecryptUpdate(&this->ectx, compressed_buf, &safe_size, ciphered_buf, ciphered_buf_size) != 1){
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                                    return -1;
                                }
                                if (EVP_DecryptFinal_ex(&this->ectx, compressed_buf + safe_size, &remaining_size) != 1){
                                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                                    return -1;
                                }
                                compressed_buf_size = safe_size + remaining_size;


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
            else {
                // TODO this is blocking read, add support for timeout reading
                // TODO add check for O_WOULDBLOCK, as this is is blockig it would be bad
                size_t remaining_len = len;
                while (remaining_len) {
                    ssize_t ret = ::read(this->fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                    if (ret < 0){
                        if (errno == EINTR){
                            continue;
                        }
                        // Error should still be there next time we try to read
                        if (remaining_len != len){
                            return len - remaining_len;
                        }
                        return ret;
                    }
                    // We must exit loop or we will enter infinite loop
                    if (ret == 0){
                        break;
                    }
                    remaining_len -= ret;
                }
                return len - remaining_len;
            }
        }

        int file_close()
        {
            if (-1 != this->fd) {
                const int ret = ::close(this->fd);
                this->fd = -1;
                return ret;
            }
            return 0;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        ssize_t raw_read(void * data, size_t len)
        {
            // TODO this is blocking read, add support for timeout reading
            // TODO add check for O_WOULDBLOCK, as this is is blockig it would be bad
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(this->fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        ssize_t err = len - remaining_len;
                        if (err < ssize_t(len)){
                            return (err < 0 ? err : -1);
                        }
                        break;
                    }
                    if (ret < ssize_t(len)){
                        return (ret < 0 ? ret : -1);
                    }
                    break;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            if (remaining_len > 0){
                return -1;
            }
            return 0;
        }
    } cfb;

    class buf_meta_t {
        CryptoContext * cctx;
        int file_fd;
        int encryption;
        char           decrypt_buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX decrypt_ectx;                    // [en|de]cryption context
        uint32_t       decrypt_pos;                     // current position in buf
        uint32_t       decrypt_raw_size;                // the unciphered/uncompressed file size
        uint32_t       decrypt_state;                   // enum crypto_file_state
        unsigned int   decrypt_MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

    public:
        buf_meta_t(CryptoContext * cctx, int encryption)
            : cctx(cctx)
            , file_fd(-1)
            , encryption(encryption)
            {}

        ssize_t file_read(void * data, size_t len)
        {
            // TODO this is blocking read, add support for timeout reading
            // TODO add check for O_WOULDBLOCK, as this is is blockig it would be bad
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(this->file_fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        return len - remaining_len;
                    }
                    return ret;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            return len - remaining_len;
        }

        void open(const char * meta_filename)
        {
            LOG(LOG_INFO, "InMetaSequenceTransport::open()");
            if (this->encryption){
                unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
                size_t base_len = 0;
                const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(meta_filename, base_len));
                this->cctx->get_derived_key(trace_key, base, base_len);

                this->file_close();
                this->file_fd = ::open(meta_filename, O_RDONLY);
                int err = this->file_fd;
                if (err < 0) {
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }

                ::memset(this->decrypt_buf, 0, sizeof(this->decrypt_buf));
                ::memset(&this->decrypt_ectx, 0, sizeof(this->decrypt_ectx));

                this->decrypt_pos = 0;
                this->decrypt_raw_size = 0;
                this->decrypt_state = 0;
                const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
                this->decrypt_MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

                unsigned char tmp_buf[40];
                {

                    ssize_t err = this->file_read(tmp_buf, 40);
                    if (err < ssize_t(40)){
                        throw Error(ERR_TRANSPORT_OPEN_FAILED);
                    }
                }
                // Check magic
                const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
                if (magic != WABCRYPTOFILE_MAGIC) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n",
                        ::getpid(), magic, WABCRYPTOFILE_MAGIC);
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
                const int version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
                if (version > WABCRYPTOFILE_VERSION) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                        ::getpid(), version, WABCRYPTOFILE_VERSION);
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }

                unsigned char * const iv = tmp_buf + 8;

                const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
                const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
                const int          nrounds = 5;
                unsigned char      key[32];
                const int i = ::EVP_BytesToKey(cipher,
                                    ::EVP_sha1(),
                                    reinterpret_cast<const unsigned char *>(salt),
                                    trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
                if (i != 32) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }

                ::EVP_CIPHER_CTX_init(&this->decrypt_ectx);
                if(::EVP_DecryptInit_ex(&this->decrypt_ectx, cipher, nullptr, key, iv) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
            }
            else {
                this->file_close();
                this->file_fd = ::open(meta_filename, O_RDONLY);
                int err = this->file_fd;
                if (err < 0) {
                    throw Error(ERR_TRANSPORT_OPEN_FAILED);
                }
            }
        }

        ssize_t read(void * data, size_t len)
        {
            if (this->encryption){
                if (this->decrypt_state & CF_EOF) {
                    //printf("cf EOF\n");
                    return 0;
                }

                unsigned int requested_size = len;

                while (requested_size > 0) {
                    // Check how much we have decoded
                    if (!this->decrypt_raw_size) {
                        // Buffer is empty. Read a chunk from file
                        /*
                         i f (-1 == ::do_chunk_read*(this)) {
                             return -1;
                    }
                    */
                        // TODO: avoid reading size directly into an integer, performance enhancement is minimal
                        // and it's not portable because of endianness issue => read in a buffer and decode by hand
                        unsigned char tmp_buf[4] = {};
                        {
                            ssize_t err = this->file_read(tmp_buf, 4);
                            if (err < ssize_t(4)){
                                return (err < 0 ? err : -1);
                            }
                        }
                        uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);

                        if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                            this->decrypt_state |= CF_EOF;
                            this->decrypt_pos = 0;
                            this->decrypt_raw_size = 0;
                        }
                        else {
                            if (ciphered_buf_size > this->decrypt_MAX_CIPHERED_SIZE) {
                                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                                return -1;
                            }
                            else {
                                uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;
                                //char ciphered_buf[ciphered_buf_size];
                                unsigned char ciphered_buf[65536];
                                //char compressed_buf[compressed_buf_size];
                                unsigned char compressed_buf[65536];
                                {
                                    ssize_t err = this->file_read(ciphered_buf, ciphered_buf_size);
                                    if (err < ssize_t(ciphered_buf_size)){
                                        return (err < 0 ? err : -1);
                                    }
                                }

                                {
                                    int safe_size = compressed_buf_size;
                                    int remaining_size = 0;

                                    /* allows reusing of ectx for multiple encryption cycles */
                                    if (EVP_DecryptInit_ex(&this->decrypt_ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                                        return -1;
                                    }
                                    if (EVP_DecryptUpdate(&this->decrypt_ectx, compressed_buf, &safe_size, ciphered_buf, ciphered_buf_size) != 1){
                                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                                        return -1;
                                    }
                                    if (EVP_DecryptFinal_ex(&this->decrypt_ectx, compressed_buf + safe_size, &remaining_size) != 1){
                                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                                        return -1;
                                    }
                                    compressed_buf_size = safe_size + remaining_size;
                                }

                                size_t chunk_size = CRYPTO_BUFFER_SIZE;
                                const snappy_status status = snappy_uncompress(
                                        reinterpret_cast<char *>(compressed_buf),
                                        compressed_buf_size, this->decrypt_buf, &chunk_size);

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

                                this->decrypt_pos = 0;
                                // When reading, raw_size represent the current chunk size
                                this->decrypt_raw_size = chunk_size;
                            }
                        }

                        // TODO: check that
                        if (!this->decrypt_raw_size) { // end of file reached
                            break;
                        }
                    }
                    // remaining_size is the amount of data available in decoded buffer
                    unsigned int remaining_size = this->decrypt_raw_size - this->decrypt_pos;
                    // Check how much we can copy
                    unsigned int copiable_size = MIN(remaining_size, requested_size);
                    // Copy buffer to caller
                    ::memcpy(static_cast<char*>(data) + (len - requested_size), this->decrypt_buf + this->decrypt_pos, copiable_size);
                    this->decrypt_pos      += copiable_size;
                    requested_size -= copiable_size;
                    // Check if we reach the end
                    if (this->decrypt_raw_size == this->decrypt_pos) {
                        this->decrypt_raw_size = 0;
                    }
                }
                return len - requested_size;
            }
            else {
                return this->file_read(data, len);
            }
            return -1;
        }

        int file_close()
        {
            if (-1 != this->file_fd) {
                const int ret = ::close(this->file_fd);
                this->file_fd = -1;
                return ret;
            }
            return 0;
        }

    } buf_meta;

    struct rl_t {
        char buf[1024];
        char * eof;
        char * cur;
        rl_t() : eof(this->buf), cur(this->buf) {}
    } rl;

    unsigned meta_header_version;
    bool meta_header_has_checksum;

public:
    MetaLine meta_line;
    char meta_path[2048];
    int encryption;

    int buf_close()
    { return this->cfb.file_close(); }

public:
    ssize_t buf_reader_read_line(char * dest, size_t len, int err)
    {
        ssize_t total_read = 0;
        while (1) {
            char * pos = std::find(this->rl.cur, this->rl.eof, '\n');
            if (len < static_cast<size_t>(pos - this->rl.cur)) {
                total_read += len;
                memcpy(dest, this->rl.cur, len);
                this->rl.cur += len;
                break;
            }
            total_read += pos - this->rl.cur;
            memcpy(dest, this->rl.cur, pos - this->rl.cur);
            dest += pos - this->rl.cur;
            this->rl.cur = pos + 1;
            if (pos != this->rl.eof) {
                break;
            }

            ssize_t ret = this->buf_meta.read(this->rl.buf, sizeof(this->rl.buf));
            // TODO test on EINTR suspicious here, check that
            if (ret < 0 && errno != EINTR) {
                LOG(LOG_INFO, "InMetaSequenceTransport::ERR_TRANSPORT_READ_FAILED");
                return -ERR_TRANSPORT_READ_FAILED;
            }
            if (ret == 0) {
                return -err;
            }
            this->rl.eof = this->rl.buf + ret;
            this->rl.cur = this->rl.buf;
        }
        return total_read;
    }

    int buf_reader_next_line()
    {
        char * pos;
        while ((pos = std::find(this->rl.cur, this->rl.eof, '\n')) == this->rl.eof) {
            ssize_t ret = this->buf_meta.read(this->rl.buf, sizeof(this->rl.buf));
            // TODO test on EINTR suspicious here, check that
            if (ret < 0 && errno != EINTR) {
                return -ERR_TRANSPORT_READ_FAILED;
            }
            if (ret == 0) {
                return -ERR_TRANSPORT_READ_FAILED;
            }
            this->rl.eof = this->rl.buf + ret;
            this->rl.cur = this->rl.buf;
        }
        this->rl.cur = pos+1;
        return 0;
    }

public:
    ssize_t buf_read(void * data, size_t len)
    {
        if (!this->cfb.is_open()) {
            if (const int e1 = this->buf_next_line()) {
                this->status = false;
                throw Error(ERR_TRANSPORT_READ_FAILED, e1);
            }
            else {
                const int e2 = this->cfb.open(this->meta_line.filename);
                if (e2 < 0) {
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, e2);
                }
            }
        }

        ssize_t res = this->cfb.read(data, len);
        if (res < 0) {
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        if (size_t(res) != len) {
            ssize_t res2 = res;
            do {
                if (/*const ssize_t err = */this->buf_close()) {
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                data = static_cast<char*>(data) + res2;
                if (this->buf_next_line()) {
                    if (res < 0){
                        this->status = false;
                        throw Error(ERR_TRANSPORT_READ_FAILED, res);
                    }
                    return res;
                }
                else {
                    const int e = this->cfb.open(this->meta_line.filename);
                    if (e < 0) {
                        this->status = false;
                        throw Error(ERR_TRANSPORT_READ_FAILED, res);
                    }
                }
                len -= res2;
                res2 = this->cfb.read(data, len);
                if (res2 < 0) {
                    this->status = false;
                    throw Error(ERR_TRANSPORT_READ_FAILED, res);
                }
                res += res2;
            } while (size_t(res2) != len);
        }
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        return res;
    }

    int buf_read_meta_file_v1(MetaLine & meta_line) {
        char line[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + (1+MD_HASH::DIGEST_LENGTH*2) * 2 + 2];
        ssize_t len = this->buf_reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
        if (len < 0) {
            return -len;
        }
        line[len] = 0;

        // Line format "fffff sssss eeeee hhhhh HHHHH"
        //                               ^  ^  ^  ^
        //                               |  |  |  |
        //                               |hash1|  |
        //                               |     |  |
        //                           space3    |hash2
        //                                     |
        //                                   space4
        //
        // filename(1 or >) + space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

        // TODO Code below looks much too complicated for what it's doing

        typedef std::reverse_iterator<char*> reverse_iterator;

        using std::begin;

        reverse_iterator last(line);
        reverse_iterator first(line + len);
        reverse_iterator e1 = std::find(first, last, ' ');
        if (e1 - first == 64) {
            int err = 0;
            auto phash = begin(meta_line.hash2);
            for (char * b = e1.base(), * e = b + 64; e != b; ++b, ++phash) {
                *phash = (chex_to_int(*b, err) << 4);
                *phash |= chex_to_int(*++b, err);
            }
            REDASSERT(!err);
        }

        reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
        if (e2 - (e1 + 1) == 64) {
            int err = 0;
            auto phash = begin(meta_line.hash1);
            for (char * b = e2.base(), * e = b + 64; e != b; ++b, ++phash) {
                *phash = (chex_to_int(*b, err) << 4);
                *phash |= chex_to_int(*++b, err);
            }
            REDASSERT(!err);
        }

        if (e1 - first == 64 && e2 != last) {
            first = e2 + 1;
            e1 = std::find(first, last, ' ');
            e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
        }

        meta_line.stop_time = meta_parse_sec(e1.base(), first.base());
        if (e1 != last) {
            ++e1;
        }
        meta_line.start_time = meta_parse_sec(e2.base(), e1.base());

        if (e2 != last) {
            *e2 = 0;
        }

        auto path_len = std::min(int(e2.base() - line), PATH_MAX);
        memcpy(meta_line.filename, line, path_len);
        meta_line.filename[path_len] = 0;

        return 0;
    }

    char const * buf_sread_filename(char * p, char const * e, char const * pline)
    {
        e -= 1;
        for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
            *p = *pline;
        }
        *p = 0;
        return pline;
    }

    template<bool read_start_stop_time>
    int buf_read_meta_file_v2_impl(bool has_checksum, MetaLine & meta_line) {
        char line[
            PATH_MAX + 1 + 1 +
            (std::numeric_limits<long long>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<unsigned long long>::digits10 + 1 + 1) * 2 +
            (1 + MD_HASH::DIGEST_LENGTH*2) * 2 +
            2
        ];
        ssize_t len = this->buf_reader_read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
        if (len < 0) {
            return -len;
        }
        line[len] = 0;

        // Line format "fffff
        // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
        // sssss eeeee hhhhh HHHHH"
        //            ^  ^  ^  ^
        //            |  |  |  |
        //            |hash1|  |
        //            |     |  |
        //        space3    |hash2
        //                  |
        //                space4
        //
        // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
        //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
        //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

        auto pline = line + (this->buf_sread_filename(std::begin(meta_line.filename), std::end(meta_line.filename), line) - line);

        LOG(LOG_INFO, "meta_line.filename=%s", meta_line.filename);

        int err = 0;
        auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
        err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);
        if (read_start_stop_time) {
            err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
            err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);
        }

        // TODO Why do this with lambda ? Is it so important to avoid typing 3 lines of code two times ?
        if (has_checksum){
            err |= len - (pend - line) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 2;
            if (!err)
            {
                {
                    auto phash = std::begin(meta_line.hash1);
                    for (auto e = ++pend + sizeof(meta_line.hash1) * 2u; pend != e; ++pend, ++phash) {
                        *phash = (chex_to_int(*pend, err) << 4);
                        *phash |= chex_to_int(*++pend, err);
                    }
                }
                err |= (*pend != ' ');
                {
                    auto phash = std::begin(meta_line.hash2);
                    for (auto e = ++pend + sizeof(meta_line.hash2) * 2u; pend != e; ++pend, ++phash) {
                        *phash = (chex_to_int(*pend, err) << 4);
                        *phash |= chex_to_int(*++pend, err);
                    }
                }
            }
        }
        err |= bool(*pend);

        if (err) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        return 0;
    }

    int buf_read_meta_file_v2(MetaLine & meta_line) {
        return buf_read_meta_file_v2_impl<true>(this->meta_header_has_checksum, meta_line);
    }

    int buf_read_meta_file(MetaLine & meta_line)
    {
        if (this->meta_header_version == 1) {
            return this->buf_read_meta_file_v1(meta_line);
        }
        else {
            return this->buf_read_meta_file_v2(meta_line);
        }
    }

private:
    int buf_open_next() {
        if (const int e = this->buf_reader_next_line()) {
            return e < 0 ? e : -1;
        }
        const int e = this->cfb.open(this->meta_line.filename);
        return (e < 0) ? e : 0;
    }

    int buf_next_line()
    {
        if (auto err = this->buf_read_meta_file(this->meta_line)) {
            return err;
        }

        if (!file_exist(this->meta_line.filename)) {
            char original_path[1024] = {};
            char basename[1024] = {};
            char extension[256] = {};
            char filename[2048] = {};

            canonical_path( this->meta_line.filename
                          , original_path, sizeof(original_path)
                          , basename, sizeof(basename)
                          , extension, sizeof(extension));
            std::snprintf(filename, sizeof(filename), "%s%s%s", this->meta_path, basename, extension);

            if (file_exist(filename)) {
                strcpy(this->meta_line.filename, filename);
            }
        }

        return 0;
    }

public:
    InMetaSequenceTransport(
        CryptoContext * cctx,
        const char * filename,
        const char * extension,
        int encryption)
    : cfb(cctx, encryption)
    , buf_meta(cctx, encryption)
    , meta_header_version(1)
    , meta_header_has_checksum(false)
    , encryption(encryption)
    {
        LOG(LOG_INFO, "InMetaSequenceTransport::constructeur");
        assert(encryption ? bool(cctx) : true);

        temporary_concat tmp(filename, extension);
        const char * meta_filename = tmp.c_str();
        this->buf_meta.open(meta_filename);

        char line[32];
        auto sz = this->buf_reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
        if (sz < 0) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        // v2
        if (line[0] == 'v') {
            if (this->buf_reader_next_line()
             || (sz = this->buf_reader_read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED)) < 0
            ) {
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            this->meta_header_version = 2;
            this->meta_header_has_checksum = (line[0] == 'c');
        }
        // else v1

        if (this->buf_reader_next_line()) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        if (this->buf_reader_next_line()) {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        this->meta_line.start_time = 0;
        this->meta_line.stop_time = 0;

        this->meta_path[0] = 0;

        char basename[1024] = {};
        char extension2[256] = {};

        canonical_path( meta_filename
                      , this->meta_path, sizeof(this->meta_path)
                      , basename, sizeof(basename)
                      , extension2, sizeof(extension2));
    }

    ~InMetaSequenceTransport(){
        this->cfb.file_close();
        this->buf_meta.file_close();
    }


    bool disconnect() override {
        return !this->buf_close();
    }

    time_t begin_chunk_time() const noexcept
    {
        return this->meta_line.start_time;
    }

    time_t end_chunk_time() const noexcept
    {
        return this->meta_line.stop_time;
    }

    const char * path() const noexcept
    { return this->meta_line.filename; }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        if (this->cfb.is_open()) {
            this->buf_close();
        }

        const ssize_t res = this->buf_next_line();
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        ++this->seqno;
        return true;
    }

//     void do_recv(uint8_t ** pbuffer, size_t len) override {
//         const ssize_t res = this->buf_read(*pbuffer, len);
//         if (res < 0){
//             this->status = false;
//             throw Error(ERR_TRANSPORT_READ_FAILED, res);
//         }
//         *pbuffer += res;
//         this->last_quantum_received += res;
//         if (static_cast<size_t>(res) != len){
//             this->status = false;
//             throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
//         }
//     }

    void do_recv_new(uint8_t * buffer, size_t len) override {

        const ssize_t res = this->buf_read(buffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }

        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    bool do_atomic_read(uint8_t * buffer, size_t len) override {

        const ssize_t res = this->buf_read(buffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }

        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            if (res == 0){
                return false;
            }
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        return true;
    }

};

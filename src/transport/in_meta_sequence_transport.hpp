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

#ifndef REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_IN_META_SEQUENCE_TRANSPORT_HPP

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include "openssl_crypto.hpp"
#include "log.hpp"
#include "transport/detail/meta_opener.hpp"
#include "transport/mixin_transport.hpp"
#include "transport/buffer/file_buf.hpp"

#include "transport/cryptofile.hpp"

#include "urandom_read.hpp"

namespace transfil {

    class decrypt_filter
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       state;                   // enum crypto_file_state
        unsigned int   MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

    public:
        decrypt_filter() = default;
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


namespace transbuf {

    class ifile_buf_meta
    {
        int fd;
    public:
        ifile_buf_meta() : fd(-1) {}
        
        ~ifile_buf_meta()
        {
            this->close();
        }

        int open(const char * filename)
        {
            this->close();
            this->fd = ::open(filename, O_RDONLY);
            return this->fd;
        }

        int open(const char * filename, mode_t /*mode*/)
        {
            TODO("see why mode is ignored even if it's provided as a parameter?");
            this->close();
            this->fd = ::open(filename, O_RDONLY);
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

        bool is_open() const noexcept
        { return -1 != this->fd; }

        ssize_t read(void * data, size_t len)
        {
            TODO("this is blocking read, add support for timeout reading");
            TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
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

        off64_t seek(off64_t offset, int whence) const
        { return lseek64(this->fd, offset, whence); }
    };

    class icrypto_filename_buf
    {
        transfil::decrypt_filter decrypt;
        CryptoContext * cctx;
        ifile_buf_meta file;

    public:
        explicit icrypto_filename_buf(CryptoContext * cctx)
        : cctx(cctx)
        {}

        int open(const char * filename, mode_t mode = 0600)
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

            this->cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == this->cctx->compute_hmac(trace_key, derivator)) {
                return -1;
            }

            int err = this->file.open(filename, mode);
            if (err < 0) {
                return err;
            }

            return this->decrypt.open(this->file, trace_key);
        }

        ssize_t read(void * data, size_t len)
        { return this->decrypt.read(this->file, data, len); }

        int close()
        { return this->file.close(); }

        bool is_open() const noexcept
        { return this->file.is_open(); }

        off64_t seek(off64_t offset, int whence) const
        { return this->file.seek(offset, whence); }
    };
}

struct InMetaSequenceTransport
: InputNextTransport<detail::in_meta_sequence_buf<
    detail::empty_ctor<transbuf::ifile_buf>,
    detail::empty_ctor<transbuf::ifile_buf_meta>
> >
{
    InMetaSequenceTransport(CryptoContext * cctx, const char * filename, const char * extension, uint32_t verbose = 0)
    : InMetaSequenceTransport::TransportType(
        detail::in_meta_sequence_buf_param<>(detail::temporary_concat(filename, extension).str, verbose))
    {
        this->verbose = verbose;
    }

    explicit InMetaSequenceTransport(CryptoContext * cctx, const char * filename, uint32_t verbose = 0)
    : InMetaSequenceTransport::TransportType(detail::in_meta_sequence_buf_param<>(filename, verbose))
    {
        this->verbose = verbose;
    }

    unsigned begin_chunk_time() const noexcept
    { return this->buffer().get_begin_chunk_time(); }

    unsigned end_chunk_time() const noexcept
    { return this->buffer().get_end_chunk_time(); }

    const char * path() const noexcept
    { return this->buffer().current_path(); }
};

struct CryptoInMetaSequenceTransport
: InputNextTransport<detail::in_meta_sequence_buf<
    transbuf::icrypto_filename_buf,
    transbuf::icrypto_filename_buf
> >
{
    CryptoInMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : CryptoInMetaSequenceTransport::TransportType(
        detail::in_meta_sequence_buf_param<CryptoContext*,CryptoContext*>(
            detail::temporary_concat(filename, extension).c_str(), 0, crypto_ctx, crypto_ctx))
    {}

    CryptoInMetaSequenceTransport(CryptoContext * crypto_ctx, const char * filename)
    : CryptoInMetaSequenceTransport::TransportType(
        detail::in_meta_sequence_buf_param<CryptoContext*,CryptoContext*>(filename, 0, crypto_ctx, crypto_ctx))
    {}

    time_t begin_chunk_time() const noexcept
    { return this->buffer().get_begin_chunk_time(); }

    time_t end_chunk_time() const noexcept
    { return this->buffer().get_end_chunk_time(); }

    const char * path() const noexcept
    { return this->buffer().current_path(); }
};

#endif

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

#ifndef REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include "openssl_crypto.hpp"

#include "transport/cryptofile.hpp"

#include "transport/detail/meta_writer.hpp"
#include "transport/mixin_transport.hpp"
#include "transport/buffer/file_buf.hpp"
#include "fdbuf.hpp"
#include "log.hpp"
#include "urandom_read.hpp"
#include "utils/genrandom.hpp"
#include "fileutils.hpp"
#include "transport/detail/meta_hash.hpp"
#include "transport/buffer/checksum_buf.hpp"
#include "transport/buffer/null_buf.hpp"
#include "urandom_read.hpp"


namespace transfil {

    class encrypt_filter
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        EVP_MD_CTX     hctx;                    // hash context
        EVP_MD_CTX     hctx4k;                  // hash context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       file_size;               // the current file size

    public:
        encrypt_filter() = default;
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



namespace detail
{
    struct ocrypto_filter
    : transfil::encrypt_filter
    {
        ocrypto_filter(CryptoContext &)
        {}

        template<class Buf>
        int open(Buf & buf, CryptoContext & cctx, char const * filename) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

//            printf("ocrypto_filter: cctx.get_derivator\n");
            cctx.get_derivator(filename, derivator, DERIVATOR_LENGTH);

            unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
            unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
            memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
            memcpy(tmp_derivation + DERIVATOR_LENGTH, cctx.get_crypto_key(), CRYPTO_KEY_LENGTH);
            SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated);
            memcpy(trace_key, derivated, HMAC_KEY_LENGTH);

            unsigned char iv[32];
            cctx.random(iv, 32);
            return transfil::encrypt_filter::open(buf, trace_key, &cctx, iv);
        }
    };
}

namespace detail
{
    template<class Buf, class Params>
    struct OutHashedMetaSequenceTransport
    : // FlushingTransport<
    RequestCleaningTransport<OutputNextTransport<CloseWrapper<Buf>, detail::GetCurrentPath>>
    // >
    {
        OutHashedMetaSequenceTransport(
            CryptoContext * crypto_ctx,
            const char * path,
            const char * hash_path,
            const char * basename,
            timeval now,
            uint16_t width,
            uint16_t height,
            const int groupid,
            auth_api * authentifier = nullptr,
            unsigned verbose = 0,
            FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
        : OutHashedMetaSequenceTransport::TransportType(Params(
            *crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            *crypto_ctx,
            verbose
        ))
        {
            this->verbose = verbose;

            if (authentifier) {
                this->set_authentifier(authentifier);
            }

            detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
        }

        void timestamp(timeval now) override {
            this->buffer().update_sec(now.tv_sec);
        }

        const FilenameGenerator * seqgen() const noexcept
        {
            return &(this->buffer().seqgen());
        }
    };


    class ochecksum_filter
    {
        transbuf::ochecksum_buf<transbuf::null_buf> sum_buf;

    public:
        ochecksum_filter(CryptoContext & cctx)
        : sum_buf(cctx.get_hmac_key())
        {}

        template<class Buf>
        int open(Buf &, CryptoContext &, char const * /*filename*/) {
            return this->sum_buf.open();
        }

        template<class Buf>
        int write(Buf & buf, const void * data, size_t len) {
            this->sum_buf.write(data, len);
            return buf.write(data, len);
        }

        template<class Buf>
        int close(Buf &, hash_type & hash, unsigned char (&)[MD_HASH_LENGTH]) {
            return this->sum_buf.close(hash);
        }
    };

    struct cctx_ofile_buf
    : transbuf::ofile_buf
    {
        cctx_ofile_buf(CryptoContext &)
        {}
    };

    struct cctx_ochecksum_file
    : transbuf::ochecksum_buf<transbuf::ofile_buf>
    {
        cctx_ochecksum_file(CryptoContext & cctx)
        : transbuf::ochecksum_buf<transbuf::ofile_buf>(cctx.get_hmac_key())
        {}
    };
}

#include "transport/buffer/file_buf.hpp"
#include "transport/filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace transbuf {
    class ocrypto_filename_buf
    {
        transfil::encrypt_filter encrypt;
        CryptoContext * cctx;
        ofile_buf file;

    public:
        explicit ocrypto_filename_buf(CryptoContext * cctx)
        : cctx(cctx)
        {}

        explicit ocrypto_filename_buf(CryptoContext & cctx)
        : cctx(&cctx)
        {}

        ~ocrypto_filename_buf()
        {
            if (this->is_open()) {
                this->close();
            }
        }

        int open(const char * filename, mode_t mode = 0600)
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

            cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            unsigned char tmp_derivation[DERIVATOR_LENGTH + CRYPTO_KEY_LENGTH] = {}; // derivator + masterkey
            unsigned char derivated[SHA256_DIGEST_LENGTH  + CRYPTO_KEY_LENGTH] = {}; // really should be MAX, but + will do
            memcpy(tmp_derivation, derivator, DERIVATOR_LENGTH);
            memcpy(tmp_derivation + DERIVATOR_LENGTH, cctx->get_crypto_key(), CRYPTO_KEY_LENGTH);
            SHA256(tmp_derivation, CRYPTO_KEY_LENGTH + DERIVATOR_LENGTH, derivated);
            memcpy(trace_key, derivated, HMAC_KEY_LENGTH);

            int err = this->file.open(filename, mode);
            if (err < 0) {
                return err;
            }

            unsigned char iv[32];
            this->cctx->random(iv, 32);
//            if (-1 == urandom_read(iv, 32)) {
//                LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
//                return -1;
//            }

            return this->encrypt.open(this->file, trace_key, this->cctx, iv);
        }

        ssize_t write(const void * data, size_t len)
        { return this->encrypt.write(this->file, data, len); }

        int close(unsigned char hash[MD_HASH_LENGTH << 1])
        {
            const int res1 = this->encrypt.close(this->file, hash, this->cctx->get_hmac_key());
            const int res2 = this->file.close();
            return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
        }

        int close()
        {
            unsigned char hash[MD_HASH_LENGTH << 1];
            return this->close(hash);
        }

        bool is_open() const noexcept
        { return this->file.is_open(); }

        off64_t seek(off64_t offset, int whence) const
        { return this->file.seek(offset, whence); }

        int flush() const
        { return this->file.flush(); }
    };
}


struct OutMetaSequenceTransport
: 
RequestCleaningTransport<
    OutputNextTransport<detail::out_meta_sequence_filename_buf<
        detail::empty_ctor<io::posix::fdbuf>,
        detail::empty_ctor<transbuf::ofile_buf>
    >, detail::GetCurrentPath>
>
{
    OutMetaSequenceTransport(
        CryptoContext * cctx,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : OutMetaSequenceTransport::TransportType(detail::out_meta_sequence_filename_buf_param<>(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid, verbose
    ))
    {
        this->verbose = verbose;

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, false);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
};

using OutMetaSequenceTransportWithSum = detail::OutHashedMetaSequenceTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        detail::ochecksum_filter,
        detail::cctx_ochecksum_file,
        detail::cctx_ofile_buf
    >,
    detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>
>;

using CryptoOutMetaSequenceTransport = detail::OutHashedMetaSequenceTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        detail::ocrypto_filter,
        transbuf::ocrypto_filename_buf,
        transbuf::ocrypto_filename_buf
    >,
    detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>
>;

#endif

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

#ifndef REDEMPTION_TRANSPORT_CRYPTO_OUT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_CRYPTO_OUT_META_SEQUENCE_TRANSPORT_HPP

#include "detail/meta_writer.hpp"
#include "buffer/crypto_filename_buf.hpp"
#include "mixin_transport.hpp"
#include "urandom_read.hpp"
#include "fileutils.hpp"

namespace detail {
    struct crypto_out_meta_sequence_filename_buf_param
    {
        out_meta_sequence_filename_buf_param<CryptoContext*> meta_sq_params;
        const char * hash_prefix;
        CryptoContext & cctx;
        uint32_t verbose;

        crypto_out_meta_sequence_filename_buf_param(
            CryptoContext & cctx,
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const hash_prefix,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            uint32_t verbose = 0)
        : meta_sq_params(start_sec, format, prefix, filename, extension, groupid, &cctx)
        , hash_prefix(hash_prefix)
        , cctx(cctx)
        , verbose(verbose)
        {}
    };

    template<class BufWrm, class BufMwrm>
    class crypto_meta_sequence_filename_buf
    : public out_meta_sequence_filename_buf<BufWrm, BufMwrm>
    {
        detail::MetaFilename hf_;
        CryptoContext & cctx;
        transfil::encrypt_filter encrypt_wrm;
        uint32_t verbose;

        typedef out_meta_sequence_filename_buf<BufWrm, BufMwrm> sequence_base_type;

    public:
        crypto_meta_sequence_filename_buf(crypto_out_meta_sequence_filename_buf_param const & params)
        : sequence_base_type(params.meta_sq_params)
        , hf_(params.hash_prefix, params.meta_sq_params.sq_params.filename, params.meta_sq_params.sq_params.format)
        , cctx(params.cctx)
        , verbose(params.verbose)
        {}

        ~crypto_meta_sequence_filename_buf()
        {
            this->close();
        }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        {
            if (!this->buf().is_open()) {
                const char * filename = this->get_filename_generate();
                const int res = this->open_filename(filename);
                if (res < 0) {
                    return res;
                }
                unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
                unsigned char derivator[DERIVATOR_LENGTH];
                get_derivator(filename, derivator, DERIVATOR_LENGTH);
                if (-1 == compute_hmac(trace_key, this->cctx.crypto_key, derivator)) {
                    return -1;
                }

                unsigned char iv[32];
                if (-1 == urandom_read(iv, 32)) {
                    LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
                    this->buf().close();
                    return -1;
                }

                this->encrypt_wrm.open(this->buf(), trace_key, &this->cctx, iv);
            }
            return this->encrypt_wrm.write(this->buf(), data, len);
        }

        int close() /*noexcept*/
        {
            if (this->buf().is_open()) {
                if (this->next()) {
                    return 1;
                }
            }

            if (!this->meta_buf().is_open()) {
                return 1;
            }

            char path[1024] = {};
            char basename[1024] = {};
            char extension[256] = {};
            char filename[2048] = {};

            canonical_path( hf_.filename, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension)
                          , this->verbose);
            snprintf(filename, sizeof(filename), "%s%s", basename, extension);

            unsigned char hash[HASH_LEN + 1] = {0};
            hash[0] = ' ';

            if (this->meta_buf().close(hash+1)) {
                return 1;
            }

            transbuf::ocrypto_filename_base crypto_hash(&this->cctx);
            if (crypto_hash.open(hf_.filename) >= 0) {
                const size_t len = strlen(filename);
                if (crypto_hash.write(filename, len) != long(len)
                 || crypto_hash.write(hash, HASH_LEN+1) != HASH_LEN+1
                 || crypto_hash.close(hash) != 0) {
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [%u]\n", hf_.filename, -HASH_LEN);
                    return 1;
                }

                if (chmod(hf_.filename, S_IRUSR|S_IRGRP) == -1){
                    LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", hf_.filename, strerror(errno), errno);
                }
            }
            else {
                int e = errno;
                LOG(LOG_ERR, "Open to transport failed: code=%d", e);
                errno = e;
                return 1;
            }

            return 0;
        }

        int next() /*noexcept*/
        {
            if (this->buf().is_open()) {
                unsigned char hash[HASH_LEN];
                {
                    const int res1 = this->encrypt_wrm.close(this->buf(), hash, this->cctx.hmac_key);
                    const int res2 = this->buf().close();
                    if (res1) {
                        return res1;
                    }
                    if (res2) {
                        return res2;
                    }
                }
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                const char * filename = this->rename_filename();
                if (!filename) {
                    return 1;
                }
                ssize_t len = strlen(filename);
                ssize_t res = this->meta_buf().write(filename, len);
                if (res == len) {
                    char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + HASH_LEN*2 + 2];
                    len = sprintf(mes, " %u %u", (unsigned)this->start_sec_, (unsigned)this->stop_sec_+1);
                    char * p = mes + len;
                    *p++ = ' ';                           //     1 octet
                    for (int i = 0; i < HASH_LEN / 2; i++, p += 2) {
                        sprintf(p, "%02x", hash[i]); //    64 octets (hash1)
                    }
                    *p++ = ' ';                           //     1 octet
                    for (int i = HASH_LEN / 2; i < HASH_LEN; i++, p += 2) {
                        sprintf(p, "%02x", hash[i]); //    64 octets (hash2)
                    }
                    *p++ = '\n';
                    len = p-mes;
                    res = this->meta_buf().write(mes, len);
                }
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                this->start_sec_ = this->stop_sec_;
                return 0;
            }
            return 1;
        }
    };
}

struct CryptoOutMetaSequenceTransport
: //SeekableTransport<
RequestCleaningTransport<
    OutputNextTransport<detail::crypto_meta_sequence_filename_buf<
        detail::empty_ctor<io::posix::fdbuf>,
        transbuf::ocrypto_filename_base
    >, detail::GetCurrentPath>
>
// >
{
    CryptoOutMetaSequenceTransport(
        CryptoContext * crypto_ctx,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = NULL,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : CryptoOutMetaSequenceTransport::TransportType(
        detail::crypto_out_meta_sequence_filename_buf_param(
            *crypto_ctx,
            now.tv_sec,
            format, hash_path, path, basename, ".wrm", groupid, verbose))
    {
        this->verbose = verbose;

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier);
    }

    virtual void timestamp(timeval now) /*noexcept*/
    {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    {
        return &(this->buffer().seqgen());
    }
};

#endif

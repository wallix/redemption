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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_CRYPTO_META_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_CRYPTO_META_TRANSPORT_HPP

#include "meta_transport.hpp"
#include "buffer/crypto_filename_buf.hpp"
#include <fileutils.hpp>

struct crypto_out_meta_nexter_params
{
    CryptoContext * crypto_ctx;
    const char * hash_path;
    const char * basename;
    time_t start_sec;
    FilenameFormat format;

    crypto_out_meta_nexter_params(
        CryptoContext * crypto_ctx,
        time_t start_sec,
        const char * hash_path,
        const char * basename,
        FilenameFormat format
    )
    : crypto_ctx(crypto_ctx)
    , hash_path(hash_path)
    , basename(basename)
    , start_sec(start_sec)
    , format(format)
    {}
};

class crypto_out_meta_nexter
: public transbuf::ocrypto_filename_base
{
    detail::MetaFilename hf;
    time_t start_sec;
    time_t stop_sec;

public:
    crypto_out_meta_nexter(crypto_out_meta_nexter_params const & params) /*noexcept*/
    : transbuf::ocrypto_filename_base(params.crypto_ctx)
    , hf(params.hash_path, params.basename, params.format)
    , start_sec(params.start_sec)
    , stop_sec(params.start_sec)
    {}

    template<class Transport, class TransportBuf>
    int next(Transport & /*trans*/, TransportBuf & buf) /*noexcept*/
    {
        if (buf.is_open()) {
            unsigned char hash[HASH_LEN];
            buf.close(hash);
            const char * filename = buf.impl().seqgen().get(buf.impl().seqnum());
            size_t len = strlen(filename);
            ssize_t res = this->write(filename, len);
            if (res >= 0 && size_t(res) == len) {
                char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + HASH_LEN*2 + 5];
                len = snprintf(mes, sizeof(mes) - 3, " %u %u",
                               (unsigned)this->start_sec,
                               (unsigned)this->stop_sec+1);
                char * p = mes + len;
                *p++ = ' ';                           //     1 octet
                for (int i = 0; i < HASH_LEN / 2; i++, p += 2) {
                    sprintf(p, "%02x", hash[i]);      //    64 octets (hash1)
                }
                *p++ = ' ';                           //     1 octet
                for (int i = HASH_LEN / 2; i < HASH_LEN; i++, p += 2) {
                    sprintf(p, "%02x", hash[i]);      //    64 octets (hash2)
                }
                *p++ = '\n';
                res = this->write(mes, p-mes);
                this->start_sec = this->stop_sec;
            }
            if (res < 0) {
                int err = errno;
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
                return res;
            }
            return 0;
        }
        return 1;
    }

    template<class TransportBuf>
    bool next_end(TransportBuf & buf) /*noexcept*/
    {
        if (buf.is_open()) {
            if (this->next(buf, buf)) {
                return false;
            }
        }

        if (!this->is_open()) {
            return false;
        }

        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};
        char filename[2048] = {};

        canonical_path(hf.filename,
                       path, sizeof(path),
                       basename, sizeof(basename),
                       extension, sizeof(extension));
        std::snprintf(filename, sizeof(filename), "%s%s", basename, extension);

        unsigned char hash[HASH_LEN + 1] = {0};
        hash[0] = ' ';

        if (this->close(hash+1) < 0) {
            return false;
        }

        transbuf::ocrypto_filename_base crypto_hash(this->crypto_context());
        TODO("check errors when storing hash");
        if (crypto_hash.open(hf.filename) >= 0) {
            const size_t len = strlen(filename);
            if (crypto_hash.write(filename, len) != long(len)
             || crypto_hash.write(hash, HASH_LEN+1) != HASH_LEN+1) {
                LOG(LOG_ERR, "Failed writing signature to hash file %s [%u]\n", hf.filename, -HASH_LEN);
                return false;
            }
            else {
                crypto_hash.close(hash);
            }

            if (chmod(hf.filename, S_IRUSR|S_IRGRP) == -1){
                LOG(LOG_ERR, "can't set file %s mod to u+r, g+r : %s [%u]", hf.filename, strerror(errno), errno);
            }
        }
        else {
            LOG(LOG_ERR, "Open to transport failed: code=%d", errno);
            return false;
        }

        return true;
    }

    void update_sec(time_t sec) /*noexcept*/
    { this->stop_sec = sec; }
};

namespace detail {
    struct crypto_out_meta_buf_base_params
    {
        CryptoContext & ctx;
        FilenameSequencePolicyParams fsq_params;

        crypto_out_meta_buf_base_params(CryptoContext & ctx, const FilenameSequencePolicyParams & fsq_params)
        : ctx(ctx)
        , fsq_params(fsq_params)
        {}
    };

    class crypto_out_meta_buf_base
    {
        transfil::encrypt_filter encrypt;
        CryptoContext ctx;
        io::posix::fdbuf file;
        FilenameSequencePolicy fsq;

    public:
        crypto_out_meta_buf_base(const crypto_out_meta_buf_base_params & params)
        : ctx(params.ctx)
        , fsq(params.fsq_params)
        {}

        int open(int fd) /*noexcept*/
        {
            int e = this->file.open(fd);
            if (e < 0) {
                return e;
            }

            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            const char * filename = this->fsq.seqgen().get(this->fsq.seqnum()+1);
            unsigned char derivator[DERIVATOR_LENGTH];
            get_derivator(filename, derivator, DERIVATOR_LENGTH);
            const int err = compute_hmac(trace_key, this->ctx.crypto_key, derivator);
            if (err < 0) {
                return err;
            }

            unsigned char iv[32];
            if (-1 == urandom_read(iv, 32)) {
                LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
                return -1;
            }

            return this->encrypt.open(this->file, trace_key, &this->ctx, iv);
        }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        {
            if (!this->is_open()) {
                const int res = fsq.init(*this);
                if (res < 0) {
                    return res;
                }
            }
            return this->encrypt.write(this->file, data, len);
        }

        int close(unsigned char hash[MD_HASH_LENGTH << 1]) /*noexcept*/
        {
            const int res1 = this->encrypt.close(this->file, hash, this->ctx.hmac_key);
            const int res2 = this->fsq.close(this->file);
            return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
        }

        int close() /*noexcept*/
        {
            unsigned char hash[MD_HASH_LENGTH << 1];
            return this->close(hash);
        }

        bool is_open() const /*noexcept*/
        { return this->file.is_open(); }

        const FilenameSequencePolicy & impl() const /*noexcept*/
        { return this->fsq; }

        FilenameSequencePolicy & impl() /*noexcept*/
        { return this->fsq; }
    };
}

struct CryptoOutMetaTransport
: OutBufferTransport<
    detail::crypto_out_meta_buf_base,
    crypto_out_meta_nexter
>
{
    CryptoOutMetaTransport(CryptoContext * crypto_ctx,
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
    : CryptoOutMetaTransport::TransportType(
        detail::crypto_out_meta_buf_base_params(
            *crypto_ctx,
            detail::FilenameSequencePolicyParams(format, path, basename, ".wrm", groupid)),
        crypto_out_meta_nexter_params(crypto_ctx, now.tv_sec, hash_path, basename, format)
    )
    {
        (void)verbose;

        detail::MetaFilename mf(path, basename, format);

        if (this->nexter().open(mf.filename, S_IRUSR) < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->nexter(), path, width, height, this->authentifier);
    }

    virtual void timestamp(timeval now)
    {
        this->update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    { return &this->impl().seqgen(); }

    virtual void request_full_cleaning()
    {
        this->impl().request_full_cleaning();
    }
};


struct CryptoInMetaTransport
: InBufferTransport<
    transbuf::reopen_input<transbuf::icrypto_filename_base, detail::MetaOpener<transbuf::icrypto_filename_base> >,
    in_meta_nexter
>
{
    CryptoInMetaTransport(CryptoContext * crypto_ctx, const char * filename, const char * extension)
    : CryptoInMetaTransport::TransportType(transbuf::buf_params(
        crypto_ctx, transbuf::buf_params(crypto_ctx, detail::temporary_concat(filename, extension).c_str())))
    {}

    unsigned begin_chunk_time() const /*noexcept*/
    { return this->impl().get_begin_chunk_time(); }

    unsigned end_chunk_time() const /*noexcept*/
    { return this->impl().get_end_chunk_time(); }

    const char * path() const /*noexcept*/
    { return this->impl().get_path(); }

    unsigned get_seqno() const /*noexcept*/
    { return this->impl().get_seqno(); }
};

#endif

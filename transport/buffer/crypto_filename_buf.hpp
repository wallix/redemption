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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_CRYPTO_FILENAME_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_CRYPTO_FILENAME_BUF_HPP

#include "file_buf.hpp"
#include "filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace transbuf {
    namespace detail {
        template<class Buf>
        int init_trace_key(Buf & buf, CryptoContext * ctx, const char * filename, mode_t mode, unsigned char * trace_key) /*noexcept*/
        {
            unsigned char derivator[DERIVATOR_LENGTH];
            get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == compute_hmac(trace_key, ctx->crypto_key, derivator)) {
                return -1;
            }

            return buf.open(filename, mode);
        }
    }

    class icrypto_filename_base
    {
        transfil::decrypt_filter decrypt;
        CryptoContext * ctx;
        ifile_base file;

    public:
        icrypto_filename_base(CryptoContext * ctx)
        : ctx(ctx)
        {}

        int open(const char * filename, mode_t mode = 0600) /*noexcept*/
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            const int err = detail::init_trace_key(this->file, this->ctx, filename, mode, trace_key);
            if (err < 0) {
                return err;
            }

            return this->decrypt.open(this->file, trace_key);
        }

        ssize_t read(void * data, size_t len) /*noexcept*/
        { return this->decrypt.read(this->file, data, len); }

        int close() /*noexcept*/
        { return this->file.close(); }

        bool is_open() const /*noexcept*/
        { return this->file.is_open(); }

    protected:
        CryptoContext * crypto_context() const /*noexcept*/
        { return this->ctx; }

        void crypto_context(CryptoContext * ctx) /*noexcept*/
        { this->ctx = ctx; }
    };

    class ocrypto_filename_base
    {
        transfil::encrypt_filter encrypt;
        CryptoContext * ctx;
        ofile_base file;

    public:
        ocrypto_filename_base(CryptoContext * ctx)
        : ctx(ctx)
        {}

        int open(const char * filename, mode_t mode = 0600) /*noexcept*/
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            const int err = detail::init_trace_key(this->file, this->ctx, filename, mode, trace_key);
            if (err < 0) {
                return err;
            }

            unsigned char iv[32];
            if (-1 == urandom_read(iv, 32)) {
                LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
                return -1;
            }

            return this->encrypt.open(this->file, trace_key, this->ctx, iv);
        }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        { return this->encrypt.write(this->file, data, len); }

        int close(unsigned char hash[MD_HASH_LENGTH << 1]) /*noexcept*/
        {
            const int res1 = this->encrypt.close(this->file, hash, this->ctx->hmac_key);
            const int res2 = this->file.close();
            return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
        }

        int close() /*noexcept*/
        {
            unsigned char hash[MD_HASH_LENGTH << 1];
            return this->close(hash);
        }

        bool is_open() const /*noexcept*/
        { return this->file.is_open(); }

    protected:
        CryptoContext * crypto_context() const /*noexcept*/
        { return this->ctx; }

        void crypto_context(CryptoContext * ctx) /*noexcept*/
        { this->ctx = ctx; }
    };
}

#endif

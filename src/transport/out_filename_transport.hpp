/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_OUT_FILENAME_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_OUT_FILENAME_TRANSPORT_HPP

// #include "buffer/buffering_buf.hpp"
#include "transport/mixin_transport.hpp"
#include "transport/buffer/file_buf.hpp"
#include "transport/buffer/file_buf.hpp"
#include "transport/filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace transbuf {
    class ocrypto_filename_buf2
    {
        transfil::encrypt_filter encrypt;
        CryptoContext * cctx;
        ofile_buf file;

    public:
        explicit ocrypto_filename_buf2(CryptoContext * cctx)
        : cctx(cctx)
        {}

        explicit ocrypto_filename_buf2(CryptoContext & cctx)
        : cctx(&cctx)
        {}

        ~ocrypto_filename_buf2()
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
            if (-1 == this->cctx->compute_hmac(trace_key, derivator)) {
                return -1;
            }

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

struct OutFilenameTransport
: SeekableTransport<OutputTransport<transbuf::ofile_buf>>
{
    OutFilenameTransport(const char * filename)
    {
        if (this->buffer().open(filename, 0600) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};

struct CryptoOutFilenameTransport
: OutputTransport<transbuf::ocrypto_filename_buf2>
{
    CryptoOutFilenameTransport(CryptoContext * crypto_ctx, const char * filename, auth_api * authentifier = nullptr)
    : CryptoOutFilenameTransport::TransportType(crypto_ctx)
    {
        if (this->buffer().open(filename, 0600) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }
};

#endif

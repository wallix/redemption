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

// #include "buffer/buffering_buf.hpp"
#include "transport/buffer/crypto_filename_buf.hpp"
#include "transport/buffer/file_buf.hpp"
#include "transport/mixin_transport.hpp"

struct InFilenameTransport
: SeekableTransport< InputTransport< /*transbuf::ibuffering_buf<*/transbuf::ifile_buf/*>*/ > >
{
    InFilenameTransport(const char * filename)
    {
        if (this->buffer().open(filename, 0600) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};

namespace transbuf {
    class icrypto_filename_buf2
    {
        transfil::decrypt_filter decrypt;
        CryptoContext * cctx;
        ifile_buf file;

    public:
        explicit icrypto_filename_buf2(CryptoContext * cctx)
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


struct CryptoInFilenameTransport
: InputTransport<transbuf::icrypto_filename_buf2>
{
    CryptoInFilenameTransport(CryptoContext * crypto_ctx, const char * filename)
    : CryptoInFilenameTransport::TransportType(crypto_ctx)
    {
        if (this->buffer().open(filename, 0600) < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};

#endif

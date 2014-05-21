/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_INFILENAMETRANSPORT_HPP
#define REDEMPTION_TRANSPORT_INFILENAMETRANSPORT_HPP

#include "transport.hpp"
#include "rio/cryptofile.hpp"

/****************************
* CryptoInFilenameTransport *
****************************/

class CryptoInFilenameTransport
: public Transport
{
    CryptoContext * crypto_ctx;
    crypto_file cf_struct;
    uint32_t verbose;

public:
    CryptoInFilenameTransport(CryptoContext * crypto_ctx, const char * filename, unsigned verbose = 0)
    : verbose(verbose)
    , crypto_ctx(crypto_ctx)
    {
        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(filename, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (compute_hmac(trace_key, this->crypto_ctx->crypto_key, derivator) == -1){
            throw Error(ERR_TRANSPORT);
        }

        LOG("crypto open read... filename=%s\n", filename);

        int system_fd = open(filename, O_RDONLY, 0600);
        if (system_fd == -1){
            LOG("failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT);
        }

        if (-1 == cf_struct.open_read_init(system_fd, trace_key, this->crypto_ctx)) {
            close(system_fd);
            LOG("open failed for crypto filename=%s\n", filename);
            throw Error(ERR_TRANSPORT);
        }
    }

    virtual ~CryptoInFilenameTransport()
    {
        unsigned char hash[HASH_LEN];
        this->cf_struct.close(hash, this->crypto_ctx->hmac_key);
    }

    using Transport::recv;
    virtual void srecv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = this->cf_struct.read(*pbuffer, len);
        if (res == -1) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        else if (res <= 0){
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        *pbuffer += res;
        if (res != (ssize_t)len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }
};

#endif

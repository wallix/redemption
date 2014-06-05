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
#include "crypto_transport.hpp"

/****************************
* CryptoInFilenameTransport *
****************************/

class CryptoInFilenameTransport
: public Transport
{
    CryptoContext & crypto_ctx;
    crypto_file cf;

public:
    CryptoInFilenameTransport(CryptoContext * crypto_ctx, const char * filename, unsigned verbose = 0)
    : crypto_ctx(*crypto_ctx)
    {
        int system_fd = open(filename, O_RDONLY, 0600);
        if (system_fd == -1){
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        init_crypto_read(this->cf, this->crypto_ctx, system_fd, filename, ERR_TRANSPORT_OPEN_FAILED);
    }

    virtual ~CryptoInFilenameTransport()
    {
        unsigned char hash[HASH_LEN];
        this->cf.close(hash, this->crypto_ctx.hmac_key);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = this->cf.read(*pbuffer, len);
        if (res <= 0) {
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        *pbuffer += res;
        if (res != (ssize_t)len) {
            this->status = false;
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

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

   Transport layer abstraction, outfile implementation
*/

#ifndef _REDEMPTION_TRANSPORT_OUTFILETRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_OUTFILETRANSPORT_HPP_

#include "transport.hpp"
#include "rio/rio.h"

class OutFileTransport : public Transport {
    public:
    RIO rio;
    uint32_t verbose;

    OutFileTransport(int fd, unsigned verbose = 0)
        : verbose(verbose) 
    {
        RIO_ERROR status = rio_init_outfile(&this->rio, fd);
        if (status != RIO_ERROR_OK){
            LOG(LOG_ERR, "rio outfile initialisation failed (%u)", status);
            throw Error(ERR_TRANSPORT);
        }
    }

    virtual ~OutFileTransport() 
    {
        rio_clear(&this->rio);
    }


    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {  
        LOG(LOG_INFO, "OutFileTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }
    
    virtual void seek(int64_t offset, int whence) throw (Error) { throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE); }
};

#endif

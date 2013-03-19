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

#ifndef _REDEMPTION_CORE_OUTMETATRANSPORT_HPP_
#define _REDEMPTION_CORE_OUTMETATRANSPORT_HPP_

#include "transport.hpp"
#include "error.hpp"

#include "../libs/rio.h"

class OutmetaTransport : public Transport {
public:
    timeval now;
    FileSequence sequence;
    char meta_path[1024];
    char path[1024];

    RIO * rio;
    SQ * seq;

    OutmetaTransport(const char * path, const char * basename, 
                      timeval now, uint16_t width, uint16_t height, FileSequence ** pwrm_sequence, unsigned verbose = 0)
    : now(now)
    , sequence("path file pid count extension", path, basename, ".wrm")
    , rio(NULL)
    , seq(NULL)
    {
        *pwrm_sequence = &this->sequence;
        RIO_ERROR status = RIO_ERROR_OK;
        char filename[1024];
        sprintf(filename, "%s%s-%06u", path, basename, getpid());
        char header1[1024];
        sprintf(header1, "%u %u", width, height);
        this->rio = rio_new_outmeta(&status, &this->seq, filename, ".mwrm", header1, "0", "", &now);
        if (status < 0){
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    ~OutmetaTransport()
    {
        rio_delete(this->rio);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t res = rio_send(this->rio, buffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    virtual void timestamp(timeval now)
    {
        this->future = now;
        sq_timestamp(this->seq, &now);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {  
        LOG(LOG_INFO, "OutFileTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual bool next()
    {
        sq_next(this->seq);
        return true;
    }
};

#endif

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

#ifndef _REDEMPTION_CORE_INBYMETASEQUENCETRANSPORT_HPP_
#define _REDEMPTION_CORE_INBYMETASEQUENCETRANSPORT_HPP_

#include"transport.hpp"
#include"error.hpp"

class InByMetaSequenceTransport : public Transport {
public:
    char path[1024];
    unsigned begin_chunk_time;
    unsigned end_chunk_time;
    unsigned chunk_num;

    RIO * rio;
    SQ * seq;

    InByMetaSequenceTransport(const char * filename, const char * extension)
    : Transport()
    {
        memset(this->path, 0, sizeof(path));
        this->begin_chunk_time = 0;
        this->end_chunk_time = 0;
        this->chunk_num = 0;

        RIO_ERROR status = RIO_ERROR_OK;
        SQ * seq = NULL;
        this->rio = rio_new_inmeta(&status, &seq, filename, extension);
        if (status != RIO_ERROR_OK){
            throw Error(ERR_TRANSPORT);
        }
        this->seq = seq;
    }

    ~InByMetaSequenceTransport()
    {
        if (this->rio){
            rio_delete(this->rio);
        }
    }

    void next_chunk_info()
    {
        {
            timeval tv_begin = {};
            timeval tv_end = {};
            RIO_ERROR status = sq_get_chunk_info(this->seq, &this->chunk_num, this->path, sizeof(this->path), &tv_begin, &tv_end);
            if (status != RIO_ERROR_OK){
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            this->begin_chunk_time = tv_begin.tv_sec;
            this->end_chunk_time = tv_end.tv_sec;
        }
        // if some error occurs calling sq_next 
        // it will be took care of when opening next chunk, not now
        sq_next(this->seq);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        ssize_t res = rio_recv(this->rio, *pbuffer, len);
        if (res <= 0){
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
};

#endif

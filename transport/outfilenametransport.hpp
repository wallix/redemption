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

#ifndef _REDEMPTION_TRANSPORT_OUTFILENAMETRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_OUTFILENAMETRANSPORT_HPP_

#include "transport.hpp"
#include "rio/rio.h"

class OutFilenameTransport : public Transport {
public:
    SQ seq;
    RIO rio;

    OutFilenameTransport(
            SQ_FORMAT format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            unsigned verbose = 0)
    {
        RIO_ERROR status1 = sq_init_outfilename(&this->seq, format, prefix, filename, extension, groupid);
        if (status1 != RIO_ERROR_OK){
            LOG(LOG_ERR, "Sequence outfilename initialisation failed (%u)", status1);
            throw Error(ERR_TRANSPORT);
        }
        RIO_ERROR status2 = rio_init_outsequence(&this->rio, &this->seq);
        if (status2 != RIO_ERROR_OK){
            LOG(LOG_ERR, "rio outsequence initialisation failed (%u)", status2);
            throw Error(ERR_TRANSPORT);
        }
    }

    ~OutFilenameTransport()
    {
        rio_clear(&this->rio);
        sq_clear(&this->seq);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    virtual void timestamp(timeval now)
    {
        sq_timestamp(&this->seq, &now);
        Transport::timestamp(now);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {  
        LOG(LOG_INFO, "OutFilenameTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error)
    {
        print("outfilename seek offset=%u whence=%u\n", (unsigned)offset, (unsigned)whence);
        ssize_t res = rio_seek(&this->rio, offset, whence);
        print("outfilename seek res=%u\n", (unsigned)res);
        if (res != RIO_ERROR_OK){
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }    
    }

    virtual bool next()
    {
        sq_next(&this->seq);
        return Transport::next();
    }
};



/*****************************
* CryptoOutFilenameTransport *
*****************************/

class CryptoOutFilenameTransport : public Transport {
public:
    SQ seq;
    RIO rio;

    CryptoOutFilenameTransport(
            SQ_FORMAT format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            unsigned verbose = 0)
    {
        RIO_ERROR status1 = sq_init_cryptooutfilename(&this->seq, format, prefix, filename, extension, groupid);
        if (status1 != RIO_ERROR_OK){
            LOG(LOG_ERR, "Sequence outfilename initialisation failed (%u)", status1);
            throw Error(ERR_TRANSPORT);
        }
        RIO_ERROR status2 = rio_init_outsequence(&this->rio, &this->seq);
        if (status2 != RIO_ERROR_OK){
            LOG(LOG_ERR, "rio outsequence initialisation failed (%u)", status2);
            throw Error(ERR_TRANSPORT);
        }
    }

    ~CryptoOutFilenameTransport()
    {
        rio_clear(&this->rio);
        sq_clear(&this->seq);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    virtual void timestamp(timeval now)
    {
        sq_timestamp(&this->seq, &now);
        Transport::timestamp(now);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw (Error)
    {  
        LOG(LOG_INFO, "OutFilenameTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
         return rio_seek(&this->rio, offset, whence);
    }

    virtual bool next()
    {
        sq_next(&this->seq);
        return Transport::next();
    }
};

#endif

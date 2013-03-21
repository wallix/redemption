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

#ifndef _REDEMPTION_CORE_OUTFILENAMETRANSPORT_HPP_
#define _REDEMPTION_CORE_OUTFILENAMETRANSPORT_HPP_

#include "transport.hpp"
#include "../libs/rio.h"

class OutFilenameTransport : public OutFileTransport {
public:
    SQ seq;
    char path[1024];

    OutFilenameTransport(
            SQ_FORMAT format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    {
        RIO_ERROR status = sq_init_outfilename(&this->seq, format, prefix, filename, extension);
        if (status != RIO_ERROR_OK){
            LOG(LOG_ERR, "Sequence outfilename initialisation failed (%u)", status);
            throw Error(ERR_TRANSPORT);
        }
    }

    ~OutFilenameTransport()
    {
        sq_clear(&this->seq);
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            sq_im_SQOutfilename_get_name(&(this->seq.u.outfilename), this->path, sizeof(this->path), this->seqno);
            this->fd = ::creat(this->path, 0777);
            if (this->fd == -1){
                LOG(LOG_INFO, "OutByFilename transport write failed with error : %s", strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        OutFileTransport::send(buffer, len);
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        this->OutFileTransport::next();
        return true;
    }
};


#endif

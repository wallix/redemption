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

#ifndef _REDEMPTION_CORE_OUTBYFILENAMESEQUENCETRANSPORT_HPP_
#define _REDEMPTION_CORE_OUTBYFILENAMESEQUENCETRANSPORT_HPP_

#include "transport.hpp"
#include "../libs/rio.h"

class OutByFilenameSequenceTransport : public OutFileTransport {
public:
    const FileSequence & sequence;
    char path[1024];

    OutByFilenameSequenceTransport(const FileSequence & sequence, unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , sequence(sequence)
    {
    }

    ~OutByFilenameSequenceTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
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


class OutByFilenameSequenceTransport2 : public OutFileTransport {
public:
    const FileSequence & sequence;
    char path[1024];

    OutByFilenameSequenceTransport2(const FileSequence & sequence, unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , sequence(sequence)
    {
    }

    ~OutByFilenameSequenceTransport2()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
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


//class OutByFilenameSequenceTransport3 : public Transport {
//public:
//    SQ  * seq;
//    RIO * rio;
//    char path[1024];

//    OutByFilenameSequenceTransport3(const char * path, const char * basename, const char * extension, unsigned verbose = 0)
//    : rio(NULL)
//    {
//        RIO_ERROR status = RIO_ERROR_OK;
//        this->seq = sq_new_outfilename(&status, SQF_PREFIX_PID_COUNT_EXTENSION, basename, extension);
//        this->rio = rio_new_outsequence(&status, sequence);
//    }

//    ~OutByFilenameSequenceTransport3()
//    {
//    }

//    using Transport::send;
//    virtual void send(const char * const buffer, size_t len) throw (Error) {
//        if (this->fd == -1){
//            this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
//            this->fd = ::creat(this->path, 0777);
//            if (this->fd == -1){
//                LOG(LOG_INFO, "OutByFilename transport write failed with error : %s", strerror(errno));
//                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
//            }
//        }
//        OutFileTransport::send(buffer, len);
//    }

//    virtual bool next()
//    {
//        if (this->fd != -1){
//            ::close(this->fd);
//            this->fd = -1;
//        }
//        this->OutFileTransport::next();
//        return true;
//    }
//};

#endif

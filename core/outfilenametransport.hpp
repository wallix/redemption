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


class FileSequence
{
    SQ sq;
    int pid;
    SQ_FORMAT sqf;
public:
    FileSequence(
        const char * const format,
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : pid(getpid())
    , sqf(SQF_PREFIX_EXTENSION)
    {
        if (0 == strcmp(format, "path file pid count extension")){
            this->sqf = SQF_PREFIX_PID_COUNT_EXTENSION;
        }
        else if (0 == strcmp(format, "path file count extension")){
            this->sqf = SQF_PREFIX_COUNT_EXTENSION;
        }
        else if (0 == strcmp(format, "path file pid extension")){
            this->sqf = SQF_PREFIX_PID_EXTENSION;
        }
        else if (0 == strcmp(format, "path file extension")){
            this->sqf = SQF_PREFIX_EXTENSION;
        }
        else {
            LOG(LOG_ERR, "Unsupported sequence format string");
            throw Error(ERR_TRANSPORT);
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s%s", prefix, filename);
        TODO("sanity check check path len")

        RIO_ERROR status = sq_init_outfilename(&this->sq, sqf, path, extension);
        if (status < 0){
            LOG(LOG_ERR, "Sequence outfilename initialisation failed");
            throw Error(ERR_TRANSPORT);
        }
    }

    ~FileSequence()
    {
        sq_clear(&this->sq);
    }

    void get_name(char * const buffer, size_t len, uint32_t count) const {
        sq_im_SQOutfilename_get_name(&(this->sq.u.outfilename), buffer, len, count);
    }

    ssize_t filesize(uint32_t count) const {
        char filename[1024];
        this->get_name(filename, sizeof(filename), count);
        return ::filesize(filename);
    }

    ssize_t unlink(uint32_t count) const {
        char filename[1024];
        this->get_name(filename, sizeof(filename), count);
        return ::unlink(filename);
    }
};


class OutFilenameTransport : public OutFileTransport {
public:
    FileSequence sequence;
    char path[1024];

    OutFilenameTransport(
            const char * const format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , sequence(format, prefix, filename, extension)
    {
    }

    ~OutFilenameTransport()
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


#endif

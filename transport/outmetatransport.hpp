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
   Author(s): Christophe Grosjean, Raphael Zhou

   Transport layer abstraction
*/

#ifndef _REDEMPTION_TRANSPORT_OUTMETATRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_OUTMETATRANSPORT_HPP_

#include "transport.hpp"
#include "error.hpp"

#include <limits>
#include "rio/rio.h"
#include "outfiletransport.hpp"
#include "outfilenametransport.hpp"


class OutmetaTransport
: public Transport
{
    detail::OutFilenameCreator filename_creator;
    io::posix::fdbuf fdbuf;
    timeval start_tv;
    timeval stop_tv;

public:
    OutmetaTransport(const char * path, const char * basename, timeval now,
                     uint16_t width, uint16_t height, const int groupid, auth_api * authentifier = NULL,
                     unsigned verbose = 0)
    : filename_creator(SQF_PATH_FILE_PID_COUNT_EXTENSION, path, basename, ".wrm", groupid)
    , start_tv(now)
    , stop_tv(now)
    {
        {
            char meta_filename[2048];
            int res = snprintf(meta_filename, sizeof(meta_filename), "%s%s-%06u.mwrm", path, basename, getpid());
            if (res > int(sizeof(meta_filename) - 6) || res < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
            if (this->fdbuf.open(meta_filename, O_WRONLY|O_CREAT, S_IRUSR) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }
        }

        char header1[(std::numeric_limits<uint16_t>::digits10 + 1) * 2 + 2];
        sprintf(header1, "%u %u", width, height);
        ssize_t res = this->fdbuf.write(header1, strlen(header1));
        if (res > 0) {
            res = this->fdbuf.write("\n0\n\n", 4);
        }

        if (res < 0) {
            int err = errno;
            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                this->authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_INFO, "Write to transport failed (M): code=%d", err);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    virtual ~OutmetaTransport()
    {
        this->filename_creator.next();
        this->write_wrm(false);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw(Error)
    {
        this->filename_creator.send(buffer, len, this->authentifier);
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw(Error)
    {
        LOG(LOG_INFO, "OutmetaTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw(Error)
    {
        this->filename_creator.seek(offset, whence);
    }

    virtual bool next()
    {
        this->filename_creator.next();
        this->write_wrm(true);
        return Transport::next();
    }

    virtual void timestamp(timeval now)
    {
        this->stop_tv.tv_sec = now.tv_sec;
        this->stop_tv.tv_usec = now.tv_usec;
    }

private:
    void write_wrm(bool send_exception)
    {
        const char * filename = this->filename_creator.get_filename();
        size_t len = strlen(filename);
        ssize_t res = this->fdbuf.write(filename, len);
        if (res > 0) {
            char mes[(std::numeric_limits<uint16_t>::digits10 + 1) * 2 + 5];
            len = snprintf(mes, sizeof(mes), " %u %u\n",
                           (unsigned)this->start_tv.tv_sec,
                           (unsigned)this->stop_tv.tv_sec+1);
            res = this->fdbuf.write(mes, len);
            this->start_tv.tv_sec = this->stop_tv.tv_sec;
            this->start_tv.tv_usec = this->stop_tv.tv_usec;
        }
        if (res < 0) {
            int err = errno;
            LOG(LOG_INFO, "Write to transport failed (M): code=%d", err);
            if (send_exception) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
    }
};


/*************************
* CryptoOutmetaTransport *
*************************/

class CryptoOutmetaTransport : public Transport
{
public:
    RIO   rio;
    SQ  * seq;
    char  path[512];

    CryptoOutmetaTransport(CryptoContext * crypto_ctx, const char * path, const char * hash_path,
        const char * basename, timeval now, uint16_t width, uint16_t height,
        const int groupid, auth_api * authentifier = NULL, unsigned verbose = 0)
    : seq(NULL)
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        char filename[1024];
        snprintf(filename, sizeof(filename), "%s-%06u", basename, getpid());
        char header1[64];
        sprintf(header1, "%u %u", width, height);
        RIO_ERROR status = rio_init_cryptooutmeta(&this->rio,
            &this->seq, crypto_ctx, path, hash_path, filename, ".mwrm", header1, "0", "",
            &now, groupid);
        if (status < 0)
        {
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", this->path);
                this->authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_INFO, "Write to transport failed (CM): code=%d", errno);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }

        size_t max_path_length = sizeof(path) - 1;
        strncpy(this->path, path, max_path_length);
        this->path[max_path_length] = 0;
    }

    ~CryptoOutmetaTransport()
    {
        if (this->full_cleaning_requested)
        {
            rio_full_clear(&this->rio);
        }
        else
        {
            rio_clear(&this->rio);
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw(Error)
    {
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0)
        {
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", this->path);
                this->authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_INFO, "Write to transport failed (CM): code=%d", errno);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
    }

    using Transport::recv;
    virtual void recv(char**, size_t) throw(Error)
    {
        LOG(LOG_INFO, "CryptoOutmetaTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void seek(int64_t offset, int whence) throw(Error)
    {
        LOG(LOG_INFO, "Set position within transport failed: code=%d", errno);
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual void timestamp(timeval now)
    {
        sq_timestamp(this->seq, &now);
        Transport::timestamp(now);
    }

    virtual bool next()
    {
        sq_next(this->seq);
        return Transport::next();
    }
};

#endif

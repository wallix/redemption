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

class OutmetaTransport : public OutFileTransport {
public:
    timeval now;
    FileSequence sequence;
    char meta_path[1024];
    char path[1024];

    OutmetaTransport(const char * path, const char * basename, timeval now, uint16_t width, uint16_t height, FileSequence ** pwrm_sequence, unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , now(now)
    , sequence("path file pid count extension", path, basename, "wrm")
    {
        *pwrm_sequence = &this->sequence;
        this->timestamp(now);
        snprintf(this->meta_path, 1024, "%s%s-%06u.mwrm", path, basename, getpid());
        TODO("Add error management")
        TODO("use system constants instead of fixed numeric values for paths len")
        int mfd = ::creat(this->meta_path, 0777);
        char buffer[2048];
        size_t len = sprintf(buffer, "%u %u\n0\n\n", width, height);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        ::close(mfd);
    }

    ~OutmetaTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        int mfd = ::open(this->meta_path, O_APPEND|O_WRONLY, 0777);
        if (mfd < 0){
            LOG(LOG_ERR, "Failed to open meta_file %s : error %s", this->meta_path, strerror(errno));
            throw Error(ERR_TRANSPORT);
        }
        char buffer[2048];
        size_t len = sprintf(buffer, "%s %u %u\n", this->path, (unsigned)this->now.tv_sec, (unsigned)this->future.tv_sec);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        ::close(mfd);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            LOG(LOG_INFO, "next chunk file: path=%s\n", this->path);
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
        int mfd = ::open(this->meta_path, O_APPEND|O_WRONLY, 0777);
        if (mfd < 0){
            LOG(LOG_ERR, "Failed to open meta_file %s : error %s", this->meta_path, strerror(errno));
            throw Error(ERR_TRANSPORT);
        }
        char buffer[2048];
        size_t len = sprintf(buffer, "%s %u %u\n", this->path, (unsigned)this->now.tv_sec, (unsigned)this->future.tv_sec);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->now = this->future;
        ::close(mfd);
        this->OutFileTransport::next();
        return true;
    }
};

#endif

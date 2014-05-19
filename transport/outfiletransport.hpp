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

#ifndef REDEMPTION_TRANSPORT_OUTFILETRANSPORT_HPP
#define REDEMPTION_TRANSPORT_OUTFILETRANSPORT_HPP

#include "transport.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>


class OutFileTransport
: public Transport
{
    int fd;
    uint32_t verbose;

public:
    OutFileTransport(int fd, unsigned verbose = 0)
    : fd(fd)
    , verbose(verbose)
    {}

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        const ssize_t res = this->privsend(buffer, len);
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

    virtual void seek(int64_t offset, int whence) throw (Error)
    {
        if (lseek(this->fd, offset, whence) < 0) {
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

private:
    ssize_t privsend(const char * data, size_t len) const
    {
        ssize_t ret = 0;
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ret = ::write(this->fd, data + total_sent, remaining_len);
            if (ret <= 0){
                if (errno == EINTR){
                    continue;
                }
                return -1;
            }
            remaining_len -= ret;
            total_sent += ret;
        }
        return total_sent;
    }
};

#endif

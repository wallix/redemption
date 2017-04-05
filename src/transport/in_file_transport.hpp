/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include "core/error.hpp"
#include "transport/transport.hpp"

#include <cerrno>


class InFileTransport : public Transport
{
protected:
    int fd;

public:
    explicit InFileTransport(int fd)
    : fd(fd)
    {}

    ~InFileTransport()
    {
        this->disconnect();
    }

    bool disconnect() override {
        if (-1 != this->fd) {
            const int ret = ::close(this->fd);
            this->fd = -1;
            return !ret;
        }
        return !0;
    }

private:

    void do_recv_new(uint8_t * buffer, size_t len) override {
        // TODO the do_recv API is annoying (need some intermediate pointer to get result), fix it => read all or raise exeception?
        ssize_t res = -1;
        size_t remaining_len = len;
        while (remaining_len) {
            res = ::read(this->fd, buffer + (len - remaining_len), remaining_len);
            if (res <= 0){
                if ((res == 0)
                ||  ((errno != EINTR) && (remaining_len != len))){
                    break;
                }
                if (errno == EINTR){
                    continue;
                }
                this->status = false;
                throw Error(ERR_TRANSPORT_READ_FAILED, res);
            }
            remaining_len -= res;
        }
        res = len - remaining_len;
        //*pbuffer += res;
        this->last_quantum_received += res;
        if (remaining_len != 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    bool do_atomic_read(uint8_t * buffer, size_t len) override {
        ssize_t res = -1;
        size_t remaining_len = len;
        while (remaining_len) {
            res = ::read(this->fd, buffer + (len - remaining_len), remaining_len);
            if (res <= 0){
                if (res == 0) {
                    if (remaining_len == len){
                        return false;
                    }
                }
                if ((res != 0) && (errno == EINTR)){
                    continue;
                }
                this->status = false;
                throw Error(ERR_TRANSPORT_READ_FAILED, res);
            }
            remaining_len -= res;
        }
        this->last_quantum_received += len;
        if (remaining_len != 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        return true;
    }
};

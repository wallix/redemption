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

#include "utils/fdbuf.hpp"
#include "transport/transport.hpp"

class OutFileTransport
: public Transport
{
    io::posix::fdbuf file;

public:
    explicit OutFileTransport(int fd, auth_api * auth = nullptr) noexcept
    : file(fd)
    {
        if (auth) {
            this->authentifier = auth;
        }
    }

    bool disconnect() override {
        return !this->file.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override
    {
        const ssize_t res = this->file.write(data, len);
        if (res < 0) {
            this->status = false;
            auto eid = ERR_TRANSPORT_WRITE_FAILED;
            if (errno == ENOSPC) {
                this->authentifier->report("FILESYSTEM_FULL", "100|unknow");
                errno = ENOSPC;
                eid = ERR_TRANSPORT_WRITE_NO_ROOM;
            }
            throw Error(eid, errno);
        }
        this->last_quantum_sent += res;
    }

protected:
    io::posix::fdbuf & buffer() noexcept
    { return this->file; }

    const io::posix::fdbuf & buffer() const noexcept
    { return this->file; }

    typedef OutFileTransport TransportType;
};

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

namespace detail
{
    struct NoCurrentPath2 {
        template<class Buf>
        static const char * current_path(Buf &)
        { return nullptr; }
    };
}

template <class Buf, class PathTraits = detail::NoCurrentPath2>
class OutputTransportFile
: public Transport
{
    Buf buf;

public:
    OutputTransportFile() = default;

    template<class T>
    explicit OutputTransportFile(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect() override {
        return !this->buf.close();
    }

private:
    void do_send(const char * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                const char * filename = PathTraits::current_path(this->buf);
                snprintf(message, sizeof(message), "100|%s", filename ? filename : "unknow");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

protected:
    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    typedef OutputTransportFile TransportType;
};


struct OutFileTransport
: OutputTransportFile<io::posix::fdbuf>
{
    explicit OutFileTransport(int fd) noexcept
    : OutFileTransport::TransportType(fd)
    {}
};

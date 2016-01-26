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

#ifndef REDEMPTION_TRANSPORT_IN_FILE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_IN_FILE_TRANSPORT_HPP

// #include "buffer/buffering_buf.hpp"
#include "mixin_transport.hpp"
#include "fdbuf.hpp"

class InputTransportFlat : public Transport
{
    io::posix::fdbuf buf;

public:
    InputTransportFlat() = default;

    template<class T>
    explicit InputTransportFlat(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect() override {
        return !this->buf.close();
    }

private:
    void do_recv(char ** pbuffer, size_t len) override {
        const ssize_t res = this->buf.read(*pbuffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        *pbuffer += res;
        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

protected:
    io::posix::fdbuf & buffer() noexcept
    { return this->buf; }

    const io::posix::fdbuf & buffer() const noexcept
    { return this->buf; }

    typedef InputTransportFlat TransportType;
};


struct InFileTransport : InputTransportFlat
{
    explicit InFileTransport(int fd) noexcept
    : InFileTransport::TransportType(fd)
    {}
};

struct InFileSeekableTransport : InputTransportFlat
{
    explicit InFileSeekableTransport(int fd) noexcept
    : InFileSeekableTransport::TransportType(fd)
    {}
    
    void seek(int64_t offset, int whence) override {
        if (static_cast<off64_t>(-1) == this->buffer().seek(offset, whence)){
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }
};

#endif

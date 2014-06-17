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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_TRANSPORT_HPP

#include "transport.hpp"
#include <cerrno>

template<class Trans>
struct DispatchTransport
: Transport
{
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        this->do_recv(pbuffer, len);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        this->do_send(buffer, len);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual ~DispatchTransport()
    {}

protected:
    virtual void do_recv(char ** pbuffer, size_t len) {
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    virtual void do_send(const char * const buffer, size_t len) {
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }
};

struct nexter_transport_base
{
    template<class Transport, class Buf>
    int next(Transport & /*trans*/, Buf & buf) /*noexcept*/
    {
        return buf.close();
    }
};

template<class Buf, class Nexter = nexter_transport_base>
struct InBufferTransport
: DispatchTransport<InBufferTransport<Buf> >
, protected Buf
, protected Nexter
{
    InBufferTransport()
    {}

    template<class T>
    InBufferTransport(const T & params)
    : Buf(params)
    {}

    template<class T, class U>
    InBufferTransport(const T & buf_params, const U & nexter_params)
    : Buf(buf_params)
    , Nexter(nexter_params)
    {}

    void do_recv(char ** pbuffer, size_t len) {
        const ssize_t res = this->Buf::read(*pbuffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        *pbuffer += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->Nexter::next(*this, static_cast<Buf&>(*this));
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, res);
            }
            else {
                throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
            }
        }
        return 0;
    }

protected:
    typedef InBufferTransport TransportType;
    typedef Buf BufferType;
    typedef Nexter NexterType;
};

template<class Buf, class Nexter = nexter_transport_base>
struct OutBufferTransport
: DispatchTransport<OutBufferTransport<Buf> >
, protected Buf
, protected Nexter
{
    OutBufferTransport()
    {}

    template<class T>
    OutBufferTransport(const T & buf_params)
    : Buf(buf_params)
    {}

    template<class T, class U>
    OutBufferTransport(const T & buf_params, const U & nexter_params)
    : Buf(buf_params)
    , Nexter(nexter_params)
    {}

    ~OutBufferTransport()
    {
        this->Nexter::next(*this, static_cast<Buf&>(*this));
    }

    void do_send(const char * buffer, size_t len) {
        const ssize_t res = this->Buf::write(buffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_WRITE_FAILED, res);
        }
    }

    virtual bool next()
    {
        const ssize_t res = this->Nexter::next(*this, static_cast<Buf&>(*this));
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_WRITE_FAILED, res);
        }
        if (!res) {
            ++this->seqno;
        }
        return !res;
    }

protected:
    typedef OutBufferTransport TransportType;
    typedef Buf BufferType;
    typedef Nexter NexterType;

    NexterType & nexter()
    { return *this; }

    BufferType & buffer()
    { return *this; }
};

#endif

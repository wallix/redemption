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

#ifndef REDEMPTION_TRANSPORT_BUFFER_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_BUFFER_TRANSPORT_HPP

#include "transport.hpp"
#include <cerrno>

struct nexter_transport_base
{
    template<class Buf>
    int next(Buf & buf) /*noexcept*/
    {
        return buf.close();
    }

    template<class Buf>
    /*constexpr*/ bool next_end(Buf & buf) /*noexcept*/
    {
        return 0;
    }

    template<class Buf>
    /*constexpr*/ const char * current_path(Buf & buf) /*noexcept*/
    {
        return "";
    }
};

namespace detail
{
    template<class Buf, class Nexter>
    struct BufAndNextTransport
    : Transport
    {
        BufAndNextTransport()
        {}

        template<class T>
        BufAndNextTransport(const T & params)
        : buf_and_next(params)
        {}

        template<class T, class U>
        BufAndNextTransport(const T & buf_params, const U & nexter_params)
        : buf_and_next(buf_params, nexter_params)
        {}

    protected:
        typedef Buf BufferType;
        typedef Nexter NexterType;
        typedef BufAndNextTransport BufAndNextType;

        NexterType & nexter()
        { return static_cast<NexterType&>(this->buf_and_next); }

        BufferType & buffer()
        { return static_cast<BufferType&>(this->buf_and_next); }

        const BufferType & buffer() const
        { return static_cast<const BufferType&>(this->buf_and_next); }

    private:
        //padding optimisation
        struct BufAndNext : Buf , Nexter
        {
            BufAndNext()
            {}

            template<class T>
            BufAndNext(const T & params)
            : Buf(params)
            {}

            template<class T, class U>
            BufAndNext(const T & buf_params, const U & nexter_params)
            : Buf(buf_params)
            , Nexter(nexter_params)
            {}
        } buf_and_next;
    };
}

template<class Buf, class Nexter = nexter_transport_base>
struct InBufferTransport
: detail::BufAndNextTransport<Buf, Nexter>
{
    InBufferTransport()
    {}

    template<class T>
    InBufferTransport(const T & params)
    : InBufferTransport::BufAndNextType(params)
    {}

    template<class T, class U>
    InBufferTransport(const T & buf_params, const U & nexter_params)
    : InBufferTransport::BufAndNextType(buf_params, nexter_params)
    {}

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->nexter().next(this->buffer());
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

private:
    void do_recv(char ** pbuffer, size_t len) {
        const ssize_t res = this->buffer().read(*pbuffer, len);
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
    typedef InBufferTransport TransportType;
};


template<class Buf, class Nexter = nexter_transport_base>
struct OutBufferTransport
: detail::BufAndNextTransport<Buf, Nexter>
{
    OutBufferTransport()
    {}

    template<class T>
    OutBufferTransport(const T & buf_params)
    : OutBufferTransport::BufAndNextType(buf_params)
    {}

    template<class T, class U>
    OutBufferTransport(const T & buf_params, const U & nexter_params)
    : OutBufferTransport::BufAndNextType(buf_params, nexter_params)
    {}

    ~OutBufferTransport()
    {
        this->nexter().next_end(this->buffer());
    }

    bool disconnect()
    {
        return this->nexter().next_end(this->buffer());
    }

    virtual bool next()
    {
        const ssize_t res = this->nexter().next(this->buffer());
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_WRITE_FAILED, res);
        }
        if (!res) {
            ++this->seqno;
        }
        return !res;
    }

private:
    void do_send(const char * buffer, size_t len)
    {
        const ssize_t res = this->buffer().write(buffer, len);
        if (res < 0){
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                const char * filename = this->nexter().current_path(this->buffer());
                snprintf(message, sizeof(message), "100|%s", filename ? filename : "unknow");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, res);
        }
        this->last_quantum_sent += res;
    }

protected:
    typedef OutBufferTransport TransportType;
};

#endif

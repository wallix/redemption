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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TRANSPORT_MIXIN_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_MIXIN_TRANSPORT_HPP

#include "transport.hpp"
#include <cerrno>

namespace detail
{
    struct NoCurrentPath {
        template<class Buf>
        static const char * current_path(Buf &)
        { return nullptr; }
    };

    struct GetCurrentPath {
        template<class Buf>
        static const char * current_path(Buf & buf)
        { return buf.current_path(); }
    };
}


template <class Buf, class PathTraits = detail::NoCurrentPath>
class OutputTransport
: public Transport
{
    Buf buf;

public:
    OutputTransport() = default;

    template<class T>
    OutputTransport(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect()
    {
        return !this->buf.close();
    }

private:
    void do_send(const char * data, size_t len)
    {
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

    typedef OutputTransport TransportType;
};

template <class Buf>
class InputTransport
: public Transport
{
    Buf buf;

public:
    InputTransport() = default;

    template<class T>
    InputTransport(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect()
    {
        return !this->buf.close();
    }

private:
    void do_recv(char ** pbuffer, size_t len) {
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
    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    typedef InputTransport TransportType;
};


template<class Buf>
struct InputNextTransport
: InputTransport<Buf>
{
    InputNextTransport() = default;

    template<class T>
    InputNextTransport(const T & buf_params)
    : InputTransport<Buf>(buf_params)
    {}

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res){
            this->status = false;
            if (res < 0) {
                throw Error(ERR_TRANSPORT_READ_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
        ++this->seqno;
        return true;
    }

protected:
    typedef InputNextTransport TransportType;
};

template<class Buf, class PathTraits = detail::NoCurrentPath>
struct OutputNextTransport
: OutputTransport<Buf, PathTraits>
{
    OutputNextTransport() = default;

    template<class T>
    OutputNextTransport(const T & buf_params)
    : OutputTransport<Buf, PathTraits>(buf_params)
    {}

    virtual bool next()
    {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

protected:
    typedef OutputNextTransport TransportType;
};


template<class TTransport>
struct SeekableTransport
: TTransport
{
    SeekableTransport() = default;

    template<class T>
    SeekableTransport(const T & params)
    : TTransport(params)
    {}

    virtual void seek(int64_t offset, int whence)
    {
        if ((off64_t)-1 == this->buffer().seek(offset, whence)){
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

protected:
    typedef SeekableTransport TransportType;
};


template<class TTransport>
struct RequestCleaningTransport
: TTransport
{
    RequestCleaningTransport() = default;

    template<class T>
    RequestCleaningTransport(const T & params)
    : TTransport(params)
    {}

    virtual void request_full_cleaning()
    {
        this->buffer().request_full_cleaning();
    }

protected:
    typedef RequestCleaningTransport TransportType;
};


template<class TTransport>
struct FlushingTransport
: TTransport
{
    FlushingTransport() = default;

    template<class T>
    FlushingTransport(const T & params)
    : TTransport(params)
    {}

    virtual void flush()
    {
        int res = this->buffer().flush();
        if (res) {
            throw Error(ERR_TRANSPORT_WRITE_FAILED, res < 0 ? 0 : res);
        }
    }

protected:
    typedef FlushingTransport TransportType;
};

#endif

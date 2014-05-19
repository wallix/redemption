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

#ifndef REDEMPTION_TRANSPORT_TESTTRANSPORT_HPP
#define REDEMPTION_TRANSPORT_TESTTRANSPORT_HPP

#include "rio/rio.h"
#include "transport.hpp"
#include "unique_ptr.hpp"

#include <new>
#include <algorithm>


namespace detail {
    struct BufArrayTransport
    {
        unique_ptr<uint8_t[]> data;
        std::size_t len;
        std::size_t current;

        BufArrayTransport(const char * data, size_t len)
        : data(new(std::nothrow) uint8_t[len])
        , len(len)
        {
            if (!this->data) {
                throw Error(ERR_TRANSPORT, 0);
            }
            memcpy(this->data.get(), data, len);
        }
    };
}

class GeneratorTransport
: public Transport
{
    detail::BufArrayTransport buffer;

public:
    GeneratorTransport(const char * data, size_t len, uint32_t verbose = 0)
    : buffer(data, len)
    {}

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        const size_t rlen = std::min<size_t>(this->buffer.len - this->buffer.current, len);
        memcpy(*pbuffer, this->buffer.data.get() + this->buffer.current, rlen);
        this->buffer.current += rlen;
        *pbuffer += rlen;
        if (rlen < len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
    }

    using Transport::send;
    virtual void send(const char * const /*buffer*/, size_t /*len*/) throw (Error)
    {
        // send perform like a /dev/null and does nothing in generator transport
    }

    virtual void seek(int64_t /*offset*/, int /*whence*/) throw (Error)
    {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool get_status() const
    {
        return true;
    }
};


class CheckTransport
: public Transport
{
    detail::BufArrayTransport buffer;

public:
    CheckTransport(const char * data, size_t len, uint32_t verbose = 0)
    : buffer(data, len)
    {}

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        // CheckTransport does never receive anything
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

    using Transport::send;
    virtual void send(const char * const data, size_t len) throw (Error)
    {
        const size_t available_len = std::min<size_t>(this->buffer.len - this->buffer.current, len);
        if (0 != memcmp(data, this->buffer.data.get() + this->buffer.current, available_len)){
            // data differs, find where
            uint32_t differs = 0;
            for (size_t i = 0; i < available_len ; i++){
                if (data[i] != this->buffer.data.get()[this->buffer.current+i]) {
                    differs = i;
                    break;
                }
            }
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump_c(data, differs);
            LOG(LOG_INFO, "=============== Expected ==========");
            hexdump_c(this->buffer.data.get() + this->buffer.current + differs, available_len - differs);
            LOG(LOG_INFO, "=============== Got ===============");
            hexdump_c(data + differs, available_len - differs);
            this->buffer.data.reset();
            throw Error(ERR_TRANSPORT_DIFFERS);
        }

        this->buffer.current += available_len;

        if (available_len != len){
            LOG(LOG_INFO, "Check transport out of reference data available=%u len=%u", available_len, len);
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump_c(data, available_len);
            LOG(LOG_INFO, "=============== Got Unexpected Data ==========");
            hexdump_c(data + available_len, len - available_len);
            this->buffer.data.reset();
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
    }

    virtual void seek(int64_t offset, int whence) throw (Error)
    {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool get_status() const
    {
        return true;
    }
};


class TestTransport
: public Transport
{
    CheckTransport check;
    GeneratorTransport gen;
    unique_ptr<uint8_t[]> public_key;
    std::size_t public_key_length;

public:
    TestTransport(const char * name, const char * outdata, size_t outlen,
                  const char * indata, size_t inlen, uint32_t verbose = 0)
    : check(indata, inlen, verbose)
    , gen(outdata, outlen, verbose)
    , public_key_length(0)
    {}

    void set_public_key(uint8_t * data, size_t data_size) {
        this->public_key.reset(new uint8_t[data_size]);
        this->public_key_length = data_size;
        memcpy(this->public_key.get(), data, data_size);
    }

    virtual const uint8_t * get_public_key() const {
        return this->public_key.get();
    }

    virtual size_t get_public_key_length() const {
        return this->public_key_length;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        this->gen.recv(pbuffer, len);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        this->check.send(buffer, len);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool get_status() const
    {
        return this->check.get_status() && this->gen.get_status();
    }
};


class LogTransport
: public Transport
{
public:
    LogTransport()
    {}

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        throw Error(RIO_ERROR_SEND_ONLY, 0);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        hexdump_c(buffer, len);
    }

    virtual void seek(int64_t offset, int whence) throw (Error) {
        throw Error(ERR_TRANSPORT_SEEK_NOT_AVAILABLE);
    }

    virtual bool get_status() const
    {
        return true;
    }
};

#endif


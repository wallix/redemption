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
   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan

   Transport layer abstraction
*/

#ifndef REDEMPTION_TRANSPORT_TEST_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_TEST_TRANSPORT_HPP

#include "transport.hpp"
#include "mixin_transport.hpp"
#include "buffer/dynarray_buf.hpp"
#include "buffer/check_buf.hpp"

#include <new>
#include <memory>

struct GeneratorTransport
: InputTransport<transbuf::dynarray_base>
{
    GeneratorTransport(const char * data, size_t len, uint32_t verbose = 0)
    {
        if (this->buffer().open(len, data)) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};


class CheckTransport
: public Transport
{
    std::unique_ptr<uint8_t[]> data;
    std::size_t len;
    std::size_t current;

public:
    CheckTransport(const char * data, size_t len, uint32_t verbose = 0)
    : data(new(std::nothrow) uint8_t[len])
    , len(len)
    , current(0)
    {
        if (!this->data) {
            throw Error(ERR_TRANSPORT, 0);
        }
        memcpy(this->data.get(), data, len);
    }

private:
    void do_send(const char * const data, size_t len)
    {
        const size_t available_len = (this->current + len > this->len) ? (this->len - this->current) : len;
        if (0 != memcmp(data, this->data.get() + this->current, available_len)){
            // data differs, find where
            uint32_t differs = 0;
            while (differs < available_len && data[differs] == this->data.get()[this->current+differs]) {
                ++differs;
            }
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump_c(data, differs);
            LOG(LOG_INFO, "=============== Expected ==========");
            hexdump_c(this->data.get() + this->current + differs, available_len - differs);
            LOG(LOG_INFO, "=============== Got ===============");
            hexdump_c(data + differs, available_len - differs);
            this->data.reset();
            this->status = false;
            throw Error(ERR_TRANSPORT_DIFFERS);
        }

        this->current += available_len;

        if (available_len != len){
            LOG(LOG_INFO, "Check transport out of reference data available=%u len=%u", available_len, len);
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump_c(data, available_len);
            LOG(LOG_INFO, "=============== Got Unexpected Data ==========");
            hexdump_c(data + available_len, len - available_len);
            this->data.reset();
            this->status = false;
            throw Error(ERR_TRANSPORT_DIFFERS);
        }
    }
};


class TestTransport
: public Transport
{
    CheckTransport check;
    GeneratorTransport gen;
    std::unique_ptr<uint8_t[]> public_key;
    std::size_t public_key_length;

public:
    TestTransport(const char * name, const char * outdata, size_t outlen,
                  const char * indata, size_t inlen, uint32_t verbose = 0)
    : check(indata, inlen, verbose)
    , gen(outdata, outlen, verbose)
    , public_key_length(0)
    {}

    void set_public_key(const uint8_t * data, size_t data_size) {
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

    virtual bool get_status() const {
        return this->check.get_status() && this->gen.get_status();
    }

private:
    void do_recv(char ** pbuffer, size_t len) {
        this->gen.recv(pbuffer, len);
    }

    void do_send(const char * const buffer, size_t len) {
        this->check.send(buffer, len);
    }
};


class LogTransport
: public Transport
{
    void do_send(const char * const buffer, size_t len) {
        LOG(LOG_INFO, "Sending on target (-1) %u bytes", len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Sent dumped on target (-1) %u bytes", len);
    }
};


class MemoryTransport : public Transport {
public:
    BStream         stream;
    FixedSizeStream in_stream;
    FixedSizeStream out_stream;

    MemoryTransport()
    : Transport()
    , stream(65536)
    , in_stream(stream.get_data(), stream.get_capacity())
    , out_stream(stream.get_data(), stream.get_capacity()) {}

    virtual void do_recv(char ** pbuffer, size_t len) {
        char * buffer = *pbuffer;
        this->in_stream.in_copy_bytes(buffer, len);
        (*pbuffer) = buffer + len;
    }

    virtual void do_send(const char * const buffer, size_t len) {
        this->out_stream.out_copy_bytes(buffer, len);
    }
};

#endif


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


#pragma once

#include <new>
#include <memory>
#include <stdexcept>
#include <sstream>

#include <fstream>
#include <sstream>
#include <string>

#include <algorithm>
#include <cstring>

#include "transport/transport.hpp"
#include "utils/stream.hpp"

namespace transbuf {

    class dynarray_buf
    {
        std::unique_ptr<uint8_t[]> data;
        std::size_t len = 0;
        std::size_t current = 0;

    public:
        dynarray_buf() = default;

        int open(size_t len, const void * data = nullptr)
        {
            this->data.reset(new(std::nothrow) uint8_t[len]);
            if (!this->data) {
                return -1;
            }
            if (data) {
                memcpy(this->data.get(), data, len);
            }
            this->len = len;
            this->current = 0;
            return 0;
        }

        int close() noexcept
        {
            this->data.reset();
            this->current = 0;
            this->len = 0;
            return 0;
        }

        long int read(void * buffer, size_t len)
        { return this->copy(buffer, this->data.get() + this->current, len); }

        long int write(const void * buffer, size_t len)
        { return this->copy(this->data.get() + this->current, buffer, len); }

        bool is_open() const noexcept
        { return this->data.get(); }

        int flush() const noexcept
        { return 0; }

    private:
        long int copy(void * dest, const void * src, size_t len)
        {
            const size_t rlen = std::min<size_t>(this->len - this->current, len);
            memcpy(dest, src, rlen);
            this->current += rlen;
            return rlen;
        }
    };

}


namespace transbuf {

    class check_base
    {
        std::unique_ptr<uint8_t[]> data;
        std::size_t len;
        std::size_t current;

    public:
        check_base() noexcept
        : len(0)
        , current(0)
        {}

        int open(const char * data, size_t len)
        {
            this->data.reset(new(std::nothrow) uint8_t[len]);
            memcpy(this->data.get(), data, len);
            this->len = len;
            this->current = 0;
            return 0;
        }

        int close() noexcept
        {
            this->data.reset();
            this->current = 0;
            this->len = 0;
            return 0;
        }

        int write(const void * buffer, size_t len)
        {
            const size_t rlen = std::min<size_t>(this->len - this->current, len);
            const uint8_t * databuf = static_cast<const uint8_t *>(buffer);
            const uint8_t * p = std::mismatch(databuf, databuf + rlen, this->data.get() + this->current).first;
            this->current += rlen;
            return p - (databuf + rlen);
        }

        bool is_open() const noexcept
        { return this->data.get(); }

        int flush() const noexcept
        { return 0; }
    };

}



class InputTransportDynarray : public Transport
{
    transbuf::dynarray_buf buf;

public:
    InputTransportDynarray() = default;

    template<class T>
    explicit InputTransportDynarray(const T & buf_params)
    : buf(buf_params)
    {}

    bool disconnect() override {
        return !this->buf.close();
    }

private:
    void do_recv(uint8_t ** pbuffer, size_t len) override {
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

    void do_recv_new(uint8_t * buffer, size_t len) override {
        const ssize_t res = this->buf.read(buffer, len);
        if (res < 0){
            this->status = false;
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        buffer += res;
        this->last_quantum_received += res;
        if (static_cast<size_t>(res) != len){
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }

protected:
    transbuf::dynarray_buf & buffer() noexcept
    { return this->buf; }

    const transbuf::dynarray_buf & buffer() const noexcept
    { return this->buf; }

    typedef InputTransportDynarray TransportType;
};


struct GeneratorTransport : public InputTransportDynarray
{
    GeneratorTransport(const void * data, size_t len, bool verbose = false)
    : verbose(verbose)
    {
        if (this->buffer().open(len, data)) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }

    void do_send(const uint8_t * const buffer, size_t len) override {
        LOG(LOG_INFO, "do_send %zu bytes", len);
        if (this->verbose){
            LOG(LOG_INFO, "Sending on target (-1) %zu bytes", len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "Sent dumped on target (-1) %zu bytes", len);
        }
    }

private:
    bool verbose;
};


class CheckTransport
: public Transport
{
    std::unique_ptr<uint8_t[]> data;
    std::size_t len;
    std::size_t current;
    bool remaining_is_error = true;

    struct remaining_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

public:
    CheckTransport(const char * data, size_t len, bool verbose = false)
    : data(new(std::nothrow) uint8_t[len])
    , len(len)
    , current(0)
    {
        (void)verbose;
        if (!this->data) {
            throw Error(ERR_TRANSPORT, 0);
        }
        memcpy(this->data.get(), data, len);
    }

    void disable_remaining_error() {
        this->remaining_is_error = false;
    }

    ~CheckTransport() override {
        this->disconnect();
    }

    bool disconnect() override {
        if (this->remaining_is_error && this->len != this->current) {
            this->status = false;
            std::ostringstream out;
            out << "Check transport remaining=" << (this->len-this->current) << " len=" << this->len;
            throw remaining_error{out.str()};
        }
        return true;
    }

private:
    void do_send(const uint8_t * const data, size_t len) override
    {
        const size_t available_len = std::min<size_t>(this->len - this->current, len );
        if (0 != memcmp(data, this->data.get() + this->current, available_len)){
            // data differs, find where
            uint32_t differs = 0;
            while (differs < available_len && data[differs] == this->data.get()[this->current+differs]) {
                ++differs;
            }
            LOG(LOG_ERR, "=============== Common Part =======");
            hexdump_c(data, differs);
            LOG(LOG_ERR, "=============== Expected ==========");
            hexdump_c(this->data.get() + this->current + differs, available_len - differs);
            LOG(LOG_ERR, "=============== Got ===============");
            hexdump_c(data + differs, available_len - differs);
            this->data.reset();
            this->status = false;
            this->remaining_is_error = false;
            throw Error(ERR_TRANSPORT_DIFFERS);
        }

        this->current += available_len;

        if (available_len != len){
            LOG(LOG_ERR, "Check transport out of reference data available=%zu len=%zu", available_len, len);
            LOG(LOG_ERR, "=============== Common Part =======");
            hexdump_c(data, available_len);
            LOG(LOG_ERR, "=============== Got Unexpected Data ==========");
            hexdump_c(data + available_len, len - available_len);
            this->data.reset();
            this->status = false;
            this->remaining_is_error = false;
            throw Error(ERR_TRANSPORT_DIFFERS);
        }

        this->last_quantum_sent += len;
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
    TestTransport(
        const char * outdata, size_t outlen,
        const char * indata, size_t inlen,
        bool verbose = false)
    : check(indata, inlen, verbose)
    , gen(outdata, outlen, verbose)
    , public_key_length(0)
    {}


    void disable_remaining_error() {
        this->check.disable_remaining_error();
    }

    void set_public_key(const uint8_t * data, size_t data_size) {
        this->public_key.reset(new uint8_t[data_size]);
        this->public_key_length = data_size;
        memcpy(this->public_key.get(), data, data_size);
    }

    const uint8_t * get_public_key() const override {
        return this->public_key.get();
    }

    size_t get_public_key_length() const override {
        return this->public_key_length;
    }

    bool get_status() const override {
        return this->check.get_status() && this->gen.get_status();
    }

private:
    void do_recv(uint8_t ** pbuffer, size_t len) override {

        this->gen.recv(pbuffer, len);
    }

    void do_recv_new(uint8_t * buffer, size_t len) override {

        this->gen.recv_new(buffer, len);
    }

    void do_send(const uint8_t * const buffer, size_t len) override {
        this->check.send(buffer, len);
    }
};



class LogTransport
: public Transport
{
    void do_send(const uint8_t * const buffer, size_t len) override {
        LOG(LOG_INFO, "Sending on target (-1) %zu bytes", len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Sent dumped on target (-1) %zu bytes", len);
    }
};


class MemoryTransport : public Transport
{
    uint8_t buf[65536];

public:
    InStream    in_stream{buf};
    OutStream   out_stream{buf};

    void do_recv(uint8_t ** buffer, size_t len) override {
        this->in_stream.in_copy_bytes(*buffer, len);
    }

    void do_recv_new(uint8_t * buffer, size_t len) override {
        this->in_stream.in_copy_bytes(buffer, len);
    }

    void do_send(const uint8_t * const buffer, size_t len) override {
        this->out_stream.out_copy_bytes(buffer, len);
    }
};

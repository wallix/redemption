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

struct RemainingError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct GeneratorTransport : Transport
{
    GeneratorTransport(const void * data, size_t len)
    : len(len)
    {
        this->data.reset(new(std::nothrow) uint8_t[len]);
        if (!this->data) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        if (data) {
            memcpy(this->data.get(), data, len);
        }
    }

    ~GeneratorTransport()
    {
        this->disconnect();
    }

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    bool disconnect() override
    {
        if (this->remaining_is_error && this->len != this->current) {
            this->remaining_is_error = false;
            LOG(LOG_ERR, "=============== Expected ==========");
            hexdump_c(this->data.get() + this->current, this->len - this->current);
            std::ostringstream out;
            out << "~GeneratorTransport() remaining=" << (this->len-this->current) << " len=" << this->len;
            throw RemainingError{out.str()};
        }
        return true;
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        size_t const remaining = this->len - this->current;
        if (!remaining) {
            return Read::Eof;
        }
        if (len > remaining) {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        memcpy(buffer, this->data.get() + this->current, len);
        this->current += len;
        return Read::Ok;
    }

    void do_send(const uint8_t * const buffer, size_t len) override
    {
        LOG(LOG_INFO, "Sending on target (-1) %zu bytes", len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Sent dumped on target (-1) %zu bytes", len);
    }

    std::unique_ptr<uint8_t[]> data;
    std::size_t len = 0;
    std::size_t current = 0;
    bool remaining_is_error = true;
};


struct CheckTransport : Transport
{
    CheckTransport(const char * data, size_t len)
    : data(new(std::nothrow) uint8_t[len])
    , len(len)
    , current(0)
    {
        if (!this->data) {
            throw Error(ERR_TRANSPORT, 0);
        }
        memcpy(this->data.get(), data, len);
    }

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    ~CheckTransport() override {
        this->disconnect();
    }

    bool disconnect() override
    {
        if (this->remaining_is_error && this->len != this->current) {
            this->remaining_is_error = false;
            LOG(LOG_ERR, "=============== Expected ==========");
            hexdump_c(this->data.get() + this->current, this->len - this->current);
            std::ostringstream out;
            out << "~CheckTransport() remaining=" << (this->len-this->current) << " len=" << this->len;
            throw RemainingError{out.str()};
        }
        return true;
    }

private:
    void do_send(const uint8_t * const data, size_t len) override
    {
        const size_t available_len = std::min<size_t>(this->len - this->current, len);
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
            this->remaining_is_error = false;
            throw Error(ERR_TRANSPORT_DIFFERS);
        }

        this->last_quantum_sent += len;
    }

    std::unique_ptr<uint8_t[]> data;
    std::size_t len;
    std::size_t current;
    bool remaining_is_error = true;
};


struct TestTransport : public Transport
{
    TestTransport(
        const char * indata, size_t inlen,
        const char * outdata, size_t outlen)
    : check(outdata, outlen)
    , gen(indata, inlen)
    , public_key_length(0)
    {}

    void disable_remaining_error()
    {
        this->check.disable_remaining_error();
        this->gen.disable_remaining_error();
    }

    void set_public_key(const uint8_t * data, size_t data_size)
    {
        this->public_key.reset(new uint8_t[data_size]);
        this->public_key_length = data_size;
        memcpy(this->public_key.get(), data, data_size);
    }

    const uint8_t * get_public_key() const override
    {
        return this->public_key.get();
    }

    size_t get_public_key_length() const override
    {
        return this->public_key_length;
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        return this->gen.atomic_read(buffer, len);
    }

    void do_send(const uint8_t * const buffer, size_t len) override
    {
        this->check.send(buffer, len);
    }

    CheckTransport check;
    GeneratorTransport gen;
    std::unique_ptr<uint8_t[]> public_key;
    std::size_t public_key_length;
};


class LogTransport : public Transport
{
    void do_send(const uint8_t * const buffer, size_t len) override
    {
        LOG(LOG_INFO, "Sending on target (-1) %zu bytes", len);
        hexdump_c(buffer, len);
        LOG(LOG_INFO, "Sent dumped on target (-1) %zu bytes", len);
    }
};


class MemoryTransport : public Transport
{
    uint8_t buf[65536];
    bool remaining_is_error = true;

public:
    InStream    in_stream{buf};
    OutStream   out_stream{buf};

    ~MemoryTransport()
    {
        this->disconnect();
    }

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    bool disconnect() override
    {
        if (this->remaining_is_error && this->in_stream.get_offset() != this->out_stream.get_offset()) {
            std::ostringstream out;
            out << "~MemoryTransport() remaining=" << this->in_stream.get_offset() << " len=" << this->out_stream.get_offset();
            throw RemainingError{out.str()};
        }
        return true;
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        auto const in_offset = this->in_stream.get_offset();
        auto const out_offset = this->out_stream.get_offset();
        if (in_offset == out_offset){
            return Read::Eof;
        }
        if (in_offset + len > out_offset){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        this->in_stream.in_copy_bytes(buffer, len);
        return Read::Ok;
    }

    void do_send(const uint8_t * const buffer, size_t len) override
    {
        if (len > this->out_stream.tailroom()) {
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        this->out_stream.out_copy_bytes(buffer, len);
    }
};

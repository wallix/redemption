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

#include <vector>
#include <string>
#include <memory>

#include "transport/transport.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"


struct GeneratorTransport : Transport
{
    GeneratorTransport(bytes_view buffer);

    ~GeneratorTransport();

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    bool disconnect() override;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    size_t do_partial_read(uint8_t* buffer, size_t len) override;

    const std::unique_ptr<uint8_t[]> data;
    const std::size_t len;
    std::size_t current = 0;
    bool remaining_is_error = true;
};


struct BufTransport : Transport
{
    std::string buf;

    [[nodiscard]] std::size_t size() const noexcept { return buf.size(); }
    void clear() noexcept { buf.clear(); }
    std::string& data() noexcept { return buf; }

private:
    void do_send(const uint8_t * const data, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;
    size_t do_partial_read(uint8_t* buffer, size_t len) override;
};


struct BufSequenceTransport : Transport
{
    BufSequenceTransport();

    std::string& operator[](std::size_t i) noexcept { return this->datas[i]; }
    [[nodiscard]] std::size_t size() const noexcept { return this->datas.size(); }
    [[nodiscard]] bool empty() const noexcept;

private:
    void do_send(const uint8_t * const data, size_t len) override;

    bool next() override;

    std::vector<std::string> datas;
};


struct CheckTransport : Transport
{
    CheckTransport(buffer_view buffer);

    [[nodiscard]] size_t remaining() const
    {
        return this->len - this->current;
    }

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    ~CheckTransport() override;

    bool disconnect() override;

private:
    void do_send(const uint8_t * const data, size_t len) override;

    std::unique_ptr<uint8_t[]> data;
    std::size_t len;
    std::size_t current = 0;
    bool remaining_is_error = true;
};


struct TestTransport : public Transport
{
    TestTransport(bytes_view indata, bytes_view outdata);

    size_t remaining()
    {
        return this->check.remaining();
    }

    void disable_remaining_error()
    {
        this->check.disable_remaining_error();
        this->gen.disable_remaining_error();
    }

    void set_public_key(bytes_view key);

    [[nodiscard]] u8_array_view get_public_key() const override;

protected:
    size_t do_partial_read(uint8_t* buffer, size_t len) override;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;

    CheckTransport check;
    GeneratorTransport gen;
    std::unique_ptr<uint8_t[]> public_key;
    std::size_t public_key_length = 0;
};


class MemoryTransport : public Transport
{
    uint8_t buf[65536];
    bool remaining_is_error = true;

public:
    InStream    in_stream{buf};
    OutStream   out_stream{buf};

    ~MemoryTransport();

    void disable_remaining_error()
    {
        this->remaining_is_error = false;
    }

    bool disconnect() override;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    size_t do_partial_read(uint8_t* buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;
};

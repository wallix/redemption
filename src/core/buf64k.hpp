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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/cxx.hpp"
#include "transport/transport.hpp" // InTransport

#include <cstring>

class PartialReaderAPI {
public:
    virtual size_t partial_read(byte_ptr buffer, size_t len) = 0;
    virtual ~PartialReaderAPI() = default;
};

struct Buf64k
{
    REDEMPTION_NON_COPYABLE(Buf64k);

    Buf64k() = default;

    uint16_t remaining() const noexcept
    {
        return uint16_t(this->len - this->idx);
    }

    array_view_u8 av() noexcept
    {
        return {this->buf + this->idx, this->remaining()};
    }

    array_view_u8 av(std::size_t n) noexcept
    {
        assert(n <= this->remaining());
        return {this->buf + this->idx, n};
    }

    array_view_const_u8 av(std::size_t n) const noexcept
    {
        assert(n <= this->remaining());
        return {this->buf + this->idx, n};
    }

    array_view_u8 sub(std::size_t i, std::size_t n) noexcept
    {
        assert(n <= this->remaining());
        return {this->buf + this->idx + i, n};
    }

    array_view_const_u8 sub(std::size_t i, std::size_t n) const noexcept
    {
        assert(n <= this->remaining());
        return {this->buf + this->idx + i, n};
    }

    void advance(std::size_t n) noexcept
    {
        assert(n <= this->remaining());
        this->idx += n;
    }

    array_view_u8 get_buffer_and_advance(std::size_t n) noexcept
    {
        auto av = this->av(n);
        this->advance(n);
        return av;
    }

    // TODO: read_from interface is slightly annoying. It will be a problem when compiling code to js with emscripten
    // because the input buffer will have to be filled from outside by websocket code.
    // web socket implementation also suggest it is risky to copy data from a part of buffer to another
    // (because of asynchronous access). And should we find a way to lock len and idx ?
    void read_from(InTransport trans)
    {
        if (this->idx == this->len) {
            this->len = trans.partial_read(this->buf, max_len);
            this->idx = 0;
        }
        else {
            if (this->idx) {
                std::memmove(this->buf, this->buf + this->idx, this->remaining());
                this->len -= this->idx;
                this->idx = 0;
            }
            this->len += trans.partial_read(this->buf + this->len, max_len - this->len);
        }
    }

    void read_from(PartialReaderAPI & trans)
    {
        if (this->idx == this->len) {
            this->len = trans.partial_read(this->buf, max_len);
            this->idx = 0;
        }
        else {
            if (this->idx) {
                std::memmove(this->buf, this->buf + this->idx, this->remaining());
                this->len -= this->idx;
                this->idx = 0;
            }
            this->len += trans.partial_read(this->buf + this->len, max_len - this->len);
        }
    }


private:
    static constexpr std::size_t max_len = uint16_t(~uint16_t{});
    uint8_t buf[max_len];
    uint16_t len = 0;
    uint16_t idx = 0;
};

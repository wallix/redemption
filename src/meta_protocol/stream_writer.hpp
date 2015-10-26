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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_META_PROTOCOL_STREAM_WRITER_HPP
#define REDEMPTION_SRC_META_PROTOCOL_STREAM_WRITER_HPP

#include "types.hpp"
#include "stream.hpp"

namespace meta_protocol
{

struct dummy_ {};
void write(dummy_, OutStream &, uint8_t *, size_t) {}

struct stream_writer
{
    OutStream & out_stream;

    template<class T, T x, class Tag>
    void operator()(types::out_integral<T, x, Tag>) const {
        (*this)(types::out_dyn<T, Tag>{x});
    }

    void operator()(types::out_dyn_u8 x) const {
        this->out_stream.out_uint8(x.x);
    }

    void operator()(types::out_dyn_u16_be x) const {
        this->out_stream.out_uint16_be(x.x);
    }

    void operator()(types::out_dyn_u32_be x) const {
        this->out_stream.out_uint32_be(x.x);
    }

    void operator()(types::out_dyn_u64_be x) const {
        this->out_stream.out_uint64_be(x.x);
    }

    void operator()(types::out_dyn_u16_le x) const {
        this->out_stream.out_uint16_le(x.x);
    }

    void operator()(types::out_dyn_u32_le x) const {
        this->out_stream.out_uint32_le(x.x);
    }

    void operator()(types::out_dyn_u64_le x) const {
        this->out_stream.out_uint64_le(x.x);
    }

    void operator()(types::out_bytes x) const {
        this->out_stream.out_copy_bytes(x.data(), x.size());
    }

    template<size_t N>
    void operator()(types::out_array<N> x) const {
        this->out_stream.out_copy_bytes(x.data(), x.size());
    }

    template<class Fn>
    void operator()(Fn && fn, uint8_t * pkt, uint8_t len) const {
        this->dispatch_pkt_expr(1, fn, pkt, len);
    }

    template<class Fn>
    auto dispatch_pkt_expr(int, Fn && fn, uint8_t * pkt, uint8_t len) const
    -> decltype(void(write(fn, std::declval<OutStream&>(), pkt, len)))
    {
        write(fn, this->out_stream, pkt, len);
    }

    template<class Fn>
    void dispatch_pkt_expr(unsigned, Fn && fn, uint8_t * pkt, uint8_t len) const {
        (*this)(fn(pkt, len));
    }
};

}

#endif

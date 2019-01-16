/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE TestTdpuBuffer
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include <numeric>
#include "core/buf64k.hpp"

struct BlockTransport : Transport
{
    BlockTransport(array_view_const_u8 data, std::size_t n_by_bloc)
      : data(data)
      , n_by_bloc(n_by_bloc)
    {}

    size_t operator()(uint8_t* buffer, size_t len)
    {
        std::size_t const n = std::min({len, this->data.size(), this->n_by_bloc});
        if (!n) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        memcpy(buffer, this->data.to_u8p(), n);
        this->data = this->data.array_from_offset(n);
        return n;
    }

private:
    cbytes_view data;
    std::size_t n_by_bloc;
};

RED_AUTO_TEST_CASE(Test1Read1)
{
    Buf64k buf;
    RED_CHECK_EQ(0, buf.remaining());
    RED_CHECK_MEM(const_bytes_view{}, buf.av());

    uint8_t data[100000];
    std::iota(std::begin(data), std::end(data), 0);

    uint16_t const k64 { 64*1024 - 1 };

    {
        BlockTransport t(data, 1);
        for (unsigned i = 0; i < k64; ++i) {
            buf.read_with(t);
        }
        RED_CHECK_EQ(k64, buf.remaining());
        RED_CHECK_MEM(make_array_view(data, k64), buf.av());
        RED_CHECK_EXCEPTION_ERROR_ID(buf.read_with(t), ERR_TRANSPORT_NO_MORE_DATA);
        RED_CHECK_EQ(k64, buf.remaining());
        buf.advance(k64);
        buf.read_with(t);
        RED_CHECK_EQ(1, buf.remaining());
        RED_CHECK_MEM(const_bytes_view(data+k64, 1), buf.av());
    }

    buf.advance(buf.remaining());

    {
        BlockTransport t(data, 24000);
        buf.read_with(t);
        buf.read_with(t);
        RED_CHECK_EQ(48000, buf.remaining());
        buf.read_with(t);
        RED_CHECK_EQ(k64, buf.remaining());
        buf.advance(1000);
        buf.read_with(t);
        RED_CHECK_EQ(k64, buf.remaining());
        RED_CHECK_MEM(const_bytes_view(data+1000, k64), buf.av());
    }
}

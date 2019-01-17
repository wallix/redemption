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


#include "core/RDP/tpdu_buffer.hpp"

struct BlockTransport : Transport
{
    BlockTransport(cbytes_view data, std::size_t n_by_bloc)
      : data(data)
      , n_by_bloc(n_by_bloc)
    {}

    size_t do_partial_read(uint8_t* buffer, size_t len) override
    {
        std::size_t const n = std::min({len, this->data.size(), this->n_by_bloc});
        if (!n) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        memcpy(buffer, this->data.to_u8p(), n);
        this->data = this->data.array_from_offset(n);
        return n;
    }

    std::size_t remaining() const
    {
        return this->data.size();
    }

private:
    cbytes_view data;
    std::size_t n_by_bloc;
};

constexpr auto data1 = cstr_array_view(
/* 0000 */ "\x03\x00\x00\x55\x50\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...UP......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x6a\x62\x62\x65" //e: mstshash=jbbe
/* 0020 */ "\x72\x74\x68\x65\x6c\x69\x6e\x0d\x0a\x01\x08\x08\x00\x0b\x00\x00" //rthelin.........
/* 0030 */ "\x00\x06\x00\x24\x00\x75\xcc\x9f\xac\x96\xa5\x41\x82\xbd\x1c\x2d" //...$.u.....A...-
/* 0040 */ "\x63\x52\xc7\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //cR..............
/* 0050 */ "\x00\x00\x00\x00\x00" //.....
);
static_assert(data1.size() == 85, "");

constexpr auto data2 = cstr_array_view(
/* 0000 */ "\x03\x00\x00\x55\x50\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...UP......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x6a\x62\x62\x65" //e: mstshash=jbbe
/* 0020 */ "\x72\x74\x68\x65\x6c\x69\x6e\x0d\x0a\x01\x08\x08\x00\x0b\x00\x00" //rthelin.........
/* 0030 */ "\x00\x06\x00\x24\x00\x75\xcc\x9f\xac\x96\xa5\x41\x82\xbd\x1c\x2d" //...$.u.....A...-
/* 0040 */ "\x63\x52\xc7\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //cR..............
/* 0050 */ "\x00\x00\x00\x00\x00" //.....
/* 0000 */ "\x03\x00\x00\x55\x50\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...UP......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x6a\x62\x62\x65" //e: mstshash=jbbe
/* 0020 */ "\x72\x74\x68\x65\x6c\x69\x6e\x0d\x0a\x01\x08\x08\x00\x0b\x00\x00" //rthelin.........
/* 0030 */ "\x00\x06\x00\x24\x00\x75\xcc\x9f\xac\x96\xa5\x41\x82\xbd\x1c\x2d" //...$.u.....A...-
/* 0040 */ "\x63\x52\xc7\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //cR..............
/* 0050 */ "\x00\x00\x00\x00\x00" //.....
);
static_assert(data2.size() == 85*2, "");


RED_AUTO_TEST_CASE(Test1Read1)
{
    BlockTransport t(data1, 1);
    TpduBuffer buf;

    for (int i = 0; i < 84; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data1);
}

RED_AUTO_TEST_CASE(Test1Read10)
{
    BlockTransport t(data1, 10);
    TpduBuffer buf;

    for (int i = 0; i < 8; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data1);
}

RED_AUTO_TEST_CASE(Test1Read100)
{
    BlockTransport t(data1, 100);
    TpduBuffer buf;

    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data1);
}


RED_AUTO_TEST_CASE(Test2Read1)
{
    BlockTransport t(data2, 1);
    TpduBuffer buf;

    for (int i = 0; i < 84; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 85);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));

    for (int i = 0; i < 84; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));
}

RED_AUTO_TEST_CASE(Test2Read10)
{
    BlockTransport t(data2, 10);
    TpduBuffer buf;

    for (int i = 0; i < 8; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 80);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));

    for (int i = 0; i < 7; ++i) {
        buf.load_data(t); RED_CHECK(!buf.next_pdu());
    }
    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));
}

RED_AUTO_TEST_CASE(Test2Read100)
{
    BlockTransport t(data2, 100);
    TpduBuffer buf;

    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 70);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));

    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));
}

RED_AUTO_TEST_CASE(Test2Read1000)
{
    BlockTransport t(data2, 1000);
    TpduBuffer buf;

    buf.load_data(t); RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    auto av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));

    RED_CHECK(buf.next_pdu());
    RED_CHECK_EQ(t.remaining(), 0);

    RED_CHECK(!buf.current_pdu_is_fast_path());

    av = buf.current_pdu_buffer();
    RED_CHECK_MEM(av, data2.array_from_offset(85));
}

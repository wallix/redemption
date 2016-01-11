/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGCC
#include <boost/test/auto_unit_test.hpp>

//#define LOGNULL
#define LOGPRINT

#include "src/core/RDP/channels/rdpdr.hpp"

BOOST_AUTO_TEST_CASE(TestDeviceCreateRequest1)
{
    const char in_data[] =
            "\x81\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x07\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00\x00\x0e\x00\x00\x00" // ........ .......
            "\x5c\x00\x2e\x00\x63\x00\x70\x00\x61\x00\x6e\x00\x00\x00"         // ....c.p.a.n...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateRequest device_create_request;

    device_create_request.receive(in_stream);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(TestDeviceCreateRequest2)
{
    const char in_data[] =
            "\x89\x00\x12\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x07\x00\x00\x00\x01\x00\x00\x00\x60\x00\x00\x00\x36\x00\x00\x00" // ........`...6...
            "\x5c\x00\x50\x00\x72\x00\x6f\x00\x67\x00\x72\x00\x61\x00\x6d\x00" // ..P.r.o.g.r.a.m.
            "\x20\x00\x46\x00\x69\x00\x6c\x00\x65\x00\x73\x00\x5c\x00\x64\x00" //  .F.i.l.e.s...d.
            "\x65\x00\x73\x00\x6b\x00\x74\x00\x6f\x00\x70\x00\x2e\x00\x69\x00" // e.s.k.t.o.p...i.
            "\x6e\x00\x69\x00\x00\x00"                                         // n.i...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateRequest device_create_request;

    device_create_request.receive(in_stream);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

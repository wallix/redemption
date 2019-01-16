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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestTestTransport
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/error.hpp"

RED_AUTO_TEST_CASE(TestGeneratorTransport)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv_boom(p, 0);
    RED_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv_boom(p, 1);
    p += 1;
    RED_CHECK_EQUAL(p-buffer, 1);
    RED_CHECK_EQUAL(buffer[0], 'W');
    RED_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv_boom(p, 2);
    p += 2;
    RED_CHECK_EQUAL(p-buffer, 2);
    RED_CHECK_EQUAL(buffer[0], 'e');
    RED_CHECK_EQUAL(buffer[1], ' ');
    RED_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv_boom(p, 9);
    p += 9;
    RED_CHECK_EQUAL(p-buffer, 9);
    RED_CHECK_MEM_C(make_array_view(buffer, 9), "read what");

    p = buffer;
    gt.recv_boom(p, 12);
    p += 12;
    RED_CHECK_EQUAL(p-buffer, 12);
    RED_CHECK_MEM_C(make_array_view(buffer, 12), " we provide!");

    //p = buffer;
    //gt.recv_boom(p, 0);
    //RED_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    RED_CHECK_EXCEPTION_ERROR_ID(gt.recv_boom(p, 1), ERR_TRANSPORT_NO_MORE_DATA);
    RED_CHECK_EQUAL(p-buffer, 0);
}

RED_AUTO_TEST_CASE(TestGeneratorTransport2)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!", 24);
    char buffer[128] = {};
    char * p = buffer;
    gt.recv_boom(p, 0);
    RED_CHECK_EQUAL(p-buffer, 0);

    p = buffer;
    gt.recv_boom(p, 1);
    p += 1;
    RED_CHECK_EQUAL(p-buffer, 1);
    RED_CHECK_EQUAL(buffer[0], 'W');
    RED_CHECK_EQUAL(buffer[1], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv_boom(p, 2);
    p += 2;
    RED_CHECK_EQUAL(p-buffer, 2);
    RED_CHECK_EQUAL(buffer[0], 'e');
    RED_CHECK_EQUAL(buffer[1], ' ');
    RED_CHECK_EQUAL(buffer[2], 0); // unchanged, not put by GeneratorTransport

    p = buffer;
    gt.recv_boom(p, 9);
    p += 9;
    RED_CHECK_EQUAL(p-buffer, 9);
    RED_CHECK_MEM_C(make_array_view(buffer, 9), "read what");

    p = buffer;
    RED_CHECK_EXCEPTION_ERROR_ID(gt.recv_boom(p, 13), ERR_TRANSPORT_READ_FAILED);

    gt.recv_boom(p, 12);
    p += 12;
    RED_CHECK_EQUAL(p-buffer, 12);
    RED_CHECK_MEM_C(make_array_view(buffer, 12), " we provide!");
}

RED_AUTO_TEST_CASE(TestTestTransport)
{
    // TestTransport is bidirectional
    // We provide both an output and an input source
    // when using a test Transport we read what we provide in input source
    // and we check that what we write to output is identical to output source
    // if send fails, the difference between expected and actual data is showed
    // and status is set to false (and will stay so) to allow tests to fail.
    // inside Transport, the difference is shown in trace logs.
    TestTransport gt("OUTPUT", 6, "input", 5);
    gt.disable_remaining_error();
    char buf[128] = {};
    char * p = buf;
    uint32_t sz = 3;
    gt.recv_boom(p, sz);
    p += sz;
    RED_CHECK_MEM_C(make_array_view(p - sz, sz), "OUT");
    gt.send("in", 2);
    sz = 3;
    gt.recv_boom(p, sz);
    p += sz;
    RED_CHECK_MEM_C(make_array_view(p - sz, sz), "PUT");
    gt.send("put", 3);
}

RED_AUTO_TEST_CASE(TestMemoryTransport)
{
    MemoryTransport mt;

    char     s_data[]    = "0123456789ABCDEF";
    uint32_t s_data_size = sizeof(s_data)-1;

    mt.send(s_data, s_data_size);

    char     r_data[32]  = { 0 };

    mt.recv_boom(r_data, 4);
    RED_CHECK_MEM_C(make_array_view(r_data, 4), "0123");

    mt.recv_boom(r_data, 4);
    RED_CHECK_MEM_C(make_array_view(r_data, 4), "4567");

    mt.recv_boom(r_data, s_data_size - 8);
    RED_CHECK_MEM_C(make_array_view(r_data, s_data_size - 8), "89ABCDEF");
}

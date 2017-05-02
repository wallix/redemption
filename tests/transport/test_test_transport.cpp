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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

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
    RED_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    gt.recv_boom(p, 12);
    p += 12;
    RED_CHECK_EQUAL(p-buffer, 12);
    RED_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 12));

    p = buffer;
    gt.recv_boom(p, 0);
    RED_CHECK_EQUAL(p-buffer, 0);

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
    RED_CHECK_EQUAL(0, strncmp(buffer, "read what", 9));

    p = buffer;
    RED_CHECK_EXCEPTION_ERROR_ID(gt.recv_boom(p, 13), ERR_TRANSPORT_NO_MORE_DATA);
    RED_CHECK_EQUAL(p-buffer, 0);
    RED_CHECK_EQUAL(0, strncmp(buffer, " we provide!", 0));
}

RED_AUTO_TEST_CASE(TestCheckTransport)
{
    CheckTransport gt("input", 5);
    gt.disable_remaining_error();
    RED_CHECK_EQUAL(gt.get_status(), true);
    RED_CHECK_NO_THROW(gt.send("in", 2));
    RED_CHECK_EQUAL(gt.get_status(), true);
    RED_CHECK_EXCEPTION_ERROR_ID(gt.send("in", 2), ERR_TRANSPORT_DIFFERS);
    RED_CHECK(!gt.get_status());
}

RED_AUTO_TEST_CASE(TestCheckTransportInputOverflow)
{
    CheckTransport gt("0123456789ABCDEF", 16);
    RED_CHECK_EQUAL(gt.get_status(), true);
    RED_CHECK_EXCEPTION_ERROR_ID(gt.send("0123456789ABCDEFGHI", 19), ERR_TRANSPORT_DIFFERS);
    RED_CHECK(!gt.get_status());
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
    RED_CHECK_EQUAL(gt.get_status(), true);
    char buf[128] = {};
    char * p = buf;
    uint32_t sz = 3;
    gt.recv_boom(p, sz);
    p += sz;
    RED_CHECK(0 == memcmp(p - sz, "OUT", sz));
    gt.send("in", 2);
    RED_CHECK_EQUAL(gt.get_status(), true);
    sz = 3;
    gt.recv_boom(p, sz);
    p += sz;
    RED_CHECK(0 == memcmp(p - sz, "PUT", sz));
    RED_CHECK_EXCEPTION_ERROR_ID(gt.send("pot", 3), ERR_TRANSPORT_DIFFERS);
    RED_CHECK(!gt.get_status());
}

RED_AUTO_TEST_CASE(TestMemoryTransport)
{
    MemoryTransport mt;

    char     s_data[]    = "0123456789ABCDEF";
    uint32_t s_data_size = strlen(s_data);

    mt.send(reinterpret_cast<char *>(&s_data_size), sizeof(s_data_size));
    mt.send(s_data, s_data_size);

    char     r_data[32]  = { 0 };
    uint32_t r_data_size = 0;

    char * r_buffer = reinterpret_cast<char *>(&r_data_size);
    mt.recv_boom(reinterpret_cast<uint8_t *>(r_buffer), sizeof(uint32_t));
    RED_CHECK_EQUAL(r_data_size, s_data_size);
    //LOG(LOG_INFO, "r_data_size=%u", r_data_size);

    r_buffer = r_data;
    mt.recv_boom(reinterpret_cast<uint8_t *>(r_buffer), r_data_size);
    RED_CHECK_EQUAL(memcmp(r_data, s_data, r_data_size), 0);
    //LOG(LOG_INFO, "r_data=\"%s\"", r_data);
}

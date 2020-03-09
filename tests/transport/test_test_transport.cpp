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

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/error.hpp"

RED_AUTO_TEST_CASE(TestGeneratorTransport)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!"_av);
    char buffer[128] = {};
    RED_CHECK(gt.recv_boom(buffer, 0) == ""_av);

    RED_CHECK(gt.recv_boom(buffer, 1) == "W"_av);
    RED_CHECK_EQ(buffer[1], '\0'); // unchanged, not put by GeneratorTransport
    RED_CHECK(gt.recv_boom(buffer, 2) == "e "_av);
    RED_CHECK_EQ(buffer[3], '\0'); // unchanged, not put by GeneratorTransport
    RED_CHECK(gt.recv_boom(buffer, 9) == "read what"_av);
    RED_CHECK(gt.recv_boom(buffer, 12) == " we provide!"_av);
    RED_CHECK_EXCEPTION_ERROR_ID(gt.recv_boom(buffer, 1), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestGeneratorTransport2)
{
    // test we can read from a GeneratorTransport;
    GeneratorTransport gt("We read what we provide!"_av);
    char buffer[128] = {};
    RED_CHECK(gt.recv_boom(buffer, 0) == ""_av);

    // buffer[1] unchanged, not put by GeneratorTransport
    RED_CHECK(gt.recv_boom(buffer, 1) == "W"_av);
    RED_CHECK(gt.recv_boom(buffer, 2) == "e "_av);
    RED_CHECK(gt.recv_boom(buffer, 9) == "read what"_av);
    RED_CHECK_EXCEPTION_ERROR_ID(gt.recv_boom(buffer, 13), ERR_TRANSPORT_READ_FAILED);
    RED_CHECK(gt.recv_boom(buffer, 12) == " we provide!"_av);
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
    TestTransport gt("OUTPUT"_av, "input"_av);
    gt.disable_remaining_error();
    char buffer[128] = {};
    RED_CHECK(gt.recv_boom(buffer, 3) == "OUT"_av);
    gt.send("in", 2);
    RED_CHECK(gt.recv_boom(buffer, 3) == "PUT"_av);
    gt.send("put", 3);
}

RED_AUTO_TEST_CASE(TestMemoryTransport)
{
    MemoryTransport mt;

    auto data = "0123456789ABCDEF"_av;

    mt.send(data);

    char r_data[32] {};

    RED_CHECK(mt.recv_boom(r_data, 4) == "0123"_av);
    RED_CHECK(mt.recv_boom(r_data, 4) == "4567"_av);
    RED_CHECK(mt.recv_boom(r_data, data.size() - 8) == "89ABCDEF"_av);
}

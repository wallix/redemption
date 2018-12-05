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
   Copyright (C) Wallix 2016
   Author(s): Jennifer Inthavong

   T.124 Generic Conference Control (GCC) Unit Test
*/

#define RED_TEST_MODULE TestSC_CORE
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/gcc/userdata/sc_core.hpp"  

RED_AUTO_TEST_CASE(Test_gcc_sc_core)
{
    const char expected[] =
        "\x01\x0c\x0c\x00" // TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12 bytes
        "\x04\x00\x08\x00" // TS_UD_SC_CORE::version = 0x0080004
        "\x00\x00\x00\x00" // TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP
    ;

    uint8_t buf[12];
    GCC::UserData::SCCore sc_core;
    sc_core.length = 12;
    sc_core.version = 0x0080004;
    sc_core.clientRequestedProtocols = 0;
    OutStream out_stream(buf);
    sc_core.emit(out_stream);
    RED_CHECK_EQUAL(12, out_stream.get_offset());
    RED_CHECK(0 == memcmp(expected, out_stream.get_data(), 12));

    GCC::UserData::SCCore sc_core2;

    InStream in_stream(buf);
    sc_core2.recv(in_stream);
    RED_CHECK_EQUAL(SC_CORE, sc_core2.userDataType);
    RED_CHECK_EQUAL(12, sc_core2.length);
    RED_CHECK_EQUAL(0x0080004, sc_core2.version);
    RED_CHECK_EQUAL(0, sc_core2.clientRequestedProtocols);

    sc_core2.log("Server Received");
}

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

#define RED_TEST_MODULE TestCS_MONITOR
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"

#define TS_MONITOR_PRIMARY 0x00000001

RED_AUTO_TEST_CASE(Test_gcc_user_data_cs_monitor)
{
    const char indata[] =
        "\x05\xc0"         // TS_UD_HEADER::type = CS_MONITOR (0xc005)
        "\x20\x00"         // length = 32 bytes
        "\x00\x00\x00\x00" // flags. Unused. MUST be set to zero
        "\x01\x00\x00\x00" // TS_UD_CS_MONITOR::monitorCount

        				   // TS_UD_CS_MONITOR::monitorDefArray
		"\x00\x00\x00\x00" // | left
		"\x00\x00\x00\x00" // | top
		"\x64\x00\x00\x00" // | right
		"\xC4\xFF\xFF\xFF" // | bottom
        "\x00\x00\x00\x00" // | flags = TS_MONITOR_PRIMARY
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv_boom(end, sz);
    GCC::UserData::CSMonitor cs_monitor;
    InStream stream(buf);
    cs_monitor.recv(stream);
    RED_CHECK_EQUAL(32, cs_monitor.length);
    RED_CHECK_EQUAL(CS_MONITOR, cs_monitor.userDataType);
    RED_CHECK_EQUAL(1, cs_monitor.monitorCount);

    RED_CHECK_EQUAL(0, cs_monitor.monitorDefArray[0].left);  //(left, top) = (0,0)
    RED_CHECK_EQUAL(0, cs_monitor.monitorDefArray[0].top);
    RED_CHECK_EQUAL(100, cs_monitor.monitorDefArray[0].right); //(right, bottom) = (100,-60)
    RED_CHECK_EQUAL(-60, cs_monitor.monitorDefArray[0].bottom);
	RED_CHECK_EQUAL(0, (cs_monitor.monitorDefArray[0].flags & TS_MONITOR_PRIMARY) );

    cs_monitor.log("Client Received");

}

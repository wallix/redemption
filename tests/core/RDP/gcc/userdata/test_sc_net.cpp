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

#define RED_TEST_MODULE TestSC_NET
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/gcc/userdata/sc_net.hpp"

RED_AUTO_TEST_CASE(Test_gcc_sc_net)
{
    const char expected[] =
        "\x03\x0c\x10\x00" // TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes
        "\xeb\x03"         // TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        "\x03\x00"         // TS_UD_SC_NET::channelCount = 3
        "\xec\x03"         // channel0 = 0x3ec = 1004 (rdpdr)
        "\xed\x03"         // channel1 = 0x3ed = 1005 (cliprdr)
        "\xee\x03"         // channel2 = 0x3ee = 1006 (rdpsnd)
        "\x00\x00"         // padding
    ;

    uint8_t buf[16];
    OutStream out_stream(buf);
    GCC::UserData::SCNet sc_net;
    sc_net.MCSChannelId = 1003;
    sc_net.channelCount = 3;
    sc_net.channelDefArray[0].id = 1004;
    sc_net.channelDefArray[1].id = 1005;
    sc_net.channelDefArray[2].id = 1006;
    sc_net.emit(out_stream);
    RED_CHECK_EQUAL(16, out_stream.get_offset());
    RED_CHECK(0 == memcmp(expected, out_stream.get_data(), 12));

    GCC::UserData::SCNet sc_net2;

    const bool bogus_sc_net_size = false;
    InStream in_stream(buf);
    RED_CHECK_NO_THROW(sc_net2.recv(in_stream, bogus_sc_net_size));
    RED_CHECK_EQUAL(SC_NET, sc_net2.userDataType);
    RED_CHECK_EQUAL(1003, sc_net2.MCSChannelId);
    RED_CHECK_EQUAL(3, sc_net2.channelCount);
    RED_CHECK_EQUAL(1004, sc_net2.channelDefArray[0].id);
    RED_CHECK_EQUAL(1005, sc_net2.channelDefArray[1].id);
    RED_CHECK_EQUAL(1006, sc_net2.channelDefArray[2].id);

    sc_net2.log("Server Received");
}

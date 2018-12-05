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

#define RED_TEST_MODULE TestCS_MULTITRANSPORT
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_multitransport.hpp"

RED_AUTO_TEST_CASE(Test_gcc_user_data_cs_multitransport)
{
    const char indata[] =
        "\x0a\xc0"         // CS_MULTITRANSPORT
        "\x08\x00"         // 8 bytes user Data

        "\x05\x03\x00\x00" // TS_UD_CS_MULTITRANSPORT::flags
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv_boom(end, sz);
    GCC::UserData::CSMultiTransport cs_multitransport;
    InStream stream(buf);
    cs_multitransport.recv(stream);
    RED_CHECK_EQUAL(CS_MULTITRANSPORT, cs_multitransport.userDataType);
    RED_CHECK_EQUAL(8, cs_multitransport.length);
    RED_CHECK_EQUAL((GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDPFECR |
                     GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDPFECL |
                     GCC::UserData::CSMultiTransport::TRANSPORTTYPE_UDP_PREFERRED |
                     GCC::UserData::CSMultiTransport::SOFTSYNC_TCP_TO_UDP),
                    cs_multitransport.flags);

    cs_multitransport.log("Client Received");
}

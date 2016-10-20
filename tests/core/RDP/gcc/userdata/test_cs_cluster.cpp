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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCS_CLUSTER
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
// #define LOGPRINT

#include "transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_cluster.hpp"

BOOST_AUTO_TEST_CASE(Test_gcc_user_data_cs_cluster)
{
    const char indata[] =
        "\x04\xc0"         // CS_CLUSTER
        "\x0c\x00"         // 12 bytes user Data
        "\x0d\x00\x00\x00" // TS_UD_CS_CLUSTER::Flags = 0x0d
        // 0x0d
        // = 0x03 << 2 | 0x01
        // = REDIRECTION_VERSION4 << 2 | REDIRECTION_SUPPORTED
        "\x00\x00\x00\x00" // TS_UD_CS_CLUSTER::RedirectedSessionID
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv(&end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSCluster cs_cluster;
    cs_cluster.recv(stream);
    BOOST_CHECK_EQUAL(CS_CLUSTER, cs_cluster.userDataType);
    BOOST_CHECK_EQUAL(12, cs_cluster.length);
    BOOST_CHECK_EQUAL(13, cs_cluster.flags);
    BOOST_CHECK_EQUAL(0, cs_cluster.redirectedSessionID);

    cs_cluster.log("Client Received");
}

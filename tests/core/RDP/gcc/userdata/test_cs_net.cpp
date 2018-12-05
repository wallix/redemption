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

   T.124 Generic Conference Control (GCC) Unit Test
*/

#define RED_TEST_MODULE TestCS_NET
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_net.hpp"

RED_AUTO_TEST_CASE(Test_gcc_user_data_cs_net)
{
    const char indata[] =
        "\x03\xc0"         // CS_NET
        "\x20\x00"         // 32 bytes user Data
        "\x02\x00\x00\x00" // ChannelCount
        "\x63\x6c\x69\x70\x72\x64\x72\x00" // "cliprdr"
        "\x00\x00\xa0\xc0" // = CHANNEL_OPTION_INITIALIZED
                           // | CHANNEL_OPTION_ENCRYPT_RDP
                           // | CHANNEL_OPTION_COMPRESS_RDP
                           // | CHANNEL_OPTION_SHOW_PROTOCOL
        "\x72\x64\x70\x64\x72\x00\x00\x00" // "rdpdr"
        "\x00\x00\x80\x80" // = CHANNEL_OPTION_INITIALIZED
                           // | CHANNEL_OPTION_COMPRESS_RDP
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv_boom(end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSNet cs_net;
    cs_net.recv(stream);
    RED_CHECK_EQUAL(CS_NET, cs_net.userDataType);
    RED_CHECK_EQUAL(32, cs_net.length);
    RED_CHECK_EQUAL(2, cs_net.channelCount);
    RED_CHECK_EQUAL('c', cs_net.channelDefArray[0].name[0]);
    RED_CHECK_EQUAL(0, memcmp("cliprdr\0", cs_net.channelDefArray[0].name, 8));
    RED_CHECK_EQUAL(( GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                     | GCC::UserData::CSNet::CHANNEL_OPTION_ENCRYPT_RDP
                     | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP
                     | GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL)
                     , cs_net.channelDefArray[0].options);
    RED_CHECK_EQUAL(0, memcmp("rdpdr\0\0\0", cs_net.channelDefArray[1].name, 8));
    RED_CHECK_EQUAL(( GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED
                     | GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS_RDP)
                     , cs_net.channelDefArray[1].options);

    cs_net.log("Client Received");

}


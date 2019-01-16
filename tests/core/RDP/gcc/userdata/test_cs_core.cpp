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

#define RED_TEST_MODULE TestCS_CORE
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_core.hpp"


RED_AUTO_TEST_CASE(Test_gcc_user_data_cs_core)
{
    const char indata[] =
        "\x01\xc0"         // TS_UD_HEADER::type = CS_CORE (0xc001)
        "\xd8\x00"         // length = 216 bytes
        "\x04\x00\x08\x00" // TS_UD_CS_CORE::version = 0x0008004
        "\x00\x05"         // TS_UD_CS_CORE::desktopWidth = 1280
        "\x00\x04"         // TS_UD_CS_CORE::desktopHeight = 1024
        "\x01\xca"         // TS_UD_CS_CORE::colorDepth = RNS_UD_COLOR_8BPP (0xca01)
        "\x03\xaa"         // TS_UD_CS_CORE::SASSequence
        "\x09\x04\x00\x00" // TS_UD_CS_CORE::keyboardLayout = 0x409 = 1033 = English (US)
        "\xce\x0e\x00\x00" // TS_UD_CS_CORE::clientBuild = 3790
                           // TS_UD_CS_CORE::clientName = ELTONS-TEST2
        "\x45\x00\x4c\x00\x54\x00\x4f\x00\x4e\x00\x53\x00\x2d\x00\x44\x00"
        "\x45\x00\x56\x00\x32\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x04\x00\x00\x00" // TS_UD_CS_CORE::keyboardType
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::keyboardSubtype
        "\x0c\x00\x00\x00" // TS_UD_CS_CORE::keyboardFunctionKey
                           // TS_UD_CS_CORE::imeFileName = ""
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\xca"         // TS_UD_CS_CORE::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (0xca01)
        "\x01\x00"         // TS_UD_CS_CORE::clientProductId
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::serialNumber
        "\x18\x00"         // TS_UD_CS_CORE::highColorDepth = 24 bpp
        "\x07\x00"         // TS_UD_CS_CORE::supportedColorDepths
                           //0x07 = 0x01 | 0x02 | 0x04 = RNS_UD_24BPP_SUPPORT | RNS_UD_16BPP_SUPPORT | RNS_UD_15BPP_SUPPORT
        "\x01\x00"         // TS_UD_CS_CORE::earlyCapabilityFlags
                           //0x01 = RNS_UD_CS_SUPPORT_ERRINFO_PDU
                           //TS_UD_CS_CORE::clientDigProductId = "69712-783-0357974-42714"
        "\x36\x00\x39\x00\x37\x00\x31\x00\x32\x00\x2d\x00\x37\x00\x38\x00"
        "\x33\x00\x2d\x00\x30\x00\x33\x00\x35\x00\x37\x00\x39\x00\x37\x00"
        "\x34\x00\x2d\x00\x34\x00\x32\x00\x37\x00\x31\x00\x34\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00"             // TS_UD_CS_CORE::connectionType = 0 (not used as RNS_UD_CS_VALID_CONNECTION_TYPE not set)
        "\x00"             // TS_UD_CS_CORE::pad1octet
        "\x00\x00\x00\x00" // TS_UD_CS_CORE::serverSelectedProtocol
    ;

    constexpr std::size_t sz = sizeof(indata) - 1;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv_boom(end, sz);
    InStream stream(buf, sz);
    GCC::UserData::CSCore cs_core;
    cs_core.recv(stream);
    RED_CHECK_EQUAL(CS_CORE, cs_core.userDataType);
    RED_CHECK_EQUAL(216, cs_core.length);

    cs_core.log("Client Received");
}

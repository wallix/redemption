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

#define RED_TEST_MODULE TestCS_SECURITY
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "test_only/transport/test_transport.hpp"
#include "core/RDP/gcc/userdata/cs_security.hpp"

// 02 c0 0c 00 -> TS_UD_HEADER::type = CS_SECURITY (0xc002), length = 12 bytes


RED_AUTO_TEST_CASE(Test_gcc_user_data_cs_security)
{
    const char indata[] =
        "\x02\xc0"         // CS_SECURITY
        "\x0c\x00"         // 12 bytes user Data

        "\x1b\x00\x00\x00" // TS_UD_CS_SEC::encryptionMethods
                           // 0x1b = 0x01 | 0x02 | 0x08 | 0x10
                           // = 40BIT_ENCRYPTION_FLAG
                           // | 128BIT_ENCRYPTION_FLAG
                           // | 56BIT_ENCRYPTION_FLAG
                           // | FIPS_ENCRYPTION_FLAG
        "\x00\x00\x00\x00" // TS_UD_CS_SEC::extEncryptionMethods
    ;

    constexpr auto sz = sizeof(indata) - 1u;
    GeneratorTransport gt(indata, sz);
    uint8_t buf[sz];
    auto end = buf;
    gt.recv_boom(end, sz);
    GCC::UserData::CSSecurity cs_security;
    InStream stream(buf);
    cs_security.recv(stream);
    RED_CHECK_EQUAL(CS_SECURITY, cs_security.userDataType);
    RED_CHECK_EQUAL(12, cs_security.length);
    RED_CHECK_EQUAL(27, cs_security.encryptionMethods);
    RED_CHECK_EQUAL(0, cs_security.extEncryptionMethods);

    cs_security.log("Client Received");
}

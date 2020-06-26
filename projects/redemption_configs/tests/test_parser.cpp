/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#define RED_TEST_MODULE TestParser
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/autogen/enums.hpp"
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wunused-function")
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
#include "configs/autogen/enums_func_ini.hpp"
#include "configs/io.hpp"
#include "configs/autogen/enums_func_ini.tcc"
REDEMPTION_DIAGNOSTIC_POP

RED_TEST_DELEGATE_PRINT(parse_error, (x ? x.c_str() : "no error"));

namespace
{
    bool operator == (parse_error const& lhs, parse_error const& rhs)
    {
        return lhs.c_str() == rhs.c_str();
    }

    bool operator != (parse_error const& lhs, parse_error const& rhs)
    {
        return lhs.c_str() != rhs.c_str();
    }
}

RED_AUTO_TEST_CASE(TestEnumParser)
{
    char zbuffer[1024];
    auto zbuf_av = make_writable_array_view(zbuffer);

    // increment
    {
        RED_CHECK( is_valid_enum_value(ServerCertCheck(3)));
        RED_CHECK(!is_valid_enum_value(ServerCertCheck(4)));

        configs::spec_type<ServerCertCheck> spec{};

        ServerCertCheck v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "3"_zv));
        RED_CHECK(v == ServerCertCheck(3));
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "4"_zv));
        RED_CHECK(v == ServerCertCheck(3));

        RED_CHECK("2"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<ServerCertCheck>{}, ServerCertCheck(2)));
    }

    // flags
    {
        RED_CHECK( is_valid_enum_value(CaptureFlags(15)));
        RED_CHECK(!is_valid_enum_value(CaptureFlags(16)));

        configs::spec_type<CaptureFlags> spec{};

        CaptureFlags v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "15"_zv));
        RED_CHECK(v == CaptureFlags(15));
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "16"_zv));

        RED_CHECK("15"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<CaptureFlags>{}, CaptureFlags(15)));
    }

    // str
    {
        RED_CHECK( is_valid_enum_value(RdpModeConsole(2)));
        RED_CHECK(!is_valid_enum_value(RdpModeConsole(3)));

        configs::spec_type<RdpModeConsole> spec{};

        RdpModeConsole v{};
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "forbid"_zv));
        RED_CHECK(v == RdpModeConsole::forbid);
        RED_CHECK(no_parse_error == parse_from_cfg(v, spec, "aLloW"_zv));
        RED_CHECK(v == RdpModeConsole::allow);
        RED_CHECK(no_parse_error != parse_from_cfg(v, spec, "foRbI"_zv));
        RED_CHECK(v == RdpModeConsole::allow);

        RED_CHECK("forbid"_av ==
            assign_zbuf_from_cfg(zbuf_av, cfg_s_type<RdpModeConsole>{}, RdpModeConsole(2)));
    }
}

RED_AUTO_TEST_CASE(TestOtherParser)
{
    // unsigned
    {
        unsigned u;
        configs::spec_type<unsigned> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0"_zv));
        RED_CHECK_EQUAL(0, u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "3"_zv));
        RED_CHECK_EQUAL(3, u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x3"_zv));
        RED_CHECK_EQUAL(3, u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x00000007"_zv));
        RED_CHECK_EQUAL(7, u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0000000000000007"_zv));
        RED_CHECK_EQUAL(7, u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0007"_zv));
        RED_CHECK_EQUAL(7, u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "1357"_zv));
        RED_CHECK_EQUAL(1357, u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x1357"_zv));
        RED_CHECK_EQUAL(0x1357, u);

        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0A"_zv));
        RED_CHECK_EQUAL(0x0a, u);
        RED_CHECK(no_parse_error == parse_from_cfg(u, stype, "0x0a"_zv));
        RED_CHECK_EQUAL(0x0a, u);

        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "0x0000000I"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "-0"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "-1"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "0x"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, "I"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(u, stype, ""_zv));
    }

    // int
    {
        int i;
        configs::spec_type<int> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "3600"_zv));
        RED_CHECK_EQUAL(3600, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0xA3600"_zv));
        RED_CHECK_EQUAL(0xA3600, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0"_zv));
        RED_CHECK_EQUAL(0, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "-3600"_zv));
        RED_CHECK_EQUAL(-3600, i);

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "999999999999999999999999999999"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, ""_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-0xA3600"_zv));
    }

    // file_permission
    {
        uint32_t i;
        configs::spec_type<configs::spec_types::file_permission> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0000777"_zv));
        RED_CHECK_EQUAL(0777, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0000123"_zv));
        RED_CHECK_EQUAL(0123, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "0777"_zv));
        RED_CHECK_EQUAL(0777, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "777"_zv));
        RED_CHECK_EQUAL(0777, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "123"_zv));
        RED_CHECK_EQUAL(0123, i);
        RED_CHECK(no_parse_error == parse_from_cfg(i, stype, "23"_zv));
        RED_CHECK_EQUAL(023, i);

        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, ""_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "1234"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "288"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "-36"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(i, stype, "a66"_zv));
    }

    // fixed_binary
    {
        std::array<unsigned char, 4> d;
        configs::spec_type<configs::spec_types::fixed_binary> stype;

        RED_CHECK(no_parse_error == parse_from_cfg(d, stype, "12345678"_zv));
        RED_CHECK_EQUAL("\x12\x34\x56\x78"_av, d);
        RED_CHECK(no_parse_error == parse_from_cfg(d, stype, "abcdefab"_zv));
        RED_CHECK_EQUAL("\xAB\xCD\xEF\xAB"_av, d);

        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "ababab"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "ababababab"_zv));
        RED_CHECK(no_parse_error != parse_from_cfg(d, stype, "aa-bb-cc"_zv));
    }
}

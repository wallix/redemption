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
#include "configs/autogen/enums_func_ini.hpp"

RED_TEST_DELEGATE_PRINT(ServerCertCheck, long(x))
RED_TEST_DELEGATE_PRINT_NS(configs, parse_error, x.c_str())

RED_AUTO_TEST_CASE(TestParser)
{
    // increment
    {
        RED_CHECK( is_valid_enum_value(ServerCertCheck(3)));
        RED_CHECK(!is_valid_enum_value(ServerCertCheck(4)));

        ServerCertCheck v{};
        RED_CHECK(!parse(v, configs::spec_type<ServerCertCheck>{}, cstr_array_view("3")));
        RED_CHECK_EQ(v, ServerCertCheck(3));
        RED_CHECK(!!parse(v, configs::spec_type<ServerCertCheck>{}, cstr_array_view("4")));
        RED_CHECK_EQ(v, ServerCertCheck(3));

        auto x = ServerCertCheck(2);
        configs::zstr_buffer_from<ServerCertCheck> buf;
        auto av = configs::assign_zbuf_from_cfg(buf, configs::cfg_s_type<ServerCertCheck>{}, x);
        RED_CHECK_SMEM_C(av, "2");
    }

    // flags
    {
        RED_CHECK( is_valid_enum_value(CaptureFlags(15)));
        RED_CHECK(!is_valid_enum_value(CaptureFlags(16)));

        CaptureFlags v{};
        RED_CHECK(!parse(v, configs::spec_type<CaptureFlags>{}, cstr_array_view("15")));
        RED_CHECK_EQ(v, CaptureFlags(15));
        RED_CHECK(!!parse(v, configs::spec_type<CaptureFlags>{}, cstr_array_view("16")));

        auto x = CaptureFlags(15);
        configs::zstr_buffer_from<CaptureFlags> buf;
        auto av = configs::assign_zbuf_from_cfg(buf, configs::cfg_s_type<CaptureFlags>{}, x);
        RED_CHECK_SMEM_C(av, "15");
    }

    // str
    {
        RED_CHECK( is_valid_enum_value(RdpModeConsole(2)));
        RED_CHECK(!is_valid_enum_value(RdpModeConsole(3)));

        RdpModeConsole v{};
        RED_CHECK(!parse(v, configs::spec_type<RdpModeConsole>{}, cstr_array_view("forbid")));
        RED_CHECK_EQ(v, RdpModeConsole::forbid);
        RED_CHECK(!parse(v, configs::spec_type<RdpModeConsole>{}, cstr_array_view("aLloW")));
        RED_CHECK_EQ(v, RdpModeConsole::allow);
        RED_CHECK(!!parse(v, configs::spec_type<RdpModeConsole>{}, cstr_array_view("foRbI")));
        RED_CHECK_EQ(v, RdpModeConsole::allow);

        auto x = RdpModeConsole(2);
        configs::zstr_buffer_from<RdpModeConsole> buf;
        auto av = configs::assign_zbuf_from_cfg(buf, configs::cfg_s_type<RdpModeConsole>{}, x);
        RED_CHECK_SMEM_C(av, "forbid");
    }
}

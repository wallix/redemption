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
   Copyright (C) Wallix 2019
   Author(s): Meng Tan

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/rdp_negociation.hpp"

#include <string_view>

using namespace std::string_view_literals;

RED_AUTO_TEST_CASE(TestRdpLogonInfo)
{
    RED_TEST_DATAS(
        ("mer"sv, "mollusque@rocher"sv, ""sv, "mollusque@rocher"sv, false)
        ("mer"sv, "mollusque@rocher"sv, "rocher"sv, "mollusque"sv, true)
        ("what"sv, "tounicoti\\tournicoton"sv, "tounicoti"sv, "tournicoton"sv, false)
        ("what"sv, "tounicoti\\tournicoton"sv, "tounicoti"sv, "tournicoton"sv, true)
        ("ohnon"sv, "zigouigoui"sv, ""sv, "zigouigoui"sv, false)
        ("ohnon"sv, "zigouigoui"sv, ""sv, "zigouigoui"sv, true)
        ("sable"sv, "coquillage\\crustacé@plageabandonnée"sv,
            ""sv, "coquillage\\crustacé@plageabandonnée"sv, false)
        ("sable"sv, "coquillage\\crustacé@plageabandonnée"sv,
            "coquillage"sv, "crustacé@plageabandonnée"sv, true)
    ) >>= [&](
        std::string_view hostname,
        std::string_view target_user,
        std::string_view domain,
        std::string_view username,
        bool split_domain
    ){
        RdpLogonInfo logon_info(truncated_bounded_array_view(hostname), false, target_user.data(), split_domain);
        RED_CHECK(logon_info.domain() == domain);
        RED_CHECK(logon_info.username() == username);
    };
}

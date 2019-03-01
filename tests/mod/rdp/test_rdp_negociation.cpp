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
#include "test_only/test_framework/data_test_case.hpp"

#include "mod/rdp/rdp_negociation.hpp"

#include <array>


RED_BIND_DATA_TEST_CASE(TestRdpLogonInfo, (std::array{
    std::array{"mer", "mollusque@rocher", "", "mollusque@rocher"},
    std::array{"sable", "coquillage\\crustacé@plageabandonnée",
               "", "coquillage\\crustacé@plageabandonnée"},
    std::array{"what", "tounicoti\\tournicoton", "tounicoti", "tournicoton"},
    std::array{"ohnon", "zigouigoui", "", "zigouigoui"}
}), hostname, target_user, domain, username)
{
    RdpLogonInfo logon_info(hostname, false, target_user, false);
    RED_CHECK_EQUAL(logon_info.domain(), domain);
    RED_CHECK_EQUAL(logon_info.username(), username);
}

RED_BIND_DATA_TEST_CASE(TestRdpLogonInfoLegacy, (std::array{
    std::array{"mer", "mollusque@rocher", "rocher", "mollusque"},
    std::array{"sable", "coquillage\\crustacé@plageabandonnée",
               "coquillage", "crustacé@plageabandonnée"},
    std::array{"what", "tounicoti\\tournicoton", "tounicoti", "tournicoton"},
    std::array{"ohnon", "zigouigoui", "", "zigouigoui"}
}), hostname, target_user, domain, username)
{
    RdpLogonInfo logon_info(hostname, false, target_user, true);
    RED_CHECK_EQUAL(logon_info.domain(), domain);
    RED_CHECK_EQUAL(logon_info.username(), username);
}

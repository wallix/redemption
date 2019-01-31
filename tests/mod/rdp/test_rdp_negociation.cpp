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

#define RED_TEST_MODULE TestRdpNegociation
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/rdp_negociation.hpp"

RED_AUTO_TEST_CASE(TestRdpLogonInfo)
{
    {
        RdpLogonInfo logon_info("mer", false,
                                "mollusque@rocher", false);
        RED_CHECK_EQUAL(logon_info.domain(), "");
        RED_CHECK_EQUAL(logon_info.username(), "mollusque@rocher");
    }
    {
        RdpLogonInfo logon_info("sable", false,
                                "coquillage\\crustacé@plageabandonnée", false);
        RED_CHECK_EQUAL(logon_info.domain(), "");
        RED_CHECK_EQUAL(logon_info.username(),
                        "coquillage\\crustacé@plageabandonnée");
    }
    {
        RdpLogonInfo logon_info("what", false,
                                "tounicoti\\tournicoton", false);
        RED_CHECK_EQUAL(logon_info.domain(), "tounicoti");
        RED_CHECK_EQUAL(logon_info.username(), "tournicoton");
    }
    {
        RdpLogonInfo logon_info("ohnon", false,
                                "zigouigoui", false);
        RED_CHECK_EQUAL(logon_info.domain(), "");
        RED_CHECK_EQUAL(logon_info.username(), "zigouigoui");
    }
}

RED_AUTO_TEST_CASE(TestRdpLogonInfoLegacy)
{
    // Legacy Login Format
    {
        RdpLogonInfo logon_info("mer", false,
                                "mollusque@rocher", true);
        RED_CHECK_EQUAL(logon_info.domain(), "rocher");
        RED_CHECK_EQUAL(logon_info.username(), "mollusque");
    }
    {
        // This legacy behavior is incorrect
        RdpLogonInfo logon_info("sable", false,
                                "coquillage\\crustacé@plageabandonnée", true);
        RED_CHECK_EQUAL(logon_info.domain(), "coquillage");
        RED_CHECK_EQUAL(logon_info.username(),
                        "crustacé@plageabandonnée");
    }
    {
        RdpLogonInfo logon_info("what", false,
                                "tounicoti\\tournicoton", true);
        RED_CHECK_EQUAL(logon_info.domain(), "tounicoti");
        RED_CHECK_EQUAL(logon_info.username(), "tournicoton");
    }
    {
        RdpLogonInfo logon_info("ohnon", false,
                                "zigouigoui", true);
        RED_CHECK_EQUAL(logon_info.domain(), "");
        RED_CHECK_EQUAL(logon_info.username(), "zigouigoui");
    }

}

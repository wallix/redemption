/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/rdp_negociation.hpp"


RED_AUTO_TEST_CASE(TestRdpLogonInfo)
{
    RED_TEST_DATAS(
        ("mer"_av, "mollusque@rocher"_av, ""_av, "mollusque@rocher"_av, false)
        ("mer"_av, "mollusque@rocher"_av, "rocher"_av, "mollusque"_av, true)
        ("what"_av, "tounicoti\\tournicoton"_av, "tounicoti"_av, "tournicoton"_av, false)
        ("what"_av, "tounicoti\\tournicoton"_av, "tounicoti"_av, "tournicoton"_av, true)
        ("ohnon"_av, "zigouigoui"_av, ""_av, "zigouigoui"_av, false)
        ("ohnon"_av, "zigouigoui"_av, ""_av, "zigouigoui"_av, true)
        ("sable"_av, "coquillage\\crustacé@plageabandonnée"_av,
            ""_av, "coquillage\\crustacé@plageabandonnée"_av, false)
        ("sable"_av, "coquillage\\crustacé@plageabandonnée"_av,
            "coquillage"_av, "crustacé@plageabandonnée"_av, true)
    ) >>= [&](
        chars_view hostname,
        chars_view target_user,
        chars_view domain,
        chars_view username,
        bool split_domain
    ){
        RdpLogonInfo logon_info(truncated_bounded_array_view(hostname), false, target_user.data(), split_domain);
        RED_CHECK(logon_info.domain() == domain);
        RED_CHECK(logon_info.username() == username);
    };
}

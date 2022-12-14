/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/channels/sespro_channel_params.hpp"


RED_AUTO_TEST_CASE(TestExtraSystemProcesses)
{
    zstring_view rules;

    rules = "  abcd.exe, aaaaa.exe  ,   bbbb.exe,cccc.exe"_zv;
    RED_TEST_CONTEXT(rules) {
        ExtraSystemProcesses esp(rules);
        RED_CHECK(*esp.get(0) == "abcd.exe"_av);
        RED_CHECK(*esp.get(1) == "aaaaa.exe  "_av);
        RED_CHECK(*esp.get(2) == "bbbb.exe"_av);
        RED_CHECK(*esp.get(3) == "cccc.exe"_av);
        RED_CHECK(esp.get(4) == nullptr);
    }
}

RED_AUTO_TEST_CASE(TestOutboundConnectionMonitorRules)
{
    using Type = OutboundConnectionMonitorRules::Type;

    zstring_view rules;

    rules = "$deny:192.168.0.0/24:5900,$allow:192.168.0.110:21"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Deny);
        RED_CHECK(ocmr.get(0)->address() == "192.168.0.0/24"_av);
        RED_CHECK(ocmr.get(0)->port_range() == "5900"_av);
        RED_CHECK(ocmr.get(0)->description() == "$deny:192.168.0.0/24:5900"_av);

        RED_CHECK(ocmr.get(1)->type() == Type::Allow);
        RED_CHECK(ocmr.get(1)->address() == "192.168.0.110"_av);
        RED_CHECK(ocmr.get(1)->port_range() == "21"_av);
        RED_CHECK(ocmr.get(1)->description() == "$allow:192.168.0.110:21"_av);

        RED_CHECK(!ocmr.get(2));
    }

    rules = "$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389,$allow:[20D1:0:3238:DFE1:63::FEFB]:21"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Deny);
        RED_CHECK(ocmr.get(0)->address() == "2001:0db8:85a3:0000:0000:8a2e:0370:7334"_av);
        RED_CHECK(ocmr.get(0)->port_range() == "3389"_av);
        RED_CHECK(ocmr.get(0)->description() == "$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389"_av);

        RED_CHECK(ocmr.get(1)->type() == Type::Allow);
        RED_CHECK(ocmr.get(1)->address() == "20D1:0:3238:DFE1:63::FEFB"_av);
        RED_CHECK(ocmr.get(1)->port_range() == "21"_av);
        RED_CHECK(ocmr.get(1)->description() == "$allow:[20D1:0:3238:DFE1:63::FEFB]:21"_av);

        RED_CHECK(!ocmr.get(2));
    }

    rules = "$allow:host.domain.net:3389"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Allow);
        RED_CHECK(ocmr.get(0)->address() == "host.domain.net"_av);
        RED_CHECK(ocmr.get(0)->port_range() == "3389"_av);
        RED_CHECK(ocmr.get(0)->description() == "$allow:host.domain.net:3389"_av);

        RED_CHECK(!ocmr.get(1));
    }

    rules = "10.1.0.0/16:22"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Deny);
        RED_CHECK(ocmr.get(0)->address() == "10.1.0.0/16"_av);
        RED_CHECK(ocmr.get(0)->port_range() == "22"_av);
        RED_CHECK(ocmr.get(0)->description() == "10.1.0.0/16:22"_av);

        RED_CHECK(!ocmr.get(1));
    }

    // bad format
    rules = "$allow:[20D1:0:3238:DFE1:63::FEFB]"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Allow);
        RED_CHECK(ocmr.get(0)->address() == "20D1:0:3238:DFE1:63::FEFB"_av);
        RED_CHECK(ocmr.get(0)->port_range() == ""_av);
        RED_CHECK(ocmr.get(0)->description() == "$allow:[20D1:0:3238:DFE1:63::FEFB]"_av);

        RED_CHECK(!ocmr.get(1));
    }

    // bad format
    rules = "$allow:[20D1:0:3238:DFE1:63::FEFB:21"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Allow);
        RED_CHECK(ocmr.get(0)->address() == "[20D1:0:3238:DFE1:63::FEFB"_av);
        RED_CHECK(ocmr.get(0)->port_range() == "21"_av);
        RED_CHECK(ocmr.get(0)->description() == "$allow:[20D1:0:3238:DFE1:63::FEFB:21"_av);

        RED_CHECK(!ocmr.get(1));
    }

    // bad format
    rules = "10.1.0.0/16"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Deny);
        RED_CHECK(ocmr.get(0)->address() == "10.1.0.0/16"_av);
        RED_CHECK(ocmr.get(0)->port_range() == ""_av);
        RED_CHECK(ocmr.get(0)->description() == "10.1.0.0/16"_av);

        RED_CHECK(!ocmr.get(1));
    }

    // bad format
    rules = ":"_zv;
    RED_TEST_CONTEXT(rules) {
        OutboundConnectionMonitorRules ocmr(rules);

        RED_CHECK(ocmr.get(0)->type() == Type::Deny);
        RED_CHECK(ocmr.get(0)->address() == ":"_av);
        RED_CHECK(ocmr.get(0)->port_range() == ""_av);
        RED_CHECK(ocmr.get(0)->description() == ":"_av);

        RED_CHECK(!ocmr.get(1));
    }
}

RED_AUTO_TEST_CASE(TestProcessMonitorRules)
{
    using Type = ProcessMonitorRules::Type;

    zstring_view rules;

    rules = "$notify:task1,task2,$deny:task3"_zv;
    RED_TEST_CONTEXT(rules) {
        ProcessMonitorRules pmr(rules);

        RED_CHECK(pmr.get(0)->type() == Type::Notify);
        RED_CHECK(pmr.get(0)->pattern() == "task1"_av);
        RED_CHECK(pmr.get(0)->description() == "$notify:task1"_av);

        RED_CHECK(pmr.get(1)->type() == Type::Deny);
        RED_CHECK(pmr.get(1)->pattern() == "task2"_av);
        RED_CHECK(pmr.get(1)->description() == "task2"_av);

        RED_CHECK(pmr.get(2)->type() == Type::Deny);
        RED_CHECK(pmr.get(2)->pattern() == "task3"_av);
        RED_CHECK(pmr.get(2)->description() == "$deny:task3"_av);

        RED_CHECK(!pmr.get(3));
    }
}

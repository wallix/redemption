/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/channels/sespro_channel_params.hpp"


RED_AUTO_TEST_CASE(TestExtraSystemProcesses)
{
    ExtraSystemProcesses esp("  abcd.exe, aaaaa.exe  ,   bbbb.exe,cccc.exe"_zv);
    RED_CHECK(*esp.get(0) == "abcd.exe"_av);
    RED_CHECK(*esp.get(1) == "aaaaa.exe"_av);
    RED_CHECK(*esp.get(2) == "bbbb.exe"_av);
    RED_CHECK(*esp.get(3) == "cccc.exe"_av);
    RED_CHECK(esp.get(4) == nullptr);
}

RED_AUTO_TEST_CASE(TestOutboundConnectionMonitorRules)
{
    OutboundConnectionMonitorRules ocmr;

    unsigned    out_type;
    std::string out_host_address_or_subnet;
    std::string out_port_range;
    std::string out_description;

    ocmr = OutboundConnectionMonitorRules("$deny:192.168.0.0/24:5900,$allow:192.168.0.110:21"_zv);
    RED_CHECK(ocmr.get(0, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 1);
    RED_CHECK(out_host_address_or_subnet == "192.168.0.0/24"_av);
    RED_CHECK(out_port_range == "5900"_av);
    RED_CHECK(out_description == "$deny:192.168.0.0/24:5900"_av);
    RED_CHECK(ocmr.get(1, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 2);
    RED_CHECK(out_host_address_or_subnet == "192.168.0.110"_av);
    RED_CHECK(out_port_range == "21"_av);
    RED_CHECK(out_description == "$allow:192.168.0.110:21"_av);
    RED_CHECK(!ocmr.get(2, out_type, out_host_address_or_subnet, out_port_range, out_description));

    ocmr = OutboundConnectionMonitorRules("$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389,$allow:[20D1:0:3238:DFE1:63::FEFB]:21"_zv);
    RED_CHECK(ocmr.get(0, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 1);
    RED_CHECK(out_host_address_or_subnet == "2001:0db8:85a3:0000:0000:8a2e:0370:7334"_av);
    RED_CHECK(out_port_range == "3389"_av);
    RED_CHECK(out_description == "$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389"_av);
    RED_CHECK(ocmr.get(1, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 2);
    RED_CHECK(out_host_address_or_subnet == "20D1:0:3238:DFE1:63::FEFB"_av);
    RED_CHECK(out_port_range == "21"_av);
    RED_CHECK(out_description == "$allow:[20D1:0:3238:DFE1:63::FEFB]:21"_av);
    RED_CHECK(!ocmr.get(2, out_type, out_host_address_or_subnet, out_port_range, out_description));

    ocmr = OutboundConnectionMonitorRules("$allow:host.domain.net:3389"_zv);
    RED_CHECK(ocmr.get(0, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 2);
    RED_CHECK(out_host_address_or_subnet == "host.domain.net"_av);
    RED_CHECK(out_port_range == "3389"_av);
    RED_CHECK(out_description == "$allow:host.domain.net:3389"_av);
    RED_CHECK(!ocmr.get(1, out_type, out_host_address_or_subnet, out_port_range, out_description));

    ocmr = OutboundConnectionMonitorRules("10.1.0.0/16:22"_zv);
    RED_CHECK(ocmr.get(0, out_type, out_host_address_or_subnet, out_port_range, out_description));
    RED_CHECK(out_type == 1);
    RED_CHECK(out_host_address_or_subnet == "10.1.0.0/16"_av);
    RED_CHECK(out_port_range == "22"_av);
    RED_CHECK(out_description == "10.1.0.0/16:22"_av);
    RED_CHECK(!ocmr.get(1, out_type, out_host_address_or_subnet, out_port_range, out_description));
}

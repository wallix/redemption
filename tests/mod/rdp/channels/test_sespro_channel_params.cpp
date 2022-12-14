/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/compare_collection.hpp"

#include "mod/rdp/channels/sespro_channel_params.hpp"

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
#include <iomanip>
#endif


RED_AUTO_TEST_CASE(TestExtraSystemProcesses)
{
    ExtraSystemProcesses esp("  abcd.exe, aaaaa.exe  ,   bbbb.exe,cccc.exe"_zv);
    RED_CHECK(*esp.get(0) == "abcd.exe"_av);
    RED_CHECK(*esp.get(1) == "aaaaa.exe  "_av);
    RED_CHECK(*esp.get(2) == "bbbb.exe"_av);
    RED_CHECK(*esp.get(3) == "cccc.exe"_av);
    RED_CHECK(esp.get(4) == nullptr);
}

namespace
{
    struct OutboundConnectionMonitorRule
    {
        OutboundConnectionMonitorRule(OutboundConnectionMonitorRules::Rule const* rule = nullptr)
        : has_rule(rule)
        , type(rule ? rule->type() : OutboundConnectionMonitorRules::Type())
        , address(rule ? rule->address().as<std::string>() : std::string())
        , port_range(rule ? rule->port_range().as<std::string>() : std::string())
        , description(rule ? rule->description().as<std::string>() : std::string())
        {}

        OutboundConnectionMonitorRule(OutboundConnectionMonitorRules::Type type,
             std::string address,
             std::string port_range,
             std::string description)
        : has_rule(true)
        , type(type)
        , address(address)
        , port_range(port_range)
        , description(description)
        {}

        bool operator==(OutboundConnectionMonitorRule const& other) const
        {
            if (!has_rule || !other.has_rule) {
                return has_rule == other.has_rule;
            }
            return (
                type == other.type
             && address == other.address
             && port_range == other.port_range
             && description == other.description
            );
        }

        bool has_rule;
        OutboundConnectionMonitorRules::Type type;
        std::string address;
        std::string port_range;
        std::string description;
    };

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    static ut::assertion_result test_comp_ocm_rule(OutboundConnectionMonitorRule const& a, OutboundConnectionMonitorRule const& b)
    {
        auto put = [&](std::ostream& oss, OutboundConnectionMonitorRule const& rule){
            if (rule.has_rule) {
                oss
                  << "{.type=" << rule.type
                  << ", .address=" << rule.address
                  << ", .port_range=" << rule.port_range
                  << ", .description=" << rule.description
                  << "}"
                ;
            }
            else {
                oss << "nullptr";
            }
        };

        return ut::create_assertion_result(a == b, a, "!=", b, put);
    }
#endif
}

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
RED_TEST_DISPATCH_COMPARISON_EQ((), (::OutboundConnectionMonitorRule), (::OutboundConnectionMonitorRule), ::test_comp_ocm_rule)
#endif

RED_AUTO_TEST_CASE(TestOutboundConnectionMonitorRules)
{
    using Type = OutboundConnectionMonitorRules::Type;
    using Rule = OutboundConnectionMonitorRule;

    OutboundConnectionMonitorRules ocmr;

    ocmr = OutboundConnectionMonitorRules("$deny:192.168.0.0/24:5900,$allow:192.168.0.110:21"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Deny, "192.168.0.0/24", "5900", "$deny:192.168.0.0/24:5900"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule(Type::Allow, "192.168.0.110", "21", "$allow:192.168.0.110:21"));
    RED_CHECK(Rule(ocmr.get(2)) == Rule());

    ocmr = OutboundConnectionMonitorRules("$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389,$allow:[20D1:0:3238:DFE1:63::FEFB]:21"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Deny, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", "3389", "$deny:2001:0db8:85a3:0000:0000:8a2e:0370:7334:3389"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule(Type::Allow, "20D1:0:3238:DFE1:63::FEFB", "21", "$allow:[20D1:0:3238:DFE1:63::FEFB]:21"));
    RED_CHECK(Rule(ocmr.get(2)) == Rule());

    ocmr = OutboundConnectionMonitorRules("$allow:host.domain.net:3389"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Allow, "host.domain.net", "3389", "$allow:host.domain.net:3389"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());

    ocmr = OutboundConnectionMonitorRules("10.1.0.0/16:22"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Deny, "10.1.0.0/16", "22", "10.1.0.0/16:22"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());

    // bad format
    ocmr = OutboundConnectionMonitorRules("$allow:[20D1:0:3238:DFE1:63::FEFB]"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Allow, "20D1:0:3238:DFE1:63::FEFB", "", "$allow:[20D1:0:3238:DFE1:63::FEFB]"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());

    // bad format
    ocmr = OutboundConnectionMonitorRules("$allow:[20D1:0:3238:DFE1:63::FEFB:21"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Allow, "[20D1:0:3238:DFE1:63::FEFB", "21", "$allow:[20D1:0:3238:DFE1:63::FEFB:21"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());

    // bad format
    ocmr = OutboundConnectionMonitorRules("10.1.0.0/16"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Deny, "10.1.0.0/16", "", "10.1.0.0/16"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());

    // bad format
    ocmr = OutboundConnectionMonitorRules(":"_zv);
    RED_CHECK(Rule(ocmr.get(0)) == Rule(Type::Deny, ":", "", ":"));
    RED_CHECK(Rule(ocmr.get(1)) == Rule());
}

RED_AUTO_TEST_CASE(TestProcessMonitorRules)
{
    using Type = ProcessMonitorRules::Type;

    ProcessMonitorRules pmr("$notify:task1,task2,$deny:task3"_zv);

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

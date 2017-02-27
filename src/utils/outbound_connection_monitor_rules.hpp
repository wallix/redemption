/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once

#include "utils/sugar/make_unique.hpp"

#include <string>
#include <vector>

#include <cstring>


class OutboundConnectionMonitorRules
{
    struct outbound_connection_monitor_rule
    {
        unsigned type;
        std::string address;
        std::string port_range;
        std::string description;
    };

    std::vector<outbound_connection_monitor_rule> rules;

public:
    OutboundConnectionMonitorRules(
        const char * comme_separated_monitoring_rules
    ) {
        if (comme_separated_monitoring_rules) {
            const char * rule = comme_separated_monitoring_rules;

            char const RULE_PREFIX_ALLOW[]  = "$allow:";
            char const RULE_PREFIX_NOTIFY[] = "$notify:";
            char const RULE_PREFIX_DENY[]   = "$deny:";

            while (*rule) {
                if ((*rule == ',') || (*rule == '\t') || (*rule == ' ')) {
                    rule++;
                    continue;
                }

                char const * rule_begin = rule;

                unsigned uType = 1; // Deny
                if (strcasestr(rule, RULE_PREFIX_ALLOW) == rule)
                {
                    uType  = 2;                             // Allow
                    rule  += sizeof(RULE_PREFIX_ALLOW) - 1;
                }
                else if (strcasestr(rule, RULE_PREFIX_NOTIFY) == rule)
                {
                    uType  = 0;                             // Notify
                    rule  += sizeof(RULE_PREFIX_NOTIFY) - 1;
                }
                else if (strcasestr(rule, RULE_PREFIX_DENY) == rule)
                {
                    uType  = 1;                             // Deny
                    rule  += sizeof(RULE_PREFIX_DENY) - 1;
                }

                const char * rule_separator = strchr(rule, ',');

                std::string description_string(rule_begin, (rule_separator ? rule_separator - rule_begin : ::strlen(rule_begin)));

                std::string rule_string(rule, (rule_separator ? rule_separator - rule : ::strlen(rule)));

                const char * rule_c_str = rule_string.c_str();

                const char * info_separator = strchr(rule_c_str, ':');

                if (info_separator)
                {
                    std::string host_address_or_subnet(rule_c_str, info_separator - rule_c_str);

                    this->rules.push_back({
                        uType, std::move(host_address_or_subnet), std::string(info_separator + 1),
                        std::move(description_string)
                    });
                }

                if (!rule_separator) {
                    break;
                }

                rule = rule_separator + 1;
            }
        }
    }

    bool get(
        size_t index,
        unsigned int & out__type,
        std::string & out__host_address_or_subnet,
        std::string & out__port_range,
        std::string & out__description
    ) {
        if (this->rules.size() <= index) {
            out__type = 0;
            out__host_address_or_subnet.clear();
            out__port_range.clear();
            out__description.clear();

            return false;
        }

        out__type                   = this->rules[index].type;
        out__host_address_or_subnet = this->rules[index].address;
        out__port_range             = this->rules[index].port_range;
        out__description            = this->rules[index].description;

        return true;
    }
};

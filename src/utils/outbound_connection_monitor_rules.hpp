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
#include <tuple>
#include <vector>

#include <cstring>


class OutboundConnectionMonitorRules
{
    struct outbound_connection_monitor_rule
    {
        unsigned type;
        std::string address;
        unsigned port;
    };

    std::vector<outbound_connection_monitor_rule> rules;

public:
    OutboundConnectionMonitorRules(
        const char * comma_separated_notify_rules,
        const char * comme_separated_block_rules
    ) {
        (void)comma_separated_notify_rules;
        if (comme_separated_block_rules) {
            const char * rule = comme_separated_block_rules;

            while (*rule) {
                if ((*rule == ',') || (*rule == '\t') || (*rule == ' ')) {
                    rule++;
                    continue;
                }

                const char * rule_separator = strchr(rule, ',');

                std::string rule_string(rule, (rule_separator ? rule_separator - rule : ::strlen(rule)));

                const char * rule_c_str = rule_string.c_str();

                const char * info_separator = strchr(rule_c_str, ':');

                if (info_separator)
                {
                    std::string host_address_or_subnet(rule_c_str, info_separator - rule_c_str);

                    this->rules.push_back({
                        1, std::move(host_address_or_subnet), static_cast<unsigned>(::atoi(info_separator + 1))
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
        unsigned int & out__port
    ) {
        if (this->rules.size() <= index) {
            out__type                   = 0;
            out__host_address_or_subnet.clear();
            out__port                   = 0;

            return false;
        }

        out__type                   = this->rules[index].type;
        out__host_address_or_subnet = this->rules[index].address;
        out__port                   = this->rules[index].port;

        return true;
    }
};

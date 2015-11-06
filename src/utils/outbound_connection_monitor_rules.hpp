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

#ifndef _REDEMPTION_UTILS_OUTBOUNDCONNECTIONMONITORRULES_HPP_
#define _REDEMPTION_UTILS_OUTBOUNDCONNECTIONMONITORRULES_HPP_

#include <string.h>

class OutboundConnectionMonitorRules {
public:
    // Type, host address or subnet, and port.
    typedef std::tuple<unsigned int, std::unique_ptr<std::string>, unsigned int>
        outbound_connection_monitor_rule;

private:
    typedef std::vector<outbound_connection_monitor_rule>
        outbound_connection_monitor_rule_collection;

    outbound_connection_monitor_rule_collection rules;

public:
    OutboundConnectionMonitorRules(const char * soh_separated_notify_rules,
            const char * soh_separated_kill_rules) {
        char prefix[] = "$ocm:";

        if (soh_separated_kill_rules) {
            const char * rule = soh_separated_kill_rules;

            while (*rule) {
                if ((*rule == '\x01') || (*rule == '\t') || (*rule == ' ')) {
                    rule++;
                    continue;
                }

                const char * rule_separator = strchr(rule, '\x01');

                if (strcasestr(rule, prefix) == rule) {
                    rule += (sizeof(prefix) - 1);

                    std::string rule_string(rule, (rule_separator ? rule_separator - rule : ::strlen(rule)));

                    const char * rule_c_str = rule_string.c_str();

                    const char * info_separator = strchr(rule_c_str, ':');

                    if (info_separator)
                    {
                        std::unique_ptr<std::string> host_address_or_subnet =
                            std::make_unique<std::string>(rule_c_str, info_separator - rule_c_str);

                        this->rules.push_back(
                                std::make_tuple(
                                        1, std::move(host_address_or_subnet), ::atoi(info_separator + 1)
                                    )
                            );
                    }
                }

                if (!rule_separator) {
                    break;
                }

                rule = rule_separator + 1;
            }
        }
    }

    bool get(size_t index, unsigned int & out__type,
            std::string & out__host_address_or_subnet,
            unsigned int & out__port) {
        if (this->rules.size() <= index) {
            out__type                   = 0;
            out__host_address_or_subnet = "";
            out__port                   = 0;

            return false;
        }

        out__type                   = std::get<0>(this->rules[index]);
        out__host_address_or_subnet = std::get<1>(this->rules[index])->c_str();
        out__port                   = std::get<2>(this->rules[index]);

        return true;
    }
};

#endif  // #ifndef _REDEMPTION_UTILS_OUTBOUNDCONNECTIONMONITORRULES_HPP_

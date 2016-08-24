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


class ProcessMonitorRules
{
    struct process_monitor_rule
    {
        unsigned    type;
        std::string pattern;
        std::string description;
    };

    std::vector<process_monitor_rule> rules;

public:
    ProcessMonitorRules(const char * comme_separated_rules) {
        if (comme_separated_rules) {
            const char * rule = comme_separated_rules;

            char const RULE_PREFIX_NOTIFY[] = "$notify:";
            char const RULE_PREFIX_DENY[]   = "$deny:";

            while (*rule) {
                if ((*rule == ',') || (*rule == '\t') || (*rule == ' ')) {
                    rule++;
                    continue;
                }

                char const * rule_begin = rule;

                unsigned uType = 1; // Deny
                if (strcasestr(rule, RULE_PREFIX_NOTIFY) == rule)
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

                std::string pattern(rule, (rule_separator ? rule_separator - rule : ::strlen(rule)));

                this->rules.push_back({
                    uType, std::move(pattern), std::move(description_string)
                });

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
        std::string & out__pattern,
        std::string & out__description
    ) {
        if (this->rules.size() <= index) {
            out__type = 0;
            out__pattern.clear();
            out__description.clear();

            return false;
        }

        out__type                   = this->rules[index].type;
        out__pattern                = this->rules[index].pattern;
        out__description            = this->rules[index].description;

        return true;
    }
};

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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "mod/rdp/channels/sespro_channel_params.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "utils/sugar/split.hpp"
#include "utils/strutils.hpp"

#include <iterator>

#include <cassert>


ExtraSystemProcesses::ExtraSystemProcesses(zstring_view comma_separated_processes)
{
    for (auto process : split_with(comma_separated_processes, ',')) {
        process = trim(process);
        if (!process.empty()) {
            this->processes.emplace_back(process.begin(), process.end());
        }
    }
}

std::string const* ExtraSystemProcesses::get(size_t index) const
{
    return (index < this->processes.size())
        ? &this->processes[index]
        : nullptr;
}

std::string ExtraSystemProcesses::to_string() const
{
    std::string r;
    for (auto& x : this->processes) {
        str_append(r, x, ", ");
    }
    return r;
}


OutboundConnectionMonitorRules::OutboundConnectionMonitorRules(
    zstring_view comma_separated_monitoring_rules
) {
    for (auto rule : split_with(comma_separated_monitoring_rules, ',')) {
        rule = trim(rule);
        if (REDEMPTION_UNLIKELY(rule.empty())) {
            continue;
        }

        Type type = Type::Deny;
        chars_view addr = rule;

        struct D { Type type; chars_view prefix; };
        for (D d : {
            D{Type::Allow, "$allow:"_zv},
            D{Type::Notify, "$notify:"_zv},
            D{Type::Deny, "$deny:"_zv},
        }) {
            if (utils::starts_with(rule, d.prefix)) {
                type = d.type;
                addr = rule.drop_front(d.prefix.size());
                break;
            }
        }

        if (REDEMPTION_UNLIKELY(addr.empty())) {
            continue;
        }

        auto reverse_find = [](char const* first, char const* last, char c){
            while (first < --last) {
                if (*last == c) {
                    return last;
                }
            }
            return first;
        };

        chars_view host_address_or_subnet = addr;
        char const* port_sep = addr.end();

        char const* start = addr.begin() + 1;
        char const* end_bracket;

        if (addr[0] == '[' && start != (end_bracket = reverse_find(start, addr.end(), ']'))) {
            host_address_or_subnet = {start, end_bracket};
            if (end_bracket[1] == ':') {
                port_sep = end_bracket + 2;
            }
        }
        else {
            char const* pos = reverse_find(start, addr.end(), ':');
            if (pos != start) {
                port_sep = pos + 1;
                host_address_or_subnet = {addr.begin(), pos};
            }
        }

        this->rules.push_back({
            type,
            host_address_or_subnet.as<std::string>(),
            chars_view{port_sep, addr.end()}.as<std::string>(),
            rule.as<std::string>(),
        });
    }
}

// TODO {std:string const&, bool} ?
bool OutboundConnectionMonitorRules::get(
    size_t index,
    unsigned int & out_type,
    std::string & out_host_address_or_subnet,
    std::string & out_port_range,
    std::string & out_description
) const {
    if (this->rules.size() <= index) {
        out_type = 0;
        out_host_address_or_subnet.clear();
        out_port_range.clear();
        out_description.clear();

        return false;
    }

    out_type                   = unsigned(this->rules[index].type);
    out_host_address_or_subnet = this->rules[index].address;
    out_port_range             = this->rules[index].port_range;
    out_description            = this->rules[index].description;

    return true;
}

std::string OutboundConnectionMonitorRules::to_string() const
{
    std::string r;
    constexpr chars_view type_s[]{
        "Notify"_av,
        "Deny"_av,
        "Allow"_av,
    };
    for (auto& x : this->rules) {
        assert(unsigned(x.type) < std::size(type_s));
        str_append(r, '{',
            type_s[unsigned(x.type)], ", ",
            x.address, ", ",
            x.port_range, ", ",
            x.description, "}, "
        );
    }
    return r;
}


ProcessMonitorRules::ProcessMonitorRules(const char * comme_separated_rules)
{
    if (comme_separated_rules) {
        const char * rule = comme_separated_rules;

        auto RULE_PREFIX_NOTIFY = "$notify:"_zv;
        auto RULE_PREFIX_DENY   = "$deny:"_zv;

        while (*rule) {
            if ((*rule == ',') || (*rule == '\t') || (*rule == ' ')) {
                rule++;
                continue;
            }

            char const * rule_begin = rule;

            Type uType = Type::Deny;
            if (strcasestr(rule, RULE_PREFIX_NOTIFY.c_str()) == rule)
            {
                uType  = Type::Notify;
                rule  += RULE_PREFIX_NOTIFY.size();
            }
            else if (strcasestr(rule, RULE_PREFIX_DENY.c_str()) == rule)
            {
                uType  = Type::Deny;
                rule  += RULE_PREFIX_DENY.size();
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

// TODO {std:string const&, bool} ?
bool ProcessMonitorRules::get(
    size_t index,
    unsigned int & out_type,
    std::string & out_pattern,
    std::string & out_description
) const {
    if (this->rules.size() <= index) {
        out_type = 0;
        out_pattern.clear();
        out_description.clear();

        return false;
    }

    out_type                   = unsigned(this->rules[index].type);
    out_pattern                = this->rules[index].pattern;
    out_description            = this->rules[index].description;

    return true;
}

std::string ProcessMonitorRules::to_string() const
{
    std::string r;
    constexpr chars_view type_s[]{
        "Notify"_av,
        "Deny"_av,
    };
    for (auto& x : this->rules) {
        assert(unsigned(x.type) < std::size(type_s));
        str_append(r, '{',
            type_s[unsigned(x.type)], ", ",
            x.pattern, ", ",
            x.description, "}, "
        );
    }
    return r;
}

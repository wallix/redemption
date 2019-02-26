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

#pragma once

#include "configs/autogen/enums.hpp"
#include "utils/sugar/algostring.hpp"

#include <chrono>
#include <string>
#include <vector>
#include <iterator>

#include <cstdint>
#include <cassert>


class ExtraSystemProcesses
{
    std::vector<std::string> processes;

public:
    explicit ExtraSystemProcesses() = default;

    explicit ExtraSystemProcesses(const char * comme_separated_processes)
    {
        if (comme_separated_processes) {
            const char * process = comme_separated_processes;

            while (*process) {
                if ((*process == ',') || (*process == '\t') || (*process == ' ')) {
                    process++;
                    continue;
                }

                char const * process_begin = process;

                const char * process_separator = strchr(process, ',');

                this->processes.emplace_back(
                    process_begin,
                    (process_separator ? process_separator - process_begin : ::strlen(process_begin))
                );

                if (!process_separator) {
                    break;
                }

                process = process_separator + 1;
            }
        }
    }

    bool get(
        size_t index,
        std::string & out__name
    ) const {
        if (this->processes.size() <= index) {
            out__name.clear();

            return false;
        }

        out__name = this->processes[index];

        return true;
    }

    std::string to_string() const
    {
        std::string r;
        for (auto& x : this->processes) {
            str_append(r, x, ", ");
        }
        return r;
    }
};


class OutboundConnectionMonitorRules
{
    enum class Type : unsigned char
    {
        Notify,
        Deny,
        Allow,
    };

    struct outbound_connection_monitor_rule
    {
        Type type;
        std::string address;
        std::string port_range;
        std::string description;
    };

    std::vector<outbound_connection_monitor_rule> rules;

public:
    explicit OutboundConnectionMonitorRules() = default;

    explicit OutboundConnectionMonitorRules(
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

                Type uType = Type::Deny;
                if (strcasestr(rule, RULE_PREFIX_ALLOW) == rule)
                {
                    uType  = Type::Allow;
                    rule  += sizeof(RULE_PREFIX_ALLOW) - 1;
                }
                else if (strcasestr(rule, RULE_PREFIX_NOTIFY) == rule)
                {
                    uType  = Type::Notify;
                    rule  += sizeof(RULE_PREFIX_NOTIFY) - 1;
                }
                else if (strcasestr(rule, RULE_PREFIX_DENY) == rule)
                {
                    uType  = Type::Deny;
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
                        uType,
                        std::move(host_address_or_subnet),
                        std::string(info_separator + 1),
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
        // TODO enum class
        unsigned int & out__type,
        std::string & out__host_address_or_subnet,
        std::string & out__port_range,
        std::string & out__description
    ) const {
        if (this->rules.size() <= index) {
            out__type = 0;
            out__host_address_or_subnet.clear();
            out__port_range.clear();
            out__description.clear();

            return false;
        }

        out__type                   = unsigned(this->rules[index].type);
        out__host_address_or_subnet = this->rules[index].address;
        out__port_range             = this->rules[index].port_range;
        out__description            = this->rules[index].description;

        return true;
    }

    std::string to_string() const
    {
        std::string r;
        constexpr array_view_const_char type_s[]{
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
};


class ProcessMonitorRules
{
    enum class Type : bool
    {
        Notify,
        Deny,
    };

    struct process_monitor_rule
    {
        Type type;
        std::string pattern;
        std::string description;
    };

    std::vector<process_monitor_rule> rules;

public:
    explicit ProcessMonitorRules() = default;

    explicit ProcessMonitorRules(const char * comme_separated_rules)
    {
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

                Type uType = Type::Deny;
                if (strcasestr(rule, RULE_PREFIX_NOTIFY) == rule)
                {
                    uType  = Type::Notify;
                    rule  += sizeof(RULE_PREFIX_NOTIFY) - 1;
                }
                else if (strcasestr(rule, RULE_PREFIX_DENY) == rule)
                {
                    uType  = Type::Deny;
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
        // TODO enum class
        unsigned int & out__type,
        std::string & out__pattern,
        std::string & out__description
    ) const {
        if (this->rules.size() <= index) {
            out__type = 0;
            out__pattern.clear();
            out__description.clear();

            return false;
        }

        out__type                   = unsigned(this->rules[index].type);
        out__pattern                = this->rules[index].pattern;
        out__description            = this->rules[index].description;

        return true;
    }

    std::string to_string() const
    {
        std::string r;
        constexpr array_view_const_char type_s[]{
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
};


struct SessionProbeVirtualChannelParams
{
    ExtraSystemProcesses           extra_system_processes;
    OutboundConnectionMonitorRules outbound_connection_monitor_rules;
    ProcessMonitorRules            process_monitor_rules;
    ExtraSystemProcesses           windows_of_these_applications_as_unidentified_input_field;

    std::chrono::milliseconds effective_launch_timeout {};
    std::chrono::milliseconds keepalive_timeout {};

    std::chrono::milliseconds disconnected_application_limit {};
    std::chrono::milliseconds disconnected_session_limit {};
    std::chrono::milliseconds idle_session_limit {};

    SessionProbeOnLaunchFailure on_launch_failure = SessionProbeOnLaunchFailure::disconnect_user;

    SessionProbeOnKeepaliveTimeout on_keepalive_timeout = SessionProbeOnKeepaliveTimeout::disconnect_user;

    uint32_t handle_usage_limit = 0;
    uint32_t memory_usage_limit = 0;

    bool enable_log = false;
    bool enable_log_rotation = true;

    bool allow_multiple_handshake = false;

    bool enable_crash_dump = false;

    bool ignore_ui_less_processes_during_end_of_session_check = true;

    bool childless_window_as_unidentified_input_field = true;

    bool end_disconnected_session = false;

    explicit SessionProbeVirtualChannelParams() {}
};


struct SessionProbeClipboardBasedLauncherParams
{
    std::chrono::milliseconds clipboard_initialization_delay_ms{};
    std::chrono::milliseconds start_delay_ms{};
    std::chrono::milliseconds long_delay_ms{};
    std::chrono::milliseconds short_delay_ms{};
};

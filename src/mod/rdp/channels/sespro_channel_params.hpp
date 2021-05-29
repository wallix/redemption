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

    explicit ExtraSystemProcesses(const char * comme_separated_processes);

    bool get(
        size_t index,
        std::string & out_name
    ) const;

    [[nodiscard]] std::string to_string() const;
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
    );

    bool get(
        size_t index,
        unsigned int & out_type,
        std::string & out_host_address_or_subnet,
        std::string & out_port_range,
        std::string & out_description
    ) const;

    [[nodiscard]] std::string to_string() const;
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

    explicit ProcessMonitorRules(const char * comme_separated_rules);

    bool get(
        size_t index,
        unsigned int & out_type,
        std::string & out_pattern,
        std::string & out_description
    ) const;

    [[nodiscard]] std::string to_string() const;
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

    SessionProbeDisabledFeature disabled_features = SessionProbeDisabledFeature::none;

    bool bestsafe_integration = false;

    bool enable_log = false;
    bool enable_log_rotation = true;
    SessionProbeLogLevel log_level = SessionProbeLogLevel::Debug;

    bool allow_multiple_handshake = false;

    bool enable_crash_dump = false;

    std::chrono::milliseconds end_of_session_check_delay_time {};

    bool ignore_ui_less_processes_during_end_of_session_check = true;
    bool update_disabled_features                             = true;

    bool childless_window_as_unidentified_input_field = true;

    bool end_disconnected_session = false;

    std::chrono::milliseconds launcher_abort_delay {};

    bool session_shadowing_support = true;

    explicit SessionProbeVirtualChannelParams() = default;

    SessionProbeOnAccountManipulation on_account_manipulation = SessionProbeOnAccountManipulation::allow;

    bool at_end_of_session_freeze_connection_and_wait = true;

    bool launch_application_driver                = false;
    bool launch_application_driver_then_terminate = false;

    bool enable_self_cleaner = false;
};


struct SessionProbeClipboardBasedLauncherParams
{
    std::chrono::milliseconds clipboard_initialization_delay_ms{};
    std::chrono::milliseconds start_delay_ms{};
    std::chrono::milliseconds long_delay_ms{};
    std::chrono::milliseconds short_delay_ms{};

    bool reset_keyboard_status = true;
};

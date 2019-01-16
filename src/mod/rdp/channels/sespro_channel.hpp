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

#include "gdi/screen_functions.hpp"
#include "core/error.hpp"
#include "core/front_api.hpp"
#include "core/session_reactor.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "utils/genrandom.hpp"
#include "utils/parse_server_message.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/algostring.hpp"

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cinttypes> // PRId64, ...
#include <cstring>

enum {
    INVALID_RECONNECTION_COOKIE = 0xFFFFFFFF
};

// Proxy Options
enum {
    OPTION_DELAY_DISABLED_LAUNCH_MASK = 0x00000001
};

// Session Probe Options
enum {
    OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK = 0x00000001
};

class ExtraSystemProcesses
{
    std::vector<std::string> processes;

public:
    explicit ExtraSystemProcesses(const char * comme_separated_processes) {
        if (comme_separated_processes) {
            const char * process = comme_separated_processes;

            while (*process) {
                if ((*process == ',') || (*process == '\t') || (*process == ' ')) {
                    process++;
                    continue;
                }

                char const * process_begin = process;

                const char * process_separator = strchr(process, ',');

                std::string name_string(process_begin, (process_separator ? process_separator - process_begin : ::strlen(process_begin)));

                this->processes.push_back(std::move(name_string));

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
    ) {
        if (this->processes.size() <= index) {
            out__name.clear();

            return false;
        }

        out__name = this->processes[index];

        return true;
    }
};


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


class SessionProbeVirtualChannel final : public BaseVirtualChannel
{
private:
    bool session_probe_ending_in_progress  = false;
    bool session_probe_keep_alive_received = true;
    bool session_probe_ready               = false;

    bool session_probe_launch_timeout_timer_started = false;

    const std::chrono::milliseconds session_probe_effective_launch_timeout;

    const std::chrono::milliseconds param_session_probe_keepalive_timeout;

    const SessionProbeOnKeepaliveTimeout param_session_probe_on_keepalive_timeout;

    const SessionProbeOnLaunchFailure param_session_probe_on_launch_failure;

    const bool     param_session_probe_end_disconnected_session;

    std::string    param_target_informations;

    const uint16_t param_front_width;
    const uint16_t param_front_height;

    const std::chrono::milliseconds param_session_probe_disconnected_application_limit;
    const std::chrono::milliseconds param_session_probe_disconnected_session_limit;
    const std::chrono::milliseconds param_session_probe_idle_session_limit;

    const bool param_session_probe_enable_log;
    const bool param_session_probe_enable_log_rotation;

    std::string param_real_alternate_shell;
    std::string param_real_working_dir;

    Translation::language_t param_lang;

    const bool param_bogus_refresh_rect_ex;

    const bool param_show_maximized;

    const bool param_allow_multiple_handshake;

    const bool param_enable_crash_dump;

    const uint32_t param_handle_usage_limit;
    const uint32_t param_memory_usage_limit;

    const bool param_session_probe_ignore_ui_less_processes_during_end_of_session_check;

    const bool param_session_probe_childless_window_as_unidentified_input_field;

    FrontAPI& front;

    mod_api& mod;
    rdp_api& rdp;

    FileSystemVirtualChannel& file_system_virtual_channel;

    ExtraSystemProcesses           extra_system_processes;
    OutboundConnectionMonitorRules outbound_connection_monitor_rules;
    ProcessMonitorRules            process_monitor_rules;
    ExtraSystemProcesses           windows_of_these_applications_as_unidentified_input_field;

    bool disconnection_reconnection_required = false; // Cause => Authenticated user changed.

    SessionProbeLauncher* session_probe_stop_launch_sequence_notifier = nullptr;

    std::string server_message;

    uint16_t other_version = 0x0100;

    bool start_application_query_processed = false;
    bool start_application_started         = false;

    Random & gen;

    uint32_t reconnection_cookie = INVALID_RECONNECTION_COOKIE;

    SessionReactor& session_reactor;
    SessionReactor::TimerPtr session_probe_timer;
    SessionReactor::GraphicEventPtr freeze_mod_screen;

    uint32_t options = 0;

    static long long ms2ll(std::chrono::milliseconds const& ms)
    {
        return ms.count();
    }

public:
    struct Params : public BaseVirtualChannel::Params
    {
        uninit_checked<std::chrono::milliseconds> session_probe_launch_timeout;
        uninit_checked<std::chrono::milliseconds> session_probe_launch_fallback_timeout;
        uninit_checked<std::chrono::milliseconds> session_probe_keepalive_timeout;

        uninit_checked<SessionProbeOnKeepaliveTimeout> session_probe_on_keepalive_timeout;

        uninit_checked<SessionProbeOnLaunchFailure> session_probe_on_launch_failure;

        uninit_checked<bool> session_probe_end_disconnected_session;

        uninit_checked<const char*> target_informations;

        uninit_checked<uint16_t> front_width;
        uninit_checked<uint16_t> front_height;

        uninit_checked<std::chrono::milliseconds> session_probe_disconnected_application_limit;
        uninit_checked<std::chrono::milliseconds> session_probe_disconnected_session_limit;
        uninit_checked<std::chrono::milliseconds> session_probe_idle_session_limit;

        uninit_checked<bool> session_probe_enable_log;
        uninit_checked<bool> session_probe_enable_log_rotation;

        uninit_checked<const char*> real_alternate_shell;
        uninit_checked<const char*> real_working_dir;

        uninit_checked<const char*> session_probe_extra_system_processes;

        uninit_checked<const char*> session_probe_outbound_connection_monitoring_rules;

        uninit_checked<const char*> session_probe_process_monitoring_rules;

        uninit_checked<const char*> session_probe_windows_of_these_applications_as_unidentified_input_field;

        uninit_checked<bool> session_probe_allow_multiple_handshake;

        uninit_checked<bool> session_probe_enable_crash_dump;

        uninit_checked<uint32_t> session_probe_handle_usage_limit;
        uninit_checked<uint32_t> session_probe_memory_usage_limit;

        uninit_checked<bool> session_probe_ignore_ui_less_processes_during_end_of_session_check;

        uninit_checked<bool> session_probe_childless_window_as_unidentified_input_field;

        uninit_checked<Translation::language_t> lang;

        uninit_checked<bool> bogus_refresh_rect_ex;

        uninit_checked<bool> show_maximized;

        explicit Params(ReportMessageApi & report_message)
          : BaseVirtualChannel::Params(report_message)
        {}
    };

    explicit SessionProbeVirtualChannel(
        SessionReactor& session_reactor,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        mod_api& mod,
        rdp_api& rdp,
        FileSystemVirtualChannel& file_system_virtual_channel,
        Random & gen,
        const Params& params)
    : BaseVirtualChannel(nullptr,
                         to_server_sender_,
                         params)
    , session_probe_effective_launch_timeout(
            (params.session_probe_on_launch_failure ==
             SessionProbeOnLaunchFailure::disconnect_user) ?
            params.session_probe_launch_timeout :
            params.session_probe_launch_fallback_timeout
        )
    , param_session_probe_keepalive_timeout(
          params.session_probe_keepalive_timeout)
    , param_session_probe_on_keepalive_timeout(
          params.session_probe_on_keepalive_timeout)
    , param_session_probe_on_launch_failure(
          params.session_probe_on_launch_failure)
    , param_session_probe_end_disconnected_session(
          params.session_probe_end_disconnected_session)
    , param_target_informations(params.target_informations)
    , param_front_width(params.front_width)
    , param_front_height(params.front_height)
    , param_session_probe_disconnected_application_limit(
        params.session_probe_disconnected_application_limit)
    , param_session_probe_disconnected_session_limit(
        params.session_probe_disconnected_session_limit)
    , param_session_probe_idle_session_limit(
        params.session_probe_idle_session_limit)
    , param_session_probe_enable_log(params.session_probe_enable_log)
    , param_session_probe_enable_log_rotation(params.session_probe_enable_log_rotation)
    , param_real_alternate_shell(params.real_alternate_shell)
    , param_real_working_dir(params.real_working_dir)
    , param_lang(params.lang)
    , param_bogus_refresh_rect_ex(params.bogus_refresh_rect_ex)
    , param_show_maximized(params.show_maximized)
    , param_allow_multiple_handshake(params.session_probe_allow_multiple_handshake)
    , param_enable_crash_dump(params.session_probe_enable_crash_dump)
    , param_handle_usage_limit(params.session_probe_handle_usage_limit)
    , param_memory_usage_limit(params.session_probe_memory_usage_limit)
    , param_session_probe_ignore_ui_less_processes_during_end_of_session_check(params.session_probe_ignore_ui_less_processes_during_end_of_session_check)
    , param_session_probe_childless_window_as_unidentified_input_field(params.session_probe_childless_window_as_unidentified_input_field)
    , front(front)
    , mod(mod)
    , rdp(rdp)
    , file_system_virtual_channel(file_system_virtual_channel)
    , extra_system_processes(params.session_probe_extra_system_processes)
    , outbound_connection_monitor_rules(
          params.session_probe_outbound_connection_monitoring_rules)
    , process_monitor_rules(
          params.session_probe_process_monitoring_rules)
    , windows_of_these_applications_as_unidentified_input_field(params.session_probe_windows_of_these_applications_as_unidentified_input_field)
    , gen(gen)
    , session_reactor(session_reactor)
    {
        if (bool(this->verbose & RDPVerbose::sesprobe)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::SessionProbeVirtualChannel: "
                    "timeout=%lld fallback_timeout=%lld"
                    " effective_timeout=%lld on_launch_failure=%d",
                ms2ll(params.session_probe_launch_timeout),
                ms2ll(params.session_probe_launch_fallback_timeout),
                ms2ll(this->session_probe_effective_launch_timeout),
                static_cast<int>(this->param_session_probe_on_launch_failure));
        }

        this->front.session_probe_started(false);
        this->front.set_focus_on_password_textbox(false);
        this->front.set_focus_on_unidentified_input_field(false);
        this->front.set_consent_ui_visible(false);
    }

    bool has_been_launched() const {
        return this->session_probe_ready;
    }

    void start_launch_timeout_timer()
    {
        if ((this->session_probe_effective_launch_timeout.count() > 0) &&
            !this->session_probe_ready) {
            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO, "SessionProbeVirtualChannel::start_launch_timeout_timer");
            }

            if (!this->session_probe_launch_timeout_timer_started) {
                this->session_probe_timer = this->session_reactor.create_timer()
                .set_delay(this->session_probe_effective_launch_timeout)
                .on_action([this](JLN_TIMER_CTX ctx){
                    this->process_event_launch();
                    return ctx.ready_to(this->session_probe_effective_launch_timeout);
                });
                this->session_probe_launch_timeout_timer_started = true;
            }
        }
    }

protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "";
    }

public:
    void give_additional_launch_time() {
        if (!this->session_probe_ready && this->session_probe_timer) {
            this->session_probe_timer->set_delay(this->session_probe_effective_launch_timeout);

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::give_additional_launch_time");
            }
        }
    }

    bool is_disconnection_reconnection_required() {
        return this->disconnection_reconnection_required;
    }

    void request_keep_alive() {
        this->session_probe_keep_alive_received = false;

        {
            StaticOutStream<1024> out_s;

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_skip_bytes(sizeof(uint16_t));

            {
                const char string[] = "Request=Keep-Alive";
                out_s.out_copy_bytes(string, sizeof(string) - 1u);
            }

            out_s.out_clear_bytes(1);   // Null-terminator.

            out_s.set_out_uint16_le(
                out_s.get_offset() - message_length_offset -
                    sizeof(uint16_t),
                message_length_offset);

            this->send_message_to_server(out_s.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_s.get_data(), out_s.get_offset());
        }

        if (bool(this->verbose & RDPVerbose::sesprobe_repetitive)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::request_keep_alive: "
                    "Session Probe keep alive requested");
        }

        this->session_probe_timer->set_delay(this->param_session_probe_keepalive_timeout);
    }

    bool client_input_disabled_because_session_probe_keepalive_is_missing = false;

    void process_event_launch()
    {
        LOG(((this->param_session_probe_on_launch_failure ==
                SessionProbeOnLaunchFailure::disconnect_user) ?
                LOG_ERR : LOG_WARNING),
            "SessionProbeVirtualChannel::process_event: "
                "Session Probe is not ready yet!");

        error_type err_id = ERR_SESSION_PROBE_LAUNCH;

        if (this->session_probe_stop_launch_sequence_notifier) {
            this->session_probe_stop_launch_sequence_notifier->stop(false, err_id);
            this->session_probe_stop_launch_sequence_notifier = nullptr;
        }

        this->session_probe_timer.reset();

        const bool disable_input_event     = false;
        const bool disable_graphics_update = false;
        const bool need_full_screen_update =
            this->mod.disable_input_event_and_graphics_update(
                disable_input_event, disable_graphics_update);

        if (this->param_session_probe_on_launch_failure
         != SessionProbeOnLaunchFailure::ignore_and_continue) {
            throw Error(err_id);
        }

        if (need_full_screen_update) {
            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_event: "
                        "Force full screen update. Rect=(0, 0, %u, %u)",
                    this->param_front_width, this->param_front_height);
            }
            this->mod.rdp_input_invalidate(Rect(0, 0,
                this->param_front_width, this->param_front_height));
        }

        this->rdp.sespro_launch_process_ended();
    }

    void process_event_ready()
    {
        if (!this->session_probe_keep_alive_received) {
            if (!this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                this->mod.disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);

                LOG(LOG_ERR,
                    "SessionProbeVirtualChannel::process_event: "
                        "No keep alive received from Session Probe!");
            }

            if (!this->disconnection_reconnection_required) {
                if (this->session_probe_ending_in_progress) {
                    this->rdp.sespro_ending_in_progress();
                    return ;
                }

                if (SessionProbeOnKeepaliveTimeout::disconnect_user ==
                    this->param_session_probe_on_keepalive_timeout) {
                    this->report_message.report("SESSION_PROBE_KEEPALIVE_MISSED", "");
                }
                else if (SessionProbeOnKeepaliveTimeout::freeze_connection_and_wait ==
                            this->param_session_probe_on_keepalive_timeout) {

                    if (!this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                        const bool disable_input_event     = true;
                        const bool disable_graphics_update = true;
                            this->mod.disable_input_event_and_graphics_update(
                                disable_input_event, disable_graphics_update);

                        this->client_input_disabled_because_session_probe_keepalive_is_missing = true;

                        this->freeze_mod_screen = this->session_reactor
                        .create_graphic_event(mod.get_dim())
                        .on_action(jln::one_shot([](gdi::GraphicApi& drawable, Dimension const& dim){
                            gdi_freeze_screen(drawable, dim);
                        }));
                    }
                    this->request_keep_alive();
                    this->mod.display_osd_message("No keep alive received from Session Probe!");
                }
                else {
                    this->front.session_probe_started(false);
                }
            }
        }
        else {
            this->request_keep_alive();
        }
    }

    template <class T>
    void send_client_message(T t) {
        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));

        t(out_s);

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.set_out_uint16_le(
            out_s.get_offset() - message_length_offset -
                sizeof(uint16_t),
            message_length_offset);

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_data(), out_s.get_offset());
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask>& out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        if (bool(this->verbose & RDPVerbose::sesprobe)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (bool(this->verbose & RDPVerbose::sesprobe_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        uint16_t message_length = chunk.in_uint16_le();
        this->server_message.reserve(message_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_message.clear();
        }

        this->server_message.append(char_ptr_cast(chunk.get_current()),
            chunk.in_remain());

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            return;
        }

        while (this->server_message.back() == '\0') {
            this->server_message.pop_back();
        }
        if (bool(this->verbose & RDPVerbose::sesprobe)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: \"%s\"",
                this->server_message.c_str());
        }

        // TODO string_view
        std::string              order_;
        // TODO vector<string_view>
        std::vector<std::string> parameters_;
        const bool parse_server_message_result =
            ::parse_server_message(this->server_message.c_str(), order_, parameters_);
        if (!parse_server_message_result) {
            LOG(LOG_WARNING,
                "SessionProbeVirtualChannel::process_server_message: "
                    "Failed to parse server message. Message=\"%s\"", this->server_message.c_str());
            return;
        }

        if (!::strcasecmp(order_.c_str(), "Options") && !parameters_.empty()) {
            this->options = ::strtoul(parameters_[0].c_str(), nullptr, 10);

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO, "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                    this->options);
            }
        }
        else if (!::strcasecmp(order_.c_str(), "Request") && !parameters_.empty()) {
            if (!::strcasecmp(parameters_[0].c_str(), "Hello")) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Session Probe is ready.");

                uint32_t remote_reconnection_cookie = INVALID_RECONNECTION_COOKIE;
                if (parameters_.size() > 1) {
                    remote_reconnection_cookie =
                        ::strtoul(parameters_[1].c_str(), nullptr, 10);
                }
                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "LocalCookie=0x%X RemoteCookie=0x%X",
                        this->reconnection_cookie, remote_reconnection_cookie);
                }

                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO, "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                        this->options);
                }

                bool const delay_disabled_launch_mask = (options & OPTION_DELAY_DISABLED_LAUNCH_MASK);


                error_type err_id = NO_ERROR;

                if (this->session_probe_ready) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Session Probe reconnection detect.");

                    if (!this->param_allow_multiple_handshake &&
                        (this->reconnection_cookie != remote_reconnection_cookie)) {
                        this->report_message.report("SESSION_PROBE_RECONNECTION", "");
                    }
                }
                else {
                    if (this->session_probe_stop_launch_sequence_notifier) {
                        this->session_probe_stop_launch_sequence_notifier->stop(true, err_id);
                        this->session_probe_stop_launch_sequence_notifier = nullptr;
                    }

                    this->session_probe_ready = true;
                }

                this->front.session_probe_started(true);

                if (!delay_disabled_launch_mask) {
                    const bool disable_input_event     = false;
                    const bool disable_graphics_update = false;
                    if (this->mod.disable_input_event_and_graphics_update(
                            disable_input_event, disable_graphics_update)) {
                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO,
                                "SessionProbeVirtualChannel::process_server_message: "
                                    "Force full screen update. Rect=(0, 0, %u, %u)",
                                this->param_front_width, this->param_front_height);
                        }
                        if (this->param_bogus_refresh_rect_ex) {
                            this->mod.rdp_suppress_display_updates();
                            this->mod.rdp_allow_display_updates(0, 0,
                                this->param_front_width, this->param_front_height);
                        }
                        this->mod.rdp_input_invalidate(Rect(0, 0,
                            this->param_front_width, this->param_front_height));
                    }
                }

                this->file_system_virtual_channel.disable_session_probe_drive();

                this->session_probe_timer.reset();

                this->rdp.sespro_launch_process_ended();

                if (this->param_session_probe_keepalive_timeout.count() > 0) {
                    send_client_message([](OutStream & out_s) {
                            const char cstr[] = "Request=Keep-Alive";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        });

                    if (bool(this->verbose & RDPVerbose::sesprobe_repetitive)) {
                        LOG(LOG_INFO,
                            "SessionProbeVirtualChannel::process_event: "
                                "Session Probe keep alive requested");
                    }

                    this->session_probe_timer = this->session_reactor.create_timer()
                    .set_delay(this->param_session_probe_keepalive_timeout)
                    .on_action([this](auto ctx){
                        this->process_event_ready();
                        return ctx.ready_to(this->param_session_probe_keepalive_timeout);
                    });
                }

                send_client_message([](OutStream & out_s) {
                        const char cstr[] = "Version=" "1" "\x01" "4";
                        out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                    });

                {
                    uint32_t options = 0;

                    if (this->param_session_probe_ignore_ui_less_processes_during_end_of_session_check) {
                        options |= OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK;
                    }

                    if (options)
                    {
                        send_client_message([options](OutStream & out_s) {
                                {
                                    const char cstr[] = "Options=";
                                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                                }

                                {
                                    char cstr[128];
                                    int len = std::snprintf(cstr, sizeof(cstr), "%u", options);
                                    out_s.out_copy_bytes(cstr, size_t(len));
                                }
                            });
                    }
                }

                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "ChildlessWindowAsUnidentifiedInputField=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        if (this->param_session_probe_childless_window_as_unidentified_input_field) {
                            const char cstr[] = "Yes";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                        else {
                            const char cstr[] = "No";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                    });

                send_client_message([](OutStream & out_s) {
                        {
                            const char cstr[] = "ExtraInfo=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        {
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%d", ::getpid());
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                    });

                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "AutomaticallyEndDisconnectedSession=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        if (this->param_session_probe_end_disconnected_session) {
                            const char cstr[] = "Yes";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                        else {
                            const char cstr[] = "No";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                    });

                {
                    unsigned int const disconnect_session_limit =
                        (this->param_real_alternate_shell.empty() ?
                         // Normal RDP session
                         this->param_session_probe_disconnected_session_limit.count() :
                         // Application session
                         this->param_session_probe_disconnected_application_limit.count());

                    if (disconnect_session_limit) {
                        send_client_message([disconnect_session_limit](OutStream & out_s) {
                                {
                                    const char cstr[] = "DisconnectedSessionLimit=";
                                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                                }

                                {
                                    char cstr[128];
                                    std::snprintf(cstr, sizeof(cstr), "%u",
                                        disconnect_session_limit);
                                    out_s.out_copy_bytes(cstr, strlen(cstr));
                                }
                            });
                    }
                }

                if (this->param_session_probe_idle_session_limit.count()) {
                    send_client_message([this](OutStream & out_s) {
                            {
                                const char cstr[] = "IdleSessionLimit=";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }

                            {
                                char cstr[128];
                                int len = std::snprintf(cstr, sizeof(cstr), "%lld",
                                    ms2ll(this->param_session_probe_idle_session_limit));
                                out_s.out_copy_bytes(cstr, size_t(len));
                            }
                        });
                }

                {
                    this->reconnection_cookie = this->gen.rand32();
                    if (INVALID_RECONNECTION_COOKIE == this->reconnection_cookie) {
                        this->reconnection_cookie &= ~(0x80000000);
                    }

                    send_client_message([this](OutStream & out_s) {
                            {
                                const char cstr[] = "ReconnectionCookie=";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }

                            {
                                char cstr[128];
                                std::snprintf(cstr, sizeof(cstr), "%u",
                                    this->reconnection_cookie);
                                out_s.out_copy_bytes(cstr, strlen(cstr));
                            }
                        });
                }

                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "EnableCrashDump=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        if (this->param_enable_crash_dump) {
                            const char cstr[] = "Yes";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                        else {
                            const char cstr[] = "No";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }
                    });

                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "Bushido=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        {
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%u",
                                this->param_handle_usage_limit,
                                this->param_memory_usage_limit);
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "DisableLaunchMask")) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                if (this->mod.disable_input_event_and_graphics_update(
                        disable_input_event, disable_graphics_update)) {
                    if (bool(this->verbose & RDPVerbose::sesprobe)) {
                        LOG(LOG_INFO,
                            "SessionProbeVirtualChannel::process_server_message: "
                                "Force full screen update. Rect=(0, 0, %u, %u)",
                            this->param_front_width, this->param_front_height);
                    }
                    if (this->param_bogus_refresh_rect_ex) {
                        this->mod.rdp_suppress_display_updates();
                        this->mod.rdp_allow_display_updates(0, 0,
                            this->param_front_width, this->param_front_height);
                    }
                    this->mod.rdp_input_invalidate(Rect(0, 0,
                        this->param_front_width, this->param_front_height));
                }
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get target informations")) {
                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "TargetInformations=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        out_s.out_copy_bytes(this->param_target_informations.data(),
                            this->param_target_informations.size());
                    });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get startup application")) {
                if (this->param_real_alternate_shell != "[None]" ||
                    this->start_application_started) {
                    send_client_message([this](OutStream & out_s) {
                            {
                                const char cstr[] = "StartupApplication=";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }

                            if (this->param_real_alternate_shell.empty()) {
                                const char cstr[] = "[Windows Explorer]";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }
                            else if (this->param_real_alternate_shell == "[None]") {
                                const char cstr[] = "[None]";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }
                            else {
                                if (!this->param_real_working_dir.empty()) {
                                    out_s.out_copy_bytes(
                                        this->param_real_working_dir.data(),
                                        this->param_real_working_dir.size());
                                }
                                out_s.out_uint8('\x01');

                                out_s.out_copy_bytes(
                                    this->param_real_alternate_shell.data(),
                                    this->param_real_alternate_shell.size());

                                if (0x0102 <= this->other_version) {
                                    if (!this->param_show_maximized) {
                                        out_s.out_uint8('\x01');

                                        const char cstr[] = "Normal";
                                        out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                                    }
                                }
                            }
                        });
                }

                this->start_application_query_processed = true;
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Disconnection-Reconnection")) {
                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Disconnection-Reconnection required.");
                }

                this->disconnection_reconnection_required = true;

                send_client_message([](OutStream & out_s) {
                        const char cstr[] = "Confirm=Disconnection-Reconnection";
                        out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                    });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get extra system process") &&
                     (2 <= parameters_.size())) {
                const unsigned int proc_index =
                    ::strtoul(parameters_[1].c_str(), nullptr, 10);

                // ExtraSystemProcess=ProcIndex\x01ErrorCode[\x01ProcName]
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, proc_index](OutStream & out_s) {
                        {
                            const char cstr[] = "ExtraSystemProcess=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        std::string name;

                        const bool result =
                            this->extra_system_processes.get(proc_index, name);

                        {
                            const int error_code = (result ? 0 : -1);
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                                proc_index, error_code);
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }

                        if (result) {
                            char cstr[1024];
                            std::snprintf(cstr, sizeof(cstr), "\x01" "%s",
                                name.c_str());
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                    });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get outbound connection monitoring rule") &&
                     (2 <= parameters_.size())) {
                const unsigned int rule_index =
                    ::strtoul(parameters_[1].c_str(), nullptr, 10);

                // OutboundConnectionMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01HostAddrOrSubnet\x01Port]
                // RuleType  : 0 - notify, 1 - deny, 2 - allow.
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, rule_index](OutStream & out_s) {
                        {
                            const char cstr[] = "OutboundConnectionMonitoringRule=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        unsigned int type = 0;
                        std::string  host_address_or_subnet;
                        std::string  port_range;
                        std::string  description;

                        const bool result =
                            this->outbound_connection_monitor_rules.get(
                                rule_index, type, host_address_or_subnet, port_range,
                                description);

                        {
                            const int error_code = (result ? 0 : -1);
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                                rule_index, error_code);
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }

                        if (result) {
                            char cstr[1024];
                            std::snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01" "%s" "\x01" "%s",
                                type, host_address_or_subnet.c_str(), port_range.c_str());
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                    });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get process monitoring rule") &&
                     (2 <= parameters_.size())) {
                const unsigned int rule_index =
                    ::strtoul(parameters_[1].c_str(), nullptr, 10);

                // ProcessMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01Pattern]
                // RuleType  : 0 - notify, 1 - deny.
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, rule_index](OutStream & out_s) {
                        {
                            const char cstr[] = "ProcessMonitoringRule=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        unsigned int type = 0;
                        std::string  pattern;
                        std::string  description;

                        const bool result =
                            this->process_monitor_rules.get(
                                rule_index, type, pattern, description);

                        {
                            const int error_code = (result ? 0 : -1);
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                                rule_index, error_code);
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }

                        if (result) {
                            char cstr[1024];
                            std::snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01" "%s",
                                type, pattern.c_str());
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                    });
            }

            else if (!::strcasecmp(parameters_[0].c_str(), "Get windows of application as unidentified input field") &&
                     (2 <= parameters_.size())) {
                const unsigned int app_index =
                    ::strtoul(parameters_[1].c_str(), nullptr, 10);

                // WindowsOfApplicationAsUnidentifiedInputField=AppIndex\x01ErrorCode[\x01AppName]
                // ErrorCode : 0 on success. -1 if an error occurred.


                send_client_message([this, app_index](OutStream & out_s) {
                        {
                            const char cstr[] = "WindowsOfApplicationAsUnidentifiedInputField=";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        std::string name;

                        const bool result =
                            this->windows_of_these_applications_as_unidentified_input_field.get(app_index, name);

                        {
                            const int error_code = (result ? 0 : -1);
                            char cstr[128];
                            std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                                app_index, error_code);
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }

                        if (result) {
                            char cstr[1024];
                            std::snprintf(cstr, sizeof(cstr), "\x01" "%s",
                                name.c_str());
                            out_s.out_copy_bytes(cstr, strlen(cstr));
                        }
                    });
            }

            else {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Unexpected request. Message=\"%s\"",
                    this->server_message.c_str());
            }
        }
        else if (!::strcasecmp(order_.c_str(), "ExecuteResult") && (4 <= parameters_.size())) {
            this->rdp.sespro_rail_exec_result(
                    ::atoi(parameters_[3].c_str()), /*NOLINT*/
                    parameters_[0].c_str(),
                    ::atoi(parameters_[1].c_str()), /*NOLINT*/
                    ::atoi(parameters_[2].c_str())  /*NOLINT*/
                );
        }
        else if (!::strcasecmp(order_.c_str(), "ExtraInfo") && !parameters_.empty()) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "SessionProbePID=%s",
                parameters_[0].c_str());
        }
        else if (!::strcasecmp(order_.c_str(), "Version") && (2 <= parameters_.size())) {
            const uint8_t major = uint8_t(::strtoul(parameters_[0].c_str(), nullptr, 10));
            const uint8_t minor = uint8_t(::strtoul(parameters_[1].c_str(), nullptr, 10));

            this->other_version = ((major << 8) | minor);

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "OtherVersion=%u.%u",
                    unsigned(major), unsigned(minor));
            }

            if (this->param_session_probe_enable_log) {
                send_client_message([this](OutStream & out_s) {
                        {
                            const char cstr[] = "EnableLog=Yes";
                            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                        }

                        if (this->param_session_probe_enable_log_rotation) {
                            if (0x0103 <= this->other_version) {
                                out_s.out_uint8('\x01');

                                const char cstr[] = "Yes";
                                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                            }
                            else {
                                LOG(LOG_INFO,
                                    "SessionProbeVirtualChannel::process_event: "
                                        "Log file rotation is not supported by Session Probe! OtherVersion=0x%X",
                                    this->other_version);
                            }
                        }
                    });
            }
        }
        else if (!::strcasecmp(order_.c_str(), "Log") && !parameters_.empty()) {
            LOG(LOG_INFO, "SessionProbe: %s", parameters_[0].c_str());
        }
        else if (!::strcasecmp(order_.c_str(), "KeepAlive") && !parameters_.empty() &&
                 !::strcasecmp(parameters_[0].c_str(), "OK")) {
            if (bool(this->verbose & RDPVerbose::sesprobe_repetitive)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Recevied Keep-Alive from Session Probe.");
            }
            this->session_probe_keep_alive_received = true;

            if (this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                 this->mod.disable_input_event_and_graphics_update(
                     disable_input_event, disable_graphics_update);

                std::string string_message;
                this->mod.display_osd_message(string_message);

                this->mod.rdp_input_invalidate(Rect(0, 0,
                    this->param_front_width, this->param_front_height));

                this->request_keep_alive();

                this->client_input_disabled_because_session_probe_keepalive_is_missing = false;
            }
        }
        else if (!::strcasecmp(order_.c_str(), "SESSION_ENDING_IN_PROGRESS")) {
            auto info = key_qvalue_pairs({
                {"type", "SESSION_ENDING_IN_PROGRESS"},
            });

            ArcsightLogInfo arc_info;
            arc_info.name = "SESSION_ENDING_IN_PROGRESS";
            arc_info.signatureID = ArcsightLogInfo::SESSION_ENDING_IN_PROGRESS;
            arc_info.ApplicationProtocol = "rdp";
            arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

            this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO, "%s", info);
            }

            this->session_probe_ending_in_progress = true;
        }
        else {
            this->front.session_update({this->server_message.c_str(), this->server_message.size()});

            bool message_format_invalid = false;

            if (!parameters_.empty()) {

                if (!::strcasecmp(order_.c_str(), "KERBEROS_TICKET_CREATION") ||
                    !::strcasecmp(order_.c_str(), "KERBEROS_TICKET_DELETION")) {
                    if (parameters_.size() == 7) {
                        auto info = key_qvalue_pairs({
                                { "type",            order_         },
                                { "encryption_type", parameters_[0] },
                                { "client_name",     parameters_[1] },
                                { "server_name",     parameters_[2] },
                                { "start_time",      parameters_[3] },
                                { "end_time",        parameters_[4] },
                                { "renew_time",      parameters_[5] },
                                { "flags",           parameters_[6] }
                            });

                        ArcsightLogInfo arc_info;
                        arc_info.name = order_;
                        arc_info.signatureID = ArcsightLogInfo::KERBEROS_TICKET;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "PASSWORD_TEXT_BOX_GET_FOCUS")) {
                    auto info = key_qvalue_pairs({
                        {"type",   "PASSWORD_TEXT_BOX_GET_FOCUS"},
                        {"status", parameters_[0]},
                    });

                    ArcsightLogInfo arc_info;
                    arc_info.name = "PASSWORD_TEXT_BOX_GET_FOCUS";
                    arc_info.signatureID = ArcsightLogInfo::PASSWORD_TEXT_BOX_GET_FOCUS;
                    arc_info.WallixBastionStatus = parameters_[0];
                    arc_info.ApplicationProtocol = "rdp";
                    arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                    this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                    if (bool(this->verbose & RDPVerbose::sesprobe)) {
                        LOG(LOG_INFO, "%s", info);
                    }

                    if (parameters_.size() == 1) {
                        this->front.set_focus_on_password_textbox(
                            !::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "UNIDENTIFIED_INPUT_FIELD_GET_FOCUS")) {
                    auto info = key_qvalue_pairs({
                        {"type",   "UNIDENTIFIED_INPUT_FIELD_GET_FOCUS"},
                        {"status", parameters_[0]},
                    });

                    ArcsightLogInfo arc_info;
                    arc_info.name = "UNIDENTIFIED_INPUT_FIELD_GET_FOCUS";
                    arc_info.signatureID = ArcsightLogInfo::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS;
                    arc_info.WallixBastionStatus = parameters_[0];
                    arc_info.ApplicationProtocol = "rdp";
                    arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                    this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                    if (bool(this->verbose & RDPVerbose::sesprobe)) {
                        LOG(LOG_INFO, "%s", info);
                    }

                    if (parameters_.size() == 1) {
                        this->front.set_focus_on_unidentified_input_field(
                            !::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "UAC_PROMPT_BECOME_VISIBLE")) {
                    if (parameters_.size() == 1) {
                        auto info = key_qvalue_pairs({
                            {"type",   "UAC_PROMPT_BECOME_VISIBLE"},
                            {"status", parameters_[0]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "UAC_PROMPT_BECOME_VISIBLE";
                        arc_info.signatureID = ArcsightLogInfo::UAC_PROMPT_BECOME_VISIBLE;
                        arc_info.WallixBastionStatus = parameters_[0];
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        this->front.set_consent_ui_visible(!::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "INPUT_LANGUAGE")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",         "INPUT_LANGUAGE"},
                            {"identifier",   parameters_[0]},
                            {"display_name", parameters_[1]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "INPUT_LANGUAGE";
                        arc_info.signatureID = ArcsightLogInfo::INPUT_LANGUAGE;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        this->front.set_keylayout(
                            ::strtol(parameters_[0].c_str(), nullptr, 16));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "NEW_PROCESS") ||
                         !::strcasecmp(order_.c_str(), "COMPLETED_PROCESS")) {
                    if (parameters_.size() == 1) {
                        auto info = key_qvalue_pairs({
                            {"type",         order_.c_str()},
                            {"command_line", parameters_[0]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = order_;
                        arc_info.signatureID = ArcsightLogInfo::PROCESS;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "STARTUP_APPLICATION_FAIL_TO_RUN")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",             "STARTUP_APPLICATION_FAIL_TO_RUN"},
                            {"application_name", parameters_[0]},
                            {"raw_result",       parameters_[1]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "STARTUP_APPLICATION";
                        arc_info.signatureID = ArcsightLogInfo::STARTUP_APPLICATION;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.WallixBastionStatus = "FAIL_TO_RUN";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: %s", info);

                        this->report_message.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "STARTUP_APPLICATION_FAIL_TO_RUN_2")) {
                    if (parameters_.size() == 3) {
                        auto info = key_qvalue_pairs({
                            {"type",               "STARTUP_APPLICATION_FAIL_TO_RUN"},
                            {"application_name",   parameters_[0]},
                            {"raw_result",         parameters_[1]},
                            {"raw_result_message", parameters_[2]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "STARTUP_APPLICATION";
                        arc_info.signatureID = ArcsightLogInfo::STARTUP_APPLICATION;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.WallixBastionStatus = "FAIL_TO_RUN";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: %s", info);

                        this->report_message.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_BLOCKED")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",             "OUTBOUND_CONNECTION_BLOCKED"},
                            {"rule",             parameters_[0]},
                            {"application_name", parameters_[1]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "OUTBOUND_CONNECTION_BLOCKED";
                        arc_info.signatureID = ArcsightLogInfo::OUTBOUND_CONNECTION;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_DETECTED")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",             "OUTBOUND_CONNECTION_DETECTED"},
                            {"rule",             parameters_[0]},
                            {"application_name", parameters_[1]}
                            });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "OUTBOUND_CONNECTION_DETECTED";
                        arc_info.signatureID = ArcsightLogInfo::OUTBOUND_CONNECTION;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        char message[4096];

                        REDEMPTION_DIAGNOSTIC_PUSH
                        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
                        std::snprintf(message, sizeof(message),
                            TR(trkeys::process_interrupted_security_policies,
                                this->param_lang),
                            parameters_[1].c_str());
                        REDEMPTION_DIAGNOSTIC_POP

                        std::string string_message = message;
                        this->mod.display_osd_message(string_message);
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_BLOCKED_2") ||
                         !::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_DETECTED_2")) {
                    bool deny = (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_BLOCKED_2"));

                    if ((!deny && (parameters_.size() == 5)) ||
                        (deny && (parameters_.size() == 6))) {
                        unsigned int type = 0;
                        std::string  host_address_or_subnet;
                        std::string  port_range;
                        std::string  description;

                        const bool result =
                            this->outbound_connection_monitor_rules.get(
                                ::strtoul(parameters_[0].c_str(), nullptr, 10),
                                type, host_address_or_subnet, port_range,
                                description);

                        if (result) {
                            auto info = key_qvalue_pairs({
                                {"type",         order_.c_str()},
                                {"rule",         description},
                                {"app_name",     parameters_[1]},
                                {"app_cmd_line", parameters_[2]},
                                {"dst_addr",     parameters_[3]},
                                {"dst_port",     parameters_[4]},
                                });

                            ArcsightLogInfo arc_info;
                            arc_info.name = order_;
                            arc_info.signatureID = ArcsightLogInfo::OUTBOUND_CONNECTION;
                            arc_info.message = info;
                            arc_info.ApplicationProtocol = "rdp";
                            arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                            this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                            {
                                char message[4096];

                                // rule, app_name, app_cmd_line, dst_addr, dst_port
                                snprintf(message, sizeof(message), "%s|%s|%s|%s|%s",
                                    description.c_str(), parameters_[1].c_str(), parameters_[2].c_str(),
                                    parameters_[3].c_str(), parameters_[4].c_str());

                                this->report_message.report(
                                    (deny ? "FINDCONNECTION_DENY" : "FINDCONNECTION_NOTIFY"),
                                    message);
                            }

                            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                                LOG(LOG_INFO, "%s", info);
                            }

                            if (deny) {
                                if (::strtoul(parameters_[5].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block outbound connection!");
                                    this->report_message.report(
                                        "SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    REDEMPTION_DIAGNOSTIC_PUSH
                                    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
                                    std::snprintf(message, sizeof(message),
                                        TR(trkeys::process_interrupted_security_policies,
                                           this->param_lang),
                                        parameters_[1].c_str());
                                    REDEMPTION_DIAGNOSTIC_POP

                                    std::string string_message = message;
                                    this->mod.display_osd_message(string_message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "PROCESS_BLOCKED") ||
                         !::strcasecmp(order_.c_str(), "PROCESS_DETECTED")) {
                    bool deny = (!::strcasecmp(order_.c_str(), "PROCESS_BLOCKED"));

                    if ((!deny && (parameters_.size() == 3)) ||
                        (deny && (parameters_.size() == 4))) {
                        unsigned int type = 0;
                        std::string  pattern;
                        std::string  description;
                        const bool result =
                            this->process_monitor_rules.get(
                                ::strtoul(parameters_[0].c_str(), nullptr, 10),
                                type, pattern, description);

                        if (result) {
                            auto info = key_qvalue_pairs({
                                {"type",         order_.c_str()},
                                {"rule",         description},
                                {"app_name",     parameters_[1]},
                                {"app_cmd_line", parameters_[2]},
                                });

                            ArcsightLogInfo arc_info;
                            arc_info.name = order_;
                            arc_info.signatureID = ArcsightLogInfo::PROCESS;
                            arc_info.message = info;
                            arc_info.ApplicationProtocol = "rdp";
                            arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                            this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                            {
                                char message[4096];

                                // rule, app_name, app_cmd_line, dst_addr, dst_port
                                snprintf(message, sizeof(message), "%s|%s|%s",
                                    description.c_str(), parameters_[1].c_str(), parameters_[2].c_str());

                                this->report_message.report(
                                    (deny ? "FINDPROCESS_DENY" : "FINDPROCESS_NOTIFY"),
                                    message);
                            }

                            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                                LOG(LOG_INFO, "%s", info);
                            }

                            if (deny) {
                                if (::strtoul(parameters_[3].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block process!");
                                    this->report_message.report(
                                        "SESSION_PROBE_PROCESS_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    REDEMPTION_DIAGNOSTIC_PUSH
                                    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
                                    std::snprintf(message, sizeof(message),
                                                TR(trkeys::process_interrupted_security_policies,
                                                this->param_lang),
                                                parameters_[1].c_str());
                                    REDEMPTION_DIAGNOSTIC_POP

                                    this->mod.display_osd_message(message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "FOREGROUND_WINDOW_CHANGED")) {
                    if ((parameters_.size() == 2) || (parameters_.size() == 3)) {
                        auto info = key_qvalue_pairs({
                            {"type",   "TITLE_BAR"},
                            {"source", "Probe"},
                            {"window", parameters_[0]},
                            });

                        ArcsightLogInfo arc_info;
                        arc_info.name = order_;
                        arc_info.signatureID = ArcsightLogInfo::FOREGROUND_WINDOW_CHANGED;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "BUTTON_CLICKED")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",   "BUTTON_CLICKED"},
                            {"window", parameters_[0]},
                            {"button", parameters_[1]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = order_;
                        arc_info.signatureID = ArcsightLogInfo::BUTTON_CLICKED;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "EDIT_CHANGED")) {
                    if (parameters_.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",   "EDIT_CHANGED"},
                            {"window", parameters_[0]},
                            {"edit",   parameters_[1]},
                        });

                        ArcsightLogInfo arc_info;
                        arc_info.name = "EDIT_CHANGED";
                        arc_info.signatureID = ArcsightLogInfo::EDIT_CHANGED;
                        arc_info.message = info;
                        arc_info.ApplicationProtocol = "rdp";
                        arc_info.direction_flag = ArcsightLogInfo::SERVER_SRC;

                        this->report_message.log6(info, arc_info, this->session_reactor.get_current_time());

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else {
                    LOG(LOG_WARNING,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Unexpected order. Message=\"%s\"",
                        this->server_message.c_str());
                }
            }
            else {
                message_format_invalid = true;
            }

            if (message_format_invalid) {
                LOG(LOG_WARNING,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Invalid message format. Message=\"%s\"",
                    this->server_message.c_str());
            }
        }
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_stop_launch_sequence_notifier = launcher;
    }

    void start_end_session_check() {
        if (this->param_real_alternate_shell != "[None]") {
            return;
        }

        this->start_application_started = true;

        if (!this->start_application_query_processed) {
            return;
        }

        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));

        {
            const char cstr[] = "StartupApplication=";
            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
        }

        {
            const char cstr[] = "[None]";
            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
        }

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.set_out_uint16_le(
            out_s.get_offset() - message_length_offset -
                sizeof(uint16_t),
            message_length_offset);

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_data(), out_s.get_offset());
    }

    void rail_exec(const char* application_name, const char* command_line,
        const char* current_directory, bool show_maximized, uint16_t flags) {
        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));

        {
            const char cstr[] = "Execute=";
            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
        }

        if (application_name && *application_name) {
            out_s.out_copy_bytes(application_name, ::strlen(application_name));
        }

        out_s.out_uint8('\x01');
        if (command_line && *command_line) {
            out_s.out_copy_bytes(command_line, ::strlen(command_line));
        }

        out_s.out_uint8('\x01');
        if (current_directory && *current_directory) {
            out_s.out_copy_bytes(current_directory, ::strlen(current_directory));
        }

        out_s.out_uint8('\x01');
        if (show_maximized) {
            const char cstr[] = "Minimized";
            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
        }
        else {
            const char cstr[] = "Normal";
            out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
        }

        out_s.out_uint8('\x01');
        {
            std::ostringstream oss;
            oss << flags;

            std::string s = oss.str();

            out_s.out_copy_bytes(s.c_str(), s.length());
        }

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.set_out_uint16_le(
            out_s.get_offset() - message_length_offset -
                sizeof(uint16_t),
            message_length_offset);

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_data(), out_s.get_offset());
    }
};  // class SessionProbeVirtualChannel

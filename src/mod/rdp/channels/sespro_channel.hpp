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

#include "core/front_api.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "utils/extra_system_processes.hpp"
#include "utils/outbound_connection_monitor_rules.hpp"
#include "utils/process_monitor_rules.hpp"
#include "utils/stream.hpp"
#include "utils/translation.hpp"
#include "utils/sugar/algostring.hpp"
#include "core/error.hpp"
#include "mod/mod_api.hpp"

#include <chrono>
#include <memory>
#include <sstream>
#include <cinttypes> // PRId64, ...

class SessionProbeVirtualChannel final : public BaseVirtualChannel
{
private:
    bool session_probe_ending_in_progress  = false;
    bool session_probe_keep_alive_received = true;
    bool session_probe_ready               = false;

    bool session_probe_launch_timeout_timer_started = false;

    const std::chrono::milliseconds session_probe_effective_launch_timeout;

    const std::chrono::milliseconds param_session_probe_keepalive_timeout;
    const bool     param_session_probe_on_keepalive_timeout_disconnect_user;

    const SessionProbeOnLaunchFailure param_session_probe_on_launch_failure;

    const bool     param_session_probe_end_disconnected_session;

    std::string    param_target_informations;

    const uint16_t param_front_width;
    const uint16_t param_front_height;

    const std::chrono::milliseconds param_session_probe_disconnected_application_limit;
    const std::chrono::milliseconds param_session_probe_disconnected_session_limit;
    const std::chrono::milliseconds param_session_probe_idle_session_limit;

    const bool param_session_probe_enable_log;

    std::string param_real_alternate_shell;
    std::string param_real_working_dir;

    Translation::language_t param_lang;

    const bool param_bogus_refresh_rect_ex;

    const bool param_show_maximized;

    FrontAPI& front;

    mod_api& mod;
    rdp_api& rdp;

    FileSystemVirtualChannel& file_system_virtual_channel;

    wait_obj session_probe_event;

    ExtraSystemProcesses           extra_system_processes;
    OutboundConnectionMonitorRules outbound_connection_monitor_rules;
    ProcessMonitorRules            process_monitor_rules;

    bool disconnection_reconnection_required = false; // Cause => Authenticated user changed.

    SessionProbeLauncher* session_probe_stop_launch_sequence_notifier = nullptr;

    bool has_additional_launch_time = false;

    std::string server_message;

    uint16_t other_version = 0x0100;

    bool start_application_query_processed = false;
    bool start_application_started         = false;

public:
    struct Params : public BaseVirtualChannel::Params {
        std::chrono::milliseconds session_probe_launch_timeout;
        std::chrono::milliseconds session_probe_launch_fallback_timeout;
        std::chrono::milliseconds session_probe_keepalive_timeout;
        bool     session_probe_on_keepalive_timeout_disconnect_user;

        SessionProbeOnLaunchFailure session_probe_on_launch_failure;

        bool session_probe_end_disconnected_session;

        const char* target_informations;

        uint16_t front_width;
        uint16_t front_height;

        std::chrono::milliseconds session_probe_disconnected_application_limit;
        std::chrono::milliseconds session_probe_disconnected_session_limit;
        std::chrono::milliseconds session_probe_idle_session_limit;

        bool session_probe_enable_log;

        const char* real_alternate_shell;
        const char* real_working_dir;

        const char* session_probe_extra_system_processes;

        const char* session_probe_outbound_connection_monitoring_rules;

        const char* session_probe_process_monitoring_rules;

        Translation::language_t lang;

        bool bogus_refresh_rect_ex;

        bool show_maximized;

        Params(ReportMessageApi & report_message) : BaseVirtualChannel::Params(report_message) {}
    };

    SessionProbeVirtualChannel(
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        mod_api& mod,
        rdp_api& rdp,
        FileSystemVirtualChannel& file_system_virtual_channel,
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
    , param_session_probe_on_keepalive_timeout_disconnect_user(
          params.session_probe_on_keepalive_timeout_disconnect_user)
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
    , param_real_alternate_shell(params.real_alternate_shell)
    , param_real_working_dir(params.real_working_dir)
    , param_lang(params.lang)
    , param_bogus_refresh_rect_ex(params.bogus_refresh_rect_ex)
    , param_show_maximized(params.show_maximized)
    , front(front)
    , mod(mod)
    , rdp(rdp)
    , file_system_virtual_channel(file_system_virtual_channel)
    , extra_system_processes(params.session_probe_extra_system_processes)
    , outbound_connection_monitor_rules(
          params.session_probe_outbound_connection_monitoring_rules)
    , process_monitor_rules(
          params.session_probe_process_monitoring_rules)
    {
        if (bool(this->verbose & RDPVerbose::sesprobe)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::SessionProbeVirtualChannel: "
                    "timeout=%" PRId64 " fallback_timeout=%" PRId64
                    " effective_timeout=%" PRId64 " on_launch_failure=%d",
                params.session_probe_launch_timeout.count(),
                params.session_probe_launch_fallback_timeout.count(),
                this->session_probe_effective_launch_timeout.count(),
                static_cast<int>(this->param_session_probe_on_launch_failure));
        }
    }

    void start_launch_timeout_timer()
    {
        if ((this->session_probe_effective_launch_timeout.count() > 0) &&
            !this->session_probe_ready) {
            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO, "SessionProbeVirtualChannel::start_launch_timeout_timer");
            }

            if (!this->session_probe_launch_timeout_timer_started) {
                this->session_probe_event.set_trigger_time(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->session_probe_effective_launch_timeout).count());

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
    wait_obj* get_event()
    {
        if (this->session_probe_event.is_trigger_time_set()) {
            if (this->has_additional_launch_time) {
                if (!this->session_probe_ready) {
                    this->session_probe_event.set_trigger_time(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            this->session_probe_effective_launch_timeout).count());
                }

                this->has_additional_launch_time = false;
            }
            return &this->session_probe_event;
        }

        return nullptr;
    }

    void give_additional_launch_time() {
        if (!this->session_probe_ready) {
            this->has_additional_launch_time = true;

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::give_additional_launch_time");
            }
        }
    }

    bool is_event_signaled() {
        return (this->session_probe_event.is_trigger_time_set() &&
            this->session_probe_event.is_waked_up_by_time());
    }

    bool is_disconnection_reconnection_required() {
        return this->disconnection_reconnection_required;
    }

    void process_event()
    {
        if (!this->session_probe_event.is_trigger_time_set() ||
            !this->session_probe_event.is_waked_up_by_time()) {
            return;
        }

        this->session_probe_event.reset_trigger_time();

        if (this->session_probe_effective_launch_timeout.count() &&
            !this->session_probe_ready &&
            !this->has_additional_launch_time) {
            LOG(((this->param_session_probe_on_launch_failure ==
                  SessionProbeOnLaunchFailure::disconnect_user) ?
                 LOG_ERR : LOG_WARNING),
                "SessionProbeVirtualChannel::process_event: "
                    "Session Probe is not ready yet!");

            if (this->session_probe_stop_launch_sequence_notifier) {
                this->session_probe_stop_launch_sequence_notifier->stop(false);
                this->session_probe_stop_launch_sequence_notifier = nullptr;
            }

            const bool disable_input_event     = false;
            const bool disable_graphics_update = false;
            const bool need_full_screen_update =
                 this->mod.disable_input_event_and_graphics_update(
                     disable_input_event, disable_graphics_update);

            if (this->param_session_probe_on_launch_failure ==
                SessionProbeOnLaunchFailure::ignore_and_continue) {
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
            }
            else {
                throw Error(ERR_SESSION_PROBE_LAUNCH);
            }
        }

        if (this->session_probe_ready &&
            this->param_session_probe_keepalive_timeout.count()) {
            if (!this->session_probe_keep_alive_received) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                this->mod.disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);

                LOG(LOG_ERR,
                    "SessionProbeVirtualChannel::process_event: "
                        "No keep alive received from Session Probe!");

                if (!this->disconnection_reconnection_required) {
                    if (this->session_probe_ending_in_progress) {
                        throw Error(ERR_SESSION_PROBE_ENDING_IN_PROGRESS);
                    }

                    if (this->param_session_probe_on_keepalive_timeout_disconnect_user) {
                        this->report_message.report("SESSION_PROBE_KEEPALIVE_MISSED", "");
                    }
                    else {
                        this->front.session_probe_started(false);
                    }
                }
            }
            else {
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
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set_trigger_time(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->param_session_probe_keepalive_timeout ).count());
            }
        }
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

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST)
            this->server_message.clear();

        this->server_message.append(char_ptr_cast(chunk.get_current()),
            chunk.in_remain());

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST))
            return;

        while (this->server_message.back() == '\0') {
            this->server_message.pop_back();
        }
        if (bool(this->verbose & RDPVerbose::sesprobe)) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: \"%s\"",
                this->server_message.c_str());
        }

        const char request_extra_system_process[] =
            "Request=Get extra system process\x01";

        const char request_outbound_connection_monitoring_rule[] =
            "Request=Get outbound connection monitoring rule\x01";

        const char request_process_monitoring_rule[] =
            "Request=Get process monitoring rule\x01";

        const char request_hello[] = "Request=Hello";

        const char ExtraInfo[]     = "ExtraInfo=";
        const char Version[]       = "Version=";
        const char ExecuteResult[] = "ExecuteResult=";
        const char Log[]           = "Log=";

        if (!this->server_message.compare(request_hello)) {
            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Session Probe is ready.");
            }

            if (this->session_probe_stop_launch_sequence_notifier) {
                this->session_probe_stop_launch_sequence_notifier->stop(true);
                this->session_probe_stop_launch_sequence_notifier = nullptr;
            }

            this->session_probe_ready = true;

            this->front.session_probe_started(true);

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

            this->file_system_virtual_channel.disable_session_probe_drive();

            this->session_probe_event.reset_trigger_time();

            if (this->param_session_probe_keepalive_timeout.count() > 0) {
                {
                    StaticOutStream<1024> out_s;

                    const size_t message_length_offset = out_s.get_offset();
                    out_s.out_skip_bytes(sizeof(uint16_t));

                    {
                        const char cstr[] = "Request=Keep-Alive";
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

                if (bool(this->verbose & RDPVerbose::sesprobe_repetitive)) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set_trigger_time(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        this->param_session_probe_keepalive_timeout).count());
            }

            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "Version=" "1" "\x01" "1";
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

            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "ExtraInfo=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                {
                    char cstr[128];
                    std::snprintf(cstr, sizeof(cstr), "%u", ::getpid());
                    out_s.out_copy_bytes(cstr, strlen(cstr));
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

            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            if (this->param_session_probe_enable_log)
            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "EnableLog=Yes";
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

            unsigned int const disconnect_session_limit =
                (this->param_real_alternate_shell.empty() ?
                 // Normal RDP session
                 this->param_session_probe_disconnected_session_limit.count() :
                 // Application session
                 this->param_session_probe_disconnected_application_limit.count());

            if (disconnect_session_limit)
            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            if (this->param_session_probe_idle_session_limit.count())
            {
                StaticOutStream<1024> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "IdleSessionLimit=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                {
                    char cstr[128];
                    std::snprintf(cstr, sizeof(cstr), "%" PRId64,
                        this->param_session_probe_idle_session_limit.count());
                    out_s.out_copy_bytes(cstr, strlen(cstr));
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
        }
        else if (!this->server_message.compare(
                     "Request=Get target informations")) {
            StaticOutStream<1024> out_s;

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_skip_bytes(sizeof(uint16_t));

            {
                const char cstr[] = "TargetInformations=";
                out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
            }

            out_s.out_copy_bytes(this->param_target_informations.data(),
                this->param_target_informations.size());

            out_s.out_clear_bytes(1);   // Null-terminator.

            out_s.set_out_uint16_le(
                out_s.get_offset() - message_length_offset -
                    sizeof(uint16_t),
                message_length_offset);

            this->send_message_to_server(out_s.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_s.get_data(), out_s.get_offset());
        }
        else if (!this->server_message.compare(
                     "Request=Get startup application")) {
            if (this->param_real_alternate_shell.compare(
                     "[None]") ||
                this->start_application_started) {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "StartupApplication=";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }

                if (this->param_real_alternate_shell.empty()) {
                    const char cstr[] = "[Windows Explorer]";
                    out_s.out_copy_bytes(cstr, sizeof(cstr) - 1u);
                }
                else if (!this->param_real_alternate_shell.compare("[None]")) {
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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }

            this->start_application_query_processed = true;
        }
        else if (!this->server_message.compare(
                     "Request=Disconnection-Reconnection")) {
            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Disconnection-Reconnection required.");
            }

            this->disconnection_reconnection_required = true;

            {
                StaticOutStream<512> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

                {
                    const char cstr[] = "Confirm=Disconnection-Reconnection";
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
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(ExecuteResult) - 1,
                     ExecuteResult)) {

            std::vector<std::string> parameters;

            {
                std::istringstream ss(this->server_message.c_str() + sizeof(ExecuteResult) - 1);
                std::string        parameter;

                while (std::getline(ss, parameter, '\x01')) {
                    parameters.push_back(std::move(parameter));
                }
            }

            if (4 <= parameters.size()) {
                this->rdp.sespro_rail_exec_result(
                        ::atoi(parameters[3].c_str()),
                        parameters[0].c_str(),
                        ::atoi(parameters[1].c_str()),
                        ::atoi(parameters[2].c_str())
                    );
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(ExtraInfo) - 1,
                     ExtraInfo)) {
            const char * session_probe_pid =
                (this->server_message.c_str() + sizeof(ExtraInfo) - 1);

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "SessionProbePID=%s",
                    session_probe_pid);
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(Version) - 1,
                     Version)) {
            const char * subitems          =
                (this->server_message.c_str() + sizeof(Version) - 1);
            const char * subitem_separator =
                ::strchr(subitems, '\x01');

            if (subitem_separator && (subitem_separator != subitems)) {
                const uint8_t major = uint8_t(::strtoul(subitems, nullptr, 10));
                const uint8_t minor = uint8_t(::strtoul(subitem_separator + 1, nullptr, 10));

                this->other_version = ((major << 8) | minor);

                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "OtherVersion=%u.%u",
                        unsigned(major), unsigned(minor));
                }
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(Log) - 1,
                     Log)) {
            const char * log_string =
                (this->server_message.c_str() + sizeof(Log) - 1);
            LOG(LOG_INFO, "SessionProbe: %s", log_string);
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(request_extra_system_process) - 1,
                     request_extra_system_process)) {
            const char * remaining_data =
                (this->server_message.c_str() +
                 sizeof(request_extra_system_process) - 1);

            const unsigned int proc_index =
                ::strtoul(remaining_data, nullptr, 10);

            // ExtraSystemProcess=ProcIndex\x01ErrorCode[\x01ProcName]
            // ErrorCode : 0 on success. -1 if an error occurred.

            {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(request_outbound_connection_monitoring_rule) - 1,
                     request_outbound_connection_monitoring_rule)) {
            const char * remaining_data =
                (this->server_message.c_str() +
                 sizeof(request_outbound_connection_monitoring_rule) - 1);

            const unsigned int rule_index =
                ::strtoul(remaining_data, nullptr, 10);

            // OutboundConnectionMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01HostAddrOrSubnet\x01Port]
            // RuleType  : 0 - notify, 1 - deny, 2 - allow.
            // ErrorCode : 0 on success. -1 if an error occurred.

            {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare(
                     0,
                     sizeof(request_process_monitoring_rule) - 1,
                     request_process_monitoring_rule)) {
            const char * remaining_data =
                (this->server_message.c_str() +
                 sizeof(request_process_monitoring_rule) - 1);

            const unsigned int rule_index =
                ::strtoul(remaining_data, nullptr, 10);

            // ProcessMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01Pattern]
            // RuleType  : 0 - notify, 1 - deny.
            // ErrorCode : 0 on success. -1 if an error occurred.

            {
                StaticOutStream<8192> out_s;

                const size_t message_length_offset = out_s.get_offset();
                out_s.out_skip_bytes(sizeof(uint16_t));

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

                out_s.out_clear_bytes(1);   // Null-terminator.

                out_s.set_out_uint16_le(
                    out_s.get_offset() - message_length_offset -
                        sizeof(uint16_t),
                    message_length_offset);

                this->send_message_to_server(out_s.get_offset(),
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_data(), out_s.get_offset());
            }
        }
        else if (!this->server_message.compare("KeepAlive=OK")) {
            if (bool(this->verbose & RDPVerbose::sesprobe_repetitive)) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Recevied Keep-Alive from Session Probe.");
            }
            this->session_probe_keep_alive_received = true;
        }
        else if (!this->server_message.compare("SESSION_ENDING_IN_PROGRESS")) {

            auto info = key_qvalue_pairs({
                {"type",   "SESSION_ENDING_IN_PROGRESS"},
            });

            this->report_message.log5(info);

            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                LOG(LOG_INFO, "%s", info);
            }

            this->session_probe_ending_in_progress = true;
        }
        else {
            const char * message   = this->server_message.c_str();
            this->front.session_update({message, this->server_message.size()});

            const char * separator = ::strchr(message, '=');

            bool message_format_invalid = false;

            if (separator) {
                // TODO string_view
                std::string order(message, separator - message);
                // TODO vector<string_view>
                std::vector<std::string> parameters;

                /** TODO
                 * for (r : get_split(separator, this->server_message.c_str() + this->server_message.size(), '\ x01')) {
                 *     parameters.push_back({r.begin(), r.end()});
                 * }
                 */
                {
                    std::istringstream ss(separator + 1);
                    std::string        parameter;

                    while (std::getline(ss, parameter, '\x01')) {
                        parameters.push_back(std::move(parameter));
                    }
                }

                if (!order.compare("PASSWORD_TEXT_BOX_GET_FOCUS")) {

                    auto info = key_qvalue_pairs({
                        {"type",   "PASSWORD_TEXT_BOX_GET_FOCUS"},
                        {"status", parameters[0]},
                    });

                    this->report_message.log5(info);

                    if (bool(this->verbose & RDPVerbose::sesprobe)) {
                        LOG(LOG_INFO, "%s", info);
                    }

                    if (parameters.size() == 1) {
                        this->front.set_focus_on_password_textbox(
                            !parameters[0].compare("yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("UAC_PROMPT_BECOME_VISIBLE")) {
                    if (parameters.size() == 1) {
                        auto info = key_qvalue_pairs({
                            {"type",   "UAC_PROMPT_BECOME_VISIBLE"},
                            {"status", parameters[0]},
                        });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        this->front.set_consent_ui_visible(!parameters[0].compare("yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("INPUT_LANGUAGE")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",         "INPUT_LANGUAGE"},
                            {"identifier",   parameters[0]},
                            {"display_name", parameters[1]},
                        });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        this->front.set_keylayout(
                            ::strtol(parameters[0].c_str(), nullptr, 16));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("NEW_PROCESS") ||
                         !order.compare("COMPLETED_PROCESS")) {
                    if (parameters.size() == 1) {
                        auto info = key_qvalue_pairs({
                            {"type",         order.c_str()},
                            {"command_line", parameters[0]},
                        });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("STARTUP_APPLICATION_FAIL_TO_RUN")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",             "STARTUP_APPLICATION_FAIL_TO_RUN"},
                            {"application_name", parameters[0]},
                            {"RawResult",        parameters[1]},
                        });

                        this->report_message.log5(info);

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
                else if (!order.compare("OUTBOUND_CONNECTION_BLOCKED")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",             "OUTBOUND_CONNECTION_BLOCKED"},
                            {"rule",             parameters[0]},
                            {"application_name", parameters[1]},
                        });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("OUTBOUND_CONNECTION_DETECTED")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type", "OUTBOUND_CONNECTION_DETECTED"},
                            {"rule", parameters[0]},
                            {"application_name", parameters[1]}
                            });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }

                        char message[4096];

                        REDEMPTION_DIAGNOSTIC_PUSH
                        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")
                        std::snprintf(message, sizeof(message),
                            TR(trkeys::process_interrupted_security_policies,
                                this->param_lang),
                            parameters[1].c_str());
                        REDEMPTION_DIAGNOSTIC_POP

                        std::string string_message = message;
                        this->mod.display_osd_message(string_message);
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("OUTBOUND_CONNECTION_BLOCKED_2") ||
                         !order.compare("OUTBOUND_CONNECTION_DETECTED_2")) {
                    bool deny = (!order.compare("OUTBOUND_CONNECTION_BLOCKED_2"));

                    if ((!deny && (parameters.size() == 5)) ||
                        (deny && (parameters.size() == 6))) {
                        unsigned int type = 0;
                        std::string  host_address_or_subnet;
                        std::string  port_range;
                        std::string  description;

                        const bool result =
                            this->outbound_connection_monitor_rules.get(
                                ::strtoul(parameters[0].c_str(), nullptr, 10),
                                type, host_address_or_subnet, port_range,
                                description);

                        if (result) {
                            auto info = key_qvalue_pairs({
                                {"type",         order.c_str()},
                                {"rule",         description},
                                {"app_name",     parameters[1]},
                                {"app_cmd_line", parameters[2]},
                                {"dst_addr",     parameters[3]},
                                {"dst_port",     parameters[4]},
                                });

                            this->report_message.log5(info);

                            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                                LOG(LOG_INFO, "%s", info);
                            }

                            if (deny) {
                                if (::strtoul(parameters[5].c_str(), nullptr, 10)) {
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
                                        parameters[1].c_str());
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
                else if (!order.compare("PROCESS_BLOCKED") ||
                         !order.compare("PROCESS_DETECTED")) {
                    bool deny = (!order.compare("PROCESS_BLOCKED"));

                    if ((!deny && (parameters.size() == 3)) ||
                        (deny && (parameters.size() == 4))) {
                        unsigned int type = 0;
                        std::string  pattern;
                        std::string  description;
                        const bool result =
                            this->process_monitor_rules.get(
                                ::strtoul(parameters[0].c_str(), nullptr, 10),
                                type, pattern, description);

                        if (result) {
                            auto info = key_qvalue_pairs({
                                {"type",         order.c_str()},
                                {"rule",         description},
                                {"app_name",     parameters[1]},
                                {"app_cmd_line", parameters[2]},
                                });

                            this->report_message.log5(info);

                            if (bool(this->verbose & RDPVerbose::sesprobe)) {
                                LOG(LOG_INFO, "%s", info);
                            }

                            if (deny) {
                                if (::strtoul(parameters[3].c_str(), nullptr, 10)) {
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
                                                parameters[1].c_str());
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
                else if (!order.compare("FOREGROUND_WINDOW_CHANGED")) {
                    if ((parameters.size() == 2) || (parameters.size() == 3)) {
                        auto info = key_qvalue_pairs({
                            {"type",       "TITLE_BAR"},
                            {"source",     "Probe"},
                            {"window",     parameters[0]},
                            });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
<<<<<<< HEAD

=======
>>>>>>> 18019d85e4b68234cb7683ccef448c03a5fdb415
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("BUTTON_CLICKED")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",       "BUTTON_CLICKED"},
                            {"window",     parameters[0]},
                            {"button",     parameters[1]},
                        });

                        this->report_message.log5(info);

                        if (bool(this->verbose & RDPVerbose::sesprobe)) {
                            LOG(LOG_INFO, "%s", info);
                        }
<<<<<<< HEAD

=======
>>>>>>> 18019d85e4b68234cb7683ccef448c03a5fdb415
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("EDIT_CHANGED")) {
                    if (parameters.size() == 2) {
                        auto info = key_qvalue_pairs({
                            {"type",   "EDIT_CHANGED"},
                            {"window", parameters[0]},
                            {"edit",   parameters[1]},
                        });

                        this->report_message.log5(info);

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
                        message);
                }
            }
            else {
                message_format_invalid = true;
            }

            if (message_format_invalid) {
                LOG(LOG_WARNING,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Invalid message format. Message=\"%s\"",
                    message);
            }
        }
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_stop_launch_sequence_notifier = launcher;
    }

    void start_end_session_check() {
        if (this->param_real_alternate_shell.compare("[None]")) {
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

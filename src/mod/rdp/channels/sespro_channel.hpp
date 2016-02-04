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

#ifndef REDEMPTION_MOD_RDP_CHANNELS_SESPROCHANNEL_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_SESPROCHANNEL_HPP

#include "core/front_api.hpp"
#include "utils/outbound_connection_monitor_rules.hpp"
#include "utils/stream.hpp"

#include "base_channel.hpp"

#include <memory>

class SessionProbeVirtualChannel : public BaseVirtualChannel
{
private:
    bool session_probe_ending_in_progress  = false;
    bool session_probe_keep_alive_received = true;
    bool session_probe_ready               = false;

    const unsigned session_probe_effective_launch_timeout;

    const bool     param_session_probe_loading_mask_enabled;

    const unsigned param_session_probe_keepalive_timeout;

    const ::configs::SessionProbeOnLaunchFailure
                   param_session_probe_on_launch_failure;

    const bool     param_session_probe_end_disconnected_session;

    std::string    param_target_informations;

    const uint16_t param_front_width;
    const uint16_t param_front_height;

    std::string param_real_alternate_shell;
    std::string param_real_working_dir;

    Translation::language_t param_lang;

    auth_api* param_acl;

    FrontAPI& front;

    mod_api& mod;

    FileSystemDriveManager& file_system_drive_manager;

    VirtualChannelDataSender& file_system_virtual_channel_to_server_sender;

    wait_obj session_probe_event;

    OutboundConnectionMonitorRules outbound_connection_monitor_rules;

    bool disconnection_reconnection_required = false; // Cause => Authenticated user changed.

public:
    struct Params : public BaseVirtualChannel::Params {
        bool session_probe_loading_mask_enabled;

        unsigned session_probe_launch_timeout;
        unsigned session_probe_launch_fallback_timeout;
        unsigned session_probe_keepalive_timeout;

        ::configs::SessionProbeOnLaunchFailure session_probe_on_launch_failure;

        bool session_probe_end_disconnected_session;

        const char* target_informations;

        uint16_t front_width;
        uint16_t front_height;

        const char* real_alternate_shell;
        const char* real_working_dir;

        const char* outbound_connection_notifying_rules;
        const char* outbound_connection_killing_rules;

        Translation::language_t lang;

        auth_api* acl;
    };

    SessionProbeVirtualChannel(
        VirtualChannelDataSender* to_server_sender_,
        FileSystemDriveManager& file_system_drive_manager,
        FrontAPI& front,
        mod_api& mod,
        VirtualChannelDataSender& file_system_virtual_channel_to_server_sender,
        const Params& params)
    : BaseVirtualChannel(nullptr,
                         to_server_sender_,
                         params)
    , session_probe_effective_launch_timeout(
            (params.session_probe_on_launch_failure ==
             ::configs::SessionProbeOnLaunchFailure::disconnect_user) ?
            params.session_probe_launch_timeout :
            params.session_probe_launch_fallback_timeout
        )
    , param_session_probe_loading_mask_enabled(
          params.session_probe_loading_mask_enabled)
    , param_session_probe_keepalive_timeout(
          params.session_probe_keepalive_timeout)
    , param_session_probe_on_launch_failure(
          params.session_probe_on_launch_failure)
    , param_session_probe_end_disconnected_session(
          params.session_probe_end_disconnected_session)
    , param_target_informations(params.target_informations)
    , param_front_width(params.front_width)
    , param_front_height(params.front_height)
    , param_real_alternate_shell(params.real_alternate_shell)
    , param_real_working_dir(params.real_working_dir)
    , param_lang(params.lang)
    , param_acl(params.acl)
    , front(front)
    , mod(mod)
    , file_system_drive_manager(file_system_drive_manager)
    , file_system_virtual_channel_to_server_sender(
          file_system_virtual_channel_to_server_sender)
    , outbound_connection_monitor_rules(
          params.outbound_connection_notifying_rules,
          params.outbound_connection_killing_rules)
    {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::SessionProbeVirtualChannel: "
                    "timeout=%u fallback_timeout=%u effective_timeout=%u on_launch_failure=%d",
                params.session_probe_launch_timeout,
                params.session_probe_launch_fallback_timeout,
                this->session_probe_effective_launch_timeout,
                static_cast<int>(this->param_session_probe_on_launch_failure));
        }

        this->session_probe_event.object_and_time = true;

        if (this->session_probe_effective_launch_timeout > 0) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::SessionProbeVirtualChannel: "
                        "Enable Session Probe launch timer");
            }

            this->session_probe_event.set(
                this->session_probe_effective_launch_timeout * 1000);
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
        if (this->session_probe_event.set_state) {
            return &this->session_probe_event;
        }

        return nullptr;
    }

    bool is_event_signaled() {
        return (this->session_probe_event.set_state &&
            this->session_probe_event.waked_up_by_time);
    }

    bool is_disconnection_reconnection_required() {
        return this->disconnection_reconnection_required;
    }

    void process_event()
    {
        if (!this->session_probe_event.set_state ||
            !this->session_probe_event.waked_up_by_time) {
            return;
        }

        this->session_probe_event.reset();
        this->session_probe_event.waked_up_by_time = false;

        if (this->session_probe_effective_launch_timeout &&
            !this->session_probe_ready) {
            LOG(((this->param_session_probe_on_launch_failure ==
                  ::configs::SessionProbeOnLaunchFailure::disconnect_user) ?
                 LOG_ERR : LOG_WARNING),
                "SessionProbeVirtualChannel::process_event: "
                    "Session Probe is not ready yet!");

            const bool need_full_screen_update =
                (this->param_session_probe_loading_mask_enabled ?
                 this->front.disable_input_event_and_graphics_update(false) :
                 false);

            if (this->param_session_probe_on_launch_failure ==
                ::configs::SessionProbeOnLaunchFailure::ignore_and_continue) {
                if (need_full_screen_update) {
                    if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
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
            this->param_session_probe_keepalive_timeout) {
            if (!this->session_probe_keep_alive_received) {
                if (this->param_session_probe_loading_mask_enabled) {
                    this->front.disable_input_event_and_graphics_update(
                        false);
                }

                LOG(LOG_ERR,
                    "SessionProbeVirtualChannel::process_event: "
                        "No keep alive received from Session Probe!");

                if (!this->disconnection_reconnection_required) {
                    if (this->session_probe_ending_in_progress) {
                        throw Error(ERR_SESSION_PROBE_ENDING_IN_PROGRESS);
                    }

                    throw Error(ERR_SESSION_PROBE_KEEPALIVE);
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

                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set(
                    this->param_session_probe_keepalive_timeout * 1000);
            }
        }
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask>& out_asynchronous_task) override
    {
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
            LOG(LOG_ERR,
                "SessionProbeVirtualChannel::process_server_message: "
                    "Chunked Virtual Channel Data is not supported!");
        }

        uint16_t message_length = chunk.in_uint16_le();
        REDASSERT(message_length == chunk.in_remain());
        // Disable -Wunused-variable if REDASSERT is disabled.
        (void)message_length;

        std::string session_probe_message(
            char_ptr_cast(chunk.get_current()), chunk.in_remain());
        while (session_probe_message.back() == '\0') {
            session_probe_message.pop_back();
        }
        if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: \"%s\"",
                session_probe_message.c_str());
        }

        const char request_outbound_connection_monitoring_rule[] =
            "Request=Get outbound connection monitoring rule\x01";

        const char self_status_eq[] = "Self.Status=";

        const char request_hello[] = "Request=Hello";

        if (!session_probe_message.compare(request_hello)) {
            REDASSERT(!this->session_probe_ready);

            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Session Probe is ready.");
            }

            this->session_probe_ready = true;

            this->front.session_probe_started();

            if (this->param_session_probe_loading_mask_enabled &&
                this->front.disable_input_event_and_graphics_update(false)) {
                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Force full screen update. Rect=(0, 0, %u, %u)",
                        this->param_front_width, this->param_front_height);
                }
                this->mod.rdp_input_invalidate(Rect(0, 0,
                    this->param_front_width, this->param_front_height));
            }

            this->file_system_drive_manager.DisableSessionProbeDrive(
                file_system_virtual_channel_to_server_sender,
                this->verbose);

            this->session_probe_event.reset();

            if (this->param_session_probe_keepalive_timeout > 0) {
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

                if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event: "
                            "Session Probe keep alive requested");
                }

                this->session_probe_event.set(
                    this->param_session_probe_keepalive_timeout * 1000);
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
        }
        else if (!session_probe_message.compare(
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
        else if (!session_probe_message.compare(
                    "Request=Get startup application")) {
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
        else if (!session_probe_message.compare(
                     "Request=Disconnection-Reconnection")) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE) {
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
        else if (!session_probe_message.compare(
                     0,
                     sizeof(request_outbound_connection_monitoring_rule) - 1,
                     request_outbound_connection_monitoring_rule)) {
            const char * remaining_data =
                (session_probe_message.c_str() +
                 sizeof(request_outbound_connection_monitoring_rule) - 1);

            const unsigned int rule_index =
                ::strtoul(remaining_data, nullptr, 10);

            // OutboundConnectionMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01HostAddrOrSubnet\x01Port]
            // RuleType  : 0 - notify, 1 - kill.
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
                unsigned int port = 0;

                const bool result =
                    this->outbound_connection_monitor_rules.get(
                        rule_index, type, host_address_or_subnet, port);

                {
                    const int error_code = (result ? 0 : -1);
                    char cstr[128];
                    snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d" "\x01",
                        rule_index, error_code);
                    out_s.out_copy_bytes(cstr, strlen(cstr));
                }

                if (result) {
                    char cstr[1024];
                    snprintf(cstr, sizeof(cstr), "%u" "\x01" "%s" "\x01" "%u",
                        type, host_address_or_subnet.c_str(), port);
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
        else if (!session_probe_message.compare("KeepAlive=OK")) {
            if (this->verbose & MODRDP_LOGLEVEL_SESPROBE_REPETITIVE) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Recevied Keep-Alive from Session Probe.");
            }
            this->session_probe_keep_alive_received = true;
        }
        else if (!session_probe_message.compare(0, std::string::npos,
                                                self_status_eq,
                                                sizeof(self_status_eq) - 1)) {
            std::string parameters(session_probe_message,
                sizeof(self_status_eq) - 1, std::string::npos);

            if (parameters.compare("EndingInProgress")) {
                this->param_acl->log4(
                    (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                    "Self.Status", "status=EndingInProgress");

                this->session_probe_ending_in_progress = true;
            }
        }
        else if (!session_probe_message.compare("Self.Status=EndingInProgress")) {
            this->param_acl->log4(
                (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                "Self.Status", "status=EndingInProgress");

            this->session_probe_ending_in_progress = true;
        }
        else {
            const char * message   = session_probe_message.c_str();
            this->front.session_update({message, session_probe_message.size()});

            const char * separator = ::strchr(message, '=');

            bool message_format_invalid = false;

            if (separator) {
                std::string order(message, separator - message);
                std::string parameters(separator + 1);

                if (!order.compare("PasswordTextBox.SetFocus")) {
                    std::string info("status=\"" + parameters + "\"");
                    this->param_acl->log4(
                        (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                        order.c_str(), info.c_str());

                    this->front.set_focus_on_password_textbox(
                        !parameters.compare("yes"));
                }
                else if (!order.compare("ConsentUI.IsVisible")) {
                    std::string info("status=\"" + parameters + "\"");
                    this->param_acl->log4
                        ((this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                        order.c_str(), info.c_str());

                    this->front.set_consent_ui_visible(
                        !parameters.compare("yes"));
                }
                else if (!order.compare("InputLanguage")) {
                    const char * subitems          = parameters.c_str();
                    const char * subitem_separator =
                        ::strchr(subitems, '\x01');

                    if (subitem_separator) {
                        std::string code(subitems,
                                         subitem_separator - subitems);
                        std::string display_name(subitem_separator + 1);

                        std::string info(
                            "code=\"" + code +
                            "\" name=\"" + display_name + "\"");
                        this->param_acl->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());

                        this->front.set_keylayout(::strtol(code.c_str(),
                            nullptr, 16));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("NewProcess") ||
                         !order.compare("CompletedProcess")) {
                    std::string info("command_line=\"" + parameters + "\"");
                    this->param_acl->log4(
                        (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                        order.c_str(), info.c_str());
                }
                else if (!order.compare("OutboundConnectionBlocked")) {
                    const char * subitems          = parameters.c_str();
                    const char * subitem_separator =
                        ::strchr(subitems, '\x01');

                    if (subitem_separator) {
                        std::string rule(subitems,
                            subitem_separator - subitems);
                        std::string application_name(subitem_separator + 1);

                        std::string info(
                            "rule=\"" + rule +
                            "\" application_name=\"" + application_name + "\"");
                        this->param_acl->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());

                        char message[4096];
#ifdef __GNUG__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
# endif
                        snprintf(message, sizeof(message),
                            TR("process_interrupted_security_policies",
                               this->param_lang),
                            application_name.c_str());
#ifdef __GNUG__
    #pragma GCC diagnostic pop
# endif

                        std::string string_message = message;
                        this->mod.display_osd_message(string_message);
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("ForegroundWindowChanged")) {
                    const char * subitems          = parameters.c_str();
                    const char * subitem_separator =
                        ::strchr(subitems, '\x01');

                    if (subitem_separator) {
                        std::string text(subitems,
                            subitem_separator - subitems);
                        std::string remaining(subitem_separator + 1);

                        subitems          = remaining.c_str();
                        subitem_separator = ::strchr(subitems, '\x01');

                        if (subitem_separator) {
                            std::string window_class(subitems,
                                subitem_separator - subitems);
                            std::string command_line(subitem_separator + 1);

                            std::string info(
                                "text=\"" + text +
                                "\" class=\"" + window_class +
                                "\" command_line=\"" + command_line + "\"");
                            this->param_acl->log4(
                                (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                                order.c_str(), info.c_str());
                        }
                        else {
                            message_format_invalid = true;
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("Button.Clicked")) {
                    const char * subitems          = parameters.c_str();
                    const char * subitem_separator =
                        ::strchr(subitems, '\x01');

                    if (subitem_separator) {
                        std::string window(subitems,
                            subitem_separator - subitems);
                        std::string button(subitem_separator + 1);

                        std::string info(
                            "window=\"" + window +
                            "\" button=\"" + button + "\"");
                        this->param_acl->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!order.compare("Edit.Changed")) {
                    const char * subitems          = parameters.c_str();
                    const char * subitem_separator =
                        ::strchr(subitems, '\x01');

                    if (subitem_separator) {
                        std::string window(subitems,
                            subitem_separator - subitems);
                        std::string edit(subitem_separator + 1);

                        std::string info(
                            "window=\"" + window +
                            "\" edit=\"" + edit + "\"");
                        this->param_acl->log4(
                            (this->verbose & MODRDP_LOGLEVEL_SESPROBE),
                            order.c_str(), info.c_str());
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
};  // class SessionProbeVirtualChannel

#endif  // #ifndef REDEMPTION_MOD_RDP_CHANNELS_SESPROCHANNEL_HPP

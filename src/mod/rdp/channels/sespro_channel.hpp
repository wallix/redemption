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

#include "configs/config_access.hpp"
#include "configs/config.hpp"

#include "acl/auth_api.hpp"
#include "gdi/screen_functions.hpp"
#include "core/error.hpp"
#include "core/log_id.hpp"
#include "core/front_api.hpp"
#include "utils/timebase.hpp"
#include "core/window_constants.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/sespro_channel_params.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "utils/genrandom.hpp"
#include "utils/parse_server_message.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/uninit_checked.hpp"
#include "utils/translation.hpp"

#include <chrono>
#include <memory>
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
    OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK = 0x00000001,
    OPTION_UPDATE_DISABLED_FEATURES                             = 0x00000002,
    OPTION_LAUNCH_APPLICATION_THEN_TERMINATE                    = 0x00000004,
    OPTION_ENABLE_SELF_CLEANER                                  = 0x00000008,
    OPTION_DISCONNECT_SESSION_INSTEAD_OF_LOGOFF_SESSION         = 0x00000010
};

using SessionProbeVariables = vcfg::variables<
    vcfg::var<cfg::context::rd_shadow_invitation_error_code,    vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_error_message, vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_userdata,                 vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_id,            vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_addr,          vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_invitation_port,          vcfg::accessmode::set>,
    vcfg::var<cfg::context::rd_shadow_available,                vcfg::accessmode::set>
>;

class SessionProbeVirtualChannel final : public BaseVirtualChannel
{
public:
    struct Callbacks {
        virtual void freeze_screen() = 0;
        virtual void disable_graphics_update() = 0;
        virtual void enable_graphics_update() = 0;
        virtual void disable_input_event() = 0;
        virtual void enable_input_event() = 0;
        virtual void display_osd_message(std::string_view message) = 0;
        virtual ~Callbacks() = default;
    };

private:
    bool session_probe_ending_in_progress  = false;
    bool session_probe_keep_alive_received = true;
    bool session_probe_ready               = false;

    bool session_probe_launch_timeout_timer_started = false;

    const SessionProbeVirtualChannelParams sespro_params;

    std::string param_target_informations;

    const uint16_t param_front_width;
    const uint16_t param_front_height;

    std::string param_real_alternate_shell;
    std::string param_real_working_dir;

    Translator tr;

    bool param_bogus_refresh_rect_ex = false;

    const bool param_show_maximized;

    const bool param_disconnect_session_instead_of_logoff_session;

    FrontAPI& front;

    rdp_api& rdp;

    FileSystemVirtualChannel& file_system_virtual_channel;

    bool disconnection_reconnection_required = false; // Cause => Authenticated user changed.

    SessionProbeLauncher* session_probe_stop_launch_sequence_notifier = nullptr;

    std::string server_message;

    uint16_t other_version = 0x0100;

    bool start_application_query_processed = false;
    bool start_application_started         = false;

    Random & gen;

    uint32_t reconnection_cookie = INVALID_RECONNECTION_COOKIE;

    EventsGuard events_guard;
    Callbacks & callbacks;
    SessionLogApi& session_log;
    SessionProbeVariables vars;

    RDPVerbose verbose;
    EventRef session_probe_timer;
    bool launch_aborted = false;

    uint32_t options = 0;

    static long long ms2ll(std::chrono::milliseconds const& ms)
    {
        return ms.count();
    }

    void log6(LogId id, KVLogList kv_list)
    {
        this->session_log.log6(id, kv_list);

        if (REDEMPTION_UNLIKELY(bool(this->verbose & RDPVerbose::sesprobe))) {
            std::string msg;
            for (auto const& kv : kv_list) {
                str_append(msg, kv.key, '=', kv.value, ' ');
            }
            LOG(LOG_INFO, "type=%s %s", msg, detail::log_id_string_map[unsigned(id)].data());
        }
    }

public:

    struct Params
    {
        SessionProbeVirtualChannelParams sespro_params;

        uninit_checked<const char*> target_informations;

        uninit_checked<uint16_t> front_width;
        uninit_checked<uint16_t> front_height;

        uninit_checked<const char*> real_alternate_shell;
        uninit_checked<const char*> real_working_dir;

        uninit_checked<Language> lang;

        uninit_checked<bool> bogus_refresh_rect_ex;

        uninit_checked<bool> show_maximized;

        uninit_checked<bool> disconnect_session_instead_of_logoff_session;

        explicit Params() = default;
    };

    explicit SessionProbeVirtualChannel(
        EventContainer& events,
        SessionLogApi& session_log,
        SessionProbeVariables vars,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        rdp_api& rdp,
        FileSystemVirtualChannel& file_system_virtual_channel,
        Random & gen,
        const Params& params,
        Callbacks & callbacks,
        RDPVerbose verbose)
    : BaseVirtualChannel(nullptr, to_server_sender_)
    , sespro_params(params.sespro_params)
    , param_target_informations(params.target_informations)
    , param_front_width(params.front_width)
    , param_front_height(params.front_height)
    , param_real_alternate_shell(params.real_alternate_shell)
    , param_real_working_dir(params.real_working_dir)
    , tr(params.lang)
    , param_bogus_refresh_rect_ex(params.bogus_refresh_rect_ex)
    , param_show_maximized(params.show_maximized)
    , param_disconnect_session_instead_of_logoff_session(params.disconnect_session_instead_of_logoff_session)
    , front(front)
    , rdp(rdp)
    , file_system_virtual_channel(file_system_virtual_channel)
    , gen(gen)
    , events_guard(events)
    , callbacks(callbacks)
    , session_log(session_log)
    , vars(vars)
    , verbose(verbose)
    {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
            "SessionProbeVirtualChannel::SessionProbeVirtualChannel:"
                " effective_timeout=%lld on_launch_failure=%d",
            ms2ll(this->sespro_params.effective_launch_timeout),
            this->sespro_params.on_launch_failure);

        this->front.session_probe_started(false);
        this->front.set_focus_on_password_textbox(false);
        this->front.set_focus_on_unidentified_input_field(false);
        this->front.set_consent_ui_visible(false);
        this->front.set_session_locked(false);
    }

    ~SessionProbeVirtualChannel() = default;

    void enable_bogus_refresh_rect_ex_support(bool enable) {
        this->param_bogus_refresh_rect_ex = enable;
    }

    [[nodiscard]] bool has_been_launched() const {
        return this->session_probe_ready;
    }

    void start_launch_timeout_timer()
    {
        if (this->sespro_params.effective_launch_timeout.count() > 0
         && !this->session_probe_ready
        ) {
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO, "SessionProbeVirtualChannel::start_launch_timeout_timer");

            if (!this->session_probe_launch_timeout_timer_started) {
                this->session_probe_timer = this->events_guard.create_event_timeout(
                    "Session Probe Timer",
                    this->sespro_params.effective_launch_timeout,
                    [this](Event&event)
                    {
                        this->process_event_launch();
                        event.alarm.reset_timeout(this->events_guard.get_current_time()
                            + this->sespro_params.effective_launch_timeout);
                    });
                this->session_probe_launch_timeout_timer_started = true;
            }
        }
    }

    void abort_launch()
    {
        this->launch_aborted = true;

        this->session_probe_timer = this->events_guard.create_event_timeout(
            "Session Probe Timer",
            this->sespro_params.launcher_abort_delay,
            [this](Event&event){
                this->process_event_launch();
                event.garbage = true;
            });
    }

    void give_additional_launch_time() {
        if (!this->session_probe_ready && this->session_probe_timer && !this->launch_aborted) {
            this->session_probe_timer.reset_timeout(
                this->events_guard.get_current_time() + this->sespro_params.effective_launch_timeout);
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::give_additional_launch_time");
        }
    }

    bool is_disconnection_reconnection_required() {
        return this->disconnection_reconnection_required;
    }

private:
    void request_keep_alive() {
        this->session_probe_keep_alive_received = false;

        {
            StaticOutStream<1024> out_s;

            const size_t message_length_offset = out_s.get_offset();
            out_s.out_skip_bytes(sizeof(uint16_t));
            out_s.out_copy_bytes("Request=Keep-Alive"_av);
            out_s.out_clear_bytes(1);   // Null-terminator.

            out_s.stream_at(message_length_offset).out_uint16_le(
                out_s.get_offset() - message_length_offset - sizeof(uint16_t));

            this->send_message_to_server(out_s.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_s.get_produced_bytes());
        }

        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
            "SessionProbeVirtualChannel::request_keep_alive: "
                "Session Probe keep alive requested");

        this->session_probe_timer.reset_timeout(
            this->events_guard.get_current_time() + this->sespro_params.keepalive_timeout);
    }

    bool client_input_disabled_because_session_probe_keepalive_is_missing = false;

    void process_event_launch()
    {
        LOG(((this->sespro_params.on_launch_failure ==
                SessionProbeOnLaunchFailure::disconnect_user) ?
                LOG_ERR : LOG_WARNING),
            "SessionProbeVirtualChannel::process_event_launch: "
                "Session Probe is not ready yet!");

        error_type err_id = ERR_SESSION_PROBE_LAUNCH;

        if (this->session_probe_stop_launch_sequence_notifier) {
            this->session_probe_stop_launch_sequence_notifier->stop(false, err_id);
            this->session_probe_stop_launch_sequence_notifier = nullptr;
        }

        this->session_probe_timer.garbage();

        this->callbacks.enable_graphics_update();
        this->callbacks.enable_input_event();

        if (this->sespro_params.on_launch_failure != SessionProbeOnLaunchFailure::ignore_and_continue) {
            throw Error(err_id);
        }

        this->rdp.sespro_launch_process_ended();
    }

    void process_event_ready()
    {
        if (!this->session_probe_keep_alive_received) {
            LOG(LOG_ERR,
                "SessionProbeVirtualChannel::process_event_ready: "
                    "No keep alive received from Session Probe!");

            if (!this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                this->callbacks.enable_graphics_update();
                this->callbacks.enable_input_event();
            }

            if (!this->disconnection_reconnection_required) {
                if (this->session_probe_ending_in_progress) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_event_ready: "
                            "Session ending is in progress.");

                    if (this->sespro_params.at_end_of_session_freeze_connection_and_wait) {
                        LOG(LOG_INFO,
                            "SessionProbeVirtualChannel::process_event_ready: "
                                "Freezes connection and wait end of session.");

                        if (!this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                            this->callbacks.disable_graphics_update();
                            this->callbacks.disable_input_event();
                            this->client_input_disabled_because_session_probe_keepalive_is_missing = true;
                        }
                        this->request_keep_alive();
                        this->callbacks.display_osd_message("No keep alive received from Session Probe! (End of session in progress.)");
                    }
                    else {
                        LOG(LOG_INFO,
                            "SessionProbeVirtualChannel::process_event_ready: "
                                "Precipitates the end of the session.");

                        this->rdp.sespro_ending_in_progress();
                    }

                    return ;
                }

                if (SessionProbeOnKeepaliveTimeout::disconnect_user ==
                    this->sespro_params.on_keepalive_timeout) {
                    this->session_log.report("SESSION_PROBE_KEEPALIVE_MISSED", "");
                }
                else if (SessionProbeOnKeepaliveTimeout::freeze_connection_and_wait ==
                            this->sespro_params.on_keepalive_timeout) {

                    if (!this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                        this->callbacks.disable_graphics_update();
                        this->callbacks.disable_input_event();

                        this->client_input_disabled_because_session_probe_keepalive_is_missing = true;
                        this->callbacks.freeze_screen();
                    }
                    this->request_keep_alive();
                    this->callbacks.display_osd_message("No keep alive received from Session Probe!");
                }
                else {
                    this->front.session_probe_started(false);
                }
            }
        }
        else {
//            LOG(LOG_INFO, "================== SESSION PROBE KEEPALIVE =====================");
//            this->callbacks.display_osd_message("Session Probe Keepalive");
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

        out_s.stream_at(message_length_offset).out_uint16_le(
            out_s.get_offset() - message_length_offset - sizeof(uint16_t));

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_produced_bytes());
    }

public:
    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
            "SessionProbeVirtualChannel::process_server_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%zu",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::sesprobe_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }

        if (flags && !(flags &~ (CHANNELS::CHANNEL_FLAG_SUSPEND | CHANNELS::CHANNEL_FLAG_RESUME))) {
            return;
        }

        InStream chunk(chunk_data);

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
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
            "SessionProbeVirtualChannel::process_server_message: \"%s\"", this->server_message);

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

            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                this->options);
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
                    LOG(LOG_INFO, "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                        this->options);
                }

                bool const delay_disabled_launch_mask = (this->options & OPTION_DELAY_DISABLED_LAUNCH_MASK);

                error_type err_id = NO_ERROR;

                if (this->session_probe_ready) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Session Probe reconnection detect.");

                    if (!this->sespro_params.allow_multiple_handshake &&
                        (this->reconnection_cookie != remote_reconnection_cookie)) {
                        this->session_log.report("SESSION_PROBE_RECONNECTION", "");
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
                    this->callbacks.enable_input_event();
                    this->callbacks.enable_graphics_update();
                }

                if (!this->sespro_params.launch_application_driver) {
                    this->file_system_virtual_channel.disable_session_probe_drive();
                }

                this->session_probe_timer.garbage();
                this->rdp.sespro_launch_process_ended();

                // The order of the messages sent is very important!

                if (this->sespro_params.keepalive_timeout.count() > 0) {
                    send_client_message([](OutStream & out_s) {
                        out_s.out_copy_bytes("Request=Keep-Alive"_av);
                    });

                    LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "Session Probe keep alive requested");

                    this->session_probe_timer = this->events_guard.create_event_timeout(
                        "Session Probe Keepalive Timer",
                        this->sespro_params.keepalive_timeout,
                        [this](Event&event)
                        {
                            this->process_event_ready();
                            event.alarm.reset_timeout(event.alarm.now+this->sespro_params.keepalive_timeout);
                        });
                }

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("Version=" "1" "\x01" "6"_av);
                });

                {
                    if (this->sespro_params.end_of_session_check_delay_time.count() > 0) {
                        send_client_message([this](OutStream & out_s) {
                            out_s.out_copy_bytes("EndOfSessionCheckDelayTime="_av);

                            {
                                char cstr[128];
                                int len = std::snprintf(cstr, sizeof(cstr), "%ld",
                                    this->sespro_params.end_of_session_check_delay_time.count());
                                out_s.out_copy_bytes(cstr, size_t(len));
                            }
                        });
                    }
                }

                {
                    uint32_t options = 0;

                    if (this->sespro_params.ignore_ui_less_processes_during_end_of_session_check) {
                        options |= OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK;
                    }

                    if (this->sespro_params.update_disabled_features) {
                        options |= OPTION_UPDATE_DISABLED_FEATURES;
                    }

                    if (this->sespro_params.launch_application_driver_then_terminate) {
                        options |= OPTION_LAUNCH_APPLICATION_THEN_TERMINATE;
                    }

                    if (this->sespro_params.enable_self_cleaner) {
                        options |= OPTION_ENABLE_SELF_CLEANER;
                    }

                    if (this->param_disconnect_session_instead_of_logoff_session) {
                        options |= OPTION_DISCONNECT_SESSION_INSTEAD_OF_LOGOFF_SESSION;
                    }

                    if (options)
                    {
                        send_client_message([options](OutStream & out_s) {
                            out_s.out_copy_bytes("Options="_av);

                            {
                                char cstr[128];
                                int len = std::snprintf(cstr, sizeof(cstr), "%u", options);
                                out_s.out_copy_bytes(cstr, size_t(len));
                            }
                        });
                    }
                }

                if (this->sespro_params.on_account_manipulation != SessionProbeOnAccountManipulation::allow) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("AccountManipulationAction="_av);

                        if (this->sespro_params.on_account_manipulation == SessionProbeOnAccountManipulation::notify) {
                            out_s.out_copy_bytes("notify"_av);
                        }
                        else {
                            out_s.out_copy_bytes("deny"_av);
                        }
                    });
                }

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("ChildlessWindowAsUnidentifiedInputField="_av);

                    if (this->sespro_params.childless_window_as_unidentified_input_field) {
                        out_s.out_copy_bytes("Yes"_av);
                    }
                    else {
                        out_s.out_copy_bytes("No"_av);
                    }
                });

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("ExtraInfo="_av);

                    {
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%d", ::getpid());
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("AutomaticallyEndDisconnectedSession="_av);

                    if (this->sespro_params.end_disconnected_session) {
                        out_s.out_copy_bytes("Yes"_av);
                    }
                    else {
                        out_s.out_copy_bytes("No"_av);
                    }
                });

                {
                    unsigned int const disconnect_session_limit =
                        (this->param_real_alternate_shell.empty() ?
                         // Normal RDP session
                         this->sespro_params.disconnected_session_limit.count() :
                         // Application session
                         this->sespro_params.disconnected_application_limit.count());

                    if (disconnect_session_limit) {
                        send_client_message([disconnect_session_limit](OutStream & out_s) {
                            out_s.out_copy_bytes("DisconnectedSessionLimit="_av);

                            {
                                char cstr[128];
                                int len = std::snprintf(cstr, sizeof(cstr), "%u",
                                    disconnect_session_limit);
                                out_s.out_copy_bytes(cstr, size_t(len));
                            }
                        });
                    }
                }

                if (this->sespro_params.idle_session_limit.count()) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("IdleSessionLimit="_av);

                        {
                            char cstr[128];
                            int len = std::snprintf(cstr, sizeof(cstr), "%lld",
                                ms2ll(this->sespro_params.idle_session_limit));
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
                        out_s.out_copy_bytes("ReconnectionCookie="_av);

                        {
                            char cstr[128];
                            int len = std::snprintf(cstr, sizeof(cstr), "%u",
                                this->reconnection_cookie);
                            out_s.out_copy_bytes(cstr, size_t(len));
                        }
                    });
                }

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("EnableCrashDump="_av);

                    if (this->sespro_params.enable_crash_dump) {
                        out_s.out_copy_bytes("Yes"_av);
                    }
                    else {
                        out_s.out_copy_bytes("No"_av);
                    }
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("Bushido="_av);

                    {
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%u",
                            this->sespro_params.handle_usage_limit,
                            this->sespro_params.memory_usage_limit);
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("BestSafeIntegration="_av);

                    if (this->sespro_params.bestsafe_integration) {
                        out_s.out_copy_bytes("Yes"_av);
                    }
                    else {
                        out_s.out_copy_bytes("No"_av);
                    }
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("DisabledFeatures="_av);

                    {
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "0x%08X",
                            static_cast<unsigned>(this->sespro_params.disabled_features));
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }
                });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "DisableLaunchMask")) {
                this->callbacks.enable_input_event();
                this->callbacks.enable_graphics_update();
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "DisableRedirectedDrive")) {
                assert(this->sespro_params.launch_application_driver);

                this->file_system_virtual_channel.disable_session_probe_drive();
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get target informations")) {
                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("TargetInformations="_av);
                    out_s.out_copy_bytes(this->param_target_informations);
                });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get startup application")) {
                if (this->param_real_alternate_shell != "[None]" ||
                    this->start_application_started) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("StartupApplication="_av);

                        if (this->param_real_alternate_shell.empty()) {
                            out_s.out_copy_bytes("[Windows Explorer]"_av);
                        }
                        else if (this->param_real_alternate_shell == "[None]") {
                            out_s.out_copy_bytes("[None]"_av);
                        }
                        else {
                            if (!this->param_real_working_dir.empty()) {
                                out_s.out_copy_bytes(this->param_real_working_dir);
                            }
                            out_s.out_uint8('\x01');

                            out_s.out_copy_bytes(this->param_real_alternate_shell);

                            if (0x0102 <= this->other_version) {
                                if (!this->param_show_maximized) {
                                    out_s.out_uint8('\x01');
                                    out_s.out_copy_bytes("Normal"_av);
                                }
                            }
                        }
                    });
                }

                this->start_application_query_processed = true;
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Disconnection-Reconnection")) {
                LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Disconnection-Reconnection required.");

                this->disconnection_reconnection_required = true;

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("Confirm=Disconnection-Reconnection"_av);
                });
            }
            else if (!::strcasecmp(parameters_[0].c_str(), "Get extra system process") &&
                     (2 <= parameters_.size())) {
                const unsigned int proc_index =
                    ::strtoul(parameters_[1].c_str(), nullptr, 10);

                // ExtraSystemProcess=ProcIndex\x01ErrorCode[\x01ProcName]
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, proc_index](OutStream & out_s) {
                    out_s.out_copy_bytes("ExtraSystemProcess="_av);

                    std::string name;

                    const bool result =
                        this->sespro_params.extra_system_processes.get(proc_index, name);

                    {
                        const int error_code = (result ? 0 : -1);
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                            proc_index, error_code);
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }

                    if (result) {
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(name);
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
                    out_s.out_copy_bytes("OutboundConnectionMonitoringRule="_av);

                    unsigned int type = 0;
                    std::string  host_address_or_subnet;
                    std::string  port_range;
                    std::string  description;

                    const bool result =
                        this->sespro_params.outbound_connection_monitor_rules.get(
                            rule_index, type, host_address_or_subnet, port_range,
                            description);

                    {
                        const int error_code = (result ? 0 : -1);
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                            rule_index, error_code);
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }

                    if (result) {
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01", type);
                        out_s.out_copy_bytes(cstr, size_t(len));
                        out_s.out_copy_bytes(host_address_or_subnet);
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(port_range);
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
                    out_s.out_copy_bytes("ProcessMonitoringRule="_av);

                    unsigned int type = 0;
                    std::string  pattern;
                    std::string  description;

                    const bool result =
                        this->sespro_params.process_monitor_rules.get(
                            rule_index, type, pattern, description);

                    {
                        const int error_code = (result ? 0 : -1);
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                            rule_index, error_code);
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }

                    if (result) {
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "\x01" "%u" "\x01", type);
                        out_s.out_copy_bytes(cstr, size_t(len));
                        out_s.out_copy_bytes(pattern);
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
                    out_s.out_copy_bytes("WindowsOfApplicationAsUnidentifiedInputField="_av);

                    std::string name;

                    const bool result =
                        this->sespro_params.windows_of_these_applications_as_unidentified_input_field.get(app_index, name);

                    {
                        const int error_code = (result ? 0 : -1);
                        char cstr[128];
                        int len = std::snprintf(cstr, sizeof(cstr), "%u" "\x01" "%d",
                            app_index, error_code);
                        out_s.out_copy_bytes(cstr, size_t(len));
                    }

                    if (result) {
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(name);
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

            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "OtherVersion=%u.%u",
                unsigned(major), unsigned(minor));

            if (this->sespro_params.enable_log) {
                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("EnableLog=Yes"_av);

                    if (0x0103 <= this->other_version) {
                        out_s.out_uint8('\x01');
                        if (this->sespro_params.enable_log_rotation) {
                            out_s.out_copy_bytes("Yes"_av);
                        }
                        else {
                            out_s.out_copy_bytes("No"_av);
                        }

                        if (0x0104 <= this->other_version) {
                            out_s.out_uint8('\x01');

                            char cstr[128];
                            int len = std::snprintf(cstr, sizeof(cstr), "%d",
                                static_cast<int>(this->sespro_params.log_level));
                            out_s.out_copy_bytes(cstr, size_t(len));
                        }
                        else {
                            if (this->sespro_params.log_level >= SessionProbeLogLevel::Off) {
                                LOG(LOG_WARNING,
                                    "SessionProbeVirtualChannel::process_server_message: "
                                        "Log levels are not supported by Session Probe! OtherVersion=0x%X",
                                    this->other_version);
                            }
                        }
                    }
                    else {
                        if (this->sespro_params.enable_log_rotation) {
                            LOG(LOG_WARNING,
                                "SessionProbeVirtualChannel::process_server_message: "
                                    "Log file rotation is not supported by Session Probe! OtherVersion=0x%X",
                                this->other_version);
                        }
                    }
                });
            }
        }
        else if (!::strcasecmp(order_.c_str(), "Log") && !parameters_.empty()) {
            LOG(LOG_INFO, "SessionProbe: %s", parameters_[0]);
        }
        else if (!::strcasecmp(order_.c_str(), "KeepAlive") && !parameters_.empty() &&
                 !::strcasecmp(parameters_[0].c_str(), "OK")) {
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_repetitive), LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "Received Keep-Alive from Session Probe.");
            this->session_probe_keep_alive_received = true;

            if (this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                this->callbacks.enable_input_event();
                this->callbacks.enable_graphics_update();

                std::string string_message;
                this->callbacks.display_osd_message(string_message);
                this->request_keep_alive();
                this->client_input_disabled_because_session_probe_keepalive_is_missing = false;
            }
        }
        else if (!::strcasecmp(order_.c_str(), "SESSION_ENDING_IN_PROGRESS")) {
            this->log6(LogId::SESSION_ENDING_IN_PROGRESS, {});

            this->session_probe_ending_in_progress = true;
        }
        else {
            bool message_format_invalid = false;

            if (!parameters_.empty()) {

                if (!::strcasecmp(order_.c_str(), "KERBEROS_TICKET_CREATION") ||
                    !::strcasecmp(order_.c_str(), "KERBEROS_TICKET_DELETION")) {
                    if (parameters_.size() == 7) {
                        this->log6(
                            !::strcasecmp(order_.c_str(), "KERBEROS_TICKET_CREATION")
                                ? LogId::KERBEROS_TICKET_CREATION
                                : LogId::KERBEROS_TICKET_DELETION, {
                            KVLog("encryption_type"_av, parameters_[0]),
                            KVLog("client_name"_av,     parameters_[1]),
                            KVLog("server_name"_av,     parameters_[2]),
                            KVLog("start_time"_av,      parameters_[3]),
                            KVLog("end_time"_av,        parameters_[4]),
                            KVLog("renew_time"_av,      parameters_[5]),
                            KVLog("flags"_av,           parameters_[6]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "BESTSAFE_SERVICE_LOG")) {
                    KVLog kvlogs[255];
                    array_view logs {parameters_};
                    assert(logs.size() % 2 == 0);
                    while (logs.size() > 1) {
                        // WrmChunkType::SESSION_UPDATE is limited to 255 entry
                        auto n = std::min(logs.size() / 2, std::size_t(255));
                        auto first = logs.first(n * 2);
                        auto* pkv = kvlogs;
                        // WrmChunkType::SESSION_UPDATE packet len
                        int data_len = 0;
                        for (size_t i = 0, c = first.size() / 2; i < c; ++i) {
                            *pkv = KVLog{first[i * 2], first[i * 2 + 1]};
                            data_len += int(pkv->key.size() + pkv->value.size()) + 3;
                            ++pkv;
                            // maximal size of WrmChunkType::SESSION_UPDATE
                            if (data_len > 1024 * 16 - 10) {
                                n = i + 1u;
                                break;
                            }
                        }
                        logs = logs.from_offset(n);
                        this->log6(LogId::BESTSAFE_SERVICE_LOG, array_view{kvlogs, pkv});
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "PASSWORD_TEXT_BOX_GET_FOCUS")) {
                    this->log6(
                        LogId::PASSWORD_TEXT_BOX_GET_FOCUS, {
                        KVLog("status"_av, parameters_[0]),
                    });

                    if (parameters_.size() == 1) {
                        this->front.set_focus_on_password_textbox(
                            !::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "UNIDENTIFIED_INPUT_FIELD_GET_FOCUS")) {
                    this->log6(
                        LogId::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS, {
                        KVLog("status"_av, parameters_[0]),
                    });

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
                        this->log6(
                            LogId::UAC_PROMPT_BECOME_VISIBLE, {
                            KVLog("status"_av, parameters_[0]),
                        });

                        this->front.set_consent_ui_visible(!::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "SESSION_LOCKED")) {
                    if (parameters_.size() == 1) {
                        this->log6(
                            LogId::SESSION_LOCKED, {
                            KVLog("status"_av, parameters_[0]),
                        });

                        this->front.set_session_locked(!::strcasecmp(parameters_[0].c_str(), "yes"));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "INPUT_LANGUAGE")) {
                    if (parameters_.size() == 2) {
                        this->log6(
                            LogId::INPUT_LANGUAGE, {
                            KVLog("identifier"_av,   parameters_[0]),
                            KVLog("display_name"_av, parameters_[1]),
                        });

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
                        this->log6(
                            !::strcasecmp(order_.c_str(), "NEW_PROCESS")
                                ? LogId::NEW_PROCESS
                                : LogId::COMPLETED_PROCESS, {
                            KVLog("command_line"_av, parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "STARTUP_APPLICATION_FAIL_TO_RUN")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::STARTUP_APPLICATION_FAIL_TO_RUN, {
                            KVLog("application_name"_av, parameters_[0]),
                            KVLog("raw_result"_av,       parameters_[1]),
                        });

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: "
                            "app_name=%s  raw_result=%s",
                            parameters_[0], parameters_[1]);

                        this->session_log.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "STARTUP_APPLICATION_FAIL_TO_RUN_2")) {
                    if (parameters_.size() == 3) {
                        this->log6(
                            LogId::STARTUP_APPLICATION_FAIL_TO_RUN_2, {
                            KVLog("application_name"_av,   parameters_[0]),
                            KVLog("raw_result"_av,         parameters_[1]),
                            KVLog("raw_result_message"_av, parameters_[2]),
                        });

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: "
                            "app_name=%s  raw_result=%s  raw_result_message=%s",
                            parameters_[0], parameters_[1], parameters_[2]);

                        this->session_log.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_BLOCKED")) {
                    if (parameters_.size() == 2) {
                        this->log6(
                            LogId::OUTBOUND_CONNECTION_BLOCKED, {
                            KVLog("rule"_av,             parameters_[0]),
                            KVLog("application_name"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "OUTBOUND_CONNECTION_DETECTED")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::OUTBOUND_CONNECTION_DETECTED, {
                            KVLog("rule"_av,             parameters_[0]),
                            KVLog("application_name"_av, parameters_[1]),
                        });

                        char message[4096];

                        this->tr.fmt(message, sizeof(message),
                            trkeys::process_interrupted_security_policies,
                            parameters_[1].c_str());

                        this->callbacks.display_osd_message(message);
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
                            this->sespro_params.outbound_connection_monitor_rules.get(
                                ::strtoul(parameters_[0].c_str(), nullptr, 10),
                                type, host_address_or_subnet, port_range,
                                description);

                        if (result) {
                            this->log6(
                                deny
                                    ? LogId::OUTBOUND_CONNECTION_BLOCKED_2
                                    : LogId::OUTBOUND_CONNECTION_DETECTED_2, {
                                KVLog("rule"_av,         description),
                                KVLog("app_name"_av,     parameters_[1]),
                                KVLog("app_cmd_line"_av, parameters_[2]),
                                KVLog("dst_addr"_av,     parameters_[3]),
                                KVLog("dst_port"_av,     parameters_[4]),
                            });

                            {
                                char message[4096];

                                // rule, app_name, app_cmd_line, dst_addr, dst_port
                                snprintf(message, sizeof(message), "%s|%s|%s|%s|%s",
                                    description.c_str(), parameters_[1].c_str(), parameters_[2].c_str(),
                                    parameters_[3].c_str(), parameters_[4].c_str());

                                this->session_log.report(
                                    (deny ? "FINDCONNECTION_DENY" : "FINDCONNECTION_NOTIFY"),
                                    message);
                            }

                            if (deny) {
                                if (::strtoul(parameters_[5].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block outbound connection!");
                                    this->session_log.report(
                                        "SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    this->tr.fmt(message, sizeof(message),
                                        trkeys::process_interrupted_security_policies,
                                        parameters_[1].c_str());

                                    std::string string_message = message;
                                    this->callbacks.display_osd_message(string_message);
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
                            this->sespro_params.process_monitor_rules.get(
                                ::strtoul(parameters_[0].c_str(), nullptr, 10),
                                type, pattern, description);

                        if (result) {
                            this->log6(
                                deny ? LogId::PROCESS_BLOCKED : LogId::PROCESS_DETECTED, {
                                KVLog("rule"_av,         description),
                                KVLog("app_name"_av,     parameters_[1]),
                                KVLog("app_cmd_line"_av, parameters_[2]),
                            });

                            {
                                char message[4096];

                                // rule, app_name, app_cmd_line
                                snprintf(message, sizeof(message), "%s|%s|%s",
                                    description.c_str(), parameters_[1].c_str(), parameters_[2].c_str());

                                this->session_log.report(
                                    (deny ? "FINDPROCESS_DENY" : "FINDPROCESS_NOTIFY"),
                                    message);
                            }

                            if (deny) {
                                if (::strtoul(parameters_[3].c_str(), nullptr, 10)) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block process!");
                                    this->session_log.report(
                                        "SESSION_PROBE_PROCESS_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    this->tr.fmt(message, sizeof(message),
                                        trkeys::process_interrupted_security_policies,
                                        parameters_[1].c_str());

                                    this->callbacks.display_osd_message(message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "ACCOUNT_MANIPULATION_BLOCKED") ||
                         !::strcasecmp(order_.c_str(), "ACCOUNT_MANIPULATION_DETECTED")) {
                    bool deny = (!::strcasecmp(order_.c_str(), "ACCOUNT_MANIPULATION_BLOCKED"));

                    if (parameters_.size() == 6) {
                        this->log6(
                            deny ? LogId::ACCOUNT_MANIPULATION_BLOCKED : LogId::ACCOUNT_MANIPULATION_DETECTED, {
                            KVLog("operation"_av,    parameters_[0]),
                            KVLog("server_name"_av,  parameters_[1]),
                            KVLog("group_name"_av,   parameters_[2]),
                            KVLog("account_name"_av, parameters_[3]),
                            KVLog("app_name"_av,     parameters_[4]),
                            KVLog("app_cmd_line"_av, parameters_[5]),
                        });

                        {
                            char message[4096];

                            // operation, server_name, group_name, account_name, app_name, app_cmd_line
                            snprintf(message, sizeof(message), "%s|%s|%s|%s|%s|%s",
                                parameters_[0].c_str(), parameters_[1].c_str(), parameters_[2].c_str(),
                                parameters_[3].c_str(), parameters_[4].c_str(), parameters_[5].c_str());

                            this->session_log.report(
                                (deny ? "ACCOUNTMANIPULATION_DENY" : "ACCOUNTMANIPULATION_NOTIFY"),
                                message);
                        }

                        if (deny) {
                            char message[4096];

                            this->tr.fmt(message, sizeof(message),
                                trkeys::account_manipulation_blocked_security_policies,
                                parameters_[3].c_str());

                            this->callbacks.display_osd_message(message);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "FOREGROUND_WINDOW_CHANGED")) {
                    if (not parameters_.empty()) {
                        this->log6(LogId::TITLE_BAR, {
                            KVLog("source"_av, "Probe"_av),
                            KVLog("window"_av, parameters_[0]),
                        });
                    }
                    if ((parameters_.size() == 2) || (parameters_.size() == 3)) {
                        this->log6(LogId::FOREGROUND_WINDOW_CHANGED, {
                            KVLog("text"_av,         parameters_[0]),
                            KVLog("class_name"_av,   parameters_[1]),
                            KVLog("command_line"_av, (parameters_.size() == 2)
                                ? chars_view{} : parameters_[2]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "BUTTON_CLICKED")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::BUTTON_CLICKED, {
                            KVLog("window"_av, parameters_[0]),
                            KVLog("button"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "CHECKBOX_CLICKED")) {
                    if (parameters_.size() == 3) {
                        this->log6(LogId::CHECKBOX_CLICKED, {
                            KVLog("window"_av, parameters_[0]),
                            KVLog("checkbox"_av, parameters_[1]),
                            KVLog("state"_av,
                                ::button_state_to_string(::atoi(parameters_[2].c_str()))),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (!::strcasecmp(order_.c_str(), "EDIT_CHANGED")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::EDIT_CHANGED, {
                            KVLog("window"_av, parameters_[0]),
                            KVLog("edit"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (!::strcasecmp(order_.c_str(), "WEB_ATTEMPT_TO_PRINT")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::WEB_ATTEMPT_TO_PRINT, {
                            KVLog("url"_av,   parameters_[0]),
                            KVLog("title"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_BEFORE_NAVIGATE")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::WEB_BEFORE_NAVIGATE, {
                            KVLog("url"_av,  parameters_[0]),
                            KVLog("post"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_DOCUMENT_COMPLETE")) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::WEB_DOCUMENT_COMPLETE, {
                            KVLog("url"_av,   parameters_[0]),
                            KVLog("title"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_NAVIGATE_ERROR")) {
                    if (parameters_.size() == 4) {
                        this->log6(LogId::WEB_NAVIGATE_ERROR, {
                            KVLog("url"_av,          parameters_[0]),
                            KVLog("title"_av,        parameters_[1]),
                            KVLog("code"_av,         parameters_[2]),
                            KVLog("display_name"_av, parameters_[3]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_NAVIGATION")) {
                    if (parameters_.size() == 1) {
                        this->log6(LogId::WEB_NAVIGATION, {
                            KVLog("url"_av,   parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_PRIVACY_IMPACTED")) {
                    if (parameters_.size() == 1) {
                        this->log6(LogId::WEB_PRIVACY_IMPACTED, {
                            KVLog("impacted"_av, parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_ENCRYPTION_LEVEL_CHANGED")) {
                    if (parameters_.size() == 2) {
                        this->log6(
                            LogId::WEB_ENCRYPTION_LEVEL_CHANGED, {
                            KVLog("identifier"_av,   parameters_[0]),
                            KVLog("display_name"_av, parameters_[1]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (!::strcasecmp(order_.c_str(), "WEB_THIRD_PARTY_URL_BLOCKED")) {
                    if (parameters_.size() == 1) {
                        this->log6(LogId::WEB_THIRD_PARTY_URL_BLOCKED, {
                            KVLog("url"_av,   parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (!::strcasecmp(order_.c_str(), "GROUP_MEMBERSHIP")) {
                    if (parameters_.size() == 1) {
                        this->log6(LogId::GROUP_MEMBERSHIP, {
                            KVLog("groups"_av, parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (!::strcasecmp(order_.c_str(), "SHADOW_SESSION_SUPPORTED")) {
                    if (parameters_.size() == 1) {
                        if ((!::strcasecmp(parameters_[0].c_str(), "yes"))
                         && this->sespro_params.session_shadowing_support
                        ) {
                            this->vars.set_acl<cfg::context::rd_shadow_available>(true);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (!::strcasecmp(order_.c_str(), "SHADOW_SESSION_RESPONSE")) {
                    if (parameters_.size() >= 3)
                    {
                        const uint32_t shadow_errcode = ::strtoul(parameters_[0].c_str(), nullptr, 16);
                        const auto&    shadow_errmsg  = parameters_[1];
                        const auto&    shadow_userdata = parameters_[2];
                        if (parameters_.size() >= 6) {
                            const auto&    shadow_id   = parameters_[3];
                            const auto&    shadow_addr = parameters_[4];
                            const uint16_t shadow_port = ::strtoul(parameters_[5].c_str(), nullptr, 10);

                            this->set_rd_shadow_invitation(shadow_errcode, shadow_errmsg, shadow_userdata, shadow_id, shadow_addr, shadow_port);
                        }
                        else {
                            this->set_rd_shadow_invitation(shadow_errcode, shadow_errmsg, shadow_userdata, "", "", 0);
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
        out_s.out_copy_bytes("StartupApplication="_av);
        out_s.out_copy_bytes("[None]"_av);
        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.stream_at(message_length_offset).out_uint16_le(
            out_s.get_offset() - message_length_offset - sizeof(uint16_t));

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_produced_bytes());
    }

    void rail_exec(const char* application_name, const char* command_line,
        const char* current_directory, bool show_maximized, uint16_t flags) {
        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));
        out_s.out_copy_bytes("Execute="_av);

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
            out_s.out_copy_bytes("Minimized"_av);
        }
        else {
            out_s.out_copy_bytes("Normal"_av);
        }

        out_s.out_uint8('\x01');
        {
            char cstr[128];
            int len = std::snprintf(cstr, sizeof(cstr), "%u", flags);
            out_s.out_copy_bytes(cstr, size_t(len));
        }

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.stream_at(message_length_offset).out_uint16_le(
            out_s.get_offset() - message_length_offset - sizeof(uint16_t));

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_produced_bytes());
    }

    void create_shadow_session(const char * userdata, const char * type) {
        LOG(LOG_INFO, "sespro_channel::create_shadow_session()");

        bool bTakeControl       = true;
        bool bRequestPermission = true;

        if (!strcasecmp(type, "PermissionControl")) {
            bTakeControl       = true;
            bRequestPermission = true;
        }
        else if (!strcasecmp(type, "PermissionView")) {
            bTakeControl       = false;
            bRequestPermission = true;
        }
        else if (!strcasecmp(type, "SilentControl")) {
            bTakeControl       = true;
            bRequestPermission = false;
        }
        else if (!strcasecmp(type, "SilentView")) {
            bTakeControl       = false;
            bRequestPermission = false;
        }
        else {
            LOG(LOG_WARNING,
                "SessionProbeVirtualChannel::create_shadow_session: "
                    "Invalid shadow session type! Operation canceled. Type=%s",
                type);

            this->set_rd_shadow_invitation(
                    0x80004005, // E_FAIL
                    "The shadow session type specified is invalid!",
                    userdata,
                    "",
                    "",
                    0
                );

            return;
        }

        send_client_message([bTakeControl, bRequestPermission, userdata](OutStream & out_s) {
            out_s.out_copy_bytes("SHADOW_SESSION_REQUEST="_av);

            if (bTakeControl) {
                out_s.out_copy_bytes("Yes"_av);
            }
            else {
                out_s.out_copy_bytes("No"_av);
            }

            out_s.out_uint8('\x01');

            if (bRequestPermission) {
                out_s.out_copy_bytes("Yes"_av);
            }
            else {
                out_s.out_copy_bytes("No"_av);
            }

            out_s.out_uint8('\x01');

            out_s.out_copy_bytes(userdata, ::strlen(userdata));

        });
    }

private:
    void set_rd_shadow_invitation(
        uint32_t error_code, std::string_view error_message,
        std::string_view userdata, std::string_view id,
        std::string_view addr, uint16_t port
    ) {
        this->vars.set_acl<cfg::context::rd_shadow_invitation_error_code>(error_code);
        this->vars.set_acl<cfg::context::rd_shadow_invitation_error_message>(error_message);
        this->vars.set_acl<cfg::context::rd_shadow_userdata>(userdata);
        this->vars.set_acl<cfg::context::rd_shadow_invitation_id>(id);
        this->vars.set_acl<cfg::context::rd_shadow_invitation_addr>(addr);
        this->vars.set_acl<cfg::context::rd_shadow_invitation_port>(port);
    }
};  // class SessionProbeVirtualChannel

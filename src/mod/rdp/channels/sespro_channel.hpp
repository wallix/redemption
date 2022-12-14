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
#include "acl/kv_list_from_strings.hpp"
#include "gdi/screen_functions.hpp"
#include "keyboard/keylayouts.hpp"
#include "core/error.hpp"
#include "core/log_id.hpp"
#include "core/front_api.hpp"
#include "utils/timebase.hpp"
#include "core/window_constants.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/sespro_channel_params.hpp"
#include "mod/rdp/channels/sespro_api.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "utils/genrandom.hpp"
#include "utils/parse_server_message.hpp"
#include "utils/stream.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/uninit_checked.hpp"
#include "utils/translation.hpp"
#include "utils/sugar/chars_to_int.hpp"
#include "utils/ascii.hpp"

#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <cinttypes> // PRId64, ...
#include <cstring>


enum {
    INVALID_RECONNECTION_COOKIE = 0xFFFFFFFF
};

// Proxy Options
enum {
    OPTION_DELAY_DISABLED_LAUNCH_MASK      = 0x00000001,
    OPTION_DELAY_DISABLED_REDIRECTED_DRIVE = 0x00000002
};

// Session Probe Options
enum {
    OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK = 0x00000001,
    OPTION_UPDATE_DISABLED_FEATURES                             = 0x00000002,
    OPTION_LAUNCH_APPLICATION_THEN_TERMINATE                    = 0x00000004,
    OPTION_ENABLE_SELF_CLEANER                                  = 0x00000008,
    OPTION_DISCONNECT_SESSION_INSTEAD_OF_LOGOFF_SESSION         = 0x00000010,
    OPTION_SUPPORT_MULTIPLE_NETWORK_INTERFACES_IN_SHADOW_SESSION_RESPONSE
                                                                = 0x00000020,
    OPTION_REMOTE_PROGRAM_SESSION                               = 0x00000040
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

class SessionProbeVirtualChannel final : public BaseVirtualChannel, public sespro_api
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
    static constexpr std::string_view REPLACEMENT_HIDE = "********";
    static constexpr char TAG_HIDE = '\x03';

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
            LOG(LOG_INFO, "type=%s %s", msg, detail::log_id_string_map[unsigned(id)]);
        }
    }

    static void out_selected_string(OutStream & out, bool cond, chars_view yes, chars_view no) noexcept
    {
        if (cond) {
            out.out_copy_bytes(yes);
        }
        else {
            out.out_copy_bytes(no);
        }
    }

    static void out_yes_or_no(OutStream & out, bool cond) noexcept
    {
        out_selected_string(out, cond, "Yes"_av, "No"_av);
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
                        event.alarm.reset_timeout(this->events_guard.get_monotonic_time()
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
                this->events_guard.get_monotonic_time() + this->sespro_params.effective_launch_timeout);
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::give_additional_launch_time");
        }
    }

    bool is_disconnection_reconnection_required() const {
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
            this->events_guard.get_monotonic_time() + this->sespro_params.keepalive_timeout);
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
            // LOG(LOG_INFO, "================== SESSION PROBE KEEPALIVE =====================");
            // this->callbacks.display_osd_message("Session Probe Keepalive");
            this->request_keep_alive();
        }
    }

    template <class T>
    void send_client_message(T t)
    {
        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));

        t(out_s);

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.stream_at(message_length_offset).out_uint16_le(
            checked_int{out_s.get_offset() - message_length_offset - sizeof(uint16_t)});

        this->send_message_to_server(checked_int{out_s.get_offset()},
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_produced_bytes());
    }

public:
    void process_server_message(uint32_t total_length, uint32_t flags, bytes_view chunk_data) override
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

        if (flags && !(flags & ~uint32_t(CHANNELS::CHANNEL_FLAG_SUSPEND | CHANNELS::CHANNEL_FLAG_RESUME))) {
            return;
        }

        InStream chunk(chunk_data);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->server_message.clear();
            uint16_t message_length = chunk.in_uint16_le();
            this->server_message.reserve(message_length);
        }

        this->server_message.append(char_ptr_cast(chunk.get_current()),
            chunk.in_remain());

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            return;
        }

        if (this->server_message[0] != '\0') {
            while (this->server_message.back() == '\0') {
                this->server_message.pop_back();
            }
        }
        LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
            "SessionProbeVirtualChannel::process_server_message: \"%s\"", this->server_message);

        ParseServerMessage parse_server_message_result;
        if (!parse_server_message_result.parse(this->server_message)) {
            LOG(LOG_WARNING,
                "SessionProbeVirtualChannel::process_server_message: "
                    "Failed to parse server message. Message=\"%s\"", this->server_message);
            return;
        }

        auto const upper_order = parse_server_message_result.upper_order();
        auto const parameters_ = parse_server_message_result.parameters();

        if (upper_order == "Options"_ascii_upper && !parameters_.empty()) {
            this->options = unchecked_decimal_chars_to_int(parameters_[0]);

            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                this->options);
        }
        else if (upper_order == "Request"_ascii_upper && !parameters_.empty()) {
            auto const upper_param0 = ascii_to_limited_upper<126>(parameters_[0]);
            if (upper_param0 == "Hello"_ascii_upper) {
                LOG(LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Session Probe is ready.");

                this->windows_and_notification_icons_synchronized = false;

                uint32_t remote_reconnection_cookie = INVALID_RECONNECTION_COOKIE;
                if (parameters_.size() > 1) {
                    remote_reconnection_cookie = unchecked_decimal_chars_to_int(parameters_[1]);
                }

                if (bool(this->verbose & RDPVerbose::sesprobe)) {
                    LOG(LOG_INFO,
                        "SessionProbeVirtualChannel::process_server_message: "
                            "LocalCookie=0x%X RemoteCookie=0x%X",
                        this->reconnection_cookie, remote_reconnection_cookie);
                    LOG(LOG_INFO, "SessionProbeVirtualChannel::process_server_message: Options=0x%X",
                        this->options);
                }

                bool const delay_disabled_launch_mask      = (this->options & OPTION_DELAY_DISABLED_LAUNCH_MASK);
                bool const delay_disabled_redirected_drive = (this->options & OPTION_DELAY_DISABLED_REDIRECTED_DRIVE);

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

                    if (!this->sespro_params.launch_application_driver &&
                        delay_disabled_redirected_drive)
                    {
                        send_client_message([](OutStream & out_s) {
                            out_s.out_copy_bytes("Confirm=LaunchProcessStopped"_av);
                        });
                    }

                    this->session_probe_ready = true;
                }

                this->front.session_probe_started(true);

                if (!delay_disabled_launch_mask) {
                    this->callbacks.enable_input_event();
                    this->callbacks.enable_graphics_update();
                }

                if (!this->sespro_params.launch_application_driver &&
                    !delay_disabled_redirected_drive) {
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
                            event.alarm.reset_timeout(this->sespro_params.keepalive_timeout);
                        });
                }

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("Version=" "1" "\x01" "6"_av);
                });

                {
                    if (this->sespro_params.end_of_session_check_delay_time.count() > 0) {
                        send_client_message([this](OutStream & out_s) {
                            out_s.out_copy_bytes("EndOfSessionCheckDelayTime="_av);
                            out_s.out_copy_bytes(int_to_decimal_chars(
                                this->sespro_params.end_of_session_check_delay_time.count()));
                        });
                    }
                }

                {
                    uint32_t options = OPTION_SUPPORT_MULTIPLE_NETWORK_INTERFACES_IN_SHADOW_SESSION_RESPONSE;

                    options |= this->sespro_params.ignore_ui_less_processes_during_end_of_session_check
                        ? uint32_t(OPTION_IGNORE_UI_LESS_PROCESSES_DURING_END_OF_SESSION_CHECK)
                        : uint32_t();

                    options |= this->sespro_params.update_disabled_features
                        ? uint32_t(OPTION_UPDATE_DISABLED_FEATURES)
                        : uint32_t();

                    options |= this->sespro_params.launch_application_driver_then_terminate
                        ? uint32_t(OPTION_LAUNCH_APPLICATION_THEN_TERMINATE)
                        : uint32_t();

                    options |= this->sespro_params.enable_self_cleaner
                        ? uint32_t(OPTION_ENABLE_SELF_CLEANER)
                        : uint32_t();

                    options |= this->param_disconnect_session_instead_of_logoff_session
                        ? uint32_t(OPTION_DISCONNECT_SESSION_INSTEAD_OF_LOGOFF_SESSION)
                        : uint32_t();

                    options |= this->sespro_params.enable_remote_program
                        ? uint32_t(OPTION_REMOTE_PROGRAM_SESSION)
                        : uint32_t();

                    if (options)
                    {
                        send_client_message([options](OutStream & out_s) {
                            out_s.out_copy_bytes("Options="_av);
                            out_s.out_copy_bytes(int_to_decimal_chars(options));
                        });
                    }
                }

                if (this->sespro_params.on_account_manipulation != SessionProbeOnAccountManipulation::allow) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("AccountManipulationAction="_av);
                        out_selected_string(out_s, this->sespro_params.on_account_manipulation == SessionProbeOnAccountManipulation::notify,
                                            "notify"_av, "deny"_av);
                    });
                }

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("ChildlessWindowAsUnidentifiedInputField="_av);
                    out_yes_or_no(out_s, this->sespro_params.childless_window_as_unidentified_input_field);
                });

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("ExtraInfo="_av);
                    out_s.out_copy_bytes(int_to_decimal_chars(::getpid()));
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("AutomaticallyEndDisconnectedSession="_av);
                    out_yes_or_no(out_s, this->sespro_params.end_disconnected_session);
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
                            out_s.out_copy_bytes(int_to_decimal_chars(disconnect_session_limit));
                        });
                    }
                }

                if (this->sespro_params.idle_session_limit.count()) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("IdleSessionLimit="_av);
                        out_s.out_copy_bytes(int_to_decimal_chars(
                            ms2ll(this->sespro_params.idle_session_limit)));
                    });
                }

                {
                    this->reconnection_cookie = this->gen.rand32();
                    if (INVALID_RECONNECTION_COOKIE == this->reconnection_cookie) {
                        this->reconnection_cookie &= ~(0x80000000);
                    }

                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("ReconnectionCookie="_av);
                        out_s.out_copy_bytes(int_to_decimal_chars(this->reconnection_cookie));
                    });
                }

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("EnableCrashDump="_av);
                    out_yes_or_no(out_s, this->sespro_params.enable_crash_dump);
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("Bushido="_av);
                    out_s.out_copy_bytes(int_to_decimal_chars(
                        this->sespro_params.handle_usage_limit));
                    out_s.out_uint8('\x01');
                    out_s.out_copy_bytes(int_to_decimal_chars(
                        this->sespro_params.memory_usage_limit));
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("BestSafeIntegration="_av);
                    out_yes_or_no(out_s, this->sespro_params.bestsafe_integration);
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("CPUUsageAlarmThresholdAndAction="_av);
                    out_s.out_copy_bytes(int_to_decimal_chars(
                        safe_cast<uint32_t>(this->sespro_params.cpu_usage_alarm_action)));
                    out_s.out_uint8('\x01');
                    out_s.out_copy_bytes(int_to_decimal_chars(
                        this->sespro_params.cpu_usage_alarm_threshold));
                });

                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("DisabledFeatures=0x"_av);
                    out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(
                        safe_cast<uint32_t>(this->sespro_params.disabled_features)
                    ));
                });

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("Notify=EndOfSettings"_av);
                });
            }
            else if (upper_param0 == "DisableLaunchMask"_ascii_upper) {
                this->callbacks.enable_input_event();
                this->callbacks.enable_graphics_update();
            }
            else if (upper_param0 == "DisableRedirectedDrive"_ascii_upper) {
                this->file_system_virtual_channel.disable_session_probe_drive();
            }
            else if (upper_param0 == "Get target informations"_ascii_upper) {
                send_client_message([this](OutStream & out_s) {
                    out_s.out_copy_bytes("TargetInformations="_av);
                    out_s.out_copy_bytes(this->param_target_informations);
                });
            }

            else if (upper_param0 == "Get remote program windows and notification icons"_ascii_upper) {
                if (this->windows_and_notification_icons.size()) {
                    send_client_message([this](OutStream & out_s) {
                        out_s.out_copy_bytes("RemotePrgramNewOrExistingWindowsAndNotificationIcons="_av);
                        bool bFirst = true;
                        for (auto const& window_or_notification_icon : this->windows_and_notification_icons) {
                            if (bFirst) {
                                bFirst = false;
                            }
                            else {
                                out_s.out_uint8('\x01');
                            }
                            out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(window_or_notification_icon.window_id));
                            out_s.out_uint8('\x01');
                            out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(window_or_notification_icon.notification_icon_id));
                        }
                    });
                }

                this->windows_and_notification_icons_synchronized = true;
            }

            else if (upper_param0 == "Get startup application"_ascii_upper) {
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
            else if (upper_param0 == "Disconnection-Reconnection"_ascii_upper) {
                LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                    "SessionProbeVirtualChannel::process_server_message: "
                        "Disconnection-Reconnection required.");

                this->disconnection_reconnection_required = true;

                send_client_message([](OutStream & out_s) {
                    out_s.out_copy_bytes("Confirm=Disconnection-Reconnection"_av);
                });
            }
            else if (upper_param0 == "Get extra system process"_ascii_upper &&
                     (2 <= parameters_.size())) {
                const unsigned int proc_index = unchecked_decimal_chars_to_int(parameters_[1]);

                // ExtraSystemProcess=ProcIndex\x01ErrorCode[\x01ProcName]
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, proc_index](OutStream & out_s) {
                    out_s.out_copy_bytes("ExtraSystemProcess="_av);

                    std::string const* name =
                        this->sespro_params.extra_system_processes.get(proc_index);

                    out_s.out_copy_bytes(int_to_decimal_chars(proc_index));
                    out_s.out_uint8('\x01');
                    out_selected_string(out_s, name, "0"_av, "-1"_av);

                    if (name) {
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(*name);
                    }
                });
            }
            else if (upper_param0 == "Get outbound connection monitoring rule"_ascii_upper &&
                     (2 <= parameters_.size())) {
                const unsigned int rule_index = unchecked_decimal_chars_to_int(parameters_[1]);

                // OutboundConnectionMonitoringRule=RuleIndex\x01ErrorCode[\x01RuleType\x01HostAddrOrSubnet\x01Port]
                // RuleType  : 0 - notify, 1 - deny, 2 - allow.
                // ErrorCode : 0 on success. -1 if an error occurred.

                send_client_message([this, rule_index](OutStream & out_s) {
                    out_s.out_copy_bytes("OutboundConnectionMonitoringRule="_av);

                    auto const* rule =
                        this->sespro_params.outbound_connection_monitor_rules.get(rule_index);

                    out_s.out_copy_bytes(int_to_decimal_chars(rule_index));
                    out_s.out_uint8('\x01');

                    if (rule) {
                        out_s.out_uint8('0');
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(int_to_decimal_chars(underlying_cast(rule->type())));
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(rule->address());
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(rule->port_range());
                    }
                    else {
                        out_s.out_copy_bytes("-1"_av);
                    }
                });
            }
            else if (upper_param0 == "Get process monitoring rule"_ascii_upper &&
                     (2 <= parameters_.size())) {
                const unsigned int rule_index = unchecked_decimal_chars_to_int(parameters_[1]);

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

                    out_s.out_copy_bytes(int_to_decimal_chars(rule_index));
                    out_s.out_uint8('\x01');
                    out_selected_string(out_s, result, "0"_av, "-1"_av);

                    if (result) {
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(int_to_decimal_chars(type));
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(pattern);
                    }
                });
            }

            else if (upper_param0 == "Get windows of application as unidentified input field"_ascii_upper &&
                     (2 <= parameters_.size())) {
                const unsigned int app_index = unchecked_decimal_chars_to_int(parameters_[1]);

                // WindowsOfApplicationAsUnidentifiedInputField=AppIndex\x01ErrorCode[\x01AppName]
                // ErrorCode : 0 on success. -1 if an error occurred.


                send_client_message([this, app_index](OutStream & out_s) {
                    out_s.out_copy_bytes("WindowsOfApplicationAsUnidentifiedInputField="_av);

                    std::string const* name =
                        this->sespro_params.windows_of_these_applications_as_unidentified_input_field.get(app_index);

                    out_s.out_copy_bytes(int_to_decimal_chars(app_index));
                    out_s.out_uint8('\x01');
                    out_selected_string(out_s, name, "0"_av, "-1"_av);

                    if (name) {
                        out_s.out_uint8('\x01');
                        out_s.out_copy_bytes(*name);
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
        else if (upper_order == "ExecuteResult"_ascii_upper && (4 <= parameters_.size())) {
            this->rdp.sespro_rail_exec_result(
                    unchecked_decimal_chars_to_int(parameters_[3]),
                    parameters_[0],
                    unchecked_decimal_chars_to_int(parameters_[1]),
                    unchecked_decimal_chars_to_int(parameters_[2])
                );
        }
        else if (upper_order == "ExtraInfo"_ascii_upper && !parameters_.empty()) {
            LOG(LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "SessionProbePID=%.*s",
                int(parameters_[0].size()), parameters_[0].data());
        }
        else if (upper_order == "Version"_ascii_upper && (2 <= parameters_.size())) {
            const uint8_t major = unchecked_decimal_chars_to_int(parameters_[0]);
            const uint8_t minor = unchecked_decimal_chars_to_int(parameters_[1]);

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
                        out_yes_or_no(out_s, this->sespro_params.enable_log_rotation);

                        if (0x0104 <= this->other_version) {
                            out_s.out_uint8('\x01');
                            out_s.out_copy_bytes(int_to_decimal_chars(
                                underlying_cast(this->sespro_params.log_level)));
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
        else if (upper_order == "Log"_ascii_upper && !parameters_.empty()) {
            LOG(LOG_INFO, "SessionProbe: %.*s", int(parameters_[0].size()), parameters_[0].data());
        }
        else if (upper_order == "KeepAlive"_ascii_upper && !parameters_.empty() &&
                 insensitive_eq(parameters_[0], "OK"_ascii_upper)) {
            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe_repetitive), LOG_INFO,
                "SessionProbeVirtualChannel::process_server_message: "
                    "Received Keep-Alive from Session Probe.");
            this->session_probe_keep_alive_received = true;

            if (this->client_input_disabled_because_session_probe_keepalive_is_missing) {
                this->callbacks.enable_input_event();
                this->callbacks.enable_graphics_update();
                this->callbacks.display_osd_message("");
                this->request_keep_alive();
                this->client_input_disabled_because_session_probe_keepalive_is_missing = false;
            }
        }
        else if (upper_order == "SESSION_ENDING_IN_PROGRESS"_ascii_upper) {
            this->log6(LogId::SESSION_ENDING_IN_PROGRESS, {});

            this->session_probe_ending_in_progress = true;
        }
        else {
            bool message_format_invalid = false;

            if (!parameters_.empty()) {

                if (upper_order == "KERBEROS_TICKET_CREATION"_ascii_upper ||
                    upper_order == "KERBEROS_TICKET_DELETION"_ascii_upper) {
                    if (parameters_.size() == 7) {
                        this->log6(
                            upper_order == "KERBEROS_TICKET_CREATION"_ascii_upper
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
                else if (upper_order == "BESTSAFE_SERVICE_LOG"_ascii_upper) {
                    KVListFromStrings builder(parameters_);
                    while (auto kv_list = builder.next()) {
                        this->log6(LogId::BESTSAFE_SERVICE_LOG, kv_list);
                    }
                }
                else if (upper_order == "PASSWORD_TEXT_BOX_GET_FOCUS"_ascii_upper) {
                    this->log6(
                        LogId::PASSWORD_TEXT_BOX_GET_FOCUS, {
                        KVLog("status"_av, parameters_[0]),
                    });

                    if (parameters_.size() == 1) {
                        this->front.set_focus_on_password_textbox(
                            insensitive_eq(parameters_[0], "yes"_ascii_upper));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "UNIDENTIFIED_INPUT_FIELD_GET_FOCUS"_ascii_upper) {
                    this->log6(
                        LogId::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS, {
                        KVLog("status"_av, parameters_[0]),
                    });

                    if (parameters_.size() == 1) {
                        this->front.set_focus_on_unidentified_input_field(
                            insensitive_eq(parameters_[0], "yes"_ascii_upper));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "UAC_PROMPT_BECOME_VISIBLE"_ascii_upper) {
                    if (parameters_.size() == 1) {
                        this->log6(
                            LogId::UAC_PROMPT_BECOME_VISIBLE, {
                            KVLog("status"_av, parameters_[0]),
                        });

                        this->front.set_consent_ui_visible(insensitive_eq(parameters_[0], "yes"_ascii_upper));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "SESSION_LOCKED"_ascii_upper) {
                    if (parameters_.size() == 1) {
                        this->log6(
                            LogId::SESSION_LOCKED, {
                            KVLog("status"_av, parameters_[0]),
                        });

                        this->front.set_session_locked(insensitive_eq(parameters_[0], "yes"_ascii_upper));
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "INPUT_LANGUAGE"_ascii_upper) {
                    if (parameters_.size() == 2) {
                        this->log6(
                            LogId::INPUT_LANGUAGE, {
                            KVLog("identifier"_av,   parameters_[0]),
                            KVLog("display_name"_av, parameters_[1]),
                        });

                        KeyLayout::KbdId kbdid = unchecked_hexadecimal_chars_with_prefix_to_int(parameters_[0]);
                        auto* layout = find_layout_by_id(kbdid);
                        if (layout) {
                            this->front.set_keylayout(*layout);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "NEW_PROCESS"_ascii_upper ||
                         upper_order == "COMPLETED_PROCESS"_ascii_upper) {
                    if (parameters_.size() == 1) {
                        this->log6(
                            upper_order == "NEW_PROCESS"_ascii_upper
                                ? LogId::NEW_PROCESS
                                : LogId::COMPLETED_PROCESS, {
                            KVLog("command_line"_av, parameters_[0]),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "STARTUP_APPLICATION_FAIL_TO_RUN"_ascii_upper) {
                    if (parameters_.size() == 2) {
                        std::string transformed_app_name(parameters_[0].data(),
                                                         parameters_[0].size());

                        utils::str_replace_inplace_between_pattern(transformed_app_name,
                                                                   TAG_HIDE,
                                                                   REPLACEMENT_HIDE);

                        this->log6(LogId::STARTUP_APPLICATION_FAIL_TO_RUN, {
                            KVLog("application_name"_av, transformed_app_name),
                            KVLog("raw_result"_av,       parameters_[1]),
                        });

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: "
                            "raw_result=%.*s",
                            int(parameters_[1].size()), parameters_[1].data());

                        this->session_log.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "STARTUP_APPLICATION_FAIL_TO_RUN_2"_ascii_upper) {
                    if (parameters_.size() == 3) {
                        std::string transformed_app_name(parameters_[0].data(),
                                                         parameters_[0].size());

                        utils::str_replace_inplace_between_pattern(transformed_app_name,
                                                                   TAG_HIDE,
                                                                   REPLACEMENT_HIDE);

                        this->log6(
                            LogId::STARTUP_APPLICATION_FAIL_TO_RUN_2, {
                            KVLog("application_name"_av,   transformed_app_name),
                            KVLog("raw_result"_av,         parameters_[1]),
                            KVLog("raw_result_message"_av, parameters_[2]),
                        });

                        LOG(LOG_ERR,
                            "Session Probe failed to run startup application: "
                            "raw_result=%.*s  raw_result_message=%.*s",
                            int(parameters_[1].size()), parameters_[1].data(),
                            int(parameters_[2].size()), parameters_[2].data());

                        this->session_log.report(
                            "SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED", "");
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "OUTBOUND_CONNECTION_DETECTED"_ascii_upper) {
                    if (parameters_.size() == 2) {
                        this->log6(LogId::OUTBOUND_CONNECTION_DETECTED, {
                            KVLog("rule"_av,             parameters_[0]),
                            KVLog("application_name"_av, parameters_[1]),
                        });

                        char message[4096];

                        this->tr.fmt(message, sizeof(message),
                            trkeys::process_interrupted_security_policies,
                            int(parameters_[1].size()), parameters_[1].data());

                        this->callbacks.display_osd_message(message);
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "OUTBOUND_CONNECTION_BLOCKED_2"_ascii_upper ||
                         upper_order == "OUTBOUND_CONNECTION_DETECTED_2"_ascii_upper) {
                    bool deny = (upper_order == "OUTBOUND_CONNECTION_BLOCKED_2"_ascii_upper);

                    if ((!deny && (parameters_.size() == 5))
                     || (deny && (parameters_.size() == 6))
                    ) {
                        auto const* rule =
                            this->sespro_params.outbound_connection_monitor_rules.get(
                                unchecked_decimal_chars_to_int(parameters_[0]));

                        if (rule) {
                            this->log6(
                                deny
                                    ? LogId::OUTBOUND_CONNECTION_BLOCKED_2
                                    : LogId::OUTBOUND_CONNECTION_DETECTED_2, {
                                KVLog("rule"_av,         rule->description()),
                                KVLog("app_name"_av,     parameters_[1]),
                                KVLog("app_cmd_line"_av, parameters_[2]),
                                KVLog("dst_addr"_av,     parameters_[3]),
                                KVLog("dst_port"_av,     parameters_[4]),
                            });

                            {
                                char message[4096];

                                // rule, app_name, app_cmd_line, dst_addr, dst_port
                                snprintf(message, sizeof(message), "%.*s|%.*s|%.*s|%.*s|%.*s",
                                    int(rule->description().size()), rule->description().data(),
                                    int(parameters_[1].size()), parameters_[1].data(),
                                    int(parameters_[2].size()), parameters_[2].data(),
                                    int(parameters_[3].size()), parameters_[3].data(),
                                    int(parameters_[4].size()), parameters_[4].data());

                                this->session_log.report(
                                    (deny ? "FINDCONNECTION_DENY" : "FINDCONNECTION_NOTIFY"),
                                    message);
                            }

                            if (deny) {
                                unsigned long pid = unchecked_decimal_chars_to_int(parameters_[5]);
                                if (pid) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block outbound connection!");
                                    this->session_log.report(
                                        "SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    this->tr.fmt(message, sizeof(message),
                                        trkeys::process_interrupted_security_policies,
                                        int(parameters_[1].size()), parameters_[1].data());

                                    this->callbacks.display_osd_message(message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "PROCESS_BLOCKED"_ascii_upper ||
                         upper_order == "PROCESS_DETECTED"_ascii_upper) {
                    bool deny = (upper_order == "PROCESS_BLOCKED"_ascii_upper);

                    if ((!deny && (parameters_.size() == 3)) ||
                        (deny && (parameters_.size() == 4))) {
                        unsigned int type = 0;
                        std::string  pattern;
                        std::string  description;
                        const bool result =
                            this->sespro_params.process_monitor_rules.get(
                                unchecked_decimal_chars_to_int(parameters_[0]),
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
                                snprintf(message, sizeof(message), "%s|%.*s|%.*s",
                                    description.c_str(),
                                    int(parameters_[1].size()), parameters_[1].data(),
                                    int(parameters_[2].size()), parameters_[2].data());

                                this->session_log.report(
                                    (deny ? "FINDPROCESS_DENY" : "FINDPROCESS_NOTIFY"),
                                    message);
                            }

                            if (deny) {
                                unsigned long pid = unchecked_decimal_chars_to_int(parameters_[3]);
                                if (pid) {
                                    LOG(LOG_ERR,
                                        "Session Probe failed to block process!");
                                    this->session_log.report(
                                        "SESSION_PROBE_PROCESS_BLOCKING_FAILED", "");
                                }
                                else {
                                    char message[4096];

                                    this->tr.fmt(message, sizeof(message),
                                        trkeys::process_interrupted_security_policies,
                                        int(parameters_[1].size()), parameters_[1].data());

                                    this->callbacks.display_osd_message(message);
                                }
                            }
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "ACCOUNT_MANIPULATION_BLOCKED"_ascii_upper ||
                         upper_order == "ACCOUNT_MANIPULATION_DETECTED"_ascii_upper) {
                    bool deny = (upper_order == "ACCOUNT_MANIPULATION_BLOCKED"_ascii_upper);

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
                            snprintf(message, sizeof(message), "%.*s|%.*s|%.*s|%.*s|%.*s|%.*s",
                                int(parameters_[0].size()), parameters_[0].data(),
                                int(parameters_[1].size()), parameters_[1].data(),
                                int(parameters_[2].size()), parameters_[2].data(),
                                int(parameters_[3].size()), parameters_[3].data(),
                                int(parameters_[4].size()), parameters_[4].data(),
                                int(parameters_[5].size()), parameters_[5].data());

                            this->session_log.report(
                                (deny ? "ACCOUNTMANIPULATION_DENY" : "ACCOUNTMANIPULATION_NOTIFY"),
                                message);
                        }

                        if (deny) {
                            char message[4096];

                            int slen = this->tr.fmt(message, sizeof(message),
                                trkeys::account_manipulation_blocked_security_policies,
                                int(parameters_[3].size()), parameters_[3].data());
                            std::size_t len = (slen == -1) ? sizeof(message) : std::size_t(slen);

                            this->callbacks.display_osd_message(std::string_view(message, len));
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }
                else if (upper_order == "FOREGROUND_WINDOW_CHANGED"_ascii_upper) {
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
                else if (upper_order == "CHECKBOX_CLICKED"_ascii_upper) {
                    if (parameters_.size() == 3) {
                        this->log6(LogId::CHECKBOX_CLICKED, {
                            KVLog("window"_av, parameters_[0]),
                            KVLog("checkbox"_av, parameters_[1]),
                            KVLog("state"_av,
                                ::button_state_to_string(unchecked_decimal_chars_to_int(parameters_[2]))),
                        });
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (upper_order == "SHADOW_SESSION_SUPPORTED"_ascii_upper) {
                    if (parameters_.size() == 1) {
                        if (this->sespro_params.session_shadowing_support
                         && insensitive_eq(parameters_[0], "yes"_ascii_upper)
                        ) {
                            this->vars.set_acl<cfg::context::rd_shadow_available>(true);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (upper_order == "SHADOW_SESSION_RESPONSE"_ascii_upper ||
                         upper_order == "SHADOW_SESSION_RESPONSE_2"_ascii_upper) {
                    if (parameters_.size() >= 3)
                    {
                        const uint32_t shadow_errcode  = unchecked_hexadecimal_chars_with_prefix_to_int(parameters_[0]);
                        const auto&    shadow_errmsg   = parameters_[1];
                        const auto&    shadow_userdata = parameters_[2];
                        if (parameters_.size() >= 6) {
                            auto is_ipv4 = [](std::string_view ip) {
                                return std::find(ip.begin(), ip.end(), ':') == ip.end();
                            };

                            auto const shadow_id = str_concat(int_to_decimal_chars(time(nullptr)), "*", parameters_[3]);

                            std::size_t const max_arity                 = 16;
                            std::size_t       item_count                = 0;
                            int               best_adress_port_index    = -1;
                            int               default_adress_port_index = -1;
                            std::string_view  target_ip                 = this->sespro_params.target_ip;
                            bool              target_ip_is_ipv4         = is_ipv4(target_ip);

                            LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                                "SessionProbeVirtualChannel::process_server_message: target_ip=%s",
                                this->sespro_params.target_ip);

                            std::array<std::string_view, max_arity> shadow_addresses;
                            std::array<uint16_t, max_arity>         shadow_ports;

                            auto shadow_addresses_reader = split_with(parameters_[4], '|');
                            auto shadow_ports_reader     = split_with(parameters_[5], '|');

                            for (auto addr_iter = shadow_addresses_reader.begin(),
                                      addr_end  = shadow_addresses_reader.end(),
                                      port_iter = shadow_ports_reader.begin(),
                                      port_end  = shadow_ports_reader.end();
                                 addr_iter != addr_end && port_iter != port_end;
                                 ++addr_iter, ++port_iter
                            ) {
                                if (item_count == max_arity) {
                                    LOG(LOG_WARNING, "SessionProbeVirtualChannel::process_server_message: "
                                        "Too many network adressess/ports in Shadow Session Response!");
                                    break;
                                }

                                shadow_addresses[item_count] = addr_iter->as<std::string_view>();
                                shadow_ports[item_count]     = unchecked_decimal_chars_to_int(*port_iter);
                                LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                                    "SessionProbeVirtualChannel::process_server_message: address=%.*s port=%u",
                                    static_cast<int>(shadow_addresses[item_count].size()), shadow_addresses[item_count].data(),
                                    shadow_ports[item_count]);

                                if (shadow_addresses[item_count] == target_ip) {
                                    best_adress_port_index = item_count;
                                }

                                if (default_adress_port_index == -1 && is_ipv4(shadow_addresses[item_count]) == target_ip_is_ipv4) {
                                    default_adress_port_index = item_count;
                                }

                                ++item_count;
                            }

                            if (item_count)
                            {
                                if (default_adress_port_index == -1) {
                                    default_adress_port_index = 0;
                                }

                                std::string_view shadow_addr = shadow_addresses[default_adress_port_index];
                                uint16_t         shadow_port = shadow_ports[default_adress_port_index];

                                if (best_adress_port_index > -1) {
                                    shadow_addr = shadow_addresses[best_adress_port_index];
                                    shadow_port = shadow_ports[best_adress_port_index];

                                    LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                                        "SessionProbeVirtualChannel::process_server_message: "
                                            "Use best Shadow address/port: (%.*s):%u",
                                        static_cast<int>(shadow_addr.size()), shadow_addr.data(), shadow_port);
                                }
                                else {
                                    LOG_IF(bool(this->verbose & RDPVerbose::sesprobe), LOG_INFO,
                                        "SessionProbeVirtualChannel::process_server_message: "
                                            "Use default Shadow address/port: (%.*s):%u",
                                        static_cast<int>(shadow_addr.size()), shadow_addr.data(), shadow_port);
                                }

                                this->set_rd_shadow_invitation(shadow_errcode, shadow_errmsg, shadow_userdata, shadow_id, shadow_addr, shadow_port);
                            }
                            else {
                                LOG(LOG_WARNING, "SessionProbeVirtualChannel::process_server_message: "
                                    "No usable address/port found!");

                                this->set_rd_shadow_invitation(0xFFFFFFFF, "No usable address/port found!", shadow_userdata, "", "", 0);
                            }
                        }
                        else {
                            this->set_rd_shadow_invitation(shadow_errcode, shadow_errmsg, shadow_userdata, "", "", 0);
                        }
                    }
                    else {
                        message_format_invalid = true;
                    }
                }

                else if (execute_log6_if(upper_order, parameters_,
                    [this](LogId logid, KVLogList kvlist) { this->log6(logid, kvlist); },
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(OUTBOUND_CONNECTION_BLOCKED),
                        "rule"_av,
                        "application_name"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(BUTTON_CLICKED),
                        "window"_av,
                        "button"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED),
                        "window"_av,
                        "edit"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED_2),
                        "window"_av,
                        "edit"_av,
                        "value"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_ATTEMPT_TO_PRINT),
                        "url"_av,
                        "title"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_BEFORE_NAVIGATE),
                        "url"_av,
                        "post"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_DOCUMENT_COMPLETE),
                        "url"_av,
                        "title"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_NAVIGATE_ERROR),
                        "url"_av,
                        "title"_av,
                        "code"_av,
                        "display_name"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_NAVIGATION),
                        "url"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_PRIVACY_IMPACTED),
                        "impacted"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_ENCRYPTION_LEVEL_CHANGED),
                        "identifier"_av,
                        "display_name"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(WEB_THIRD_PARTY_URL_BLOCKED),
                        "url"_av),
                    executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(GROUP_MEMBERSHIP),
                        "groups"_av)
                )) {
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

    void rail_exec(chars_view application_name, chars_view command_line,
        chars_view current_directory, bool show_maximized, uint16_t flags)
    {
        StaticOutStream<8192> out_s;

        const size_t message_length_offset = out_s.get_offset();
        out_s.out_skip_bytes(sizeof(uint16_t));
        out_s.out_copy_bytes("Execute="_av);
        out_s.out_copy_bytes(application_name);

        out_s.out_uint8('\x01');
        out_s.out_copy_bytes(command_line);

        out_s.out_uint8('\x01');
        out_s.out_copy_bytes(current_directory);

        out_s.out_uint8('\x01');
        out_selected_string(out_s, show_maximized, "Minimized"_av, "Normal"_av);

        out_s.out_uint8('\x01');
        out_s.out_copy_bytes(int_to_decimal_chars(flags));

        out_s.out_clear_bytes(1);   // Null-terminator.

        out_s.stream_at(message_length_offset).out_uint16_le(
            out_s.get_offset() - message_length_offset - sizeof(uint16_t));

        this->send_message_to_server(out_s.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_produced_bytes());
    }

    void create_shadow_session(std::string_view userdata, std::string_view type)
    {
        LOG(LOG_INFO, "sespro_channel::create_shadow_session()");

        bool bTakeControl       = true;
        bool bRequestPermission = true;

        auto upper_type = ascii_to_limited_upper<64>(type);

        if (upper_type == "PermissionControl"_ascii_upper) {
            bTakeControl       = true;
            bRequestPermission = true;
        }
        else if (upper_type == "PermissionView"_ascii_upper) {
            bTakeControl       = false;
            bRequestPermission = true;
        }
        else if (upper_type == "SilentControl"_ascii_upper) {
            bTakeControl       = true;
            bRequestPermission = false;
        }
        else if (upper_type == "SilentView"_ascii_upper) {
            bTakeControl       = false;
            bRequestPermission = false;
        }
        else {
            LOG(LOG_WARNING,
                "SessionProbeVirtualChannel::create_shadow_session: "
                    "Invalid shadow session type! Operation canceled. Type=%.*s",
                int(type.size()), type.data());

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

            out_yes_or_no(out_s, bTakeControl);

            out_s.out_uint8('\x01');

            out_yes_or_no(out_s, bRequestPermission);

            out_s.out_uint8('\x01');

            out_s.out_copy_bytes(userdata);
        });
    }

    struct window_or_notification_icon {
        uint32_t window_id;

        uint32_t notification_icon_id;

        bool operator<(window_or_notification_icon const& other) const {
            return (this->window_id < other.window_id)
                || (   (this->window_id == other.window_id)
                    && (this->notification_icon_id < other.notification_icon_id))
                ;
        }
    };

    std::set<window_or_notification_icon> windows_and_notification_icons;

    bool windows_and_notification_icons_synchronized = false;

    void rail_add_window_or_notification_icon(uint32_t window_id, uint32_t notification_icon_id) override
    {
        LOG(LOG_INFO, "SessionProbeVirtualChannel::rail_add_window_or_notification_icon(): WindowId=0x%X NotificationIconId=0x%X", window_id, notification_icon_id);

        auto const insert_result = this->windows_and_notification_icons.insert({
                .window_id            = window_id,
                .notification_icon_id = notification_icon_id
            });

        if (this->windows_and_notification_icons_synchronized &&
            insert_result.second) {
            send_client_message([window_id, notification_icon_id](OutStream & out_s) {
                out_s.out_copy_bytes("RemotePrgramNewOrExistingWindowsAndNotificationIcons="_av);
                out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(window_id));
                out_s.out_uint8('\x01');
                out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(notification_icon_id));
            });
        }

        LOG(LOG_INFO, "SessionProbeVirtualChannel::rail_add_window_or_notification_icon(): ItemCount=%zu", this->windows_and_notification_icons.size());
    }

    void rail_remove_window_or_notification_icon(uint32_t window_id, uint32_t notification_icon_id) override
    {
        LOG(LOG_INFO, "SessionProbeVirtualChannel::rail_remove_window_or_notification_icon(): WindowId=0x%X NotificationIconId=0x%X", window_id, notification_icon_id);

        this->windows_and_notification_icons.erase({
                .window_id            = window_id,
                .notification_icon_id = notification_icon_id
            });

        if (this->windows_and_notification_icons_synchronized) {
            send_client_message([window_id, notification_icon_id](OutStream & out_s) {
                out_s.out_copy_bytes("RemotePrgramDeletedWindowOrNotificationIcon="_av);
                out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(window_id));
                out_s.out_uint8('\x01');
                out_s.out_copy_bytes(int_to_fixed_hexadecimal_upper_chars(notification_icon_id));
            });
        }

        LOG(LOG_INFO, "SessionProbeVirtualChannel::rail_remove_window_or_notification_icon(): ItemCount=%zu", this->windows_and_notification_icons.size());
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

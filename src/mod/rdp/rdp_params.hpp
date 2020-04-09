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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "core/RDP/caches/bmpcache.hpp"
#include "core/channel_names.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/rdp/channels/validator_params.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "core/RDP/windows_execute_shell_params.hpp"
#include "core/RDP/capabilities/order.hpp"

#ifndef __EMSCRIPTEN__
# include "mod/rdp/params/rdp_session_probe_params.hpp"
#endif

# include "mod/rdp/params/rdp_application_params.hpp"

#include <chrono>
#include <string>
#include <cstdint>


class ClientExecute;
class Transport;
class Theme;
class Font;
class TLSClientParams;

struct ModRDPParams
{
#ifndef __EMSCRIPTEN__
    ModRdpSessionProbeParams session_probe_params;
#endif

    const char * target_user;
    const char * target_password;
    const char * target_host;
    char client_address[16];

    bool enable_tls = true;
    bool enable_nla = true;
    bool enable_krb = false;
    bool enable_fastpath = true;           // If true, fast-path must be supported.
    bool enable_new_pointer = true;
    bool enable_glyph_cache = false;
    bool enable_remotefx = false;

    bool enable_restricted_admin_mode = false;

    ValidatorParams validator_params;

    struct ClipboardParams
    {
        bool disable_log_syslog = false;
        bool log_only_relevant_activities = true;
    };

    ClipboardParams clipboard_params;

    struct FileSystemParams
    {
        bool disable_log_syslog = false;
        bool bogus_ios_rdpdr_virtual_channel = true;
        bool enable_rdpdr_data_analysis = true;
        bool smartcard_passthrough = false;
    };

    FileSystemParams file_system_params;

    Transport  * persistent_key_list_transport = nullptr;

    int key_flags;

    bool         ignore_auth_channel = false;
    CHANNELS::ChannelNameId auth_channel;

    CHANNELS::ChannelNameId checkout_channel;

    ApplicationParams application_params;

    RdpCompression rdp_compression = RdpCompression::none;

    std::string *        error_message = nullptr;
    std::chrono::seconds open_session_timeout {};
    bool                 disconnect_on_logon_user_change = false;

    bool               server_cert_store = true;
    ServerCertCheck    server_cert_check = ServerCertCheck::fails_if_no_match_and_succeed_if_no_know;
    ServerNotification server_access_allowed_message = ServerNotification::syslog;
    ServerNotification server_cert_create_message = ServerNotification::syslog;
    ServerNotification server_cert_success_message = ServerNotification::syslog;
    ServerNotification server_cert_failure_message = ServerNotification::syslog;
    ServerNotification server_cert_error_message = ServerNotification::syslog;

    bool enable_server_cert_external_validation = false;

    bool hide_client_name = false;

    const char * device_id = "";

    PrimaryDrawingOrdersSupport disabled_orders {};

    bool enable_persistent_disk_bitmap_cache = false;
    bool enable_cache_waiting_list = false;
    bool persist_bitmap_cache_on_disk = false;

    uint32_t password_printing_mode = 0;

    bool bogus_sc_net_size = true;
    bool bogus_refresh_rect = true;

    struct DriveParams
    {
        const char * proxy_managed_drives = "";
        const char * proxy_managed_prefix = "";
    };

    DriveParams drive_params;

    Translation::language_t lang = Translation::EN;

    bool allow_using_multiple_monitors = false;

    bool adjust_performance_flags_for_recording = false;

    struct RemoteAppParams
    {
        bool enable_remote_program   = false;
        bool remote_program_enhanced = false;

        bool convert_remoteapp_to_desktop = false;

        bool use_client_provided_remoteapp = false;

        bool should_ignore_first_client_execute = false;

        WindowsExecuteShellParams client_execute;

        ClientExecute * rail_client_execute = nullptr;
        std::chrono::milliseconds rail_disconnect_message_delay {};

        std::chrono::milliseconds bypass_legal_notice_delay {};
        std::chrono::milliseconds bypass_legal_notice_timeout {};
    };

    RemoteAppParams remote_app_params;

    Font const & font;
    Theme const & theme;

    bool clean_up_32_bpp_cursor = false;

    bool large_pointer_support = true;

    bool perform_automatic_reconnection = false;
    std::array<uint8_t, 28>& server_auto_reconnect_packet_ref;

    std::string& close_box_extra_message_ref;

    const char * load_balance_info = "";

    bool experimental_fix_input_event_sync = true;

    bool support_connection_redirection_during_recording = true;

    bool split_domain = false;

    bool use_license_store = true;

    bool accept_monitor_layout_change_if_capture_is_not_started = true;

    RDPVerbose verbose;
    BmpCache::Verbose cache_verbose = BmpCache::Verbose::none;

    ModRDPParams( const char * target_user
                , const char * target_password
                , const char * target_host
                , const char * client_address
                , int key_flags
                , Font const & font
                , Theme const & theme
                , std::array<uint8_t, 28>& server_auto_reconnect_packet_ref
                , std::string& close_box_extra_message_ref
                , RDPVerbose verbose
                )
        : target_user(target_user)
        , target_password(target_password)
        , target_host(target_host)
	, client_address { }
        , key_flags(key_flags)
        , font(font)
        , theme(theme)
        , server_auto_reconnect_packet_ref(server_auto_reconnect_packet_ref)
        , close_box_extra_message_ref(close_box_extra_message_ref)
        , verbose(verbose)
    {
      std::strcpy(this->client_address, client_address);
    }

    void log() const
    {
        auto yes_or_no = [](bool x) -> char const * { return x ? "yes" : "no"; };
        auto hidden_or_null = [](bool x) -> char const * { return x ? "<hidden>" : "<null>"; };
        auto s_or_null = [](char const * s) -> char const * { return s ? s : "<null>"; };
        auto s_or_none = [](char const * s) -> char const * { return s ? s : "<none>"; };
        auto from_sec = [](std::chrono::seconds sec) -> unsigned { return sec.count(); };
        auto from_millisec = [](std::chrono::milliseconds millisec) -> unsigned { return millisec.count(); };

#define RDP_PARAMS_LOG(format, get, member) \
    LOG(LOG_INFO, "ModRDPParams " #member "=" format, get (this->member))
#define RDP_PARAMS_LOG_GET
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    target_user);
        RDP_PARAMS_LOG("\"%s\"", hidden_or_null,        target_password);
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    target_host);
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    client_address);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.primary_user_id);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.target_application);

        RDP_PARAMS_LOG("%" PRIx32, RDP_PARAMS_LOG_GET,  disabled_orders.as_uint());
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_tls);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_nla);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_krb);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_fastpath);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_new_pointer);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_glyph_cache);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_remotefx);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_restricted_admin_mode);

#ifndef __EMSCRIPTEN__
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.enable_session_probe);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.enable_launch_mask);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.used_clipboard_based_launcher);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.start_launch_timeout_timer_only_after_logon);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.effective_launch_timeout);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      session_probe_params.vc_params.on_launch_failure);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.keepalive_timeout);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      session_probe_params.vc_params.on_keepalive_timeout);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.end_disconnected_session);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.disconnected_application_limit);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.disconnected_session_limit);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.idle_session_limit);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.customize_executable_name);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.enable_log);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.enable_log_rotation);

        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.clipboard_based_launcher.clipboard_initialization_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.clipboard_based_launcher.start_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.clipboard_based_launcher.long_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.clipboard_based_launcher.short_delay_ms);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.allow_multiple_handshake);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.enable_crash_dump);

        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, session_probe_params.vc_params.handle_usage_limit);
        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, session_probe_params.vc_params.memory_usage_limit);

        RDP_PARAMS_LOG("0x%08X", static_cast<unsigned>, session_probe_params.vc_params.disabled_features);

        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.end_of_session_check_delay_time);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.ignore_ui_less_processes_during_end_of_session_check);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.childless_window_as_unidentified_input_field);

        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_params.vc_params.launcher_abort_delay);

        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    session_probe_params.vc_params.extra_system_processes.to_string());

        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    session_probe_params.vc_params.outbound_connection_monitor_rules.to_string());

        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    session_probe_params.vc_params.process_monitor_rules.to_string());

        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    session_probe_params.vc_params.windows_of_these_applications_as_unidentified_input_field.to_string());

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.is_public_session);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.used_to_launch_remote_program);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.fix_too_long_cookie);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_params.vc_params.session_shadowing_support);

        RDP_PARAMS_LOG("%d",     static_cast<int>,      session_probe_params.vc_params.on_account_manipulation);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             session_probe_params.alternate_directory_environment_variable.c_str());
#endif

        RDP_PARAMS_LOG("%s",     yes_or_no,             clipboard_params.disable_log_syslog);

        RDP_PARAMS_LOG("%s",     yes_or_no,             file_system_params.disable_log_syslog);

        RDP_PARAMS_LOG("<%p>",   static_cast<void*>,    persistent_key_list_transport);

        RDP_PARAMS_LOG("%d",     RDP_PARAMS_LOG_GET,    key_flags);

        RDP_PARAMS_LOG("%s",     yes_or_no,             ignore_auth_channel);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             auth_channel.c_str());

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             checkout_channel.c_str());

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.alternate_shell);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.shell_arguments);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.shell_working_dir);
        RDP_PARAMS_LOG("%s",     yes_or_no,             application_params.use_client_provided_alternate_shell);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             application_params.target_application_account);
        RDP_PARAMS_LOG("\"%s\"", hidden_or_null,        application_params.target_application_password);

        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, rdp_compression);

        RDP_PARAMS_LOG("<%p>",   static_cast<void*>,    error_message);
        RDP_PARAMS_LOG("%s",     yes_or_no,             disconnect_on_logon_user_change);
        RDP_PARAMS_LOG("%u",     from_sec,              open_session_timeout);

        RDP_PARAMS_LOG("%s",     yes_or_no,             server_cert_store);
        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, server_cert_check);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      server_access_allowed_message);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      server_cert_create_message);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      server_cert_success_message);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      server_cert_failure_message);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      server_cert_error_message);

        RDP_PARAMS_LOG("%s",     yes_or_no,             hide_client_name);

        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_persistent_disk_bitmap_cache);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_cache_waiting_list);
        RDP_PARAMS_LOG("%s",     yes_or_no,             persist_bitmap_cache_on_disk);

        RDP_PARAMS_LOG("%u",     RDP_PARAMS_LOG_GET,    password_printing_mode);

        RDP_PARAMS_LOG("%s",     yes_or_no,             bogus_sc_net_size);

        RDP_PARAMS_LOG("%s",     yes_or_no,             bogus_refresh_rect);

        RDP_PARAMS_LOG("%s",     s_or_none,             drive_params.proxy_managed_drives);
        RDP_PARAMS_LOG("%s",     s_or_none,             drive_params.proxy_managed_prefix);

        auto to_lang = [](Translation::language_t lang) {
            return
                lang == Translation::EN ? "EN" :
                lang == Translation::FR ? "FR" :
                "<unknown>";
        };
        RDP_PARAMS_LOG("%s",     to_lang,               lang);

        RDP_PARAMS_LOG("%s",     yes_or_no,             allow_using_multiple_monitors);

        RDP_PARAMS_LOG("%s",     yes_or_no,             adjust_performance_flags_for_recording);

        RDP_PARAMS_LOG("<%p>",   static_cast<void*>,    remote_app_params.rail_client_execute);

        RDP_PARAMS_LOG("0x%04X", RDP_PARAMS_LOG_GET,    remote_app_params.client_execute.flags);

        RDP_PARAMS_LOG("%s",     s_or_none,             remote_app_params.client_execute.exe_or_file.c_str());
        RDP_PARAMS_LOG("%s",     s_or_none,             remote_app_params.client_execute.working_dir.c_str());
        RDP_PARAMS_LOG("%s",     s_or_none,             remote_app_params.client_execute.arguments.c_str());

        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_app_params.use_client_provided_remoteapp);

        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_app_params.should_ignore_first_client_execute);

        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_app_params.enable_remote_program);
        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_app_params.remote_program_enhanced);

        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_app_params.convert_remoteapp_to_desktop);

        RDP_PARAMS_LOG("%s",     yes_or_no,             clean_up_32_bpp_cursor);

        RDP_PARAMS_LOG("%s",     yes_or_no,             large_pointer_support);

        RDP_PARAMS_LOG("%s",     s_or_none,             load_balance_info);

        RDP_PARAMS_LOG("%u",     from_millisec,         remote_app_params.rail_disconnect_message_delay);

        RDP_PARAMS_LOG("%s",     yes_or_no,             file_system_params.bogus_ios_rdpdr_virtual_channel);

        RDP_PARAMS_LOG("%s",     yes_or_no,             file_system_params.enable_rdpdr_data_analysis);

        RDP_PARAMS_LOG("%s",     yes_or_no,             file_system_params.smartcard_passthrough);

        RDP_PARAMS_LOG("%u",     from_millisec,         remote_app_params.bypass_legal_notice_delay);
        RDP_PARAMS_LOG("%u",     from_millisec,         remote_app_params.bypass_legal_notice_timeout);

        RDP_PARAMS_LOG("%s",     yes_or_no,             experimental_fix_input_event_sync);

        RDP_PARAMS_LOG("%s",     yes_or_no,             support_connection_redirection_during_recording);

        RDP_PARAMS_LOG("%s",     yes_or_no,             clipboard_params.log_only_relevant_activities);

        RDP_PARAMS_LOG("%s",     yes_or_no,             use_license_store);

        RDP_PARAMS_LOG("%s",     yes_or_no,             accept_monitor_layout_change_if_capture_is_not_started);

        RDP_PARAMS_LOG("0x%08X", static_cast<unsigned>, verbose);
        RDP_PARAMS_LOG("0x%08X", static_cast<unsigned>, cache_verbose);

#undef RDP_PARAMS_LOG
#undef RDP_PARAMS_LOG_GET
    }   // void log() const
};  // struct ModRDPParams

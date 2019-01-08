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

#include "configs/config_access.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/channel_names.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "mod/rdp/channels/sespro_clipboard_based_launcher.hpp"

#include <chrono>
#include <string>

class ClientExecute;
class Transport;
class Theme;
class Font;

using ModRdpVariables = vcfg::variables<
    vcfg::var<cfg::context::auth_notify,                       vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_flags,       vcfg::accessmode::set>,
    vcfg::var<cfg::context::auth_notify_rail_exec_exe_or_file, vcfg::accessmode::set>,
    vcfg::var<cfg::context::target_service,                    vcfg::accessmode::get>,
    vcfg::var<cfg::context::session_id,                        vcfg::accessmode::get>,
    vcfg::var<cfg::globals::auth_user,                         vcfg::accessmode::get>,
    vcfg::var<cfg::globals::host,                              vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_device,                     vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_dir_path,                  vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_file_turnover_interval,    vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::sign_key,                      vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::enable_rdp_metrics,            vcfg::accessmode::get>,
    vcfg::var<cfg::metrics::log_interval,                  vcfg::accessmode::get>
>;

struct ModRDPParams
{
    const char * target_user;
    const char * target_password;
    const char * target_host;
    const char * client_address;

    const char * primary_user_id = "";
    const char * target_application = "";

    bool enable_tls = true;
    bool enable_nla = true;
    bool enable_krb = false;
    bool enable_fastpath = true;           // If true, fast-path must be supported.
    bool enable_mem3blt = true;
    bool enable_new_pointer = true;
    bool enable_glyph_cache = false;
    bool enable_session_probe = false;
    bool session_probe_enable_launch_mask = true;
    bool enable_ninegrid_bitmap = false;

    bool disable_clipboard_log_syslog = false;
    bool disable_clipboard_log_wrm = false;
    bool disable_file_system_log_syslog = false;
    bool disable_file_system_log_wrm = false;

    bool                         session_probe_use_clipboard_based_launcher = false;
    std::chrono::milliseconds    session_probe_launch_timeout {};
    std::chrono::milliseconds    session_probe_launch_fallback_timeout {};
    bool                         session_probe_start_launch_timeout_timer_only_after_logon = true;
    SessionProbeOnLaunchFailure  session_probe_on_launch_failure = SessionProbeOnLaunchFailure::disconnect_user;
    std::chrono::milliseconds    session_probe_keepalive_timeout {};
    SessionProbeOnKeepaliveTimeout
                                 session_probe_on_keepalive_timeout = SessionProbeOnKeepaliveTimeout::disconnect_user;
    bool                         session_probe_end_disconnected_session = false;
    bool                         session_probe_customize_executable_name = false;
    std::chrono::milliseconds    session_probe_disconnected_application_limit {};
    std::chrono::milliseconds    session_probe_disconnected_session_limit {};
    std::chrono::milliseconds    session_probe_idle_session_limit {};
    const char *                 session_probe_exe_or_file = "";
    const char *                 session_probe_arguments = "";
    bool                         session_probe_enable_log = false;
    bool                         session_probe_enable_log_rotation = true;

    struct SessionProbeClipboardBasedLauncher::Params session_probe_clipboard_based_launcher;

    bool                         session_probe_allow_multiple_handshake = false;

    bool                         session_probe_enable_crash_dump = false;

    uint32_t                     session_probe_handle_usage_limit = 0;
    uint32_t                     session_probe_memory_usage_limit = 0;

    bool                         session_probe_ignore_ui_less_processes_during_end_of_session_check = true;

    bool                         session_probe_childless_window_as_unidentified_input_field = true;

    bool                         session_probe_public_session = false;

    Transport  * persistent_key_list_transport = nullptr;

    int key_flags;

    const char * session_probe_extra_system_processes               = "";
    const char * session_probe_outbound_connection_monitoring_rules = "";
    const char * session_probe_process_monitoring_rules             = "";

    const char * session_probe_windows_of_these_applications_as_unidentified_input_field = "";

    bool         ignore_auth_channel = false;
    CHANNELS::ChannelNameId auth_channel;

    CHANNELS::ChannelNameId checkout_channel;

    // Application Bastion
    const char * alternate_shell = "";
    const char * shell_arguments = "";
    const char * shell_working_dir = "";

    bool         use_client_provided_alternate_shell = false;

    const char * target_application_account = "";
    const char * target_application_password = "";

    RdpCompression rdp_compression = RdpCompression::none;

    std::string *        error_message = nullptr;
    bool                 disconnect_on_logon_user_change = false;
    std::chrono::seconds open_session_timeout {};

    bool               server_cert_store = true;
    ServerCertCheck    server_cert_check = ServerCertCheck::fails_if_no_match_and_succeed_if_no_know;
    ServerNotification server_access_allowed_message = ServerNotification::syslog;
    ServerNotification server_cert_create_message = ServerNotification::syslog;
    ServerNotification server_cert_success_message = ServerNotification::syslog;
    ServerNotification server_cert_failure_message = ServerNotification::syslog;
    ServerNotification server_cert_error_message = ServerNotification::syslog;

    bool hide_client_name = false;

    const char * device_id = "";

    const char * extra_orders = "";

    bool enable_persistent_disk_bitmap_cache = false;
    bool enable_cache_waiting_list = false;
    bool persist_bitmap_cache_on_disk = false;

    uint32_t password_printing_mode = 0;

    const std::string * allow_channels = nullptr;
    const std::string * deny_channels = nullptr;

    bool bogus_sc_net_size = true;
    bool bogus_refresh_rect = true;

    const char * proxy_managed_drives = "";
    const char * proxy_managed_drive_prefix = "";

    Translation::language_t lang = Translation::EN;

    bool allow_using_multiple_monitors = false;

    bool adjust_performance_flags_for_recording = false;

    ClientExecute * client_execute = nullptr;

    uint16_t     client_execute_flags = 0;
    const char * client_execute_exe_or_file = "";
    const char * client_execute_working_dir = "";
    const char * client_execute_arguments = "";

    bool use_client_provided_remoteapp = false;
    bool should_ignore_first_client_execute = false;

    bool remote_program          = false;
    bool remote_program_enhanced = false;

    Font const & font;
    Theme const & theme;

    bool clean_up_32_bpp_cursor = false;

    bool large_pointer_support = true;

    bool perform_automatic_reconnection = false;
    std::array<uint8_t, 28>& server_auto_reconnect_packet_ref;

    std::string& close_box_extra_message_ref;

    const char * load_balance_info = "";

    std::chrono::milliseconds rail_disconnect_message_delay {};

    bool use_session_probe_to_launch_remote_program = true;

    bool bogus_ios_rdpdr_virtual_channel = true;

    bool enable_rdpdr_data_analysis = true;

    std::chrono::milliseconds remoteapp_bypass_legal_notice_delay {};
    std::chrono::milliseconds remoteapp_bypass_legal_notice_timeout {};

    bool experimental_fix_input_event_sync = true;
    bool experimental_fix_too_long_cookie  = true;

    bool log_only_relevant_clipboard_activities = true;

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
        , client_address(client_address)
        , key_flags(key_flags)
        , font(font)
        , theme(theme)
        , server_auto_reconnect_packet_ref(server_auto_reconnect_packet_ref)
        , close_box_extra_message_ref(close_box_extra_message_ref)
        , verbose(verbose)
    {}

    void log() const
    {
        auto yes_or_no = [](bool x) -> char const * { return x ? "yes" : "no"; };
        auto hidden_or_null = [](bool x) -> char const * { return x ? "<hidden>" : "<null>"; };
        auto s_or_null = [](char const * s) -> char const * { return s ? s : "<null>"; };
        auto s_or_none = [](char const * s) -> char const * { return s ? s : "<none>"; };
        auto str_or_none = [](std::string const * str) -> char const * { return str ? str->c_str() : "<none>"; };
        auto from_sec = [](std::chrono::seconds sec) -> unsigned { return sec.count(); };
        auto from_millisec = [](std::chrono::milliseconds millisec) -> unsigned { return millisec.count(); };

#define RDP_PARAMS_LOG(format, get, member) \
    LOG(LOG_INFO, "ModRDPParams " #member "=" format, get (this->member))
#define RDP_PARAMS_LOG_GET
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    target_user);
        RDP_PARAMS_LOG("\"%s\"", hidden_or_null,        target_password);
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    target_host);
        RDP_PARAMS_LOG("\"%s\"", RDP_PARAMS_LOG_GET,    client_address);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             primary_user_id);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             target_application);

        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_tls);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_nla);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_krb);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_fastpath);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_mem3blt);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_new_pointer);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_glyph_cache);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_session_probe);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_enable_launch_mask);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_use_clipboard_based_launcher);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_launch_timeout);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_launch_fallback_timeout);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_start_launch_timeout_timer_only_after_logon);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      session_probe_on_launch_failure);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_keepalive_timeout);
        RDP_PARAMS_LOG("%d",     static_cast<int>,      session_probe_on_keepalive_timeout);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_end_disconnected_session);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_disconnected_application_limit);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_disconnected_session_limit);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_idle_session_limit);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_customize_executable_name);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_enable_log);
        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_enable_log_rotation);

        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_clipboard_based_launcher.clipboard_initialization_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_clipboard_based_launcher.start_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_clipboard_based_launcher.long_delay_ms);
        RDP_PARAMS_LOG("%u",     from_millisec,         session_probe_clipboard_based_launcher.short_delay_ms);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_allow_multiple_handshake);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_enable_crash_dump);

        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, session_probe_handle_usage_limit);
        RDP_PARAMS_LOG("%u",     static_cast<unsigned>, session_probe_memory_usage_limit);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_ignore_ui_less_processes_during_end_of_session_check);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_childless_window_as_unidentified_input_field);

        RDP_PARAMS_LOG("%s",     yes_or_no,             session_probe_public_session);

        RDP_PARAMS_LOG("%s",     yes_or_no,             disable_clipboard_log_syslog);
        RDP_PARAMS_LOG("%s",     yes_or_no,             disable_clipboard_log_wrm);

        RDP_PARAMS_LOG("%s",     yes_or_no,             disable_file_system_log_syslog);
        RDP_PARAMS_LOG("%s",     yes_or_no,             disable_file_system_log_wrm);

        RDP_PARAMS_LOG("<%p>",   static_cast<void*>,    persistent_key_list_transport);

        RDP_PARAMS_LOG("%d",     RDP_PARAMS_LOG_GET,    key_flags);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             session_probe_extra_system_processes);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             session_probe_outbound_connection_monitoring_rules);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             session_probe_process_monitoring_rules);

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             session_probe_windows_of_these_applications_as_unidentified_input_field);

        RDP_PARAMS_LOG("%s",     yes_or_no,             ignore_auth_channel);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             auth_channel.c_str());

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             checkout_channel.c_str());

        RDP_PARAMS_LOG("\"%s\"", s_or_null,             alternate_shell);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             shell_arguments);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             shell_working_dir);
        RDP_PARAMS_LOG("%s",     yes_or_no,             use_client_provided_alternate_shell);
        RDP_PARAMS_LOG("\"%s\"", s_or_null,             target_application_account);
        RDP_PARAMS_LOG("\"%s\"", hidden_or_null,        target_application_password);

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

        RDP_PARAMS_LOG("%s",     s_or_none,             extra_orders);

        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_persistent_disk_bitmap_cache);
        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_cache_waiting_list);
        RDP_PARAMS_LOG("%s",     yes_or_no,             persist_bitmap_cache_on_disk);

        RDP_PARAMS_LOG("%u",     RDP_PARAMS_LOG_GET,    password_printing_mode);

        RDP_PARAMS_LOG("%s",     str_or_none,           allow_channels);
        RDP_PARAMS_LOG("%s",     str_or_none,           deny_channels);

        RDP_PARAMS_LOG("%s",     yes_or_no,             bogus_sc_net_size);

        RDP_PARAMS_LOG("%s",     yes_or_no,             bogus_refresh_rect);

        RDP_PARAMS_LOG("%s",     s_or_none,             proxy_managed_drives);

        auto to_lang = [](Translation::language_t lang) {
            return
                lang == Translation::EN ? "EN" :
                lang == Translation::FR ? "FR" :
                "<unknown>";
        };
        RDP_PARAMS_LOG("%s",     to_lang,               lang);

        RDP_PARAMS_LOG("%s",     yes_or_no,             allow_using_multiple_monitors);

        RDP_PARAMS_LOG("%s",     yes_or_no,             adjust_performance_flags_for_recording);

        RDP_PARAMS_LOG("<%p>",   static_cast<void*>,    client_execute);

        RDP_PARAMS_LOG("0x%04X", RDP_PARAMS_LOG_GET,    client_execute_flags);

        RDP_PARAMS_LOG("%s",     s_or_none,             client_execute_exe_or_file);
        RDP_PARAMS_LOG("%s",     s_or_none,             client_execute_working_dir);
        RDP_PARAMS_LOG("%s",     s_or_none,             client_execute_arguments);

        RDP_PARAMS_LOG("%s",     yes_or_no,             use_client_provided_remoteapp);

        RDP_PARAMS_LOG("%s",     yes_or_no,             should_ignore_first_client_execute);

        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_program);
        RDP_PARAMS_LOG("%s",     yes_or_no,             remote_program_enhanced);

        RDP_PARAMS_LOG("%s",     yes_or_no,             clean_up_32_bpp_cursor);

        RDP_PARAMS_LOG("%s",     yes_or_no,             large_pointer_support);

        RDP_PARAMS_LOG("%s",     s_or_none,             load_balance_info);

        RDP_PARAMS_LOG("%u",     from_millisec,         rail_disconnect_message_delay);

        RDP_PARAMS_LOG("%s",     yes_or_no,             use_session_probe_to_launch_remote_program);

        RDP_PARAMS_LOG("%s",     yes_or_no,             bogus_ios_rdpdr_virtual_channel);

        RDP_PARAMS_LOG("%s",     yes_or_no,             enable_rdpdr_data_analysis);

        RDP_PARAMS_LOG("%u",     from_millisec,         remoteapp_bypass_legal_notice_delay);
        RDP_PARAMS_LOG("%u",     from_millisec,         remoteapp_bypass_legal_notice_timeout);

        RDP_PARAMS_LOG("%s",     yes_or_no,             experimental_fix_input_event_sync);
        RDP_PARAMS_LOG("%s",     yes_or_no,             experimental_fix_too_long_cookie);

        RDP_PARAMS_LOG("%s",     yes_or_no,             log_only_relevant_clipboard_activities);

        RDP_PARAMS_LOG("0x%08X", static_cast<unsigned>, verbose);
        RDP_PARAMS_LOG("0x%08X", static_cast<unsigned>, cache_verbose);

#undef RDP_PARAMS_LOG
#undef RDP_PARAMS_LOG_GET
    }   // void log() const
};  // struct ModRDPParams

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#include "acl/module_manager.hpp"
#include "configs/config.hpp"
#include "core/channels_authorizations.hpp"
#include "core/client_info.hpp"
#include "core/report_message_api.hpp"
#include "acl/dispatch_report_message.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/metrics_hmac.hpp"
#include "mod/rdp/rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/file_validator_service.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/netutils.hpp"
#include "utils/parse_primary_drawing_orders.hpp"
#include "capture/fdx_capture.hpp"

namespace
{
    void file_verification_error(
        FrontAPI& front,
        SessionReactor& session_reactor,
        ReportMessageApi& report_message,
        array_view_const_char up_target_name,
        array_view_const_char down_target_name,
        array_view_const_char msg)
    {
        for (auto&& service : {up_target_name, down_target_name}) {
            if (not service.empty()) {
                report_message.log6(
                    LogId::FILE_VERIFICATION_ERROR, session_reactor.get_current_time(), {
                    KVLog("icap_service"_av, service),
                    KVLog("status"_av, msg),
                });
                front.session_update(LogId::FILE_VERIFICATION_ERROR, {
                    KVLog("icap_service"_av, service),
                    KVLog("status"_av, msg),
                });
            }
        }
    }
}

void ModuleManager::create_mod_rdp(
    AuthApi& authentifier, ReportMessageApi& report_message,
    Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo client_info /* /!\ modified */,
    ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags,
    std::array<uint8_t, 28>& server_auto_reconnect_packet)
{
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP'");

    switch (ini.get<cfg::context::mode_console>()) {
        case RdpModeConsole::force:
            client_info.console_session = true;
            LOG(LOG_INFO, "Session::mode console : force");
            break;
        case RdpModeConsole::forbid:
            client_info.console_session = false;
            LOG(LOG_INFO, "Session::mode console : forbid");
            break;
        case RdpModeConsole::allow:
            break;
    }
    //else {
    //    // default is "allow", do nothing special
    //}

    unique_fd client_sck = [this, &report_message]() {
            try {
                return this->connect_to_target_host(
                    report_message, trkeys::authentification_rdp_fail);
            }
            catch (...) {
                this->front.must_be_stop_capture();

                throw;
            }
        }();

    // BEGIN READ PROXY_OPT
    if (ini.get<cfg::globals::enable_wab_integration>()) {
        ChannelsAuthorizations::update_authorized_channels(
            ini.get_mutable_ref<cfg::mod_rdp::allow_channels>(),
            ini.get_mutable_ref<cfg::mod_rdp::deny_channels>(),
            ini.get<cfg::context::proxy_opt>()
        );
    }
    // END READ PROXY_OPT

    const bool smartcard_passthrough = ini.get<cfg::mod_rdp::force_smartcard_authentication>();

    ini.get_mutable_ref<cfg::context::close_box_extra_message>().clear();
    ModRDPParams mod_rdp_params(
        (smartcard_passthrough ? "" : ini.get<cfg::globals::target_user>().c_str())
      , (smartcard_passthrough ? "" : ini.get<cfg::context::target_password>().c_str())
      , ini.get<cfg::context::target_host>().c_str()
      , "0.0.0.0"   // client ip is silenced
      , key_flags
      , this->glyphs
      , this->theme
      , server_auto_reconnect_packet
      , ini.get_mutable_ref<cfg::context::close_box_extra_message>()
      , to_verbose_flags(ini.get<cfg::debug::mod_rdp>())
      //, RDPVerbose::basic_trace4 | RDPVerbose::basic_trace3 | RDPVerbose::basic_trace7 | RDPVerbose::basic_trace
    );

    SCOPE_EXIT(ini.set<cfg::context::perform_automatic_reconnection>(false));
    mod_rdp_params.perform_automatic_reconnection = ini.get<cfg::context::perform_automatic_reconnection>();

    mod_rdp_params.device_id                           = ini.get<cfg::globals::device_id>().c_str();

    mod_rdp_params.application_params.primary_user_id                     = ini.get<cfg::globals::primary_user_id>().c_str();
    mod_rdp_params.application_params.target_application                  = ini.get<cfg::globals::target_application>().c_str();

    //mod_rdp_params.enable_tls                          = true;
    TLSClientParams tls_client_params;
    tls_client_params.tls_min_level                       = ini.get<cfg::mod_rdp::tls_min_level>();
    tls_client_params.tls_max_level                       = ini.get<cfg::mod_rdp::tls_max_level>();
    tls_client_params.show_common_cipher_list             = ini.get<cfg::mod_rdp::show_common_cipher_list>();
    tls_client_params.cipher_string                       = ini.get<cfg::mod_rdp::cipher_string>();

    if (!mod_rdp_params.target_password[0]) {
        mod_rdp_params.enable_nla                      = false;
    }
    else {
        mod_rdp_params.enable_nla                      = ini.get<cfg::mod_rdp::enable_nla>();
    }
    mod_rdp_params.enable_krb                          = ini.get<cfg::mod_rdp::enable_kerberos>();
    mod_rdp_params.enable_fastpath                     = ini.get<cfg::mod_rdp::fast_path>();
    //mod_rdp_params.enable_new_pointer                  = true;
    mod_rdp_params.enable_glyph_cache                  = ini.get<cfg::globals::glyph_cache>();

    mod_rdp_params.session_probe_params.enable_session_probe                = ini.get<cfg::mod_rdp::enable_session_probe>();
    mod_rdp_params.session_probe_params.enable_launch_mask    = ini.get<cfg::mod_rdp::session_probe_enable_launch_mask>();

    mod_rdp_params.session_probe_params.used_clipboard_based_launcher
                                                        = ini.get<cfg::mod_rdp::session_probe_use_clipboard_based_launcher>();

    mod_rdp_params.session_probe_params.start_launch_timeout_timer_only_after_logon
                                                        = ini.get<cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon>();

    mod_rdp_params.session_probe_params.vc_params.effective_launch_timeout
        = ini.get<cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon>()
        ? ((ini.get<cfg::mod_rdp::session_probe_on_launch_failure>()
                == SessionProbeOnLaunchFailure::disconnect_user)
            ? ini.get<cfg::mod_rdp::session_probe_launch_timeout>()
            : ini.get<cfg::mod_rdp::session_probe_launch_fallback_timeout>())
        : std::chrono::milliseconds::zero();
    mod_rdp_params.session_probe_params.vc_params.on_launch_failure     = ini.get<cfg::mod_rdp::session_probe_on_launch_failure>();
    mod_rdp_params.session_probe_params.vc_params.keepalive_timeout     = ini.get<cfg::mod_rdp::session_probe_keepalive_timeout>();
    mod_rdp_params.session_probe_params.vc_params.on_keepalive_timeout  =
                                                            ini.get<cfg::mod_rdp::session_probe_on_keepalive_timeout>();
    mod_rdp_params.session_probe_params.vc_params.end_disconnected_session
                                                        = ini.get<cfg::mod_rdp::session_probe_end_disconnected_session>();
    mod_rdp_params.session_probe_params.customize_executable_name
                                                        = ini.get<cfg::mod_rdp::session_probe_customize_executable_name>();
    mod_rdp_params.session_probe_params.vc_params.disconnected_application_limit =
                                                        ini.get<cfg::mod_rdp::session_probe_disconnected_application_limit>();
    mod_rdp_params.session_probe_params.vc_params.disconnected_session_limit =
                                                        ini.get<cfg::mod_rdp::session_probe_disconnected_session_limit>();
    mod_rdp_params.session_probe_params.vc_params.idle_session_limit    =
                                                        ini.get<cfg::mod_rdp::session_probe_idle_session_limit>();
    mod_rdp_params.session_probe_params.exe_or_file           = ini.get<cfg::mod_rdp::session_probe_exe_or_file>();
    mod_rdp_params.session_probe_params.arguments             = ini.get<cfg::mod_rdp::session_probe_arguments>();

    mod_rdp_params.session_probe_params.vc_params.launcher_abort_delay                             = ini.get<cfg::mod_rdp::session_probe_launcher_abort_delay>();

    mod_rdp_params.session_probe_params.clipboard_based_launcher.clipboard_initialization_delay_ms = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_clipboard_initialization_delay>();
    mod_rdp_params.session_probe_params.clipboard_based_launcher.start_delay_ms                    = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_start_delay>();
    mod_rdp_params.session_probe_params.clipboard_based_launcher.long_delay_ms                     = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_long_delay>();
    mod_rdp_params.session_probe_params.clipboard_based_launcher.short_delay_ms                    = ini.get<cfg::mod_rdp::session_probe_clipboard_based_launcher_short_delay>();

    mod_rdp_params.session_probe_params.vc_params.end_of_session_check_delay_time =
                                                        ini.get<cfg::mod_rdp::session_probe_end_of_session_check_delay_time>();

    mod_rdp_params.session_probe_params.vc_params.ignore_ui_less_processes_during_end_of_session_check = ini.get<cfg::mod_rdp::session_probe_ignore_ui_less_processes_during_end_of_session_check>();

    mod_rdp_params.session_probe_params.vc_params.childless_window_as_unidentified_input_field = ini.get<cfg::mod_rdp::session_probe_childless_window_as_unidentified_input_field>();

    mod_rdp_params.session_probe_params.is_public_session        = ini.get<cfg::mod_rdp::session_probe_public_session>();

    mod_rdp_params.session_probe_params.vc_params.session_shadowing_support = ini.get<cfg::mod_rdp::session_shadowing_support>();

    mod_rdp_params.session_probe_params.vc_params.on_account_manipulation   = ini.get<cfg::mod_rdp::session_probe_on_account_manipulation>();

    mod_rdp_params.clipboard_params.disable_log_syslog        = bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::syslog);
    mod_rdp_params.file_system_params.disable_log_syslog      = bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::syslog);
    mod_rdp_params.session_probe_params.vc_params.extra_system_processes =
        ExtraSystemProcesses(
            ini.get<cfg::context::session_probe_extra_system_processes>().c_str());
    mod_rdp_params.session_probe_params.vc_params.outbound_connection_monitor_rules =
        OutboundConnectionMonitorRules(
            ini.get<cfg::context::session_probe_outbound_connection_monitoring_rules>().c_str());
    mod_rdp_params.session_probe_params.vc_params.process_monitor_rules             =
        ProcessMonitorRules(
            ini.get<cfg::context::session_probe_process_monitoring_rules>().c_str());

    mod_rdp_params.session_probe_params.vc_params.windows_of_these_applications_as_unidentified_input_field =
        ExtraSystemProcesses(
            ini.get<cfg::context::session_probe_windows_of_these_applications_as_unidentified_input_field>().c_str());

    mod_rdp_params.session_probe_params.vc_params.enable_log            = ini.get<cfg::mod_rdp::session_probe_enable_log>();
    mod_rdp_params.session_probe_params.vc_params.enable_log_rotation   = ini.get<cfg::mod_rdp::session_probe_enable_log_rotation>();

    mod_rdp_params.session_probe_params.vc_params.allow_multiple_handshake
                                                        = ini.get<cfg::mod_rdp::session_probe_allow_multiple_handshake>();

    mod_rdp_params.session_probe_params.vc_params.enable_crash_dump     = ini.get<cfg::mod_rdp::session_probe_enable_crash_dump>();

    mod_rdp_params.session_probe_params.vc_params.handle_usage_limit    = ini.get<cfg::mod_rdp::session_probe_handle_usage_limit>();
    mod_rdp_params.session_probe_params.vc_params.memory_usage_limit    = ini.get<cfg::mod_rdp::session_probe_memory_usage_limit>();

    const bool enable_bestsafe_integration = ini.get<cfg::mod_rdp::enable_bestsafe_integration>();
    // bestsafe integration option should be in separate parameter for session probe
    mod_rdp_params.session_probe_params.vc_params.disabled_features     = ini.get<cfg::mod_rdp::session_probe_disabled_features>() | (enable_bestsafe_integration ? SessionProbeDisabledFeature::none : SessionProbeDisabledFeature::reserved);
    mod_rdp_params.ignore_auth_channel                 = ini.get<cfg::mod_rdp::ignore_auth_channel>();
    mod_rdp_params.auth_channel                        = CHANNELS::ChannelNameId(ini.get<cfg::mod_rdp::auth_channel>());
    mod_rdp_params.checkout_channel                    = CHANNELS::ChannelNameId(ini.get<cfg::mod_rdp::checkout_channel>());
    mod_rdp_params.application_params.alternate_shell                     = ini.get<cfg::mod_rdp::alternate_shell>().c_str();
    mod_rdp_params.application_params.shell_arguments                     = ini.get<cfg::mod_rdp::shell_arguments>().c_str();
    mod_rdp_params.application_params.shell_working_dir                   = ini.get<cfg::mod_rdp::shell_working_directory>().c_str();
    mod_rdp_params.application_params.use_client_provided_alternate_shell = ini.get<cfg::mod_rdp::use_client_provided_alternate_shell>();
    mod_rdp_params.application_params.target_application_account          = ini.get<cfg::globals::target_application_account>().c_str();
    mod_rdp_params.application_params.target_application_password         = ini.get<cfg::globals::target_application_password>().c_str();
    mod_rdp_params.rdp_compression                     = ini.get<cfg::mod_rdp::rdp_compression>();
    mod_rdp_params.error_message                       = &ini.get_mutable_ref<cfg::context::auth_error_message>();
    mod_rdp_params.disconnect_on_logon_user_change     = ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>();
    mod_rdp_params.open_session_timeout                = ini.get<cfg::mod_rdp::open_session_timeout>();

    mod_rdp_params.server_cert_store                   = ini.get<cfg::mod_rdp::server_cert_store>();
    mod_rdp_params.server_cert_check                   = ini.get<cfg::mod_rdp::server_cert_check>();
    mod_rdp_params.server_access_allowed_message       = ini.get<cfg::mod_rdp::server_access_allowed_message>();
    mod_rdp_params.server_cert_create_message          = ini.get<cfg::mod_rdp::server_cert_create_message>();
    mod_rdp_params.server_cert_success_message         = ini.get<cfg::mod_rdp::server_cert_success_message>();
    mod_rdp_params.server_cert_failure_message         = ini.get<cfg::mod_rdp::server_cert_failure_message>();
    mod_rdp_params.server_cert_error_message           = ini.get<cfg::mod_rdp::server_cert_error_message>();

    mod_rdp_params.enable_server_cert_external_validation = ini.get<cfg::mod_rdp::enable_server_cert_external_validation>();

    mod_rdp_params.hide_client_name                    = ini.get<cfg::mod_rdp::hide_client_name>();

    mod_rdp_params.enable_persistent_disk_bitmap_cache = ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>();
    mod_rdp_params.enable_cache_waiting_list           = ini.get<cfg::mod_rdp::cache_waiting_list>();
    mod_rdp_params.persist_bitmap_cache_on_disk        = ini.get<cfg::mod_rdp::persist_bitmap_cache_on_disk>();
    mod_rdp_params.password_printing_mode              = ini.get<cfg::debug::password>();
    mod_rdp_params.cache_verbose                       = to_verbose_flags(ini.get<cfg::debug::cache>());

    mod_rdp_params.disabled_orders                     += parse_primary_drawing_orders(
        ini.get<cfg::mod_rdp::disabled_orders>().c_str(),
        bool(to_verbose_flags(ini.get<cfg::debug::mod_rdp>()) & (RDPVerbose::basic_trace | RDPVerbose::capabilities)));

    mod_rdp_params.bogus_sc_net_size                   = ini.get<cfg::mod_rdp::bogus_sc_net_size>();
    mod_rdp_params.bogus_refresh_rect                  = ini.get<cfg::globals::bogus_refresh_rect>();

    mod_rdp_params.drive_params.proxy_managed_drives                = ini.get<cfg::mod_rdp::proxy_managed_drives>().c_str();
    mod_rdp_params.drive_params.proxy_managed_prefix          = app_path(AppPath::DriveRedirection);

    mod_rdp_params.lang                                = language(ini);

    mod_rdp_params.allow_using_multiple_monitors       = ini.get<cfg::globals::allow_using_multiple_monitors>();

    mod_rdp_params.adjust_performance_flags_for_recording
                                                        = (ini.get<cfg::globals::is_rec>() &&
                                                            ini.get<cfg::client::auto_adjust_performance_flags>() &&
                                                            ((ini.get<cfg::video::capture_flags>() &
                                                            (CaptureFlags::wrm | CaptureFlags::ocr)) !=
                                                            CaptureFlags::none));
    mod_rdp_params.remote_app_params.rail_client_execute               = &rail_client_execute;

    mod_rdp_params.remote_app_params.client_execute                      = rail_client_execute.get_client_execute();

    mod_rdp_params.remote_app_params.should_ignore_first_client_execute  = rail_client_execute.should_ignore_first_client_execute();

    bool const rail_is_required = (ini.get<cfg::mod_rdp::use_native_remoteapp_capability>()
        && ((mod_rdp_params.application_params.target_application
          && *mod_rdp_params.application_params.target_application)
         || (ini.get<cfg::mod_rdp::use_client_provided_remoteapp>()
         && not mod_rdp_params.remote_app_params.client_execute.exe_or_file.empty())));

    mod_rdp_params.remote_app_params.enable_remote_program
      = ((client_info.remote_program ||
          (ini.get<cfg::mod_rdp::wabam_uses_translated_remoteapp>() &&
           ini.get<cfg::context::is_wabam>())) &&
         rail_is_required);
    mod_rdp_params.remote_app_params.remote_program_enhanced             = client_info.remote_program_enhanced;
    mod_rdp_params.remote_app_params.convert_remoteapp_to_desktop        = (!client_info.remote_program &&
                                                          ini.get<cfg::mod_rdp::wabam_uses_translated_remoteapp>() &&
                                                          ini.get<cfg::context::is_wabam>() &&
                                                          rail_is_required);
    mod_rdp_params.remote_app_params.use_client_provided_remoteapp       = ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();

    mod_rdp_params.clean_up_32_bpp_cursor              = ini.get<cfg::mod_rdp::clean_up_32_bpp_cursor>();

    mod_rdp_params.large_pointer_support               = ini.get<cfg::globals::large_pointer_support>();

    mod_rdp_params.load_balance_info                   = ini.get<cfg::mod_rdp::load_balance_info>().c_str();

    mod_rdp_params.remote_app_params.rail_disconnect_message_delay       = ini.get<cfg::context::rail_disconnect_message_delay>();

    mod_rdp_params.session_probe_params.used_to_launch_remote_program
                                                        = ini.get<cfg::context::use_session_probe_to_launch_remote_program>();

    mod_rdp_params.file_system_params.bogus_ios_rdpdr_virtual_channel     = ini.get<cfg::mod_rdp::bogus_ios_rdpdr_virtual_channel>();

    mod_rdp_params.file_system_params.enable_rdpdr_data_analysis          =  ini.get<cfg::mod_rdp::enable_rdpdr_data_analysis>();

    mod_rdp_params.remote_app_params.bypass_legal_notice_delay = ini.get<cfg::mod_rdp::remoteapp_bypass_legal_notice_delay>();
    mod_rdp_params.remote_app_params.bypass_legal_notice_timeout
                                                       = ini.get<cfg::mod_rdp::remoteapp_bypass_legal_notice_timeout>();

    mod_rdp_params.experimental_fix_input_event_sync   = ini.get<cfg::mod_rdp::experimental_fix_input_event_sync>();
    mod_rdp_params.session_probe_params.fix_too_long_cookie    = ini.get<cfg::mod_rdp::experimental_fix_too_long_cookie>();

    mod_rdp_params.support_connection_redirection_during_recording =
                                                         ini.get<cfg::globals::support_connection_redirection_during_recording>();

    mod_rdp_params.clipboard_params.log_only_relevant_activities
                                                       = ini.get<cfg::mod_rdp::log_only_relevant_clipboard_activities>();
    mod_rdp_params.split_domain                        = ini.get<cfg::mod_rdp::split_domain>();

    mod_rdp_params.validator_params.log_if_accepted = ini.get<cfg::file_verification::log_if_accepted>();
    mod_rdp_params.validator_params.enable_clipboard_text_up = ini.get<cfg::file_verification::clipboard_text_up>();
    mod_rdp_params.validator_params.enable_clipboard_text_down = ini.get<cfg::file_verification::clipboard_text_down>();
    mod_rdp_params.validator_params.up_target_name = ini.get<cfg::file_verification::enable_up>() ? "up" : "";
    mod_rdp_params.validator_params.down_target_name = ini.get<cfg::file_verification::enable_down>() ? "down" : "";

    mod_rdp_params.enable_remotefx = ini.get<cfg::mod_rdp::enable_remotefx>();

    mod_rdp_params.use_license_store                   = ini.get<cfg::mod_rdp::use_license_store>();

    mod_rdp_params.accept_monitor_layout_change_if_capture_is_not_started
                                                       = ini.get<cfg::mod_rdp::accept_monitor_layout_change_if_capture_is_not_started>();

    mod_rdp_params.enable_restricted_admin_mode = ini.get<cfg::mod_rdp::enable_restricted_admin_mode>();
    mod_rdp_params.file_system_params.smartcard_passthrough        = smartcard_passthrough;

    try {
        using LogCategoryFlags = DispatchReportMessage::LogCategoryFlags;

        LogCategoryFlags dont_log_category;
        if (bool(ini.get<cfg::video::disable_file_system_log>() & FileSystemLogFlags::wrm)) {
            dont_log_category |= LogCategoryId::Drive;
        }
        if (bool(ini.get<cfg::video::disable_clipboard_log>() & ClipboardLogFlags::wrm)) {
            dont_log_category |= LogCategoryId::Clipboard;
        }

        const char * const name = "RDP Target";

        Rect const adjusted_client_execute_rect =
            rail_client_execute.adjust_rect(client_info.cs_monitor.get_widget_rect(
                    client_info.screen_info.width,
                    client_info.screen_info.height
                ));

        const bool host_mod_in_widget =
            (client_info.remote_program &&
                !mod_rdp_params.remote_app_params.enable_remote_program);

        if (host_mod_in_widget) {
            client_info.screen_info.width  = adjusted_client_execute_rect.cx / 4 * 4;
            client_info.screen_info.height = adjusted_client_execute_rect.cy;
            client_info.cs_monitor = GCC::UserData::CSMonitor{};
        }
        else {
            rail_client_execute.reset(false);
        }

        struct RdpData
        {
            struct ModMetrics : Metrics
            {
                using Metrics::Metrics;

                RDPMetrics protocol_metrics{*this};
                SessionReactor::TimerPtr metrics_timer;
            };

            struct FileValidator
            {
                struct FileValidatorTransport : FileTransport
                {
                    using FileTransport::FileTransport;

                    size_t do_partial_read(uint8_t * buffer, size_t len) override
                    {
                        size_t r = FileTransport::do_partial_read(buffer, len);
                        if (r == 0) {
                            LOG(LOG_ERR, "ModuleManager::create_mod_rdp: ModRDPWithMetrics::FileValidator::do_partial_read: No data read!");
                            throw this->get_report_error()(Error(ERR_TRANSPORT_NO_MORE_DATA, errno));
                        }
                        return r;
                    }
                };

                struct CtxError
                {
                    ReportMessageApi & report_message;
                    std::string up_target_name;
                    std::string down_target_name;
                    SessionReactor& session_reactor;
                    FrontAPI& front;
                };

                CtxError ctx_error;
                FileValidatorTransport trans;
                // TODO wait result (add delay)
                FileValidatorService service;
                SessionReactor::TopFdPtr validator_event;

                FileValidator(unique_fd&& fd, CtxError&& ctx_error)
                : ctx_error(std::move(ctx_error))
                , trans(std::move(fd), ReportError([this](Error err){
                    file_verification_error(
                        this->ctx_error.front,
                        this->ctx_error.session_reactor,
                        this->ctx_error.report_message,
                        this->ctx_error.up_target_name,
                        this->ctx_error.down_target_name,
                        err.errmsg()
                    );
                    return err;
                }))
                , service(this->trans)
                {}

                ~FileValidator()
                {
                    try {
                        this->service.send_close_session();
                    }
                    catch (...) {
                    }
                }
            };

            std::unique_ptr<ModMetrics> metrics;
            std::unique_ptr<FileValidator> file_validator;
            std::unique_ptr<FdxCapture> fdx_capture;
            Fstat fstat;
        };

        struct ModRDPWithMetrics : DispatchReportMessage, RdpData, ModRdpFactory, mod_rdp
        {
            FdxCapture* get_fdx_capture(ModuleManager& mm)
            {
                if (!this->fdx_capture) {
                    LOG(LOG_INFO, "Enable clipboard file storage");
                    int  const groupid = mm.ini.get<cfg::video::capture_groupid>();
                    auto const& session_id = mm.ini.get<cfg::context::session_id>();
                    auto const& subdir = mm.ini.get<cfg::capture::record_subdirectory>();
                    auto const& record_dir = mm.ini.get<cfg::video::record_path>();
                    auto const& hash_dir = mm.ini.get<cfg::video::hash_path>();
                    auto const& filebase = mm.ini.get<cfg::capture::record_filebase>();

                    this->fdx_capture = std::make_unique<FdxCapture>(
                        str_concat(record_dir.as_string(), subdir),
                        str_concat(hash_dir.as_string(), subdir),
                        filebase,
                        session_id, groupid, mm.cctx, mm.gen, this->fstat,
                        /* TODO should be a log (siem?)*/
                        ReportError());

                    mm.ini.set_acl<cfg::capture::fdx_path>(this->fdx_capture->get_fdx_path());
                }

                return this->fdx_capture.get();
            }

            ModRdpFactory& get_rdp_factory()
            {
                return *this;
            }

            explicit ModRDPWithMetrics(
                Transport & trans,
                SessionReactor& session_reactor,
                gdi::GraphicApi & gd,
                FrontAPI & front,
                const ClientInfo & info,
                RedirectionInfo & redir_info,
                Random & gen,
                TimeObj & timeobj,
                ChannelsAuthorizations channels_authorizations,
                const ModRDPParams & mod_rdp_params,
                const TLSClientParams & tls_client_params,
                AuthApi & authentifier,
                ReportMessageApi & report_message,
                LicenseApi & license_store,
                LogCategoryFlags dont_log_category,
                ModRdpVariables vars,
                RDPMetrics * metrics,
                FileValidatorService * file_validator_service)
            : DispatchReportMessage(report_message, front, dont_log_category)
            , mod_rdp(
                trans, session_reactor, gd, front, info, redir_info, gen, timeobj,
                channels_authorizations, mod_rdp_params, tls_client_params, authentifier,
                static_cast<DispatchReportMessage&>(*this), license_store, vars,
                metrics, file_validator_service, this->get_rdp_factory())
            {}
        };

        bool enable_validator = ini.get<cfg::file_verification::enable_up>() || ini.get<cfg::file_verification::enable_down>();
        bool const enable_metrics = (ini.get<cfg::metrics::enable_rdp_metrics>()
            && create_metrics_directory(ini.get<cfg::metrics::log_dir_path>().as_string()));

        std::unique_ptr<ModRDPWithMetrics::ModMetrics> metrics;
        std::unique_ptr<ModRDPWithMetrics::FileValidator> file_validator;
        int validator_fd = -1;

        if (enable_validator) {
            auto const& socket_path = ini.get<cfg::file_verification::socket_path>();
            bool const no_log_for_unix_socket = false;
            unique_fd ufd = addr_connect(socket_path.c_str(), no_log_for_unix_socket);
            if (ufd) {
                validator_fd = ufd.fd();
                fcntl(validator_fd, F_SETFL, fcntl(validator_fd, F_GETFL) & ~O_NONBLOCK);
                file_validator = std::make_unique<ModRDPWithMetrics::FileValidator>(
                    std::move(ufd),
                    ModRDPWithMetrics::FileValidator::CtxError{
                        report_message,
                        mod_rdp_params.validator_params.up_target_name,
                        mod_rdp_params.validator_params.down_target_name,
                        this->session_reactor, this->front
                    });
                file_validator->service.send_infos({
                    "server_ip"_av, ini.get<cfg::context::target_host>(),
                    "client_ip"_av, ini.get<cfg::globals::host>(),
                    "auth_user"_av, ini.get<cfg::globals::auth_user>()
                });
            }
            else {
                LOG(LOG_ERR, "Error, can't connect to validator, file validation disable");
                file_verification_error(
                    front, session_reactor, report_message,
                    mod_rdp_params.validator_params.up_target_name,
                    mod_rdp_params.validator_params.down_target_name,
                    "Unable to connect to FileValidator service"_av
                );
                // enable_validator = false;
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
        }

        if (enable_metrics) {
            metrics = std::make_unique<ModRDPWithMetrics::ModMetrics>(
                ini.get<cfg::metrics::log_dir_path>().as_string(),
                ini.get<cfg::context::session_id>(),
                hmac_user(
                    ini.get<cfg::globals::auth_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_account(
                    ini.get<cfg::globals::target_user>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_device_service(
                    ini.get<cfg::globals::target_device>(),
                    ini.get<cfg::context::target_service>(),
                    ini.get<cfg::metrics::sign_key>()),
                hmac_client_info(
                    ini.get<cfg::globals::host>(),
                    client_info.screen_info,
                    ini.get<cfg::metrics::sign_key>()),
                this->timeobj.get_time(),
                ini.get<cfg::metrics::log_file_turnover_interval>(),
                ini.get<cfg::metrics::log_interval>());
        }

        auto new_mod = std::make_unique<ModWithSocket<ModRDPWithMetrics>>(
            *this,
            authentifier,
            name,
            std::move(client_sck),
            ini.get<cfg::debug::mod_rdp>(),
            &ini.get_mutable_ref<cfg::context::auth_error_message>(),
            sock_mod_barrier(),
            this->session_reactor,
            drawable,
            front,
            client_info,
            ini.get_mutable_ref<cfg::mod_rdp::redir_info>(),
            this->gen,
            this->timeobj,
            ChannelsAuthorizations(
                ini.get<cfg::mod_rdp::allow_channels>(),
                ini.get<cfg::mod_rdp::deny_channels>()),
            mod_rdp_params,
            tls_client_params,
            authentifier,
            report_message,
            file_system_license_store,
            dont_log_category,
            ini,
            enable_metrics ? &metrics->protocol_metrics : nullptr,
            enable_validator ? &file_validator->service : nullptr
        );

        if (enable_validator) {
            new_mod->file_validator = std::move(file_validator);
            new_mod->file_validator->validator_event = this->session_reactor.create_fd_event(validator_fd)
            .set_timeout(std::chrono::milliseconds::max())
            .on_timeout(jln::always_ready([]{}))
            .on_exit(jln::propagate_exit())
            .on_action(jln::always_ready([rdp=new_mod.get()]() {
                rdp->DLP_antivirus_check_channels_files();
            }));
        }

        if (enable_metrics) {
            new_mod->RdpData::metrics = std::move(metrics);
            new_mod->RdpData::metrics->metrics_timer = session_reactor.create_timer()
                .set_delay(std::chrono::seconds(ini.get<cfg::metrics::log_interval>()))
                .on_action([metrics = new_mod->RdpData::metrics.get()](JLN_TIMER_CTX ctx){
                    metrics->log(ctx.get_current_time());
                    return ctx.ready();
                })
            ;
        }

        if (new_mod) {
            assert(&ini == &this->ini);
            new_mod->get_rdp_factory().always_file_storage
              = (ini.get<cfg::file_storage::store_file>() == RdpStoreFile::always);
            switch (ini.get<cfg::file_storage::store_file>())
            {
                case RdpStoreFile::never:
                    break;
                case RdpStoreFile::on_invalid_verification:
                    if (!enable_validator) {
                        break;
                    }
                    [[fallthrough]];
                case RdpStoreFile::always:
                    new_mod->get_rdp_factory().get_fdx_capture = [mod = new_mod.get(), this]{
                        return mod->get_fdx_capture(*this);
                    };
            }
        }

        if (host_mod_in_widget) {
            LOG(LOG_INFO, "ModuleManager::Creation of internal module 'RailModuleHostMod'");

            std::string target_info = str_concat(
                ini.get<cfg::context::target_str>(),
                ':',
                ini.get<cfg::globals::primary_user_id>());

            rail_client_execute.set_target_info(target_info);

            auto* host_mod = new RailModuleHostMod(
                ini,
                this->session_reactor,
                drawable,
                front,
                client_info.screen_info.width,
                client_info.screen_info.height,
                adjusted_client_execute_rect,
                std::move(new_mod),
                rail_client_execute,
                this->glyphs,
                this->theme,
                client_info.cs_monitor,
                !ini.get<cfg::globals::is_rec>()
            );

            this->set_mod(host_mod, nullptr, &rail_client_execute);
            this->rail_module_host_mod_ptr = host_mod;
            LOG(LOG_INFO, "ModuleManager::internal module 'RailModuleHostMod' ready");
        }
        else {
            rdp_api*       rdpapi = new_mod.get();
            windowing_api* winapi = new_mod->get_windowing_api();
            this->set_mod(new_mod.release(), rdpapi, winapi);
        }

        /* If provided by connection policy, session timeout update */
        report_message.update_inactivity_timeout();
    }
    catch (...) {
        report_message.log6(LogId::SESSION_CREATION_FAILED, this->session_reactor.get_current_time(), {});

        this->front.must_be_stop_capture();

        throw;
    }

    if (ini.get<cfg::globals::bogus_refresh_rect>() &&
        ini.get<cfg::globals::allow_using_multiple_monitors>() &&
        (client_info.cs_monitor.monitorCount > 1)) {
        this->get_mod_wrapper().mod->rdp_suppress_display_updates();
        this->get_mod_wrapper().mod->rdp_allow_display_updates(0, 0,
            client_info.screen_info.width, client_info.screen_info.height);
    }
    this->get_mod_wrapper().mod->rdp_input_invalidate(Rect(0, 0, client_info.screen_info.width, client_info.screen_info.height));
    LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded");
    ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
    this->connected = true;
}

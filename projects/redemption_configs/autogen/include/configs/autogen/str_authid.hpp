// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN
// 

#pragma once

#include "utils/sugar/zstring_view.hpp"
#include "configs/loggable.hpp"
#include "configs/autogen/authid.hpp"

namespace configs
{
    constexpr zstring_view const authstr[] = {
        "front_connection_time"_zv,
        "target_connection_time"_zv,
        "nla_login"_zv,
        "login"_zv,
        "ip_client"_zv,
        "ip_target"_zv,
        "target_device"_zv,
        "device_id"_zv,
        "primary_user_id"_zv,
        "target_login"_zv,
        "target_application"_zv,
        "target_application_account"_zv,
        "target_application_password"_zv,
        "globals:inactivity_timeout"_zv,
        "trace_type"_zv,
        "is_rec"_zv,
        "mod_recv_timeout"_zv,
        "keyboard_layout"_zv,
        "disable_tsk_switch_shortcuts"_zv,
        "all_target_mod:tcp_user_timeout"_zv,
        "mod_rdp:disabled_orders"_zv,
        "mod_rdp:enable_nla"_zv,
        "mod_rdp:enable_kerberos"_zv,
        "mod_rdp:tls_min_level"_zv,
        "mod_rdp:tls_max_level"_zv,
        "mod_rdp:cipher_string"_zv,
        "mod_rdp:show_common_cipher_list"_zv,
        "allow_channels"_zv,
        "deny_channels"_zv,
        "mod_rdp:allowed_dynamic_channels"_zv,
        "mod_rdp:denied_dynamic_channels"_zv,
        "mod_rdp:server_redirection_support"_zv,
        "mod_rdp:load_balance_info"_zv,
        "rdp_bogus_sc_net_size"_zv,
        "proxy_managed_drives"_zv,
        "ignore_auth_channel"_zv,
        "alternate_shell"_zv,
        "shell_arguments"_zv,
        "shell_working_directory"_zv,
        "mod_rdp:use_client_provided_alternate_shell"_zv,
        "mod_rdp:use_client_provided_remoteapp"_zv,
        "mod_rdp:use_native_remoteapp_capability"_zv,
        "mod_rdp:enable_rdpdr_data_analysis"_zv,
        "mod_rdp:wabam_uses_translated_remoteapp"_zv,
        "mod_rdp:enable_remotefx"_zv,
        "mod_rdp:enable_restricted_admin_mode"_zv,
        "mod_rdp:force_smartcard_authentication"_zv,
        "mod_rdp:enable_ipv6"_zv,
        "mod_rdp:mode_console"_zv,
        "mod_rdp:auto_reconnection_on_losing_target_link"_zv,
        "mod_rdp:forward_client_build_number"_zv,
        "mod_rdp:bogus_monitor_layout_treatment"_zv,
        "effective_krb_armoring_user"_zv,
        "effective_krb_armoring_password"_zv,
        "mod_rdp:remote_programs_disconnect_message_delay"_zv,
        "mod_rdp:use_session_probe_to_launch_remote_program"_zv,
        "session_probe:enable_session_probe"_zv,
        "session_probe:use_smart_launcher"_zv,
        "session_probe:enable_launch_mask"_zv,
        "session_probe:on_launch_failure"_zv,
        "session_probe:launch_timeout"_zv,
        "session_probe:launch_fallback_timeout"_zv,
        "session_probe:start_launch_timeout_timer_only_after_logon"_zv,
        "session_probe:keepalive_timeout"_zv,
        "session_probe:on_keepalive_timeout"_zv,
        "session_probe:end_disconnected_session"_zv,
        "session_probe:enable_log"_zv,
        "session_probe:enable_log_rotation"_zv,
        "session_probe:log_level"_zv,
        "session_probe:disconnected_application_limit"_zv,
        "session_probe:disconnected_session_limit"_zv,
        "session_probe:idle_session_limit"_zv,
        "session_probe:smart_launcher_clipboard_initialization_delay"_zv,
        "session_probe:smart_launcher_start_delay"_zv,
        "session_probe:smart_launcher_long_delay"_zv,
        "session_probe:smart_launcher_short_delay"_zv,
        "session_probe:smart_launcher_enable_wabam_affinity"_zv,
        "session_probe:launcher_abort_delay"_zv,
        "session_probe:enable_crash_dump"_zv,
        "session_probe:handle_usage_limit"_zv,
        "session_probe:memory_usage_limit"_zv,
        "session_probe:cpu_usage_alarm_threshold"_zv,
        "session_probe:cpu_usage_alarm_action"_zv,
        "session_probe:end_of_session_check_delay_time"_zv,
        "session_probe:ignore_ui_less_processes_during_end_of_session_check"_zv,
        "session_probe:childless_window_as_unidentified_input_field"_zv,
        "session_probe:update_disabled_features"_zv,
        "session_probe:disabled_features"_zv,
        "session_probe:enable_bestsafe_interaction"_zv,
        "session_probe:on_account_manipulation"_zv,
        "session_probe:alternate_directory_environment_variable"_zv,
        "session_probe:public_session"_zv,
        "session_probe:outbound_connection_monitoring_rules"_zv,
        "session_probe:process_monitoring_rules"_zv,
        "session_probe:extra_system_processes"_zv,
        "session_probe:windows_of_these_applications_as_unidentified_input_field"_zv,
        "server_cert:server_cert_store"_zv,
        "server_cert:server_cert_check"_zv,
        "server_cert:server_access_allowed_message"_zv,
        "server_cert:server_cert_create_message"_zv,
        "server_cert:server_cert_success_message"_zv,
        "server_cert:server_cert_failure_message"_zv,
        "enable_external_validation"_zv,
        "external_cert"_zv,
        "external_response"_zv,
        "clipboard_up"_zv,
        "clipboard_down"_zv,
        "vnc_server_clipboard_encoding_type"_zv,
        "vnc_bogus_clipboard_infinite_loop"_zv,
        "mod_vnc:server_is_macos"_zv,
        "mod_vnc:server_unix_alt"_zv,
        "mod_vnc:support_cursor_pseudo_encoding"_zv,
        "mod_vnc:enable_ipv6"_zv,
        "session_log:keyboard_input_masking_level"_zv,
        "record_filebase"_zv,
        "record_subdirectory"_zv,
        "fdx_path"_zv,
        "capture:disable_keyboard_log"_zv,
        "hash_path"_zv,
        "record_tmp_path"_zv,
        "record_path"_zv,
        "rt_display"_zv,
        "use_redis"_zv,
        "redis_address"_zv,
        "redis_port"_zv,
        "redis_password"_zv,
        "redis_db"_zv,
        "redis_use_tls"_zv,
        "redis_tls_cacert"_zv,
        "redis_tls_cert"_zv,
        "redis_tls_key"_zv,
        "redis_timeout"_zv,
        "file_verification:enable_up"_zv,
        "file_verification:enable_down"_zv,
        "file_verification:clipboard_text_up"_zv,
        "file_verification:clipboard_text_down"_zv,
        "file_verification:block_invalid_file_up"_zv,
        "file_verification:block_invalid_file_down"_zv,
        "file_verification:log_if_accepted"_zv,
        "file_verification:max_file_size_rejected"_zv,
        "file_storage:store_file"_zv,
        "encryption_key"_zv,
        "sign_key"_zv,
        "psid"_zv,
        "bpp"_zv,
        "height"_zv,
        "width"_zv,
        "selector"_zv,
        "selector_current_page"_zv,
        "selector_device_filter"_zv,
        "selector_group_filter"_zv,
        "selector_proto_filter"_zv,
        "selector_lines_per_page"_zv,
        "selector_number_of_pages"_zv,
        "target_password"_zv,
        "target_host"_zv,
        "tunneling_target_host"_zv,
        "target_str"_zv,
        "target_service"_zv,
        "target_port"_zv,
        "proto_dest"_zv,
        "password"_zv,
        "nla_password_hash"_zv,
        "reporting"_zv,
        "auth_channel_answer"_zv,
        "auth_channel_target"_zv,
        "message"_zv,
        "display_link"_zv,
        "mod_timeout"_zv,
        "accept_message"_zv,
        "display_message"_zv,
        "rejected"_zv,
        "keepalive"_zv,
        "session_id"_zv,
        "timeclose"_zv,
        "real_target_device"_zv,
        "authentication_challenge"_zv,
        "ticket"_zv,
        "comment"_zv,
        "duration"_zv,
        "duration_max"_zv,
        "waitinforeturn"_zv,
        "showform"_zv,
        "formflag"_zv,
        "module"_zv,
        "try_alternate_target"_zv,
        "has_more_target"_zv,
        "proxy_opt"_zv,
        "pattern_kill"_zv,
        "pattern_notify"_zv,
        "opt_message"_zv,
        "login_message"_zv,
        "disconnect_reason"_zv,
        "disconnect_reason_ack"_zv,
        "recording_started"_zv,
        "rt_ready"_zv,
        "auth_command"_zv,
        "auth_notify"_zv,
        "auth_notify_rail_exec_flags"_zv,
        "auth_notify_rail_exec_exe_or_file"_zv,
        "auth_command_rail_exec_exec_result"_zv,
        "auth_command_rail_exec_flags"_zv,
        "auth_command_rail_exec_original_exe_or_file"_zv,
        "auth_command_rail_exec_exe_or_file"_zv,
        "auth_command_rail_exec_working_dir"_zv,
        "auth_command_rail_exec_arguments"_zv,
        "auth_command_rail_exec_account"_zv,
        "auth_command_rail_exec_password"_zv,
        "session_probe_launch_error_message"_zv,
        "is_wabam"_zv,
        "pm_response"_zv,
        "pm_request"_zv,
        "native_session_id"_zv,
        "rd_shadow_available"_zv,
        "rd_shadow_userdata"_zv,
        "rd_shadow_type"_zv,
        "rd_shadow_invitation_error_code"_zv,
        "rd_shadow_invitation_error_message"_zv,
        "rd_shadow_invitation_id"_zv,
        "rd_shadow_invitation_addr"_zv,
        "rd_shadow_invitation_port"_zv,
        "smartcard_login"_zv,
        "banner_message"_zv,
        "banner_type"_zv,
        "replay_path"_zv,
        "replay_on_loop"_zv,
        "language"_zv,
        "login_language"_zv,
    };


    // value from connpolicy but not used by the proxy
    constexpr zstring_view const unused_connpolicy_authstr[] = {
        "mod_rdp:krb_armoring_account"_zv,
        "mod_rdp:krb_armoring_realm"_zv,
        "mod_rdp:krb_armoring_fallback_user"_zv,
        "mod_rdp:krb_armoring_fallback_password"_zv,
        "vnc_over_ssh:enable"_zv,
        "vnc_over_ssh:ssh_port"_zv,
        "vnc_over_ssh:tunneling_credential_source"_zv,
        "vnc_over_ssh:ssh_login"_zv,
        "vnc_over_ssh:ssh_password"_zv,
        "vnc_over_ssh:scenario_account_name"_zv,
        "vnc_over_ssh:tunneling_type"_zv,
    };


constexpr U64BitFlags<1> unused_connpolicy_loggable{ {
  0b0000000000000000000000000000000000000000000000000000011011110111
},
{
  0b0000000000000000000000000000000000000000000000000000000000000000
} };
} // namespace configs

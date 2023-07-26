// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN
// 

#pragma once

namespace cfg
{
    struct globals {
        struct front_connection_time;
        struct target_connection_time;
        struct auth_user;
        struct host;
        struct target;
        struct target_device;
        struct device_id;
        struct primary_user_id;
        struct target_user;
        struct target_application;
        struct target_application_account;
        struct target_application_password;
        struct port;
        struct encryptionLevel;
        struct authfile;
        struct handshake_timeout;
        struct base_inactivity_timeout;
        struct inactivity_timeout;
        struct keepalive_grace_delay;
        struct authentication_timeout;
        struct trace_type;
        struct listen_address;
        struct enable_transparent_mode;
        struct certificate_password;
        struct is_rec;
        struct enable_bitmap_update;
        struct enable_close_box;
        struct close_timeout;
        struct enable_osd;
        struct enable_osd_display_remote_target;
        struct enable_wab_integration;
        struct allow_using_multiple_monitors;
        struct allow_scale_factor;
        struct bogus_refresh_rect;
        struct large_pointer_support;
        struct new_pointer_update_support;
        struct unicode_keyboard_event_support;
        struct mod_recv_timeout;
        struct experimental_enable_serializer_data_block_size_limit;
        struct experimental_support_resize_session_during_recording;
        struct support_connection_redirection_during_recording;
        struct rdp_keepalive_connection_interval;
        struct enable_ipv6;
        struct minimal_memory_available_before_connection_silently_closed;
    };

    struct client {
        struct keyboard_layout;
        struct ignore_logon_password;
        struct performance_flags_default;
        struct performance_flags_force_present;
        struct performance_flags_force_not_present;
        struct auto_adjust_performance_flags;
        struct tls_fallback_legacy;
        struct tls_support;
        struct tls_min_level;
        struct tls_max_level;
        struct show_common_cipher_list;
        struct enable_nla;
        struct disable_tsk_switch_shortcuts;
        struct rdp_compression;
        struct max_color_depth;
        struct persistent_disk_bitmap_cache;
        struct cache_waiting_list;
        struct persist_bitmap_cache_on_disk;
        struct bitmap_compression;
        struct fast_path;
        struct enable_suppress_output;
        struct ssl_cipher_list;
        struct show_target_user_in_f12_message;
        struct bogus_ios_glyph_support_level;
        struct transform_glyph_to_bitmap;
        struct recv_timeout;
        struct enable_osd_4_eyes;
        struct enable_remotefx;
        struct disabled_orders;
    };

    struct all_target_mod {
        struct connection_establishment_timeout;
        struct tcp_user_timeout;
    };

    struct remote_program {
        struct allow_resize_hosted_desktop;
    };

    struct mod_rdp {
        struct rdp_compression;
        struct disconnect_on_logon_user_change;
        struct open_session_timeout;
        struct disabled_orders;
        struct enable_nla;
        struct enable_kerberos;
        struct tls_min_level;
        struct tls_max_level;
        struct cipher_string;
        struct show_common_cipher_list;
        struct persistent_disk_bitmap_cache;
        struct cache_waiting_list;
        struct persist_bitmap_cache_on_disk;
        struct allow_channels;
        struct deny_channels;
        struct allowed_dynamic_channels;
        struct denied_dynamic_channels;
        struct fast_path;
        struct server_redirection_support;
        struct client_address_sent;
        struct load_balance_info;
        struct proxy_managed_drives;
        struct ignore_auth_channel;
        struct auth_channel;
        struct checkout_channel;
        struct alternate_shell;
        struct shell_arguments;
        struct shell_working_directory;
        struct use_client_provided_alternate_shell;
        struct use_client_provided_remoteapp;
        struct use_native_remoteapp_capability;
        struct application_driver_exe_or_file;
        struct application_driver_script_argument;
        struct application_driver_chrome_dt_script;
        struct application_driver_chrome_uia_script;
        struct application_driver_firefox_uia_script;
        struct application_driver_ie_script;
        struct hide_client_name;
        struct use_license_store;
        struct bogus_ios_rdpdr_virtual_channel;
        struct enable_rdpdr_data_analysis;
        struct remoteapp_bypass_legal_notice_delay;
        struct remoteapp_bypass_legal_notice_timeout;
        struct log_only_relevant_clipboard_activities;
        struct experimental_fix_too_long_cookie;
        struct split_domain;
        struct wabam_uses_translated_remoteapp;
        struct session_shadowing_support;
        struct enable_remotefx;
        struct accept_monitor_layout_change_if_capture_is_not_started;
        struct enable_restricted_admin_mode;
        struct force_smartcard_authentication;
        struct enable_ipv6;
        struct mode_console;
        struct auto_reconnection_on_losing_target_link;
        struct allow_session_reconnection_by_shortcut;
        struct session_reconnection_delay;
        struct forward_client_build_number;
        struct bogus_monitor_layout_treatment;
        struct effective_krb_armoring_user;
        struct effective_krb_armoring_password;
        struct remote_programs_disconnect_message_delay;
        struct use_session_probe_to_launch_remote_program;
        struct replace_null_pointer_by_default_pointer;
    };

    struct protocol {
        struct save_session_info_pdu;
    };

    struct session_probe {
        struct enable_session_probe;
        struct exe_or_file;
        struct arguments;
        struct use_smart_launcher;
        struct enable_launch_mask;
        struct on_launch_failure;
        struct launch_timeout;
        struct launch_fallback_timeout;
        struct start_launch_timeout_timer_only_after_logon;
        struct keepalive_timeout;
        struct on_keepalive_timeout;
        struct end_disconnected_session;
        struct enable_autodeployed_appdriver_affinity;
        struct enable_log;
        struct enable_log_rotation;
        struct log_level;
        struct disconnected_application_limit;
        struct disconnected_session_limit;
        struct idle_session_limit;
        struct smart_launcher_clipboard_initialization_delay;
        struct smart_launcher_start_delay;
        struct smart_launcher_long_delay;
        struct smart_launcher_short_delay;
        struct smart_launcher_enable_wabam_affinity;
        struct launcher_abort_delay;
        struct enable_crash_dump;
        struct handle_usage_limit;
        struct memory_usage_limit;
        struct cpu_usage_alarm_threshold;
        struct cpu_usage_alarm_action;
        struct end_of_session_check_delay_time;
        struct ignore_ui_less_processes_during_end_of_session_check;
        struct extra_system_processes;
        struct childless_window_as_unidentified_input_field;
        struct windows_of_these_applications_as_unidentified_input_field;
        struct update_disabled_features;
        struct disabled_features;
        struct enable_bestsafe_interaction;
        struct on_account_manipulation;
        struct alternate_directory_environment_variable;
        struct public_session;
        struct outbound_connection_monitoring_rules;
        struct process_monitoring_rules;
        struct customize_executable_name;
        struct allow_multiple_handshake;
        struct at_end_of_session_freeze_connection_and_wait;
        struct enable_cleaner;
        struct clipboard_based_launcher_reset_keyboard_status;
        struct process_command_line_retrieve_method;
        struct periodic_task_run_interval;
        struct pause_if_session_is_disconnected;
    };

    struct server_cert {
        struct server_cert_store;
        struct server_cert_check;
        struct server_access_allowed_message;
        struct server_cert_create_message;
        struct server_cert_success_message;
        struct server_cert_failure_message;
        struct error_message;
        struct enable_external_validation;
        struct external_cert;
        struct external_response;
    };

    struct mod_vnc {
        struct clipboard_up;
        struct clipboard_down;
        struct encodings;
        struct server_clipboard_encoding_type;
        struct bogus_clipboard_infinite_loop;
        struct server_is_macos;
        struct server_unix_alt;
        struct support_cursor_pseudo_encoding;
        struct enable_ipv6;
    };

    struct session_log {
        struct enable_session_log;
        struct enable_arcsight_log;
        struct keyboard_input_masking_level;
    };

    struct ocr {
        struct version;
        struct locale;
        struct interval;
        struct on_title_bar_only;
        struct max_unrecog_char_rate;
    };

    struct capture {
        struct record_filebase;
        struct record_subdirectory;
        struct fdx_path;
        struct disable_keyboard_log;
    };

    struct video {
        struct capture_flags;
        struct png_interval;
        struct break_interval;
        struct png_limit;
        struct hash_path;
        struct record_tmp_path;
        struct record_path;
        struct disable_keyboard_log;
        struct disable_clipboard_log;
        struct disable_file_system_log;
        struct wrm_color_depth_selection_strategy;
        struct wrm_compression_algorithm;
        struct codec_id;
        struct framerate;
        struct ffmpeg_options;
        struct notimestamp;
        struct smart_video_cropping;
        struct play_video_with_corrupted_bitmap;
        struct allow_rt_without_recording;
        struct file_permissions;
    };

    struct audit {
        struct rt_display;
        struct use_redis;
        struct redis_timeout;
        struct redis_address;
        struct redis_port;
        struct redis_password;
        struct redis_db;
        struct redis_use_tls;
        struct redis_tls_cacert;
        struct redis_tls_cert;
        struct redis_tls_key;
    };

    struct file_verification {
        struct socket_path;
        struct enable_up;
        struct enable_down;
        struct clipboard_text_up;
        struct clipboard_text_down;
        struct block_invalid_file_up;
        struct block_invalid_file_down;
        struct block_invalid_clipboard_text_up;
        struct block_invalid_clipboard_text_down;
        struct log_if_accepted;
        struct max_file_size_rejected;
        struct tmpdir;
    };

    struct file_storage {
        struct store_file;
    };

    struct crypto {
        struct encryption_key;
        struct sign_key;
    };

    struct websocket {
        struct enable_websocket;
        struct use_tls;
        struct listen_address;
    };

    struct context {
        struct psid;
        struct opt_bpp;
        struct opt_height;
        struct opt_width;
        struct auth_error_message;
        struct selector;
        struct selector_current_page;
        struct selector_device_filter;
        struct selector_group_filter;
        struct selector_proto_filter;
        struct selector_lines_per_page;
        struct selector_number_of_pages;
        struct target_password;
        struct target_host;
        struct tunneling_target_host;
        struct target_str;
        struct target_service;
        struct target_port;
        struct target_protocol;
        struct password;
        struct reporting;
        struct auth_channel_answer;
        struct auth_channel_target;
        struct message;
        struct display_link;
        struct mod_timeout;
        struct accept_message;
        struct display_message;
        struct rejected;
        struct keepalive;
        struct session_id;
        struct end_date_cnx;
        struct real_target_device;
        struct authentication_challenge;
        struct ticket;
        struct comment;
        struct duration;
        struct duration_max;
        struct waitinforeturn;
        struct showform;
        struct formflag;
        struct module;
        struct try_alternate_target;
        struct has_more_target;
        struct proxy_opt;
        struct pattern_kill;
        struct pattern_notify;
        struct opt_message;
        struct login_message;
        struct disconnect_reason;
        struct disconnect_reason_ack;
        struct ip_target;
        struct recording_started;
        struct rt_ready;
        struct sharing_ready;
        struct auth_command;
        struct auth_notify;
        struct auth_notify_rail_exec_flags;
        struct auth_notify_rail_exec_exe_or_file;
        struct auth_command_rail_exec_exec_result;
        struct auth_command_rail_exec_flags;
        struct auth_command_rail_exec_original_exe_or_file;
        struct auth_command_rail_exec_exe_or_file;
        struct auth_command_rail_exec_working_dir;
        struct auth_command_rail_exec_arguments;
        struct auth_command_rail_exec_account;
        struct auth_command_rail_exec_password;
        struct session_probe_launch_error_message;
        struct close_box_extra_message;
        struct is_wabam;
        struct pm_response;
        struct pm_request;
        struct native_session_id;
        struct rd_shadow_available;
        struct rd_shadow_userdata;
        struct rd_shadow_type;
        struct rd_shadow_invitation_error_code;
        struct rd_shadow_invitation_error_message;
        struct rd_shadow_invitation_id;
        struct rd_shadow_invitation_addr;
        struct rd_shadow_invitation_port;
        struct session_sharing_userdata;
        struct session_sharing_enable_control;
        struct session_sharing_ttl;
        struct session_sharing_invitation_error_code;
        struct session_sharing_invitation_error_message;
        struct session_sharing_invitation_id;
        struct session_sharing_invitation_addr;
        struct session_sharing_target_ip;
        struct session_sharing_target_login;
        struct rail_module_host_mod_is_active;
        struct smartcard_login;
        struct banner_message;
        struct banner_type;
        struct redirection_password_or_cookie;
    };

    struct internal_mod {
        struct enable_target_field;
        struct keyboard_layout_proposals;
    };

    struct mod_replay {
        struct replay_path;
        struct on_end_of_data;
        struct replay_on_loop;
    };

    struct translation {
        struct language;
        struct login_language;
    };

    struct theme {
        struct enable_theme;
        struct logo_path;
        struct bgcolor;
        struct fgcolor;
        struct separator_color;
        struct focus_color;
        struct error_color;
        struct edit_bgcolor;
        struct edit_fgcolor;
        struct edit_focus_color;
        struct tooltip_bgcolor;
        struct tooltip_fgcolor;
        struct tooltip_border_color;
        struct selector_line1_bgcolor;
        struct selector_line1_fgcolor;
        struct selector_line2_bgcolor;
        struct selector_line2_fgcolor;
        struct selector_selected_bgcolor;
        struct selector_selected_fgcolor;
        struct selector_focus_bgcolor;
        struct selector_focus_fgcolor;
        struct selector_label_bgcolor;
        struct selector_label_fgcolor;
    };

    struct debug {
        struct fake_target_ip;
        struct capture;
        struct auth;
        struct session;
        struct front;
        struct mod_rdp;
        struct mod_vnc;
        struct mod_internal;
        struct sck_mod;
        struct sck_front;
        struct password;
        struct compression;
        struct cache;
        struct ocr;
        struct ffmpeg;
        struct config;
        struct mod_rdp_use_failure_simulation_socket_transport;
        struct probe_client_addresses;
    };

} // namespace cfg

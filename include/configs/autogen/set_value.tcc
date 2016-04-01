//
// ATTENTION -- This file is auto-generated
//

inline void Inifile::ConfigurationHolder::set_value(const char * context, const char * key, const char * value) {
    if (0) {}
    else if (0 == strcmp(context, "client")) {
        if (0) {}
        else if (0 == strcmp(key, "keyboard_layout_proposals")) {
            ::configs::parse(static_cast<cfg::client::keyboard_layout_proposals&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "ignore_logon_password")) {
            ::configs::parse(static_cast<cfg::client::ignore_logon_password&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "performance_flags_default")) {
            ::configs::parse(static_cast<cfg::client::performance_flags_default&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "performance_flags_force_present")) {
            ::configs::parse(static_cast<cfg::client::performance_flags_force_present&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "performance_flags_force_not_present")) {
            ::configs::parse(static_cast<cfg::client::performance_flags_force_not_present&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "tls_fallback_legacy")) {
            ::configs::parse(static_cast<cfg::client::tls_fallback_legacy&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "tls_support")) {
            ::configs::parse(static_cast<cfg::client::tls_support&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bogus_neg_request")) {
            ::configs::parse(static_cast<cfg::client::bogus_neg_request&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bogus_user_id")) {
            ::configs::parse(static_cast<cfg::client::bogus_user_id&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disable_tsk_switch_shortcuts")) {
            ::configs::parse(static_cast<cfg::client::disable_tsk_switch_shortcuts&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "rdp_compression")) {
            ::configs::parse(static_cast<cfg::client::rdp_compression&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "max_color_depth")) {
            ::configs::parse(static_cast<cfg::client::max_color_depth&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
            ::configs::parse(static_cast<cfg::client::persistent_disk_bitmap_cache&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "cache_waiting_list")) {
            ::configs::parse(static_cast<cfg::client::cache_waiting_list&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
            ::configs::parse(static_cast<cfg::client::persist_bitmap_cache_on_disk&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bitmap_compression")) {
            ::configs::parse(static_cast<cfg::client::bitmap_compression&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "fast_path")) {
            ::configs::parse(static_cast<cfg::client::fast_path&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "crypto")) {
        if (0) {}
        else if (0 == strcmp(key, "encryption_key")) {
            ::configs::parse(static_cast<cfg::crypto::key0&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "sign_key")) {
            ::configs::parse(static_cast<cfg::crypto::key1&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "debug")) {
        if (0) {}
        else if (0 == strcmp(key, "x224")) {
            ::configs::parse(static_cast<cfg::debug::x224&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "mcs")) {
            ::configs::parse(static_cast<cfg::debug::mcs&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "sec")) {
            ::configs::parse(static_cast<cfg::debug::sec&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "rdp")) {
            ::configs::parse(static_cast<cfg::debug::rdp&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "primary_orders")) {
            ::configs::parse(static_cast<cfg::debug::primary_orders&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "secondary_orders")) {
            ::configs::parse(static_cast<cfg::debug::secondary_orders&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bitmap")) {
            ::configs::parse(static_cast<cfg::debug::bitmap&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "capture")) {
            ::configs::parse(static_cast<cfg::debug::capture&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "auth")) {
            ::configs::parse(static_cast<cfg::debug::auth&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session")) {
            ::configs::parse(static_cast<cfg::debug::session&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "front")) {
            ::configs::parse(static_cast<cfg::debug::front&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "mod_rdp")) {
            ::configs::parse(static_cast<cfg::debug::mod_rdp&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "mod_vnc")) {
            ::configs::parse(static_cast<cfg::debug::mod_vnc&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "mod_int")) {
            ::configs::parse(static_cast<cfg::debug::mod_int&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "mod_xup")) {
            ::configs::parse(static_cast<cfg::debug::mod_xup&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "widget")) {
            ::configs::parse(static_cast<cfg::debug::widget&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "input")) {
            ::configs::parse(static_cast<cfg::debug::input&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "password")) {
            ::configs::parse(static_cast<cfg::debug::password&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "compression")) {
            ::configs::parse(static_cast<cfg::debug::compression&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "cache")) {
            ::configs::parse(static_cast<cfg::debug::cache&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bitmap_update")) {
            ::configs::parse(static_cast<cfg::debug::bitmap_update&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "performance")) {
            ::configs::parse(static_cast<cfg::debug::performance&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "pass_dialog_box")) {
            ::configs::parse(static_cast<cfg::debug::pass_dialog_box&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "config")) {
            ::configs::parse(static_cast<cfg::debug::config&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "globals")) {
        if (0) {}
        else if (0 == strcmp(key, "bitmap_cache")) {
            ::configs::parse(static_cast<cfg::globals::bitmap_cache&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "glyph_cache")) {
            ::configs::parse(static_cast<cfg::globals::glyph_cache&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "port")) {
            ::configs::parse(static_cast<cfg::globals::port&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "nomouse")) {
            ::configs::parse(static_cast<cfg::globals::nomouse&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "notimestamp")) {
            ::configs::parse(static_cast<cfg::globals::notimestamp&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "encryptionLevel")) {
            ::configs::parse(static_cast<cfg::globals::encryptionLevel&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "authfile")) {
            ::configs::parse(static_cast<cfg::globals::authfile&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "handshake_timeout")) {
            ::configs::parse(static_cast<cfg::globals::handshake_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_timeout")) {
            ::configs::parse(static_cast<cfg::globals::session_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "keepalive_grace_delay")) {
            ::configs::parse(static_cast<cfg::globals::keepalive_grace_delay&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "authentication_timeout")) {
            ::configs::parse(static_cast<cfg::globals::authentication_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "close_timeout")) {
            ::configs::parse(static_cast<cfg::globals::close_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "trace_type")) {
            ::configs::parse(static_cast<cfg::globals::trace_type&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "listen_address")) {
            ::configs::parse(static_cast<cfg::globals::listen_address&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_ip_transparent")) {
            ::configs::parse(static_cast<cfg::globals::enable_ip_transparent&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "certificate_password")) {
            ::configs::parse(static_cast<cfg::globals::certificate_password&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "png_path")) {
            ::configs::parse(static_cast<cfg::globals::png_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "wrm_path")) {
            ::configs::parse(static_cast<cfg::globals::wrm_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "movie")) {
            ::configs::parse(static_cast<cfg::globals::movie&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "movie_path")) {
            ::configs::parse(static_cast<cfg::globals::movie_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_bitmap_update")) {
            ::configs::parse(static_cast<cfg::globals::enable_bitmap_update&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_close_box")) {
            ::configs::parse(static_cast<cfg::globals::enable_close_box&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_osd")) {
            ::configs::parse(static_cast<cfg::globals::enable_osd&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_osd_display_remote_target")) {
            ::configs::parse(static_cast<cfg::globals::enable_osd_display_remote_target&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "persistent_path")) {
            ::configs::parse(static_cast<cfg::globals::persistent_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disable_proxy_opt")) {
            ::configs::parse(static_cast<cfg::globals::disable_proxy_opt&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "internal_mod")) {
        if (0) {}
        else if (0 == strcmp(key, "load_theme")) {
            ::configs::parse(static_cast<cfg::internal_mod::theme&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_rdp")) {
        if (0) {}
        else if (0 == strcmp(key, "rdp_compression")) {
            ::configs::parse(static_cast<cfg::mod_rdp::rdp_compression&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disconnect_on_logon_user_change")) {
            ::configs::parse(static_cast<cfg::mod_rdp::disconnect_on_logon_user_change&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "open_session_timeout")) {
            ::configs::parse(static_cast<cfg::mod_rdp::open_session_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "extra_orders")) {
            ::configs::parse(static_cast<cfg::mod_rdp::extra_orders&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_nla")) {
            ::configs::parse(static_cast<cfg::mod_rdp::enable_nla&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_kerberos")) {
            ::configs::parse(static_cast<cfg::mod_rdp::enable_kerberos&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
            ::configs::parse(static_cast<cfg::mod_rdp::persistent_disk_bitmap_cache&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "cache_waiting_list")) {
            ::configs::parse(static_cast<cfg::mod_rdp::cache_waiting_list&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
            ::configs::parse(static_cast<cfg::mod_rdp::persist_bitmap_cache_on_disk&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "allow_channels")) {
            ::configs::parse(static_cast<cfg::mod_rdp::allow_channels&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "deny_channels")) {
            ::configs::parse(static_cast<cfg::mod_rdp::deny_channels&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "fast_path")) {
            ::configs::parse(static_cast<cfg::mod_rdp::fast_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_redirection_support")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_redirection_support&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bogus_sc_net_size")) {
            ::configs::parse(static_cast<cfg::mod_rdp::bogus_sc_net_size&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "proxy_managed_drives")) {
            ::configs::parse(static_cast<cfg::mod_rdp::proxy_managed_drives&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "ignore_auth_channel")) {
            ::configs::parse(static_cast<cfg::mod_rdp::ignore_auth_channel&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "auth_channel")) {
            ::configs::parse(static_cast<cfg::mod_rdp::auth_channel&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "alternate_shell")) {
            ::configs::parse(static_cast<cfg::mod_rdp::alternate_shell&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "shell_working_directory")) {
            ::configs::parse(static_cast<cfg::mod_rdp::shell_working_directory&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "use_client_provided_alternate_shell")) {
            ::configs::parse(static_cast<cfg::mod_rdp::use_client_provided_alternate_shell&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_session_probe")) {
            ::configs::parse(static_cast<cfg::mod_rdp::enable_session_probe&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_use_smart_launcher")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_use_clipboard_based_launcher&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "enable_session_probe_launch_mask")) {
            ::configs::parse(static_cast<cfg::mod_rdp::enable_session_probe_launch_mask&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_on_launch_failure")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_on_launch_failure&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_launch_timeout")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_launch_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_launch_fallback_timeout")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_launch_fallback_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_start_launch_timeout_timer_only_after_logon")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_start_launch_timeout_timer_only_after_logon&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_keepalive_timeout")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_keepalive_timeout&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_on_keepalive_timeout_disconnect_user")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_on_keepalive_timeout_disconnect_user&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_end_disconnected_session")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_end_disconnected_session&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_customize_executable_name")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_customize_executable_name&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "session_probe_alternate_shell")) {
            ::configs::parse(static_cast<cfg::mod_rdp::session_probe_alternate_shell&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_store")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_store&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_check")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_check&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_access_allowed_message")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_access_allowed_message&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_create_message")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_create_message&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_success_message")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_success_message&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_failure_message")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_failure_message&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_cert_error_message")) {
            ::configs::parse(static_cast<cfg::mod_rdp::server_cert_error_message&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "hide_client_name")) {
            ::configs::parse(static_cast<cfg::mod_rdp::hide_client_name&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_replay")) {
        if (0) {}
        else if (0 == strcmp(key, "on_end_of_data")) {
            ::configs::parse(static_cast<cfg::mod_replay::on_end_of_data&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_vnc")) {
        if (0) {}
        else if (0 == strcmp(key, "clipboard_up")) {
            ::configs::parse(static_cast<cfg::mod_vnc::clipboard_up&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "clipboard_down")) {
            ::configs::parse(static_cast<cfg::mod_vnc::clipboard_down&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "encodings")) {
            ::configs::parse(static_cast<cfg::mod_vnc::encodings&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "allow_authentification_retries")) {
            ::configs::parse(static_cast<cfg::mod_vnc::allow_authentification_retries&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "server_clipboard_encoding_type")) {
            ::configs::parse(static_cast<cfg::mod_vnc::server_clipboard_encoding_type&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "bogus_clipboard_infinite_loop")) {
            ::configs::parse(static_cast<cfg::mod_vnc::bogus_clipboard_infinite_loop&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "session_log")) {
        if (0) {}
        else if (0 == strcmp(key, "enable_session_log")) {
            ::configs::parse(static_cast<cfg::session_log::enable_session_log&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "keyboard_input_masking_level")) {
            ::configs::parse(static_cast<cfg::session_log::keyboard_input_masking_level&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "translation")) {
        if (0) {}
        else if (0 == strcmp(key, "language")) {
            ::configs::parse(static_cast<cfg::translation::language&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "password_en")) {
            ::configs::parse(static_cast<cfg::translation::password_en&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "password_fr")) {
            ::configs::parse(static_cast<cfg::translation::password_fr&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "video")) {
        if (0) {}
        else if (0 == strcmp(key, "capture_groupid")) {
            ::configs::parse(static_cast<cfg::video::capture_groupid&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "capture_flags")) {
            ::configs::parse(static_cast<cfg::video::capture_flags&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "png_interval")) {
            ::configs::parse(static_cast<cfg::video::png_interval&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "frame_interval")) {
            ::configs::parse(static_cast<cfg::video::frame_interval&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "break_interval")) {
            ::configs::parse(static_cast<cfg::video::break_interval&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "png_limit")) {
            ::configs::parse(static_cast<cfg::video::png_limit&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "replay_path")) {
            ::configs::parse(static_cast<cfg::video::replay_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "hash_path")) {
            ::configs::parse(static_cast<cfg::video::hash_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "record_tmp_path")) {
            ::configs::parse(static_cast<cfg::video::record_tmp_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "record_path")) {
            ::configs::parse(static_cast<cfg::video::record_path&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disable_keyboard_log")) {
            ::configs::parse(static_cast<cfg::video::disable_keyboard_log&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disable_clipboard_log")) {
            ::configs::parse(static_cast<cfg::video::disable_clipboard_log&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "disable_file_system_log")) {
            ::configs::parse(static_cast<cfg::video::disable_file_system_log&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "rt_display")) {
            ::configs::parse(static_cast<cfg::video::rt_display&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "wrm_color_depth_selection_strategy")) {
            ::configs::parse(static_cast<cfg::video::wrm_color_depth_selection_strategy&>(this->variables).value, value);
        }
        else if (0 == strcmp(key, "wrm_compression_algorithm")) {
            ::configs::parse(static_cast<cfg::video::wrm_compression_algorithm&>(this->variables).value, value);
        }

        else if (static_cast<cfg::debug::config>(this->variables).value) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (static_cast<cfg::debug::config>(this->variables).value) {
        LOG(LOG_ERR, "unknown section [%s]", context);
    }
}

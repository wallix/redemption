void Inifile::set_value(const char * context, const char * key, const char * value) {
    if (0) {}
    else if (0 == strcmp(context, "client")) {
        if (0) {}
        else if (0 == strcmp(key, "ignore_logon_password")) {
            parse(this->client.ignore_logon_password, value);
        }
        else if (0 == strcmp(key, "performance_flags_default")) {
            parse(this->client.performance_flags_default, value);
        }
        else if (0 == strcmp(key, "performance_flags_force_present")) {
            parse(this->client.performance_flags_force_present, value);
        }
        else if (0 == strcmp(key, "performance_flags_force_not_present")) {
            parse(this->client.performance_flags_force_not_present, value);
        }
        else if (0 == strcmp(key, "tls_fallback_legacy")) {
            parse(this->client.tls_fallback_legacy, value);
        }
        else if (0 == strcmp(key, "tls_support")) {
            parse(this->client.tls_support, value);
        }
        else if (0 == strcmp(key, "bogus_neg_request")) {
            parse(this->client.bogus_neg_request, value);
        }
        else if (0 == strcmp(key, "bogus_user_id")) {
            parse(this->client.bogus_user_id, value);
        }
        else if (0 == strcmp(key, "disable_tsk_switch_shortcuts")) {
            parse(this->client.disable_tsk_switch_shortcuts, value);
        }
        else if (0 == strcmp(key, "rdp_compression")) {
            parse(this->client.rdp_compression, value);
        }
        else if (0 == strcmp(key, "max_color_depth")) {
            parse(this->client.max_color_depth, value);
        }
        else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
            parse(this->client.persistent_disk_bitmap_cache, value);
        }
        else if (0 == strcmp(key, "cache_waiting_list")) {
            parse(this->client.cache_waiting_list, value);
        }
        else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
            parse(this->client.persist_bitmap_cache_on_disk, value);
        }
        else if (0 == strcmp(key, "bitmap_compression")) {
            parse(this->client.bitmap_compression, value);
        }
        else if (0 == strcmp(key, "fast_path")) {
            parse(this->client.fast_path, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "crypto")) {
        if (0) {}
        else if (0 == strcmp(key, "key0")) {
            parse(this->crypto.key0, value);
        }
        else if (0 == strcmp(key, "key1")) {
            parse(this->crypto.key1, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "debug")) {
        if (0) {}
        else if (0 == strcmp(key, "x224")) {
            parse(this->debug.x224, value);
        }
        else if (0 == strcmp(key, "mcs")) {
            parse(this->debug.mcs, value);
        }
        else if (0 == strcmp(key, "sec")) {
            parse(this->debug.sec, value);
        }
        else if (0 == strcmp(key, "rdp")) {
            parse(this->debug.rdp, value);
        }
        else if (0 == strcmp(key, "primary_orders")) {
            parse(this->debug.primary_orders, value);
        }
        else if (0 == strcmp(key, "secondary_orders")) {
            parse(this->debug.secondary_orders, value);
        }
        else if (0 == strcmp(key, "bitmap")) {
            parse(this->debug.bitmap, value);
        }
        else if (0 == strcmp(key, "capture")) {
            parse(this->debug.capture, value);
        }
        else if (0 == strcmp(key, "auth")) {
            parse(this->debug.auth, value);
        }
        else if (0 == strcmp(key, "session")) {
            parse(this->debug.session, value);
        }
        else if (0 == strcmp(key, "front")) {
            parse(this->debug.front, value);
        }
        else if (0 == strcmp(key, "mod_rdp")) {
            parse(this->debug.mod_rdp, value);
        }
        else if (0 == strcmp(key, "mod_vnc")) {
            parse(this->debug.mod_vnc, value);
        }
        else if (0 == strcmp(key, "mod_int")) {
            parse(this->debug.mod_int, value);
        }
        else if (0 == strcmp(key, "mod_xup")) {
            parse(this->debug.mod_xup, value);
        }
        else if (0 == strcmp(key, "widget")) {
            parse(this->debug.widget, value);
        }
        else if (0 == strcmp(key, "input")) {
            parse(this->debug.input, value);
        }
        else if (0 == strcmp(key, "password")) {
            parse(this->debug.password, value);
        }
        else if (0 == strcmp(key, "compression")) {
            parse(this->debug.compression, value);
        }
        else if (0 == strcmp(key, "cache")) {
            parse(this->debug.cache, value);
        }
        else if (0 == strcmp(key, "bitmap_update")) {
            parse(this->debug.bitmap_update, value);
        }
        else if (0 == strcmp(key, "performance")) {
            parse(this->debug.performance, value);
        }
        else if (0 == strcmp(key, "pass_dialog_box")) {
            parse(this->debug.pass_dialog_box, value);
        }
        else if (0 == strcmp(key, "config")) {
            parse(this->debug.config, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "globals")) {
        if (0) {}
        else if (0 == strcmp(key, "bitmap_cache")) {
            parse(this->globals.bitmap_cache, value);
        }
        else if (0 == strcmp(key, "glyph_cache")) {
            parse(this->globals.glyph_cache, value);
        }
        else if (0 == strcmp(key, "port")) {
            parse(this->globals.port, value);
        }
        else if (0 == strcmp(key, "nomouse")) {
            parse(this->globals.nomouse, value);
        }
        else if (0 == strcmp(key, "notimestamp")) {
            parse(this->globals.notimestamp, value);
        }
        else if (0 == strcmp(key, "encryptionLevel")) {
            parse(this->globals.encryptionLevel, value);
        }
        else if (0 == strcmp(key, "authip")) {
            parse(this->globals.authip, value);
        }
        else if (0 == strcmp(key, "authport")) {
            parse(this->globals.authport, value);
        }
        else if (0 == strcmp(key, "session_timeout")) {
            parse(this->globals.session_timeout, value);
        }
        else if (0 == strcmp(key, "keepalive_grace_delay")) {
            parse(this->globals.keepalive_grace_delay, value);
        }
        else if (0 == strcmp(key, "close_timeout")) {
            parse(this->globals.close_timeout, value);
        }
        else if (0 == strcmp(key, "auth_channel")) {
            parse(this->globals.auth_channel, value);
        }
        else if (0 == strcmp(key, "enable_file_encryption")) {
            parse(this->globals.enable_file_encryption, value);
        }
        else if (0 == strcmp(key, "listen_address")) {
            parse(this->globals.listen_address, value);
        }
        else if (0 == strcmp(key, "enable_ip_transparent")) {
            parse(this->globals.enable_ip_transparent, value);
        }
        else if (0 == strcmp(key, "certificate_password")) {
            parse(this->globals.certificate_password, value);
        }
        else if (0 == strcmp(key, "png_path")) {
            parse(this->globals.png_path, value);
        }
        else if (0 == strcmp(key, "wrm_path")) {
            parse(this->globals.wrm_path, value);
        }
        else if (0 == strcmp(key, "alternate_shell")) {
            parse(this->globals.alternate_shell, value);
        }
        else if (0 == strcmp(key, "shell_working_directory")) {
            parse(this->globals.shell_working_directory, value);
        }
        else if (0 == strcmp(key, "codec_id")) {
            parse(this->globals.codec_id, value);
        }
        else if (0 == strcmp(key, "movie")) {
            parse(this->globals.movie, value);
        }
        else if (0 == strcmp(key, "movie_path")) {
            parse(this->globals.movie_path, value);
        }
        else if (0 == strcmp(key, "video_quality")) {
            parse(this->globals.video_quality, value);
        }
        else if (0 == strcmp(key, "enable_bitmap_update")) {
            parse(this->globals.enable_bitmap_update, value);
        }
        else if (0 == strcmp(key, "enable_close_box")) {
            parse(this->globals.enable_close_box, value);
        }
        else if (0 == strcmp(key, "enable_osd")) {
            parse(this->globals.enable_osd, value);
        }
        else if (0 == strcmp(key, "enable_osd_display_remote_target")) {
            parse(this->globals.enable_osd_display_remote_target, value);
        }
        else if (0 == strcmp(key, "enable_wab_agent")) {
            parse(this->globals.enable_wab_agent, value);
        }
        else if (0 == strcmp(key, "wab_agent_launch_timeout")) {
            parse(this->globals.wab_agent_launch_timeout, value);
        }
        else if (0 == strcmp(key, "wab_agent_keepalive_timeout")) {
            parse(this->globals.wab_agent_keepalive_timeout, value);
        }
        else if (0 == strcmp(key, "persistent_path")) {
            parse(this->globals.persistent_path, value);
        }
        else if (0 == strcmp(key, "disable_proxy_opt")) {
            parse(this->globals.disable_proxy_opt, value);
        }
        else if (0 == strcmp(key, "max_chunked_virtual_channel_data_length")) {
            parse(this->globals.max_chunked_virtual_channel_data_length, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "internal_mod")) {
        if (0) {}
        else if (0 == strcmp(key, "load_theme")) {
            parse(this->internal_mod.theme, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_rdp")) {
        if (0) {}
        else if (0 == strcmp(key, "rdp_compression")) {
            parse(this->mod_rdp.rdp_compression, value);
        }
        else if (0 == strcmp(key, "disconnect_on_logon_user_change")) {
            parse(this->mod_rdp.disconnect_on_logon_user_change, value);
        }
        else if (0 == strcmp(key, "open_session_timeout")) {
            parse(this->mod_rdp.open_session_timeout, value);
        }
        else if (0 == strcmp(key, "certificate_change_action")) {
            parse(this->mod_rdp.certificate_change_action, value);
        }
        else if (0 == strcmp(key, "extra_orders")) {
            parse(this->mod_rdp.extra_orders, value);
        }
        else if (0 == strcmp(key, "enable_nla")) {
            parse(this->mod_rdp.enable_nla, value);
        }
        else if (0 == strcmp(key, "enable_kerberos")) {
            parse(this->mod_rdp.enable_kerberos, value);
        }
        else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
            parse(this->mod_rdp.persistent_disk_bitmap_cache, value);
        }
        else if (0 == strcmp(key, "cache_waiting_list")) {
            parse(this->mod_rdp.cache_waiting_list, value);
        }
        else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
            parse(this->mod_rdp.persist_bitmap_cache_on_disk, value);
        }
        else if (0 == strcmp(key, "allow_channels")) {
            parse(this->mod_rdp.allow_channels, value);
        }
        else if (0 == strcmp(key, "deny_channels")) {
            parse(this->mod_rdp.deny_channels, value);
        }
        else if (0 == strcmp(key, "fast_path")) {
            parse(this->mod_rdp.fast_path, value);
        }
        else if (0 == strcmp(key, "server_redirection_support")) {
            parse(this->mod_rdp.server_redirection_support, value);
        }
        else if (0 == strcmp(key, "bogus_sc_net_size")) {
            parse(this->mod_rdp.bogus_sc_net_size, value);
        }
        else if (0 == strcmp(key, "client_device_announce_timeout")) {
            parse(this->mod_rdp.client_device_announce_timeout, value);
        }
        else if (0 == strcmp(key, "proxy_managed_drives")) {
            parse(this->mod_rdp.proxy_managed_drives, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_replay")) {
        if (0) {}
        else if (0 == strcmp(key, "on_end_of_data")) {
            parse(this->mod_replay.on_end_of_data, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "mod_vnc")) {
        if (0) {}
        else if (0 == strcmp(key, "clipboard_up")) {
            parse(this->mod_vnc.clipboard_up, value);
        }
        else if (0 == strcmp(key, "clipboard_down")) {
            parse(this->mod_vnc.clipboard_down, value);
        }
        else if (0 == strcmp(key, "encodings")) {
            parse(this->mod_vnc.encodings, value);
        }
        else if (0 == strcmp(key, "allow_authentification_retries")) {
            parse(this->mod_vnc.allow_authentification_retries, value);
        }
        else if (0 == strcmp(key, "server_clipboard_encoding_type")) {
            parse(this->mod_vnc.server_clipboard_encoding_type, value);
        }
        else if (0 == strcmp(key, "bogus_clipboard_infinite_loop")) {
            parse(this->mod_vnc.bogus_clipboard_infinite_loop, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "translation")) {
        if (0) {}
        else if (0 == strcmp(key, "language")) {
            parse(this->translation.language, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (0 == strcmp(context, "video")) {
        if (0) {}
        else if (0 == strcmp(key, "capture_groupid")) {
            parse(this->video.capture_groupid, value);
        }
        else if (0 == strcmp(key, "capture_flags")) {
            parse(this->video.capture_flags, value);
        }
        else if (0 == strcmp(key, "ocr_locale")) {
            parse(this->video.ocr_locale, value);
        }
        else if (0 == strcmp(key, "ocr_interval")) {
            parse(this->video.ocr_interval, value);
        }
        else if (0 == strcmp(key, "ocr_on_title_bar_only")) {
            parse(this->video.ocr_on_title_bar_only, value);
        }
        else if (0 == strcmp(key, "ocr_max_unrecog_char_rate")) {
            parse(this->video.ocr_max_unrecog_char_rate, value);
        }
        else if (0 == strcmp(key, "png_interval")) {
            parse(this->video.png_interval, value);
        }
        else if (0 == strcmp(key, "frame_interval")) {
            parse(this->video.frame_interval, value);
        }
        else if (0 == strcmp(key, "break_interval")) {
            parse(this->video.break_interval, value);
        }
        else if (0 == strcmp(key, "png_limit")) {
            parse(this->video.png_limit, value);
        }
        else if (0 == strcmp(key, "replay_path")) {
            parse(this->video.replay_path, value);
        }
        else if (0 == strcmp(key, "l_bitrate")) {
            parse(this->video.l_bitrate, value);
        }
        else if (0 == strcmp(key, "l_framerate")) {
            parse(this->video.l_framerate, value);
        }
        else if (0 == strcmp(key, "l_height")) {
            parse(this->video.l_height, value);
        }
        else if (0 == strcmp(key, "l_width")) {
            parse(this->video.l_width, value);
        }
        else if (0 == strcmp(key, "l_qscale")) {
            parse(this->video.l_qscale, value);
        }
        else if (0 == strcmp(key, "m_bitrate")) {
            parse(this->video.m_bitrate, value);
        }
        else if (0 == strcmp(key, "m_framerate")) {
            parse(this->video.m_framerate, value);
        }
        else if (0 == strcmp(key, "m_height")) {
            parse(this->video.m_height, value);
        }
        else if (0 == strcmp(key, "m_width")) {
            parse(this->video.m_width, value);
        }
        else if (0 == strcmp(key, "m_qscale")) {
            parse(this->video.m_qscale, value);
        }
        else if (0 == strcmp(key, "h_bitrate")) {
            parse(this->video.h_bitrate, value);
        }
        else if (0 == strcmp(key, "h_framerate")) {
            parse(this->video.h_framerate, value);
        }
        else if (0 == strcmp(key, "h_height")) {
            parse(this->video.h_height, value);
        }
        else if (0 == strcmp(key, "h_width")) {
            parse(this->video.h_width, value);
        }
        else if (0 == strcmp(key, "h_qscale")) {
            parse(this->video.h_qscale, value);
        }
        else if (0 == strcmp(key, "hash_path")) {
            parse(this->video.hash_path, value);
        }
        else if (0 == strcmp(key, "record_tmp_path")) {
            parse(this->video.record_tmp_path, value);
        }
        else if (0 == strcmp(key, "record_path")) {
            parse(this->video.record_path, value);
        }
        else if (0 == strcmp(key, "disable_keyboard_log")) {
            parse(this->video.disable_keyboard_log, value);
        }
        else if (0 == strcmp(key, "disable_clipboard_log")) {
            parse(this->video.disable_clipboard_log, value);
        }
        else if (0 == strcmp(key, "rt_display")) {
            parse(this->video.rt_display, value);
        }
        else if (0 == strcmp(key, "wrm_color_depth_selection_strategy")) {
            parse(this->video.wrm_color_depth_selection_strategy, value);
        }
        else if (0 == strcmp(key, "wrm_compression_algorithm")) {
            parse(this->video.wrm_compression_algorithm, value);
        }

        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
        }
    }
    else if (this->debug.config) {
        LOG(LOG_ERR, "unknown section [%s]", context);
    }
}

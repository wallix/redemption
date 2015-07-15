inline void Inifile::initialize() {

    this->to_send_set.insert(AUTHID_AUTH_USER);
    this->globals.auth_user.attach_ini(this, AUTHID_AUTH_USER);
    this->globals.auth_user.ask();
    this->to_send_set.insert(AUTHID_HOST);
    this->globals.host.attach_ini(this, AUTHID_HOST);
    this->to_send_set.insert(AUTHID_TARGET);
    this->globals.target.attach_ini(this, AUTHID_TARGET);
    this->to_send_set.insert(AUTHID_TARGET_DEVICE);
    this->globals.target_device.attach_ini(this, AUTHID_TARGET_DEVICE);
    this->globals.target_device.ask();
    this->to_send_set.insert(AUTHID_TARGET_USER);
    this->globals.target_user.attach_ini(this, AUTHID_TARGET_USER);
    this->globals.target_user.ask();
    this->globals.target_application.attach_ini(this, AUTHID_TARGET_APPLICATION);
    this->globals.target_application_account.attach_ini(this, AUTHID_TARGET_APPLICATION_ACCOUNT);
    this->globals.target_application_password.attach_ini(this, AUTHID_TARGET_APPLICATION_PASSWORD);



    this->globals.enable_file_encryption.attach_ini(this, AUTHID_OPT_FILE_ENCRYPTION);


    this->globals.alternate_shell.attach_ini(this, AUTHID_ALTERNATE_SHELL);
    this->globals.shell_working_directory.attach_ini(this, AUTHID_SHELL_WORKING_DIRECTORY);

    this->globals.codec_id.attach_ini(this, AUTHID_OPT_CODEC_ID);
    this->globals.codec_id.set("flv");
    this->globals.movie.attach_ini(this, AUTHID_OPT_MOVIE);
    this->globals.movie.set(0);
    this->globals.movie_path.attach_ini(this, AUTHID_OPT_MOVIE_PATH);
    this->globals.video_quality.attach_ini(this, AUTHID_VIDEO_QUALITY);
    this->globals.video_quality.set(static_cast<Level>(1));


    this->globals.enable_wab_agent.attach_ini(this, AUTHID_OPT_WABAGENT);
    this->globals.enable_wab_agent.set(0);
    this->globals.wab_agent_launch_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT);
    this->globals.wab_agent_launch_timeout.set(0);
    this->globals.wab_agent_keepalive_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT);
    this->globals.wab_agent_keepalive_timeout.set(0);



    this->client.keyboard_layout.attach_ini(this, AUTHID_KEYBOARD_LAYOUT);
    this->client.keyboard_layout.set(0);
    this->to_send_set.insert(AUTHID_KEYBOARD_LAYOUT);



    this->client.disable_tsk_switch_shortcuts.attach_ini(this, AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS);
    this->client.disable_tsk_switch_shortcuts.set(0);
















    this->mod_rdp.bogus_sc_net_size.attach_ini(this, AUTHID_RDP_BOGUS_SC_NET_SIZE);
    this->mod_rdp.bogus_sc_net_size.set(1);

    this->mod_rdp.client_device_announce_timeout.attach_ini(this, AUTHID_OPT_CLIENT_DEVICE_ANNOUNCE_TIMEOUT);
    this->mod_rdp.client_device_announce_timeout.set(1000);

    this->mod_rdp.proxy_managed_drives.attach_ini(this, AUTHID_OPT_PROXY_MANAGED_DRIVES);
    this->mod_vnc.clipboard_up.attach_ini(this, AUTHID_VNC_CLIPBOARD_UP);
    this->mod_vnc.clipboard_down.attach_ini(this, AUTHID_VNC_CLIPBOARD_DOWN);



    this->mod_vnc.server_clipboard_encoding_type.attach_ini(this, AUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE);
    this->mod_vnc.server_clipboard_encoding_type.set(static_cast<ClipboardEncodingType>(1));

    this->mod_vnc.bogus_clipboard_infinite_loop.attach_ini(this, AUTHID_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP);
    this->mod_vnc.bogus_clipboard_infinite_loop.set(0);












    this->video.disable_keyboard_log.attach_ini(this, AUTHID_DISABLE_KEYBOARD_LOG);
    this->to_send_set.insert(AUTHID_DISABLE_KEYBOARD_LOG);

    this->video.disable_clipboard_log.attach_ini(this, AUTHID_DISABLE_CLIPBOARD_LOG);

    this->video.rt_display.attach_ini(this, AUTHID_RT_DISPLAY);
    this->video.rt_display.set(0);


    this->translation.language.set(static_cast<Language>(0));
    this->translation.language.attach_ini(this, AUTHID_LANGUAGE);

    this->context.opt_bitrate.set(40000);
    this->context.opt_bitrate.attach_ini(this, AUTHID_OPT_BITRATE);
    this->context.opt_framerate.set(5);
    this->context.opt_framerate.attach_ini(this, AUTHID_OPT_FRAMERATE);
    this->context.opt_qscale.set(15);
    this->context.opt_qscale.attach_ini(this, AUTHID_OPT_QSCALE);

    this->to_send_set.insert(AUTHID_OPT_BPP);
    this->context.opt_bpp.set(24);
    this->context.opt_bpp.attach_ini(this, AUTHID_OPT_BPP);
    this->to_send_set.insert(AUTHID_OPT_HEIGHT);
    this->context.opt_height.set(600);
    this->context.opt_height.attach_ini(this, AUTHID_OPT_HEIGHT);
    this->to_send_set.insert(AUTHID_OPT_WIDTH);
    this->context.opt_width.set(800);
    this->context.opt_width.attach_ini(this, AUTHID_OPT_WIDTH);

    this->context.auth_error_message.attach_ini(this, AUTHID_AUTH_ERROR_MESSAGE);
    this->context.auth_error_message.use();

    this->to_send_set.insert(AUTHID_SELECTOR);
    this->context.selector.set(0);
    this->context.selector.attach_ini(this, AUTHID_SELECTOR);
    this->to_send_set.insert(AUTHID_SELECTOR_CURRENT_PAGE);
    this->context.selector_current_page.set(1);
    this->context.selector_current_page.attach_ini(this, AUTHID_SELECTOR_CURRENT_PAGE);
    this->to_send_set.insert(AUTHID_SELECTOR_DEVICE_FILTER);
    this->context.selector_device_filter.attach_ini(this, AUTHID_SELECTOR_DEVICE_FILTER);
    this->to_send_set.insert(AUTHID_SELECTOR_GROUP_FILTER);
    this->context.selector_group_filter.attach_ini(this, AUTHID_SELECTOR_GROUP_FILTER);
    this->to_send_set.insert(AUTHID_SELECTOR_PROTO_FILTER);
    this->context.selector_proto_filter.attach_ini(this, AUTHID_SELECTOR_PROTO_FILTER);
    this->to_send_set.insert(AUTHID_SELECTOR_LINES_PER_PAGE);
    this->context.selector_lines_per_page.set(0);
    this->context.selector_lines_per_page.attach_ini(this, AUTHID_SELECTOR_LINES_PER_PAGE);
    this->context.selector_number_of_pages.set(1);
    this->context.selector_number_of_pages.attach_ini(this, AUTHID_SELECTOR_NUMBER_OF_PAGES);

    this->to_send_set.insert(AUTHID_TARGET_PASSWORD);
    this->context.target_password.ask();
    this->context.target_password.attach_ini(this, AUTHID_TARGET_PASSWORD);
    this->to_send_set.insert(AUTHID_TARGET_HOST);
    this->context.target_host.set("");
    this->context.target_host.ask();
    this->context.target_host.attach_ini(this, AUTHID_TARGET_HOST);
    this->context.target_port.set(3389);
    this->context.target_port.ask();
    this->context.target_port.attach_ini(this, AUTHID_TARGET_PORT);
    this->to_send_set.insert(AUTHID_TARGET_PROTOCOL);
    this->context.target_protocol.set("RDP");
    this->context.target_protocol.ask();
    this->context.target_protocol.attach_ini(this, AUTHID_TARGET_PROTOCOL);

    this->to_send_set.insert(AUTHID_PASSWORD);
    this->context.password.ask();
    this->context.password.attach_ini(this, AUTHID_PASSWORD);

    this->to_send_set.insert(AUTHID_REPORTING);
    this->context.reporting.attach_ini(this, AUTHID_REPORTING);

    this->context.auth_channel_answer.attach_ini(this, AUTHID_AUTH_CHANNEL_ANSWER);
    this->to_send_set.insert(AUTHID_AUTH_CHANNEL_RESULT);
    this->context.auth_channel_result.attach_ini(this, AUTHID_AUTH_CHANNEL_RESULT);
    this->to_send_set.insert(AUTHID_AUTH_CHANNEL_TARGET);
    this->context.auth_channel_target.attach_ini(this, AUTHID_AUTH_CHANNEL_TARGET);

    this->context.message.attach_ini(this, AUTHID_MESSAGE);
    this->context.pattern_kill.attach_ini(this, AUTHID_PATTERN_KILL);
    this->context.pattern_notify.attach_ini(this, AUTHID_PATTERN_NOTIFY);

    this->to_send_set.insert(AUTHID_ACCEPT_MESSAGE);
    this->context.accept_message.attach_ini(this, AUTHID_ACCEPT_MESSAGE);
    this->to_send_set.insert(AUTHID_DISPLAY_MESSAGE);
    this->context.display_message.attach_ini(this, AUTHID_DISPLAY_MESSAGE);

    this->context.rejected.attach_ini(this, AUTHID_REJECTED);

    this->context.authenticated.set(0);
    this->context.authenticated.attach_ini(this, AUTHID_AUTHENTICATED);

    this->context.keepalive.set(0);
    this->context.keepalive.attach_ini(this, AUTHID_KEEPALIVE);
    this->to_send_set.insert(AUTHID_KEEPALIVE);

    this->context.session_id.attach_ini(this, AUTHID_SESSION_ID);

    this->context.end_date_cnx.set(0);
    this->context.end_date_cnx.attach_ini(this, AUTHID_END_DATE_CNX);
    this->context.end_time.attach_ini(this, AUTHID_END_TIME);

    this->context.mode_console.set("allow");
    this->context.mode_console.attach_ini(this, AUTHID_MODE_CONSOLE);
    this->context.timezone.set(-3600);
    this->context.timezone.attach_ini(this, AUTHID_TIMEZONE);

    this->to_send_set.insert(AUTHID_REAL_TARGET_DEVICE);
    this->context.real_target_device.attach_ini(this, AUTHID_REAL_TARGET_DEVICE);

    this->context.authentication_challenge.ask();
    this->context.authentication_challenge.attach_ini(this, AUTHID_AUTHENTICATION_CHALLENGE);

    this->to_send_set.insert(AUTHID_TICKET);
    this->context.ticket.set("");
    this->context.ticket.attach_ini(this, AUTHID_TICKET);
    this->context.ticket.use();
    this->to_send_set.insert(AUTHID_COMMENT);
    this->context.comment.set("");
    this->context.comment.attach_ini(this, AUTHID_COMMENT);
    this->context.comment.use();
    this->to_send_set.insert(AUTHID_DURATION);
    this->context.duration.set("");
    this->context.duration.attach_ini(this, AUTHID_DURATION);
    this->context.duration.use();
    this->to_send_set.insert(AUTHID_WAITINFORETURN);
    this->context.waitinforeturn.set("");
    this->context.waitinforeturn.attach_ini(this, AUTHID_WAITINFORETURN);
    this->context.waitinforeturn.use();
    this->to_send_set.insert(AUTHID_SHOWFORM);
    this->context.showform.set(0);
    this->context.showform.attach_ini(this, AUTHID_SHOWFORM);
    this->context.showform.use();
    this->to_send_set.insert(AUTHID_FORMFLAG);
    this->context.formflag.set(0);
    this->context.formflag.attach_ini(this, AUTHID_FORMFLAG);
    this->context.formflag.use();

    this->to_send_set.insert(AUTHID_MODULE);
    this->context.module.set("login");
    this->context.module.attach_ini(this, AUTHID_MODULE);
    this->context.module.use();
    this->context.forcemodule.set(0);
    this->context.forcemodule.attach_ini(this, AUTHID_FORCEMODULE);
    this->context.proxy_opt.attach_ini(this, AUTHID_PROXY_OPT);
}

inline void Inifile::initialize() {
    this->ask<cfg::globals::capture_chunk>();

    this->ask<cfg::globals::auth_user>();
    this->insert_index<cfg::globals::host>(std::true_type());
    this->insert_index<cfg::globals::target>(std::true_type());
    this->ask<cfg::globals::target_device>();
    this->ask<cfg::globals::target_user>();
    this->insert_index<cfg::globals::target_application>(std::true_type());
    this->insert_index<cfg::globals::target_application_account>(std::true_type());
    this->insert_index<cfg::globals::target_application_password>(std::true_type());



    this->insert_index<cfg::globals::enable_file_encryption>(std::true_type());


    this->insert_index<cfg::globals::alternate_shell>(std::true_type());
    this->insert_index<cfg::globals::shell_working_directory>(std::true_type());

    this->insert_index<cfg::globals::movie>(std::true_type());
    this->insert_index<cfg::globals::movie_path>(std::true_type());


    this->globals.enable_wab_agent.attach_ini(this, AUTHID_OPT_WABAGENT);
    this->globals.enable_wab_agent.set(0);
    this->globals.wab_agent_launch_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT);
    this->globals.wab_agent_launch_timeout.set(0);
    this->globals.wab_agent_keepalive_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT);
    this->globals.wab_agent_keepalive_timeout.set(0);



    this->insert_index<cfg::client::keyboard_layout>(std::true_type());



    this->insert_index<cfg::client::disable_tsk_switch_shortcuts>(std::true_type());
















    this->insert_index<cfg::mod_rdp::bogus_sc_net_size>(std::true_type());

    this->insert_index<cfg::mod_rdp::client_device_announce_timeout>(std::true_type());

    this->insert_index<cfg::mod_rdp::proxy_managed_drives>(std::true_type());
    this->insert_index<cfg::mod_vnc::clipboard_up>(std::true_type());
    this->insert_index<cfg::mod_vnc::clipboard_down>(std::true_type());



    this->insert_index<cfg::mod_vnc::server_clipboard_encoding_type>(std::true_type());

    this->insert_index<cfg::mod_vnc::bogus_clipboard_infinite_loop>(std::true_type());






    this->insert_index<cfg::video::disable_keyboard_log>(std::true_type());

    this->insert_index<cfg::video::disable_clipboard_log>(std::true_type());

    this->insert_index<cfg::video::rt_display>(std::true_type());


    this->insert_index<cfg::translation::language>(std::true_type());

    this->insert_index<cfg::context::opt_bitrate>(std::true_type());
    this->insert_index<cfg::context::opt_framerate>(std::true_type());
    this->insert_index<cfg::context::opt_qscale>(std::true_type());

    this->insert_index<cfg::context::opt_bpp>(std::true_type());
    this->insert_index<cfg::context::opt_height>(std::true_type());
    this->insert_index<cfg::context::opt_width>(std::true_type());

    this->ask<cfg::context::auth_error_message>();

    this->insert_index<cfg::context::selector>(std::true_type());
    this->insert_index<cfg::context::selector_current_page>(std::true_type());
    this->insert_index<cfg::context::selector_device_filter>(std::true_type());
    this->insert_index<cfg::context::selector_group_filter>(std::true_type());
    this->insert_index<cfg::context::selector_proto_filter>(std::true_type());
    this->insert_index<cfg::context::selector_lines_per_page>(std::true_type());
    this->insert_index<cfg::context::selector_number_of_pages>(std::true_type());

    this->ask<cfg::context::target_password>();
    this->ask<cfg::context::target_host>();
    this->ask<cfg::context::target_port>();
    this->ask<cfg::context::target_protocol>();

    this->ask<cfg::context::password>();

    this->insert_index<cfg::context::reporting>(std::true_type());

    this->insert_index<cfg::context::auth_channel_answer>(std::true_type());
    this->insert_index<cfg::context::auth_channel_result>(std::true_type());
    this->insert_index<cfg::context::auth_channel_target>(std::true_type());

    this->insert_index<cfg::context::message>(std::true_type());
    this->insert_index<cfg::context::pattern_kill>(std::true_type());
    this->insert_index<cfg::context::pattern_notify>(std::true_type());

    this->insert_index<cfg::context::accept_message>(std::true_type());
    this->insert_index<cfg::context::display_message>(std::true_type());

    this->insert_index<cfg::context::rejected>(std::true_type());

    this->insert_index<cfg::context::authenticated>(std::true_type());

    this->insert_index<cfg::context::keepalive>(std::true_type());

    this->insert_index<cfg::context::session_id>(std::true_type());

    this->insert_index<cfg::context::end_date_cnx>(std::true_type());
    this->insert_index<cfg::context::end_time>(std::true_type());

    this->insert_index<cfg::context::mode_console>(std::true_type());
    this->insert_index<cfg::context::timezone>(std::true_type());

    this->insert_index<cfg::context::real_target_device>(std::true_type());

    this->ask<cfg::context::authentication_challenge>();

    this->insert_index<cfg::context::ticket>(std::true_type());
    this->insert_index<cfg::context::comment>(std::true_type());
    this->insert_index<cfg::context::duration>(std::true_type());
    this->insert_index<cfg::context::waitinforeturn>(std::true_type());
    this->ask<cfg::context::showform>();
    this->ask<cfg::context::formflag>();

    this->insert_index<cfg::context::forcemodule>(std::true_type());
    this->insert_index<cfg::context::proxy_opt>(std::true_type());
}

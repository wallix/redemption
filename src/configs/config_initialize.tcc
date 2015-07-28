inline void Inifile::initialize() {
    this->insert_index<cfg::globals::capture_chunk>(std::true_type());

    this->ask<cfg::globals::host>();
    this->ask<cfg::globals::target>();
    this->ask<cfg::globals::target_device>();
    this->ask<cfg::globals::target_user>();
    this->ask<cfg::globals::target_application>();
    this->ask<cfg::globals::target_application_account>();
    this->ask<cfg::globals::target_application_password>();



    this->ask<cfg::globals::enable_file_encryption>();


    this->ask<cfg::globals::alternate_shell>();
    this->ask<cfg::globals::shell_working_directory>();

    this->ask<cfg::globals::movie>();
    this->ask<cfg::globals::movie_path>();


    this->ask<cfg::globals::enable_wab_agent>();
    this->ask<cfg::globals::wab_agent_launch_timeout>();
    this->ask<cfg::globals::wab_agent_on_launch_failure>();
    this->ask<cfg::globals::wab_agent_keepalive_timeout>();



    this->ask<cfg::client::keyboard_layout>();



    this->ask<cfg::client::disable_tsk_switch_shortcuts>();
















    this->ask<cfg::mod_rdp::bogus_sc_net_size>();

    this->ask<cfg::mod_rdp::client_device_announce_timeout>();

    this->ask<cfg::mod_rdp::proxy_managed_drives>();
    this->ask<cfg::mod_vnc::clipboard_up>();
    this->ask<cfg::mod_vnc::clipboard_down>();



    this->ask<cfg::mod_vnc::server_clipboard_encoding_type>();

    this->ask<cfg::mod_vnc::bogus_clipboard_infinite_loop>();






    this->ask<cfg::video::disable_keyboard_log>();

    this->ask<cfg::video::disable_clipboard_log>();

    this->ask<cfg::video::rt_display>();


    this->ask<cfg::translation::language>();

    this->ask<cfg::context::opt_bitrate>();
    this->ask<cfg::context::opt_framerate>();
    this->ask<cfg::context::opt_qscale>();

    this->ask<cfg::context::opt_bpp>();
    this->ask<cfg::context::opt_height>();
    this->ask<cfg::context::opt_width>();

    this->insert_index<cfg::context::auth_error_message>(std::true_type());

    this->ask<cfg::context::selector>();
    this->insert_index<cfg::context::selector_device_filter>(std::true_type());
    this->insert_index<cfg::context::selector_group_filter>(std::true_type());
    this->insert_index<cfg::context::selector_proto_filter>(std::true_type());
    this->ask<cfg::context::selector_number_of_pages>();

    this->insert_index<cfg::context::target_password>(std::true_type());
    this->insert_index<cfg::context::target_host>(std::true_type());
    this->insert_index<cfg::context::target_port>(std::true_type());
    this->ask<cfg::context::target_protocol>();


    this->ask<cfg::context::reporting>();

    this->ask<cfg::context::auth_channel_answer>();
    this->ask<cfg::context::auth_channel_result>();
    this->ask<cfg::context::auth_channel_target>();

    this->ask<cfg::context::message>();
    this->ask<cfg::context::pattern_kill>();
    this->ask<cfg::context::pattern_notify>();

    this->ask<cfg::context::accept_message>();
    this->ask<cfg::context::display_message>();

    this->ask<cfg::context::rejected>();

    this->ask<cfg::context::authenticated>();

    this->ask<cfg::context::keepalive>();

    this->ask<cfg::context::session_id>();

    this->ask<cfg::context::end_date_cnx>();
    this->ask<cfg::context::end_time>();

    this->ask<cfg::context::mode_console>();
    this->ask<cfg::context::timezone>();

    this->ask<cfg::context::real_target_device>();

    this->ask<cfg::context::ticket>();
    this->ask<cfg::context::comment>();
    this->ask<cfg::context::duration>();
    this->ask<cfg::context::waitinforeturn>();
    this->ask<cfg::context::showform>();
    this->insert_index<cfg::context::formflag>(std::true_type());

    this->ask<cfg::context::proxy_opt>();
}

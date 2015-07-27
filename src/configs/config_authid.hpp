enum authid_t {
    AUTHID_UNKNOWN = 0,
    AUTHID_accept_message,
#define STRAUTHID_accept_message "accept_message"
    AUTHID_alternate_shell,
#define STRAUTHID_alternate_shell "alternate_shell"
    AUTHID_auth_channel_answer,
#define STRAUTHID_auth_channel_answer "auth_channel_answer"
    AUTHID_auth_channel_result,
#define STRAUTHID_auth_channel_result "auth_channel_result"
    AUTHID_auth_channel_target,
#define STRAUTHID_auth_channel_target "auth_channel_target"
    AUTHID_authenticated,
#define STRAUTHID_authenticated "authenticated"
    AUTHID_comment,
#define STRAUTHID_comment "comment"
    AUTHID_disable_clipboard_log,
#define STRAUTHID_disable_clipboard_log "disable_clipboard_log"
    AUTHID_disable_keyboard_log,
#define STRAUTHID_disable_keyboard_log "disable_keyboard_log"
    AUTHID_disable_tsk_switch_shortcuts,
#define STRAUTHID_disable_tsk_switch_shortcuts "disable_tsk_switch_shortcuts"
    AUTHID_display_message,
#define STRAUTHID_display_message "display_message"
    AUTHID_duration,
#define STRAUTHID_duration "duration"
    AUTHID_end_date_cnx,
#define STRAUTHID_end_date_cnx "timeclose"
    AUTHID_end_time,
#define STRAUTHID_end_time "end_time"
    AUTHID_forcemodule,
#define STRAUTHID_forcemodule "forcemodule"
    AUTHID_host,
#define STRAUTHID_host "ip_client"
    AUTHID_keepalive,
#define STRAUTHID_keepalive "keepalive"
    AUTHID_keyboard_layout,
#define STRAUTHID_keyboard_layout "keyboard_layout"
    AUTHID_language,
#define STRAUTHID_language "language"
    AUTHID_message,
#define STRAUTHID_message "message"
    AUTHID_mode_console,
#define STRAUTHID_mode_console "mode_console"
    AUTHID_module,
#define STRAUTHID_module "module"
    AUTHID_opt_bitrate,
#define STRAUTHID_opt_bitrate "bitrate"
    AUTHID_opt_bpp,
#define STRAUTHID_opt_bpp "bpp"
    AUTHID_opt_client_device_announce_timeout,
#define STRAUTHID_opt_client_device_announce_timeout "client_device_announce_timeout"
    AUTHID_opt_file_encryption,
#define STRAUTHID_opt_file_encryption "file_encryption"
    AUTHID_opt_framerate,
#define STRAUTHID_opt_framerate "framerate"
    AUTHID_opt_height,
#define STRAUTHID_opt_height "height"
    AUTHID_opt_movie,
#define STRAUTHID_opt_movie "is_rec"
    AUTHID_opt_movie_path,
#define STRAUTHID_opt_movie_path "rec_patch"
    AUTHID_opt_proxy_managed_drives,
#define STRAUTHID_opt_proxy_managed_drives "proxy_managed_drives"
    AUTHID_opt_qscale,
#define STRAUTHID_opt_qscale "qscale"
    AUTHID_opt_wabagent,
#define STRAUTHID_opt_wabagent "wab_agent"
    AUTHID_opt_wabagent_keepalive_timeout,
#define STRAUTHID_opt_wabagent_keepalive_timeout "wab_agent_keepalive_timeout"
    AUTHID_opt_wabagent_launch_timeout,
#define STRAUTHID_opt_wabagent_launch_timeout "wab_agent_launch_timeout"
    AUTHID_opt_wabagent_on_launch_failure,
#define STRAUTHID_opt_wabagent_on_launch_failure "wab_agent_on_launch_failure"
    AUTHID_opt_width,
#define STRAUTHID_opt_width "width"
    AUTHID_pattern_kill,
#define STRAUTHID_pattern_kill "pattern_kill"
    AUTHID_pattern_notify,
#define STRAUTHID_pattern_notify "pattern_notify"
    AUTHID_proxy_opt,
#define STRAUTHID_proxy_opt "proxy_opt"
    AUTHID_rdp_bogus_sc_net_size,
#define STRAUTHID_rdp_bogus_sc_net_size "rdp_bogus_sc_net_size"
    AUTHID_real_target_device,
#define STRAUTHID_real_target_device "real_target_device"
    AUTHID_rejected,
#define STRAUTHID_rejected "rejected"
    AUTHID_reporting,
#define STRAUTHID_reporting "reporting"
    AUTHID_rt_display,
#define STRAUTHID_rt_display "rt_display"
    AUTHID_selector,
#define STRAUTHID_selector "selector"
    AUTHID_selector_current_page,
#define STRAUTHID_selector_current_page "selector_current_page"
    AUTHID_selector_device_filter,
#define STRAUTHID_selector_device_filter "selector_device_filter"
    AUTHID_selector_group_filter,
#define STRAUTHID_selector_group_filter "selector_group_filter"
    AUTHID_selector_lines_per_page,
#define STRAUTHID_selector_lines_per_page "selector_lines_per_page"
    AUTHID_selector_number_of_pages,
#define STRAUTHID_selector_number_of_pages "selector_number_of_pages"
    AUTHID_selector_proto_filter,
#define STRAUTHID_selector_proto_filter "selector_proto_filter"
    AUTHID_session_id,
#define STRAUTHID_session_id "session_id"
    AUTHID_shell_working_directory,
#define STRAUTHID_shell_working_directory "shell_working_directory"
    AUTHID_target,
#define STRAUTHID_target "ip_target"
    AUTHID_target_application,
#define STRAUTHID_target_application "target_application"
    AUTHID_target_application_account,
#define STRAUTHID_target_application_account "target_application_account"
    AUTHID_target_application_password,
#define STRAUTHID_target_application_password "target_application_password"
    AUTHID_ticket,
#define STRAUTHID_ticket "ticket"
    AUTHID_timezone,
#define STRAUTHID_timezone "timezone"
    AUTHID_vnc_bogus_clipboard_infinite_loop,
#define STRAUTHID_vnc_bogus_clipboard_infinite_loop "vnc_bogus_clipboard_infinite_loop"
    AUTHID_vnc_clipboard_down,
#define STRAUTHID_vnc_clipboard_down "clipboard_down"
    AUTHID_vnc_clipboard_up,
#define STRAUTHID_vnc_clipboard_up "clipboard_up"
    AUTHID_vnc_server_clipboard_encoding_type,
#define STRAUTHID_vnc_server_clipboard_encoding_type "vnc_server_clipboard_encoding_type"
    AUTHID_waitinforeturn,
#define STRAUTHID_waitinforeturn "waitinforeturn"
    MAX_AUTHID
};
constexpr char const * const authstr[] = {
    STRAUTHID_accept_message, // AUTHID_accept_message
    STRAUTHID_alternate_shell, // AUTHID_alternate_shell
    STRAUTHID_auth_channel_answer, // AUTHID_auth_channel_answer
    STRAUTHID_auth_channel_result, // AUTHID_auth_channel_result
    STRAUTHID_auth_channel_target, // AUTHID_auth_channel_target
    STRAUTHID_authenticated, // AUTHID_authenticated
    STRAUTHID_comment, // AUTHID_comment
    STRAUTHID_disable_clipboard_log, // AUTHID_disable_clipboard_log
    STRAUTHID_disable_keyboard_log, // AUTHID_disable_keyboard_log
    STRAUTHID_disable_tsk_switch_shortcuts, // AUTHID_disable_tsk_switch_shortcuts
    STRAUTHID_display_message, // AUTHID_display_message
    STRAUTHID_duration, // AUTHID_duration
    STRAUTHID_end_date_cnx, // AUTHID_end_date_cnx
    STRAUTHID_end_time, // AUTHID_end_time
    STRAUTHID_forcemodule, // AUTHID_forcemodule
    STRAUTHID_host, // AUTHID_host
    STRAUTHID_keepalive, // AUTHID_keepalive
    STRAUTHID_keyboard_layout, // AUTHID_keyboard_layout
    STRAUTHID_language, // AUTHID_language
    STRAUTHID_message, // AUTHID_message
    STRAUTHID_mode_console, // AUTHID_mode_console
    STRAUTHID_module, // AUTHID_module
    STRAUTHID_opt_bitrate, // AUTHID_opt_bitrate
    STRAUTHID_opt_bpp, // AUTHID_opt_bpp
    STRAUTHID_opt_client_device_announce_timeout, // AUTHID_opt_client_device_announce_timeout
    STRAUTHID_opt_file_encryption, // AUTHID_opt_file_encryption
    STRAUTHID_opt_framerate, // AUTHID_opt_framerate
    STRAUTHID_opt_height, // AUTHID_opt_height
    STRAUTHID_opt_movie, // AUTHID_opt_movie
    STRAUTHID_opt_movie_path, // AUTHID_opt_movie_path
    STRAUTHID_opt_proxy_managed_drives, // AUTHID_opt_proxy_managed_drives
    STRAUTHID_opt_qscale, // AUTHID_opt_qscale
    STRAUTHID_opt_wabagent, // AUTHID_opt_wabagent
    STRAUTHID_opt_wabagent_keepalive_timeout, // AUTHID_opt_wabagent_keepalive_timeout
    STRAUTHID_opt_wabagent_launch_timeout, // AUTHID_opt_wabagent_launch_timeout
    STRAUTHID_opt_wabagent_on_launch_failure, // AUTHID_opt_wabagent_on_launch_failure
    STRAUTHID_opt_width, // AUTHID_opt_width
    STRAUTHID_pattern_kill, // AUTHID_pattern_kill
    STRAUTHID_pattern_notify, // AUTHID_pattern_notify
    STRAUTHID_proxy_opt, // AUTHID_proxy_opt
    STRAUTHID_rdp_bogus_sc_net_size, // AUTHID_rdp_bogus_sc_net_size
    STRAUTHID_real_target_device, // AUTHID_real_target_device
    STRAUTHID_rejected, // AUTHID_rejected
    STRAUTHID_reporting, // AUTHID_reporting
    STRAUTHID_rt_display, // AUTHID_rt_display
    STRAUTHID_selector, // AUTHID_selector
    STRAUTHID_selector_current_page, // AUTHID_selector_current_page
    STRAUTHID_selector_device_filter, // AUTHID_selector_device_filter
    STRAUTHID_selector_group_filter, // AUTHID_selector_group_filter
    STRAUTHID_selector_lines_per_page, // AUTHID_selector_lines_per_page
    STRAUTHID_selector_number_of_pages, // AUTHID_selector_number_of_pages
    STRAUTHID_selector_proto_filter, // AUTHID_selector_proto_filter
    STRAUTHID_session_id, // AUTHID_session_id
    STRAUTHID_shell_working_directory, // AUTHID_shell_working_directory
    STRAUTHID_target, // AUTHID_target
    STRAUTHID_target_application, // AUTHID_target_application
    STRAUTHID_target_application_account, // AUTHID_target_application_account
    STRAUTHID_target_application_password, // AUTHID_target_application_password
    STRAUTHID_ticket, // AUTHID_ticket
    STRAUTHID_timezone, // AUTHID_timezone
    STRAUTHID_vnc_bogus_clipboard_infinite_loop, // AUTHID_vnc_bogus_clipboard_infinite_loop
    STRAUTHID_vnc_clipboard_down, // AUTHID_vnc_clipboard_down
    STRAUTHID_vnc_clipboard_up, // AUTHID_vnc_clipboard_up
    STRAUTHID_vnc_server_clipboard_encoding_type, // AUTHID_vnc_server_clipboard_encoding_type
    STRAUTHID_waitinforeturn, // AUTHID_waitinforeturn
};


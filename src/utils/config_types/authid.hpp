/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_SRC_UTILS_CONFIG_TYPES_AUTHID_HPP
#define REDEMPTION_SRC_UTILS_CONFIG_TYPES_AUTHID_HPP

namespace config_types {

enum authid_t {
    AUTHID_UNKNOWN = 0,

    AUTHID_LANGUAGE,

    // Options
    AUTHID_KEYBOARD_LAYOUT,         // keyboard_layout
    AUTHID_VNC_CLIPBOARD_UP,        // clipboard_up
    AUTHID_VNC_CLIPBOARD_DOWN,      // clipboard_down
    AUTHID_OPT_FILE_ENCRYPTION,     // file encryption

    // Video capture
    AUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
    AUTHID_OPT_MOVIE,       // is_rec
    AUTHID_OPT_MOVIE_PATH,  // rec_path
    AUTHID_VIDEO_QUALITY,

    // Alternate shell
    AUTHID_ALTERNATE_SHELL,
    AUTHID_SHELL_WORKING_DIRECTORY,

    // Context
    AUTHID_OPT_BITRATE,     // Bit rate for video encoding
    AUTHID_OPT_FRAMERATE,   // Frame rate for video encoding
    AUTHID_OPT_QSCALE,      // QScale parameter for vdeo encoding

    AUTHID_OPT_BPP,         // bits per planes (number of colors)
    AUTHID_OPT_HEIGHT,      // client height
    AUTHID_OPT_WIDTH,       // client width

    AUTHID_AUTH_ERROR_MESSAGE,

    AUTHID_SELECTOR,
    AUTHID_SELECTOR_CURRENT_PAGE,       // current page
    AUTHID_SELECTOR_DEVICE_FILTER,      // device filter text
    AUTHID_SELECTOR_GROUP_FILTER,       // group filter text
    AUTHID_SELECTOR_PROTO_FILTER,       // protocol filter text
    AUTHID_SELECTOR_LINES_PER_PAGE,     // number of lines per page
    AUTHID_SELECTOR_NUMBER_OF_PAGES,    // number of pages

    AUTHID_TARGET_DEVICE,               // target_device
    AUTHID_TARGET_PASSWORD,             // target_password
    AUTHID_TARGET_HOST,                 // target_host
    AUTHID_TARGET_PORT,                 // target_port
    AUTHID_TARGET_PROTOCOL,             // proto_dest
    AUTHID_TARGET_USER,                 // target_login
    AUTHID_TARGET_APPLICATION,          // target_application
    AUTHID_TARGET_APPLICATION_ACCOUNT,  // target_application_account
    AUTHID_TARGET_APPLICATION_PASSWORD, // target_application_password

    AUTHID_AUTH_USER,       // login
    AUTHID_HOST,            // ip_client
    AUTHID_TARGET,          // ip_target
    AUTHID_PASSWORD,        // password

    AUTHID_REPORTING,       // reporting message (client -> server)

    AUTHID_AUTHCHANNEL_ANSWER,  // WabLauncher target answer
    AUTHID_AUTHCHANNEL_RESULT,  // WabLauncher session result
    AUTHID_AUTHCHANNEL_TARGET,  // WabLauncher target request

    AUTHID_MESSAGE, // warning_message
    AUTHID_PATTERN_KILL,   // regex to close connexion
    AUTHID_PATTERN_NOTIFY, // regex to notify to authentifier

    AUTHID_ACCEPT_MESSAGE,  // display a dialog to valid a message
    AUTHID_DISPLAY_MESSAGE, // display a dialog box with a message

    AUTHID_AUTHENTICATED,
    AUTHID_REJECTED,        // rejected

    AUTHID_KEEPALIVE,

    AUTHID_SESSION_ID,      // session_id

    AUTHID_END_DATE_CNX,    // timeclose
    AUTHID_END_TIME,        // end time as text

    AUTHID_MODE_CONSOLE,
    AUTHID_TIMEZONE,

    AUTHID_REAL_TARGET_DEVICE,  // target device in ip transparent mode

    AUTHID_AUTHENTICATION_CHALLENGE,

    AUTHID_MODULE,
    AUTHID_FORCEMODULE,
    AUTHID_TICKET,
    AUTHID_COMMENT,
    AUTHID_DURATION,
    AUTHID_WAITINFORETURN,
    AUTHID_SHOWFORM,
    AUTHID_FORMFLAG,

    AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS,
    AUTHID_PROXY_OPT,

    AUTHID_DISABLE_KEYBOARD_LOG,
    AUTHID_DISABLE_CLIPBOARD_LOG,

    AUTHID_RT_DISPLAY,

    AUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE,

    AUTHID_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP,

    AUTHID_RDP_BOGUS_SC_NET_SIZE,

    AUTHID_OPT_WABAGENT,
    AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT,
    AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT,

    AUTHID_OPT_CLIENT_DEVICE_ANNOUNCE_TIMEOUT,

    AUTHID_OPT_PROXY_MANAGED_DRIVES,

    MAX_AUTHID
};

#define STRAUTHID_LANGUAGE                      "language"
// Options
#define STRAUTHID_KEYBOARD_LAYOUT               "keyboard_layout"
#define STRAUTHID_VNC_CLIPBOARD_UP              "clipboard_up"
#define STRAUTHID_VNC_CLIPBOARD_DOWN            "clipboard_down"
#define STRAUTHID_OPT_FILE_ENCRYPTION           "file_encryption"
// Video capture
#define STRAUTHID_OPT_CODEC_ID                  "codec_id"
#define STRAUTHID_OPT_MOVIE                     "is_rec"
#define STRAUTHID_OPT_MOVIE_PATH                "rec_path"
#define STRAUTHID_VIDEO_QUALITY                 "video_quality"
// Alternate shell
#define STRAUTHID_ALTERNATE_SHELL               "alternate_shell"
#define STRAUTHID_SHELL_WORKING_DIRECTORY       "shell_working_directory"
// Context
#define STRAUTHID_OPT_BITRATE                   "bitrate"
#define STRAUTHID_OPT_FRAMERATE                 "framerate"
#define STRAUTHID_OPT_QSCALE                    "qscale"
#define STRAUTHID_OPT_BPP                       "bpp"
#define STRAUTHID_OPT_HEIGHT                    "height"
#define STRAUTHID_OPT_WIDTH                     "width"

#define STRAUTHID_AUTH_ERROR_MESSAGE            "error_message"

#define STRAUTHID_SELECTOR                      "selector"
#define STRAUTHID_SELECTOR_CURRENT_PAGE         "selector_current_page"
#define STRAUTHID_SELECTOR_DEVICE_FILTER        "selector_device_filter"
#define STRAUTHID_SELECTOR_GROUP_FILTER         "selector_group_filter"
#define STRAUTHID_SELECTOR_PROTO_FILTER         "selector_proto_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE       "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES      "selector_number_of_pages"

#define STRAUTHID_TARGET_DEVICE                 "target_device"
#define STRAUTHID_TARGET_PASSWORD               "target_password"
#define STRAUTHID_TARGET_HOST                   "target_host"
#define STRAUTHID_TARGET_PORT                   "target_port"
#define STRAUTHID_TARGET_PROTOCOL               "proto_dest"
#define STRAUTHID_TARGET_USER                   "target_login"
#define STRAUTHID_TARGET_APPLICATION            "target_application"
#define STRAUTHID_TARGET_APPLICATION_ACCOUNT    "target_application_account"
#define STRAUTHID_TARGET_APPLICATION_PASSWORD   "target_application_password"

#define STRAUTHID_AUTH_USER                     "login"
#define STRAUTHID_HOST                          "ip_client"
#define STRAUTHID_TARGET                        "ip_target"
#define STRAUTHID_PASSWORD                      "password"

#define STRAUTHID_REPORTING                     "reporting"

#define STRAUTHID_AUTHCHANNEL_ANSWER            "auth_channel_answer"
#define STRAUTHID_AUTHCHANNEL_RESULT            "auth_channel_result"
#define STRAUTHID_AUTHCHANNEL_TARGET            "auth_channel_target"

#define STRAUTHID_MESSAGE                       "message"
#define STRAUTHID_PATTERN_KILL                  "pattern_kill"
#define STRAUTHID_PATTERN_NOTIFY                "pattern_notify"

#define STRAUTHID_ACCEPT_MESSAGE                "accept_message"
#define STRAUTHID_DISPLAY_MESSAGE               "display_message"


#define STRAUTHID_AUTHENTICATED                 "authenticated"
#define STRAUTHID_REJECTED                      "rejected"

#define STRAUTHID_KEEPALIVE                     "keepalive"

#define STRAUTHID_SESSION_ID                    "session_id"

#define STRAUTHID_END_DATE_CNX                  "timeclose"
#define STRAUTHID_END_TIME                      "end_time"

#define STRAUTHID_MODE_CONSOLE                  "mode_console"
#define STRAUTHID_TIMEZONE                      "timezone"

#define STRAUTHID_REAL_TARGET_DEVICE            "real_target_device"

#define STRAUTHID_AUTHENTICATION_CHALLENGE      "authentication_challenge"
#define STRAUTHID_MODULE                        "module"
#define STRAUTHID_FORCEMODULE                   "forcemodule"
#define STRAUTHID_TICKET                        "ticket"
#define STRAUTHID_COMMENT                       "comment"
#define STRAUTHID_DURATION                      "duration"
#define STRAUTHID_WAITINFORETURN                "waitinforeturn"
#define STRAUTHID_SHOWFORM                      "showform"
#define STRAUTHID_FORMFLAG                      "formflag"
#define STRAUTHID_DISABLE_TSK_SWITCH_SHORTCUTS  "disable_tsk_switch_shortcuts"
#define STRAUTHID_PROXY_OPT                     "proxy_opt"

#define STRAUTHID_DISABLE_KEYBOARD_LOG          "disable_keyboard_log"
#define STRAUTHID_DISABLE_CLIPBOARD_LOG         "disable_clipboard_log"

#define STRAUTHID_RT_DISPLAY                    "rt_display"

#define STRAUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE    "vnc_server_clipboard_encoding_type"

#define STRAUTHID_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP     "vnc_bogus_clipboard_infinite_loop"

#define STRAUTHID_RDP_BOGUS_SC_NET_SIZE         "rdp_bogus_sc_net_size"

#define STRAUTHID_OPT_WABAGENT                      "wab_agent"
#define STRAUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT       "wab_agent_launch_timeout"
#define STRAUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT    "wab_agent_keepalive_timeout"

#define STRAUTHID_OPT_CLIENT_DEVICE_ANNOUNCE_TIMEOUT    "client_device_announce_timeout"

#define STRAUTHID_OPT_PROXY_MANAGED_DRIVES      "proxy_managed_drives"

static const char * const authstr[MAX_AUTHID - 1] = {

    STRAUTHID_LANGUAGE,

    // Options
    STRAUTHID_KEYBOARD_LAYOUT,         // keyboard_layout
    STRAUTHID_VNC_CLIPBOARD_UP,        // clipboard_up
    STRAUTHID_VNC_CLIPBOARD_DOWN,      // clipboard_down
    STRAUTHID_OPT_FILE_ENCRYPTION,     // file encryption

    // Video capture
    STRAUTHID_OPT_CODEC_ID,     // CODEC_ID for video encoding
    STRAUTHID_OPT_MOVIE,        // is_rec
    STRAUTHID_OPT_MOVIE_PATH,   // rec_path
    STRAUTHID_VIDEO_QUALITY,

    // Alternate shell
    STRAUTHID_ALTERNATE_SHELL,
    STRAUTHID_SHELL_WORKING_DIRECTORY,

    // Context
    STRAUTHID_OPT_BITRATE,      // Bit rate for video encoding
    STRAUTHID_OPT_FRAMERATE,    // Frame rate for video encoding
    STRAUTHID_OPT_QSCALE,       // QScale parameter for vdeo encoding

    STRAUTHID_OPT_BPP,          // bits per planes (number of colors)
    STRAUTHID_OPT_HEIGHT,       // client height
    STRAUTHID_OPT_WIDTH,        // client width

    STRAUTHID_AUTH_ERROR_MESSAGE,

    STRAUTHID_SELECTOR,
    STRAUTHID_SELECTOR_CURRENT_PAGE,    // current page
    STRAUTHID_SELECTOR_DEVICE_FILTER,   // device filter text
    STRAUTHID_SELECTOR_GROUP_FILTER,    // group filter text
    STRAUTHID_SELECTOR_PROTO_FILTER,    // protocol filter text
    STRAUTHID_SELECTOR_LINES_PER_PAGE,  // number of lines per page
    STRAUTHID_SELECTOR_NUMBER_OF_PAGES, // number of pages

    STRAUTHID_TARGET_DEVICE,                // target_device
    STRAUTHID_TARGET_PASSWORD,              // target_password
    STRAUTHID_TARGET_HOST,                  // target_host
    STRAUTHID_TARGET_PORT,                  // target_port
    STRAUTHID_TARGET_PROTOCOL,              // proto_dest
    STRAUTHID_TARGET_USER,                  // target_login
    STRAUTHID_TARGET_APPLICATION,           // target_application
    STRAUTHID_TARGET_APPLICATION_ACCOUNT,   // target_application_account
    STRAUTHID_TARGET_APPLICATION_PASSWORD,  // target_application_password

    STRAUTHID_AUTH_USER,        // login
    STRAUTHID_HOST,             // ip_client
    STRAUTHID_TARGET,           // ip_target
    STRAUTHID_PASSWORD,         // password

    STRAUTHID_REPORTING,        // reporting message (client -> server)

    STRAUTHID_AUTHCHANNEL_ANSWER,   // WabLauncher target answer
    STRAUTHID_AUTHCHANNEL_RESULT,   // WabLauncher session result
    STRAUTHID_AUTHCHANNEL_TARGET,   // WabLauncher target request

    STRAUTHID_MESSAGE,        // warning_message
    STRAUTHID_PATTERN_KILL,   // regex to close connexion
    STRAUTHID_PATTERN_NOTIFY, // regex to notify to authentifier

    STRAUTHID_ACCEPT_MESSAGE,   // display a dialog to valid a message
    STRAUTHID_DISPLAY_MESSAGE,  // display a dialog box with a message

    STRAUTHID_AUTHENTICATED,
    STRAUTHID_REJECTED,         // rejected

    STRAUTHID_KEEPALIVE,

    STRAUTHID_SESSION_ID,   // session_id

    STRAUTHID_END_DATE_CNX, // timeclose
    STRAUTHID_END_TIME,     // end time as text

    STRAUTHID_MODE_CONSOLE,
    STRAUTHID_TIMEZONE,

    STRAUTHID_REAL_TARGET_DEVICE,

    STRAUTHID_AUTHENTICATION_CHALLENGE,

    STRAUTHID_MODULE,
    STRAUTHID_FORCEMODULE,
    STRAUTHID_TICKET,
    STRAUTHID_COMMENT,
    STRAUTHID_DURATION,
    STRAUTHID_WAITINFORETURN,
    STRAUTHID_SHOWFORM,
    STRAUTHID_FORMFLAG,

    STRAUTHID_DISABLE_TSK_SWITCH_SHORTCUTS,

    STRAUTHID_PROXY_OPT,

    STRAUTHID_DISABLE_KEYBOARD_LOG,
    STRAUTHID_DISABLE_CLIPBOARD_LOG,

    STRAUTHID_RT_DISPLAY,

    STRAUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE,

    STRAUTHID_VNC_BOGUS_CLIPBOARD_INFINITE_LOOP,

    STRAUTHID_RDP_BOGUS_SC_NET_SIZE,

    STRAUTHID_OPT_WABAGENT,
    STRAUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT,
    STRAUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT,

    STRAUTHID_OPT_CLIENT_DEVICE_ANNOUNCE_TIMEOUT,

    STRAUTHID_OPT_PROXY_MANAGED_DRIVES
};

}

#endif

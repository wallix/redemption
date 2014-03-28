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
  Copyright (C) Wallix 2012
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

  configuration file,
  parsing config file rdpproxy.ini
*/

#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include <dirent.h>
#include <stdio.h>

#include "log.hpp"

#include <istream>
#include <string>
#include <stdint.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <fileutils.hpp>
#include <string.hpp>

TODO("move SHARE_PATH to configuration (still used in front, checkfiles, session, transparent, some internal mods)");
#if !defined(SHARE_PATH)
#define SHARE_PATH "/usr/local/share/rdpproxy"
#endif

TODO("move CFG_PATH to configuration (still used in main and sockettransport)");
#if !defined(CFG_PATH)
#define CFG_PATH "/etc/rdpproxy"
#endif

TODO("move PID_PATH to configuration");
#if !defined(PID_PATH)
#define PID_PATH "/var/run"
#endif

TODO("move LICENSE_PATH to configuration (still used in mod/rdp and mod/rdp_transparent)");
#if !defined(LICENSE_PATH)
#define LICENSE_PATH "/var/certs/redemption"
#endif

#if !defined(FLV_PATH)
#define FLV_PATH "/tmp"
#endif

#if !defined(OCR_PATH)
#define OCR_PATH "/tmp"
#endif

#if !defined(PNG_PATH)
#define PNG_PATH "/tmp"
#endif

#if !defined(WRM_PATH)
#define WRM_PATH "/tmp"
#endif

TODO("move HASH_PATH to configuration (still used in front and recorder)");
#if !defined(HASH_PATH)
#define HASH_PATH "/tmp/hash"
#endif

TODO("move CERTIF_PATH to configuration (still used in sockettransport)");
#if !defined(CERTIF_PATH)
#define CERTIF_PATH "/etc/rdpproxy/certificate"
#endif

#if !defined(RECORD_PATH)
#define RECORD_PATH "/var/rdpproxy/recorded"
#endif

#if !defined(PERSISTENT_PATH)
#define PERSISTENT_PATH "/var/lib/redemption/cache"
#endif

#if !defined(RECORD_TMP_PATH)
#define RECORD_TMP_PATH "/var/rdpproxy/tmp"
#endif

TODO("move these into configuration");
#define LOGIN_LOGO24 "ad24b.png"
#define LOGIN_WAB_BLUE "wablogoblue.png"
#define LOGO_PNG "logo.png"
#define LOGO_BMP "logo.bmp"
#define HELP_ICON "helpicon.png"
#define CURSOR0 "cursor0.cur"
#define CURSOR1 "cursor1.cur"
#define FONT1 "sans-10.fv1"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.png"
#define LOCKFILE "rdpproxy.pid"


#define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"
#define THEME_INI    "theme.ini"

#define RDPPROXY_CRT "rdpproxy.crt"
#define RDPPROXY_KEY "rdpproxy.key"
#define DH1024_PEM "dh1024.pem"
#define DH2048_PEM "dh2048.pem"
#define DH_PEM DH2048_PEM

using namespace std;

static inline bool check_name(const char * str)
{
    return ((strlen(str) > 0) && (strlen(str) < 250));
}

static inline bool check_ask(const char * str)
{
    return (0 == strcmp(str, "ask"));
}

static inline void ask_string(const char * str, char buffer[], bool & flag)
{
    flag = check_ask(str);
    if (!flag) {
        strncpy(buffer, str, strlen(str));
        buffer[strlen(str)] = 0;
    }
    else {
        buffer[0] = 0;
    }
}

struct IniAccounts {
    char accountname[255];
    char username[255]; // should use string
    char password[255]; // should use string
};

typedef enum
    {
        AUTHID_UNKNOWN = 0,

        // Translation text
        AUTHID_TRANS_BUTTON_OK,
        AUTHID_TRANS_BUTTON_CANCEL,
        AUTHID_TRANS_BUTTON_HELP,
        AUTHID_TRANS_BUTTON_CLOSE,
        AUTHID_TRANS_BUTTON_REFUSED,
        AUTHID_TRANS_LOGIN,
        AUTHID_TRANS_USERNAME,
        AUTHID_TRANS_PASSWORD,
        AUTHID_TRANS_TARGET,
        AUTHID_TRANS_DIAGNOSTIC,
        AUTHID_TRANS_CONNECTION_CLOSED,
        AUTHID_TRANS_HELP_MESSAGE,
        AUTHID_TRANS_MANAGER_CLOSE_CNX,

        AUTHID_LANGUAGE,

        // Options
        AUTHID_KEYBOARD_LAYOUT,         // keyboard_layout
        AUTHID_OPT_CLIPBOARD,           // clipboard
        AUTHID_OPT_DEVICEREDIRECTION,   // device_redirection
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

        AUTHID_TARGET_DEVICE,       // target_device
        AUTHID_TARGET_PASSWORD,     // target_password
        AUTHID_TARGET_PORT,         // target_port
        AUTHID_TARGET_PROTOCOL,     // proto_dest
        AUTHID_TARGET_USER,         // target_login
        AUTHID_TARGET_APPLICATION,  // target_application

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
        AUTHID_PROXY_TYPE,

        AUTHID_TRACE_SEAL,      // after closing trace file trace is sealed using a signature hash

        AUTHID_SESSION_ID,      // session_id

        AUTHID_END_DATE_CNX,    // timeclose
        AUTHID_END_TIME,        // end time as text

        AUTHID_MODE_CONSOLE,
        AUTHID_TIMEZONE,

        AUTHID_REAL_TARGET_DEVICE,  // target device in ip transparent mode

        AUTHID_AUTHENTICATION_CHALLENGE,

        AUTHID_MODULE,
        AUTHID_TICKET,
        AUTHID_COMMENT,

        AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS,

        AUTHID_DISABLE_KEYBOARD_LOG,

        MAX_AUTHID
    } authid_t;

// Translation text
#define STRAUTHID_TRANS_BUTTON_OK          "trans_ok"
#define STRAUTHID_TRANS_BUTTON_CANCEL      "trans_cancel"
#define STRAUTHID_TRANS_BUTTON_HELP        "trans_help"
#define STRAUTHID_TRANS_BUTTON_CLOSE       "trans_close"
#define STRAUTHID_TRANS_BUTTON_REFUSED     "trans_refused"
#define STRAUTHID_TRANS_LOGIN              "trans_login"
#define STRAUTHID_TRANS_USERNAME           "trans_username"
#define STRAUTHID_TRANS_PASSWORD           "trans_password"
#define STRAUTHID_TRANS_TARGET             "trans_target"
#define STRAUTHID_TRANS_DIAGNOSTIC         "trans_diagnostic"
#define STRAUTHID_TRANS_CONNECTION_CLOSED  "trans_connection_closed"
#define STRAUTHID_TRANS_HELP_MESSAGE       "trans_help_message"
#define STRAUTHID_TRANS_MANAGER_CLOSE_CNX  "trans_manager_close_cnx"
#define STRAUTHID_LANGUAGE                 "language"
// Options
#define STRAUTHID_KEYBOARD_LAYOUT          "keyboard_layout"
#define STRAUTHID_OPT_CLIPBOARD            "clipboard"
#define STRAUTHID_OPT_DEVICEREDIRECTION    "device_redirection"
#define STRAUTHID_OPT_FILE_ENCRYPTION      "file_encryption"
// Video capture
#define STRAUTHID_OPT_CODEC_ID             "codec_id"
#define STRAUTHID_OPT_MOVIE                "is_rec"
#define STRAUTHID_OPT_MOVIE_PATH           "rec_path"
#define STRAUTHID_VIDEO_QUALITY            "video_quality"
// Alternate shell
#define STRAUTHID_ALTERNATE_SHELL          "alternate_shell"
#define STRAUTHID_SHELL_WORKING_DIRECTORY  "shell_working_directory"
// Context
#define STRAUTHID_OPT_BITRATE              "bitrate"
#define STRAUTHID_OPT_FRAMERATE            "framerate"
#define STRAUTHID_OPT_QSCALE               "qscale"
#define STRAUTHID_OPT_BPP                  "bpp"
#define STRAUTHID_OPT_HEIGHT               "height"
#define STRAUTHID_OPT_WIDTH                "width"

#define STRAUTHID_AUTH_ERROR_MESSAGE       "error_message"

#define STRAUTHID_SELECTOR                 "selector"
#define STRAUTHID_SELECTOR_CURRENT_PAGE    "selector_current_page"
#define STRAUTHID_SELECTOR_DEVICE_FILTER   "selector_device_filter"
#define STRAUTHID_SELECTOR_GROUP_FILTER    "selector_group_filter"
#define STRAUTHID_SELECTOR_PROTO_FILTER    "selector_proto_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE  "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"

#define STRAUTHID_TARGET_DEVICE            "target_device"
#define STRAUTHID_TARGET_PASSWORD          "target_password"
#define STRAUTHID_TARGET_PORT              "target_port"
#define STRAUTHID_TARGET_PROTOCOL          "proto_dest"
#define STRAUTHID_TARGET_USER              "target_login"
#define STRAUTHID_TARGET_APPLICATION       "target_application"

#define STRAUTHID_AUTH_USER                "login"
#define STRAUTHID_HOST                     "ip_client"
#define STRAUTHID_TARGET                   "ip_target"
#define STRAUTHID_PASSWORD                 "password"

#define STRAUTHID_REPORTING                "reporting"

#define STRAUTHID_AUTHCHANNEL_ANSWER       "auth_channel_answer"
#define STRAUTHID_AUTHCHANNEL_RESULT       "auth_channel_result"
#define STRAUTHID_AUTHCHANNEL_TARGET       "auth_channel_target"

#define STRAUTHID_MESSAGE                  "message"
#define STRAUTHID_PATTERN_KILL             "pattern_kill"
#define STRAUTHID_PATTERN_NOTIFY           "pattern_notify"

#define STRAUTHID_ACCEPT_MESSAGE           "accept_message"
#define STRAUTHID_DISPLAY_MESSAGE          "display_message"

#define STRAUTHID_AUTHENTICATED            "authenticated"
#define STRAUTHID_REJECTED                 "rejected"

#define STRAUTHID_KEEPALIVE                "keepalive"
#define STRAUTHID_PROXY_TYPE               "proxy_type"

#define STRAUTHID_TRACE_SEAL               "trace_seal"

#define STRAUTHID_SESSION_ID               "session_id"

#define STRAUTHID_END_DATE_CNX             "timeclose"
#define STRAUTHID_END_TIME                 "end_time"

#define STRAUTHID_MODE_CONSOLE             "mode_console"
#define STRAUTHID_TIMEZONE                 "timezone"

#define STRAUTHID_REAL_TARGET_DEVICE       "real_target_device"

#define STRAUTHID_AUTHENTICATION_CHALLENGE "authentication_challenge"
#define STRAUTHID_MODULE                   "module"
#define STRAUTHID_TICKET                   "ticket"
#define STRAUTHID_COMMENT                  "comment"
#define STRAUTHID_DISABLE_TSK_SWITCH_SHORTCUTS        "disable_tsk_switch_shortcuts"

#define STRAUTHID_DISABLE_KEYBOARD_LOG     "disable_keyboard_log"

static const std::string authstr[MAX_AUTHID - 1] = {
    // Translation text
    STRAUTHID_TRANS_BUTTON_OK,
    STRAUTHID_TRANS_BUTTON_CANCEL,
    STRAUTHID_TRANS_BUTTON_HELP,
    STRAUTHID_TRANS_BUTTON_CLOSE,
    STRAUTHID_TRANS_BUTTON_REFUSED,
    STRAUTHID_TRANS_LOGIN,
    STRAUTHID_TRANS_USERNAME,
    STRAUTHID_TRANS_PASSWORD,
    STRAUTHID_TRANS_TARGET,
    STRAUTHID_TRANS_DIAGNOSTIC,
    STRAUTHID_TRANS_CONNECTION_CLOSED,
    STRAUTHID_TRANS_HELP_MESSAGE,
    STRAUTHID_TRANS_MANAGER_CLOSE_CNX,

    STRAUTHID_LANGUAGE,

    // Options
    STRAUTHID_KEYBOARD_LAYOUT,         // keyboard_layout
    STRAUTHID_OPT_CLIPBOARD,            // clipboard
    STRAUTHID_OPT_DEVICEREDIRECTION,    // device_redirection
    STRAUTHID_OPT_FILE_ENCRYPTION,      // file encryption

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

    STRAUTHID_TARGET_DEVICE,        // target_device
    STRAUTHID_TARGET_PASSWORD,      // target_password
    STRAUTHID_TARGET_PORT,          // target_port
    STRAUTHID_TARGET_PROTOCOL,      // proto_dest
    STRAUTHID_TARGET_USER,          // target_login
    STRAUTHID_TARGET_APPLICATION,   // target_application

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
    STRAUTHID_PROXY_TYPE,

    STRAUTHID_TRACE_SEAL,   // after closing trace file trace is sealed using a signature hash

    STRAUTHID_SESSION_ID,   // session_id

    STRAUTHID_END_DATE_CNX, // timeclose
    STRAUTHID_END_TIME,     // end time as text

    STRAUTHID_MODE_CONSOLE,
    STRAUTHID_TIMEZONE,

    STRAUTHID_REAL_TARGET_DEVICE,

    STRAUTHID_AUTHENTICATION_CHALLENGE,

    STRAUTHID_MODULE,
    STRAUTHID_TICKET,
    STRAUTHID_COMMENT,

    STRAUTHID_DISABLE_TSK_SWITCH_SHORTCUTS,

    STRAUTHID_DISABLE_KEYBOARD_LOG,
};

static inline authid_t authid_from_string(const char * strauthid) {

    std::string str = std::string(strauthid);
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID - 1 ; i++) {
        if (0 == authstr[i].compare(str)) {
            res = static_cast<authid_t>(i + 1);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    if ((authid == AUTHID_UNKNOWN) || (authid >= MAX_AUTHID))
        return "";
    return authstr[static_cast<unsigned>(authid) - 1].c_str();
}

#include "basefield.hpp"
#include "theme.hpp"

struct Inifile : public FieldObserver {
    struct Inifile_globals {
        BoolField capture_chunk;

        StringField auth_user;           // AUTHID_AUTH_USER
        StringField host;                // client_ip AUTHID_HOST
        StringField target;              // target ip AUTHID_TARGET
        StringField target_device;       // AUTHID_TARGET_DEVICE
        StringField target_user;         // AUTHID_TARGET_USER
        StringField target_application;  // AUTHID_TARGET_APPLICATION

        // BEGIN globals
        bool bitmap_cache;                // default true
        int  port;                        // default 3389
        bool nomouse;
        bool notimestamp;
        int  encryptionLevel;             // 0=low, 1=medium, 2=high
        char authip[255];                 //
        int  authport;                    //
        bool autovalidate;                // dialog autovalidation for test

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;
        int close_timeout;                // timeout of close box in seconds (0 to desactivate)

        bool internal_domain;

        char      dynamic_conf_path[1024]; // directory where to look for dynamic configuration files
        char      auth_channel[512];
        BoolField enable_file_encryption;  // AUTHID_OPT_FILE_ENCRYPTION //
        char      listen_address[256];
        bool      enable_ip_transparent;
        char      certificate_password[256];

        char png_path[1024];
        char wrm_path[1024];

        StringField alternate_shell;          // STRAUTHID_ALTERNATE_SHELL //
        StringField shell_working_directory;  // STRAUTHID_SHELL_WORKING_DIRECTORY //

        StringField codec_id;                 // AUTHID_OPT_CODEC_ID //
        BoolField   movie;                    // AUTHID_OPT_MOVIE //
        StringField movie_path;               // AUTHID_OPT_MOVIE_PATH //
        StringField video_quality;            // AUTHID_VIDEO_QUALITY //
        bool        enable_bitmap_update;
        bool        enable_close_box;
        // END globals

        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval;

        char persistent_path[1024];
    } globals;

    // section "client"
    struct {
        UnsignedField keyboard_layout;    // AUTHID_KEYBOARD_LAYOUT
        bool ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default

        uint32_t performance_flags_default;
        uint32_t performance_flags_force_present;
        uint32_t performance_flags_force_not_present;

        bool tls_fallback_legacy;
        bool tls_support;
        bool bogus_neg_request; // needed to connect with jrdp, based on bogus X224 layer code

        BoolField clipboard;                // AUTHID_OPT_CLIPBOARD //
        BoolField device_redirection;       // AUTHID_OPT_DEVICEREDIRECTION //

        BoolField disable_tsk_switch_shortcuts; // AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS //

        int rdp_compression;    // 0 - Disabled, 1 - RDP 4.0, 2 - RDP 5.0, 3 - RDP 6.0, 4 - RDP 6.1

        uint32_t max_color_depth;   // 0 - Default (24-bit), 1 - 8-bit, 2 - 15-bit, 3 - 16-bit, 4 - 24-bit, 5 - 32-bit (not yet supported)

        bool persistent_disk_bitmap_cache;  // default false
        bool cache_waiting_list;            // default true

        bool bitmap_compression;            // default true
    } client;

    struct {
        int rdp_compression;    // 0 - Disabled, 1 - RDP 4.0, 2 - RDP 5.0, 3 - RDP 6.0, 4 - RDP 6.1

        bool disconnect_on_logon_user_change;

        uint32_t open_session_timeout;

        unsigned certificate_change_action; // 0 - Interrupt connection, 1 - Replace certificate then continue

        redemption::string extra_orders;

        bool enable_nla;
        bool enable_kerberos;

        bool persistent_disk_bitmap_cache;  // default false
        bool cache_waiting_list;            // default true
    } mod_rdp;

    struct
    {
        redemption::string encodings;

        bool allow_authentification_retries;
    } mod_vnc;

    // Section "video"
    struct {
        unsigned capture_flags;   // 1 PNG capture, 2 WRM
        // video opt from capture_flags
        bool     capture_png;
        bool     capture_wrm;
        bool     capture_flv;
        bool     capture_ocr;

        unsigned ocr_interval;
        bool     ocr_on_title_bar_only;
        unsigned ocr_max_unrecog_char_rate; // expressed in percentage,
                                            //     0   - all of characters need be recognized
                                            //     100 - accept all results

        unsigned png_interval;    // time between 2 png captures (in 1/10 seconds)
        unsigned capture_groupid;
        unsigned frame_interval;  // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval;  // time between 2 wrm movies (in seconds)
        unsigned png_limit;       // number of png captures to keep
        char     replay_path[1024];

        int l_bitrate;            // bitrate for low quality
        int l_framerate;          // framerate for low quality
        int l_height;             // height for low quality
        int l_width;              // width for low quality
        int l_qscale;             // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        int m_bitrate;
        int m_framerate;
        int m_height;
        int m_width;
        int m_qscale;

        // Same for high quality
        int h_bitrate;
        int h_framerate;
        int h_height;
        int h_width;
        int h_qscale;

        char hash_path[1024];
        char record_tmp_path[1024];
        char record_path[1024];

        bool     inactivity_pause;
        unsigned inactivity_timeout;

        // 1 - Disable keyboard event logging in syslog
        // 2 - Disable keyboard event logging in WRM
        // 4 - Disable keyboard event logging in META
        UnsignedField disable_keyboard_log;    // AUTHID_DISABLE_KEYBOARD_LOG
        bool disable_keyboard_log_syslog;
        bool disable_keyboard_log_wrm;
        bool disable_keyboard_log_ocr;
    } video;

    // Section "Crypto"
    struct {
        char key0[32];
        char key1[32];
    } crypto;

    // Section "debug"
    struct {
        uint32_t x224;
        uint32_t mcs;
        uint32_t sec;
        uint32_t rdp;
        uint32_t primary_orders;
        uint32_t secondary_orders;
        uint32_t bitmap;
        uint32_t capture;
        uint32_t auth;
        uint32_t session;
        uint32_t front;
        uint32_t mod_rdp;
        uint32_t mod_vnc;
        uint32_t mod_int;
        uint32_t mod_xup;
        uint32_t widget;
        uint32_t input;

        uint32_t pass_dialog_box;
        int log_type;
        char log_file_path[1024]; // log file location
    } debug;

    // section "translation"
    struct {
        StringField button_ok;              // AUTHID_TRANS_BUTTON_OK
        StringField button_cancel;          // AUTHID_TRANS_BUTTON_CANCEL
        StringField button_help;            // AUTHID_TRANS_BUTTON_HELP
        StringField button_close;           // AUTHID_TRANS_BUTTON_CLOSE
        StringField button_refused;         // AUTHID_TRANS_BUTTON_REFUSED
        StringField login;                  // AUTHID_TRANS_LOGIN
        StringField username;               // AUTHID_TRANS_USERNAME
        StringField password;               // AUTHID_TRANS_PASSWORD
        StringField target;                 // AUTHID_TRANS_TARGET
        StringField diagnostic;             // AUTHID_TRANS_DIAGNOSTIC
        StringField connection_closed;      // AUTHID_TRANS_CONNECTION_CLOSED
        StringField help_message;           // AUTHID_TRANS_HELP_MESSAGE
        StringField manager_close_cnx;      // AUTHID_TRANS_MANAGER_CLOSE_CNX

        StringField language;
    } translation;

    // section "context"
    struct {
        unsigned           selector_focus;           // --
        char               movie[1024];              // --

        UnsignedField      opt_bitrate;              // AUTHID_OPT_BITRATE //
        UnsignedField      opt_framerate;            // AUTHID_OPT_FRAMERATE //
        UnsignedField      opt_qscale;               // AUTHID_OPT_QSCALE //

        UnsignedField      opt_bpp;                  // AUTHID_OPT_BPP //
        UnsignedField      opt_height;               // AUTHID_OPT_HEIGHT //
        UnsignedField      opt_width;                // AUTHID_OPT_WIDTH //

        // auth_error_message is left as redemption::string type
        // because SocketTransport and ReplayMod take it as argument on
        // constructor and modify it as a redemption::string
        redemption::string auth_error_message;       // AUTHID_AUTH_ERROR_MESSAGE --

        BoolField          selector;                 // AUTHID_SELECTOR //
        UnsignedField      selector_current_page;    // AUTHID_SELECTOR_CURRENT_PAGE //
        StringField        selector_device_filter;   // AUTHID_SELECTOR_DEVICE_FILTER //
        StringField        selector_group_filter;    // AUTHID_SELECTOR_GROUP_FILTER //
        StringField        selector_proto_filter;    // AUTHID_SELECTOR_PROTO_FILTER //
        UnsignedField      selector_lines_per_page;  // AUTHID_SELECTOR_LINES_PER_PAGE //
        UnsignedField      selector_number_of_pages; // AUTHID_SELECTOR_NUMBER_OF_PAGES //

        StringField        target_password;          // AUTHID_TARGET_PASSWORD //
        UnsignedField      target_port;              // AUTHID_TARGET_PORT //
        StringField        target_protocol;          // AUTHID_TARGET_PROTOCOL //

        StringField        password;                 // AUTHID_PASSWORD //

        StringField        reporting;                // AUTHID_REPORTING //

        StringField        authchannel_answer;       // AUTHID_AUTHCHANNEL_ANSWER //
        StringField        authchannel_result;       // AUTHID_AUTHCHANNEL_RESULT //
        StringField        authchannel_target;       // AUTHID_AUTHCHANNEL_TARGET //

        StringField        message;                  // AUTHID_MESSAGE //
        StringField        pattern_kill;             // AUTHID_ PATTERN_KILL //
        StringField        pattern_notify;           // AUTHID_ PATTERN_NOTIFY //

        TODO("why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button");
        StringField        accept_message;           // AUTHID_ACCEPT_MESSAGE //
        StringField        display_message;          // AUTHID_DISPLAY_MESSAGE //

        StringField        rejected;                 // AUTHID_REJECTED //

        BoolField          authenticated;            // AUTHID_AUTHENTICATED //

        BoolField          keepalive;                // AUTHID_KEEPALIVE //
        StringField        proxy_type;               // AUTHID_PROXY_TYPE //

        StringField        trace_seal;               // AUTHID_TRACE_SEAL //

        StringField        session_id;               // AUTHID_SESSION_ID //

        UnsignedField      end_date_cnx;             // AUTHID_END_DATE_CNX //
        StringField        end_time;                 // AUTHID_END_TIME //

        StringField        mode_console;             // AUTHID_MODE_CONSOLE //
        SignedField        timezone;                 // AUTHID_TIMEZONE //

        StringField        real_target_device;       // AUHTID_REAL_TARGET_DEVICE  //

        BoolField          authentication_challenge; // AUTHID_AUTHENTICATION_CHALLENGE //

        StringField        ticket;                   // AUTHID_TICKET //
        StringField        comment;                  // AUTHID_COMMENT //

        StringField        module;
    } context;

    Theme theme;

    struct IniAccounts account;

public:
    Inifile() : FieldObserver() {
        this->init();
    }

    void init() {
        //init to_send_set of authid
        this->to_send_set.insert(AUTHID_PROXY_TYPE);
        this->to_send_set.insert(AUTHID_DISPLAY_MESSAGE);
        this->to_send_set.insert(AUTHID_ACCEPT_MESSAGE);
        this->to_send_set.insert(AUTHID_PASSWORD);
        this->to_send_set.insert(AUTHID_REPORTING);
        this->to_send_set.insert(AUTHID_TARGET_PROTOCOL);
        this->to_send_set.insert(AUTHID_SELECTOR);
        this->to_send_set.insert(AUTHID_SELECTOR_GROUP_FILTER);
        this->to_send_set.insert(AUTHID_SELECTOR_DEVICE_FILTER);
        this->to_send_set.insert(AUTHID_SELECTOR_PROTO_FILTER);
        this->to_send_set.insert(AUTHID_SELECTOR_LINES_PER_PAGE);
        this->to_send_set.insert(AUTHID_SELECTOR_CURRENT_PAGE);
        this->to_send_set.insert(AUTHID_TARGET_PASSWORD);
        this->to_send_set.insert(AUTHID_OPT_WIDTH);
        this->to_send_set.insert(AUTHID_OPT_HEIGHT);
        this->to_send_set.insert(AUTHID_OPT_BPP);
        this->to_send_set.insert(AUTHID_REAL_TARGET_DEVICE);

        this->to_send_set.insert(AUTHID_AUTHCHANNEL_RESULT);
        this->to_send_set.insert(AUTHID_AUTHCHANNEL_TARGET);

        this->something_changed = false;

        //this->globals.capture_chunk = false;
        this->globals.capture_chunk.set(false);


        this->to_send_set.insert(AUTHID_AUTH_USER);
        this->globals.auth_user.attach_ini(this,              AUTHID_AUTH_USER);
        this->globals.auth_user.set_from_cstr("");

        this->to_send_set.insert(AUTHID_HOST);
        this->globals.host.attach_ini(this,                   AUTHID_HOST);
        this->globals.host.set_from_cstr("");

        this->to_send_set.insert(AUTHID_TARGET);
        this->globals.target.attach_ini(this,                 AUTHID_TARGET);
        this->globals.target.set_from_cstr("");

        this->to_send_set.insert(AUTHID_TARGET_USER);
        this->globals.target_user.attach_ini(this,            AUTHID_TARGET_USER);
        this->globals.target_user.set_from_cstr("");

        this->globals.target_application.attach_ini(this,     AUTHID_TARGET_APPLICATION);
        this->globals.target_application.set_from_cstr("");

        this->to_send_set.insert(AUTHID_TARGET_DEVICE);
        this->globals.target_device.attach_ini(this,          AUTHID_TARGET_DEVICE);
        this->globals.target_device.set_from_cstr("");

        this->globals.enable_file_encryption.attach_ini(this, AUTHID_OPT_FILE_ENCRYPTION);
        this->globals.enable_file_encryption.set(false);

        // Init globals
        this->globals.bitmap_cache = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_cstr("low");
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.autovalidate = false;

        this->globals.max_tick              = 30;
        this->globals.keepalive_grace_delay = 30;
        this->globals.close_timeout         = 600;       // timeout of close box set to 10 minutes

        this->globals.internal_domain = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
        memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);
        // this->globals.enable_file_encryption = false;
        strcpy(this->globals.listen_address, "0.0.0.0");
        this->globals.enable_ip_transparent  = false;
        strcpy(this->globals.certificate_password, "inquisition");

        strcpy(this->globals.png_path, PNG_PATH);
        strcpy(this->globals.wrm_path, WRM_PATH);

        this->globals.alternate_shell.attach_ini(this, AUTHID_ALTERNATE_SHELL);
        this->globals.shell_working_directory.attach_ini(this, AUTHID_SHELL_WORKING_DIRECTORY);

        this->globals.alternate_shell.set_empty();
        this->globals.shell_working_directory.set_empty();

        this->globals.codec_id.attach_ini(this, AUTHID_OPT_CODEC_ID);
        this->globals.movie.attach_ini(this, AUTHID_OPT_MOVIE);
        this->globals.movie_path.attach_ini(this, AUTHID_OPT_MOVIE_PATH);
        this->globals.video_quality.attach_ini(this, AUTHID_VIDEO_QUALITY);

        this->globals.codec_id.set_from_cstr("flv");
        this->globals.movie.set(false);
        this->globals.movie_path.set_empty();
        TODO("this could be some kind of enumeration");
        this->globals.video_quality.set_from_cstr("medium");
        this->globals.enable_bitmap_update = false;
        this->globals.enable_close_box = true;

        pathncpy(this->globals.persistent_path, PERSISTENT_PATH, sizeof(this->globals.persistent_path));
        // End Init globals

        this->globals.flv_break_interval = 600000000l;
        this->globals.flv_frame_interval = 1000000L;

        strcpy(this->account.accountname, "");
        strcpy(this->account.username,    "");
        strcpy(this->account.password,    "");

        // Begin Section "client".
        this->client.keyboard_layout.attach_ini(this, AUTHID_KEYBOARD_LAYOUT);
        this->client.keyboard_layout.set(0);
        this->to_send_set.insert(AUTHID_KEYBOARD_LAYOUT);

        this->client.clipboard.attach_ini(this,AUTHID_OPT_CLIPBOARD);
        this->client.device_redirection.attach_ini(this,AUTHID_OPT_DEVICEREDIRECTION);
        this->client.clipboard.set(true);
        this->client.device_redirection.set(true);

        this->client.ignore_logon_password               = false;
        //      this->client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->client.performance_flags_default           = 0;
        this->client.performance_flags_force_present     = 0;
        this->client.performance_flags_force_not_present = 0;
        this->client.tls_fallback_legacy                 = false;
        this->client.tls_support                         = true;
        this->client.bogus_neg_request                   = false;
        this->client.rdp_compression                     = 0;
        this->client.max_color_depth                     = 24;
        this->client.persistent_disk_bitmap_cache        = false;
        this->client.cache_waiting_list                  = true;

        this->client.disable_tsk_switch_shortcuts.attach_ini(this, AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS);
        this->client.disable_tsk_switch_shortcuts.set(false);

        this->client.bitmap_compression = true;
        // End Section "client"

        // Begin section "mod_rdp"
        this->mod_rdp.rdp_compression                   = 0;
        this->mod_rdp.disconnect_on_logon_user_change   = false;
        this->mod_rdp.enable_nla                        = true;
        this->mod_rdp.enable_kerberos                   = false;
        this->mod_rdp.open_session_timeout              = 0;
        this->mod_rdp.certificate_change_action         = 0;
        this->mod_rdp.persistent_disk_bitmap_cache      = false;
        this->mod_rdp.cache_waiting_list                = true;

        this->mod_rdp.extra_orders.empty();
        // End Section "mod_rdp"

        // Begin section "mod_vnc"
        this->mod_vnc.encodings.empty();

        this->mod_vnc.allow_authentification_retries = false;
        // End Section "mod_vnc"

        // Begin section video
        this->video.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->video.capture_wrm   = true;
        this->video.capture_png   = true;
        this->video.capture_flv   = false;
        this->video.capture_ocr   = false;

        this->video.ocr_interval                = 100;      // 1 every second
        this->video.ocr_on_title_bar_only       = false;
        this->video.ocr_max_unrecog_char_rate   = 40;

        this->video.png_interval    = 3000;
        this->video.capture_groupid = 33;
        this->video.frame_interval  = 40;         // 2,5 frame per second
        this->video.break_interval  = 600;        // 10 minutes interval
        this->video.png_limit       = 3;
        strcpy(this->video.replay_path, "/tmp/");

        this->video.l_bitrate   = 20000;
        this->video.l_framerate = 5;
        this->video.l_height    = 480;
        this->video.l_width     = 640;
        this->video.l_qscale    = 25;
        this->video.m_bitrate   = 40000;
        this->video.m_framerate = 5;
        this->video.m_height    = 768;
        this->video.m_width     = 1024;
        this->video.m_qscale    = 15;
        this->video.h_bitrate   = 200000;
        this->video.h_framerate = 5;
        this->video.h_height    = 1024;
        this->video.h_width     = 1280;
        this->video.h_qscale    = 15;

        pathncpy(this->video.hash_path,       HASH_PATH,       sizeof(this->video.hash_path));
        pathncpy(this->video.record_path,     RECORD_PATH,     sizeof(this->video.record_path));
        pathncpy(this->video.record_tmp_path, RECORD_TMP_PATH, sizeof(this->video.record_tmp_path));

        this->video.inactivity_pause   = false;
        this->video.inactivity_timeout = 300;

        this->video.disable_keyboard_log.attach_ini(this, AUTHID_DISABLE_KEYBOARD_LOG);
        this->video.disable_keyboard_log.set(0);
        this->to_send_set.insert(AUTHID_DISABLE_KEYBOARD_LOG);
        this->video.disable_keyboard_log_syslog = false;
        this->video.disable_keyboard_log_wrm    = false;
        this->video.disable_keyboard_log_ocr    = false;
        // End section "video"

        // Init crypto
        memcpy(this->crypto.key0,
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
            sizeof(this->crypto.key0));

        memcpy(this->crypto.key1,
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
            sizeof(this->crypto.key1));
        // End Init crypto

        // Begin Section "debug".
        this->debug.x224              = 0;
        this->debug.mcs               = 0;
        this->debug.sec               = 0;
        this->debug.rdp               = 0;
        this->debug.primary_orders    = 0;
        this->debug.secondary_orders  = 0;
        this->debug.bitmap            = 0;
        this->debug.capture           = 0;
        this->debug.auth              = 0;
        this->debug.session           = 0;
        this->debug.front             = 0;
        this->debug.mod_rdp           = 0;
        this->debug.mod_vnc           = 0;
        this->debug.mod_int           = 0;
        this->debug.mod_xup           = 0;
        this->debug.widget            = 0;
        this->debug.input             = 0;

        this->debug.log_type          = 2; // syslog by default
        this->debug.log_file_path[0]  = 0;
        this->debug.pass_dialog_box   = 0;
        // End Section "debug"

        // Begin Section "translation"
        this->translation.button_ok.set_from_cstr("OK");
        this->translation.button_ok.attach_ini(this, AUTHID_TRANS_BUTTON_OK);

        this->translation.button_cancel.set_from_cstr("Cancel");
        this->translation.button_cancel.attach_ini(this, AUTHID_TRANS_BUTTON_CANCEL);

        this->translation.button_help.set_from_cstr("Help");
        this->translation.button_help.attach_ini(this, AUTHID_TRANS_BUTTON_HELP);

        this->translation.button_close.set_from_cstr("Close");
        this->translation.button_close.attach_ini(this, AUTHID_TRANS_BUTTON_CLOSE);

        this->translation.button_refused.set_from_cstr("Refused");
        this->translation.button_refused.attach_ini(this, AUTHID_TRANS_BUTTON_REFUSED);

        this->translation.login.set_from_cstr("Login");
        this->translation.login.attach_ini(this, AUTHID_TRANS_LOGIN);

        this->translation.username.set_from_cstr("Username");
        this->translation.username.attach_ini(this, AUTHID_TRANS_USERNAME);

        this->translation.password.set_from_cstr("Password");
        this->translation.password.attach_ini(this, AUTHID_TRANS_PASSWORD);

        this->translation.target.set_from_cstr("Target");
        this->translation.target.attach_ini(this, AUTHID_TRANS_TARGET);

        this->translation.diagnostic.set_from_cstr("Diagnostic");
        this->translation.diagnostic.attach_ini(this, AUTHID_TRANS_DIAGNOSTIC);

        this->translation.connection_closed.set_from_cstr("Connection closed");
        this->translation.connection_closed.attach_ini(this, AUTHID_TRANS_CONNECTION_CLOSED);

        this->translation.help_message.set_from_cstr("Help message");
        this->translation.help_message.attach_ini(this, AUTHID_TRANS_HELP_MESSAGE);

        this->translation.manager_close_cnx.set_from_cstr("Connection closed by manager");
        this->translation.manager_close_cnx.attach_ini(this, AUTHID_TRANS_MANAGER_CLOSE_CNX);

        this->translation.language.set_from_cstr("en");
        this->translation.language.attach_ini(this,AUTHID_LANGUAGE);
        // End Section "translation"

        // Begin section "context"

        this->context.selector_focus              = 0;
        this->context.movie[0]                    = 0;


        this->context.opt_bitrate.set(40000);
        this->context.opt_framerate.set(5);
        this->context.opt_qscale.set(15);

        this->context.opt_bitrate.attach_ini(this,AUTHID_OPT_BITRATE);
        this->context.opt_framerate.attach_ini(this,AUTHID_OPT_FRAMERATE);
        this->context.opt_qscale.attach_ini(this,AUTHID_OPT_QSCALE);

        this->context.opt_bpp.set(24);
        this->context.opt_height.set(600);
        this->context.opt_width.set(800);

        this->context.auth_error_message.empty();

        this->context.selector.set(false);
        this->context.selector_current_page.set(1);
        this->context.selector_device_filter.set_empty();
        this->context.selector_group_filter.set_empty();
        this->context.selector_proto_filter.set_empty();
        this->context.selector_lines_per_page.set(0);
        this->context.selector_number_of_pages.set(1);

        this->context.selector_number_of_pages.attach_ini(this, AUTHID_SELECTOR_NUMBER_OF_PAGES);

        this->globals.target_device.ask();
        this->globals.target_user.ask();
        this->globals.auth_user.ask();


        this->context.target_password.set_empty();
        this->context.target_password.ask();

        this->context.target_port.set(3389);
        this->context.target_port.ask();


        this->context.target_protocol.set_from_cstr("RDP");
        this->context.target_protocol.ask();

        this->context.password.set_empty();
        this->context.password.ask();

        this->context.reporting.set_empty();

        this->context.authchannel_answer.set_empty();
        this->context.authchannel_result.set_empty();
        this->context.authchannel_target.set_empty();


        this->context.message.set_empty();
        this->context.message.attach_ini(this, AUTHID_MESSAGE);

        this->context.pattern_kill.set_empty();
        this->context.pattern_kill.attach_ini(this, AUTHID_PATTERN_KILL);

        this->context.pattern_notify.set_empty();
        this->context.pattern_notify.attach_ini(this, AUTHID_PATTERN_NOTIFY);

        this->context.accept_message.set_empty();
        this->context.display_message.set_empty();

        this->context.rejected.set_empty();
        this->context.rejected.attach_ini(this, AUTHID_REJECTED);

        this->context.authenticated.set(false);
        this->context.authenticated.attach_ini(this, AUTHID_AUTHENTICATED);

        this->context.keepalive.set(false);

        this->context.proxy_type.set_from_cstr("RDP");

        this->context.trace_seal.set_empty();

        this->context.session_id.set_empty();
        this->context.session_id.attach_ini(this, AUTHID_SESSION_ID);

        this->context.end_date_cnx.set(0);
        this->context.end_time.set_empty();
        this->context.end_date_cnx.attach_ini(this, AUTHID_END_DATE_CNX);
        this->context.end_time.attach_ini(this, AUTHID_END_TIME);

        this->context.mode_console.set_from_cstr("allow");
        this->context.timezone.set(-3600);
        this->context.mode_console.attach_ini(this, AUTHID_MODE_CONSOLE);
        this->context.timezone.attach_ini(this, AUTHID_TIMEZONE);

        this->context.real_target_device.set_empty();

        this->context.authentication_challenge.ask();
        this->context.authentication_challenge.attach_ini(this, AUTHID_AUTHENTICATION_CHALLENGE);

        this->to_send_set.insert(AUTHID_MODULE);
        this->to_send_set.insert(AUTHID_TICKET);
        this->to_send_set.insert(AUTHID_COMMENT);

        this->context.module.set_from_cstr("login");
        this->context.module.attach_ini(this, AUTHID_MODULE);
        this->context.module.use();

        this->context.ticket.set_from_cstr("");
        this->context.ticket.attach_ini(this, AUTHID_TICKET);
        this->context.ticket.use();
        this->context.comment.set_from_cstr("");
        this->context.comment.attach_ini(this, AUTHID_COMMENT);
        this->context.comment.use();

        // Attaching ini struct to values
        this->context.opt_bpp.attach_ini(this,AUTHID_OPT_BPP);
        this->context.opt_height.attach_ini(this,AUTHID_OPT_HEIGHT);
        this->context.opt_width.attach_ini(this,AUTHID_OPT_WIDTH);

        this->context.selector.attach_ini(this,AUTHID_SELECTOR);
        this->context.selector_current_page.attach_ini(this,AUTHID_SELECTOR_CURRENT_PAGE);
        this->context.selector_device_filter.attach_ini(this,AUTHID_SELECTOR_DEVICE_FILTER);
        this->context.selector_group_filter.attach_ini(this,AUTHID_SELECTOR_GROUP_FILTER);
        this->context.selector_proto_filter.attach_ini(this,AUTHID_SELECTOR_PROTO_FILTER);
        this->context.selector_lines_per_page.attach_ini(this,AUTHID_SELECTOR_LINES_PER_PAGE);

        this->context.target_password.attach_ini(this,AUTHID_TARGET_PASSWORD);
        this->context.target_protocol.attach_ini(this,AUTHID_TARGET_PROTOCOL);
        this->context.target_port.attach_ini(this,AUTHID_TARGET_PORT);

        this->context.password.attach_ini(this,AUTHID_PASSWORD);

        this->context.reporting.attach_ini(this,AUTHID_REPORTING);

        this->context.accept_message.attach_ini(this,AUTHID_ACCEPT_MESSAGE);
        this->context.display_message.attach_ini(this,AUTHID_DISPLAY_MESSAGE);

        this->context.proxy_type.attach_ini(this,AUTHID_PROXY_TYPE);
        this->context.real_target_device.attach_ini(this,AUTHID_REAL_TARGET_DEVICE);

        this->context.authchannel_answer.attach_ini(this,AUTHID_AUTHCHANNEL_ANSWER);
        this->context.authchannel_target.attach_ini(this,AUTHID_AUTHCHANNEL_TARGET);
        this->context.authchannel_result.attach_ini(this,AUTHID_AUTHCHANNEL_RESULT);
        this->context.keepalive.attach_ini(this,AUTHID_KEEPALIVE);
        this->context.trace_seal.attach_ini(this,AUTHID_TRACE_SEAL);


    };

    virtual void set_value(const char * context, const char * key, const char * value)
    {
        if (0 == strcmp(context, "globals")) {
            if (0 == strcmp(key, "bitmap_cache")) {
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "port")) {
                this->globals.port = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "nomouse")) {
                this->globals.nomouse = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "notimestamp")) {
                this->globals.notimestamp = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "encryptionLevel")) {
                this->globals.encryptionLevel = level_from_cstr(value);
            }
            else if (0 == strcmp(key, "authip")) {
                strncpy(this->globals.authip, value, sizeof(this->globals.authip));
                this->globals.authip[sizeof(this->globals.authip) - 1] = 0;
            }
            else if (0 == strcmp(key, "authport")) {
                this->globals.authport = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "autovalidate")) {
                this->globals.autovalidate = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_tick") ||
                     0 == strcmp(key, "inactivity_time")) {
                this->globals.max_tick = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")) {
                this->globals.keepalive_grace_delay = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "close_timeout")) {
                this->globals.close_timeout = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "internal_domain")) {
                this->globals.internal_domain = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "dynamic_conf_path")) {
                strncpy(this->globals.dynamic_conf_path, value, sizeof(this->globals.dynamic_conf_path));
                this->globals.dynamic_conf_path[sizeof(this->globals.dynamic_conf_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "auth_channel")) {
                strncpy(this->globals.auth_channel, value, 8);
                this->globals.auth_channel[7] = 0;
            }
            else if (0 == strcmp(key, "enable_file_encryption")) {
                this->globals.enable_file_encryption.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "listen_address")) {
                strncpy(this->globals.listen_address, value, sizeof(this->globals.listen_address));
                this->globals.listen_address[sizeof(this->globals.listen_address) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_ip_transparent")) {
                this->globals.enable_ip_transparent = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_password")) {
                strncpy(this->globals.certificate_password, value, sizeof(this->globals.certificate_password));
                this->globals.certificate_password[sizeof(this->globals.certificate_password) - 1] = 0;
            }
            else if (0 == strcmp(key, "png_path")) {
                strncpy(this->globals.png_path, value, sizeof(this->globals.png_path));
                this->globals.png_path[sizeof(this->globals.png_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "wrm_path")) {
                strncpy(this->globals.wrm_path, value, sizeof(this->globals.wrm_path));
                this->globals.wrm_path[sizeof(this->globals.wrm_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "alternate_shell")) {
                this->globals.alternate_shell.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "shell_working_directory")) {
                this->globals.shell_working_directory.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "codec_id")) {
                this->globals.codec_id.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie")) {
                this->globals.movie.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie_path")) {
                this->globals.movie_path.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "video_quality")) {
                this->globals.video_quality.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_bitmap_update")) {
                this->globals.enable_bitmap_update = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_close_box")) {
                this->globals.enable_close_box = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "persistent_path")) {
                pathncpy(this->globals.persistent_path, value, sizeof(this->globals.persistent_path));
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "client")) {
            if (0 == strcmp(key, "ignore_logon_password")) {
                this->client.ignore_logon_password = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_default")) {
                this->client.performance_flags_default = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_present")) {
                this->client.performance_flags_force_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_not_present")) {
                this->client.performance_flags_force_not_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_fallback_legacy")) {
                this->client.tls_fallback_legacy = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_support")) {
                this->client.tls_support = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bogus_neg_request")) {
                this->client.bogus_neg_request = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard")) {
                this->client.clipboard.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "device_redirection")) {
                this->client.device_redirection.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp_compression")) {
                this->client.rdp_compression = ulong_from_cstr(value);
                if (this->client.rdp_compression < 0)
                    this->client.rdp_compression = 0;
                else if (this->client.rdp_compression > 4)
                    this->client.rdp_compression = 4;
            }
            else if (0 == strcmp(key, "disable_tsk_switch_shortcuts")) {
                this->client.disable_tsk_switch_shortcuts.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_color_depth")) {
                this->client.max_color_depth = ulong_from_cstr(value);
                if ((this->client.max_color_depth != 8) &&
                    (this->client.max_color_depth != 15) &&
                    (this->client.max_color_depth != 16) &&
                    (this->client.max_color_depth != 24) &&
                    (this->client.max_color_depth != 32))
                    this->client.max_color_depth = 24;
            }
            else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
                this->client.persistent_disk_bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "cache_waiting_list")) {
                this->client.cache_waiting_list = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_compression")) {
                this->client.bitmap_compression = bool_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "mod_rdp")) {
            if (0 == strcmp(key, "rdp_compression")) {
                this->mod_rdp.rdp_compression = ulong_from_cstr(value);
                if (this->mod_rdp.rdp_compression < 0)
                    this->mod_rdp.rdp_compression = 0;
                else if (this->mod_rdp.rdp_compression > 4)
                    this->mod_rdp.rdp_compression = 4;
            }
            else if (0 == strcmp(key, "disconnect_on_logon_user_change")) {
                this->mod_rdp.disconnect_on_logon_user_change = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "open_session_timeout")) {
                this->mod_rdp.open_session_timeout = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_change_action")) {
                this->mod_rdp.certificate_change_action = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "extra_orders")) {
                this->mod_rdp.extra_orders.copy_c_str(value);
            }
            else if (0 == strcmp(key, "enable_nla")) {
                this->mod_rdp.enable_nla = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_kerberos")) {
                this->mod_rdp.enable_kerberos = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "persistent_disk_bitmap_cache")) {
                this->mod_rdp.persistent_disk_bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "cache_waiting_list")) {
                this->mod_rdp.cache_waiting_list = bool_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "mod_vnc")) {
            if (0 == strcmp(key, "encodings")) {
                this->mod_vnc.encodings.copy_c_str(value);
            }
            else if (0 == strcmp(key, "allow_authentification_retries")) {
                this->mod_vnc.allow_authentification_retries = bool_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "video")) {
            if (0 == strcmp(key, "capture_flags")) {
                this->video.capture_flags   = ulong_from_cstr(value);
                this->video.capture_png = 0 != (this->video.capture_flags & 1);
                this->video.capture_wrm = 0 != (this->video.capture_flags & 2);
                this->video.capture_flv = 0 != (this->video.capture_flags & 4);
                this->video.capture_ocr = 0 != (this->video.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")) {
                this->video.ocr_interval                = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "ocr_on_title_bar_only")) {
                this->video.ocr_on_title_bar_only       = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "ocr_max_unrecog_char_rate")) {
                this->video.ocr_max_unrecog_char_rate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")) {
                this->video.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")) {
                this->video.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")) {
                this->video.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")) {
                this->video.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")) {
                this->video.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")) {
                strncpy(this->video.replay_path, value, sizeof(this->video.replay_path));
                this->video.replay_path[sizeof(this->video.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")) {
                this->video.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")) {
                this->video.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")) {
                this->video.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")) {
                this->video.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")) {
                this->video.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")) {
                this->video.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")) {
                this->video.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")) {
                this->video.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")) {
                this->video.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")) {
                this->video.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")) {
                this->video.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")) {
                this->video.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")) {
                this->video.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")) {
                this->video.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")) {
                this->video.h_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie_path")) {
                this->globals.movie_path.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "hash_path")) {
                pathncpy(this->video.hash_path,       value, sizeof(this->video.hash_path));
            }
            else if (0 == strcmp(key, "record_path")) {
                pathncpy(this->video.record_path,     value, sizeof(this->video.record_path));
            }
            else if (0 == strcmp(key, "record_tmp_path")) {
                pathncpy(this->video.record_tmp_path, value, sizeof(this->video.record_tmp_path));
            }
            else if (0 == strcmp(key, "disable_keyboard_log")) {
                this->video.disable_keyboard_log.set_from_cstr(value);
                this->video.disable_keyboard_log_syslog = 0 != (this->video.disable_keyboard_log.get() & 1);
                this->video.disable_keyboard_log_wrm    = 0 != (this->video.disable_keyboard_log.get() & 2);
                this->video.disable_keyboard_log_ocr    = 0 != (this->video.disable_keyboard_log.get() & 4);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "crypto")) {
            if (0 == strcmp(key, "key0")) {
                if (strlen(value) >= sizeof(this->crypto.key0) * 2) {
                    char   hexval[3] = { 0 };
                    char * end;
                    for (size_t i = 0; i < sizeof(this->crypto.key0); i++) {
                        memcpy(hexval, value + i * 2, 2);

                        this->crypto.key0[i] = strtol(hexval, &end, 16);
                    }
                }
            }
            else if (0 == strcmp(key, "key1")) {
                if (strlen(value) >= sizeof(this->crypto.key1) * 2) {
                    char   hexval[3] = { 0 };
                    char * end;
                    for (size_t i = 0; i < sizeof(this->crypto.key1); i++) {
                        memcpy(hexval, value + i * 2, 2);

                        this->crypto.key1[i] = strtol(hexval, &end, 16);
                    }
                }
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")) {
            if (0 == strcmp(key, "x224")) {
                this->debug.x224              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mcs")) {
                this->debug.mcs               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "sec")) {
                this->debug.sec               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp")) {
                this->debug.rdp               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "primary_orders")) {
                this->debug.primary_orders    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "secondary_orders")) {
                this->debug.secondary_orders  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap")) {
                this->debug.bitmap            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture")) {
                this->debug.capture           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth")) {
                this->debug.auth              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session")) {
                this->debug.session           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "front")) {
                this->debug.front             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_rdp")) {
                this->debug.mod_rdp           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_vnc")) {
                this->debug.mod_vnc           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_int")) {
                this->debug.mod_int           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_xup")) {
                this->debug.mod_xup           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "widget")) {
                this->debug.widget            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "input")) {
                this->debug.input             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_type")) {
                this->debug.log_type = logtype_from_cstr(value);
            }
            else if (0 == strcmp(key, "pass_dialog_box")) {
                this->debug.pass_dialog_box = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_file_path")) {
                strncpy(this->debug.log_file_path, value, sizeof(this->debug.log_file_path));
                this->debug.log_file_path[sizeof(this->debug.log_file_path) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "translation")) {
            if (0 == strcmp(key, "language")) {
                if ((0 == strcmp(value, "en")) ||
                    (0 == strcmp(value, "fr"))) {
                    this->translation.language.set_from_cstr(value);
                }
            }
            else if (0 == strcmp(key, "button_ok")) {
                this->translation.button_ok.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_cancel")) {
                this->translation.button_cancel.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_help")) {
                this->translation.button_help.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_close")) {
                this->translation.button_close.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_refused")) {
                this->translation.button_refused.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "login")) {
                this->translation.login.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "username")) {
                this->translation.username.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "password")) {
                this->translation.password.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "target")) {
                this->translation.target.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "diagnostic")) {
                this->translation.diagnostic.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "connection_closed")) {
                this->translation.connection_closed.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "help_message")) {
                this->translation.help_message.set_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "internal_mod")) {
            if (0 == strcmp(key, "load_theme")) {
                if (value) {
                    char theme_path[1024] = {};
                    snprintf(theme_path, 1024, CFG_PATH "/themes/%s/" THEME_INI, value);
                    theme_path[sizeof(theme_path) - 1] = 0;
                    ConfigurationLoader theme_load(this->theme, theme_path);
                    if (this->theme.global.logo) {
                        char logo_path[1024] = {};
                        snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_PNG, value);
                        logo_path[sizeof(logo_path) - 1] = 0;
                        if (!file_exist(logo_path)) {
                            snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_BMP,
                                     value);
                            logo_path[sizeof(logo_path) - 1] = 0;
                            if (!file_exist(logo_path)) {
                                this->theme.global.logo = false;
                                return;
                            }
                        }
                        this->theme.set_logo_path(logo_path);
                    }
                }
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }   // void set_value(const char * context, const char * key, const char * value)

    TODO("Should only be used by Authentifier "
         "It currently ask if the field has been modified "
         "and set it to not modified if it is not asked ")
    bool context_has_changed(authid_t authid) {
        bool res = false;
        try {
            BaseField * field = this->field_list.at(authid);
            res = field->has_changed();
            field->use();
        }
        catch (const std::out_of_range & oor) {
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            res = false;
        }
        return res;
    }

    /******************
     * Set_from_acl sets a value to corresponding field but does not mark it as changed
     */
    void set_from_acl(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            try {
                if (authid == AUTHID_AUTH_ERROR_MESSAGE)
                {
                    this->context.auth_error_message.copy_c_str(value);
                }
                else
                {
                    BaseField * field = this->field_list.at(authid);
                    field->set_from_acl(value);
                }
            }
            catch (const std::out_of_range & oor) {
                LOG(LOG_WARNING, "Inifile::set_from_acl(id): unknown authid=%d", authid);
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    /******************
     * ask_from_acl sets a value to corresponding field but does not mark it as changed
     */
    void ask_from_acl(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            try {
                BaseField * field = this->field_list.at(authid);
                field->ask_from_acl();
            }
            catch (const std::out_of_range & oor) {
                LOG(LOG_WARNING, "Inifile::ask_from_acl(id): unknown authid=%d", authid);
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::ask_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value_by_string(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            this->context_set_value(authid, value);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_set_value(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value(authid_t authid, const char * value) {
        // this->field_list.at(authid)->set_from_cstr(value);
        switch (authid)
            {
            // Context
            case AUTHID_AUTH_ERROR_MESSAGE:
                this->context.auth_error_message.copy_c_str(value);
                break;
            default:
                try {
                    BaseField * field = this->field_list.at(authid);
                    field->set_from_cstr(value);
                }
                catch (const std::out_of_range & oor) {
                    LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
                }
                break;
            }
    }

    const char * context_get_value_by_string(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_get_value(authid);
        }

        LOG(LOG_WARNING, "Inifile::context_get_value(strid): unknown strauthid=\"%s\"", strauthid);

        return "";
    }

    const char * context_get_value(authid_t authid) {
        const char * pszReturn = "";

//        LOG(LOG_INFO, "Getting value for authid=%d", authid);
        switch (authid)
            {
            case AUTHID_AUTH_ERROR_MESSAGE:
                pszReturn = this->context.auth_error_message.c_str();
                break;
            default:
                try{
                    BaseField * field = this->field_list.at(authid);
                    if (!field->is_asked())
                        pszReturn = field->get_value();
                }
                catch (const std::out_of_range & oor) {
                    LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
                }
                break;
            }

        return pszReturn;
    }

    void context_ask_by_string(const char *strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            context_ask(authid);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_ask(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_ask(authid_t authid) {
        try{
            this->field_list.at(authid)->ask();
        }
        catch (const std::out_of_range & oor) {
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
        }
    }

    bool context_is_asked_by_string(const char *strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_is_asked(authid);
        }

        LOG(LOG_WARNING, "Inifile::context_is_asked(strid): unknown strauthid=\"%s\"", strauthid);

        return false;
    }

    bool context_is_asked(authid_t authid) {
        bool res = false;
        try{
            res = this->field_list.at(authid)->is_asked();
        }
        catch (const std::out_of_range & oor) {
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
        }
        return res;
    }

    bool context_get_bool(authid_t authid) {
        TODO("ask related behavior is a problem. How do we make the difference between a False value and asked ?");
        switch (authid)
            {
            case AUTHID_SELECTOR:
                if (!this->context.selector.is_asked()) {
                    return this->context.selector.get();
                }
                break;
            case AUTHID_KEEPALIVE:
                if (!this->context.keepalive.is_asked()) {
                    return this->context.keepalive.get();
                }
                break;
            case AUTHID_AUTHENTICATED:
                return this->context.authenticated.get();
            case AUTHID_AUTHENTICATION_CHALLENGE:
                if (!this->context.authentication_challenge.is_asked()) {
                    return this->context.authentication_challenge.get();
                }
                break;
            default:
                LOG(LOG_WARNING, "Inifile::context_get_bool(id): unknown authid=\"%d\"", authid);
                break;
            }

        return false;
    }

    void parse_username(const char * username)
    {
/*
//        LOG(LOG_INFO, "parse_username(%s)", username);
        TODO("These should be results of the parsing function, not storing it away immediately in context. Mixing context management and parsing is not right");
        char target_user[256];
        char target_device[256];
        char target_protocol[256];
        char auth_user[256];
        target_user[0] = 0;
        target_device[0] = 0;
        target_protocol[0] = 0;
        auth_user[0] = 0;

        this->context_ask(AUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        if (username[0]){
            unsigned itarget_user = 0;
            unsigned itarget_device = 0;
            unsigned iprotocol = 0;
            unsigned iauthuser = 0;
            // well if that is not obvious the code below this
            // is a finite state automata that split login@host:protocol:authuser
            // between it's components parts.
            // ':' is forbidden in login, host or authuser.
            // '@' is forbidden in host or authuser.
            // login can contain an @ character (necessary because it is used
            // for domain names), the rule is that host follow the last @,
            // the login is what is before, even if it contains an @.
            // the protocol is what follows the first :
            // the user is what follows the second :, or what follows the unique : (if only one is found)

            enum { COPY_TARGET_USER
                   , COPY_HOST
                   , COPY_AUTHUSER
            } state = COPY_TARGET_USER;

            unsigned c;

            for (unsigned i = 0; i < 255 && (c = username[i]); i++){
                switch (state) {
                case COPY_TARGET_USER:
                    switch (c){
                    case ':': state = COPY_AUTHUSER;
                        break;
                    case '@': state = COPY_HOST;
                        break;
                    default: target_user[itarget_user++] = c;
                        break;
                    }
                    break;
                case COPY_HOST:
                    switch (c){
                    case ':': state = COPY_AUTHUSER;
                        break;
                    case '@':
                        target_user[itarget_user++] = '@';
                        memcpy(target_user+itarget_user, target_device, itarget_device);
                        itarget_user += itarget_device;
                        itarget_device = 0;
                        break;
                    default: target_device[itarget_device++] = c;
                        break;
                    }
                    break;
                case COPY_AUTHUSER:
                    switch (c){
                    case ':': // second ':' means we had 'protocol:user' pair
                        memcpy(target_protocol, auth_user, iauthuser);
                        iprotocol = iauthuser;
                        iauthuser = 0;
                        break;
                    default: auth_user[iauthuser++] = c;
                        break;
                    }
                    break;
                }
            }
            target_user[itarget_user] = 0;
            target_device[itarget_device] = 0;
            target_protocol[iprotocol] = 0;
            auth_user[iauthuser] = 0;
            if (iauthuser == 0){
                if ((itarget_user > 0) && (itarget_device == 0)){
                    memcpy(auth_user, target_user, itarget_user);
                    target_user[0] = 0;
                    auth_user[itarget_user] = 0;
                }
                if ((itarget_user > 0) && (itarget_device > 0)){
                    memcpy(auth_user, target_user, itarget_user);
                    target_user[0] = 0;
                    auth_user[itarget_user] = '@';
                    memcpy(auth_user + 1 + itarget_user, target_device, itarget_device);
                    target_device[0] = 0;
                    auth_user[itarget_user + 1 + itarget_device] = 0;
                }
            }
            // 'win:user' means user@win:user
            else if ((itarget_user != 0) && (itarget_device == 0)){
                memcpy(target_device, target_user, itarget_user);
                target_device[itarget_user] = 0;
                memcpy(target_user, auth_user, iauthuser);
                target_user[iauthuser] = 0;
            }
        }

        if (*target_user == 0) {
            this->context_ask(AUTHID_TARGET_USER);
        }
        else {
            this->context_set_value(AUTHID_TARGET_USER, target_user);
        }
        if (*target_device == 0) {
            this->context_ask(AUTHID_TARGET_DEVICE);
        }
        else {
            this->context_set_value(AUTHID_TARGET_DEVICE, target_device);
        }
        if (*target_protocol == 0) {
            this->context_ask(AUTHID_TARGET_PROTOCOL);
        }
        else {
            this->context_set_value(AUTHID_TARGET_PROTOCOL, target_protocol);
        }
        if (*auth_user == 0) {
            this->context_ask(AUTHID_AUTH_USER);
        }
        else {
            this->context_set_value(AUTHID_AUTH_USER, auth_user);
        }
*/
        this->context_ask(AUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        this->context_set_value(AUTHID_AUTH_USER, username);
        this->context_ask(AUTHID_TARGET_USER);
        this->context_ask(AUTHID_TARGET_DEVICE);
        this->context_ask(AUTHID_TARGET_PROTOCOL);
    }
};

#endif

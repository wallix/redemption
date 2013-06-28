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

#include <string.hpp>

using namespace std;

/*
static inline bool bool_from_string(string str)
{
    return (boost::iequals(string("1"),str))
        || (boost::iequals(string("yes"),str))
        || (boost::iequals(string("on"),str))
        || (boost::iequals(string("true"),str));
}
*/

static inline bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

static inline unsigned level_from_cstr(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
         if (0 == strcasecmp("medium", str)) { res = 1; }
    else if (0 == strcasecmp("high",   str)) { res = 2; }
    return res;
}

static inline unsigned logtype_from_cstr(const char * str)
{ // null = 0, print = 1, syslog = 2, file = 3, encryptedfile = 4
    unsigned res = 0;
         if (0 == strcasecmp("print",         str)) { res = 1; }
    else if (0 == strcasecmp("syslog",        str)) { res = 2; }
    else if (0 == strcasecmp("file",          str)) { res = 3; }
    else if (0 == strcasecmp("encryptedfile", str)) { res = 4; }
    return res;
}

static inline unsigned ulong_from_cstr(const char * str)
{ // 10 = 10, 0x10 = 16
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, 0, 16);
    }

    return atol(str);
}

static inline signed _long_from_cstr(const char * str)
{
    return atol(str);
}

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
    if (!flag){
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

    // Options
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
    AUTHID_SELECTOR_LINES_PER_PAGE,     // number of lines per page
    AUTHID_SELECTOR_NUMBER_OF_PAGES,    // number of pages

    AUTHID_TARGET_DEVICE,   // target_device
    AUTHID_TARGET_PASSWORD, // target_password
    AUTHID_TARGET_PORT,     // target_port
    AUTHID_TARGET_PROTOCOL, // proto_dest
    AUTHID_TARGET_USER,     // target_login

    AUTHID_AUTH_USER,       // login
    AUTHID_HOST,            // ip_client
    AUTHID_TARGET,          // ip_target
    AUTHID_PASSWORD,        // password

    AUTHID_AUTHCHANNEL_ANSWER,  // WabLauncher target answer
    AUTHID_AUTHCHANNEL_RESULT,  // WabLauncher session result
    AUTHID_AUTHCHANNEL_TARGET,  // WabLauncher target request

    AUTHID_MESSAGE, // warning_message

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
// Options
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
#define STRAUTHID_SELECTOR_LINES_PER_PAGE  "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"

#define STRAUTHID_TARGET_DEVICE            "target_device"
#define STRAUTHID_TARGET_PASSWORD          "target_password"
#define STRAUTHID_TARGET_PORT              "target_port"
#define STRAUTHID_TARGET_PROTOCOL          "proto_dest"
#define STRAUTHID_TARGET_USER              "target_login"

#define STRAUTHID_AUTH_USER                "login"
#define STRAUTHID_HOST                     "ip_client"
#define STRAUTHID_TARGET                   "ip_target"
#define STRAUTHID_PASSWORD                 "password"

TODO("This is not a translation but auth_channel answer, change key name in sesman")
#define STRAUTHID_AUTHCHANNEL_ANSWER       "trans_auth_channel"
#define STRAUTHID_AUTHCHANNEL_RESULT       "auth_channel_result"
#define STRAUTHID_AUTHCHANNEL_TARGET       "auth_channel_target"

#define STRAUTHID_MESSAGE                  "message"

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

    // Options
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
    STRAUTHID_SELECTOR_LINES_PER_PAGE,  // number of lines per page
    STRAUTHID_SELECTOR_NUMBER_OF_PAGES, // number of pages

    STRAUTHID_TARGET_DEVICE,    // target_device
    STRAUTHID_TARGET_PASSWORD,  // target_password
    STRAUTHID_TARGET_PORT,      // target_port
    STRAUTHID_TARGET_PROTOCOL,  // proto_dest
    STRAUTHID_TARGET_USER,      // target_login

    STRAUTHID_AUTH_USER,        // login
    STRAUTHID_HOST,             // ip_client
    STRAUTHID_TARGET,           // ip_target
    STRAUTHID_PASSWORD,         // password

    STRAUTHID_AUTHCHANNEL_ANSWER,   // WabLauncher target answer
    STRAUTHID_AUTHCHANNEL_RESULT,   // WabLauncher session result
    STRAUTHID_AUTHCHANNEL_TARGET,   // WabLauncher target request

    STRAUTHID_MESSAGE,  // warning_message

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
};
static inline authid_t authid_from_string(const char * strauthid) {

    std::string str = std::string(strauthid);
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID - 1 ; i++) {
        if (0 == authstr[i].compare(str)) {
            res = (authid_t)(i + 1);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    if ((authid == AUTHID_UNKNOWN) || (authid >= MAX_AUTHID))
        return "";
    return authstr[(unsigned)authid - 1].c_str();
}

struct Inifile {
private:
    typedef struct {
        bool ask;
        bool modified;
        bool read;
    } meta_state_t;
    

public:
    struct Inifile_globals {
        bool capture_chunk;
        meta_state_t state_capture_chunk;

        char auth_user[512];                    // AUTHID_AUTH_USER
        meta_state_t state_auth_user;
        char host[512];                         // client_ip AUTHID_HOST
        meta_state_t state_host;                // client_ip
        char target[512];                       // target ip AUTHID_TARGET
        meta_state_t state_target;              // target ip
        char target_device[32768];              // AUTHID_TARGET_DEVICE
        meta_state_t state_target_device;
        char target_user[512];                  // AUTHID_TARGET_USER
        meta_state_t state_target_user;

        // BEGIN globals
        bool bitmap_cache;       // default true
        meta_state_t state_bitmap_cache;       // default true
        bool bitmap_compression; // default true
        meta_state_t state_bitmap_compression; // default true
        int port;                // default 3389
        meta_state_t state_port;                // default 3389
        bool nomouse;
        meta_state_t state_nomouse;
        bool notimestamp;
        meta_state_t state_notimestamp;
        int encryptionLevel;     // 0=low, 1=medium, 2=high
        meta_state_t state_encryptionLevel;     // 0=low, 1=medium, 2=high
        char authip[255];
        meta_state_t state_authip;
        int authport;
        meta_state_t state_authport;
        bool autovalidate;       // dialog autovalidation for test
        meta_state_t state_autovalidate;       // dialog autovalidation for test

        // keepalive and no traffic auto deconnexion
        int max_tick;
        meta_state_t state_max_tick;
        int keepalive_grace_delay;
        meta_state_t state_keepalive_grace_delay;

        bool internal_domain;
        meta_state_t state_internal_domain;

        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files
        meta_state_t state_dynamic_conf_path; // directory where to look for dynamic configuration files
        char auth_channel[512];
        meta_state_t state_auth_channel;
        bool enable_file_encryption;
        meta_state_t state_enable_file_encryption;
        bool enable_tls;
        meta_state_t state_enable_tls;
        char listen_address[256];
        meta_state_t state_listen_address;
        bool enable_ip_transparent;
        meta_state_t state_enable_ip_transparent;
        char certificate_password[256];
        meta_state_t state_certificate_password;

        char png_path[1024];
        meta_state_t state_png_path;
        char wrm_path[1024];
        meta_state_t state_wrm_path;

        char alternate_shell[1024];
        meta_state_t state_alternate_shell;
        char shell_working_directory[1024];
        meta_state_t state_shell_working_directory;

        char codec_id[512];
        meta_state_t state_codec_id;
        bool movie;
        meta_state_t state_movie;
        char movie_path[512];
        meta_state_t state_movie_path;
        char video_quality[512];
        meta_state_t state_video_quality;
        bool enable_bitmap_update;
        meta_state_t state_enable_bitmap_update;
        // END globals

        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        meta_state_t state_flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval;
        meta_state_t state_flv_frame_interval;
    } globals;

    // section "client"
    struct {
        bool ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default 
        meta_state_t state_ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default false

        uint32_t performance_flags_default;
        meta_state_t state_performance_flags_default;
        uint32_t performance_flags_force_present;
        meta_state_t state_performance_flags_force_present;
        uint32_t performance_flags_force_not_present;
        meta_state_t state_performance_flags_force_not_present;

        bool tls_fallback_legacy;
        meta_state_t state_tls_fallback_legacy;
   
        bool clipboard;
        meta_state_t state_clipboard;
        bool device_redirection;
        meta_state_t state_device_redirection;
    } client;

    // Section "video"
    struct {
        unsigned capture_flags;  // 1 PNG capture, 2 WRM
        meta_state_t state_capture_flags;  // 1 PNG capture, 2 WRM
        // video opt from capture_flags
        bool capture_png;
        meta_state_t state_capture_png;
        bool capture_wrm;
        meta_state_t state_capture_wrm;
        bool capture_flv;
        meta_state_t state_capture_flv;
        bool capture_ocr;
        meta_state_t state_capture_ocr;

        unsigned ocr_interval;
        meta_state_t state_ocr_interval;
        unsigned png_interval;   // time between 2 png captures (in 1/10 seconds)
        meta_state_t state_png_interval;   // time between 2 png captures (in 1/10 seconds)
        unsigned capture_groupid;
        meta_state_t state_capture_groupid;
        unsigned frame_interval; // time between 2 frame captures (in 1/100 seconds)
        meta_state_t state_frame_interval; // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval; // time between 2 wrm movies (in seconds)
        meta_state_t state_break_interval; // time between 2 wrm movies (in seconds)
        unsigned png_limit;    // number of png captures to keep
        meta_state_t state_png_limit;    // number of png captures to keep
        char replay_path[1024];
        meta_state_t state_replay_path;

        int l_bitrate;         // bitrate for low quality
        meta_state_t state_l_bitrate;         // bitrate for low quality
        int l_framerate;       // framerate for low quality
        meta_state_t state_l_framerate;       // framerate for low quality
        int l_height;          // height for low quality
        meta_state_t state_l_height;          // height for low quality
        int l_width;           // width for low quality
        meta_state_t state_l_width;           // width for low quality
        int l_qscale;          // qscale (parameter given to ffmpeg) for low quality
        meta_state_t state_l_qscale;          // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        int m_bitrate;
        meta_state_t state_m_bitrate;
        int m_framerate;
        meta_state_t state_m_framerate;
        int m_height;
        meta_state_t state_m_height;
        int m_width;
        meta_state_t state_m_width;
        int m_qscale;
        meta_state_t state_m_qscale;

        // Same for high quality
        int h_bitrate;
        meta_state_t state_h_bitrate;
        int h_framerate;
        meta_state_t state_h_framerate;
        int h_height;
        meta_state_t state_h_height;
        int h_width;
        meta_state_t state_h_width;
        int h_qscale;
        meta_state_t state_h_qscale;
    } video;

    // Section "debug"
    struct {
        uint32_t x224;
        meta_state_t state_x224;
        uint32_t mcs;
        meta_state_t state_mcs;
        uint32_t sec;
        meta_state_t state_sec;
        uint32_t rdp;
        meta_state_t state_rdp;
        uint32_t primary_orders;
        meta_state_t state_primary_orders;
        uint32_t secondary_orders;
        meta_state_t state_secondary_orders;
        uint32_t bitmap;
        meta_state_t state_bitmap;
        uint32_t capture;
        meta_state_t state_capture;
        uint32_t auth;
        meta_state_t state_auth;
        uint32_t session;
        meta_state_t state_session;
        uint32_t front;
        meta_state_t state_front;
        uint32_t mod_rdp;
        meta_state_t state_mod_rdp;
        uint32_t mod_vnc;
        meta_state_t state_mod_vnc;
        uint32_t mod_int;
        meta_state_t state_mod_int;
        uint32_t mod_xup;
        meta_state_t state_mod_xup;
        uint32_t widget;
        meta_state_t state_widget;
        uint32_t input;
        meta_state_t state_input;


        int log_type;
        meta_state_t state_log_type;
        char log_file_path[1024]; // log file location
        meta_state_t state_log_file_path; // log file location

    } debug;

    // section "translation"
    struct {
        redemption::string button_ok;
        meta_state_t state_button_ok;
        redemption::string button_cancel;
        meta_state_t state_button_cancel;
        redemption::string button_help;
        meta_state_t state_button_help;
        redemption::string button_close;
        meta_state_t state_button_close;
        redemption::string button_refused;
        meta_state_t state_button_refused;
        redemption::string login;
        meta_state_t state_login;
        redemption::string username;
        meta_state_t state_username;
        redemption::string password;
        meta_state_t state_password;
        redemption::string target;
        meta_state_t state_target;
        redemption::string diagnostic;
        meta_state_t state_diagnostic;
        redemption::string connection_closed;
        meta_state_t state_connection_closed;
        redemption::string help_message;
        meta_state_t state_help_message;
    } translation;

    // section "context"
    struct {
        unsigned           selector_focus;
        meta_state_t state_selector_focus;
        char               movie[1024];
        meta_state_t state_movie;

        unsigned           opt_bitrate;
        meta_state_t state_opt_bitrate;
        unsigned           opt_framerate;
        meta_state_t state_opt_framerate;
        unsigned           opt_qscale;
        meta_state_t state_opt_qscale;

        bool               ask_opt_bpp;
        bool               ask_opt_height;
        bool               ask_opt_width;


        unsigned           opt_bpp;                // AUTHID_OPT_BPP
        meta_state_t state_opt_bpp;
        unsigned           opt_height;             // AUTHID_OPT_HEIGHT
        meta_state_t state_opt_height; 
        unsigned           opt_width;              // AUTHID_OPT_WIDTH
        meta_state_t state_opt_width;

        redemption::string auth_error_message;
        meta_state_t state_auth_error_message;

        bool               ask_selector;
        bool               ask_selector_current_page;
        bool               ask_selector_device_filter;
        bool               ask_selector_group_filter;
        bool               ask_selector_lines_per_page;


        bool               selector;                 // AUTHID_SELECTOR
        meta_state_t state_selector;
        unsigned           selector_current_page;    // AUTHID_SELECTOR_CURRENT_PAGE
        meta_state_t state_selector_current_page;
        redemption::string selector_device_filter;   // AUTHID_SELECTOR_DEVICE_FILTER
        meta_state_t state_selector_device_filter;
        redemption::string selector_group_filter;    // AUTHID_SELECTOR_GROUP_FILTER
        meta_state_t state_selector_group_filter;
        unsigned           selector_lines_per_page;  // AUTHID_SELECTOR_LINES_PER_PAGE
        meta_state_t state_selector_lines_per_page;
        unsigned           selector_number_of_pages;
        meta_state_t state_selector_number_of_pages;

        bool               ask_target_device;
        bool               ask_target_password;
        bool               ask_target_port;
        bool               ask_target_protocol;
        bool               ask_target_user;


        redemption::string target_password;          // AUTHID_TARGET_PASSWORD
        meta_state_t state_target_password;
        unsigned           target_port;
        meta_state_t state_target_port;
        redemption::string target_protocol;          // AUTHID_TARGET_PROTOCOL
        meta_state_t state_target_protocol;

        bool               ask_auth_user;
        
        bool               ask_host;
        bool               ask_target;
        bool               ask_password;


        redemption::string password;                 // AUTHID_PASSWORD
        meta_state_t state_password;

        bool               ask_authchannel_target;
        bool               ask_authchannel_result;

        redemption::string authchannel_answer;
        meta_state_t state_authchannel_answer;
        redemption::string authchannel_result;
        meta_state_t state_authchannel_result;
        redemption::string authchannel_target;
        meta_state_t state_authchannel_target;

        bool               ask_accept_message;
        bool               ask_display_message;

        redemption::string message;
        meta_state_t state_message;
        redemption::string accept_message;           // AUTHID_ACCEPT_MESSAGE
        meta_state_t state_accept_message;
        redemption::string display_message;          // AUTHID_DISPLAY_MESSAGE
        meta_state_t state_display_message;

        redemption::string rejected;
        meta_state_t state_rejected;

        bool               authenticated;
        meta_state_t state_authenticated;

        bool               ask_keepalive;
        bool               ask_proxy_type;

        bool               keepalive;
        meta_state_t state_keepalive;
        redemption::string proxy_type;               // AUTHID_PROXY_TYPE
        meta_state_t state_proxy_type;

        bool               ask_trace_seal;

        redemption::string trace_seal;
        meta_state_t state_trace_seal;

        redemption::string session_id;
        meta_state_t state_session_id;

        unsigned           end_date_cnx;
        meta_state_t state_end_date_cnx;
        redemption::string end_time;
        meta_state_t state_end_time;

        redemption::string mode_console;
        meta_state_t state_mode_console;
        signed             timezone;
        meta_state_t state_timezone;

        redemption::string real_target_device;       // AUHTID_REAL_TARGET_DEVICE
        meta_state_t state_real_target_device;

        redemption::string authentication_challenge;
        meta_state_t state_authentication_challenge;
    } context;

    struct IniAccounts account;
public:
    Inifile() {
        std::stringstream oss("");
        this->init();
        this->cparse(oss);
    }

    Inifile(const char * filename) {
        this->init();
        this->cparse(filename);
    }

    Inifile(istream & Inifile_stream) {
        this->init();
        this->cparse(Inifile_stream);
    }

    void init(){
        this->globals.capture_chunk = false;


        this->globals.auth_user[0]     = 0;
        this->globals.host[0]          = 0;
        this->globals.target_device[0] = 0;
        this->globals.target_user[0]   = 0;

        // Init globals
        this->globals.bitmap_cache = true;
        this->globals.bitmap_compression = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_cstr("low");
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.autovalidate = false;

        this->globals.max_tick    = 30;
        this->globals.keepalive_grace_delay = 30;

        this->globals.internal_domain = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
        memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);
        this->globals.enable_file_encryption = false;
        this->globals.enable_tls             = true;
        strcpy(this->globals.listen_address, "0.0.0.0");
        this->globals.enable_ip_transparent  = false;
        strcpy(this->globals.certificate_password, "inquisition");

        strcpy(this->globals.png_path, PNG_PATH);
        strcpy(this->globals.wrm_path, WRM_PATH);

        this->globals.alternate_shell[0]         = 0;
        this->globals.shell_working_directory[0] = 0;

        strcpy(this->globals.codec_id, "flv");
        this->globals.movie            = false;
        this->globals.movie_path[0]    = 0;
        TODO("this could be some kind of enumeration")
        strcpy(this->globals.video_quality, "medium");
        this->globals.enable_bitmap_update = false;
        // End Init globals


        this->globals.flv_break_interval = 600000000l;
        this->globals.flv_frame_interval = 1000000L;

        strcpy(this->account.accountname, "");
        strcpy(this->account.username,    "");
        strcpy(this->account.password,    "");

        // Begin Section "client".
        this->client.ignore_logon_password               = false;
//      this->client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->client.performance_flags_default           = 0;
        this->client.performance_flags_force_present     = 0;
        this->client.performance_flags_force_not_present = 0;
        this->client.tls_fallback_legacy                 = false;
        this->client.clipboard                           = true;
        this->client.device_redirection                  = true;
        // End Section "client"

        // Begin section video
        this->video.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->video.capture_wrm   = true;
        this->video.capture_png   = true;
        this->video.capture_flv   = false;
        this->video.capture_ocr   = false;

        this->video.ocr_interval    = 100; // 1 every second
        this->video.png_interval    = 3000;
        this->video.capture_groupid = 33;
        this->video.frame_interval  = 40;
        this->video.break_interval  = 600;
        this->video.png_limit       = 3;
        strcpy(this->video.replay_path, "/tmp/");

        this->video.l_bitrate   = 20000;
        this->video.l_framerate = 1;
        this->video.l_height    = 480;
        this->video.l_width     = 640;
        this->video.l_qscale    = 25;
        this->video.m_bitrate   = 40000;
        this->video.m_framerate = 1;
        this->video.m_height    = 768;
        this->video.m_width     = 1024;
        this->video.m_qscale    = 15;
        this->video.h_bitrate   = 200000;
        this->video.h_framerate = 5;
        this->video.h_height    = 1024;
        this->video.h_width     = 1280;
        this->video.h_qscale    = 15;
        // End section "video"


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
        // End Section "debug"

        // Begin Section "translation"
        this->translation.button_ok.copy_c_str("OK");
        this->translation.button_cancel.copy_c_str("Cancel");
        this->translation.button_help.copy_c_str("Help");
        this->translation.button_close.copy_c_str("Close");
        this->translation.button_refused.copy_c_str("Refused");
        this->translation.login.copy_c_str("login");
        this->translation.username.copy_c_str("username");
        this->translation.password.copy_c_str("password");
        this->translation.target.copy_c_str("target");
        this->translation.diagnostic.copy_c_str("diagnostic");
        this->translation.connection_closed.copy_c_str("Connection closed");
        this->translation.help_message.copy_c_str("Help message");
        // End Section "translation"

        // Begin section "context"
        this->context.selector_focus              = 0;
        this->context.movie[0]                    = 0;

        this->context.opt_bitrate                 = 40000;
        this->context.opt_framerate               = 5;
        this->context.opt_qscale                  = 15;

        this->context.ask_opt_bpp                 = false;
        this->context.state_opt_bpp.ask                 = false;
        this->context.state_opt_bpp.modified                 = true;
        this->context.ask_opt_height              = false;
        this->context.state_opt_height.ask              = false;
        this->context.state_opt_height.modified              = true;
        this->context.ask_opt_width               = false;
        this->context.state_opt_width.ask               = false;
        this->context.state_opt_width.modified               = true;

        this->context.opt_bpp                     = 24;
        this->context.opt_height                  = 600;
        this->context.opt_width                   = 800;

        this->context.auth_error_message.empty();

        this->context.ask_selector                = false;
        this->context.state_selector.ask                = false;
        this->context.state_selector.modified                = true;
        this->context.ask_selector_current_page   = false;
        this->context.state_selector_current_page.ask   = false;
        this->context.state_selector_current_page.modified   = true;
        this->context.ask_selector_device_filter  = false;
        this->context.state_selector_device_filter.ask  = false;
        this->context.state_selector_device_filter.modified  = true;
        this->context.ask_selector_group_filter   = false;
        this->context.state_selector_group_filter.ask   = false;
        this->context.state_selector_group_filter.modified   = true;
        this->context.ask_selector_lines_per_page = false;
        this->context.state_selector_lines_per_page.ask = false;
        this->context.state_selector_lines_per_page.modified = true;

        this->context.selector                    = false;
        this->context.selector_current_page       = 1;
        this->context.selector_device_filter.empty();
        this->context.selector_group_filter.empty();
        this->context.selector_lines_per_page     = 20;
        this->context.selector_number_of_pages    = 1;

        this->context.ask_target_device           = true;
        this->globals.state_target_device.ask           = true;
        this->globals.state_target_device.modified           = true;
        this->context.ask_target_password         = true;
        this->context.state_target_password.ask         = true;
        this->context.state_target_password.modified         = true;
        this->context.ask_target_port             = true;
        this->context.state_target_port.ask             = true;
        this->context.state_target_port.modified             = true;
        this->context.ask_target_protocol         = true;
        this->context.state_target_protocol.ask         = true;
        this->context.state_target_protocol.modified         = true;
        this->context.ask_target_user             = true;
        this->globals.state_target_user.ask             = true;
        this->globals.state_target_user.modified             = true;

        this->context.target_password.empty();
        this->context.target_port                 = 3389;
        this->context.target_protocol.copy_c_str("RDP");

        // not sure about ask_host and ask_target initial values
        this->context.ask_host                    = false;
        this->globals.state_host.ask                    = false;
        this->globals.state_host.modified                    = true;
        this->context.ask_target                  = false;
        this->globals.state_target.ask                  = false;
        this->globals.state_target.modified                  = true;

        this->context.ask_auth_user               = true;
        this->globals.state_auth_user.ask               = true;
        this->globals.state_auth_user.modified               = true;
        this->context.ask_password                = true;
        this->context.state_password.ask                = true;
        this->context.state_password.modified                = true;


        this->context.password.empty();

        this->context.ask_authchannel_target      = false;
        this->context.ask_authchannel_result      = false;

        this->context.authchannel_answer.empty();
        this->context.authchannel_result.empty();
        this->context.authchannel_target.empty();

        this->context.ask_accept_message          = false;
        this->context.state_accept_message.ask          = false;
        this->context.state_accept_message.modified          = true;
        this->context.ask_display_message         = false;
        this->context.state_display_message.ask         = false;
        this->context.state_display_message.modified         = true;

        this->context.message.empty();
        this->context.accept_message.empty();
        this->context.display_message.empty();

        this->context.rejected.copy_c_str("Connection refused by authentifier.");

        this->context.authenticated               = false;

        this->context.ask_keepalive               = true;
        this->context.state_keepalive.ask               = true;
        this->context.state_keepalive.modified               = true;
        this->context.ask_proxy_type              = false;
        this->context.state_proxy_type.ask              = false;
        this->context.state_proxy_type.modified              = true;

        this->context.keepalive                   = false;
        this->context.proxy_type.copy_c_str("RDP");

        this->context.ask_trace_seal              = false;
        this->context.state_trace_seal.ask              = false;
        this->context.state_trace_seal.modified              = true;

        this->context.trace_seal.empty();

        this->context.session_id.empty();

        this->context.end_date_cnx                = 0;
        this->context.end_time.empty();

        this->context.mode_console.copy_c_str("allow");
        this->context.timezone                    = -3600;

        this->context.real_target_device.empty();
        this->context.state_real_target_device.ask              = false;
        this->context.state_real_target_device.modified              = true;

        this->context.authentication_challenge.empty();
    };

    void cparse(istream & ifs){
        const size_t maxlen = 256;
        char line[maxlen];
        char context[128] = {0};
        bool truncated = false;
        while (ifs.good()){
            ifs.getline(line, maxlen);
            if (ifs.fail() && ifs.gcount() == (maxlen-1)){
                if (!truncated){
                    LOG(LOG_INFO, "Line too long in configuration file");
                    hexdump(line, maxlen-1);
                }
                ifs.clear();
                truncated = true;
                continue;
            }
            if (truncated){
                truncated = false;
                continue;
            }
            this->parseline(line, context);
        };
    }

    void parseline(const char * line, char * context)
    {
        char key[128];
        char value[128];

        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)){
                if (*startkey == '['){
                    const char * startcontext = startkey + 1;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext){
                        memcpy(context, startcontext, endcontext - startcontext);
                        context[endcontext - startcontext] = 0;
                    }
                    return;
                }
                break;
            }
        }
        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey){
            const char * sep = endkey;
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)){
                    TODO("RZ: Possible buffer overflow if length of key is larger than 128 bytes")
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)){
                            break;
                        }
                    }
                    const char * endvalue;
/*
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        TODO("RZ: Support space in value")
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                    }
                    TODO("RZ: Possible buffer overflow if length of value is larger than 128 bytes")
                    memcpy(value, startvalue, endvalue - startvalue + 1);
                    value[endvalue - startvalue + 1] = 0;
*/
                    char *curvalue = value;
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                        else if ((*endvalue == '\\') && *(endvalue + 1)) {
                            if (endvalue > startvalue) {
                                memcpy(curvalue, startvalue, endvalue - startvalue);
                                curvalue += (endvalue - startvalue);
                            }

                            endvalue++;

                            *curvalue++ = *endvalue;

                            startvalue = endvalue + 1;
                        }
                    }
                    if (endvalue > startvalue) {
                        memcpy(curvalue, startvalue, endvalue - startvalue);
                        curvalue += (endvalue - startvalue);
                    }
                    *curvalue = 0;

                    this->setglobal_from_file(key, value, context);
                    break;
                }
            }
        }
    }

    void setglobal_from_file(const char * key, const char * value, const char * context)
    {
        if (0 == strcmp(context, "globals")){
            if (0 == strcmp(key, "bitmap_cache")){
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_compression")){
                this->globals.bitmap_compression = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "port")){
                this->globals.port = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "nomouse")){
                this->globals.nomouse = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "notimestamp")){
                this->globals.notimestamp = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "encryptionLevel")){
                this->globals.encryptionLevel = level_from_cstr(value);
            }
            else if (0 == strcmp(key, "authip")){
                strncpy(this->globals.authip, value, sizeof(this->globals.authip));
                this->globals.authip[sizeof(this->globals.authip) - 1] = 0;
            }
            else if (0 == strcmp(key, "authport")){
                this->globals.authport = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "autovalidate")){
                this->globals.autovalidate = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_tick")){
                this->globals.max_tick = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")){
                this->globals.keepalive_grace_delay = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "internal_domain")){
                this->globals.internal_domain = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "dynamic_conf_path")){
                strncpy(this->globals.dynamic_conf_path, value, sizeof(this->globals.dynamic_conf_path));
                this->globals.dynamic_conf_path[sizeof(this->globals.dynamic_conf_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "auth_channel")){
                strncpy(this->globals.auth_channel, value, 8);
                this->globals.auth_channel[7] = 0;
            }
            else if (0 == strcmp(key, "enable_file_encryption")){
                this->globals.enable_file_encryption = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_tls")){
                this->globals.enable_tls = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "listen_address")){
                strncpy(this->globals.listen_address, value, sizeof(this->globals.listen_address));
                this->globals.listen_address[sizeof(this->globals.listen_address) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_ip_transparent")){
                this->globals.enable_ip_transparent = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_password")){
                strncpy(this->globals.certificate_password, value, sizeof(this->globals.certificate_password));
                this->globals.certificate_password[sizeof(this->globals.certificate_password) - 1] = 0;
            }
            else if (0 == strcmp(key, "png_path")){
                strncpy(this->globals.png_path, value, sizeof(this->globals.png_path));
                this->globals.png_path[sizeof(this->globals.png_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "wrm_path")){
                strncpy(this->globals.wrm_path, value, sizeof(this->globals.wrm_path));
                this->globals.wrm_path[sizeof(this->globals.wrm_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "alternate_shell")) {
                strncpy(this->globals.alternate_shell, value, sizeof(this->globals.alternate_shell));
                this->globals.alternate_shell[sizeof(this->globals.alternate_shell) - 1] = 0;
            }
            else if (0 == strcmp(key, "shell_working_directory")) {
                strncpy(this->globals.shell_working_directory, value, sizeof(this->globals.shell_working_directory));
                this->globals.shell_working_directory[sizeof(this->globals.shell_working_directory) - 1] = 0;
            }
            else if (0 == strcmp(key, "codec_id")) {
                strncpy(this->globals.codec_id, value, sizeof(this->globals.codec_id));
                this->globals.codec_id[sizeof(this->globals.codec_id) - 1] = 0;
            }
            else if (0 == strcmp(key, "movie")){
                this->globals.movie = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie_path")) {
                strncpy(this->globals.movie_path, value, sizeof(this->globals.movie_path));
                this->globals.movie_path[sizeof(this->globals.movie_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "video_quality")) {
                strncpy(this->globals.video_quality, value, sizeof(this->globals.video_quality));
                this->globals.video_quality[sizeof(this->globals.video_quality) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_bitmap_update")){
                this->globals.enable_bitmap_update = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "client")){
            if (0 == strcmp(key, "ignore_logon_password")){
                this->client.ignore_logon_password = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_default")){
                this->client.performance_flags_default = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_present")){
                this->client.performance_flags_force_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_not_present")){
                this->client.performance_flags_force_not_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_fallback_legacy")){
                this->client.tls_fallback_legacy = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard")){
                this->client.clipboard = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "device_redirection")){
                this->client.device_redirection = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "video")){
            if (0 == strcmp(key, "capture_flags")){
                this->video.capture_flags   = ulong_from_cstr(value);
                this->video.capture_png = 0 != (this->video.capture_flags & 1);
                this->video.capture_wrm = 0 != (this->video.capture_flags & 2);
                this->video.capture_flv = 0 != (this->video.capture_flags & 4);
                this->video.capture_ocr = 0 != (this->video.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->video.ocr_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->video.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")){
                this->video.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->video.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->video.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->video.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strncpy(this->video.replay_path, value, sizeof(this->video.replay_path));
                this->video.replay_path[sizeof(this->video.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->video.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->video.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->video.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->video.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->video.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->video.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->video.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->video.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->video.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->video.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->video.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->video.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->video.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->video.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->video.h_qscale    = ulong_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")){
                 if (0 == strcmp(key, "x224")){
                this->debug.x224              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mcs")){
                this->debug.mcs               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "sec")){
                this->debug.sec               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp")){
                this->debug.rdp               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "primary_orders")){
                this->debug.primary_orders    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "secondary_orders")){
                this->debug.secondary_orders  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap")){
                this->debug.bitmap            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture")){
                this->debug.capture           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth")){
                this->debug.auth              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session")){
                this->debug.session           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "front")){
                this->debug.front             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_rdp")){
                this->debug.mod_rdp           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_vnc")){
                this->debug.mod_vnc           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_int")){
                this->debug.mod_int           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_xup")){
                this->debug.mod_xup           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "widget")){
                this->debug.widget            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "input")){
                this->debug.input             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_type")){
                this->debug.log_type = logtype_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_file_path")){
                strncpy(this->debug.log_file_path, value, sizeof(this->debug.log_file_path));
                this->debug.log_file_path[sizeof(this->debug.log_file_path) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "translation")){
                 if (0 == strcmp(key, "button_ok")){
                this->translation.button_ok.copy_c_str(value);
            }
            else if (0 == strcmp(key, "button_cancel")){
                this->translation.button_cancel.copy_c_str(value);
            }
            else if (0 == strcmp(key, "button_help")){
                this->translation.button_help.copy_c_str(value);
            }
            else if (0 == strcmp(key, "button_close")){
                this->translation.button_close.copy_c_str(value);
            }
            else if (0 == strcmp(key, "button_refused")){
                this->translation.button_refused.copy_c_str(value);
            }
            else if (0 == strcmp(key, "login")){
                this->translation.login.copy_c_str(value);
            }
            else if (0 == strcmp(key, "username")){
                this->translation.username.copy_c_str(value);
            }
            else if (0 == strcmp(key, "password")){
                this->translation.password.copy_c_str(value);
            }
            else if (0 == strcmp(key, "target")){
                this->translation.target.copy_c_str(value);
            }
            else if (0 == strcmp(key, "diagnostic")){
                this->translation.diagnostic.copy_c_str(value);
            }
            else if (0 == strcmp(key, "connection_closed")){
                this->translation.connection_closed.copy_c_str(value);
            }
            else if (0 == strcmp(key, "help_message")){
                this->translation.help_message.copy_c_str(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }
    TODO("Should only be used by Authentifier "
         "It currently ask if the field has been modified "
         "and set it to not modified if it is not asked ")
    bool context_has_changed(authid_t authid) {
        bool res;
        bool * changed;
        switch (authid) {
        case AUTHID_OPT_BPP:
            changed = &this->context.state_opt_bpp.modified;
            res = *changed;
            break;

        case AUTHID_OPT_HEIGHT:
            changed = &this->context.state_opt_height.modified;
            res = *changed;
            break;

        case AUTHID_OPT_WIDTH:
            changed = &this->context.state_opt_width.modified;
            res = *changed;
            break;

        case AUTHID_SELECTOR:
            changed = &this->context.state_selector.modified;
            res = *changed;
            break;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            changed = &this->context.state_selector_current_page.modified;
            res = *changed;
            break;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            changed = &this->context.state_selector_device_filter.modified;
            res = *changed;
            break;

        case AUTHID_SELECTOR_GROUP_FILTER:
            changed = &this->context.state_selector_group_filter.modified;
            res = *changed;
            break;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            changed = &this->context.state_selector_lines_per_page.modified;
            res = *changed;
            break;

        case AUTHID_TARGET_DEVICE:
            changed = &this->globals.state_target_device.modified;
            res = *changed;
            break;

        case AUTHID_TARGET_PASSWORD:
            changed = &this->context.state_target_password.modified;
            res = *changed;
            break;

        case AUTHID_TARGET_PORT:
            changed = &this->context.state_target_port.modified;
            res = *changed;
            break;

        case AUTHID_TARGET_PROTOCOL:
            changed = &this->context.state_target_protocol.modified;
            res = *changed;
            break;

        case AUTHID_TARGET_USER:
            changed = &this->globals.state_target_user.modified;
            res = *changed;
            break;

        case AUTHID_AUTH_USER:
            changed = &this->globals.state_auth_user.modified;
            res = *changed;
            break;

        case AUTHID_HOST:
            changed = &this->globals.state_host.modified;
            res = *changed;
            break;

        case AUTHID_TARGET:
            changed = &this->globals.state_target.modified;
            res = *changed;
            break;

        case AUTHID_PASSWORD:
            changed = &this->context.state_password.modified;
            res = *changed;
            break;

        case AUTHID_AUTHCHANNEL_RESULT:
            changed = &this->context.state_authchannel_result.modified;
            res = *changed;
            break;

        case AUTHID_AUTHCHANNEL_TARGET:
            changed = &this->context.state_authchannel_target.modified;
            res = *changed;
            break;

        case AUTHID_ACCEPT_MESSAGE:
            changed = &this->context.state_accept_message.modified;
            res = *changed;
            break;

        case AUTHID_DISPLAY_MESSAGE:
            changed = &this->context.state_display_message.modified;
            res = *changed;
            break;

        case AUTHID_KEEPALIVE:
            changed = &this->context.state_keepalive.modified;
            res = *changed;
            break;

        case AUTHID_PROXY_TYPE:
            changed = &this->context.state_proxy_type.modified;
            res = *changed;
            break;

        case AUTHID_TRACE_SEAL:
            changed = &this->context.state_trace_seal.modified;
            res = *changed;
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            changed = &this->context.state_real_target_device.modified;
            res = *changed;
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
        if(!this->context_is_asked(authid)){
            *changed = false;
        }
        return res;
        
    }

    void context_set_value_by_string(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            context_set_value(authid, value);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_set_value(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value(authid_t authid, const char * value) {
        switch (authid)
        {
        case AUTHID_TRANS_BUTTON_OK:
            this->translation.button_ok.copy_c_str(value);
            break;
        case AUTHID_TRANS_BUTTON_CANCEL:
            this->translation.button_cancel.copy_c_str(value);
            break;
        case AUTHID_TRANS_BUTTON_HELP:
            this->translation.button_help.copy_c_str(value);
            break;
        case AUTHID_TRANS_BUTTON_CLOSE:
            this->translation.button_close.copy_c_str(value);
            break;
        case AUTHID_TRANS_BUTTON_REFUSED:
            this->translation.button_refused.copy_c_str(value);
            break;
        case AUTHID_TRANS_LOGIN:
            this->translation.login.copy_c_str(value);
            break;
        case AUTHID_TRANS_USERNAME:
            this->translation.username.copy_c_str(value);
            break;
        case AUTHID_TRANS_PASSWORD:
            this->translation.password.copy_c_str(value);
            break;
        case AUTHID_TRANS_TARGET:
            this->translation.target.copy_c_str(value);
            break;
        case AUTHID_TRANS_DIAGNOSTIC:
            this->translation.diagnostic.copy_c_str(value);
            break;
        case AUTHID_TRANS_CONNECTION_CLOSED:
            this->translation.connection_closed.copy_c_str(value);
            break;
        case AUTHID_TRANS_HELP_MESSAGE:
            this->translation.help_message.copy_c_str(value);
            break;

        // Options
        case AUTHID_OPT_CLIPBOARD:
            this->client.clipboard = bool_from_cstr(value);
            break;
        case AUTHID_OPT_DEVICEREDIRECTION:
            this->client.device_redirection = bool_from_cstr(value);
            break;
        case AUTHID_OPT_FILE_ENCRYPTION:
            this->globals.enable_file_encryption = bool_from_cstr(value);
            break;

        // Video capture
        case AUTHID_OPT_CODEC_ID:
            strncpy(this->globals.codec_id, value, sizeof(this->globals.codec_id));
            this->globals.codec_id[sizeof(this->globals.codec_id) - 1] = 0;
            break;
        case AUTHID_OPT_MOVIE:
            this->globals.movie = bool_from_cstr(value);
            break;
        case AUTHID_OPT_MOVIE_PATH:
            strncpy(this->globals.movie_path, value, sizeof(this->globals.movie_path));
            this->globals.movie_path[sizeof(this->globals.movie_path) - 1] = 0;
            break;
        case AUTHID_VIDEO_QUALITY:
            strncpy(this->globals.video_quality, value, sizeof(this->globals.video_quality));
            this->globals.video_quality[sizeof(this->globals.video_quality) - 1] = 0;
            break;

        // Alternate shell
        case AUTHID_ALTERNATE_SHELL:
            strncpy(this->globals.alternate_shell, value, sizeof(this->globals.alternate_shell));
            this->globals.alternate_shell[sizeof(this->globals.alternate_shell) - 1] = 0;
            break;
        case AUTHID_SHELL_WORKING_DIRECTORY:
            strncpy(this->globals.shell_working_directory, value, sizeof(this->globals.shell_working_directory));
            this->globals.shell_working_directory[sizeof(this->globals.shell_working_directory) - 1] = 0;
            break;

        // Context
        case AUTHID_OPT_BITRATE:
            this->context.opt_bitrate   = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_FRAMERATE:
            this->context.opt_framerate = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_QSCALE:
            this->context.opt_qscale    = ulong_from_cstr(value);
            break;

        case AUTHID_OPT_WIDTH:
            this->context.ask_opt_width  = false;
            this->context.state_opt_width.ask = false;
            if (this->context.opt_width != ulong_from_cstr(value))
                this->context.state_opt_width.modified = true;
            this->context.opt_width      = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_HEIGHT:
            this->context.ask_opt_height = false;
            this->context.state_opt_height.ask = false;
            if (this->context.opt_height != ulong_from_cstr(value))
                this->context.state_opt_height.modified = true;
            this->context.opt_height     = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_BPP:
            this->context.ask_opt_bpp    = false;
            this->context.state_opt_bpp.ask    = false;
            if (this->context.opt_bpp != ulong_from_cstr(value))
                this->context.state_opt_bpp.modified    = true;
            this->context.opt_bpp        = ulong_from_cstr(value);
            break;

        case AUTHID_AUTH_ERROR_MESSAGE:
            this->context.auth_error_message.copy_c_str(value);
            break;

        case AUTHID_SELECTOR:
            this->context.ask_selector                = false;
            this->context.state_selector.ask                = false;
            if (this->context.selector != bool_from_cstr(value))
                this->context.state_selector.modified                = true;
            this->context.selector                    = bool_from_cstr(value);
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->context.ask_selector_current_page   = false;
            this->context.state_selector_current_page.ask   = false;
            if (this->context.selector_current_page != ulong_from_cstr(value))
                this->context.state_selector_current_page.modified   = true;
            this->context.selector_current_page   = ulong_from_cstr(value);
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->context.ask_selector_device_filter  = false;
            this->context.state_selector_device_filter.ask  = false;
            if (strcmp(value, this->context.selector_device_filter.c_str()))
                this->context.state_selector_device_filter.modified  = true;
            this->context.selector_device_filter.copy_c_str(value);
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            this->context.ask_selector_group_filter   = false;
            this->context.state_selector_group_filter.ask   = false;
            if (strcmp(value, this->context.selector_group_filter.c_str()))
                this->context.state_selector_group_filter.modified   = true;
            this->context.selector_group_filter.copy_c_str(value);
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->context.ask_selector_lines_per_page = false;
            this->context.state_selector_lines_per_page.ask = false;
            if (this->context.selector_lines_per_page != ulong_from_cstr(value))
                this->context.state_selector_lines_per_page.modified = true;
            this->context.selector_lines_per_page = ulong_from_cstr(value);
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            this->context.selector_number_of_pages    = ulong_from_cstr(value);
            break;

        case AUTHID_TARGET_DEVICE:
            this->context.ask_target_device   = false;
            this->globals.state_target_device.ask   = false;
            if (strcmp(value, this->globals.target_device))
                this->globals.state_target_device.modified   = true;
            strncpy(this->globals.target_device, value, sizeof(this->globals.target_device));
            this->globals.target_device[sizeof(this->globals.target_device) - 1] = 0;
            break;
        case AUTHID_TARGET_PASSWORD:
            this->context.ask_target_password = false;
            this->context.state_target_password.ask = false;
            if (strcmp(value, this->context.target_password.c_str())) 
                this->context.state_target_password.modified = true;
            this->context.target_password.copy_c_str(value);
            break;
        case AUTHID_TARGET_PORT:
            this->context.ask_target_port     = false;
            this->context.state_target_port.ask     = false;
            if (this->context.target_port != ulong_from_cstr(value)) 
                this->context.state_target_port.modified     = true;
            this->context.target_port         = ulong_from_cstr(value);
            break;
        case AUTHID_TARGET_PROTOCOL:
            this->context.ask_target_protocol = false;
            this->context.state_target_protocol.ask = false;
            if (strcmp(value, this->context.target_protocol.c_str()))
                this->context.state_target_protocol.modified = true;
            this->context.target_protocol.copy_c_str(value);
            break;
        case AUTHID_TARGET_USER:
            this->context.ask_target_user     = false;
            this->globals.state_target_user.ask     = false;
            if (strcmp(value, this->globals.target_user))
                this->globals.state_target_user.modified     = true;
            strncpy(this->globals.target_user,   value, sizeof(this->globals.target_user));
            this->globals.target_user[sizeof(this->globals.target_user) - 1]     = 0;
            break;

        case AUTHID_AUTH_USER:
            this->context.ask_auth_user = false;
            this->globals.state_auth_user.ask = false;
            if (strcmp(value, this->globals.auth_user))
                this->globals.state_auth_user.modified = true;
            strncpy(this->globals.auth_user, value, sizeof(this->globals.auth_user));
            this->globals.auth_user[sizeof(this->globals.auth_user) - 1]         = 0;
            break;
        case AUTHID_HOST:
            this->context.ask_host      = false;
            this->globals.state_host.ask      = false;
            if (strcmp(value, this->globals.host))
                this->globals.state_host.modified      = true;
            strncpy(this->globals.host, value, sizeof(this->globals.host));
            this->globals.host[sizeof(this->globals.host) - 1]                   = 0;
            break;

        case AUTHID_TARGET:
            this->context.ask_target      = false;
            this->globals.state_target.ask      = false;
            if (strcmp(value, this->globals.target))
                this->globals.state_target.modified      = true;
            strncpy(this->globals.target,        value, sizeof(this->globals.target));
            this->globals.target[sizeof(this->globals.target) - 1]                 = 0;
            break;

        case AUTHID_PASSWORD:
            this->context.ask_password = false;
            this->context.state_password.ask = false;
            if (strcmp(value, this->context.password.c_str()))
                this->context.state_password.modified = true;
            this->context.password.copy_c_str(value);
            break;

        case AUTHID_AUTHCHANNEL_ANSWER:
            this->context.authchannel_answer.copy_c_str(value);
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            this->context.ask_authchannel_result = false;
            this->context.state_authchannel_result.ask = false;
            if (strcmp(value, this->context.authchannel_result.c_str()))
                this->context.state_authchannel_result.modified = true;
            this->context.authchannel_result.copy_c_str(value);
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            this->context.ask_authchannel_target = false;
            this->context.state_authchannel_target.ask = false;
            if (strcmp(value, this->context.authchannel_target.c_str()))
                this->context.state_authchannel_target.modified = true;
            this->context.authchannel_target.copy_c_str(value);
            break;

        case AUTHID_MESSAGE:
            this->context.message.copy_c_str(value);
            break;

        case AUTHID_ACCEPT_MESSAGE:
            this->context.ask_accept_message  = false;
            this->context.state_accept_message.ask  = false;
            if (strcmp(value, this->context.accept_message.c_str()))
                this->context.state_accept_message.modified  = true;
            this->context.accept_message.copy_c_str(value);
            break;
        case AUTHID_DISPLAY_MESSAGE:
            this->context.ask_display_message = false;
            this->context.state_display_message.ask = false;
            if (strcmp(value, this->context.display_message.c_str()))
                this->context.state_display_message.modified = true;
            this->context.display_message.copy_c_str(value);
            break;

        case AUTHID_AUTHENTICATED:
            this->context.authenticated  = bool_from_cstr(value);
            break;
        case AUTHID_REJECTED:
            this->context.rejected.copy_c_str(value);
            break;

        case AUTHID_KEEPALIVE:
            this->context.ask_keepalive  = false;
            this->context.state_keepalive.ask  = false;
            if (this->context.keepalive != bool_from_cstr(value))
                this->context.state_keepalive.modified  = true;
            this->context.keepalive      = bool_from_cstr(value);
            break;
        case AUTHID_PROXY_TYPE:
            this->context.ask_proxy_type = false;
            this->context.state_proxy_type.ask = false;
            if (strcmp(value, this->context.proxy_type.c_str()))
                this->context.state_proxy_type.modified = true;
            this->context.proxy_type.copy_c_str(value);
            break;

        case AUTHID_TRACE_SEAL:
            this->context.ask_trace_seal = false;
            this->context.state_trace_seal.ask = false;
            if (strcmp(value, this->context.trace_seal.c_str()))
                this->context.state_trace_seal.modified = true;
            this->context.trace_seal.copy_c_str(value);
            break;

        case AUTHID_SESSION_ID:
            this->context.session_id.copy_c_str(value);
            break;

        case AUTHID_END_DATE_CNX:
            this->context.end_date_cnx = ulong_from_cstr(value);
            break;
        case AUTHID_END_TIME:
            this->context.end_time.copy_c_str(value);
            break;

        case AUTHID_MODE_CONSOLE:
            this->context.mode_console.copy_c_str(value);
            break;
        case AUTHID_TIMEZONE:
            this->context.timezone = _long_from_cstr(value);
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            this->context.state_real_target_device.ask = false;
            if (strcmp(value, this->context.real_target_device.c_str()))
                this->context.state_real_target_device.modified = true;
            this->context.real_target_device.copy_c_str(value);
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            this->context.authentication_challenge.copy_c_str(value);
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
            break;
        }
    }

    const char * context_get_value_by_string(const char * strauthid, char * buffer, size_t size) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_get_value(authid, buffer, size);
        }

        LOG(LOG_WARNING, "Inifile::context_get_value(strid): unknown strauthid=\"%s\"", strauthid);

        return "";
    }

    const char * context_get_value(authid_t authid, char * buffer, size_t size) {
        const char * pszReturn = "";

        if (size) { *buffer = 0; }

        switch (authid)
        {
        case AUTHID_TRANS_BUTTON_OK:
            pszReturn = this->translation.button_ok.c_str();
            break;
        case AUTHID_TRANS_BUTTON_CANCEL:
            pszReturn = this->translation.button_cancel.c_str();
            break;
        case AUTHID_TRANS_BUTTON_HELP:
            pszReturn = this->translation.button_help.c_str();
            break;
        case AUTHID_TRANS_BUTTON_CLOSE:
            pszReturn = this->translation.button_close.c_str();
            break;
        case AUTHID_TRANS_BUTTON_REFUSED:
            pszReturn = this->translation.button_refused.c_str();
            break;
        case AUTHID_TRANS_LOGIN:
            pszReturn = this->translation.login.c_str();
            break;
        case AUTHID_TRANS_USERNAME:
            pszReturn = this->translation.username.c_str();
            break;
        case AUTHID_TRANS_PASSWORD:
            pszReturn = this->translation.password.c_str();
            break;
        case AUTHID_TRANS_TARGET:
            pszReturn = this->translation.target.c_str();
            break;
        case AUTHID_TRANS_DIAGNOSTIC:
            pszReturn = this->translation.diagnostic.c_str();
            break;
        case AUTHID_TRANS_CONNECTION_CLOSED:
            pszReturn = this->translation.connection_closed.c_str();
            break;
        case AUTHID_TRANS_HELP_MESSAGE:
            pszReturn = this->translation.help_message.c_str();
            break;

        case AUTHID_OPT_CLIPBOARD:
            if (size) {
                strncpy(buffer, (this->client.clipboard ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_DEVICEREDIRECTION:
            if (size) {
                strncpy(buffer, (this->client.device_redirection ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_FILE_ENCRYPTION:
            if (size) {
                strncpy(buffer, (this->globals.enable_file_encryption ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;

        case AUTHID_OPT_CODEC_ID:
            if (size) {
                strncpy(buffer, this->globals.codec_id, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_MOVIE:
            if (size) {
                strncpy(buffer, (this->globals.movie ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_MOVIE_PATH:
            if (size) {
                strncpy(buffer, this->globals.movie_path, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_VIDEO_QUALITY:
            if (size) {
                strncpy(buffer, this->globals.video_quality, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;

        case AUTHID_ALTERNATE_SHELL:
            pszReturn = this->globals.alternate_shell;
            break;
        case AUTHID_SHELL_WORKING_DIRECTORY:
            pszReturn = this->globals.shell_working_directory;
            break;

        case AUTHID_OPT_BITRATE:
            if (size) {
                snprintf(buffer, size, "%u", this->context.opt_bitrate);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_FRAMERATE:
            if (size) {
                snprintf(buffer, size, "%u", this->context.opt_framerate);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_QSCALE:
            if (size) {
                snprintf(buffer, size, "%u", this->context.opt_qscale);
                pszReturn = buffer;
            }
            break;

        case AUTHID_OPT_BPP:
            if (  size
               && !this->context.ask_opt_bpp) {
                snprintf(buffer, size, "%u", this->context.opt_bpp);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_HEIGHT:
            if (  size
               && !this->context.ask_opt_height) {
                snprintf(buffer, size, "%u", this->context.opt_height);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_WIDTH:
            if (  size
               && !this->context.ask_opt_width) {
                snprintf(buffer, size, "%u", this->context.opt_width);
                pszReturn = buffer;
            }
            break;

        case AUTHID_AUTH_ERROR_MESSAGE:
            pszReturn = this->context.auth_error_message.c_str();
            break;

        case AUTHID_SELECTOR:
            if (  size
               && !this->context.ask_selector) {
                strncpy(buffer, (this->context.selector ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            if (  size
               && !this->context.ask_selector_current_page) {
                snprintf(buffer, size, "%u", this->context.selector_current_page);
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            if (!this->context.ask_selector_device_filter) {
                pszReturn = this->context.selector_device_filter.c_str();
            }
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            if ( !this->context.ask_selector_group_filter) {
                pszReturn = this->context.selector_group_filter.c_str();
            }
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            if (  size
               && !this->context.ask_selector_group_filter) {
                snprintf(buffer, size, "%u", this->context.selector_lines_per_page);
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            if (size) {
                snprintf(buffer, size, "%u", this->context.selector_number_of_pages);
                pszReturn = buffer;
            }
            break;

        case AUTHID_TARGET_DEVICE:
            if (!this->context.ask_target_device) {
                pszReturn = this->globals.target_device;
            }
            break;
        case AUTHID_TARGET_PASSWORD:
            if (!this->context.ask_target_password) {
                pszReturn = this->context.target_password.c_str();
            }
            break;
        case AUTHID_TARGET_PORT:
            if (  size
               && !this->context.ask_target_port) {
                snprintf(buffer, size, "%u", this->context.target_port);
                pszReturn = buffer;
            }
            break;
        case AUTHID_TARGET_PROTOCOL:
            if (!this->context.ask_target_protocol) {
                pszReturn = this->context.target_protocol.c_str();
            }
            break;
        case AUTHID_TARGET_USER:
            if (!this->context.ask_target_user) {
                pszReturn = this->globals.target_user;
            }
            break;

        case AUTHID_AUTH_USER:
            if (!this->context.ask_auth_user) {
                pszReturn = this->globals.auth_user;
            }
            break;
        case AUTHID_HOST:
            if ( !this->context.ask_host) {
                pszReturn = this->globals.host;
            }
            break;

        case AUTHID_TARGET:
            if (  size
               && !this->context.ask_target) {
                pszReturn = this->globals.target;
            }
            break;
        case AUTHID_PASSWORD:
            if (!this->context.ask_password) {
                pszReturn = this->context.password.c_str();
            }
            break;

        case AUTHID_AUTHCHANNEL_ANSWER:
            pszReturn = this->context.authchannel_answer.c_str();
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            if (!this->context.ask_authchannel_result) {
                pszReturn = this->context.authchannel_result.c_str();
            }
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            if (!this->context.ask_authchannel_target) {
                pszReturn = this->context.authchannel_target.c_str();
            }
            break;

        case AUTHID_MESSAGE:
            pszReturn = this->context.message.c_str();
            break;
        case AUTHID_ACCEPT_MESSAGE:
            if (!this->context.ask_accept_message) {
                pszReturn = this->context.accept_message.c_str();
            }
            break;
        case AUTHID_DISPLAY_MESSAGE:
            if (!this->context.ask_display_message) {
                pszReturn = this->context.display_message.c_str();
            }
            break;

        case AUTHID_AUTHENTICATED:
            if (size) {
                strncpy(buffer, (this->context.authenticated ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_REJECTED:
            pszReturn = this->context.rejected.c_str();
            break;

        case AUTHID_KEEPALIVE:
            if (  size
               && !this->context.ask_keepalive) {
                strncpy(buffer, (this->context.keepalive ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            break;

        case AUTHID_PROXY_TYPE:
            if (!this->context.ask_proxy_type) {
                pszReturn = this->context.proxy_type.c_str();
            }
            break;
        case AUTHID_TRACE_SEAL:
            if (!this->context.ask_trace_seal) {
                pszReturn = this->context.trace_seal.c_str();
            }
            break;

        case AUTHID_SESSION_ID:
            pszReturn = this->context.session_id.c_str();
            break;

        case AUTHID_END_DATE_CNX:
            if (size) {
                snprintf(buffer, size, "%u", this->context.end_date_cnx);
                pszReturn = buffer;
            }
            break;
        case AUTHID_END_TIME:
            pszReturn = this->context.end_time.c_str();
            break;

        case AUTHID_MODE_CONSOLE:
            pszReturn = this->context.mode_console.c_str();
            break;
        case AUTHID_TIMEZONE:
            if (size) {
                snprintf(buffer, size, "%d", this->context.timezone);
                pszReturn = buffer;
            }
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            pszReturn = this->context.real_target_device.c_str();
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            pszReturn = this->context.authentication_challenge.c_str();
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
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
        switch (authid) {
        case AUTHID_OPT_BPP:
            this->context.state_opt_bpp.ask                 = true;
            this->context.state_opt_bpp.modified                 = true;
            this->context.ask_opt_bpp                 = true; break;

        case AUTHID_OPT_HEIGHT:
            this->context.state_opt_height.ask              = true;
            this->context.state_opt_height.modified              = true;
            this->context.ask_opt_height              = true; break;

        case AUTHID_OPT_WIDTH:
            this->context.state_opt_width.ask               = true;
            this->context.state_opt_width.modified               = true;
            this->context.ask_opt_width               = true; break;

        case AUTHID_SELECTOR:
            this->context.state_selector.ask                = true;
            this->context.state_selector.modified                = true;
            this->context.ask_selector                = true; break;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->context.state_selector_current_page.ask   = true;
            this->context.state_selector_current_page.modified   = true;
            this->context.ask_selector_current_page   = true; break;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->context.state_selector_device_filter.ask  = true;
            this->context.state_selector_device_filter.modified  = true;
            this->context.ask_selector_device_filter  = true; break;

        case AUTHID_SELECTOR_GROUP_FILTER:
            this->context.state_selector_group_filter.ask   = true;
            this->context.state_selector_group_filter.modified   = true;
            this->context.ask_selector_group_filter   = true; break;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->context.state_selector_lines_per_page.ask = true;
            this->context.state_selector_lines_per_page.modified = true;
            this->context.ask_selector_lines_per_page = true; break;

        case AUTHID_TARGET_DEVICE:
            this->globals.state_target_device.ask           = true;
            this->globals.state_target_device.modified           = true;
            this->context.ask_target_device           = true; break;

        case AUTHID_TARGET_PASSWORD:
            this->context.state_target_password.ask         = true;
            this->context.state_target_password.modified         = true;
            this->context.ask_target_password         = true; break;

        case AUTHID_TARGET_PORT:
            this->context.state_target_port.ask             = true;
            this->context.state_target_port.modified             = true;
            this->context.ask_target_port             = true; break;

        case AUTHID_TARGET_PROTOCOL:
            this->context.state_target_protocol.ask         = true;
            this->context.state_target_protocol.modified         = true;
            this->context.ask_target_protocol         = true; break;

        case AUTHID_TARGET_USER:
            this->globals.state_target_user.ask             = true;
            this->globals.state_target_user.modified             = true;
            this->context.ask_target_user             = true; break;

        case AUTHID_AUTH_USER:
            this->globals.state_auth_user.ask               = true;
            this->globals.state_auth_user.modified               = true;
            this->context.ask_auth_user               = true; break;

        case AUTHID_HOST:
            this->globals.state_host.ask                    = true;
            this->globals.state_host.modified                    = true;
            this->context.ask_host                    = true; break;

        case AUTHID_TARGET:
            this->globals.state_target.ask                  = true;
            this->globals.state_target.modified                  = true;
            this->context.ask_target                  = true; break;

        case AUTHID_PASSWORD:
            this->context.state_password.ask                = true;
            this->context.state_password.modified                = true;
            this->context.ask_password                = true; break;

        case AUTHID_AUTHCHANNEL_RESULT:
            this->context.state_authchannel_result.ask      = true;
            this->context.state_authchannel_result.modified      = true;
            this->context.ask_authchannel_result      = true; break;

        case AUTHID_AUTHCHANNEL_TARGET:
            this->context.state_authchannel_target.ask      = true;
            this->context.state_authchannel_target.modified      = true;
            this->context.ask_authchannel_target      = true; break;

        case AUTHID_ACCEPT_MESSAGE:
            this->context.state_accept_message.ask          = true;
            this->context.state_accept_message.modified          = true;
            this->context.ask_accept_message          = true; break;

        case AUTHID_DISPLAY_MESSAGE:
            this->context.state_display_message.ask         = true;
            this->context.state_display_message.modified         = true;
            this->context.ask_display_message         = true; break;

        case AUTHID_KEEPALIVE:
            this->context.state_keepalive.ask               = true;
            this->context.state_keepalive.modified               = true;
            this->context.ask_keepalive               = true; break;

        case AUTHID_PROXY_TYPE:
            this->context.state_proxy_type.ask              = true;
            this->context.state_proxy_type.modified              = true;
            this->context.ask_proxy_type              = true; break;

        case AUTHID_TRACE_SEAL:
            this->context.state_trace_seal.ask              = true;
            this->context.state_trace_seal.modified              = true;
            this->context.ask_trace_seal              = true; break;

        default:
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
            break;
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
    TODO("change ask_* into state_*.ask")
    bool context_is_asked(authid_t authid) {
        switch (authid) {
        case AUTHID_OPT_BPP:
            return this->context.ask_opt_bpp;

        case AUTHID_OPT_HEIGHT:
            return this->context.ask_opt_height;

        case AUTHID_OPT_WIDTH:
            return this->context.ask_opt_width;

        case AUTHID_SELECTOR:
            return this->context.ask_selector;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            return this->context.ask_selector_current_page;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            return this->context.ask_selector_device_filter;

        case AUTHID_SELECTOR_GROUP_FILTER:
            return this->context.ask_selector_group_filter;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            return this->context.ask_selector_lines_per_page;

        case AUTHID_TARGET_DEVICE:
            return this->context.ask_target_device;

        case AUTHID_TARGET_PASSWORD:
            return this->context.ask_target_password;

        case AUTHID_TARGET_PORT:
            return this->context.ask_target_port;

        case AUTHID_TARGET_PROTOCOL:
            return this->context.ask_target_protocol;

        case AUTHID_TARGET_USER:
            return this->context.ask_target_user;

        case AUTHID_AUTH_USER:
            return this->context.ask_auth_user;

        case AUTHID_HOST:
            return this->context.ask_host;

        case AUTHID_TARGET:
            return this->context.ask_target;

        case AUTHID_PASSWORD:
            return this->context.ask_password;

        case AUTHID_AUTHCHANNEL_RESULT:
            return this->context.ask_authchannel_result;

        case AUTHID_AUTHCHANNEL_TARGET:
            return this->context.ask_authchannel_target;

        case AUTHID_ACCEPT_MESSAGE:
            return this->context.ask_accept_message;

        case AUTHID_DISPLAY_MESSAGE:
            return this->context.ask_display_message;

        case AUTHID_KEEPALIVE:
            return this->context.ask_keepalive;

        case AUTHID_PROXY_TYPE:
            return this->context.ask_proxy_type;

        case AUTHID_TRACE_SEAL:
            return this->context.ask_trace_seal;

        case AUTHID_REAL_TARGET_DEVICE:
            return false;

        default:
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
    }

    bool context_get_bool(authid_t authid) {
        switch (authid)
        {
        case AUTHID_SELECTOR:
            if (!this->context.ask_selector) {
                return this->context.selector;
            }
            break;
        case AUTHID_KEEPALIVE:
            if (!this->context.ask_keepalive) {
                return this->context.keepalive;
            }
            break;
        case AUTHID_AUTHENTICATED:
            return this->context.authenticated;
        default:
            LOG(LOG_WARNING, "Inifile::context_get_bool(id): unknown authid=\"%d\"", authid);
            break;
        }

        return false;
    }

    void cparse(const char * filename) {
        ifstream inifile(filename);
        this->cparse(inifile);
    }

    void parse_username(const char * username)
    {
//        LOG(LOG_INFO, "parse_username(%s)", username);
        TODO("These should be results of the parsing function, not storing it away immediately in context. Mixing context management and parsing is not right")
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

        if (*target_user == 0)
        {
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
    }

    // NEW SETTERS GETTERS FUNCTIONS
    /*-----------------------------------
    // GLOBALS SECTION
    ------------------------------------*/
    void set_globals_capture_chunk(bool arg) {
        if (this->globals.capture_chunk != arg) {
            this->globals.state_capture_chunk.modified = true;
            this->globals.state_capture_chunk.read = false;
        }
        this->globals.capture_chunk = arg;
    }

    const bool get_globals_capture_chunk() {
        this->globals.state_capture_chunk.read = true;
        return this->globals.capture_chunk;
    }

    bool has_changed_globals_capture_chunk() {
        meta_state_t * meta = &this->globals.state_capture_chunk;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_capture_chunk() {
        return this->globals.state_capture_chunk.read;
    }

    void set_globals_auth_user(const char * arg) {
        if (strcmp(this->globals.auth_user, arg)) {
            this->globals.state_auth_user.modified = true;
            this->globals.state_auth_user.read = false;
        }
        memcpy(this->globals.auth_user, arg, sizeof(arg));
    }

    const char * get_globals_auth_user() {
        this->globals.state_auth_user.read = true;
        return this->globals.auth_user;
    }

    bool has_changed_globals_auth_user() {
        meta_state_t * meta = &this->globals.state_auth_user;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_auth_user() {
        return this->globals.state_auth_user.read;
    }

    void set_globals_host(const char * arg) {
        if (strcmp(this->globals.host, arg)) {
            this->globals.state_host.modified = true;
            this->globals.state_host.read = false;
        }
        memcpy(this->globals.host, arg, sizeof(arg));
    }

    const char * get_globals_host() {
        this->globals.state_host.read = true;
        return this->globals.host;
    }

    bool has_changed_globals_host() {
        meta_state_t * meta = &this->globals.state_host;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_host() {
        return this->globals.state_host.read;
    }

    void set_globals_target(const char * arg) {
        if (strcmp(this->globals.target, arg)) {
            this->globals.state_target.modified = true;
            this->globals.state_target.read = false;
        }
        memcpy(this->globals.target, arg, sizeof(arg));
    }

    const char * get_globals_target() {
        this->globals.state_target.read = true;
        return this->globals.target;
    }

    bool has_changed_globals_target() {
        meta_state_t * meta = &this->globals.state_target;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_target() {
        return this->globals.state_target.read;
    }

    void set_globals_target_device(const char * arg) {
        if (strcmp(this->globals.target_device, arg)) {
            this->globals.state_target_device.modified = true;
            this->globals.state_target_device.read = false;
        }
        memcpy(this->globals.target_device, arg, sizeof(arg));
    }

    const char * get_globals_target_device() {
        this->globals.state_target_device.read = true;
        return this->globals.target_device;
    }

    bool has_changed_globals_target_device() {
        meta_state_t * meta = &this->globals.state_target_device;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_target_device() {
        return this->globals.state_target_device.read;
    }

    void set_globals_target_user(const char * arg) {
        if (strcmp(this->globals.target_user, arg)) {
            this->globals.state_target_user.modified = true;
            this->globals.state_target_user.read = false;
        }
        memcpy(this->globals.target_user, arg, sizeof(arg));
    }

    const char * get_globals_target_user() {
        this->globals.state_target_user.read = true;
        return this->globals.target_user;
    }

    bool has_changed_globals_target_user() {
        meta_state_t * meta = &this->globals.state_target_user;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_target_user() {
        return this->globals.state_target_user.read;
    }

    void set_globals_bitmap_cache(bool arg) {
        if (this->globals.bitmap_cache != arg) {
            this->globals.state_bitmap_cache.modified = true;
            this->globals.state_bitmap_cache.read = false;
        }
        this->globals.bitmap_cache = arg;
    }

    const bool get_globals_bitmap_cache() {
        this->globals.state_bitmap_cache.read = true;
        return this->globals.bitmap_cache;
    }

    bool has_changed_globals_bitmap_cache() {
        meta_state_t * meta = &this->globals.state_bitmap_cache;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_bitmap_cache() {
        return this->globals.state_bitmap_cache.read;
    }

    void set_globals_bitmap_compression(bool arg) {
        if (this->globals.bitmap_compression != arg) {
            this->globals.state_bitmap_compression.modified = true;
            this->globals.state_bitmap_compression.read = false;
        }
        this->globals.bitmap_compression = arg;
    }

    const bool get_globals_bitmap_compression() {
        this->globals.state_bitmap_compression.read = true;
        return this->globals.bitmap_compression;
    }

    bool has_changed_globals_bitmap_compression() {
        meta_state_t * meta = &this->globals.state_bitmap_compression;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_bitmap_compression() {
        return this->globals.state_bitmap_compression.read;
    }

    void set_globals_port(int arg) {
        if (this->globals.port != arg) {
            this->globals.state_port.modified = true;
            this->globals.state_port.read = false;
        }
        this->globals.port = arg;
    }

    const int get_globals_port() {
        this->globals.state_port.read = true;
        return this->globals.port;
    }

    bool has_changed_globals_port() {
        meta_state_t * meta = &this->globals.state_port;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_port() {
        return this->globals.state_port.read;
    }

    void set_globals_nomouse(bool arg) {
        if (this->globals.nomouse != arg) {
            this->globals.state_nomouse.modified = true;
            this->globals.state_nomouse.read = false;
        }
        this->globals.nomouse = arg;
    }

    const bool get_globals_nomouse() {
        this->globals.state_nomouse.read = true;
        return this->globals.nomouse;
    }

    bool has_changed_globals_nomouse() {
        meta_state_t * meta = &this->globals.state_nomouse;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_nomouse() {
        return this->globals.state_nomouse.read;
    }

    void set_globals_notimestamp(bool arg) {
        if (this->globals.notimestamp != arg) {
            this->globals.state_notimestamp.modified = true;
            this->globals.state_notimestamp.read = false;
        }
        this->globals.notimestamp = arg;
    }

    const bool get_globals_notimestamp() {
        this->globals.state_notimestamp.read = true;
        return this->globals.notimestamp;
    }

    bool has_changed_globals_notimestamp() {
        meta_state_t * meta = &this->globals.state_notimestamp;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_notimestamp() {
        return this->globals.state_notimestamp.read;
    }

    void set_globals_encryptionLevel(int arg) {
        if (this->globals.encryptionLevel != arg) {
            this->globals.state_encryptionLevel.modified = true;
            this->globals.state_encryptionLevel.read = false;
        }
        this->globals.encryptionLevel = arg;
    }

    const int get_globals_encryptionLevel() {
        this->globals.state_encryptionLevel.read = true;
        return this->globals.encryptionLevel;
    }

    bool has_changed_globals_encryptionLevel() {
        meta_state_t * meta = &this->globals.state_encryptionLevel;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_encryptionLevel() {
        return this->globals.state_encryptionLevel.read;
    }

    void set_globals_authip(const char * arg) {
        if (strcmp(this->globals.authip, arg)) {
            this->globals.state_authip.modified = true;
            this->globals.state_authip.read = false;
        }
        memcpy(this->globals.authip, arg, sizeof(arg));
    }

    const char * get_globals_authip() {
        this->globals.state_authip.read = true;
        return this->globals.authip;
    }

    bool has_changed_globals_authip() {
        meta_state_t * meta = &this->globals.state_authip;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_authip() {
        return this->globals.state_authip.read;
    }

    void set_globals_authport(int arg) {
        if (this->globals.authport != arg) {
            this->globals.state_authport.modified = true;
            this->globals.state_authport.read = false;
        }
        this->globals.authport = arg;
    }

    const int get_globals_authport() {
        this->globals.state_authport.read = true;
        return this->globals.authport;
    }

    bool has_changed_globals_authport() {
        meta_state_t * meta = &this->globals.state_authport;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_authport() {
        return this->globals.state_authport.read;
    }

    void set_globals_autovalidate(bool arg) {
        if (this->globals.autovalidate != arg) {
            this->globals.state_autovalidate.modified = true;
            this->globals.state_autovalidate.read = false;
        }
        this->globals.autovalidate = arg;
    }

    const bool get_globals_autovalidate() {
        this->globals.state_autovalidate.read = true;
        return this->globals.autovalidate;
    }

    bool has_changed_globals_autovalidate() {
        meta_state_t * meta = &this->globals.state_autovalidate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_autovalidate() {
        return this->globals.state_autovalidate.read;
    }

    void set_globals_max_tick(int arg) {
        if (this->globals.max_tick != arg) {
            this->globals.state_max_tick.modified = true;
            this->globals.state_max_tick.read = false;
        }
        this->globals.max_tick = arg;
    }

    const int get_globals_max_tick() {
        this->globals.state_max_tick.read = true;
        return this->globals.max_tick;
    }

    bool has_changed_globals_max_tick() {
        meta_state_t * meta = &this->globals.state_max_tick;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_max_tick() {
        return this->globals.state_max_tick.read;
    }

    void set_globals_keepalive_grace_delay(int arg) {
        if (this->globals.keepalive_grace_delay != arg) {
            this->globals.state_keepalive_grace_delay.modified = true;
            this->globals.state_keepalive_grace_delay.read = false;
        }
        this->globals.keepalive_grace_delay = arg;
    }

    const int get_globals_keepalive_grace_delay() {
        this->globals.state_keepalive_grace_delay.read = true;
        return this->globals.keepalive_grace_delay;
    }

    bool has_changed_globals_keepalive_grace_delay() {
        meta_state_t * meta = &this->globals.state_keepalive_grace_delay;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_keepalive_grace_delay() {
        return this->globals.state_keepalive_grace_delay.read;
    }

    void set_globals_internal_domain(bool arg) {
        if (this->globals.internal_domain != arg) {
            this->globals.state_internal_domain.modified = true;
            this->globals.state_internal_domain.read = false;
        }
        this->globals.internal_domain = arg;
    }

    const bool get_globals_internal_domain() {
        this->globals.state_internal_domain.read = true;
        return this->globals.internal_domain;
    }

    bool has_changed_globals_internal_domain() {
        meta_state_t * meta = &this->globals.state_internal_domain;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_internal_domain() {
        return this->globals.state_internal_domain.read;
    }

    void set_globals_dynamic_conf_path(const char * arg) {
        if (strcmp(this->globals.dynamic_conf_path, arg)) {
            this->globals.state_dynamic_conf_path.modified = true;
            this->globals.state_dynamic_conf_path.read = false;
        }
        memcpy(this->globals.dynamic_conf_path, arg, sizeof(arg));
    }

    const char * get_globals_dynamic_conf_path() {
        this->globals.state_dynamic_conf_path.read = true;
        return this->globals.dynamic_conf_path;
    }

    bool has_changed_globals_dynamic_conf_path() {
        meta_state_t * meta = &this->globals.state_dynamic_conf_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_dynamic_conf_path() {
        return this->globals.state_dynamic_conf_path.read;
    }

    void set_globals_auth_channel(const char * arg) {
        if (strcmp(this->globals.auth_channel, arg)) {
            this->globals.state_auth_channel.modified = true;
            this->globals.state_auth_channel.read = false;
        }
        memcpy(this->globals.auth_channel, arg, sizeof(arg));
    }

    const char * get_globals_auth_channel() {
        this->globals.state_auth_channel.read = true;
        return this->globals.auth_channel;
    }

    bool has_changed_globals_auth_channel() {
        meta_state_t * meta = &this->globals.state_auth_channel;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_auth_channel() {
        return this->globals.state_auth_channel.read;
    }

    void set_globals_enable_file_encryption(bool arg) {
        if (this->globals.enable_file_encryption != arg) {
            this->globals.state_enable_file_encryption.modified = true;
            this->globals.state_enable_file_encryption.read = false;
        }
        this->globals.enable_file_encryption = arg;
    }

    const bool get_globals_enable_file_encryption() {
        this->globals.state_enable_file_encryption.read = true;
        return this->globals.enable_file_encryption;
    }

    bool has_changed_globals_enable_file_encryption() {
        meta_state_t * meta = &this->globals.state_enable_file_encryption;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_enable_file_encryption() {
        return this->globals.state_enable_file_encryption.read;
    }

    void set_globals_enable_tls(bool arg) {
        if (this->globals.enable_tls != arg) {
            this->globals.state_enable_tls.modified = true;
            this->globals.state_enable_tls.read = false;
        }
        this->globals.enable_tls = arg;
    }

    const bool get_globals_enable_tls() {
        this->globals.state_enable_tls.read = true;
        return this->globals.enable_tls;
    }

    bool has_changed_globals_enable_tls() {
        meta_state_t * meta = &this->globals.state_enable_tls;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_enable_tls() {
        return this->globals.state_enable_tls.read;
    }

    void set_globals_listen_address(const char * arg) {
        if (strcmp(this->globals.listen_address, arg)) {
            this->globals.state_listen_address.modified = true;
            this->globals.state_listen_address.read = false;
        }
        memcpy(this->globals.listen_address, arg, sizeof(arg));
    }

    const char * get_globals_listen_address() {
        this->globals.state_listen_address.read = true;
        return this->globals.listen_address;
    }

    bool has_changed_globals_listen_address() {
        meta_state_t * meta = &this->globals.state_listen_address;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_listen_address() {
        return this->globals.state_listen_address.read;
    }

    void set_globals_enable_ip_transparent(bool arg) {
        if (this->globals.enable_ip_transparent != arg) {
            this->globals.state_enable_ip_transparent.modified = true;
            this->globals.state_enable_ip_transparent.read = false;
        }
        this->globals.enable_ip_transparent = arg;
    }

    const bool get_globals_enable_ip_transparent() {
        this->globals.state_enable_ip_transparent.read = true;
        return this->globals.enable_ip_transparent;
    }

    bool has_changed_globals_enable_ip_transparent() {
        meta_state_t * meta = &this->globals.state_enable_ip_transparent;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_enable_ip_transparent() {
        return this->globals.state_enable_ip_transparent.read;
    }

    void set_globals_certificate_password(const char * arg) {
        if (strcmp(this->globals.certificate_password, arg)) {
            this->globals.state_certificate_password.modified = true;
            this->globals.state_certificate_password.read = false;
        }
        memcpy(this->globals.certificate_password, arg, sizeof(arg));
    }

    const char * get_globals_certificate_password() {
        this->globals.state_certificate_password.read = true;
        return this->globals.certificate_password;
    }

    bool has_changed_globals_certificate_password() {
        meta_state_t * meta = &this->globals.state_certificate_password;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_certificate_password() {
        return this->globals.state_certificate_password.read;
    }

    void set_globals_png_path(const char * arg) {
        if (strcmp(this->globals.png_path, arg)) {
            this->globals.state_png_path.modified = true;
            this->globals.state_png_path.read = false;
        }
        memcpy(this->globals.png_path, arg, sizeof(arg));
    }

    const char * get_globals_png_path() {
        this->globals.state_png_path.read = true;
        return this->globals.png_path;
    }

    bool has_changed_globals_png_path() {
        meta_state_t * meta = &this->globals.state_png_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_png_path() {
        return this->globals.state_png_path.read;
    }

    void set_globals_wrm_path(const char * arg) {
        if (strcmp(this->globals.wrm_path, arg)) {
            this->globals.state_wrm_path.modified = true;
            this->globals.state_wrm_path.read = false;
        }
        memcpy(this->globals.wrm_path, arg, sizeof(arg));
    }

    const char * get_globals_wrm_path() {
        this->globals.state_wrm_path.read = true;
        return this->globals.wrm_path;
    }

    bool has_changed_globals_wrm_path() {
        meta_state_t * meta = &this->globals.state_wrm_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_wrm_path() {
        return this->globals.state_wrm_path.read;
    }

    void set_globals_alternate_shell(const char * arg) {
        if (strcmp(this->globals.alternate_shell, arg)) {
            this->globals.state_alternate_shell.modified = true;
            this->globals.state_alternate_shell.read = false;
        }
        memcpy(this->globals.alternate_shell, arg, sizeof(arg));
    }

    const char * get_globals_alternate_shell() {
        this->globals.state_alternate_shell.read = true;
        return this->globals.alternate_shell;
    }

    bool has_changed_globals_alternate_shell() {
        meta_state_t * meta = &this->globals.state_alternate_shell;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_alternate_shell() {
        return this->globals.state_alternate_shell.read;
    }

    void set_globals_shell_working_directory(const char * arg) {
        if (strcmp(this->globals.shell_working_directory, arg)) {
            this->globals.state_shell_working_directory.modified = true;
            this->globals.state_shell_working_directory.read = false;
        }
        memcpy(this->globals.shell_working_directory, arg, sizeof(arg));
    }

    const char * get_globals_shell_working_directory() {
        this->globals.state_shell_working_directory.read = true;
        return this->globals.shell_working_directory;
    }

    bool has_changed_globals_shell_working_directory() {
        meta_state_t * meta = &this->globals.state_shell_working_directory;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_shell_working_directory() {
        return this->globals.state_shell_working_directory.read;
    }

    void set_globals_codec_id(const char * arg) {
        if (strcmp(this->globals.codec_id, arg)) {
            this->globals.state_codec_id.modified = true;
            this->globals.state_codec_id.read = false;
        }
        memcpy(this->globals.codec_id, arg, sizeof(arg));
    }

    const char * get_globals_codec_id() {
        this->globals.state_codec_id.read = true;
        return this->globals.codec_id;
    }

    bool has_changed_globals_codec_id() {
        meta_state_t * meta = &this->globals.state_codec_id;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_codec_id() {
        return this->globals.state_codec_id.read;
    }

    void set_globals_movie(bool arg) {
        if (this->globals.movie != arg) {
            this->globals.state_movie.modified = true;
            this->globals.state_movie.read = false;
        }
        this->globals.movie = arg;
    }

    const bool get_globals_movie() {
        this->globals.state_movie.read = true;
        return this->globals.movie;
    }

    bool has_changed_globals_movie() {
        meta_state_t * meta = &this->globals.state_movie;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_movie() {
        return this->globals.state_movie.read;
    }

    void set_globals_movie_path(const char * arg) {
        if (strcmp(this->globals.movie_path, arg)) {
            this->globals.state_movie_path.modified = true;
            this->globals.state_movie_path.read = false;
        }
        memcpy(this->globals.movie_path, arg, sizeof(arg));
    }

    const char * get_globals_movie_path() {
        this->globals.state_movie_path.read = true;
        return this->globals.movie_path;
    }

    bool has_changed_globals_movie_path() {
        meta_state_t * meta = &this->globals.state_movie_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_movie_path() {
        return this->globals.state_movie_path.read;
    }

    void set_globals_video_quality(const char * arg) {
        if (strcmp(this->globals.video_quality, arg)) {
            this->globals.state_video_quality.modified = true;
            this->globals.state_video_quality.read = false;
        }
        memcpy(this->globals.video_quality, arg, sizeof(arg));
    }

    const char * get_globals_video_quality() {
        this->globals.state_video_quality.read = true;
        return this->globals.video_quality;
    }

    bool has_changed_globals_video_quality() {
        meta_state_t * meta = &this->globals.state_video_quality;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_video_quality() {
        return this->globals.state_video_quality.read;
    }

    void set_globals_enable_bitmap_update(bool arg) {
        if (this->globals.enable_bitmap_update != arg) {
            this->globals.state_enable_bitmap_update.modified = true;
            this->globals.state_enable_bitmap_update.read = false;
        }
        this->globals.enable_bitmap_update = arg;
    }

    const bool get_globals_enable_bitmap_update() {
        this->globals.state_enable_bitmap_update.read = true;
        return this->globals.enable_bitmap_update;
    }

    bool has_changed_globals_enable_bitmap_update() {
        meta_state_t * meta = &this->globals.state_enable_bitmap_update;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_enable_bitmap_update() {
        return this->globals.state_enable_bitmap_update.read;
    }

    void set_globals_flv_break_interval(uint64_t arg) {
        if (this->globals.flv_break_interval != arg) {
            this->globals.state_flv_break_interval.modified = true;
            this->globals.state_flv_break_interval.read = false;
        }
        this->globals.flv_break_interval = arg;
    }

    const uint64_t get_globals_flv_break_interval() {
        this->globals.state_flv_break_interval.read = true;
        return this->globals.flv_break_interval;
    }

    bool has_changed_globals_flv_break_interval() {
        meta_state_t * meta = &this->globals.state_flv_break_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_flv_break_interval() {
        return this->globals.state_flv_break_interval.read;
    }

    void set_globals_flv_frame_interval(unsigned arg) {
        if (this->globals.flv_frame_interval != arg) {
            this->globals.state_flv_frame_interval.modified = true;
            this->globals.state_flv_frame_interval.read = false;
        }
        this->globals.flv_frame_interval = arg;
    }

    const unsigned get_globals_flv_frame_interval() {
        this->globals.state_flv_frame_interval.read = true;
        return this->globals.flv_frame_interval;
    }

    bool has_changed_globals_flv_frame_interval() {
        meta_state_t * meta = &this->globals.state_flv_frame_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_globals_flv_frame_interval() {
        return this->globals.state_flv_frame_interval.read;
    }

    /*-----------------------------------
    // CLIENT SECTION
    ------------------------------------*/

    void set_client_ignore_logon_password(bool arg) {
        if (this->client.ignore_logon_password != arg) {
            this->client.state_ignore_logon_password.modified = true;
            this->client.state_ignore_logon_password.read = false;
        }
        this->client.ignore_logon_password = arg;
    }

    const bool get_client_ignore_logon_password() {
        this->client.state_ignore_logon_password.read = true;
        return this->client.ignore_logon_password;
    }

    bool has_changed_client_ignore_logon_password() {
        meta_state_t * meta = &this->client.state_ignore_logon_password;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_ignore_logon_password() {
        return this->client.state_ignore_logon_password.read;
    }

    void set_client_performance_flags_default(uint32_t arg) {
        if (this->client.performance_flags_default != arg) {
            this->client.state_performance_flags_default.modified = true;
            this->client.state_performance_flags_default.read = false;
        }
        this->client.performance_flags_default = arg;
    }

    const uint32_t get_client_performance_flags_default() {
        this->client.state_performance_flags_default.read = true;
        return this->client.performance_flags_default;
    }

    bool has_changed_client_performance_flags_default() {
        meta_state_t * meta = &this->client.state_performance_flags_default;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_performance_flags_default() {
        return this->client.state_performance_flags_default.read;
    }

    void set_client_performance_flags_force_present(uint32_t arg) {
        if (this->client.performance_flags_force_present != arg) {
            this->client.state_performance_flags_force_present.modified = true;
            this->client.state_performance_flags_force_present.read = false;
        }
        this->client.performance_flags_force_present = arg;
    }

    const uint32_t get_client_performance_flags_force_present() {
        this->client.state_performance_flags_force_present.read = true;
        return this->client.performance_flags_force_present;
    }

    bool has_changed_client_performance_flags_force_present() {
        meta_state_t * meta = &this->client.state_performance_flags_force_present;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_performance_flags_force_present() {
        return this->client.state_performance_flags_force_present.read;
    }

    void set_client_performance_flags_force_not_present(uint32_t arg) {
        if (this->client.performance_flags_force_not_present != arg) {
            this->client.state_performance_flags_force_not_present.modified = true;
            this->client.state_performance_flags_force_not_present.read = false;
        }
        this->client.performance_flags_force_not_present = arg;
    }

    const uint32_t get_client_performance_flags_force_not_present() {
        this->client.state_performance_flags_force_not_present.read = true;
        return this->client.performance_flags_force_not_present;
    }

    bool has_changed_client_performance_flags_force_not_present() {
        meta_state_t * meta = &this->client.state_performance_flags_force_not_present;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_performance_flags_force_not_present() {
        return this->client.state_performance_flags_force_not_present.read;
    }

    void set_client_tls_fallback_legacy(bool arg) {
        if (this->client.tls_fallback_legacy != arg) {
            this->client.state_tls_fallback_legacy.modified = true;
            this->client.state_tls_fallback_legacy.read = false;
        }
        this->client.tls_fallback_legacy = arg;
    }

    const bool get_client_tls_fallback_legacy() {
        this->client.state_tls_fallback_legacy.read = true;
        return this->client.tls_fallback_legacy;
    }

    bool has_changed_client_tls_fallback_legacy() {
        meta_state_t * meta = &this->client.state_tls_fallback_legacy;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_tls_fallback_legacy() {
        return this->client.state_tls_fallback_legacy.read;
    }

    void set_client_clipboard(bool arg) {
        if (this->client.clipboard != arg) {
            this->client.state_clipboard.modified = true;
            this->client.state_clipboard.read = false;
        }
        this->client.clipboard = arg;
    }

    const bool get_client_clipboard() {
        this->client.state_clipboard.read = true;
        return this->client.clipboard;
    }

    bool has_changed_client_clipboard() {
        meta_state_t * meta = &this->client.state_clipboard;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_clipboard() {
        return this->client.state_clipboard.read;
    }

    void set_client_device_redirection(bool arg) {
        if (this->client.device_redirection != arg) {
            this->client.state_device_redirection.modified = true;
            this->client.state_device_redirection.read = false;
        }
        this->client.device_redirection = arg;
    }

    const bool get_client_device_redirection() {
        this->client.state_device_redirection.read = true;
        return this->client.device_redirection;
    }

    bool has_changed_client_device_redirection() {
        meta_state_t * meta = &this->client.state_device_redirection;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_client_device_redirection() {
        return this->client.state_device_redirection.read;
    }

    /*-----------------------------------
    // VIDEO SECTION
    ------------------------------------*/

    void set_video_capture_flags(unsigned arg) {
        if (this->video.capture_flags != arg) {
            this->video.state_capture_flags.modified = true;
            this->video.state_capture_flags.read = false;
        }
        this->video.capture_flags = arg;
    }

    const unsigned get_video_capture_flags() {
        this->video.state_capture_flags.read = true;
        return this->video.capture_flags;
    }

    bool has_changed_video_capture_flags() {
        meta_state_t * meta = &this->video.state_capture_flags;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_flags() {
        return this->video.state_capture_flags.read;
    }

    void set_video_capture_png(bool arg) {
        if (this->video.capture_png != arg) {
            this->video.state_capture_png.modified = true;
            this->video.state_capture_png.read = false;
        }
        this->video.capture_png = arg;
    }

    const bool get_video_capture_png() {
        this->video.state_capture_png.read = true;
        return this->video.capture_png;
    }

    bool has_changed_video_capture_png() {
        meta_state_t * meta = &this->video.state_capture_png;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_png() {
        return this->video.state_capture_png.read;
    }

    void set_video_capture_wrm(bool arg) {
        if (this->video.capture_wrm != arg) {
            this->video.state_capture_wrm.modified = true;
            this->video.state_capture_wrm.read = false;
        }
        this->video.capture_wrm = arg;
    }

    const bool get_video_capture_wrm() {
        this->video.state_capture_wrm.read = true;
        return this->video.capture_wrm;
    }

    bool has_changed_video_capture_wrm() {
        meta_state_t * meta = &this->video.state_capture_wrm;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_wrm() {
        return this->video.state_capture_wrm.read;
    }

    void set_video_capture_flv(bool arg) {
        if (this->video.capture_flv != arg) {
            this->video.state_capture_flv.modified = true;
            this->video.state_capture_flv.read = false;
        }
        this->video.capture_flv = arg;
    }

    const bool get_video_capture_flv() {
        this->video.state_capture_flv.read = true;
        return this->video.capture_flv;
    }

    bool has_changed_video_capture_flv() {
        meta_state_t * meta = &this->video.state_capture_flv;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_flv() {
        return this->video.state_capture_flv.read;
    }

    void set_video_capture_ocr(bool arg) {
        if (this->video.capture_ocr != arg) {
            this->video.state_capture_ocr.modified = true;
            this->video.state_capture_ocr.read = false;
        }
        this->video.capture_ocr = arg;
    }

    const bool get_video_capture_ocr() {
        this->video.state_capture_ocr.read = true;
        return this->video.capture_ocr;
    }

    bool has_changed_video_capture_ocr() {
        meta_state_t * meta = &this->video.state_capture_ocr;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_ocr() {
        return this->video.state_capture_ocr.read;
    }

    void set_video_ocr_interval(unsigned arg) {
        if (this->video.ocr_interval != arg) {
            this->video.state_ocr_interval.modified = true;
            this->video.state_ocr_interval.read = false;
        }
        this->video.ocr_interval = arg;
    }

    const unsigned get_video_ocr_interval() {
        this->video.state_ocr_interval.read = true;
        return this->video.ocr_interval;
    }

    bool has_changed_video_ocr_interval() {
        meta_state_t * meta = &this->video.state_ocr_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_ocr_interval() {
        return this->video.state_ocr_interval.read;
    }

    void set_video_png_interval(unsigned arg) {
        if (this->video.png_interval != arg) {
            this->video.state_png_interval.modified = true;
            this->video.state_png_interval.read = false;
        }
        this->video.png_interval = arg;
    }

    const unsigned get_video_png_interval() {
        this->video.state_png_interval.read = true;
        return this->video.png_interval;
    }

    bool has_changed_video_png_interval() {
        meta_state_t * meta = &this->video.state_png_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_png_interval() {
        return this->video.state_png_interval.read;
    }

    void set_video_capture_groupid(unsigned arg) {
        if (this->video.capture_groupid != arg) {
            this->video.state_capture_groupid.modified = true;
            this->video.state_capture_groupid.read = false;
        }
        this->video.capture_groupid = arg;
    }

    const unsigned get_video_capture_groupid() {
        this->video.state_capture_groupid.read = true;
        return this->video.capture_groupid;
    }

    bool has_changed_video_capture_groupid() {
        meta_state_t * meta = &this->video.state_capture_groupid;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_capture_groupid() {
        return this->video.state_capture_groupid.read;
    }

    void set_video_frame_interval(unsigned arg) {
        if (this->video.frame_interval != arg) {
            this->video.state_frame_interval.modified = true;
            this->video.state_frame_interval.read = false;
        }
        this->video.frame_interval = arg;
    }

    const unsigned get_video_frame_interval() {
        this->video.state_frame_interval.read = true;
        return this->video.frame_interval;
    }

    bool has_changed_video_frame_interval() {
        meta_state_t * meta = &this->video.state_frame_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_frame_interval() {
        return this->video.state_frame_interval.read;
    }

    void set_video_break_interval(unsigned arg) {
        if (this->video.break_interval != arg) {
            this->video.state_break_interval.modified = true;
            this->video.state_break_interval.read = false;
        }
        this->video.break_interval = arg;
    }

    const unsigned get_video_break_interval() {
        this->video.state_break_interval.read = true;
        return this->video.break_interval;
    }

    bool has_changed_video_break_interval() {
        meta_state_t * meta = &this->video.state_break_interval;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_break_interval() {
        return this->video.state_break_interval.read;
    }

    void set_video_png_limit(unsigned arg) {
        if (this->video.png_limit != arg) {
            this->video.state_png_limit.modified = true;
            this->video.state_png_limit.read = false;
        }
        this->video.png_limit = arg;
    }

    const unsigned get_video_png_limit() {
        this->video.state_png_limit.read = true;
        return this->video.png_limit;
    }

    bool has_changed_video_png_limit() {
        meta_state_t * meta = &this->video.state_png_limit;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_png_limit() {
        return this->video.state_png_limit.read;
    }

    void set_video_replay_path(const char * arg) {
        if (strcmp(this->video.replay_path, arg)) {
            this->video.state_replay_path.modified = true;
            this->video.state_replay_path.read = false;
        }
        memcpy(this->video.replay_path, arg, sizeof(arg));
    }

    const char * get_video_replay_path() {
        this->video.state_replay_path.read = true;
        return this->video.replay_path;
    }

    bool has_changed_video_replay_path() {
        meta_state_t * meta = &this->video.state_replay_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_replay_path() {
        return this->video.state_replay_path.read;
    }

    void set_video_l_bitrate(int arg) {
        if (this->video.l_bitrate != arg) {
            this->video.state_l_bitrate.modified = true;
            this->video.state_l_bitrate.read = false;
        }
        this->video.l_bitrate = arg;
    }

    const int get_video_l_bitrate() {
        this->video.state_l_bitrate.read = true;
        return this->video.l_bitrate;
    }

    bool has_changed_video_l_bitrate() {
        meta_state_t * meta = &this->video.state_l_bitrate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_l_bitrate() {
        return this->video.state_l_bitrate.read;
    }

    void set_video_l_framerate(int arg) {
        if (this->video.l_framerate != arg) {
            this->video.state_l_framerate.modified = true;
            this->video.state_l_framerate.read = false;
        }
        this->video.l_framerate = arg;
    }

    const int get_video_l_framerate() {
        this->video.state_l_framerate.read = true;
        return this->video.l_framerate;
    }

    bool has_changed_video_l_framerate() {
        meta_state_t * meta = &this->video.state_l_framerate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_l_framerate() {
        return this->video.state_l_framerate.read;
    }

    void set_video_l_height(int arg) {
        if (this->video.l_height != arg) {
            this->video.state_l_height.modified = true;
            this->video.state_l_height.read = false;
        }
        this->video.l_height = arg;
    }

    const int get_video_l_height() {
        this->video.state_l_height.read = true;
        return this->video.l_height;
    }

    bool has_changed_video_l_height() {
        meta_state_t * meta = &this->video.state_l_height;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_l_height() {
        return this->video.state_l_height.read;
    }

    void set_video_l_width(int arg) {
        if (this->video.l_width != arg) {
            this->video.state_l_width.modified = true;
            this->video.state_l_width.read = false;
        }
        this->video.l_width = arg;
    }

    const int get_video_l_width() {
        this->video.state_l_width.read = true;
        return this->video.l_width;
    }

    bool has_changed_video_l_width() {
        meta_state_t * meta = &this->video.state_l_width;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_l_width() {
        return this->video.state_l_width.read;
    }

    void set_video_l_qscale(int arg) {
        if (this->video.l_qscale != arg) {
            this->video.state_l_qscale.modified = true;
            this->video.state_l_qscale.read = false;
        }
        this->video.l_qscale = arg;
    }

    const int get_video_l_qscale() {
        this->video.state_l_qscale.read = true;
        return this->video.l_qscale;
    }

    bool has_changed_video_l_qscale() {
        meta_state_t * meta = &this->video.state_l_qscale;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_l_qscale() {
        return this->video.state_l_qscale.read;
    }

    void set_video_m_bitrate(int arg) {
        if (this->video.m_bitrate != arg) {
            this->video.state_m_bitrate.modified = true;
            this->video.state_m_bitrate.read = false;
        }
        this->video.m_bitrate = arg;
    }

    const int get_video_m_bitrate() {
        this->video.state_m_bitrate.read = true;
        return this->video.m_bitrate;
    }

    bool has_changed_video_m_bitrate() {
        meta_state_t * meta = &this->video.state_m_bitrate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_m_bitrate() {
        return this->video.state_m_bitrate.read;
    }

    void set_video_m_framerate(int arg) {
        if (this->video.m_framerate != arg) {
            this->video.state_m_framerate.modified = true;
            this->video.state_m_framerate.read = false;
        }
        this->video.m_framerate = arg;
    }

    const int get_video_m_framerate() {
        this->video.state_m_framerate.read = true;
        return this->video.m_framerate;
    }

    bool has_changed_video_m_framerate() {
        meta_state_t * meta = &this->video.state_m_framerate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_m_framerate() {
        return this->video.state_m_framerate.read;
    }

    void set_video_m_height(int arg) {
        if (this->video.m_height != arg) {
            this->video.state_m_height.modified = true;
            this->video.state_m_height.read = false;
        }
        this->video.m_height = arg;
    }

    const int get_video_m_height() {
        this->video.state_m_height.read = true;
        return this->video.m_height;
    }

    bool has_changed_video_m_height() {
        meta_state_t * meta = &this->video.state_m_height;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_m_height() {
        return this->video.state_m_height.read;
    }

    void set_video_m_width(int arg) {
        if (this->video.m_width != arg) {
            this->video.state_m_width.modified = true;
            this->video.state_m_width.read = false;
        }
        this->video.m_width = arg;
    }

    const int get_video_m_width() {
        this->video.state_m_width.read = true;
        return this->video.m_width;
    }

    bool has_changed_video_m_width() {
        meta_state_t * meta = &this->video.state_m_width;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_m_width() {
        return this->video.state_m_width.read;
    }

    void set_video_m_qscale(int arg) {
        if (this->video.m_qscale != arg) {
            this->video.state_m_qscale.modified = true;
            this->video.state_m_qscale.read = false;
        }
        this->video.m_qscale = arg;
    }

    const int get_video_m_qscale() {
        this->video.state_m_qscale.read = true;
        return this->video.m_qscale;
    }

    bool has_changed_video_m_qscale() {
        meta_state_t * meta = &this->video.state_m_qscale;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_m_qscale() {
        return this->video.state_m_qscale.read;
    }

    void set_video_h_bitrate(int arg) {
        if (this->video.h_bitrate != arg) {
            this->video.state_h_bitrate.modified = true;
            this->video.state_h_bitrate.read = false;
        }
        this->video.h_bitrate = arg;
    }

    const int get_video_h_bitrate() {
        this->video.state_h_bitrate.read = true;
        return this->video.h_bitrate;
    }

    bool has_changed_video_h_bitrate() {
        meta_state_t * meta = &this->video.state_h_bitrate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_h_bitrate() {
        return this->video.state_h_bitrate.read;
    }

    void set_video_h_framerate(int arg) {
        if (this->video.h_framerate != arg) {
            this->video.state_h_framerate.modified = true;
            this->video.state_h_framerate.read = false;
        }
        this->video.h_framerate = arg;
    }

    const int get_video_h_framerate() {
        this->video.state_h_framerate.read = true;
        return this->video.h_framerate;
    }

    bool has_changed_video_h_framerate() {
        meta_state_t * meta = &this->video.state_h_framerate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_h_framerate() {
        return this->video.state_h_framerate.read;
    }

    void set_video_h_height(int arg) {
        if (this->video.h_height != arg) {
            this->video.state_h_height.modified = true;
            this->video.state_h_height.read = false;
        }
        this->video.h_height = arg;
    }

    const int get_video_h_height() {
        this->video.state_h_height.read = true;
        return this->video.h_height;
    }

    bool has_changed_video_h_height() {
        meta_state_t * meta = &this->video.state_h_height;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_h_height() {
        return this->video.state_h_height.read;
    }

    void set_video_h_width(int arg) {
        if (this->video.h_width != arg) {
            this->video.state_h_width.modified = true;
            this->video.state_h_width.read = false;
        }
        this->video.h_width = arg;
    }

    const int get_video_h_width() {
        this->video.state_h_width.read = true;
        return this->video.h_width;
    }

    bool has_changed_video_h_width() {
        meta_state_t * meta = &this->video.state_h_width;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_h_width() {
        return this->video.state_h_width.read;
    }

    void set_video_h_qscale(int arg) {
        if (this->video.h_qscale != arg) {
            this->video.state_h_qscale.modified = true;
            this->video.state_h_qscale.read = false;
        }
        this->video.h_qscale = arg;
    }

    const int get_video_h_qscale() {
        this->video.state_h_qscale.read = true;
        return this->video.h_qscale;
    }

    bool has_changed_video_h_qscale() {
        meta_state_t * meta = &this->video.state_h_qscale;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_video_h_qscale() {
        return this->video.state_h_qscale.read;
    }
    /*------------------------------
    // DEBUG SECTION
    -------------------------------*/
    void set_debug_x224(uint32_t arg) {
        if (this->debug.x224 != arg) {
            this->debug.state_x224.modified = true;
            this->debug.state_x224.read = false;
        }
        this->debug.x224 = arg;
    }

    const uint32_t get_debug_x224() {
        this->debug.state_x224.read = true;
        return this->debug.x224;
    }

    bool has_changed_debug_x224() {
        meta_state_t * meta = &this->debug.state_x224;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_x224() {
        return this->debug.state_x224.read;
    }

    void set_debug_mcs(uint32_t arg) {
        if (this->debug.mcs != arg) {
            this->debug.state_mcs.modified = true;
            this->debug.state_mcs.read = false;
        }
        this->debug.mcs = arg;
    }

    const uint32_t get_debug_mcs() {
        this->debug.state_mcs.read = true;
        return this->debug.mcs;
    }

    bool has_changed_debug_mcs() {
        meta_state_t * meta = &this->debug.state_mcs;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_mcs() {
        return this->debug.state_mcs.read;
    }

    void set_debug_sec(uint32_t arg) {
        if (this->debug.sec != arg) {
            this->debug.state_sec.modified = true;
            this->debug.state_sec.read = false;
        }
        this->debug.sec = arg;
    }

    const uint32_t get_debug_sec() {
        this->debug.state_sec.read = true;
        return this->debug.sec;
    }

    bool has_changed_debug_sec() {
        meta_state_t * meta = &this->debug.state_sec;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_sec() {
        return this->debug.state_sec.read;
    }

    void set_debug_rdp(uint32_t arg) {
        if (this->debug.rdp != arg) {
            this->debug.state_rdp.modified = true;
            this->debug.state_rdp.read = false;
        }
        this->debug.rdp = arg;
    }

    const uint32_t get_debug_rdp() {
        this->debug.state_rdp.read = true;
        return this->debug.rdp;
    }

    bool has_changed_debug_rdp() {
        meta_state_t * meta = &this->debug.state_rdp;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_rdp() {
        return this->debug.state_rdp.read;
    }

    void set_debug_primary_orders(uint32_t arg) {
        if (this->debug.primary_orders != arg) {
            this->debug.state_primary_orders.modified = true;
            this->debug.state_primary_orders.read = false;
        }
        this->debug.primary_orders = arg;
    }

    const uint32_t get_debug_primary_orders() {
        this->debug.state_primary_orders.read = true;
        return this->debug.primary_orders;
    }

    bool has_changed_debug_primary_orders() {
        meta_state_t * meta = &this->debug.state_primary_orders;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_primary_orders() {
        return this->debug.state_primary_orders.read;
    }

    void set_debug_secondary_orders(uint32_t arg) {
        if (this->debug.secondary_orders != arg) {
            this->debug.state_secondary_orders.modified = true;
            this->debug.state_secondary_orders.read = false;
        }
        this->debug.secondary_orders = arg;
    }

    const uint32_t get_debug_secondary_orders() {
        this->debug.state_secondary_orders.read = true;
        return this->debug.secondary_orders;
    }

    bool has_changed_debug_secondary_orders() {
        meta_state_t * meta = &this->debug.state_secondary_orders;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_secondary_orders() {
        return this->debug.state_secondary_orders.read;
    }

    void set_debug_bitmap(uint32_t arg) {
        if (this->debug.bitmap != arg) {
            this->debug.state_bitmap.modified = true;
            this->debug.state_bitmap.read = false;
        }
        this->debug.bitmap = arg;
    }

    const uint32_t get_debug_bitmap() {
        this->debug.state_bitmap.read = true;
        return this->debug.bitmap;
    }

    bool has_changed_debug_bitmap() {
        meta_state_t * meta = &this->debug.state_bitmap;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_bitmap() {
        return this->debug.state_bitmap.read;
    }

    void set_debug_capture(uint32_t arg) {
        if (this->debug.capture != arg) {
            this->debug.state_capture.modified = true;
            this->debug.state_capture.read = false;
        }
        this->debug.capture = arg;
    }

    const uint32_t get_debug_capture() {
        this->debug.state_capture.read = true;
        return this->debug.capture;
    }

    bool has_changed_debug_capture() {
        meta_state_t * meta = &this->debug.state_capture;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_capture() {
        return this->debug.state_capture.read;
    }

    void set_debug_auth(uint32_t arg) {
        if (this->debug.auth != arg) {
            this->debug.state_auth.modified = true;
            this->debug.state_auth.read = false;
        }
        this->debug.auth = arg;
    }

    const uint32_t get_debug_auth() {
        this->debug.state_auth.read = true;
        return this->debug.auth;
    }

    bool has_changed_debug_auth() {
        meta_state_t * meta = &this->debug.state_auth;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_auth() {
        return this->debug.state_auth.read;
    }

    void set_debug_session(uint32_t arg) {
        if (this->debug.session != arg) {
            this->debug.state_session.modified = true;
            this->debug.state_session.read = false;
        }
        this->debug.session = arg;
    }

    const uint32_t get_debug_session() {
        this->debug.state_session.read = true;
        return this->debug.session;
    }

    bool has_changed_debug_session() {
        meta_state_t * meta = &this->debug.state_session;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_session() {
        return this->debug.state_session.read;
    }

    void set_debug_front(uint32_t arg) {
        if (this->debug.front != arg) {
            this->debug.state_front.modified = true;
            this->debug.state_front.read = false;
        }
        this->debug.front = arg;
    }

    const uint32_t get_debug_front() {
        this->debug.state_front.read = true;
        return this->debug.front;
    }

    bool has_changed_debug_front() {
        meta_state_t * meta = &this->debug.state_front;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_front() {
        return this->debug.state_front.read;
    }

    void set_debug_mod_rdp(uint32_t arg) {
        if (this->debug.mod_rdp != arg) {
            this->debug.state_mod_rdp.modified = true;
            this->debug.state_mod_rdp.read = false;
        }
        this->debug.mod_rdp = arg;
    }

    const uint32_t get_debug_mod_rdp() {
        this->debug.state_mod_rdp.read = true;
        return this->debug.mod_rdp;
    }

    bool has_changed_debug_mod_rdp() {
        meta_state_t * meta = &this->debug.state_mod_rdp;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_mod_rdp() {
        return this->debug.state_mod_rdp.read;
    }

    void set_debug_mod_vnc(uint32_t arg) {
        if (this->debug.mod_vnc != arg) {
            this->debug.state_mod_vnc.modified = true;
            this->debug.state_mod_vnc.read = false;
        }
        this->debug.mod_vnc = arg;
    }

    const uint32_t get_debug_mod_vnc() {
        this->debug.state_mod_vnc.read = true;
        return this->debug.mod_vnc;
    }

    bool has_changed_debug_mod_vnc() {
        meta_state_t * meta = &this->debug.state_mod_vnc;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_mod_vnc() {
        return this->debug.state_mod_vnc.read;
    }

    void set_debug_mod_int(uint32_t arg) {
        if (this->debug.mod_int != arg) {
            this->debug.state_mod_int.modified = true;
            this->debug.state_mod_int.read = false;
        }
        this->debug.mod_int = arg;
    }

    const uint32_t get_debug_mod_int() {
        this->debug.state_mod_int.read = true;
        return this->debug.mod_int;
    }

    bool has_changed_debug_mod_int() {
        meta_state_t * meta = &this->debug.state_mod_int;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_mod_int() {
        return this->debug.state_mod_int.read;
    }

    void set_debug_mod_xup(uint32_t arg) {
        if (this->debug.mod_xup != arg) {
            this->debug.state_mod_xup.modified = true;
            this->debug.state_mod_xup.read = false;
        }
        this->debug.mod_xup = arg;
    }

    const uint32_t get_debug_mod_xup() {
        this->debug.state_mod_xup.read = true;
        return this->debug.mod_xup;
    }

    bool has_changed_debug_mod_xup() {
        meta_state_t * meta = &this->debug.state_mod_xup;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_mod_xup() {
        return this->debug.state_mod_xup.read;
    }

    void set_debug_widget(uint32_t arg) {
        if (this->debug.widget != arg) {
            this->debug.state_widget.modified = true;
            this->debug.state_widget.read = false;
        }
        this->debug.widget = arg;
    }

    const uint32_t get_debug_widget() {
        this->debug.state_widget.read = true;
        return this->debug.widget;
    }

    bool has_changed_debug_widget() {
        meta_state_t * meta = &this->debug.state_widget;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_widget() {
        return this->debug.state_widget.read;
    }

    void set_debug_input(uint32_t arg) {
        if (this->debug.input != arg) {
            this->debug.state_input.modified = true;
            this->debug.state_input.read = false;
        }
        this->debug.input = arg;
    }

    const uint32_t get_debug_input() {
        this->debug.state_input.read = true;
        return this->debug.input;
    }

    bool has_changed_debug_input() {
        meta_state_t * meta = &this->debug.state_input;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_input() {
        return this->debug.state_input.read;
    }

    void set_debug_log_type(int arg) {
        if (this->debug.log_type != arg) {
            this->debug.state_log_type.modified = true;
            this->debug.state_log_type.read = false;
        }
        this->debug.log_type = arg;
    }

    const int get_debug_log_type() {
        this->debug.state_log_type.read = true;
        return this->debug.log_type;
    }

    bool has_changed_debug_log_type() {
        meta_state_t * meta = &this->debug.state_log_type;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_log_type() {
        return this->debug.state_log_type.read;
    }

    void set_debug_log_file_path(const char * arg) {
        if (strcmp(this->debug.log_file_path, arg)) {
            this->debug.state_log_file_path.modified = true;
            this->debug.state_log_file_path.read = false;
        }
        memcpy(this->debug.log_file_path, arg, sizeof(arg));
    }

    const char * get_debug_log_file_path() {
        this->debug.state_log_file_path.read = true;
        return this->debug.log_file_path;
    }

    bool has_changed_debug_log_file_path() {
        meta_state_t * meta = &this->debug.state_log_file_path;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_debug_log_file_path() {
        return this->debug.state_log_file_path.read;
    }
    /*------------------------------
    // TRANSLATION SECTION
    -------------------------------*/
    void set_translation_button_ok(redemption::string arg) {
        if (strcmp(this->translation.button_ok.c_str(), arg.c_str())) {
            this->translation.state_button_ok.modified = true;
            this->translation.state_button_ok.read = false;
        }
        this->translation.button_ok.copy_str(arg);
    }

    const redemption::string get_translation_button_ok() {
        this->translation.state_button_ok.read = true;
        return this->translation.button_ok;
    }

    bool has_changed_translation_button_ok() {
        meta_state_t * meta = &this->translation.state_button_ok;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_button_ok() {
        return this->translation.state_button_ok.read;
    }

    void set_translation_button_cancel(redemption::string arg) {
        if (strcmp(this->translation.button_cancel.c_str(), arg.c_str())) {
            this->translation.state_button_cancel.modified = true;
            this->translation.state_button_cancel.read = false;
        }
        this->translation.button_cancel.copy_str(arg);
    }

    const redemption::string get_translation_button_cancel() {
        this->translation.state_button_cancel.read = true;
        return this->translation.button_cancel;
    }

    bool has_changed_translation_button_cancel() {
        meta_state_t * meta = &this->translation.state_button_cancel;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_button_cancel() {
        return this->translation.state_button_cancel.read;
    }

    void set_translation_button_help(redemption::string arg) {
        if (strcmp(this->translation.button_help.c_str(), arg.c_str())) {
            this->translation.state_button_help.modified = true;
            this->translation.state_button_help.read = false;
        }
        this->translation.button_help.copy_str(arg);
    }

    const redemption::string get_translation_button_help() {
        this->translation.state_button_help.read = true;
        return this->translation.button_help;
    }

    bool has_changed_translation_button_help() {
        meta_state_t * meta = &this->translation.state_button_help;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_button_help() {
        return this->translation.state_button_help.read;
    }

    void set_translation_button_close(redemption::string arg) {
        if (strcmp(this->translation.button_close.c_str(), arg.c_str())) {
            this->translation.state_button_close.modified = true;
            this->translation.state_button_close.read = false;
        }
        this->translation.button_close.copy_str(arg);
    }

    const redemption::string get_translation_button_close() {
        this->translation.state_button_close.read = true;
        return this->translation.button_close;
    }

    bool has_changed_translation_button_close() {
        meta_state_t * meta = &this->translation.state_button_close;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_button_close() {
        return this->translation.state_button_close.read;
    }

    void set_translation_button_refused(redemption::string arg) {
        if (strcmp(this->translation.button_refused.c_str(), arg.c_str())) {
            this->translation.state_button_refused.modified = true;
            this->translation.state_button_refused.read = false;
        }
        this->translation.button_refused.copy_str(arg);
    }

    const redemption::string get_translation_button_refused() {
        this->translation.state_button_refused.read = true;
        return this->translation.button_refused;
    }

    bool has_changed_translation_button_refused() {
        meta_state_t * meta = &this->translation.state_button_refused;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_button_refused() {
        return this->translation.state_button_refused.read;
    }

    void set_translation_login(redemption::string arg) {
        if (strcmp(this->translation.login.c_str(), arg.c_str())) {
            this->translation.state_login.modified = true;
            this->translation.state_login.read = false;
        }
        this->translation.login.copy_str(arg);
    }

    const redemption::string get_translation_login() {
        this->translation.state_login.read = true;
        return this->translation.login;
    }

    bool has_changed_translation_login() {
        meta_state_t * meta = &this->translation.state_login;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_login() {
        return this->translation.state_login.read;
    }

    void set_translation_username(redemption::string arg) {
        if (strcmp(this->translation.username.c_str(), arg.c_str())) {
            this->translation.state_username.modified = true;
            this->translation.state_username.read = false;
        }
        this->translation.username.copy_str(arg);
    }

    const redemption::string get_translation_username() {
        this->translation.state_username.read = true;
        return this->translation.username;
    }

    bool has_changed_translation_username() {
        meta_state_t * meta = &this->translation.state_username;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_username() {
        return this->translation.state_username.read;
    }

    void set_translation_password(redemption::string arg) {
        if (strcmp(this->translation.password.c_str(), arg.c_str())) {
            this->translation.state_password.modified = true;
            this->translation.state_password.read = false;
        }
        this->translation.password.copy_str(arg);
    }

    const redemption::string get_translation_password() {
        this->translation.state_password.read = true;
        return this->translation.password;
    }

    bool has_changed_translation_password() {
        meta_state_t * meta = &this->translation.state_password;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_password() {
        return this->translation.state_password.read;
    }

    void set_translation_target(redemption::string arg) {
        if (strcmp(this->translation.target.c_str(), arg.c_str())) {
            this->translation.state_target.modified = true;
            this->translation.state_target.read = false;
        }
        this->translation.target.copy_str(arg);
    }

    const redemption::string get_translation_target() {
        this->translation.state_target.read = true;
        return this->translation.target;
    }

    bool has_changed_translation_target() {
        meta_state_t * meta = &this->translation.state_target;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_target() {
        return this->translation.state_target.read;
    }

    void set_translation_diagnostic(redemption::string arg) {
        if (strcmp(this->translation.diagnostic.c_str(), arg.c_str())) {
            this->translation.state_diagnostic.modified = true;
            this->translation.state_diagnostic.read = false;
        }
        this->translation.diagnostic.copy_str(arg);
    }

    const redemption::string get_translation_diagnostic() {
        this->translation.state_diagnostic.read = true;
        return this->translation.diagnostic;
    }

    bool has_changed_translation_diagnostic() {
        meta_state_t * meta = &this->translation.state_diagnostic;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_diagnostic() {
        return this->translation.state_diagnostic.read;
    }

    void set_translation_connection_closed(redemption::string arg) {
        if (strcmp(this->translation.connection_closed.c_str(), arg.c_str())) {
            this->translation.state_connection_closed.modified = true;
            this->translation.state_connection_closed.read = false;
        }
        this->translation.connection_closed.copy_str(arg);
    }

    const redemption::string get_translation_connection_closed() {
        this->translation.state_connection_closed.read = true;
        return this->translation.connection_closed;
    }

    bool has_changed_translation_connection_closed() {
        meta_state_t * meta = &this->translation.state_connection_closed;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_connection_closed() {
        return this->translation.state_connection_closed.read;
    }

    void set_translation_help_message(redemption::string arg) {
        if (strcmp(this->translation.help_message.c_str(), arg.c_str())) {
            this->translation.state_help_message.modified = true;
            this->translation.state_help_message.read = false;
        }
        this->translation.help_message.copy_str(arg);
    }

    const redemption::string get_translation_help_message() {
        this->translation.state_help_message.read = true;
        return this->translation.help_message;
    }

    bool has_changed_translation_help_message() {
        meta_state_t * meta = &this->translation.state_help_message;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_translation_help_message() {
        return this->translation.state_help_message.read;
    }

    /*-----------------------------------
    // CONTEXT SECTION
    ------------------------------------*/

    void set_context_selector_focus(unsigned arg) {
        if (this->context.selector_focus != arg) {
            this->context.state_selector_focus.modified = true;
            this->context.state_selector_focus.read = false;
        }
        this->context.selector_focus = arg;
    }

    const unsigned get_context_selector_focus() {
        this->context.state_selector_focus.read = true;
        return this->context.selector_focus;
    }

    bool has_changed_context_selector_focus() {
        meta_state_t * meta = &this->context.state_selector_focus;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_focus() {
        return this->context.state_selector_focus.read;
    }

    void set_context_movie(const char * arg) {
        if (strcmp(this->context.movie, arg)) {
            this->context.state_movie.modified = true;
            this->context.state_movie.read = false;
        }
        memcpy(this->context.movie, arg, sizeof(arg));
    }

    const char * get_context_movie() {
        this->context.state_movie.read = true;
        return this->context.movie;
    }

    bool has_changed_context_movie() {
        meta_state_t * meta = &this->context.state_movie;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_movie() {
        return this->context.state_movie.read;
    }

    void set_context_opt_bitrate(unsigned arg) {
        if (this->context.opt_bitrate != arg) {
            this->context.state_opt_bitrate.modified = true;
            this->context.state_opt_bitrate.read = false;
        }
        this->context.opt_bitrate = arg;
    }

    const unsigned get_context_opt_bitrate() {
        this->context.state_opt_bitrate.read = true;
        return this->context.opt_bitrate;
    }

    bool has_changed_context_opt_bitrate() {
        meta_state_t * meta = &this->context.state_opt_bitrate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_bitrate() {
        return this->context.state_opt_bitrate.read;
    }

    void set_context_opt_framerate(unsigned arg) {
        if (this->context.opt_framerate != arg) {
            this->context.state_opt_framerate.modified = true;
            this->context.state_opt_framerate.read = false;
        }
        this->context.opt_framerate = arg;
    }

    const unsigned get_context_opt_framerate() {
        this->context.state_opt_framerate.read = true;
        return this->context.opt_framerate;
    }

    bool has_changed_context_opt_framerate() {
        meta_state_t * meta = &this->context.state_opt_framerate;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_framerate() {
        return this->context.state_opt_framerate.read;
    }

    void set_context_opt_qscale(unsigned arg) {
        if (this->context.opt_qscale != arg) {
            this->context.state_opt_qscale.modified = true;
            this->context.state_opt_qscale.read = false;
        }
        this->context.opt_qscale = arg;
    }

    const unsigned get_context_opt_qscale() {
        this->context.state_opt_qscale.read = true;
        return this->context.opt_qscale;
    }

    bool has_changed_context_opt_qscale() {
        meta_state_t * meta = &this->context.state_opt_qscale;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_qscale() {
        return this->context.state_opt_qscale.read;
    }

    

    void set_context_opt_bpp(unsigned arg) {
        if (this->context.opt_bpp != arg) {
            this->context.state_opt_bpp.modified = true;
            this->context.state_opt_bpp.read = false;
        }
        this->context.opt_bpp = arg;
    }

    const unsigned get_context_opt_bpp() {
        this->context.state_opt_bpp.read = true;
        return this->context.opt_bpp;
    }

    bool has_changed_context_opt_bpp() {
        meta_state_t * meta = &this->context.state_opt_bpp;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_bpp() {
        return this->context.state_opt_bpp.read;
    }

    void set_context_opt_height(unsigned arg) {
        if (this->context.opt_height != arg) {
            this->context.state_opt_height.modified = true;
            this->context.state_opt_height.read = false;
        }
        this->context.opt_height = arg;
    }

    const unsigned get_context_opt_height() {
        this->context.state_opt_height.read = true;
        return this->context.opt_height;
    }

    bool has_changed_context_opt_height() {
        meta_state_t * meta = &this->context.state_opt_height;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_height() {
        return this->context.state_opt_height.read;
    }

    void set_context_opt_width(unsigned arg) {
        if (this->context.opt_width != arg) {
            this->context.state_opt_width.modified = true;
            this->context.state_opt_width.read = false;
        }
        this->context.opt_width = arg;
    }

    const unsigned get_context_opt_width() {
        this->context.state_opt_width.read = true;
        return this->context.opt_width;
    }

    bool has_changed_context_opt_width() {
        meta_state_t * meta = &this->context.state_opt_width;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_opt_width() {
        return this->context.state_opt_width.read;
    }

    void set_context_auth_error_message(redemption::string arg) {
        if (strcmp(this->context.auth_error_message.c_str(), arg.c_str())) {
            this->context.state_auth_error_message.modified = true;
            this->context.state_auth_error_message.read = false;
        }
        this->context.auth_error_message.copy_str(arg);
    }

    const redemption::string get_context_auth_error_message() {
        this->context.state_auth_error_message.read = true;
        return this->context.auth_error_message;
    }

    bool has_changed_context_auth_error_message() {
        meta_state_t * meta = &this->context.state_auth_error_message;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_auth_error_message() {
        return this->context.state_auth_error_message.read;
    }

    void set_context_selector(bool arg) {
        if (this->context.selector != arg) {
            this->context.state_selector.modified = true;
            this->context.state_selector.read = false;
        }
        this->context.selector = arg;
    }

    const bool get_context_selector() {
        this->context.state_selector.read = true;
        return this->context.selector;
    }

    bool has_changed_context_selector() {
        meta_state_t * meta = &this->context.state_selector;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector() {
        return this->context.state_selector.read;
    }

    void set_context_selector_current_page(unsigned arg) {
        if (this->context.selector_current_page != arg) {
            this->context.state_selector_current_page.modified = true;
            this->context.state_selector_current_page.read = false;
        }
        this->context.selector_current_page = arg;
    }

    const unsigned get_context_selector_current_page() {
        this->context.state_selector_current_page.read = true;
        return this->context.selector_current_page;
    }

    bool has_changed_context_selector_current_page() {
        meta_state_t * meta = &this->context.state_selector_current_page;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_current_page() {
        return this->context.state_selector_current_page.read;
    }

    void set_context_selector_device_filter(redemption::string arg) {
        if (strcmp(this->context.selector_device_filter.c_str(), arg.c_str())) {
            this->context.state_selector_device_filter.modified = true;
            this->context.state_selector_device_filter.read = false;
        }
        this->context.selector_device_filter.copy_str(arg);
    }

    const redemption::string get_context_selector_device_filter() {
        this->context.state_selector_device_filter.read = true;
        return this->context.selector_device_filter;
    }

    bool has_changed_context_selector_device_filter() {
        meta_state_t * meta = &this->context.state_selector_device_filter;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_device_filter() {
        return this->context.state_selector_device_filter.read;
    }

    void set_context_selector_group_filter(redemption::string arg) {
        if (strcmp(this->context.selector_group_filter.c_str(), arg.c_str())) {
            this->context.state_selector_group_filter.modified = true;
            this->context.state_selector_group_filter.read = false;
        }
        this->context.selector_group_filter.copy_str(arg);
    }

    const redemption::string get_context_selector_group_filter() {
        this->context.state_selector_group_filter.read = true;
        return this->context.selector_group_filter;
    }

    bool has_changed_context_selector_group_filter() {
        meta_state_t * meta = &this->context.state_selector_group_filter;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_group_filter() {
        return this->context.state_selector_group_filter.read;
    }

    void set_context_selector_lines_per_page(unsigned arg) {
        if (this->context.selector_lines_per_page != arg) {
            this->context.state_selector_lines_per_page.modified = true;
            this->context.state_selector_lines_per_page.read = false;
        }
        this->context.selector_lines_per_page = arg;
    }

    const unsigned get_context_selector_lines_per_page() {
        this->context.state_selector_lines_per_page.read = true;
        return this->context.selector_lines_per_page;
    }

    bool has_changed_context_selector_lines_per_page() {
        meta_state_t * meta = &this->context.state_selector_lines_per_page;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_lines_per_page() {
        return this->context.state_selector_lines_per_page.read;
    }

    void set_context_selector_number_of_pages(unsigned arg) {
        if (this->context.selector_number_of_pages != arg) {
            this->context.state_selector_number_of_pages.modified = true;
            this->context.state_selector_number_of_pages.read = false;
        }
        this->context.selector_number_of_pages = arg;
    }

    const unsigned get_context_selector_number_of_pages() {
        this->context.state_selector_number_of_pages.read = true;
        return this->context.selector_number_of_pages;
    }

    bool has_changed_context_selector_number_of_pages() {
        meta_state_t * meta = &this->context.state_selector_number_of_pages;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_selector_number_of_pages() {
        return this->context.state_selector_number_of_pages.read;
    }

    void set_context_target_password(redemption::string arg) {
        if (strcmp(this->context.target_password.c_str(), arg.c_str())) {
            this->context.state_target_password.modified = true;
            this->context.state_target_password.read = false;
        }
        this->context.target_password.copy_str(arg);
    }

    const redemption::string get_context_target_password() {
        this->context.state_target_password.read = true;
        return this->context.target_password;
    }

    bool has_changed_context_target_password() {
        meta_state_t * meta = &this->context.state_target_password;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_target_password() {
        return this->context.state_target_password.read;
    }

    void set_context_target_port(unsigned arg) {
        if (this->context.target_port != arg) {
            this->context.state_target_port.modified = true;
            this->context.state_target_port.read = false;
        }
        this->context.target_port = arg;
    }

    const unsigned get_context_target_port() {
        this->context.state_target_port.read = true;
        return this->context.target_port;
    }

    bool has_changed_context_target_port() {
        meta_state_t * meta = &this->context.state_target_port;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_target_port() {
        return this->context.state_target_port.read;
    }

    void set_context_target_protocol(redemption::string arg) {
        if (strcmp(this->context.target_protocol.c_str(), arg.c_str())) {
            this->context.state_target_protocol.modified = true;
            this->context.state_target_protocol.read = false;
        }
        this->context.target_protocol.copy_str(arg);
    }

    const redemption::string get_context_target_protocol() {
        this->context.state_target_protocol.read = true;
        return this->context.target_protocol;
    }

    bool has_changed_context_target_protocol() {
        meta_state_t * meta = &this->context.state_target_protocol;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_target_protocol() {
        return this->context.state_target_protocol.read;
    }

    void set_context_password(redemption::string arg) {
        if (strcmp(this->context.password.c_str(), arg.c_str())) {
            this->context.state_password.modified = true;
            this->context.state_password.read = false;
        }
        this->context.password.copy_str(arg);
    }

    const redemption::string get_context_password() {
        this->context.state_password.read = true;
        return this->context.password;
    }

    bool has_changed_context_password() {
        meta_state_t * meta = &this->context.state_password;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_password() {
        return this->context.state_password.read;
    }

    void set_context_authchannel_answer(redemption::string arg) {
        if (strcmp(this->context.authchannel_answer.c_str(), arg.c_str())) {
            this->context.state_authchannel_answer.modified = true;
            this->context.state_authchannel_answer.read = false;
        }
        this->context.authchannel_answer.copy_str(arg);
    }

    const redemption::string get_context_authchannel_answer() {
        this->context.state_authchannel_answer.read = true;
        return this->context.authchannel_answer;
    }

    bool has_changed_context_authchannel_answer() {
        meta_state_t * meta = &this->context.state_authchannel_answer;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_authchannel_answer() {
        return this->context.state_authchannel_answer.read;
    }

    void set_context_authchannel_result(redemption::string arg) {
        if (strcmp(this->context.authchannel_result.c_str(), arg.c_str())) {
            this->context.state_authchannel_result.modified = true;
            this->context.state_authchannel_result.read = false;
        }
        this->context.authchannel_result.copy_str(arg);
    }

    const redemption::string get_context_authchannel_result() {
        this->context.state_authchannel_result.read = true;
        return this->context.authchannel_result;
    }

    bool has_changed_context_authchannel_result() {
        meta_state_t * meta = &this->context.state_authchannel_result;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_authchannel_result() {
        return this->context.state_authchannel_result.read;
    }

    void set_context_authchannel_target(redemption::string arg) {
        if (strcmp(this->context.authchannel_target.c_str(), arg.c_str())) {
            this->context.state_authchannel_target.modified = true;
            this->context.state_authchannel_target.read = false;
        }
        this->context.authchannel_target.copy_str(arg);
    }

    const redemption::string get_context_authchannel_target() {
        this->context.state_authchannel_target.read = true;
        return this->context.authchannel_target;
    }

    bool has_changed_context_authchannel_target() {
        meta_state_t * meta = &this->context.state_authchannel_target;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_authchannel_target() {
        return this->context.state_authchannel_target.read;
    }

    void set_context_message(redemption::string arg) {
        if (strcmp(this->context.message.c_str(), arg.c_str())) {
            this->context.state_message.modified = true;
            this->context.state_message.read = false;
        }
        this->context.message.copy_str(arg);
    }

    const redemption::string get_context_message() {
        this->context.state_message.read = true;
        return this->context.message;
    }

    bool has_changed_context_message() {
        meta_state_t * meta = &this->context.state_message;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_message() {
        return this->context.state_message.read;
    }

    void set_context_accept_message(redemption::string arg) {
        if (strcmp(this->context.accept_message.c_str(), arg.c_str())) {
            this->context.state_accept_message.modified = true;
            this->context.state_accept_message.read = false;
        }
        this->context.accept_message.copy_str(arg);
    }

    const redemption::string get_context_accept_message() {
        this->context.state_accept_message.read = true;
        return this->context.accept_message;
    }

    bool has_changed_context_accept_message() {
        meta_state_t * meta = &this->context.state_accept_message;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_accept_message() {
        return this->context.state_accept_message.read;
    }

    void set_context_display_message(redemption::string arg) {
        if (strcmp(this->context.display_message.c_str(), arg.c_str())) {
            this->context.state_display_message.modified = true;
            this->context.state_display_message.read = false;
        }
        this->context.display_message.copy_str(arg);
    }

    const redemption::string get_context_display_message() {
        this->context.state_display_message.read = true;
        return this->context.display_message;
    }

    bool has_changed_context_display_message() {
        meta_state_t * meta = &this->context.state_display_message;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_display_message() {
        return this->context.state_display_message.read;
    }

    void set_context_rejected(redemption::string arg) {
        if (strcmp(this->context.rejected.c_str(), arg.c_str())) {
            this->context.state_rejected.modified = true;
            this->context.state_rejected.read = false;
        }
        this->context.rejected.copy_str(arg);
    }

    const redemption::string get_context_rejected() {
        this->context.state_rejected.read = true;
        return this->context.rejected;
    }

    bool has_changed_context_rejected() {
        meta_state_t * meta = &this->context.state_rejected;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_rejected() {
        return this->context.state_rejected.read;
    }

    void set_context_authenticated(bool arg) {
        if (this->context.authenticated != arg) {
            this->context.state_authenticated.modified = true;
            this->context.state_authenticated.read = false;
        }
        this->context.authenticated = arg;
    }

    const bool get_context_authenticated() {
        this->context.state_authenticated.read = true;
        return this->context.authenticated;
    }

    bool has_changed_context_authenticated() {
        meta_state_t * meta = &this->context.state_authenticated;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_authenticated() {
        return this->context.state_authenticated.read;
    }

    void set_context_keepalive(bool arg) {
        if (this->context.keepalive != arg) {
            this->context.state_keepalive.modified = true;
            this->context.state_keepalive.read = false;
        }
        this->context.keepalive = arg;
    }

    const bool get_context_keepalive() {
        this->context.state_keepalive.read = true;
        return this->context.keepalive;
    }

    bool has_changed_context_keepalive() {
        meta_state_t * meta = &this->context.state_keepalive;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_keepalive() {
        return this->context.state_keepalive.read;
    }

    void set_context_proxy_type(redemption::string arg) {
        if (strcmp(this->context.proxy_type.c_str(), arg.c_str())) {
            this->context.state_proxy_type.modified = true;
            this->context.state_proxy_type.read = false;
        }
        this->context.proxy_type.copy_str(arg);
    }

    const redemption::string get_context_proxy_type() {
        this->context.state_proxy_type.read = true;
        return this->context.proxy_type;
    }

    bool has_changed_context_proxy_type() {
        meta_state_t * meta = &this->context.state_proxy_type;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_proxy_type() {
        return this->context.state_proxy_type.read;
    }

    void set_context_trace_seal(redemption::string arg) {
        if (strcmp(this->context.trace_seal.c_str(), arg.c_str())) {
            this->context.state_trace_seal.modified = true;
            this->context.state_trace_seal.read = false;
        }
        this->context.trace_seal.copy_str(arg);
    }

    const redemption::string get_context_trace_seal() {
        this->context.state_trace_seal.read = true;
        return this->context.trace_seal;
    }

    bool has_changed_context_trace_seal() {
        meta_state_t * meta = &this->context.state_trace_seal;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_trace_seal() {
        return this->context.state_trace_seal.read;
    }

    void set_context_session_id(redemption::string arg) {
        if (strcmp(this->context.session_id.c_str(), arg.c_str())) {
            this->context.state_session_id.modified = true;
            this->context.state_session_id.read = false;
        }
        this->context.session_id.copy_str(arg);
    }

    const redemption::string get_context_session_id() {
        this->context.state_session_id.read = true;
        return this->context.session_id;
    }

    bool has_changed_context_session_id() {
        meta_state_t * meta = &this->context.state_session_id;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_session_id() {
        return this->context.state_session_id.read;
    }

    void set_context_end_date_cnx(unsigned arg) {
        if (this->context.end_date_cnx != arg) {
            this->context.state_end_date_cnx.modified = true;
            this->context.state_end_date_cnx.read = false;
        }
        this->context.end_date_cnx = arg;
    }

    const unsigned get_context_end_date_cnx() {
        this->context.state_end_date_cnx.read = true;
        return this->context.end_date_cnx;
    }

    bool has_changed_context_end_date_cnx() {
        meta_state_t * meta = &this->context.state_end_date_cnx;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_end_date_cnx() {
        return this->context.state_end_date_cnx.read;
    }

    void set_context_end_time(redemption::string arg) {
        if (strcmp(this->context.end_time.c_str(), arg.c_str())) {
            this->context.state_end_time.modified = true;
            this->context.state_end_time.read = false;
        }
        this->context.end_time.copy_str(arg);
    }

    const redemption::string get_context_end_time() {
        this->context.state_end_time.read = true;
        return this->context.end_time;
    }

    bool has_changed_context_end_time() {
        meta_state_t * meta = &this->context.state_end_time;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_end_time() {
        return this->context.state_end_time.read;
    }

    void set_context_mode_console(redemption::string arg) {
        if (strcmp(this->context.mode_console.c_str(), arg.c_str())) {
            this->context.state_mode_console.modified = true;
            this->context.state_mode_console.read = false;
        }
        this->context.mode_console.copy_str(arg);
    }

    const redemption::string get_context_mode_console() {
        this->context.state_mode_console.read = true;
        return this->context.mode_console;
    }

    bool has_changed_context_mode_console() {
        meta_state_t * meta = &this->context.state_mode_console;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_mode_console() {
        return this->context.state_mode_console.read;
    }

    void set_context_timezone(signed arg) {
        if (this->context.timezone != arg) {
            this->context.state_timezone.modified = true;
            this->context.state_timezone.read = false;
        }
        this->context.timezone = arg;
    }

    const signed get_context_timezone() {
        this->context.state_timezone.read = true;
        return this->context.timezone;
    }

    bool has_changed_context_timezone() {
        meta_state_t * meta = &this->context.state_timezone;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_timezone() {
        return this->context.state_timezone.read;
    }

    void set_context_real_target_device(redemption::string arg) {
        if (strcmp(this->context.real_target_device.c_str(), arg.c_str())) {
            this->context.state_real_target_device.modified = true;
            this->context.state_real_target_device.read = false;
        }
        this->context.real_target_device.copy_str(arg);
    }

    const redemption::string get_context_real_target_device() {
        this->context.state_real_target_device.read = true;
        return this->context.real_target_device;
    }

    bool has_changed_context_real_target_device() {
        meta_state_t * meta = &this->context.state_real_target_device;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_real_target_device() {
        return this->context.state_real_target_device.read;
    }

    void set_context_authentication_challenge(redemption::string arg) {
        if (strcmp(this->context.authentication_challenge.c_str(), arg.c_str())) {
            this->context.state_authentication_challenge.modified = true;
            this->context.state_authentication_challenge.read = false;
        }
        this->context.authentication_challenge.copy_str(arg);
    }

    const redemption::string get_context_authentication_challenge() {
        this->context.state_authentication_challenge.read = true;
        return this->context.authentication_challenge;
    }

    bool has_changed_context_authentication_challenge() {
        meta_state_t * meta = &this->context.state_authentication_challenge;
        bool res = meta->modified;
        if (!meta->ask)
            meta->modified = false;
        return res;
    }

    bool been_read_context_authentication_challenge() {
        return this->context.state_authentication_challenge.read;
    }

};

#endif

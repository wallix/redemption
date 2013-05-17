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
   Author(s): Christophe Grosjean, Raphael Zhou

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
    else if (0 == strcasecmp("high", str)) { res = 2; }
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
    _AUTHID_TRANS_BUTTON_OK,
    _AUTHID_TRANS_BUTTON_CANCEL,
    _AUTHID_TRANS_BUTTON_HELP,
    _AUTHID_TRANS_BUTTON_CLOSE,
    _AUTHID_TRANS_BUTTON_REFUSED,
    _AUTHID_TRANS_LOGIN,
    _AUTHID_TRANS_USERNAME,
    _AUTHID_TRANS_PASSWORD,
    _AUTHID_TRANS_TARGET,
    _AUTHID_TRANS_DIAGNOSTIC,
    _AUTHID_TRANS_CONNECTION_CLOSED,
    _AUTHID_TRANS_HELP_MESSAGE,

    // Options
    _AUTHID_OPT_CLIPBOARD,         // clipboard
    _AUTHID_OPT_DEVICEREDIRECTION, // device_redirection
    _AUTHID_OPT_FILE_ENCRYPTION,   // file encryption

    // Video capture
    _AUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
    _AUTHID_OPT_MOVIE,       // is_rec
    _AUTHID_OPT_MOVIE_PATH,  // rec_path
    _AUTHID_VIDEO_QUALITY,

    // Alternate shell
    _AUTHID_ALTERNATE_SHELL,
    _AUTHID_SHELL_WORKING_DIRECTORY,

    // Context
    _AUTHID_OPT_BITRATE,         // Bit rate for video encoding
    _AUTHID_OPT_FRAMERATE,       // Frame rate for video encoding
    _AUTHID_OPT_QSCALE,          // QScale parameter for vdeo encoding

    _AUTHID_OPT_BPP,            // bits per planes (number of colors)
    _AUTHID_OPT_HEIGHT,         // client height
    _AUTHID_OPT_WIDTH,          // client width

    _AUTHID_AUTH_ERROR_MESSAGE,

    _AUTHID_SELECTOR,
    _AUTHID_SELECTOR_CURRENT_PAGE,      // current page
    _AUTHID_SELECTOR_DEVICE_FILTER,     // device filter text
    _AUTHID_SELECTOR_GROUP_FILTER,      // group filter text
    _AUTHID_SELECTOR_LINES_PER_PAGE,    // number of lines per page
    _AUTHID_SELECTOR_NUMBER_OF_PAGES,   // number of pages

    _AUTHID_TARGET_DEVICE,      // target_device
    _AUTHID_TARGET_PASSWORD,    // target_password
    _AUTHID_TARGET_PORT,        // target_port
    _AUTHID_TARGET_PROTOCOL,    // proto_dest
    _AUTHID_TARGET_USER,        // target_login

    _AUTHID_AUTH_USER,          // login
    _AUTHID_HOST,               // ip_client
    _AUTHID_PASSWORD,           // password

    _AUTHID_AUTHCHANNEL_ANSWER, // WabLauncher target answer
    _AUTHID_AUTHCHANNEL_RESULT, // WabLauncher session result
    _AUTHID_AUTHCHANNEL_TARGET, // WabLauncher target request

    _AUTHID_MESSAGE,    // warning_message

    _AUTHID_ACCEPT_MESSAGE,     // display a dialog to valid a message
    _AUTHID_DISPLAY_MESSAGE,    // display a dialog box with a message

    _AUTHID_AUTHENTICATED,
    _AUTHID_REJECTED,       // rejected

    _AUTHID_KEEPALIVE,
    _AUTHID_PROXY_TYPE,

    _AUTHID_TRACE_SEAL,     // after closing trace file trace is sealed using a signature hash

    _AUTHID_SESSION_ID,     // session_id

    _AUTHID_END_DATE_CNX,   // timeclose
    _AUTHID_END_TIME,       // end time as text

    _AUTHID_MODE_CONSOLE,
    _AUTHID_TIMEZONE,

    MAX_AUTHID
} authid_t;

// Translation text
#define _STRAUTHID_TRANS_BUTTON_OK         "trans_ok"
#define _STRAUTHID_TRANS_BUTTON_CANCEL     "trans_cancel"
#define _STRAUTHID_TRANS_BUTTON_HELP       "trans_help"
#define _STRAUTHID_TRANS_BUTTON_CLOSE      "trans_close"
#define _STRAUTHID_TRANS_BUTTON_REFUSED    "trans_refused"
#define _STRAUTHID_TRANS_LOGIN             "trans_login"
#define _STRAUTHID_TRANS_USERNAME          "trans_username"
#define _STRAUTHID_TRANS_PASSWORD          "trans_password"
#define _STRAUTHID_TRANS_TARGET            "trans_target"
#define _STRAUTHID_TRANS_DIAGNOSTIC        "trans_diagnostic"
#define _STRAUTHID_TRANS_CONNECTION_CLOSED "trans_connection_closed"
#define _STRAUTHID_TRANS_HELP_MESSAGE      "trans_help_message"
// Options
#define _STRAUTHID_OPT_CLIPBOARD           "clipboard"
#define _STRAUTHID_OPT_DEVICEREDIRECTION   "device_redirection"
#define _STRAUTHID_OPT_FILE_ENCRYPTION     "file_encryption"
// Video capture
#define _STRAUTHID_OPT_CODEC_ID             "codec_id"
#define _STRAUTHID_OPT_MOVIE                "is_rec"
#define _STRAUTHID_OPT_MOVIE_PATH           "rec_path"
#define _STRAUTHID_VIDEO_QUALITY            "video_quality"
// Alternate shell
#define _STRAUTHID_ALTERNATE_SHELL          "alternate_shell"
#define _STRAUTHID_SHELL_WORKING_DIRECTORY  "shell_working_directory"
// Context
#define _STRAUTHID_OPT_BITRATE              "bitrate"
#define _STRAUTHID_OPT_FRAMERATE            "framerate"
#define _STRAUTHID_OPT_QSCALE               "qscale"
#define _STRAUTHID_OPT_BPP                  "bpp"
#define _STRAUTHID_OPT_HEIGHT               "height"
#define _STRAUTHID_OPT_WIDTH                "width"

#define _STRAUTHID_AUTH_ERROR_MESSAGE       "error_message"

#define _STRAUTHID_SELECTOR                  "selector"
#define _STRAUTHID_SELECTOR_CURRENT_PAGE     "selector_current_page"
#define _STRAUTHID_SELECTOR_DEVICE_FILTER    "selector_device_filter"
#define _STRAUTHID_SELECTOR_GROUP_FILTER     "selector_group_filter"
#define _STRAUTHID_SELECTOR_LINES_PER_PAGE   "selector_lines_per_page"
#define __STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"

#define _STRAUTHID_TARGET_DEVICE            "target_device"
#define _STRAUTHID_TARGET_PASSWORD          "target_password"
#define _STRAUTHID_TARGET_PORT              "target_port"
#define _STRAUTHID_TARGET_PROTOCOL          "proto_dest"
#define _STRAUTHID_TARGET_USER              "target_login"

#define _STRAUTHID_AUTH_USER                "login"
#define _STRAUTHID_HOST                     "ip_client"
#define _STRAUTHID_PASSWORD                 "password"

TODO("This is not a translation but auth_channel answer, change key name in sesman")
#define _STRAUTHID_AUTHCHANNEL_ANSWER       "trans_auth_channel"
#define _STRAUTHID_AUTHCHANNEL_RESULT       "auth_channel_result"
#define _STRAUTHID_AUTHCHANNEL_TARGET       "auth_channel_target"

#define _STRAUTHID_MESSAGE                  "message"

#define _STRAUTHID_ACCEPT_MESSAGE           "accept_message"
#define _STRAUTHID_DISPLAY_MESSAGE          "display_message"

#define _STRAUTHID_AUTHENTICATED            "authenticated"
#define _STRAUTHID_REJECTED                 "rejected"

#define _STRAUTHID_KEEPALIVE                "keepalive"
#define _STRAUTHID_PROXY_TYPE               "proxy_type"

#define _STRAUTHID_TRACE_SEAL               "trace_seal"

#define _STRAUTHID_SESSION_ID               "session_id"

#define _STRAUTHID_END_DATE_CNX             "timeclose"
#define _STRAUTHID_END_TIME                 "end_time"

#define _STRAUTHID_MODE_CONSOLE             "mode_console"
#define _STRAUTHID_TIMEZONE                 "timezone"

static inline authid_t authid_from_string(const char * strauthid) {
    static const std::string authstr[MAX_AUTHID - 1] = {
        // Translation text
        _STRAUTHID_TRANS_BUTTON_OK,
        _STRAUTHID_TRANS_BUTTON_CANCEL,
        _STRAUTHID_TRANS_BUTTON_HELP,
        _STRAUTHID_TRANS_BUTTON_CLOSE,
        _STRAUTHID_TRANS_BUTTON_REFUSED,
        _STRAUTHID_TRANS_LOGIN,
        _STRAUTHID_TRANS_USERNAME,
        _STRAUTHID_TRANS_PASSWORD,
        _STRAUTHID_TRANS_TARGET,
        _STRAUTHID_TRANS_DIAGNOSTIC,
        _STRAUTHID_TRANS_CONNECTION_CLOSED,
        _STRAUTHID_TRANS_HELP_MESSAGE,

        // Options
        _STRAUTHID_OPT_CLIPBOARD,         // clipboard
        _STRAUTHID_OPT_DEVICEREDIRECTION, // device_redirection
        _STRAUTHID_OPT_FILE_ENCRYPTION,   // file encryption

        // Video capture
        _STRAUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
        _STRAUTHID_OPT_MOVIE,       // is_rec
        _STRAUTHID_OPT_MOVIE_PATH,  // rec_path
        _STRAUTHID_VIDEO_QUALITY,

        // Alternate shell
        _STRAUTHID_ALTERNATE_SHELL,
        _STRAUTHID_SHELL_WORKING_DIRECTORY,

        // Context
        _STRAUTHID_OPT_BITRATE,         // Bit rate for video encoding
        _STRAUTHID_OPT_FRAMERATE,       // Frame rate for video encoding
        _STRAUTHID_OPT_QSCALE,          // QScale parameter for vdeo encoding

        _STRAUTHID_OPT_BPP,            // bits per planes (number of colors)
        _STRAUTHID_OPT_HEIGHT,         // client height
        _STRAUTHID_OPT_WIDTH,          // client width

        _STRAUTHID_AUTH_ERROR_MESSAGE,

        _STRAUTHID_SELECTOR,
        _STRAUTHID_SELECTOR_CURRENT_PAGE,      // current page
        _STRAUTHID_SELECTOR_DEVICE_FILTER,     // device filter text
        _STRAUTHID_SELECTOR_GROUP_FILTER,      // group filter text
        _STRAUTHID_SELECTOR_LINES_PER_PAGE,    // number of lines per page
        __STRAUTHID_SELECTOR_NUMBER_OF_PAGES,  // number of pages

        _STRAUTHID_TARGET_DEVICE,      // target_device
        _STRAUTHID_TARGET_PASSWORD,    // target_password
        _STRAUTHID_TARGET_PORT,        // target_port
        _STRAUTHID_TARGET_PROTOCOL,    // proto_dest
        _STRAUTHID_TARGET_USER,        // target_login

        _STRAUTHID_AUTH_USER,          // login
        _STRAUTHID_HOST,               // ip_client
        _STRAUTHID_PASSWORD,           // password

        _STRAUTHID_AUTHCHANNEL_ANSWER, // WabLauncher target answer
        _STRAUTHID_AUTHCHANNEL_RESULT, // WabLauncher session result
        _STRAUTHID_AUTHCHANNEL_TARGET, // WabLauncher target request

        _STRAUTHID_MESSAGE,    // warning_message

        _STRAUTHID_ACCEPT_MESSAGE,      // display a dialog to valid a message
        _STRAUTHID_DISPLAY_MESSAGE,     // display a dialog box with a message

        _STRAUTHID_AUTHENTICATED,
        _STRAUTHID_REJECTED,        // rejected

        _STRAUTHID_KEEPALIVE,
        _STRAUTHID_PROXY_TYPE,

        _STRAUTHID_TRACE_SEAL,      // after closing trace file trace is sealed using a signature hash

        _STRAUTHID_SESSION_ID,      // session_id

        _STRAUTHID_END_DATE_CNX,    // timeclose
        _STRAUTHID_END_TIME,        // end time as text

        _STRAUTHID_MODE_CONSOLE,
        _STRAUTHID_TIMEZONE,
    };

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

struct Inifile {
    struct Inifile_globals {
        bool capture_png;
        bool capture_wrm;
        bool capture_flv;
        bool capture_ocr;
        bool capture_chunk;
        bool movie;
        char movie_path[512];
        char codec_id[512];
        char video_quality[512];
        char auth_user[512];
        char host[512];
        char target_device[512];
        char target_user[512];
        char auth_channel[512];

        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        int encryptionLevel;     // 0=low, 1=medium, 2=high
        char authip[255];
        int authport;
        bool nomouse;
        bool notimestamp;
        bool autovalidate;       // dialog autovalidation for test
        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files

        unsigned capture_flags;  // 1 PNG capture, 2 WRM
        unsigned png_interval;   // time between 2 png captures (in 1/10 seconds)
        unsigned frame_interval; // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval; // time between 2 wrm movies (in seconds)
        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval; 
        unsigned ocr_interval;
        unsigned capture_groupid;

        unsigned png_limit;    // number of png captures to keep

        int l_bitrate;         // bitrate for low quality
        int l_framerate;       // framerate for low quality
        int l_height;          // height for low quality
        int l_width;           // width for low quality
        int l_qscale;          // qscale (parameter given to ffmpeg) for low quality

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

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;

        char replay_path[1024];
        bool internal_domain;

        bool enable_file_encryption;
        bool enable_tls;
        char listen_address[256];
        bool enable_ip_transparent;
        char certificate_password[256];

        char png_path[1024];
        char wrm_path[1024];

        char alternate_shell[1024];
        char shell_working_directory[1024];

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

            int  log_type;
            char log_file_path[1024]; // log file location
        } debug;

        // section "client"
        struct {
            bool ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default false

            uint32_t performance_flags_default;
            uint32_t performance_flags_force_present;
            uint32_t performance_flags_force_not_present;

            bool tls_fallback_legacy;

            bool clipboard;
            bool device_redirection;
        } client;

        // section "translation"
        struct {
            redemption::string button_ok;
            redemption::string button_cancel;
            redemption::string button_help;
            redemption::string button_close;
            redemption::string button_refused;
            redemption::string login;
            redemption::string username;
            redemption::string password;
            redemption::string target;
            redemption::string diagnostic;
            redemption::string connection_closed;
            redemption::string help_message;
        } translation;

        // section "context"
        struct {
            unsigned           opt_bitrate;
            unsigned           opt_framerate;
            unsigned           opt_qscale;

            bool               ask_opt_bpp;
            bool               ask_opt_height;
            bool               ask_opt_width;

            unsigned           opt_bpp;
            unsigned           opt_height;
            unsigned           opt_width;

            redemption::string auth_error_message;

            bool               ask_selector;
            bool               ask_selector_current_page;
            bool               ask_selector_device_filter;
            bool               ask_selector_group_filter;
            bool               ask_selector_lines_per_page;

            bool               selector;
            unsigned           selector_current_page;
            redemption::string selector_device_filter;
            redemption::string selector_group_filter;
            unsigned           selector_lines_per_page;
            unsigned           selector_number_of_pages;

            bool               ask_target_device;
            bool               ask_target_password;
            bool               ask_target_port;
            bool               ask_target_protocol;
            bool               ask_target_user;

            redemption::string target_password;
            unsigned           target_port;
            redemption::string target_protocol;

            bool               ask_auth_user;

            bool               ask_host;
            bool               ask_password;

            redemption::string password;

            bool               ask_authchannel_target;
            bool               ask_authchannel_result;

            redemption::string authchannel_answer;
            redemption::string authchannel_result;
            redemption::string authchannel_target;

            bool               ask_accept_message;
            bool               ask_display_message;

            redemption::string message;
            redemption::string accept_message;
            redemption::string display_message;

            redemption::string rejected;

            bool               authenticated;

            bool               ask_keepalive;
            bool               ask_proxy_type;

            bool               keepalive;
            redemption::string proxy_type;

            bool               ask_trace_seal;

            redemption::string trace_seal;

            redemption::string session_id;

            unsigned           end_date_cnx;
            redemption::string end_time;

            redemption::string mode_console;
            signed             timezone;
        } context;
    } globals;

    struct IniAccounts account;

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
        this->globals.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->globals.capture_wrm   = true;
        this->globals.capture_png   = true;
        this->globals.capture_flv   = false;
        this->globals.capture_ocr   = false;
        this->globals.capture_chunk = false;
        this->globals.movie            = false;
        this->globals.movie_path[0]    = 0;
        this->globals.auth_user[0]     = 0;
        this->globals.host[0]          = 0;
        this->globals.target_device[0] = 0;
        this->globals.target_user[0]   = 0;

        this->globals.bitmap_cache = true;
        this->globals.bitmap_compression = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_cstr("low");
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.autovalidate = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
        strcpy(this->globals.codec_id, "flv");
        TODO("this could be some kind of enumeration")
        strcpy(this->globals.video_quality, "medium");

        this->globals.png_interval = 3000;
        this->globals.ocr_interval = 100; // 1 every second
        this->globals.frame_interval = 40;
        this->globals.break_interval = 600;
        this->globals.flv_break_interval = 600000000l;
        this->globals.flv_frame_interval = 1000000L; 
        this->globals.capture_groupid = 33; 

        this->globals.png_limit = 3;
        this->globals.l_bitrate   = 20000;
        this->globals.l_framerate = 1;
        this->globals.l_height    = 480;
        this->globals.l_width     = 640;
        this->globals.l_qscale    = 25;
        this->globals.m_bitrate   = 40000;
        this->globals.m_framerate = 1;
        this->globals.m_height    = 768;
        this->globals.m_width     = 1024;
        this->globals.m_qscale    = 15;
        this->globals.h_bitrate   = 200000;
        this->globals.h_framerate = 5;
        this->globals.h_height    = 1024;
        this->globals.h_width     = 1280;
        this->globals.h_qscale    = 15;
        this->globals.max_tick    = 30;
        this->globals.keepalive_grace_delay = 30;
        strcpy(this->globals.replay_path, "/tmp/");
        this->globals.internal_domain = false;
        this->globals.enable_file_encryption = false;
        this->globals.enable_tls             = true;
        strcpy(this->globals.listen_address, "0.0.0.0");
        this->globals.enable_ip_transparent  = false;
        strcpy(this->globals.certificate_password, "inquisition");

        strcpy(this->globals.png_path, PNG_PATH);
        strcpy(this->globals.wrm_path, WRM_PATH);

        this->globals.alternate_shell[0]         = 0;
        this->globals.shell_working_directory[0] = 0;

        memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);

        strcpy(this->account.accountname, "");
        strcpy(this->account.username,    "");
        strcpy(this->account.password,    "");

        // Section "debug".
        this->globals.debug.x224              = 0;
        this->globals.debug.mcs               = 0;
        this->globals.debug.sec               = 0;
        this->globals.debug.rdp               = 0;
        this->globals.debug.primary_orders    = 0;
        this->globals.debug.secondary_orders  = 0;
        this->globals.debug.bitmap            = 0;
        this->globals.debug.capture           = 0;
        this->globals.debug.auth              = 0;
        this->globals.debug.session           = 0;
        this->globals.debug.front             = 0;
        this->globals.debug.mod_rdp           = 0;
        this->globals.debug.mod_vnc           = 0;
        this->globals.debug.mod_int           = 0;
        this->globals.debug.mod_xup           = 0;
        this->globals.debug.widget            = 0;
        this->globals.debug.input             = 0;

        this->globals.debug.log_type         = 2; // syslog by default
        this->globals.debug.log_file_path[0] = 0;

        // Section "client".
        this->globals.client.ignore_logon_password               = false;
//      this->globals.client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->globals.client.performance_flags_default           = 0;
        this->globals.client.performance_flags_force_present     = 0;
        this->globals.client.performance_flags_force_not_present = 0;
        this->globals.client.tls_fallback_legacy                 = false;
        this->globals.client.clipboard                           = true;
        this->globals.client.device_redirection                  = true;

        // Section "translation"
        this->globals.translation.button_ok         = "OK";
        this->globals.translation.button_cancel     = "Cancel";
        this->globals.translation.button_help       = "Help";
        this->globals.translation.button_close      = "Close";
        this->globals.translation.button_refused    = "Refused";
        this->globals.translation.login             = "login";
        this->globals.translation.username          = "username";
        this->globals.translation.password          = "password";
        this->globals.translation.target            = "target";
        this->globals.translation.diagnostic        = "diagnostic";
        this->globals.translation.connection_closed = "Connection closed";
        this->globals.translation.help_message      = "Help message";

        // section "context"
        this->globals.context.opt_bitrate                 = 40000;
        this->globals.context.opt_framerate               = 5;
        this->globals.context.opt_qscale                  = 15;

        this->globals.context.ask_opt_bpp                 = false;
        this->globals.context.ask_opt_height              = false;
        this->globals.context.ask_opt_width               = false;

        this->globals.context.opt_bpp                     = 24;
        this->globals.context.opt_height                  = 600;
        this->globals.context.opt_width                   = 800;

        this->globals.context.auth_error_message          = "";

        this->globals.context.ask_selector                = false;
        this->globals.context.ask_selector_current_page   = false;
        this->globals.context.ask_selector_device_filter  = false;
        this->globals.context.ask_selector_group_filter   = false;
        this->globals.context.ask_selector_lines_per_page = false;

        this->globals.context.selector                    = false;
        this->globals.context.selector_current_page       = 1;
        this->globals.context.selector_device_filter      = "";
        this->globals.context.selector_group_filter       = "";
        this->globals.context.selector_lines_per_page     = 20;
        this->globals.context.selector_number_of_pages    = 1;

        this->globals.context.ask_target_device           = false;
        this->globals.context.ask_target_password         = false;
        this->globals.context.ask_target_port             = false;
        this->globals.context.ask_target_protocol         = false;
        this->globals.context.ask_target_user             = false;

        this->globals.context.target_password             = "";
        this->globals.context.target_port                 = 3389;
        this->globals.context.target_protocol             = "RDP";

        this->globals.context.password                    = "";

        this->globals.context.ask_authchannel_target      = false;
        this->globals.context.ask_authchannel_result      = false;

        this->globals.context.authchannel_answer          = "";
        this->globals.context.authchannel_result          = "";
        this->globals.context.authchannel_target          = "";

        this->globals.context.ask_accept_message          = false;
        this->globals.context.ask_display_message         = false;

        this->globals.context.message                     = "";
        this->globals.context.accept_message              = "";
        this->globals.context.display_message             = "";

        this->globals.context.rejected                    = "Connection refused by authentifier.";

        this->globals.context.authenticated               = false;

        this->globals.context.ask_keepalive               = true;
        this->globals.context.ask_proxy_type              = false;

        this->globals.context.keepalive                   = false;
        this->globals.context.proxy_type                  = "RDP";

        this->globals.context.ask_trace_seal              = false;

        this->globals.context.trace_seal                  = "";

        this->globals.context.session_id                  = "";

        this->globals.context.end_date_cnx                = 0;
        this->globals.context.end_time                    = "";

        this->globals.context.mode_console                = "allow";
        this->globals.context.timezone                    = -3600;
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

                    this->setglobal(key, value, context);
                    break;
                }
            }
        }
    }

    void setglobal(const char * key, const char * value, const char * context)
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
        }
        else if (0 == strcmp(context, "client")){
            if (0 == strcmp(key, "ignore_logon_password")){
                this->globals.client.ignore_logon_password = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_default")){
                this->globals.client.performance_flags_default = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_present")){
                this->globals.client.performance_flags_force_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_not_present")){
                this->globals.client.performance_flags_force_not_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_fallback_legacy")){
                this->globals.client.tls_fallback_legacy = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard")){
                this->globals.client.clipboard = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "device_redirection")){
                this->globals.client.device_redirection = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "video")){ 
            if (0 == strcmp(key, "capture_flags")){
                this->globals.capture_flags   = ulong_from_cstr(value);
                this->globals.capture_png = 0 != (this->globals.capture_flags & 1);
                this->globals.capture_wrm = 0 != (this->globals.capture_flags & 2);
                this->globals.capture_flv = 0 != (this->globals.capture_flags & 4);
                this->globals.capture_ocr = 0 != (this->globals.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->globals.ocr_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->globals.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")){
                this->globals.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->globals.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->globals.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->globals.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strncpy(this->globals.replay_path, value, sizeof(this->globals.replay_path));
                this->globals.replay_path[sizeof(this->globals.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->globals.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->globals.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->globals.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->globals.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->globals.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->globals.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->globals.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->globals.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->globals.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->globals.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->globals.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->globals.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->globals.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->globals.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->globals.h_qscale    = ulong_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")){
            if (0 == strcmp(key, "x224")){
                this->globals.debug.x224              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mcs")){
                this->globals.debug.mcs               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "sec")){
                this->globals.debug.sec               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp")){
                this->globals.debug.rdp               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "primary_orders")){
                this->globals.debug.primary_orders    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "secondary_orders")){
                this->globals.debug.secondary_orders  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap")){
                this->globals.debug.bitmap            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture")){
                this->globals.debug.capture           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth")){
                this->globals.debug.auth              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session")){
                this->globals.debug.session           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "front")){
                this->globals.debug.front             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_rdp")){
                this->globals.debug.mod_rdp           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_vnc")){
                this->globals.debug.mod_vnc           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_int")){
                this->globals.debug.mod_int           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_xup")){
                this->globals.debug.mod_xup           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "widget")){
                this->globals.debug.widget            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "input")){
                this->globals.debug.input             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_type")){
                this->globals.debug.log_type = logtype_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_file_path")){
                strncpy(this->globals.debug.log_file_path, value, sizeof(this->globals.debug.log_file_path));
                this->globals.debug.log_file_path[sizeof(this->globals.debug.log_file_path) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "translation")){
            if (0 == strcmp(key, "button_ok")){
                this->globals.translation.button_ok = value;
            }
            else if (0 == strcmp(key, "button_cancel")){
                this->globals.translation.button_cancel     = value;
            }
            else if (0 == strcmp(key, "button_help")){
                this->globals.translation.button_help       = value;
            }
            else if (0 == strcmp(key, "button_close")){
                this->globals.translation.button_close      = value;
            }
            else if (0 == strcmp(key, "button_refused")){
                this->globals.translation.button_refused    = value;
            }
            else if (0 == strcmp(key, "login")){
                this->globals.translation.login             = value;
            }
            else if (0 == strcmp(key, "username")){
                this->globals.translation.username          = value;
            }
            else if (0 == strcmp(key, "password")){
                this->globals.translation.password          = value;
            }
            else if (0 == strcmp(key, "target")){
                this->globals.translation.target            = value;
            }
            else if (0 == strcmp(key, "diagnostic")){
                this->globals.translation.diagnostic        = value;
            }
            else if (0 == strcmp(key, "connection_closed")){
                this->globals.translation.connection_closed = value;
            }
            else if (0 == strcmp(key, "help_message")){
                this->globals.translation.help_message      = value;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }

/*
    const char * context_get_value(const char * key, char * buffer, size_t size) {
        const char * pszReturn = "";

        if (size) {
            *buffer = 0;

            if (!strcasecmp(key, "opt_bpp")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_bpp);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "opt_height")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_height);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "opt_width")) {
                snprintf(buffer, size, "%u", this->globals.context.opt_width);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector")) {
                strncpy(buffer, (this->globals.context.selector ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_current_page")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_current_page);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_device_filter")) {
                strncpy(buffer, this->globals.context.selector_device_filter, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_group_filter")) {
                strncpy(buffer, this->globals.context.selector_group_filter, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_lines_per_page")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_lines_per_page);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "selector_number_of_pages")) {
                snprintf(buffer, size, "%u", this->globals.context.selector_number_of_pages);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_device")) {
                strncpy(buffer, this->globals.target_device, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_password")) {
                strncpy(buffer, this->globals.context.target_password, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_port")) {
                snprintf(buffer, size, "%u", this->globals.context.target_port);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_protocol")) {
                strncpy(buffer, this->globals.context.target_protocol, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "target_user")) {
                strncpy(buffer, this->globals.target_user, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "auth_user")) {
                strncpy(buffer, this->globals.auth_user, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "host")) {
                strncpy(buffer, this->globals.host, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "password")) {
                strncpy(buffer, this->globals.context.password, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_answer")) {
                strncpy(buffer, this->globals.context.authchannel_answer, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_result")) {
                strncpy(buffer, this->globals.context.authchannel_result, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authchannel_target")) {
                strncpy(buffer, this->globals.context.authchannel_target, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "message")) {
                strncpy(buffer, this->globals.context.message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "accept_message")) {
                strncpy(buffer, this->globals.context.accept_message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "display_message")) {
                strncpy(buffer, this->globals.context.display_message, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "rejected")) {
                strncpy(buffer, this->globals.context.rejected, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "authenticated")) {
                strncpy(buffer, (this->globals.context.authenticated ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "keepalive")) {
                strncpy(buffer, (this->globals.context.keepalive ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "proxy_type")) {
                strncpy(buffer, this->globals.context.proxy_type, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "trace_seal")) {
                strncpy(buffer, this->globals.context.trace_seal, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "session_id")) {
                strncpy(buffer, this->globals.context.session_id, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "end_date_cnx")) {
                snprintf(buffer, size, "%u", this->globals.context.end_date_cnx);

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "end_time")) {
                strncpy(buffer, this->globals.context.end_time, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "mode_console")) {
                strncpy(buffer, this->globals.context.mode_console, size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            else if (!strcasecmp(key, "timezone")) {
                snprintf(buffer, size, "%d", this->globals.context.timezone);

                pszReturn = buffer;
            }
            else {
                LOG(LOG_WARNING, "context_get_value: unknown key=\"%s\"", key);
            }
        }

        //LOG(LOG_INFO, "parameter %s out section [%s]: value=%s", key, "context", pszReturn);
        return pszReturn;
    }
*/

    void context_set_value(const char * strauthid, const char * value) {
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
        case _AUTHID_TRANS_BUTTON_OK:
            this->globals.translation.button_ok         = value;
            break;
        case _AUTHID_TRANS_BUTTON_CANCEL:
            this->globals.translation.button_cancel     = value;
            break;
        case _AUTHID_TRANS_BUTTON_HELP:
            this->globals.translation.button_help       = value;
            break;
        case _AUTHID_TRANS_BUTTON_CLOSE:
            this->globals.translation.button_close      = value;
            break;
        case _AUTHID_TRANS_BUTTON_REFUSED:
            this->globals.translation.button_refused    = value;
            break;
        case _AUTHID_TRANS_LOGIN:
            this->globals.translation.login             = value;
            break;
        case _AUTHID_TRANS_USERNAME:
            this->globals.translation.username          = value;
            break;
        case _AUTHID_TRANS_PASSWORD:
            this->globals.translation.password          = value;
            break;
        case _AUTHID_TRANS_TARGET:
            this->globals.translation.target            = value;
            break;
        case _AUTHID_TRANS_DIAGNOSTIC:
            this->globals.translation.diagnostic        = value;
            break;
        case _AUTHID_TRANS_CONNECTION_CLOSED:
            this->globals.translation.connection_closed = value;
            break;
        case _AUTHID_TRANS_HELP_MESSAGE:
            this->globals.translation.help_message      = value;
            break;

        // Options
        case _AUTHID_OPT_CLIPBOARD:
            this->setglobal("clipboard",              value, "client" );
            break;
        case _AUTHID_OPT_DEVICEREDIRECTION:
            this->setglobal("device_redirection",     value, "client" );
            break;
        case _AUTHID_OPT_FILE_ENCRYPTION:
            this->setglobal("enable_file_encryption", value, "globals");
            break;

        // Video capture
        case _AUTHID_OPT_CODEC_ID:
            this->setglobal("codec_id",      value, "globals");
            break;
        case _AUTHID_OPT_MOVIE:
            this->setglobal("movie",         value, "globals");
            break;
        case _AUTHID_OPT_MOVIE_PATH:
            this->setglobal("movie_path",    value, "globals");
            break;
        case _AUTHID_VIDEO_QUALITY:
            this->setglobal("video_quality", value, "globals");
            break;

        // Alternate shell
        case _AUTHID_ALTERNATE_SHELL:
            this->setglobal("alternate_shell",         value, "globals");
            break;
        case _AUTHID_SHELL_WORKING_DIRECTORY:
            this->setglobal("shell_working_directory", value, "globals");
            break;

        // Context
        case _AUTHID_OPT_BITRATE:
            this->globals.context.opt_bitrate   = ulong_from_cstr(value);
            break;
        case _AUTHID_OPT_FRAMERATE:
            this->globals.context.opt_framerate = ulong_from_cstr(value);
            break;
        case _AUTHID_OPT_QSCALE:
            this->globals.context.opt_qscale    = ulong_from_cstr(value);
            break;

        case _AUTHID_OPT_BPP:
            this->globals.context.ask_opt_bpp    = false;
            this->globals.context.opt_bpp        = ulong_from_cstr(value);
        case _AUTHID_OPT_HEIGHT:
            this->globals.context.ask_opt_height = false;
            this->globals.context.opt_height     = ulong_from_cstr(value);
            break;
        case _AUTHID_OPT_WIDTH:
            this->globals.context.ask_opt_width  = false;
            this->globals.context.opt_width      = ulong_from_cstr(value);
            break;

        case _AUTHID_AUTH_ERROR_MESSAGE:
            this->globals.context.auth_error_message = value;
            break;

        case _AUTHID_SELECTOR:
            this->globals.context.ask_selector                = false;
            this->globals.context.selector                    = bool_from_cstr(value);
            break;
        case _AUTHID_SELECTOR_CURRENT_PAGE:
            this->globals.context.ask_selector_current_page   = false;
                this->globals.context.selector_current_page   = ulong_from_cstr(value);
            break;
        case _AUTHID_SELECTOR_DEVICE_FILTER:
            this->globals.context.ask_selector_device_filter  = false;
                this->globals.context.selector_device_filter  = value;
            break;
        case _AUTHID_SELECTOR_GROUP_FILTER:
            this->globals.context.ask_selector_group_filter   = false;
                this->globals.context.selector_group_filter   = value;
            break;
        case _AUTHID_SELECTOR_LINES_PER_PAGE:
            this->globals.context.ask_selector_lines_per_page = false;
                this->globals.context.selector_lines_per_page = ulong_from_cstr(value);
            break;
        case _AUTHID_SELECTOR_NUMBER_OF_PAGES:
            this->globals.context.selector_number_of_pages    = ulong_from_cstr(value);
            break;

        case _AUTHID_TARGET_DEVICE:
            this->globals.context.ask_target_device   = false;
            strncpy(this->globals.target_device, value, sizeof(this->globals.target_device));
            this->globals.target_device[sizeof(this->globals.target_device) - 1] = 0;
            break;
        case _AUTHID_TARGET_PASSWORD:
            this->globals.context.ask_target_password = false;
            this->globals.context.target_password     = value;
            break;
        case _AUTHID_TARGET_PORT:
                this->globals.context.target_port     = ulong_from_cstr(value);
            break;
        case _AUTHID_TARGET_PROTOCOL:
            this->globals.context.ask_target_protocol = false;
            this->globals.context.target_protocol     = value;
            break;
        case _AUTHID_TARGET_USER:
            this->globals.context.ask_target_user     = false;
            strncpy(this->globals.target_user,   value, sizeof(this->globals.target_user));
            this->globals.target_user[sizeof(this->globals.target_user) - 1]     = 0;
            break;

        case _AUTHID_AUTH_USER:
            this->globals.context.ask_auth_user = false;
            strncpy(this->globals.auth_user, value, sizeof(this->globals.auth_user));
            this->globals.auth_user[sizeof(this->globals.auth_user) - 1] = 0;
            break;
        case _AUTHID_HOST:
            this->globals.context.ask_host      = false;
            strncpy(this->globals.host,      value, sizeof(this->globals.host));
            this->globals.host[sizeof(this->globals.host) - 1]           = 0;
            break;

        case _AUTHID_PASSWORD:
            this->globals.context.ask_password = false;
            this->globals.context.password     = value;
            break;

        case _AUTHID_AUTHCHANNEL_ANSWER:
            this->globals.context.authchannel_answer     = value;
            break;
        case _AUTHID_AUTHCHANNEL_RESULT:
            this->globals.context.ask_authchannel_result = false;
            this->globals.context.authchannel_result     = value;
            break;
        case _AUTHID_AUTHCHANNEL_TARGET:
            this->globals.context.ask_authchannel_target = false;
            this->globals.context.authchannel_target     = value;
            break;

        case _AUTHID_MESSAGE:
            this->globals.context.message = value;
            break;

        case _AUTHID_ACCEPT_MESSAGE:
            this->globals.context.ask_accept_message  = false;
            this->globals.context.accept_message      = value;
            break;
        case _AUTHID_DISPLAY_MESSAGE:
            this->globals.context.ask_display_message = false;
            this->globals.context.display_message     = value;
            break;

        case _AUTHID_AUTHENTICATED:
            this->globals.context.authenticated  = bool_from_cstr(value);
            break;
        case _AUTHID_REJECTED:
            this->globals.context.rejected       = value;
            break;

        case _AUTHID_KEEPALIVE:
            this->globals.context.ask_keepalive  = false;
            this->globals.context.keepalive      = bool_from_cstr(value);
            break;
        case _AUTHID_PROXY_TYPE:
            this->globals.context.ask_proxy_type = false;
            this->globals.context.proxy_type     = value;
            break;

        case _AUTHID_TRACE_SEAL:
            this->globals.context.ask_trace_seal = false;
            this->globals.context.trace_seal     = value;
            break;

        case _AUTHID_SESSION_ID:
            this->globals.context.session_id = value;
            break;

        case _AUTHID_END_DATE_CNX:
            this->globals.context.end_date_cnx = ulong_from_cstr(value);
            break;
        case _AUTHID_END_TIME:
            this->globals.context.end_time     = value;
            break;

        case _AUTHID_MODE_CONSOLE:
            this->globals.context.mode_console = value;
            break;
        case _AUTHID_TIMEZONE:
            this->globals.context.timezone     = _long_from_cstr(value);
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
            break;
        }
    }

    const char * context_get_value(const char * strauthid, char * buffer, size_t size) {
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
        case _AUTHID_OPT_FILE_ENCRYPTION:
            if (size) {
                strncpy(buffer, (this->globals.enable_file_encryption ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case _AUTHID_OPT_BPP:
            if (  size
               && !this->globals.context.ask_opt_bpp) {
                snprintf(buffer, size, "%u", this->globals.context.opt_bpp);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_OPT_HEIGHT:
            if (  size
               && !this->globals.context.ask_opt_height) {
                snprintf(buffer, size, "%u", this->globals.context.opt_height);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_OPT_WIDTH:
            if (  size
               && !this->globals.context.ask_opt_width) {
                snprintf(buffer, size, "%u", this->globals.context.opt_width);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_SELECTOR:
            if (  size
               && !this->globals.context.ask_selector) {
                strncpy(buffer, (this->globals.context.selector ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case _AUTHID_SELECTOR_CURRENT_PAGE:
            if (  size
               && !this->globals.context.ask_selector_current_page) {
                snprintf(buffer, size, "%u", this->globals.context.selector_current_page);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_SELECTOR_DEVICE_FILTER:
            if (!this->globals.context.ask_selector_device_filter) {
                pszReturn = this->globals.context.selector_device_filter;
            }
            break;
        case _AUTHID_SELECTOR_GROUP_FILTER:
            if ( !this->globals.context.ask_selector_group_filter) {
                pszReturn = this->globals.context.selector_group_filter;
            }
            break;
        case _AUTHID_SELECTOR_LINES_PER_PAGE:
            if (  size
               && !this->globals.context.ask_selector_group_filter) {
                snprintf(buffer, size, "%u", this->globals.context.selector_lines_per_page);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_SELECTOR_NUMBER_OF_PAGES:
            if (size) {
                snprintf(buffer, size, "%u", this->globals.context.selector_number_of_pages);
                pszReturn = buffer;
            }
            break;
        case _AUTHID_TARGET_DEVICE:
            if (!this->globals.context.ask_target_device) {
                pszReturn = this->globals.target_device;
            }
            break;
        case _AUTHID_TARGET_PASSWORD:
            if (!this->globals.context.ask_target_password) {
                pszReturn = this->globals.context.target_password;
            }
            break;
        case _AUTHID_TARGET_PROTOCOL:
            if (!this->globals.context.ask_target_protocol) {
                pszReturn = this->globals.context.target_protocol;
            }
            break;
        case _AUTHID_TARGET_USER:
            if (!this->globals.context.ask_target_user) {
                pszReturn = this->globals.target_user;
            }
            break;
        case _AUTHID_AUTH_USER:
            if (!this->globals.context.ask_auth_user) {
                pszReturn = this->globals.auth_user;
            }
            break;
        case _AUTHID_HOST:
            if ( !this->globals.context.ask_host) {
                pszReturn = this->globals.host;
            }
            break;
        case _AUTHID_PASSWORD:
            if (!this->globals.context.ask_password) {
                pszReturn = this->globals.context.password;
            }
            break;
        case _AUTHID_AUTHCHANNEL_RESULT:
            if (!this->globals.context.ask_authchannel_result) {
                pszReturn = this->globals.context.authchannel_result;
            }
            break;
        case _AUTHID_AUTHCHANNEL_TARGET:
            if (!this->globals.context.ask_authchannel_target) {
                pszReturn = this->globals.context.authchannel_target;
            }
            break;
        case _AUTHID_ACCEPT_MESSAGE:
            if (!this->globals.context.ask_accept_message) {
                pszReturn = this->globals.context.accept_message;
            }
            break;
        case _AUTHID_DISPLAY_MESSAGE:
            if (!this->globals.context.ask_display_message) {
                pszReturn = this->globals.context.display_message;
            }
            break;
        case _AUTHID_KEEPALIVE:
            if (  size
               && !this->globals.context.ask_keepalive) {
                strncpy(buffer, (this->globals.context.keepalive ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            break;
        case _AUTHID_PROXY_TYPE:
            if (!this->globals.context.ask_proxy_type) {
                pszReturn = this->globals.context.proxy_type;
            }
            break;
        case _AUTHID_TRACE_SEAL:
            if (!this->globals.context.ask_trace_seal) {
                pszReturn = this->globals.context.trace_seal;
            }
            break;
        case _AUTHID_END_TIME:
            pszReturn = this->globals.context.end_time;
            break;
        default:
            LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
            break;
        }

        return pszReturn;
    }

    void context_ask(const char *strauthid) {
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
        case _AUTHID_OPT_BPP:
            this->globals.context.ask_opt_bpp                 = true; break;

        case _AUTHID_OPT_HEIGHT:
            this->globals.context.ask_opt_height              = true; break;

        case _AUTHID_OPT_WIDTH:
            this->globals.context.ask_opt_width               = true; break;

        case _AUTHID_SELECTOR:
            this->globals.context.ask_selector                = true; break;

        case _AUTHID_SELECTOR_CURRENT_PAGE:
            this->globals.context.ask_selector_current_page   = true; break;

        case _AUTHID_SELECTOR_DEVICE_FILTER:
            this->globals.context.ask_selector_device_filter  = true; break;

        case _AUTHID_SELECTOR_GROUP_FILTER:
            this->globals.context.ask_selector_group_filter   = true; break;

        case _AUTHID_SELECTOR_LINES_PER_PAGE:
            this->globals.context.ask_selector_lines_per_page = true; break;

        case _AUTHID_TARGET_DEVICE:
            this->globals.context.ask_target_device           = true; break;

        case _AUTHID_TARGET_PASSWORD:
            this->globals.context.ask_target_password         = true; break;

        case _AUTHID_TARGET_PROTOCOL:
            this->globals.context.ask_target_protocol         = true; break;

        case _AUTHID_TARGET_USER:
            this->globals.context.ask_target_user             = true; break;

        case _AUTHID_AUTH_USER:
            this->globals.context.ask_auth_user               = true; break;

        case _AUTHID_HOST:
            this->globals.context.ask_host                    = true; break;

        case _AUTHID_PASSWORD:
            this->globals.context.ask_password                = true; break;

        case _AUTHID_AUTHCHANNEL_RESULT:
            this->globals.context.ask_authchannel_result      = true; break;

        case _AUTHID_AUTHCHANNEL_TARGET:
            this->globals.context.ask_authchannel_target      = true; break;

        case _AUTHID_ACCEPT_MESSAGE:
            this->globals.context.ask_accept_message          = true; break;

        case _AUTHID_DISPLAY_MESSAGE:
            this->globals.context.ask_display_message         = true; break;

        case _AUTHID_KEEPALIVE:
            this->globals.context.ask_keepalive               = true; break;

        case _AUTHID_PROXY_TYPE:
            this->globals.context.ask_proxy_type              = true; break;

        case _AUTHID_TRACE_SEAL:
            this->globals.context.ask_trace_seal              = true; break;

        default:
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
            break;
        }
    }

    bool context_is_asked(const char *strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_is_asked(authid);
        }

        LOG(LOG_WARNING, "Inifile::context_is_asked(strid): unknown strauthid=\"%s\"", strauthid);

        return false;
    }

    bool context_is_asked(authid_t authid) {
        switch (authid) {
        case _AUTHID_OPT_BPP:
            return this->globals.context.ask_opt_bpp;

        case _AUTHID_OPT_HEIGHT:
            return this->globals.context.ask_opt_height;

        case _AUTHID_OPT_WIDTH:
            return this->globals.context.ask_opt_width;

        case _AUTHID_SELECTOR:
            return this->globals.context.ask_selector;

        case _AUTHID_SELECTOR_CURRENT_PAGE:
            return this->globals.context.ask_selector_current_page;

        case _AUTHID_SELECTOR_DEVICE_FILTER:
            return this->globals.context.ask_selector_device_filter;

        case _AUTHID_SELECTOR_GROUP_FILTER:
            return this->globals.context.ask_selector_group_filter;

        case _AUTHID_SELECTOR_LINES_PER_PAGE:
            return this->globals.context.ask_selector_lines_per_page;

        case _AUTHID_TARGET_DEVICE:
            return this->globals.context.ask_target_device;

        case _AUTHID_TARGET_PASSWORD:
            return this->globals.context.ask_target_password;

        case _AUTHID_TARGET_PROTOCOL:
            return this->globals.context.ask_target_protocol;

        case _AUTHID_TARGET_USER:
            return this->globals.context.ask_target_user;

        case _AUTHID_AUTH_USER:
            return this->globals.context.ask_auth_user;

        case _AUTHID_HOST:
            return this->globals.context.ask_host;

        case _AUTHID_PASSWORD:
            return this->globals.context.ask_password;

        case _AUTHID_AUTHCHANNEL_RESULT:
            return this->globals.context.ask_authchannel_result;

        case _AUTHID_AUTHCHANNEL_TARGET:
            return this->globals.context.ask_authchannel_target;

        case _AUTHID_ACCEPT_MESSAGE:
            return this->globals.context.ask_accept_message;

        case _AUTHID_DISPLAY_MESSAGE:
            return this->globals.context.ask_display_message;

        case _AUTHID_KEEPALIVE:
            return this->globals.context.ask_keepalive;

        case _AUTHID_PROXY_TYPE:
            return this->globals.context.ask_proxy_type;

        case _AUTHID_TRACE_SEAL:
            return this->globals.context.ask_trace_seal;

        default:
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
    }

    bool context_get_bool(authid_t authid) {
        switch (authid)
        {
        case _AUTHID_SELECTOR:
            if (!this->globals.context.ask_selector) {
                return this->globals.context.selector;
            }
            break;
        case _AUTHID_KEEPALIVE:
            if (!this->globals.context.ask_keepalive) {
                return this->globals.context.keepalive;
            }
            break;
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
};

#endif

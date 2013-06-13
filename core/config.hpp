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
#define STRAUTHID_TRANS_BUTTON_OK         "trans_ok"
#define STRAUTHID_TRANS_BUTTON_CANCEL     "trans_cancel"
#define STRAUTHID_TRANS_BUTTON_HELP       "trans_help"
#define STRAUTHID_TRANS_BUTTON_CLOSE      "trans_close"
#define STRAUTHID_TRANS_BUTTON_REFUSED    "trans_refused"
#define STRAUTHID_TRANS_LOGIN             "trans_login"
#define STRAUTHID_TRANS_USERNAME          "trans_username"
#define STRAUTHID_TRANS_PASSWORD          "trans_password"
#define STRAUTHID_TRANS_TARGET            "trans_target"
#define STRAUTHID_TRANS_DIAGNOSTIC        "trans_diagnostic"
#define STRAUTHID_TRANS_CONNECTION_CLOSED "trans_connection_closed"
#define STRAUTHID_TRANS_HELP_MESSAGE      "trans_help_message"
// Options
#define STRAUTHID_OPT_CLIPBOARD           "clipboard"
#define STRAUTHID_OPT_DEVICEREDIRECTION   "device_redirection"
#define STRAUTHID_OPT_FILE_ENCRYPTION     "file_encryption"
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

#define STRAUTHID_SELECTOR                  "selector"
#define STRAUTHID_SELECTOR_CURRENT_PAGE     "selector_current_page"
#define STRAUTHID_SELECTOR_DEVICE_FILTER    "selector_device_filter"
#define STRAUTHID_SELECTOR_GROUP_FILTER     "selector_group_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE   "selector_lines_per_page"
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

static inline authid_t authid_from_string(const char * strauthid) {
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
        STRAUTHID_OPT_CLIPBOARD,         // clipboard
        STRAUTHID_OPT_DEVICEREDIRECTION, // device_redirection
        STRAUTHID_OPT_FILE_ENCRYPTION,   // file encryption

        // Video capture
        STRAUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
        STRAUTHID_OPT_MOVIE,       // is_rec
        STRAUTHID_OPT_MOVIE_PATH,  // rec_path
        STRAUTHID_VIDEO_QUALITY,

        // Alternate shell
        STRAUTHID_ALTERNATE_SHELL,
        STRAUTHID_SHELL_WORKING_DIRECTORY,

        // Context
        STRAUTHID_OPT_BITRATE,         // Bit rate for video encoding
        STRAUTHID_OPT_FRAMERATE,       // Frame rate for video encoding
        STRAUTHID_OPT_QSCALE,          // QScale parameter for vdeo encoding

        STRAUTHID_OPT_BPP,            // bits per planes (number of colors)
        STRAUTHID_OPT_HEIGHT,         // client height
        STRAUTHID_OPT_WIDTH,          // client width

        STRAUTHID_AUTH_ERROR_MESSAGE,

        STRAUTHID_SELECTOR,
        STRAUTHID_SELECTOR_CURRENT_PAGE,      // current page
        STRAUTHID_SELECTOR_DEVICE_FILTER,     // device filter text
        STRAUTHID_SELECTOR_GROUP_FILTER,      // group filter text
        STRAUTHID_SELECTOR_LINES_PER_PAGE,    // number of lines per page
        STRAUTHID_SELECTOR_NUMBER_OF_PAGES,  // number of pages

        STRAUTHID_TARGET_DEVICE,      // target_device
        STRAUTHID_TARGET_PASSWORD,    // target_password
        STRAUTHID_TARGET_PORT,        // target_port
        STRAUTHID_TARGET_PROTOCOL,    // proto_dest
        STRAUTHID_TARGET_USER,        // target_login

        STRAUTHID_AUTH_USER,          // login
        STRAUTHID_HOST,               // ip_client
        STRAUTHID_TARGET,             // ip_target
        STRAUTHID_PASSWORD,           // password

        STRAUTHID_AUTHCHANNEL_ANSWER, // WabLauncher target answer
        STRAUTHID_AUTHCHANNEL_RESULT, // WabLauncher session result
        STRAUTHID_AUTHCHANNEL_TARGET, // WabLauncher target request

        STRAUTHID_MESSAGE,    // warning_message

        STRAUTHID_ACCEPT_MESSAGE,      // display a dialog to valid a message
        STRAUTHID_DISPLAY_MESSAGE,     // display a dialog box with a message

        STRAUTHID_AUTHENTICATED,
        STRAUTHID_REJECTED,        // rejected

        STRAUTHID_KEEPALIVE,
        STRAUTHID_PROXY_TYPE,

        STRAUTHID_TRACE_SEAL,      // after closing trace file trace is sealed using a signature hash

        STRAUTHID_SESSION_ID,      // session_id

        STRAUTHID_END_DATE_CNX,    // timeclose
        STRAUTHID_END_TIME,        // end time as text

        STRAUTHID_MODE_CONSOLE,
        STRAUTHID_TIMEZONE,

        STRAUTHID_REAL_TARGET_DEVICE,

        STRAUTHID_AUTHENTICATION_CHALLENGE,
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


        bool capture_chunk; // is it used ?


        char auth_user[512];
        char host[512];     // client_ip
        char target[512];   // target ip
        char target_device[512];
        char target_user[512];

        // BEGIN globals
        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        bool nomouse;
        bool notimestamp;
        int encryptionLevel;     // 0=low, 1=medium, 2=high
        char authip[255];
        int authport;
        bool autovalidate;       // dialog autovalidation for test

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;

        bool internal_domain;

        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files
        char auth_channel[512];
        bool enable_file_encryption;
        bool enable_tls;
        char listen_address[256];
        bool enable_ip_transparent;
        char certificate_password[256];

        char png_path[1024];
        char wrm_path[1024];

        char alternate_shell[1024];
        char shell_working_directory[1024];

        char codec_id[512];
        bool movie;
        char movie_path[512];
        char video_quality[512];
        bool enable_bitmap_update;
        // END globals


        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval;

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

        // Section "video"
        struct {
            unsigned capture_flags;  // 1 PNG capture, 2 WRM
            // video opt from capture_flags
            bool capture_png;
            bool capture_wrm;
            bool capture_flv;
            bool capture_ocr;

            unsigned ocr_interval;
            unsigned png_interval;   // time between 2 png captures (in 1/10 seconds)
            unsigned capture_groupid;
            unsigned frame_interval; // time between 2 frame captures (in 1/100 seconds)
            unsigned break_interval; // time between 2 wrm movies (in seconds)
            unsigned png_limit;    // number of png captures to keep
            char replay_path[1024];

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

        } video;


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
            unsigned           selector_focus;
            char               movie[1024];

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
            bool               ask_target;
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

            redemption::string real_target_device;

            redemption::string authentication_challenge;
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
        this->globals.client.ignore_logon_password               = false;
//      this->globals.client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->globals.client.performance_flags_default           = 0;
        this->globals.client.performance_flags_force_present     = 0;
        this->globals.client.performance_flags_force_not_present = 0;
        this->globals.client.tls_fallback_legacy                 = false;
        this->globals.client.clipboard                           = true;
        this->globals.client.device_redirection                  = true;
        // End Section "client"

        // Begin section video
        this->globals.video.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->globals.video.capture_wrm   = true;
        this->globals.video.capture_png   = true;
        this->globals.video.capture_flv   = false;
        this->globals.video.capture_ocr   = false;

        this->globals.video.ocr_interval = 100; // 1 every second
        this->globals.video.png_interval = 3000;
        this->globals.video.capture_groupid = 33;
        this->globals.video.frame_interval = 40;
        this->globals.video.break_interval = 600;
        this->globals.video.png_limit = 3;
        strcpy(this->globals.video.replay_path, "/tmp/");

        this->globals.video.l_bitrate   = 20000;
        this->globals.video.l_framerate = 1;
        this->globals.video.l_height    = 480;
        this->globals.video.l_width     = 640;
        this->globals.video.l_qscale    = 25;
        this->globals.video.m_bitrate   = 40000;
        this->globals.video.m_framerate = 1;
        this->globals.video.m_height    = 768;
        this->globals.video.m_width     = 1024;
        this->globals.video.m_qscale    = 15;
        this->globals.video.h_bitrate   = 200000;
        this->globals.video.h_framerate = 5;
        this->globals.video.h_height    = 1024;
        this->globals.video.h_width     = 1280;
        this->globals.video.h_qscale    = 15;
        // End section "video"


        // Begin Section "debug".
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
        // End Section "debug"

        // Begin Section "translation"
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
        // End Section "translation"

        // Begin section "context"
        this->globals.context.selector_focus              = 0;
        this->globals.context.movie[0]                    = 0;

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

        this->globals.context.ask_target_device           = true;
        this->globals.context.ask_target_password         = true;
        this->globals.context.ask_target_port             = true;
        this->globals.context.ask_target_protocol         = true;
        this->globals.context.ask_target_user             = true;

        this->globals.context.target_password             = "";
        this->globals.context.target_port                 = 3389;
        this->globals.context.target_protocol             = "RDP";

        // not sure about ask_host and ask_target initial values
        this->globals.context.ask_host                    = false;
        this->globals.context.ask_target                  = false;

        this->globals.context.ask_auth_user               = true;
        this->globals.context.ask_password                = true;


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

        this->globals.context.real_target_device          = "";

        this->globals.context.authentication_challenge    = "";
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
            else if (0 == strcmp(key, "enable_bitmap_update")){
                this->globals.enable_bitmap_update = bool_from_cstr(value);
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
                this->globals.video.capture_flags   = ulong_from_cstr(value);
                this->globals.video.capture_png = 0 != (this->globals.video.capture_flags & 1);
                this->globals.video.capture_wrm = 0 != (this->globals.video.capture_flags & 2);
                this->globals.video.capture_flv = 0 != (this->globals.video.capture_flags & 4);
                this->globals.video.capture_ocr = 0 != (this->globals.video.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->globals.video.ocr_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->globals.video.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")){
                this->globals.video.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->globals.video.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->globals.video.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->globals.video.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strncpy(this->globals.video.replay_path, value, sizeof(this->globals.video.replay_path));
                this->globals.video.replay_path[sizeof(this->globals.video.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->globals.video.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->globals.video.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->globals.video.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->globals.video.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->globals.video.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->globals.video.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->globals.video.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->globals.video.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->globals.video.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->globals.video.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->globals.video.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->globals.video.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->globals.video.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->globals.video.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->globals.video.h_qscale    = ulong_from_cstr(value);
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
        case AUTHID_TRANS_BUTTON_OK:
            this->globals.translation.button_ok         = value;
            break;
        case AUTHID_TRANS_BUTTON_CANCEL:
            this->globals.translation.button_cancel     = value;
            break;
        case AUTHID_TRANS_BUTTON_HELP:
            this->globals.translation.button_help       = value;
            break;
        case AUTHID_TRANS_BUTTON_CLOSE:
            this->globals.translation.button_close      = value;
            break;
        case AUTHID_TRANS_BUTTON_REFUSED:
            this->globals.translation.button_refused    = value;
            break;
        case AUTHID_TRANS_LOGIN:
            this->globals.translation.login             = value;
            break;
        case AUTHID_TRANS_USERNAME:
            this->globals.translation.username          = value;
            break;
        case AUTHID_TRANS_PASSWORD:
            this->globals.translation.password          = value;
            break;
        case AUTHID_TRANS_TARGET:
            this->globals.translation.target            = value;
            break;
        case AUTHID_TRANS_DIAGNOSTIC:
            this->globals.translation.diagnostic        = value;
            break;
        case AUTHID_TRANS_CONNECTION_CLOSED:
            this->globals.translation.connection_closed = value;
            break;
        case AUTHID_TRANS_HELP_MESSAGE:
            this->globals.translation.help_message      = value;
            break;

        // Options
        case AUTHID_OPT_CLIPBOARD:
            this->setglobal("clipboard",              value, "client" );
            break;
        case AUTHID_OPT_DEVICEREDIRECTION:
            this->setglobal("device_redirection",     value, "client" );
            break;
        case AUTHID_OPT_FILE_ENCRYPTION:
            this->setglobal("enable_file_encryption", value, "globals");
            break;

        // Video capture
        case AUTHID_OPT_CODEC_ID:
            this->setglobal("codec_id",      value, "globals");
            break;
        case AUTHID_OPT_MOVIE:
            this->setglobal("movie",         value, "globals");
            break;
        case AUTHID_OPT_MOVIE_PATH:
            this->setglobal("movie_path",    value, "globals");
            break;
        case AUTHID_VIDEO_QUALITY:
            this->setglobal("video_quality", value, "globals");
            break;

        // Alternate shell
        case AUTHID_ALTERNATE_SHELL:
            this->setglobal("alternate_shell",         value, "globals");
            break;
        case AUTHID_SHELL_WORKING_DIRECTORY:
            this->setglobal("shell_working_directory", value, "globals");
            break;

        // Context
        case AUTHID_OPT_BITRATE:
            this->globals.context.opt_bitrate   = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_FRAMERATE:
            this->globals.context.opt_framerate = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_QSCALE:
            this->globals.context.opt_qscale    = ulong_from_cstr(value);
            break;

        case AUTHID_OPT_BPP:
            this->globals.context.ask_opt_bpp    = false;
            this->globals.context.opt_bpp        = ulong_from_cstr(value);
        case AUTHID_OPT_HEIGHT:
            this->globals.context.ask_opt_height = false;
            this->globals.context.opt_height     = ulong_from_cstr(value);
            break;
        case AUTHID_OPT_WIDTH:
            this->globals.context.ask_opt_width  = false;
            this->globals.context.opt_width      = ulong_from_cstr(value);
            break;

        case AUTHID_AUTH_ERROR_MESSAGE:
            this->globals.context.auth_error_message = value;
            break;

        case AUTHID_SELECTOR:
            this->globals.context.ask_selector                = false;
            this->globals.context.selector                    = bool_from_cstr(value);
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->globals.context.ask_selector_current_page   = false;
                this->globals.context.selector_current_page   = ulong_from_cstr(value);
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->globals.context.ask_selector_device_filter  = false;
                this->globals.context.selector_device_filter  = value;
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            this->globals.context.ask_selector_group_filter   = false;
                this->globals.context.selector_group_filter   = value;
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->globals.context.ask_selector_lines_per_page = false;
                this->globals.context.selector_lines_per_page = ulong_from_cstr(value);
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            this->globals.context.selector_number_of_pages    = ulong_from_cstr(value);
            break;

        case AUTHID_TARGET_DEVICE:
            this->globals.context.ask_target_device   = false;
            strncpy(this->globals.target_device, value, sizeof(this->globals.target_device));
            this->globals.target_device[sizeof(this->globals.target_device) - 1] = 0;
            break;
        case AUTHID_TARGET_PASSWORD:
            this->globals.context.ask_target_password = false;
            this->globals.context.target_password     = value;
            break;
        case AUTHID_TARGET_PORT:
                this->globals.context.target_port     = ulong_from_cstr(value);
            break;
        case AUTHID_TARGET_PROTOCOL:
            this->globals.context.ask_target_protocol = false;
            this->globals.context.target_protocol     = value;
            break;
        case AUTHID_TARGET_USER:
            this->globals.context.ask_target_user     = false;
            strncpy(this->globals.target_user,   value, sizeof(this->globals.target_user));
            this->globals.target_user[sizeof(this->globals.target_user) - 1]     = 0;
            break;

        case AUTHID_AUTH_USER:
            this->globals.context.ask_auth_user = false;
            strncpy(this->globals.auth_user, value, sizeof(this->globals.auth_user));
            this->globals.auth_user[sizeof(this->globals.auth_user) - 1] = 0;
            break;
        case AUTHID_HOST:
            this->globals.context.ask_host      = false;
            strncpy(this->globals.host,      value, sizeof(this->globals.host));
            this->globals.host[sizeof(this->globals.host) - 1]           = 0;
            break;

        case AUTHID_TARGET:
            this->globals.context.ask_target      = false;
            strncpy(this->globals.target,      value, sizeof(this->globals.target));
            this->globals.host[sizeof(this->globals.target) - 1]           = 0;
            break;

        case AUTHID_PASSWORD:
            this->globals.context.ask_password = false;
            this->globals.context.password     = value;
            break;

        case AUTHID_AUTHCHANNEL_ANSWER:
            this->globals.context.authchannel_answer     = value;
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            this->globals.context.ask_authchannel_result = false;
            this->globals.context.authchannel_result     = value;
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            this->globals.context.ask_authchannel_target = false;
            this->globals.context.authchannel_target     = value;
            break;

        case AUTHID_MESSAGE:
            this->globals.context.message = value;
            break;

        case AUTHID_ACCEPT_MESSAGE:
            this->globals.context.ask_accept_message  = false;
            this->globals.context.accept_message      = value;
            break;
        case AUTHID_DISPLAY_MESSAGE:
            this->globals.context.ask_display_message = false;
            this->globals.context.display_message     = value;
            break;

        case AUTHID_AUTHENTICATED:
            this->globals.context.authenticated  = bool_from_cstr(value);
            break;
        case AUTHID_REJECTED:
            this->globals.context.rejected       = value;
            break;

        case AUTHID_KEEPALIVE:
            this->globals.context.ask_keepalive  = false;
            this->globals.context.keepalive      = bool_from_cstr(value);
            break;
        case AUTHID_PROXY_TYPE:
            this->globals.context.ask_proxy_type = false;
            this->globals.context.proxy_type     = value;
            break;

        case AUTHID_TRACE_SEAL:
            this->globals.context.ask_trace_seal = false;
            this->globals.context.trace_seal     = value;
            break;

        case AUTHID_SESSION_ID:
            this->globals.context.session_id = value;
            break;

        case AUTHID_END_DATE_CNX:
            this->globals.context.end_date_cnx = ulong_from_cstr(value);
            break;
        case AUTHID_END_TIME:
            this->globals.context.end_time     = value;
            break;

        case AUTHID_MODE_CONSOLE:
            this->globals.context.mode_console = value;
            break;
        case AUTHID_TIMEZONE:
            this->globals.context.timezone     = _long_from_cstr(value);
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            this->globals.context.real_target_device = value;
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            this->globals.context.authentication_challenge = value;
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
        case AUTHID_OPT_FILE_ENCRYPTION:
            if (size) {
                strncpy(buffer, (this->globals.enable_file_encryption ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_BPP:
            if (  size
               && !this->globals.context.ask_opt_bpp) {
                snprintf(buffer, size, "%u", this->globals.context.opt_bpp);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_HEIGHT:
            if (  size
               && !this->globals.context.ask_opt_height) {
                snprintf(buffer, size, "%u", this->globals.context.opt_height);
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_WIDTH:
            if (  size
               && !this->globals.context.ask_opt_width) {
                snprintf(buffer, size, "%u", this->globals.context.opt_width);
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR:
            if (  size
               && !this->globals.context.ask_selector) {
                strncpy(buffer, (this->globals.context.selector ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            if (  size
               && !this->globals.context.ask_selector_current_page) {
                snprintf(buffer, size, "%u", this->globals.context.selector_current_page);
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            if (!this->globals.context.ask_selector_device_filter) {
                pszReturn = this->globals.context.selector_device_filter;
            }
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            if ( !this->globals.context.ask_selector_group_filter) {
                pszReturn = this->globals.context.selector_group_filter;
            }
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            if (  size
               && !this->globals.context.ask_selector_group_filter) {
                snprintf(buffer, size, "%u", this->globals.context.selector_lines_per_page);
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            if (size) {
                snprintf(buffer, size, "%u", this->globals.context.selector_number_of_pages);
                pszReturn = buffer;
            }
            break;
        case AUTHID_TARGET_DEVICE:
            if (!this->globals.context.ask_target_device) {
                pszReturn = this->globals.target_device;
            }
            break;
        case AUTHID_TARGET_PASSWORD:
            if (!this->globals.context.ask_target_password) {
                pszReturn = this->globals.context.target_password;
            }
            break;
        case AUTHID_TARGET_PROTOCOL:
            if (!this->globals.context.ask_target_protocol) {
                pszReturn = this->globals.context.target_protocol;
            }
            break;
        case AUTHID_TARGET_USER:
            if (!this->globals.context.ask_target_user) {
                pszReturn = this->globals.target_user;
            }
            break;
        case AUTHID_AUTH_USER:
            if (!this->globals.context.ask_auth_user) {
                pszReturn = this->globals.auth_user;
            }
            break;
        case AUTHID_HOST:
            if ( !this->globals.context.ask_host) {
                pszReturn = this->globals.host;
            }
            break;
        case AUTHID_PASSWORD:
            if (!this->globals.context.ask_password) {
                pszReturn = this->globals.context.password;
            }
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            if (!this->globals.context.ask_authchannel_result) {
                pszReturn = this->globals.context.authchannel_result;
            }
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            if (!this->globals.context.ask_authchannel_target) {
                pszReturn = this->globals.context.authchannel_target;
            }
            break;
        case AUTHID_ACCEPT_MESSAGE:
            if (!this->globals.context.ask_accept_message) {
                pszReturn = this->globals.context.accept_message;
            }
            break;
        case AUTHID_DISPLAY_MESSAGE:
            if (!this->globals.context.ask_display_message) {
                pszReturn = this->globals.context.display_message;
            }
            break;
        case AUTHID_KEEPALIVE:
            if (  size
               && !this->globals.context.ask_keepalive) {
                strncpy(buffer, (this->globals.context.keepalive ? "True" : "False"), size);
                buffer[size - 1] = 0;

                pszReturn = buffer;
            }
            break;
        case AUTHID_PROXY_TYPE:
            if (!this->globals.context.ask_proxy_type) {
                pszReturn = this->globals.context.proxy_type;
            }
            break;
        case AUTHID_TRACE_SEAL:
            if (!this->globals.context.ask_trace_seal) {
                pszReturn = this->globals.context.trace_seal;
            }
            break;
        case AUTHID_END_TIME:
            pszReturn = this->globals.context.end_time;
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            pszReturn = this->globals.context.real_target_device;
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            pszReturn = this->globals.context.authentication_challenge;
            break;
        default:
//            LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
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
        case AUTHID_OPT_BPP:
            this->globals.context.ask_opt_bpp                 = true; break;

        case AUTHID_OPT_HEIGHT:
            this->globals.context.ask_opt_height              = true; break;

        case AUTHID_OPT_WIDTH:
            this->globals.context.ask_opt_width               = true; break;

        case AUTHID_SELECTOR:
            this->globals.context.ask_selector                = true; break;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->globals.context.ask_selector_current_page   = true; break;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->globals.context.ask_selector_device_filter  = true; break;

        case AUTHID_SELECTOR_GROUP_FILTER:
            this->globals.context.ask_selector_group_filter   = true; break;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->globals.context.ask_selector_lines_per_page = true; break;

        case AUTHID_TARGET_DEVICE:
            this->globals.context.ask_target_device           = true; break;

        case AUTHID_TARGET_PASSWORD:
            this->globals.context.ask_target_password         = true; break;

        case AUTHID_TARGET_PROTOCOL:
            this->globals.context.ask_target_protocol         = true; break;

        case AUTHID_TARGET_USER:
            this->globals.context.ask_target_user             = true; break;

        case AUTHID_AUTH_USER:
            this->globals.context.ask_auth_user               = true; break;

        case AUTHID_HOST:
            this->globals.context.ask_host                    = true; break;

        case AUTHID_TARGET:
            this->globals.context.ask_target                  = true; break;

        case AUTHID_PASSWORD:
            this->globals.context.ask_password                = true; break;

        case AUTHID_AUTHCHANNEL_RESULT:
            this->globals.context.ask_authchannel_result      = true; break;

        case AUTHID_AUTHCHANNEL_TARGET:
            this->globals.context.ask_authchannel_target      = true; break;

        case AUTHID_ACCEPT_MESSAGE:
            this->globals.context.ask_accept_message          = true; break;

        case AUTHID_DISPLAY_MESSAGE:
            this->globals.context.ask_display_message         = true; break;

        case AUTHID_KEEPALIVE:
            this->globals.context.ask_keepalive               = true; break;

        case AUTHID_PROXY_TYPE:
            this->globals.context.ask_proxy_type              = true; break;

        case AUTHID_TRACE_SEAL:
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
        case AUTHID_OPT_BPP:
            return this->globals.context.ask_opt_bpp;

        case AUTHID_OPT_HEIGHT:
            return this->globals.context.ask_opt_height;

        case AUTHID_OPT_WIDTH:
            return this->globals.context.ask_opt_width;

        case AUTHID_SELECTOR:
            return this->globals.context.ask_selector;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            return this->globals.context.ask_selector_current_page;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            return this->globals.context.ask_selector_device_filter;

        case AUTHID_SELECTOR_GROUP_FILTER:
            return this->globals.context.ask_selector_group_filter;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            return this->globals.context.ask_selector_lines_per_page;

        case AUTHID_TARGET_DEVICE:
            return this->globals.context.ask_target_device;

        case AUTHID_TARGET_PASSWORD:
            return this->globals.context.ask_target_password;

        case AUTHID_TARGET_PROTOCOL:
            return this->globals.context.ask_target_protocol;

        case AUTHID_TARGET_USER:
            return this->globals.context.ask_target_user;

        case AUTHID_AUTH_USER:
            return this->globals.context.ask_auth_user;

        case AUTHID_HOST:
            return this->globals.context.ask_host;

        case AUTHID_TARGET:
            return this->globals.context.ask_target;

        case AUTHID_PASSWORD:
            return this->globals.context.ask_password;

        case AUTHID_AUTHCHANNEL_RESULT:
            return this->globals.context.ask_authchannel_result;

        case AUTHID_AUTHCHANNEL_TARGET:
            return this->globals.context.ask_authchannel_target;

        case AUTHID_ACCEPT_MESSAGE:
            return this->globals.context.ask_accept_message;

        case AUTHID_DISPLAY_MESSAGE:
            return this->globals.context.ask_display_message;

        case AUTHID_KEEPALIVE:
            return this->globals.context.ask_keepalive;

        case AUTHID_PROXY_TYPE:
            return this->globals.context.ask_proxy_type;

        case AUTHID_TRACE_SEAL:
            return this->globals.context.ask_trace_seal;

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
            if (!this->globals.context.ask_selector) {
                return this->globals.context.selector;
            }
            break;
        case AUTHID_KEEPALIVE:
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

        this->context_ask(STRAUTHID_SELECTOR);
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
            this->context_ask(STRAUTHID_AUTH_USER);
        }
        else {
            this->context_set_value(AUTHID_AUTH_USER, auth_user);
        }
    }
};

#endif

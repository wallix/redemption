/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

    Configuration file,
    parsing config file rdpproxy.ini
*/

#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include <stdio.h>
#include <stdint.h>

#include <stdexcept>
#include <string>

#include "defines.hpp"
#include "fileutils.hpp"
#include "font.hpp"
#include "log.hpp"
#include "redirection_info.hpp"

using namespace std;

struct IniAccounts {
    char username[255]; // should use string
    char password[255]; // should use string
};

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

    AUTHID_RDP_BOGUS_SC_NET_SIZE,

    AUTHID_OPT_WABAGENT,
    AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT,
    AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT,

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

#define STRAUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE   "vnc_server_clipboard_encoding_type"

#define STRAUTHID_RDP_BOGUS_SC_NET_SIZE         "rdp_bogus_sc_net_size"

#define STRAUTHID_OPT_WABAGENT                      "wab_agent"
#define STRAUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT       "wab_agent_launch_timeout"
#define STRAUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT    "wab_agent_keepalive_timeout"

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

    STRAUTHID_RDP_BOGUS_SC_NET_SIZE,

    STRAUTHID_OPT_WABAGENT,
    STRAUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT,
    STRAUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT
};

static inline authid_t authid_from_string(const char * strauthid) {
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID - 1 ; i++) {
        if (0 == strcmp(authstr[i], strauthid)) {
            res = static_cast<authid_t>(i + 1);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    if ((authid == AUTHID_UNKNOWN) || (authid >= MAX_AUTHID))
        return "";
    return authstr[static_cast<unsigned>(authid) - 1];
}

#include "basefield.hpp"
#include "theme.hpp"
#include "noncopyable.hpp"

#include <type_traits>

class Inifile : public FieldObserver
{
    struct null_fill { null_fill() {}; };

    struct StringCopier
    {
        static void copy(char * dest, char const * src, std::size_t n) {
            const char * e = dest + n - 1;
            for (; dest != e && *src; ++src, ++dest) {
                *dest = *src;
            }
            *dest = 0;
        }
    };

    template<std::size_t N, class Copier, bool NullableString = false>
    class StaticStringBase : noncopyable
    {
        struct disable_ctor { };

    public:
        StaticStringBase(typename std::conditional<NullableString, null_fill, disable_ctor>::type) noexcept
        : str{}
        {}

        StaticStringBase() noexcept
        { this->str[0] = 0; }

        StaticStringBase(const char * s)
        { *this = s; }

        StaticStringBase& operator=(const char * s) {
            Copier::copy(this->str, s, N);
            return *this;
        }

        const char * c_str() const noexcept {
            return this->str;
        }

        constexpr operator const char * () const noexcept {
            return this->str;
        }

        constexpr std::size_t max_size() const noexcept {
            return N-1;
        }

        char * data() noexcept {
            return this->str;
        }

    protected:
        char str[N];

        // disable deleted constructor
        friend class Inifile;
        StaticStringBase(StaticStringBase &&);
    };

public:
    template<std::size_t N>
    using StaticString = StaticStringBase<N, StringCopier>;

    template<std::size_t N>
    using StaticNilString = StaticStringBase<N, StringCopier, true>;

    template<std::size_t N>
    struct StaticKeyString : StaticStringBase<N, null_fill, true>
    {
        StaticKeyString(const char * s) {
            this->setmem(s);
        }

        void setmem(const char * s, std::size_t n) {
            memcpy(this->str, s, n);
        }

        void setmem(const char * s) {
            this->setmem(s, N);
        }

        constexpr std::size_t max_size() const noexcept {
            return N;
        }
    };

    struct Ipv4Copier : StringCopier {};
    using StaticIpString = StaticStringBase<16, Ipv4Copier>;

    struct PathCopier
    {
        static void copy(char * dest, char const * src, std::size_t n) {
            pathncpy(dest, src, n);
        }
    };

    template<std::size_t N>
    using StaticPath = StaticStringBase<N, PathCopier>;


    struct Inifile_globals {
        BoolField capture_chunk;

        StringField auth_user;                      // AUTHID_AUTH_USER
        StringField host;                           // client_ip AUTHID_HOST
        StringField target;                         // target ip AUTHID_TARGET
        StringField target_device;                  // AUTHID_TARGET_DEVICE
        StringField target_user;                    // AUTHID_TARGET_USER
        StringField target_application;             // AUTHID_TARGET_APPLICATION
        StringField target_application_account;     // AUTHID_TARGET_APPLICATION_ACCOUNT
        StringField target_application_password;    // AUTHID_TARGET_APPLICATION_PASSWORD

        // BEGIN globals
        bool bitmap_cache               = true;
        bool glyph_cache                = false;
        unsigned port                   = 3389;
        bool nomouse                    = false;
        bool notimestamp                = false;
        int  encryptionLevel            = 0; // 0=low, 1=medium, 2=high
        StaticIpString authip           = "127.0.0.1";
        unsigned authport               = 3350;

        // keepalive and no traffic auto deconnexion
        unsigned session_timeout        = 900;
        unsigned keepalive_grace_delay  = 30;
        unsigned close_timeout          = 600; // timeout of close box in seconds (0 to desactivate)

        StaticNilString<8> auth_channel          = null_fill();
        BoolField          enable_file_encryption;  // AUTHID_OPT_FILE_ENCRYPTION //
        StaticIpString     listen_address        = "0.0.0.0";
        bool               enable_ip_transparent = false;
        StaticString<256>  certificate_password  = "inquisition";

        StaticString<1024> png_path = PNG_PATH;
        StaticString<1024> wrm_path = WRM_PATH;

        StringField alternate_shell;            // STRAUTHID_ALTERNATE_SHELL //
        StringField shell_working_directory;    // STRAUTHID_SHELL_WORKING_DIRECTORY //

        StringField codec_id;                   // AUTHID_OPT_CODEC_ID //
        BoolField   movie;                      // AUTHID_OPT_MOVIE //
        StringField movie_path;                 // AUTHID_OPT_MOVIE_PATH //
        StringField video_quality;              // AUTHID_VIDEO_QUALITY //
        bool        enable_bitmap_update = true;
        bool        enable_close_box = true;
        bool        enable_osd = true;
        bool        enable_osd_display_remote_target = true;

        BoolField       enable_wab_agent;               // AUTHID_OPT_WABAGENT //
        UnsignedField   wab_agent_launch_timeout;       // AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT //
        UnsignedField   wab_agent_keepalive_timeout;    // AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT //
        // END globals

        StaticPath<1024> persistent_path = PERSISTENT_PATH;

        bool disable_proxy_opt = false;

        // The maximum length of the chunked virtual channel data.
        uint32_t max_chunked_virtual_channel_data_length = 2 * 1024 * 1024;

        Inifile_globals() = default;
    } globals;

    // section "client"
    struct Inifile_client {
        UnsignedField keyboard_layout;    // AUTHID_KEYBOARD_LAYOUT
        bool ignore_logon_password = false; // if true, ignore password provided by RDP client, user need do login manually. default

        uint32_t performance_flags_default           = 0;   // PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        uint32_t performance_flags_force_present     = 0x8; // Disable theme (0x8)
        uint32_t performance_flags_force_not_present = 0x80;// Disable font smoothing (0x80)

        bool tls_fallback_legacy = true;
        bool tls_support         = true;
        bool bogus_neg_request   = false;   // needed to connect with jrdp, based on bogus X224 layer code
        bool bogus_user_id       = true;    // needed to connect with Remmina 0.8.3 (Squeeze), based on bogus MCS layer code

        BoolField disable_tsk_switch_shortcuts; // AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS //

        int rdp_compression = 4; // 0 - Disabled, 1 - RDP 4.0, 2 - RDP 5.0, 3 - RDP 6.0, 4 - RDP 6.1

        uint32_t max_color_depth = 24; // 8-bit, 15-bit, 16-bit, 24-bit, 32-bit (not yet supported) Default (24-bit)

        bool persistent_disk_bitmap_cache   = false;
        bool cache_waiting_list             = true;
        bool persist_bitmap_cache_on_disk   = false;

        bool bitmap_compression = true;

        bool fast_path = true;

        Inifile_client() = default;
    } client;

    struct Inifile_mod_rdp {
        int rdp_compression = 4; // 0 - Disabled, 1 - RDP 4.0, 2 - RDP 5.0, 3 - RDP 6.0, 4 - RDP 6.1

        bool disconnect_on_logon_user_change = false;

        uint32_t open_session_timeout        = 0;

        unsigned certificate_change_action   = 0; // 0 - Interrupt connection, 1 - Replace certificate then continue

        // +----+-----------------+
        // | Id | Meaning         |
        // +----+-----------------+
        // | 15 | MultiDstBlt     |
        // +----+-----------------+
        // | 16 | MultiPatBlt     |
        // +----+-----------------+
        // | 17 | MultiScrBlt     |
        // +----+-----------------+
        // | 18 | MultiOpaqueRect |
        // +----+-----------------+
        // | 22 | Polyline        |
        // +----+-----------------+
        std::string extra_orders = "15,16,17,18,22";

        bool enable_nla      = true;
        bool enable_kerberos = false;

        bool persistent_disk_bitmap_cache   = false;
        bool cache_waiting_list             = true;
        bool persist_bitmap_cache_on_disk   = false;

        /**
         * channel1,channel2,etc
         * @{
         */
        std::string allow_channels = "*";
        std::string deny_channels;
        // @}

        bool fast_path = true;

        bool server_redirection_support = false;

        RedirectionInfo redir_info;

        // needed to connect with VirtualBox, based on bogus TS_UD_SC_NET data block
        BoolField bogus_sc_net_size;    // AUTHID_RDP_BOGUS_SC_NET_SIZE //

        Inifile_mod_rdp() = default;
    } mod_rdp;

    struct Inifile_mod_vnc {
        BoolField clipboard_up;     // AUTHID_VNC_CLIPBOARD_UP //
        BoolField clipboard_down;   // AUTHID_VNC_CLIPBOARD_DOWN //

        std::string encodings;

        bool allow_authentification_retries = false;

        Inifile_mod_vnc() = default;

        StringField server_clipboard_encoding_type;    // AUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE //
    } mod_vnc;

    struct Inifile_mod_replay {
        int on_end_of_data = 0; // 0 - Wait for Escape, 1 - End session

        Inifile_mod_replay() = default;
    } mod_replay;

    // Section "video"
    struct Inifile_video {
        unsigned capture_flags  = 3; // 1 png, 2 wrm, 4 flv, 8 ocr
        // video opt from capture_flags
        bool     capture_png    = true;
        bool     capture_wrm    = true;
        bool     capture_flv    = false;
        bool     capture_ocr    = false;

        std::string ocr_locale;
        unsigned ocr_interval               = 100; // 1 every second
        bool     ocr_on_title_bar_only      = false;
        unsigned ocr_max_unrecog_char_rate  = 40; // expressed in percentage,
                                                  //     0   - all of characters need be recognized
                                                  //     100 - accept all results

        unsigned png_interval       = 3000; // time between 2 png captures (in 1/10 seconds)
        unsigned capture_groupid    = 33;
        unsigned frame_interval     = 40;   // time between 2 frame captures (in 1/100 seconds) (default: 2,5 frame per second)
        unsigned break_interval     = 600;  // time between 2 wrm movies (in seconds)
        unsigned png_limit          = 5;    // number of png captures to keep

        uint64_t flv_break_interval = 0;  // time between 2 flv movies captures (in seconds)

        StaticString<1024> replay_path = "/tmp/";

        unsigned l_bitrate   = 10000; // bitrate for low quality
        unsigned l_framerate = 5;     // framerate for low quality
        unsigned l_height    = 480;   // height for low quality
        unsigned l_width     = 640;   // width for low quality
        unsigned l_qscale    = 28;    // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        unsigned m_bitrate   = 20000;
        unsigned m_framerate = 5;
        unsigned m_height    = 768;
        unsigned m_width     = 1024;
        unsigned m_qscale    = 14;

        // Same for high quality
        unsigned h_bitrate   = 30000;
        unsigned h_framerate = 5;
        unsigned h_height    = 2048;
        unsigned h_width     = 2048;
        unsigned h_qscale    = 7;

        StaticPath<1024> hash_path       = HASH_PATH;
        StaticPath<1024> record_tmp_path = RECORD_TMP_PATH;
        StaticPath<1024> record_path     = RECORD_PATH;

        bool     inactivity_pause   = false;
        unsigned inactivity_timeout = 300;

        // 1 - Disable keyboard event logging in syslog
        // 2 - Disable keyboard event logging in WRM
        // 4 - Disable keyboard event logging in META
        UnsignedField disable_keyboard_log;     // AUTHID_DISABLE_KEYBOARD_LOG
        bool disable_keyboard_log_syslog = false;
        bool disable_keyboard_log_wrm    = false;
        bool disable_keyboard_log_ocr    = false;

        // 1 - Disable clipboard event logging in syslog
        UnsignedField disable_clipboard_log;    // AUTHID_DISABLE_CLIPBOARD_LOG
        bool disable_clipboard_log_syslog = false;

        UnsignedField rt_display;   // AUTHID_RT_DISPLAY
                                    // 0: disable, 1: enable

        unsigned wrm_color_depth_selection_strategy = 0; // 0: 24-bit, 1: 16-bit

        unsigned wrm_compression_algorithm = 0; // 0: uncompressed, 1: GZip, 2: Snappy

        Inifile_video() = default;
    } video;

    // Section "crypto"
    struct Inifile_crypto {
        StaticKeyString<32> key0 =
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";
        StaticKeyString<32> key1 =
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F";

        Inifile_crypto() = default;
    } crypto;

    // Section "debug"
    struct Inifile_debug {
        uint32_t x224               = 0;
        uint32_t mcs                = 0;
        uint32_t sec                = 0;
        uint32_t rdp                = 0;
        uint32_t primary_orders     = 0;
        uint32_t secondary_orders   = 0;
        uint32_t bitmap             = 0;
        uint32_t capture            = 0;
        uint32_t auth               = 0;
        uint32_t session            = 0;
        uint32_t front              = 0;
        uint32_t mod_rdp            = 0;
        uint32_t mod_vnc            = 0;
        uint32_t mod_int            = 0;
        uint32_t mod_xup            = 0;
        uint32_t widget             = 0;
        uint32_t input              = 0;
        uint32_t password           = 0;
        uint32_t compression        = 0;
        uint32_t cache              = 0;
        uint32_t bitmap_update      = 0;
        uint32_t performance        = 0;

        uint32_t pass_dialog_box    = 0;

        // 0 = disable, 1 = enable, 2 = enable but no specified
        uint32_t config             = 2;

        Inifile_debug() = default;
    } debug;

    static const uint32_t ENABLE_DEBUG_CONFIG = 1;

    // section "translation"
    struct Inifile_translation {
        StringField language;

        Inifile_translation() = default;
    } translation;

    // section "context"
    struct Inifile_context {
        StaticString<1024> movie;                    // --

        UnsignedField      opt_bitrate;              // AUTHID_OPT_BITRATE //
        UnsignedField      opt_framerate;            // AUTHID_OPT_FRAMERATE //
        UnsignedField      opt_qscale;               // AUTHID_OPT_QSCALE //

        UnsignedField      opt_bpp;                  // AUTHID_OPT_BPP //
        UnsignedField      opt_height;               // AUTHID_OPT_HEIGHT //
        UnsignedField      opt_width;                // AUTHID_OPT_WIDTH //

        // auth_error_message is left as std::string type
        // because SocketTransport and ReplayMod take it as argument on
        // constructor and modify it as a std::string
        std::string        auth_error_message;       // AUTHID_AUTH_ERROR_MESSAGE --

        BoolField          selector;                 // AUTHID_SELECTOR //
        UnsignedField      selector_current_page;    // AUTHID_SELECTOR_CURRENT_PAGE //
        StringField        selector_device_filter;   // AUTHID_SELECTOR_DEVICE_FILTER //
        StringField        selector_group_filter;    // AUTHID_SELECTOR_GROUP_FILTER //
        StringField        selector_proto_filter;    // AUTHID_SELECTOR_PROTO_FILTER //
        UnsignedField      selector_lines_per_page;  // AUTHID_SELECTOR_LINES_PER_PAGE //
        UnsignedField      selector_number_of_pages; // AUTHID_SELECTOR_NUMBER_OF_PAGES //

        StringField        target_password;          // AUTHID_TARGET_PASSWORD //
        StringField        target_host;              // AUTHID_TARGET_HOST //
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

        TODO("why are the field below Strings ? They should be booleans. As they can only contain True/False to know if a user clicked on a button")
        StringField        accept_message;           // AUTHID_ACCEPT_MESSAGE //
        StringField        display_message;          // AUTHID_DISPLAY_MESSAGE //


        StringField        rejected;                 // AUTHID_REJECTED //

        BoolField          authenticated;            // AUTHID_AUTHENTICATED //

        BoolField          keepalive;                // AUTHID_KEEPALIVE //

        StringField        session_id;               // AUTHID_SESSION_ID //

        UnsignedField      end_date_cnx;             // AUTHID_END_DATE_CNX //
        StringField        end_time;                 // AUTHID_END_TIME //

        StringField        mode_console;             // AUTHID_MODE_CONSOLE //
        SignedField        timezone;                 // AUTHID_TIMEZONE //

        StringField        real_target_device;       // AUHTID_REAL_TARGET_DEVICE  //

        BoolField          authentication_challenge; // AUTHID_AUTHENTICATION_CHALLENGE //

        StringField        ticket;                   // AUTHID_TICKET //
        StringField        comment;                  // AUTHID_COMMENT //
        StringField        duration;                  // AUTHID_DURATION //
        StringField        waitinforeturn;           // AUTHID_WAITINFORETURN //
        BoolField          showform;                 // AUTHID_SHOWFORM //
        UnsignedField      formflag;                 // AUTHID_FORMFLAG //

        StringField        module;                   // AUTHID_MODULE //
        BoolField          forcemodule;              // AUTHID_FORCEMODULE //
        StringField        proxy_opt;                // AUTHID_PROXY_OPT //

        Inifile_context() = default;
    } context;

    Theme theme;

    IniAccounts account;

    Font font;

public:
    Inifile(const char * default_font_name = SHARE_PATH "/" DEFAULT_FONT_NAME) :
    font(default_font_name)
    {
        //init to_send_set of authid
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
        this->to_send_set.insert(AUTHID_TARGET_HOST);
        this->to_send_set.insert(AUTHID_TARGET_USER);
        this->to_send_set.insert(AUTHID_OPT_WIDTH);
        this->to_send_set.insert(AUTHID_OPT_HEIGHT);
        this->to_send_set.insert(AUTHID_OPT_BPP);
        this->to_send_set.insert(AUTHID_REAL_TARGET_DEVICE);

        this->to_send_set.insert(AUTHID_AUTHCHANNEL_RESULT);
        this->to_send_set.insert(AUTHID_AUTHCHANNEL_TARGET);

        this->something_changed = false;

        this->to_send_set.insert(AUTHID_AUTH_USER);
        this->globals.auth_user.attach_ini(this,              AUTHID_AUTH_USER);

        this->to_send_set.insert(AUTHID_HOST);
        this->globals.host.attach_ini(this,                   AUTHID_HOST);

        this->to_send_set.insert(AUTHID_TARGET);
        this->globals.target.attach_ini(this,                 AUTHID_TARGET);

        this->to_send_set.insert(AUTHID_TARGET_USER);
        this->globals.target_user.attach_ini(this,            AUTHID_TARGET_USER);

        this->globals.target_application.attach_ini(this,           AUTHID_TARGET_APPLICATION);
        this->globals.target_application_account.attach_ini(this,   AUTHID_TARGET_APPLICATION_ACCOUNT);
        this->globals.target_application_password.attach_ini(this,  AUTHID_TARGET_APPLICATION_PASSWORD);

        this->to_send_set.insert(AUTHID_TARGET_DEVICE);
        this->globals.target_device.attach_ini(this,          AUTHID_TARGET_DEVICE);

        this->globals.enable_file_encryption.attach_ini(this, AUTHID_OPT_FILE_ENCRYPTION);

        // Init globals

        // this->globals.enable_file_encryption = false;

        this->globals.alternate_shell.attach_ini(this, AUTHID_ALTERNATE_SHELL);
        this->globals.shell_working_directory.attach_ini(this, AUTHID_SHELL_WORKING_DIRECTORY);

        this->globals.codec_id.attach_ini(this, AUTHID_OPT_CODEC_ID);
        this->globals.movie.attach_ini(this, AUTHID_OPT_MOVIE);
        this->globals.movie_path.attach_ini(this, AUTHID_OPT_MOVIE_PATH);
        this->globals.video_quality.attach_ini(this, AUTHID_VIDEO_QUALITY);

        this->globals.codec_id.set_from_cstr("flv");
        this->globals.movie.set(false);
        TODO("this could be some kind of enumeration");
        this->globals.video_quality.set_from_cstr("medium");

        this->globals.enable_wab_agent.attach_ini(this, AUTHID_OPT_WABAGENT);
        this->globals.enable_wab_agent.set(false);

        this->globals.wab_agent_launch_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_LAUNCH_TIMEOUT);
        this->globals.wab_agent_launch_timeout.set(0);

        this->globals.wab_agent_keepalive_timeout.attach_ini(this, AUTHID_OPT_WABAGENT_KEEPALIVE_TIMEOUT);
        this->globals.wab_agent_keepalive_timeout.set(0);
        // End Init globals

        this->account.username[0] = 0;
        this->account.password[0] = 0;

        // Begin Section "client".
        this->client.keyboard_layout.attach_ini(this, AUTHID_KEYBOARD_LAYOUT);
        this->client.keyboard_layout.set(0);
        this->to_send_set.insert(AUTHID_KEYBOARD_LAYOUT);

        this->client.disable_tsk_switch_shortcuts.attach_ini(this, AUTHID_DISABLE_TSK_SWITCH_SHORTCUTS);
        this->client.disable_tsk_switch_shortcuts.set(false);
        // End Section "client"

        // Begin section "mod_rdp"
        this->mod_rdp.bogus_sc_net_size.attach_ini(this, AUTHID_RDP_BOGUS_SC_NET_SIZE);
        this->mod_rdp.bogus_sc_net_size.set(true);
        // End Section "mod_rdp"

        // Begin section "mod_vnc"
        this->mod_vnc.clipboard_up.attach_ini(this,AUTHID_VNC_CLIPBOARD_UP);
        this->mod_vnc.clipboard_down.attach_ini(this,AUTHID_VNC_CLIPBOARD_DOWN);

        this->mod_vnc.server_clipboard_encoding_type.attach_ini(this, AUTHID_VNC_SERVER_CLIPBOARD_ENCODING_TYPE);
        this->mod_vnc.server_clipboard_encoding_type.set_from_cstr("latin1");
        // End Section "mod_vnc"

        // Begin section video
        this->video.disable_keyboard_log.attach_ini(this, AUTHID_DISABLE_KEYBOARD_LOG);
        this->video.disable_keyboard_log.set(0);
        this->to_send_set.insert(AUTHID_DISABLE_KEYBOARD_LOG);

        this->video.disable_clipboard_log.attach_ini(this, AUTHID_DISABLE_CLIPBOARD_LOG);
        this->video.disable_clipboard_log.set(0);

        this->video.rt_display.attach_ini(this, AUTHID_RT_DISPLAY);
        this->video.rt_display.set(0);
        // End section "video"

        // Begin Section "translation"
        this->translation.language.set_from_cstr("en");
        this->translation.language.attach_ini(this,AUTHID_LANGUAGE);
        // End Section "translation"

        // Begin section "context"
        this->context.opt_bitrate.set(40000);
        this->context.opt_framerate.set(5);
        this->context.opt_qscale.set(15);

        this->context.opt_bitrate.attach_ini(this,AUTHID_OPT_BITRATE);
        this->context.opt_framerate.attach_ini(this,AUTHID_OPT_FRAMERATE);
        this->context.opt_qscale.attach_ini(this,AUTHID_OPT_QSCALE);

        this->context.opt_bpp.set(24);
        this->context.opt_height.set(600);
        this->context.opt_width.set(800);

        this->context.selector.set(false);
        this->context.selector_current_page.set(1);
        this->context.selector_lines_per_page.set(0);
        this->context.selector_number_of_pages.set(1);

        this->context.selector_number_of_pages.attach_ini(this, AUTHID_SELECTOR_NUMBER_OF_PAGES);

        this->globals.target_device.ask();
        this->globals.target_user.ask();
        this->globals.auth_user.ask();


        this->context.target_password.ask();

        this->context.target_port.set(3389);
        this->context.target_port.ask();

        this->context.target_host.set_from_cstr("");
        this->context.target_host.ask();

        this->context.target_protocol.set_from_cstr("RDP");
        this->context.target_protocol.ask();

        this->context.password.ask();


        this->context.message.attach_ini(this, AUTHID_MESSAGE);

        this->context.pattern_kill.attach_ini(this, AUTHID_PATTERN_KILL);

        this->context.pattern_notify.attach_ini(this, AUTHID_PATTERN_NOTIFY);

        this->context.accept_message.set_empty();
        this->context.display_message.set_empty();

        this->context.rejected.attach_ini(this, AUTHID_REJECTED);

        this->context.authenticated.set(false);
        this->context.authenticated.attach_ini(this, AUTHID_AUTHENTICATED);

        this->context.keepalive.set(false);

        this->context.session_id.attach_ini(this, AUTHID_SESSION_ID);

        this->context.end_date_cnx.set(0);
        this->context.end_date_cnx.attach_ini(this, AUTHID_END_DATE_CNX);
        this->context.end_time.attach_ini(this, AUTHID_END_TIME);

        this->context.mode_console.set_from_cstr("allow");
        this->context.timezone.set(-3600);
        this->context.mode_console.attach_ini(this, AUTHID_MODE_CONSOLE);
        this->context.timezone.attach_ini(this, AUTHID_TIMEZONE);

        this->context.authentication_challenge.ask();
        this->context.authentication_challenge.attach_ini(this, AUTHID_AUTHENTICATION_CHALLENGE);


        this->to_send_set.insert(AUTHID_MODULE);
        this->to_send_set.insert(AUTHID_TICKET);
        this->to_send_set.insert(AUTHID_COMMENT);
        this->to_send_set.insert(AUTHID_DURATION);
        this->to_send_set.insert(AUTHID_WAITINFORETURN);
        this->to_send_set.insert(AUTHID_SHOWFORM);
        this->to_send_set.insert(AUTHID_FORMFLAG);

        this->context.module.set_from_cstr("login");
        this->context.module.attach_ini(this, AUTHID_MODULE);
        this->context.module.use();
        this->context.forcemodule.set(false);
        this->context.forcemodule.attach_ini(this, AUTHID_FORCEMODULE);

        this->context.proxy_opt.attach_ini(this, AUTHID_PROXY_OPT);

        this->context.ticket.set_from_cstr("");
        this->context.ticket.attach_ini(this, AUTHID_TICKET);
        this->context.ticket.use();
        this->context.comment.set_from_cstr("");
        this->context.comment.attach_ini(this, AUTHID_COMMENT);
        this->context.comment.use();
        this->context.duration.set_from_cstr("");
        this->context.duration.attach_ini(this, AUTHID_DURATION);
        this->context.duration.use();
        this->context.waitinforeturn.set_from_cstr("");
        this->context.waitinforeturn.attach_ini(this, AUTHID_WAITINFORETURN);
        this->context.waitinforeturn.use();
        this->context.showform.set(false);
        this->context.showform.attach_ini(this, AUTHID_SHOWFORM);
        this->context.showform.use();
        this->context.formflag.set(0);
        this->context.formflag.attach_ini(this, AUTHID_FORMFLAG);
        this->context.formflag.use();

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
        this->context.target_host.attach_ini(this,AUTHID_TARGET_HOST);
        this->context.target_port.attach_ini(this,AUTHID_TARGET_PORT);

        this->context.password.attach_ini(this,AUTHID_PASSWORD);

        this->context.reporting.attach_ini(this,AUTHID_REPORTING);

        this->context.accept_message.attach_ini(this,AUTHID_ACCEPT_MESSAGE);
        this->context.display_message.attach_ini(this,AUTHID_DISPLAY_MESSAGE);

        this->context.real_target_device.attach_ini(this,AUTHID_REAL_TARGET_DEVICE);

        this->context.authchannel_answer.attach_ini(this,AUTHID_AUTHCHANNEL_ANSWER);
        this->context.authchannel_target.attach_ini(this,AUTHID_AUTHCHANNEL_TARGET);
        this->context.authchannel_result.attach_ini(this,AUTHID_AUTHCHANNEL_RESULT);
        this->context.keepalive.attach_ini(this,AUTHID_KEEPALIVE);
    }

public:
    virtual void set_value(const char * context, const char * key, const char * value)
    {
        if (0 == strcmp(context, "globals")) {
            if (0 == strcmp(key, "bitmap_cache")) {
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "glyph_cache")) {
                this->globals.glyph_cache = bool_from_cstr(value);
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
                this->globals.authip = value;
            }
            else if (0 == strcmp(key, "authport")) {
                this->globals.authport = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session_timeout")) {
                this->globals.session_timeout = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")) {
                this->globals.keepalive_grace_delay = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "close_timeout")) {
                this->globals.close_timeout = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth_channel")) {
                this->globals.auth_channel = value;
            }
            else if (0 == strcmp(key, "enable_file_encryption")) {
                this->globals.enable_file_encryption.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "listen_address")) {
                this->globals.listen_address = value;
            }
            else if (0 == strcmp(key, "enable_ip_transparent")) {
                this->globals.enable_ip_transparent = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_password")) {
                this->globals.certificate_password = value;
            }
            else if (0 == strcmp(key, "png_path")) {
                this->globals.png_path = value;
            }
            else if (0 == strcmp(key, "wrm_path")) {
                this->globals.wrm_path = value;
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
            else if (0 == strcmp(key, "enable_osd")) {
                this->globals.enable_osd = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_osd_display_remote_target")) {
                this->globals.enable_osd_display_remote_target = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "persistent_path")) {
                this->globals.persistent_path = value;
            }
            else if (0 == strcmp(key, "disable_proxy_opt")) {
                this->globals.disable_proxy_opt = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_chunked_virtual_channel_data_length")) {
                this->globals.max_chunked_virtual_channel_data_length = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_wab_agent")) {
                this->globals.enable_wab_agent.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "wab_agent_launch_timeout")) {
                this->globals.wab_agent_launch_timeout.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "wab_agent_keepalive_timeout")) {
                this->globals.wab_agent_keepalive_timeout.set_from_cstr(value);
            }
            else if (this->debug.config) {
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
            else if (0 == strcmp(key, "bogus_user_id")) {
                this->client.bogus_user_id = bool_from_cstr(value);
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
                    (this->client.max_color_depth != 24)/* &&
                    (this->client.max_color_depth != 32)*/)
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
            else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
                this->client.persist_bitmap_cache_on_disk = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "fast_path")) {
                this->client.fast_path = bool_from_cstr(value);
            }
            else if (this->debug.config) {
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
                this->mod_rdp.extra_orders = value;
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
            else if (0 == strcmp(key, "persist_bitmap_cache_on_disk")) {
                this->mod_rdp.persist_bitmap_cache_on_disk = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "allow_channels")) {
                this->mod_rdp.allow_channels = value;
            }
            else if (0 == strcmp(key, "deny_channels")) {
                this->mod_rdp.deny_channels = value;
            }
            else if (0 == strcmp(key, "fast_path")) {
                this->mod_rdp.fast_path = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "server_redirection_support")) {
                this->mod_rdp.server_redirection_support = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bogus_sc_net_size")) {
                this->mod_rdp.bogus_sc_net_size.set_from_cstr(value);
            }
            else if (this->debug.config) {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "mod_vnc")) {
            if (0 == strcmp(key, "clipboard_up")) {
                this->mod_vnc.clipboard_up.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard_down")) {
                this->mod_vnc.clipboard_down.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "encodings")) {
                this->mod_vnc.encodings = value;
            }
            else if (0 == strcmp(key, "allow_authentification_retries")) {
                this->mod_vnc.allow_authentification_retries = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "server_clipboard_encoding_type")) {
                this->mod_vnc.server_clipboard_encoding_type.set_from_cstr(value);
            }
            else if (this->debug.config) {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "mod_replay")) {
            if (0 == strcmp(key, "on_end_of_data")) {
                this->mod_replay.on_end_of_data = ulong_from_cstr(value);
            }
            else if (this->debug.config) {
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
            else if (0 == strcmp(key, "ocr_locale")) {
                this->video.ocr_locale = value;
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
                this->video.replay_path = value;
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
//            else if (0 == strcmp(key, "movie_path")) {
//                this->globals.movie_path.set_from_cstr(value);
//            }
            else if (0 == strcmp(key, "hash_path")) {
                this->video.hash_path = value;
            }
            else if (0 == strcmp(key, "record_path")) {
                this->video.record_path = value;
            }
            else if (0 == strcmp(key, "record_tmp_path")) {
                this->video.record_tmp_path = value;
            }
            else if (0 == strcmp(key, "disable_keyboard_log")) {
                this->video.disable_keyboard_log.set_from_cstr(value);
                this->video.disable_keyboard_log_syslog = 0 != (this->video.disable_keyboard_log.get() & 1);
                this->video.disable_keyboard_log_wrm    = 0 != (this->video.disable_keyboard_log.get() & 2);
                this->video.disable_keyboard_log_ocr    = 0 != (this->video.disable_keyboard_log.get() & 4);
            }
            else if (0 == strcmp(key, "disable_clipboard_log")) {
                this->video.disable_clipboard_log.set_from_cstr(value);
                this->video.disable_clipboard_log_syslog = 0 != (this->video.disable_clipboard_log.get() & 1);
            }
            else if (0 == strcmp(key, "rt_display")) {
                this->video.rt_display.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "wrm_color_depth_selection_strategy")) {
                this->video.wrm_color_depth_selection_strategy = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "wrm_compression_algorithm")) {
                this->video.wrm_compression_algorithm = ulong_from_cstr(value);
            }
            else if (this->debug.config) {
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

                        this->crypto.key0.data()[i] = strtol(hexval, &end, 16);
                    }
                }
            }
            else if (0 == strcmp(key, "key1")) {
                if (strlen(value) >= sizeof(this->crypto.key1) * 2) {
                    char   hexval[3] = { 0 };
                    char * end;
                    for (size_t i = 0; i < sizeof(this->crypto.key1); i++) {
                        memcpy(hexval, value + i * 2, 2);

                        this->crypto.key1.data()[i] = strtol(hexval, &end, 16);
                    }
                }
            }
            else if (this->debug.config) {
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
            else if (0 == strcmp(key, "password")) {
                this->debug.password          = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "compression")) {
                this->debug.compression       = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "cache")) {
                this->debug.cache             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_update")) {
                this->debug.bitmap_update     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "pass_dialog_box")) {
                this->debug.pass_dialog_box   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance")) {
                this->debug.performance       = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "config")) {
                this->debug.config            = bool_from_cstr(value) ? 1 : 0;
            }
            else if (this->debug.config) {
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
            else if (this->debug.config) {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "internal_mod")) {
            if (0 == strcmp(key, "load_theme")) {
                if (value && *value) {
                    LOG(LOG_INFO, "LOAD_THEME: %s", value);
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
            else if (this->debug.config) {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (this->debug.config) {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }   // void set_value(const char * context, const char * key, const char * value)


private:
    BaseField * get_field(authid_t authid) const {
        auto pos = this->field_list.find(authid);
        if (pos != this->field_list.end()) {
            return pos->second;
        }
        return nullptr;
    }

public:
    /******************
     * Set_from_acl sets a value to corresponding field but does not mark it as changed
     */
    void set_from_acl(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            if (authid == AUTHID_AUTH_ERROR_MESSAGE) {
                this->context.auth_error_message = value;
            }
            else {
                if (BaseField * field = this->get_field(authid)) {
                    field->set_from_acl(value);
                }
                else {
                    LOG(LOG_WARNING, "Inifile::set_from_acl(id): unknown authid=%d", authid);
                }
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
            if (BaseField * field = this->get_field(authid)) {
                field->ask_from_acl();
            }
            else {
                LOG(LOG_WARNING, "Inifile::ask_from_acl(id): unknown authid=%d", authid);
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::ask_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value(authid_t authid, const char * value) {
        switch (authid)
            {
            // Context
            case AUTHID_AUTH_ERROR_MESSAGE:
                this->context.auth_error_message = value;
                break;
            default:
                if (BaseField * field = this->get_field(authid)) {
                    field->set_from_cstr(value);
                }
                else {
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

        //LOG(LOG_INFO, "Getting value for authid=%d", authid);
        switch (authid)
            {
            case AUTHID_AUTH_ERROR_MESSAGE:
                pszReturn = this->context.auth_error_message.c_str();
                break;
            default:
                if (BaseField * field = this->get_field(authid)) {
                    if (!field->is_asked()) {
                        pszReturn = field->get_value();
                    }
                }
                else {
                    LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
                }
                break;
            }

        return pszReturn;
    }

    void context_ask(authid_t authid) {
        if (BaseField * field = this->get_field(authid)) {
            field->ask();
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
        }
    }

    bool context_is_asked(authid_t authid) {
        if (BaseField * field = this->get_field(authid)) {
            return field->is_asked();
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
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
        this->context_ask(AUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        this->context_set_value(AUTHID_AUTH_USER, username);
        this->context_ask(AUTHID_TARGET_USER);
        this->context_ask(AUTHID_TARGET_DEVICE);
        this->context_ask(AUTHID_TARGET_PROTOCOL);
    }
};

#endif

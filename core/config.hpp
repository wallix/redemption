/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   configuration file,
   parsing config file xrdp.ini values and names helped by
   lib_boost and saved in Inifile object.

*/


#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <istream>
#include <string>
#include <stdint.h>

typedef enum{
    AUTHID_UNKNOWN = 0,
    AUTHID_TARGET_USER,     // target_login
    AUTHID_TARGET_PASSWORD, // target_password
    AUTHID_HOST,            // ip_client
    AUTHID_PASSWORD,        // password
    AUTHID_AUTH_USER,       // login
    AUTHID_TARGET_DEVICE,   // target_device
    AUTHID_TARGET_PORT,     // target_port
    AUTHID_TARGET_PROTOCOL, // proto_dest
    AUTHID_END_TIME,        // end time as text
    AUTHID_SELECTOR_GROUP_FILTER, // group filter text
    AUTHID_SELECTOR_DEVICE_FILTER, // device filter text
    AUTHID_SELECTOR_LINES_PER_PAGE, // number of lines per page
    AUTHID_SELECTOR_NUMBER_OF_PAGES, // number of pages
    AUTHID_SELECTOR_CURRENT_PAGE, // current page
    AUTHID_REJECTED,        // rejected
    AUTHID_OPT_MOVIE,       // is_rec
    AUTHID_OPT_MOVIE_PATH,  // rec_path
    AUTHID_OPT_CLIPBOARD,   // clipboard
    AUTHID_OPT_DEVICEREDIRECTION, // device_redirection
    AUTHID_END_DATE_CNX,    // timeclose
    AUTHID_MESSAGE, // warning_message
    AUTHID_OPT_BITRATE,     // Bit rate for video encoding
    AUTHID_OPT_FRAMERATE,   // Frame rate for video encoding
    AUTHID_OPT_QSCALE,      // QScale parameter for vdeo encoding
    AUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
    AUTHID_OPT_WIDTH,       // client width
    AUTHID_OPT_HEIGHT,      // client height
    AUTHID_OPT_BPP,         // bits per planes (number of colors)
    AUTHID_DISPLAY_MESSAGE, // display a dialog box with a message
    AUTHID_ACCEPT_MESSAGE,  // display a dialog to valid a message
    AUTHID_AUTH_ERROR_MESSAGE,
    AUTHID_PROXY_TYPE,
    AUTHID_AUTHENTICATED,
    AUTHID_SELECTOR,
    AUTHID_KEEPALIVE,
    // Translation Text
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
    AUTHID_MODE_CONSOLE,
    AUTHID_VIDEO_QUALITY,
    AUTHID_TIMEZONE,
    MAX_AUTHID
} authid_t;


#define STRAUTHID_TARGET_USER      "target_login"
#define STRAUTHID_TARGET_PASSWORD  "target_password"
#define STRAUTHID_HOST             "ip_client"
#define STRAUTHID_PASSWORD         "password"
#define STRAUTHID_AUTH_USER        "login"
#define STRAUTHID_TARGET_DEVICE    "target_device"
#define STRAUTHID_TARGET_PORT      "target_port"
#define STRAUTHID_TARGET_PROTOCOL  "proto_dest"
#define STRAUTHID_END_TIME         "end_time"
#define STRAUTHID_SELECTOR_GROUP_FILTER "selector_group_filter"
#define STRAUTHID_SELECTOR_DEVICE_FILTER "selector_device_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"
#define STRAUTHID_SELECTOR_CURRENT_PAGE "selector_current_page"
#define STRAUTHID_REJECTED         "rejected"
#define STRAUTHID_OPT_MOVIE        "is_rec"
#define STRAUTHID_OPT_MOVIE_PATH   "rec_path"
#define STRAUTHID_OPT_CLIPBOARD    "clipboard"
#define STRAUTHID_OPT_DEVICEREDIRECTION "device_redirection"
#define STRAUTHID_END_DATE_CNX     "timeclose"
#define STRAUTHID_MESSAGE          "message"
#define STRAUTHID_OPT_BITRATE      "bitrate"
#define STRAUTHID_OPT_FRAMERATE    "framerate"
#define STRAUTHID_OPT_QSCALE       "qscale"
#define STRAUTHID_OPT_CODEC_ID     "codec_id"
#define STRAUTHID_OPT_WIDTH        "width"
#define STRAUTHID_OPT_HEIGHT       "height"
#define STRAUTHID_OPT_BPP          "bpp"
#define STRAUTHID_DISPLAY_MESSAGE  "display_message"
#define STRAUTHID_ACCEPT_MESSAGE   "accept_message"
#define STRAUTHID_AUTH_ERROR_MESSAGE "error_message"
#define STRAUTHID_PROXY_TYPE        "proxy_type"
#define STRAUTHID_AUTHENTICATED     "authenticated"
#define STRAUTHID_SELECTOR         "selector"
#define STRAUTHID_KEEPALIVE         "keepalive"
#define STRAUTHID_TRANS_BUTTON_OK   "trans_ok"
#define STRAUTHID_TRANS_BUTTON_CANCEL "trans_cancel"
#define STRAUTHID_TRANS_BUTTON_HELP   "trans_help"
#define STRAUTHID_TRANS_BUTTON_CLOSE  "trans_close"
#define STRAUTHID_TRANS_BUTTON_REFUSED  "trans_refused"
#define STRAUTHID_TRANS_LOGIN         "trans_login"
#define STRAUTHID_TRANS_USERNAME      "trans_username"
#define STRAUTHID_TRANS_PASSWORD      "trans_password"
#define STRAUTHID_TRANS_TARGET        "trans_target"
#define STRAUTHID_TRANS_DIAGNOSTIC    "trans_diagnostic"
#define STRAUTHID_TRANS_CONNECTION_CLOSED   "trans_connection_closed"
#define STRAUTHID_TRANS_HELP_MESSAGE  "trans_help_message"
#define STRAUTHID_MODE_CONSOLE        "mode_console"
#define STRAUTHID_VIDEO_QUALITY       "video_quality"
#define STRAUTHID_TIMEZONE            "timezone"

authid_t authid_from_string(const char * kw);

typedef enum{
    ID_LIB_UNKNOWN,
    ID_LIB_AUTH,
    ID_LIB_VNC,
    ID_LIB_MC,
    ID_LIB_RDP,
    ID_LIB_XUP
} idlib_t;

idlib_t idlib_from_string(std::string str);
bool bool_from_string(std::string str);


struct IniAccounts {
    char accountname[255];
    idlib_t idlib; // 0 = unknown, 1 = vnc, 2 = mc, 3 = rdp, 4 = xup

    bool accountdefined; // true if entry exists (name defined)

    // using a boolean is not enough. We have to manage from where to get
    // the value:
    // - from configuration file
    // - from command lines parameters of rdp client
    // - from user input in login box
    // If we get it from user input in login box we also should state from
    // where comes the initial value:
    // - from configuration file
    // - from command line parameters
    // - default as empty
    // It can also be a combination of the above like:
    // get it from configuration file and if it's empty use command line
    // ... it's getting quite complicated and obviously too complicated
    // to be managed by a poor lone boolean...
    bool askport;        // true if port should be asked interactively
    bool askusername;    // true if username should be asked interactively
    bool askip;          // true if ip should be asked interactively
    bool askpassword;    // true if password should be asked interactively

    int port;
    char username[255]; // should use string
    char password[255]; // should use string
    // do we want to allow asking ip to dns using hostname ?
    char ip[255];          // should use string
    // if remote authentication is on below is address of authentication server
    char authip[255];      // should use string
    int authport;
    int maxtick;
};

struct Inifile {
    struct Inifile_globals {
        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        int crypt_level;   // 0=low, 1=medium, 2=high
        // TODO: CGR : didn't changed it to boolean as I don't know if it shouldn't be a number of channel
        unsigned channel_code; /* 0 = no channels 1 = channels */
        bool autologin;      // true if we should bypass login box and go directly
                             // to server with credentials provided by rdp client
                             // obviously, to do so we need some target address
                             // the used solution is to provide a user name
                             // in the form user@host
                             // if autologin mode is set and that we provide
                             // an @host target value autologin we be used
                             // if we do not provide @host, we are directed to
                             // login box as usual.
        bool autologin_useauth; // the above user command line is incomplete
                                // the full form is user@host:authuser
                                // if autologin and autologin_useauth are on
                                // the authuser will be used with provided
                                // password for authentication by auth module
                                // that will return the real credential.
                                // Otherwise the proxy will try a direct
                                // connection to host with user account
                                // and provided password.
        char authip[255];
        int authport;
        unsigned authversion;
        bool nomouse;
        bool notimestamp;
        bool autovalidate;      // dialog autovalidation for test

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

        char movie_path[1024];

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
        } debug;

    } globals;

    struct IniAccounts account[6];

    Inifile(const char * filename);
    Inifile(std::istream & Inifile_stream);

    private:
        void init(std::istream & Inifile_stream);

};

#endif

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
#include <boost/program_options.hpp>

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
    bool askusername;    // true if username should be asked interactively
    bool askip;          // true if ip should be asked interactively
    bool askpassword;    // true if password should be asked interactively

    char username[255]; // should use string
    char password[255]; // should use string
    // do we want to allow asking ip to dns using hostname ?
    char ip[255];          // should use string
    // if remote authentication is on below is address of authentication server
    int maxtick;
};

struct Inifile {
    struct Inifile_globals {
        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        int encryptionLevel;   // 0=low, 1=medium, 2=high
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
        bool internal_domain;

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

    Inifile();
    Inifile(const char * filename);
    Inifile(std::istream & Inifile_stream);

    void init();
    void cparse(std::istream & Inifile_stream, bool getdefault = false);
    void cparse(const char * filename, bool getdefault = false);
    void parseline(const char * line, char * context, bool getdefault);
    void setglobal(const char * key, const char * value, const char * context, bool getdefault);


};

#endif

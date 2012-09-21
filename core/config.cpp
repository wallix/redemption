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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   configuration file,
   parsing config file rdproxy.ini

*/
#include "config.hpp"
#include "log.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

#include <boost/algorithm/string.hpp>

#include <stdio.h>

bool bool_from_string(string str)
{
    return (boost::iequals(string("1"),str))
        || (boost::iequals(string("yes"),str))
        || (boost::iequals(string("on"),str))
        || (boost::iequals(string("true"),str));
}

bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

unsigned level_from_string(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
    if (0 == strcmp("medium", str)) { res = 1; }
    else if (0 == strcmp("high", str)) { res = 2; }
    return res;
}

bool check_name(const char * str)
{
    return ((strlen(str) > 0) && (strlen(str) < 250));
}

bool check_ask(const char * str)
{
    return (0 == strcmp(str, "ask"));
}

authid_t authid_from_string(const char * kw)
{
    static const string authstr[MAX_AUTHID-1] = {
    STRAUTHID_TARGET_USER,
    STRAUTHID_TARGET_PASSWORD,
    STRAUTHID_HOST,
    STRAUTHID_PASSWORD,
    STRAUTHID_AUTH_USER,
    STRAUTHID_TARGET_DEVICE,
    STRAUTHID_TARGET_PORT,
    STRAUTHID_TARGET_PROTOCOL,
    STRAUTHID_END_TIME,
    STRAUTHID_SELECTOR_GROUP_FILTER,
    STRAUTHID_SELECTOR_DEVICE_FILTER,
    STRAUTHID_SELECTOR_LINES_PER_PAGE,
    STRAUTHID_SELECTOR_NUMBER_OF_PAGES,
    STRAUTHID_SELECTOR_CURRENT_PAGE,
    STRAUTHID_REJECTED,
    STRAUTHID_OPT_MOVIE,
    STRAUTHID_OPT_MOVIE_PATH,
    STRAUTHID_OPT_CLIPBOARD,
    STRAUTHID_OPT_DEVICEREDIRECTION,
    STRAUTHID_END_DATE_CNX,
    STRAUTHID_MESSAGE,
    STRAUTHID_OPT_BITRATE,
    STRAUTHID_OPT_FRAMERATE,
    STRAUTHID_OPT_QSCALE,
    STRAUTHID_OPT_CODEC_ID,
    STRAUTHID_OPT_WIDTH,
    STRAUTHID_OPT_HEIGHT,
    STRAUTHID_OPT_BPP,
    STRAUTHID_DISPLAY_MESSAGE,
    STRAUTHID_ACCEPT_MESSAGE,
    STRAUTHID_AUTH_ERROR_MESSAGE,
    STRAUTHID_PROXY_TYPE,
    STRAUTHID_AUTHENTICATED,
    STRAUTHID_SELECTOR,
    STRAUTHID_KEEPALIVE,
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
    STRAUTHID_MODE_CONSOLE,
    STRAUTHID_VIDEO_QUALITY,
    STRAUTHID_TIMEZONE,
    };

    string str = string(kw);
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID-1 ; i++){
        if (0 == authstr[i].compare(str)){
            res = (authid_t)(i+1);
            break;
        }
    }
    return res;
}

idlib_t idlib_from_string(string str)
{
    idlib_t res = ID_LIB_UNKNOWN;
    if ((0 == string("libvnc.so").compare(str))
        || (0 == string("vnc.dll").compare(str))
        || (0 == string("VNC").compare(str))
        || (0 == string("vnc").compare(str))){
            res = ID_LIB_VNC;
    }
    else if ((0 == string("librdp.so").compare(str))
        || (0 == string("rdp.dll").compare(str))
        || (0 == string("RDP").compare(str))
        || (0 == string("rdp").compare(str))){
            res = ID_LIB_RDP;
    }
    else if ((0 == string("libxup.so").compare(str))
        || (0 == string("XUP").compare(str))
        || (0 == string("xup.dll").compare(str))
        || (0 == string("xup").compare(str))){
            res = ID_LIB_XUP;
    }
    else if ((0 == string("libmc.so").compare(str))
        || (0 == string("mc.dll").compare(str))
        || (0 == string("MC").compare(str))
        || (0 == string("mc").compare(str))){
            res = ID_LIB_MC;
    }
    else if ((0 == string("auth").compare(str))){
            res = ID_LIB_AUTH;
    }
    return res;
}



void ask_string(const char * str, char buffer[], bool & flag)
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

Inifile::Inifile() {
    std::stringstream oss("");
    this->init();
    this->cparse(oss);
}

Inifile::Inifile(const char * filename) {
    this->init();
    this->cparse(filename);
}


Inifile::Inifile(istream & Inifile_stream) {
    this->init();
    this->cparse(Inifile_stream);
}

void Inifile::init(){
        this->globals.bitmap_cache = true;
        this->globals.bitmap_compression = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_string("low");
        this->globals.autologin = false;
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.authversion = 2;
        this->globals.autovalidate = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");

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
        strcpy(this->globals.movie_path, "/tmp/");
        this->globals.internal_domain = false;
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

        for (size_t i=0; i< 6; i++){
            this->account[i].idlib = idlib_from_string("UNKNOWN");
            this->account[i].accountdefined = false;
            strcpy(this->account[i].accountname, "");
            this->account[i].askusername = false;
            strcpy(this->account[i].username, "");
            this->account[i].askpassword = false;
            strcpy(this->account[i].password, "");
            this->account[i].askip = false;
            strcpy(this->account[i].ip, "");
        }
};

void Inifile::cparse(istream & ifs){
    
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



void Inifile::parseline(const char * line, char * context)
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
                memcpy(key, startkey, endkey - startkey + 1);
                key[endkey - startkey + 1] = 0;

                const char * startvalue = sep + 1;
                for ( ; *startvalue ; startvalue++) {
                    if (!isspace(*startvalue)){
                        break;
                    }
                }
                const char * endvalue;
                for (endvalue = startvalue; *endvalue ; endvalue++) {
                    if (isspace(*endvalue) || *endvalue == '#'){
                        break;
                    }
                }
                memcpy(value, startvalue, endvalue - startvalue + 1);
                value[endvalue - startvalue + 1] = 0;
                this->setglobal(key, value, context);
                break;
            }
        }
    }
}

void Inifile::setglobal(const char * key, const char * value, const char * context)
{
    if (0 == strcmp(context, "globals")){ 
        if (0 == strcmp(key, "bitmap_cache")){
            this->globals.bitmap_cache = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "bitmap_compression")){
            this->globals.bitmap_compression = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "port")){
            this->globals.port = atol(value);
        }
        else if (0 == strcmp(key, "nomouse")){
            this->globals.nomouse = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "notimestamp")){
            this->globals.notimestamp = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "encryptionLevel")){
            this->globals.encryptionLevel = level_from_string(value);
        }
        else if (0 == strcmp(key, "autologin")){
            this->globals.autologin = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "authip")){
            strcpy(this->globals.authip, value);
        }
        else if (0 == strcmp(key, "authport")){
            this->globals.authport = atol(value);
        }
        else if (0 == strcmp(key, "authversion")){
            this->globals.authversion = atol(value);
        }
        else if (0 == strcmp(key, "autovalidate")){
            this->globals.autovalidate = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "max_tick")){
            this->globals.max_tick    = atol(value);
        }
        else if (0 == strcmp(key, "keepalive_grace_delay")){
            this->globals.keepalive_grace_delay = atol(value);
        }
        else if (0 == strcmp(key, "internal_domain")){
            this->globals.internal_domain = bool_from_cstr(value);
        }
        else if (0 == strcmp(key, "dynamic_conf_path")){
            strcpy(this->globals.dynamic_conf_path, value);
        }
    }
    else if (0 == strcmp(context, "video")){ 
        if (0 == strcmp(key, "movie_path")){
            strcpy(this->globals.movie_path, value);
        }
        else if (0 == strcmp(key, "l_bitrate")){
            this->globals.l_bitrate   = atol(value);
        }
        else if (0 == strcmp(key, "l_framerate")){
            this->globals.l_framerate = atol(value);
        }
        else if (0 == strcmp(key, "l_height")){
            this->globals.l_height    = atol(value);
        }
        else if (0 == strcmp(key, "l_width")){
            this->globals.l_width     = atol(value);
        }
        else if (0 == strcmp(key, "l_qscale")){
            this->globals.l_qscale    = atol(value);
        }
        else if (0 == strcmp(key, "m_bitrate")){
            this->globals.m_bitrate   = atol(value);
        }
        else if (0 == strcmp(key, "m_framerate")){
            this->globals.m_framerate = atol(value);
        }
        else if (0 == strcmp(key, "m_height")){
            this->globals.m_height    = atol(value);
        }
        else if (0 == strcmp(key, "m_width")){
            this->globals.m_width     = atol(value);
        }
        else if (0 == strcmp(key, "m_qscale")){
            this->globals.m_qscale    = atol(value);
        }
        else if (0 == strcmp(key, "h_bitrate")){
            this->globals.h_bitrate   = atol(value);
        }
        else if (0 == strcmp(key, "h_framerate")){
            this->globals.h_framerate = atol(value);
        }
        else if (0 == strcmp(key, "h_height")){
            this->globals.h_height    = atol(value);
        }
        else if (0 == strcmp(key, "h_width")){
            this->globals.h_width     = atol(value);
        }
        else if (0 == strcmp(key, "h_qscale")){
            this->globals.h_qscale    = atol(value);
        }
    }
    else if (0 == strcmp(context, "debug")){ 
        if (0 == strcmp(key, "x224")){
            this->globals.debug.x224              = atol(value);
        }
        else if (0 == strcmp(key, "mcs")){
            this->globals.debug.mcs               = atol(value);
        }
        else if (0 == strcmp(key, "sec")){
            this->globals.debug.sec               = atol(value);
        }
        else if (0 == strcmp(key, "rdp")){
            this->globals.debug.rdp               = atol(value);
        }
        else if (0 == strcmp(key, "primary_orders")){
            this->globals.debug.primary_orders    = atol(value);
        }
        else if (0 == strcmp(key, "secondary_orders")){
            this->globals.debug.secondary_orders  = atol(value);
        }
        else if (0 == strcmp(key, "bitmap")){
            this->globals.debug.bitmap            = atol(value);
        }
        else if (0 == strcmp(key, "capture")){
            this->globals.debug.capture           = atol(value);
        }
        else if (0 == strcmp(key, "auth")){
            this->globals.debug.auth              = atol(value);
        }
        else if (0 == strcmp(key, "session")){
            this->globals.debug.session           = atol(value);
        }
        else if (0 == strcmp(key, "front")){
            this->globals.debug.front             = atol(value);
        }
        else if (0 == strcmp(key, "mod_rdp")){
            this->globals.debug.mod_rdp           = atol(value);
        }
        else if (0 == strcmp(key, "mod_vnc")){
            this->globals.debug.mod_vnc           = atol(value);
        }
        else if (0 == strcmp(key, "mod_int")){
            this->globals.debug.mod_int           = atol(value);
        }
        else if (0 == strcmp(key, "mod_xup")){
            this->globals.debug.mod_xup           = atol(value);
        }
        else if (0 == strcmp(key, "widget")){
            this->globals.debug.widget            = atol(value);
        }
        else if (0 == strcmp(key, "input")){
            this->globals.debug.input            = atol(value);
        }
    }
    else if (0 == strncmp("xrdp", context, 4) && context[4] >= '1' && context[4] <= '6' && context[5] == 0){
        int i = context[4] - '1';
        if (0 == strcmp(key, "lib")){
            this->account[i].idlib = idlib_from_string(value);
        }
        else if (0 == strcmp(key, "name")){
            if (strlen(value) > 0) {
                strcpy(this->account[i].accountname, value);
                this->account[i].accountdefined = true;
            }
        }
        else if (0 == strcmp(key, "username")){
            ask_string(value, this->account[i].username, this->account[i].askusername);
        }
        else if (0 == strcmp(key, "password")){
            ask_string(value, this->account[i].password, this->account[i].askpassword);
        }
        else if (0 == strcmp(key, "ip")){
            ask_string(value, this->account[i].ip, this->account[i].askip);
        }
    }
}


void Inifile::cparse(const char * filename){
    ifstream inifile(filename);
    this->cparse(inifile);
}



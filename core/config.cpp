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
   parsing config file xrdp.ini values and names helped by
   lib_boost and saved in Inifile object.

*/
#include "config.hpp"
#include "log.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
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

unsigned level_from_string(string str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
    if (0 == string("medium").compare(str)) { res = 1; }
    else if (0 == string("high").compare(str)) { res = 2; }
TODO(" should throw an exeption for unrecognised values")
    return res;
}

bool check_name(string str)
{
    return ((str.length() > 0) && (str.length() < 250));
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



unsigned channel_code_from_int(unsigned val)
{ // 0 no channels, 1 channels
    unsigned res = 1;
TODO(" should throw an exeption for illegal values")
    if (val <= 1) { res = val; }
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

Inifile::Inifile() : Inifile_desc("rdpproxy.ini configuration file:"){
    std::stringstream oss("");
    this->init();
    this->parse(oss, true);
}

Inifile::Inifile(const char * filename) : Inifile_desc("rdpproxy.ini configuration file:") {
    this->init();
    this->parse(filename, true);
}


Inifile::Inifile(istream & Inifile_stream) : Inifile_desc("rdpproxy.ini configuration file:") {
    this->init();
    this->parse(Inifile_stream, true);
}

void Inifile::init(){
TODO(" find a more generic way to read this struct with any number of account lines")
    this->Inifile_desc.add_options()
    ("globals.bitmap_cache", boost::program_options::value<string>()->default_value("yes"), "")
    ("globals.bitmap_compression", boost::program_options::value<string>()->default_value("yes"), "")
    ("globals.port", boost::program_options::value<int>(&this->globals.port)->default_value(3389), "")
    ("globals.encryptionLevel", boost::program_options::value<string>()->default_value("low"), "")
    ("globals.channel_code", boost::program_options::value<unsigned>()->default_value(1), "")
    ("globals.autologin", boost::program_options::value<string>()->default_value("no"), "")
    ("globals.authversion", boost::program_options::value<unsigned>()->default_value(2), "Version of Wallix Authentication Protocol")
    ("globals.authip", boost::program_options::value<string>()->default_value("127.0.0.1"), "")
    ("globals.authport", boost::program_options::value<int>()->default_value(3350), "")
    ("globals.nomouse", boost::program_options::value<string>()->default_value("false"), "")
    ("globals.notimestamp", boost::program_options::value<string>()->default_value("false"), "")
    ("globals.autovalidate", boost::program_options::value<string>()->default_value("false"), "")
    ("globals.l_bitrate", boost::program_options::value<int>()->default_value(20000), "")
    ("globals.l_framerate", boost::program_options::value<int>()->default_value(1), "")
    ("globals.l_height", boost::program_options::value<int>()->default_value(480), "")
    ("globals.l_width", boost::program_options::value<int>()->default_value(640), "")
    ("globals.l_qscale", boost::program_options::value<int>()->default_value(25), "")
    ("globals.m_bitrate", boost::program_options::value<int>()->default_value(40000), "")
    ("globals.m_framerate", boost::program_options::value<int>()->default_value(1), "")
    ("globals.m_height", boost::program_options::value<int>()->default_value(768), "")
    ("globals.m_width", boost::program_options::value<int>()->default_value(1024), "")
    ("globals.m_qscale", boost::program_options::value<int>()->default_value(15), "")
    ("globals.h_bitrate", boost::program_options::value<int>()->default_value(200000), "")
    ("globals.h_framerate", boost::program_options::value<int>()->default_value(5), "")
    ("globals.h_height", boost::program_options::value<int>()->default_value(1024), "")
    ("globals.h_width", boost::program_options::value<int>()->default_value(1280), "")
    ("globals.h_qscale", boost::program_options::value<int>()->default_value(15), "")

    ("globals.max_tick", boost::program_options::value<int>()->default_value(30), "")
    ("globals.keepalive_grace_delay", boost::program_options::value<int>()->default_value(30), "")

    ("globals.movie_path", boost::program_options::value<string>()->default_value("/tmp/"), "")
    ("globals.internal_domain", boost::program_options::value<string>()->default_value("no"), "")

    ("globals.debug_x224", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mcs", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_sec", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_rdp", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_primary_orders", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_secondary_orders", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_bitmap", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_capture", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_auth", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_session", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_front", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_rdp", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_vnc", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_int", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_xup", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_widget", boost::program_options::value<uint32_t>()->default_value(0), "")
    ("globals.debug_input", boost::program_options::value<uint32_t>()->default_value(0), "")

    ("xrdp1.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp1.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp1.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp1.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp1.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp1.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp1.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp1.authport", boost::program_options::value<int>()->default_value(3350), "")

    ("xrdp2.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp2.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp2.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp2.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp2.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp2.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp2.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp2.authport", boost::program_options::value<int>()->default_value(3350), "")

    ("xrdp3.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp3.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp3.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp3.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp3.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp3.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp3.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp3.authport", boost::program_options::value<int>()->default_value(3350), "")

    ("xrdp4.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp4.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp4.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp4.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp4.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp4.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp4.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp4.authport", boost::program_options::value<int>()->default_value(3350), "")

    ("xrdp5.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp5.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp5.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp5.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp5.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp5.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp5.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp5.authport", boost::program_options::value<int>()->default_value(3350), "")

    ("xrdp6.name", boost::program_options::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp6.lib", boost::program_options::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp6.port", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp6.username", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp6.password", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp6.ip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp6.authip", boost::program_options::value<string>()->default_value(""), "")
    ("xrdp6.authport", boost::program_options::value<int>()->default_value(3350), "")
    ;
};

void Inifile::parse(istream & Inifile_stream, bool getdefault){

    try{
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_config_file(Inifile_stream, this->Inifile_desc), vm);
        boost::program_options::notify(vm);

        if (getdefault||vm.count("globals.bitmap_cache")){
            LOG(LOG_INFO, "bitmap_cache count = %u", vm.count("globals.bitmap_cache"));
            this->globals.bitmap_cache =
                bool_from_string(vm["globals.bitmap_cache"].as<string>());
        }

        if (getdefault||vm.count("globals.nomouse")){
            this->globals.nomouse =
                bool_from_string(vm["globals.nomouse"].as<string>());
        }
        if (getdefault||vm.count("globals.notimestamp")){
            this->globals.notimestamp =
                bool_from_string(vm["globals.notimestamp"].as<string>());
        }
        if (getdefault||vm.count("globals.bitmap_compression")){
            this->globals.bitmap_compression =
                bool_from_string(vm["globals.bitmap_compression"].as<string>());
        }

        if (getdefault||vm.count("globals.encryptionLevel")){
            this->globals.encryptionLevel =
                level_from_string(vm["globals.encryptionLevel"].as<string>());
        }
        if (getdefault||vm.count("globals.channel_code")){
            this->globals.channel_code =
                channel_code_from_int(vm["globals.channel_code"].as<unsigned>());
        }

        if (getdefault||vm.count("globals.autologin")){
            LOG(LOG_INFO, "autologin count = %u", vm.count("globals.autologin"));
            this->globals.autologin =
                bool_from_string(vm["globals.autologin"].as<string>());
        }

        if (getdefault||vm.count("globals.authip")){
            strncpy(this->globals.authip, vm["globals.authip"].as<string>().data(), 255);
        }
        if (getdefault||vm.count("globals.authport")){
            this->globals.authport = vm["globals.authport"].as<int>();
        }

        if (getdefault||vm.count("globals.authversion")){
            this->globals.authversion = vm["globals.authversion"].as<unsigned>();
        }    
        if (getdefault||vm.count("globals.autovalidate")){
            this->globals.autovalidate =
                bool_from_string(vm["globals.autovalidate"].as<string>());
        }
        if (getdefault||vm.count("globals.l_bitrate")){
            this->globals.l_bitrate   = vm["globals.l_bitrate"].as<int>();
        }

        if (getdefault||vm.count("globals.l_framerate")){
             this->globals.l_framerate = vm["globals.l_framerate"].as<int>();
        }
        if (getdefault||vm.count("globals.l_height")){
            this->globals.l_height    = vm["globals.l_height"].as<int>();
        }
        if (getdefault||vm.count("globals.l_width")){
            this->globals.l_width     = vm["globals.l_width"].as<int>();
        }
        if (getdefault||vm.count("globals.l_qscale")){
            this->globals.l_qscale    = vm["globals.l_qscale"].as<int>();
        }
        if (getdefault||vm.count("globals.m_bitrate")){
            this->globals.m_bitrate   = vm["globals.m_bitrate"].as<int>();
        }
        if (getdefault||vm.count("globals.m_framerate")){
            this->globals.m_framerate = vm["globals.m_framerate"].as<int>();
        }
        if (getdefault||vm.count("globals.m_height")){
            this->globals.m_height    = vm["globals.m_height"].as<int>();
        }

        if (getdefault||vm.count("globals.m_width")){
            this->globals.m_width     = vm["globals.m_width"].as<int>();
        }
        if (getdefault||vm.count("globals.m_qscale")){
            this->globals.m_qscale    = vm["globals.m_qscale"].as<int>();
        }
        if (getdefault||vm.count("globals.h_bitrate")){
            this->globals.h_bitrate   = vm["globals.h_bitrate"].as<int>();
        }
        if (getdefault||vm.count("globals.h_framerate")){
            this->globals.h_framerate = vm["globals.h_framerate"].as<int>();
        }
        if (getdefault||vm.count("globals.h_height")){
            this->globals.h_height    = vm["globals.h_height"].as<int>();
        }
        if (getdefault||vm.count("globals.h_width")){
            this->globals.h_width     = vm["globals.h_width"].as<int>();
        }
        if (getdefault||vm.count("globals.h_qscale")){
            this->globals.h_qscale    = vm["globals.h_qscale"].as<int>();
        }

        if (getdefault||vm.count("globals.max_tick")){
            this->globals.max_tick              = vm["globals.max_tick"].as<int>();
        }
        if (getdefault||vm.count("globals.keepalive_grace_delay")){
            this->globals.keepalive_grace_delay = vm["globals.keepalive_grace_delay"].as<int>();
        }

        if (getdefault||vm.count("globals.movie_path")){
            strncpy(this->globals.movie_path, vm["globals.movie_path"].as<string>().data(), vm["globals.movie_path"].as<string>().length());
            this->globals.movie_path[vm["globals.movie_path"].as<string>().length()] = 0;
        }

        if (getdefault||vm.count("globals.internal_domain")){
            this->globals.internal_domain =
                bool_from_string(vm["globals.internal_domain"].as<string>());
        }


        if (getdefault||vm.count("globals.debug_x224")){
            this->globals.debug.x224              = vm["globals.debug_x224"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_mcs")){
            this->globals.debug.mcs               = vm["globals.debug_mcs"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_sec")){
            this->globals.debug.sec               = vm["globals.debug_sec"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_rdp")){
            this->globals.debug.rdp               = vm["globals.debug_rdp"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_primary_orders")){
            this->globals.debug.primary_orders    = vm["globals.debug_primary_orders"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_secondary_orders")){
            this->globals.debug.secondary_orders  = vm["globals.debug_secondary_orders"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_bitmap")){
            this->globals.debug.bitmap            = vm["globals.debug_bitmap"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_capture")){
            this->globals.debug.capture           = vm["globals.debug_capture"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_auth")){
            this->globals.debug.auth              = vm["globals.debug_auth"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_session")){
            this->globals.debug.session           = vm["globals.debug_session"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_front")){
            this->globals.debug.front             = vm["globals.debug_front"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_mod_rdp")){
            this->globals.debug.mod_rdp           = vm["globals.debug_mod_rdp"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_mod_vnc")){
            this->globals.debug.mod_vnc           = vm["globals.debug_mod_vnc"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_mod_int")){
            this->globals.debug.mod_int           = vm["globals.debug_mod_int"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_mod_xup")){
            this->globals.debug.mod_xup           = vm["globals.debug_mod_xup"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_widget")){
            this->globals.debug.widget            = vm["globals.debug_widget"].as<uint32_t>();
        }
        if (getdefault||vm.count("globals.debug_input")){
            this->globals.debug.input             = vm["globals.debug_input"].as<uint32_t>();
        }

        string keylib("xrdp1.lib");
        string keyname("xrdp1.name");
        string keyusername("xrdp1.username");
        string keypassword("xrdp1.password");
        string keyport("xrdp1.port");
        string keyip("xrdp1.ip");
        string keyauthport("xrdp1.authport");
        string keyauthip("xrdp1.authip");

        for (unsigned account_num = 0; account_num < 6; account_num++){

            keylib[4] = '1' + account_num;
            if (getdefault||vm.count(keylib)){
                this->account[account_num].idlib =
                    idlib_from_string(vm[keylib].as<string>());
            }

            keyname[4] = '1' + account_num;
            if (getdefault||vm.count(keyname)){
                string account = vm[keyname].as<string>();
                this->account[account_num].accountdefined = check_name(account);
                if (this->account[account_num].accountdefined){
                    strncpy(this->account[account_num].accountname, account.data(), account.length());
                    this->account[account_num].accountname[account.length()] = 0;
                }
                else {
                    this->account[account_num].accountname[0] = 0;
                }
            }

            keyport[4] = '1' + account_num;
            if (getdefault||vm.count(keyport)){
                int int_port;
                if (std::istringstream(vm[keyport].as<string>()) >> int_port){
                    this->account[account_num].port = int_port;
                }
                else {
                    switch (this->account[account_num].idlib){
                    case ID_LIB_RDP:
                        this->account[account_num].port = 3389;
                    break;
                    case ID_LIB_VNC:
                        this->account[account_num].port = 5900;
                    break;
                    case ID_LIB_XUP:
                        this->account[account_num].port = 6200;
                    break;
                    default:
                        this->account[account_num].port = 0;
                    break;
                    }
                }
            }

            keyauthport[4] = '1' + account_num;
            if (getdefault||vm.count(keyauthport)){
                this->account[account_num].authport = vm[keyauthport].as<int>();
            }

            keyusername[4] = '1' + account_num;
            if (getdefault||vm.count(keyusername)){
                ask_string(vm[keyusername].as<string>().c_str(),
                    this->account[account_num].username,
                    this->account[account_num].askusername);
            }

            keypassword[4] = '1' + account_num;
            if (getdefault||vm.count(keypassword)){
                ask_string(vm[keypassword].as<string>().c_str(),
                    this->account[account_num].password,
                    this->account[account_num].askpassword);
            }

            keyip[4] = '1' + account_num;
            if (getdefault||vm.count(keyip)){
                ask_string(vm[keyip].as<string>().c_str(),
                    this->account[account_num].ip,
                    this->account[account_num].askip);
            }

            keyauthip[4] = '1' + account_num;
            if (getdefault||vm.count(keyauthip)){
                string str = vm[keyauthip].as<string>();
                strncpy(this->account[account_num].authip, str.data(), str.length());
                this->account[account_num].authip[str.length()] = 0;
            }
        }
    } catch (exception& e){
TODO(" TODO some cleaner syntax error management. I could define a testconf target in command line and show errors. Catching all errors at once would also be handy.")
        clog << "Exception raised reading config, check configuration file :" << e.what();
        _exit(1);
    };
}

void Inifile::parse(const char * filename, bool getdefault){
    ifstream inifile(filename);
    this->parse(inifile, getdefault);
}




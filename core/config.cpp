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
namespace po = boost::program_options;

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
#warning should throw an exeption for unrecognised values
    return res;
}

bool check_name(string str)
{
    return ((str.length() > 0) && (str.length() < 250));
}

bool check_ask(string str)
{
    return (0 == str.compare("ask"));
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
#warning should throw an exeption for illegal values
    if (val <= 1) { res = val; }
    return res;
}


void ask_string(po::variables_map & vm, string & key, char buffer[], bool & flag)
{
    string str = vm[key].as<string>();
    flag = check_ask(str);
    if (!flag){
        strncpy(buffer, str.data(), str.length());
        buffer[str.length()] = 0;
    }
    else {
        buffer[0] = 0;
    }
}


Inifile::Inifile(const char * filename) {
    ifstream inifile(filename);
    this->init(inifile);
}


Inifile::Inifile(istream & Inifile_stream){
    this->init(Inifile_stream);
}


void Inifile::init(istream & Inifile_stream){
#warning find a more generic way to read this struct with any number of account lines
    po::options_description Inifile_desc("Xrdp.ini configuration file:");
    Inifile_desc.add_options()
    ("globals.bitmap_cache", po::value<string>()->default_value("yes"), "")
    ("globals.bitmap_compression", po::value<string>()->default_value("yes"), "")
    ("globals.port", po::value<int>(&this->globals.port)->default_value(3389), "")
    ("globals.crypt_level", po::value<string>()->default_value("low"), "")
    ("globals.channel_code", po::value<unsigned>()->default_value(1), "")
    ("globals.autologin", po::value<string>()->default_value("no"), "")
    ("globals.authversion", po::value<unsigned>()->default_value(2), "Version of Wallix Authentication Protocol")
    ("globals.authip", po::value<string>()->default_value("127.0.0.1"), "")
    ("globals.authport", po::value<int>()->default_value(3350), "")
    ("globals.nomouse", po::value<string>()->default_value("false"), "")
    ("globals.notimestamp", po::value<string>()->default_value("false"), "")
    ("globals.autovalidate", po::value<string>()->default_value("false"), "")
    ("globals.l_bitrate", po::value<int>()->default_value(20000), "")
    ("globals.l_framerate", po::value<int>()->default_value(1), "")
    ("globals.l_height", po::value<int>()->default_value(480), "")
    ("globals.l_width", po::value<int>()->default_value(640), "")
    ("globals.l_qscale", po::value<int>()->default_value(25), "")
    ("globals.m_bitrate", po::value<int>()->default_value(40000), "")
    ("globals.m_framerate", po::value<int>()->default_value(1), "")
    ("globals.m_height", po::value<int>()->default_value(768), "")
    ("globals.m_width", po::value<int>()->default_value(1024), "")
    ("globals.m_qscale", po::value<int>()->default_value(15), "")
    ("globals.h_bitrate", po::value<int>()->default_value(200000), "")
    ("globals.h_framerate", po::value<int>()->default_value(5), "")
    ("globals.h_height", po::value<int>()->default_value(1024), "")
    ("globals.h_width", po::value<int>()->default_value(1280), "")
    ("globals.h_qscale", po::value<int>()->default_value(15), "")

    ("globals.debug_x224", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mcs", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_sec", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_rdp", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_primary_orders", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_secondary_orders", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_bitmap", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_capture", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_auth", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_session", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_rdp", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_vnc", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_int", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_mod_xup", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_widget", po::value<uint32_t>()->default_value(0), "")
    ("globals.debug_input", po::value<uint32_t>()->default_value(0), "")

    ("xrdp1.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp1.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp1.port", po::value<string>()->default_value(""), "")
    ("xrdp1.username", po::value<string>()->default_value(""), "")
    ("xrdp1.password", po::value<string>()->default_value(""), "")
    ("xrdp1.ip", po::value<string>()->default_value(""), "")
    ("xrdp1.authip", po::value<string>()->default_value(""), "")
    ("xrdp1.authport", po::value<int>()->default_value(3350), "")

    ("xrdp2.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp2.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp2.port", po::value<string>()->default_value(""), "")
    ("xrdp2.username", po::value<string>()->default_value(""), "")
    ("xrdp2.password", po::value<string>()->default_value(""), "")
    ("xrdp2.ip", po::value<string>()->default_value(""), "")
    ("xrdp2.authip", po::value<string>()->default_value(""), "")
    ("xrdp2.authport", po::value<int>()->default_value(3350), "")

    ("xrdp3.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp3.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp3.port", po::value<string>()->default_value(""), "")
    ("xrdp3.username", po::value<string>()->default_value(""), "")
    ("xrdp3.password", po::value<string>()->default_value(""), "")
    ("xrdp3.ip", po::value<string>()->default_value(""), "")
    ("xrdp3.authip", po::value<string>()->default_value(""), "")
    ("xrdp3.authport", po::value<int>()->default_value(3350), "")

    ("xrdp4.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp4.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp4.port", po::value<string>()->default_value(""), "")
    ("xrdp4.username", po::value<string>()->default_value(""), "")
    ("xrdp4.password", po::value<string>()->default_value(""), "")
    ("xrdp4.ip", po::value<string>()->default_value(""), "")
    ("xrdp4.authip", po::value<string>()->default_value(""), "")
    ("xrdp4.authport", po::value<int>()->default_value(3350), "")

    ("xrdp5.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp5.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp5.port", po::value<string>()->default_value(""), "")
    ("xrdp5.username", po::value<string>()->default_value(""), "")
    ("xrdp5.password", po::value<string>()->default_value(""), "")
    ("xrdp5.ip", po::value<string>()->default_value(""), "")
    ("xrdp5.authip", po::value<string>()->default_value(""), "")
    ("xrdp5.authport", po::value<int>()->default_value(3350), "")

    ("xrdp6.name", po::value<string>()->default_value(""), "Entry name in the account select box")
    ("xrdp6.lib", po::value<string>()->default_value("unknown"), "back-end module : vnc, rdp, xup, mc")
    ("xrdp6.port", po::value<string>()->default_value(""), "")
    ("xrdp6.username", po::value<string>()->default_value(""), "")
    ("xrdp6.password", po::value<string>()->default_value(""), "")
    ("xrdp6.ip", po::value<string>()->default_value(""), "")
    ("xrdp6.authip", po::value<string>()->default_value(""), "")
    ("xrdp6.authport", po::value<int>()->default_value(3350), "")
    ;

    try{
        po::variables_map vm;
        po::store(po::parse_config_file(Inifile_stream, Inifile_desc), vm);
        po::notify(vm);

        this->globals.bitmap_cache =
            bool_from_string(vm["globals.bitmap_cache"].as<string>());
        this->globals.nomouse =
            bool_from_string(vm["globals.nomouse"].as<string>());
        this->globals.notimestamp =
            bool_from_string(vm["globals.notimestamp"].as<string>());
        this->globals.bitmap_compression =
            bool_from_string(vm["globals.bitmap_compression"].as<string>());
        this->globals.crypt_level =
            level_from_string(vm["globals.crypt_level"].as<string>());
        this->globals.channel_code =
            channel_code_from_int(vm["globals.channel_code"].as<unsigned>());
        this->globals.autologin =
            bool_from_string(vm["globals.autologin"].as<string>());
        strncpy(this->globals.authip, vm["globals.authip"].as<string>().data(), 255);
        this->globals.authport = vm["globals.authport"].as<int>();
        this->globals.authversion = vm["globals.authversion"].as<unsigned>();
        this->globals.autovalidate =
            bool_from_string(vm["globals.autovalidate"].as<string>());
        this->globals.l_bitrate   = vm["globals.l_bitrate"].as<int>();
        this->globals.l_framerate = vm["globals.l_framerate"].as<int>();
        this->globals.l_height    = vm["globals.l_height"].as<int>();
        this->globals.l_width     = vm["globals.l_width"].as<int>();
        this->globals.l_qscale    = vm["globals.l_qscale"].as<int>();
        this->globals.m_bitrate   = vm["globals.m_bitrate"].as<int>();
        this->globals.m_framerate = vm["globals.m_framerate"].as<int>();
        this->globals.m_height    = vm["globals.m_height"].as<int>();
        this->globals.m_width     = vm["globals.m_width"].as<int>();
        this->globals.m_qscale    = vm["globals.m_qscale"].as<int>();
        this->globals.h_bitrate   = vm["globals.h_bitrate"].as<int>();
        this->globals.h_framerate = vm["globals.h_framerate"].as<int>();
        this->globals.h_height    = vm["globals.h_height"].as<int>();
        this->globals.h_width     = vm["globals.h_width"].as<int>();
        this->globals.h_qscale    = vm["globals.h_qscale"].as<int>();

        this->globals.debug.x224              = vm["globals.debug_x224"].as<uint32_t>();
        this->globals.debug.mcs               = vm["globals.debug_mcs"].as<uint32_t>();
        this->globals.debug.sec               = vm["globals.debug_sec"].as<uint32_t>();
        this->globals.debug.rdp               = vm["globals.debug_rdp"].as<uint32_t>();
        this->globals.debug.primary_orders    = vm["globals.debug_primary_orders"].as<uint32_t>();
        this->globals.debug.secondary_orders  = vm["globals.debug_secondary_orders"].as<uint32_t>();
        this->globals.debug.bitmap            = vm["globals.debug_bitmap"].as<uint32_t>();
        this->globals.debug.capture           = vm["globals.debug_capture"].as<uint32_t>();
        this->globals.debug.auth              = vm["globals.debug_auth"].as<uint32_t>();
        this->globals.debug.session           = vm["globals.debug_session"].as<uint32_t>();
        this->globals.debug.mod_rdp           = vm["globals.debug_mod_rdp"].as<uint32_t>();
        this->globals.debug.mod_vnc           = vm["globals.debug_mod_vnc"].as<uint32_t>();
        this->globals.debug.mod_int           = vm["globals.debug_mod_int"].as<uint32_t>();
        this->globals.debug.mod_xup           = vm["globals.debug_mod_xup"].as<uint32_t>();
        this->globals.debug.widget            = vm["globals.debug_widget"].as<uint32_t>();
        this->globals.debug.input             = vm["globals.debug_input"].as<uint32_t>();

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
            this->account[account_num].idlib =
                idlib_from_string(vm[keylib].as<string>());

            keyname[4] = '1' + account_num;
            string account = vm[keyname].as<string>();
            this->account[account_num].accountdefined = check_name(account);
            if (this->account[account_num].accountdefined){
                strncpy(this->account[account_num].accountname, account.data(), account.length());
                this->account[account_num].accountname[account.length()] = 0;
            }
            else {
                this->account[account_num].accountname[0] = 0;
            }

            keyport[4] = '1' + account_num;
            int int_port;
            if (istringstream(vm[keyport].as<string>()) >> int_port){
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

            keyauthport[4] = '1' + account_num;
            this->account[account_num].authport = vm[keyauthport].as<int>();

            keyusername[4] = '1' + account_num;
            ask_string(vm, keyusername,
                this->account[account_num].username,
                this->account[account_num].askusername);

            keypassword[4] = '1' + account_num;
            ask_string(vm, keypassword,
                this->account[account_num].password,
                this->account[account_num].askpassword);

            keyip[4] = '1' + account_num;
            ask_string(vm, keyip,
                this->account[account_num].ip,
                this->account[account_num].askip);

            keyauthip[4] = '1' + account_num;
            string str = vm[keyauthip].as<string>();
            strncpy(this->account[account_num].authip, str.data(), str.length());
            this->account[account_num].authip[str.length()] = 0;

    #warning TODO ask port, see what it really means, seems not to be about asking to user
            this->account[account_num].askport = false;
        }
    } catch (exception& e){
#warning TODO some cleaner syntax error management. I could define a testconf target in command line and show errors. Catching all errors at once would also be handy.
        clog << "Exception raised reading config, check configuration file :" << e.what();
        _exit(1);
    };
}

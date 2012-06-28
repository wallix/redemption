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

   Unit test to config.cpp file
   Using lib boost functions, some tests need to be added

*/
#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfig
#include <boost/test/auto_unit_test.hpp>

#include "config.hpp"
#include <sstream>
#include <string>


BOOST_AUTO_TEST_CASE(TestConfigFromFile)
{
    // test we can read from a file (and not only from a stream)
    Inifile ini(FIXTURES_PATH "/rdpproxy.ini");
}


BOOST_AUTO_TEST_CASE(TestConfigDefault)
{
    // test we can read a config file with a global section
    std::stringstream oss("");

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(false, ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false, ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389, ini.globals.port);
    BOOST_CHECK_EQUAL(0,    ini.globals.crypt_level);
    BOOST_CHECK_EQUAL(1,    ini.globals.channel_code);
    BOOST_CHECK_EQUAL(0,    ini.globals.autologin);
    BOOST_CHECK_EQUAL(2,    ini.globals.authversion);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"), std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350, ini.globals.authport);

    BOOST_CHECK_EQUAL(ID_LIB_UNKNOWN, ini.account[0].idlib);

    BOOST_CHECK_EQUAL(false, ini.account[0].accountdefined);
    BOOST_CHECK_EQUAL(0,     ini.account[0].accountname[0]);
    BOOST_CHECK_EQUAL(false, ini.account[0].askport);
    BOOST_CHECK_EQUAL(0,     ini.account[0].port);
    BOOST_CHECK_EQUAL(false, ini.account[0].askusername);
    BOOST_CHECK_EQUAL(0,     ini.account[0].username[0]);
    BOOST_CHECK_EQUAL(false, ini.account[0].askpassword);
    BOOST_CHECK_EQUAL(0,     ini.account[0].password[0]);
    BOOST_CHECK_EQUAL(false, ini.account[0].askip);
    BOOST_CHECK_EQUAL(0,     ini.account[0].ip[0]);
    BOOST_CHECK_EQUAL(0,     ini.account[0].authip[0]);
    BOOST_CHECK_EQUAL(3350,  ini.account[0].authport);

}

BOOST_AUTO_TEST_CASE(TestConfig1)
{
    // test we can read a config file with a global section
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=yes\n"
    "bitmap_compression=true\n"
    "port=3390\n"
    "crypt_level=low\n"
    "channel_code=1\n"
    "\n"
    "[xrdp1]\n"
    "lib=libvnc.so\n"
    "name=config1\n"
    "username=myname\n"
    "password=secret\n"
    "ip=127.0.0.1\n"
    "port=5900\n"
    );

    Inifile ini(oss);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3390, ini.globals.port);
    BOOST_CHECK_EQUAL(1,    ini.globals.channel_code);

    struct IniAccounts & acc = ini.account[0];

    BOOST_CHECK_EQUAL(ID_LIB_VNC, acc.idlib);
    BOOST_CHECK_EQUAL(std::string("config1"), std::string(acc.accountname));
    BOOST_CHECK_EQUAL(true, acc.accountdefined);
    BOOST_CHECK_EQUAL(false, acc.askport);
    BOOST_CHECK_EQUAL(5900,  acc.port);
    BOOST_CHECK_EQUAL(false, acc.askusername);
    BOOST_CHECK_EQUAL(std::string("myname"), std::string(acc.username));
    BOOST_CHECK_EQUAL(false, acc.askpassword);
    BOOST_CHECK_EQUAL(std::string("secret"), std::string(acc.password));
    BOOST_CHECK_EQUAL(false, acc.askip);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"), std::string(acc.ip));
}

BOOST_AUTO_TEST_CASE(TestConfig1bis)
{
    // test we can read a config file with a global section
    // alternative ways to say yes in file, other values
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=true\n"
    "bitmap_compression=on\n"
    "crypt_level=medium\n"
    "\n"
    "[xrdp1]\n"
    "lib=libxup.so\n"
    "name=config2\n"
    "username=ask\n"
    "password=ask\n"
    "ip=ask\n"
    "port=5900\n"

    );

    Inifile ini(oss);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(1,    ini.globals.crypt_level);

    struct IniAccounts & acc = ini.account[0];

    BOOST_CHECK_EQUAL(ID_LIB_XUP,  acc.idlib);
    BOOST_CHECK_EQUAL(true, acc.askusername);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.username));
    BOOST_CHECK_EQUAL(true, acc.askpassword);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.password));
    BOOST_CHECK_EQUAL(true, acc.askip);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.ip));

}

BOOST_AUTO_TEST_CASE(TestConfig2)
{
    // test we can read a config file with a global section, other values
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=no\n"
    "bitmap_compression=false\n"
    "crypt_level=high\n"
    "channel_code=0\n"
    "\n"
    "[xrdp1]\n"
    "lib=auth\n"
    );

    Inifile ini(oss);
    BOOST_CHECK_EQUAL(false, ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(false, ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(2, ini.globals.crypt_level);
    BOOST_CHECK_EQUAL(0, ini.globals.channel_code);

    struct IniAccounts & acc = ini.account[0];

    BOOST_CHECK_EQUAL(ID_LIB_AUTH, acc.idlib);
}


BOOST_AUTO_TEST_CASE(TestMultiple)
{
    // test we can read a config file with a global section
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=yes\n"
    "bitmap_compression=TRuE\n"
    "port=3390\n"
    "crypt_level=low\n"
    "channel_code=1\n"
    "\n"
    "[xrdp1]\n"
    "lib=libvnc.so\n"
    "name=config1\n"
    "username=myname\n"
    "password=secret\n"
    "ip=127.0.0.1\n"
    "port=5900\n"
    "\n"
    "[xrdp2]\n"
    "lib=libxup.so\n"
    "name=config2\n"
    "username=ask\n"
    "password=ask\n"
    "ip=ask\n"
    "port=5900\n"
    "\n"
    "[xrdp3]\n"
    "lib=auth\n"
    "name=config3\n"
    "username=ask\n"
    "password=ask\n"
    "ip=127.0.0.1\n"
    "port=5900\n"
    "authip=127.0.0.1\n"
    "authport=3350\n"
    "\n"
    "[xrdp6]\n"
    "lib=auth\n"
    "name=config4\n"
    );

    Inifile ini(oss);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true, ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3390, ini.globals.port);
    BOOST_CHECK_EQUAL(1,    ini.globals.channel_code);

    struct IniAccounts & acc = ini.account[0];

    BOOST_CHECK_EQUAL(ID_LIB_VNC,  acc.idlib);
    BOOST_CHECK_EQUAL(std::string("config1"), std::string(acc.accountname));
    BOOST_CHECK_EQUAL(true, acc.accountdefined);
    BOOST_CHECK_EQUAL(false, acc.askport);
    BOOST_CHECK_EQUAL(5900,  acc.port);
    BOOST_CHECK_EQUAL(false, acc.askusername);
    BOOST_CHECK_EQUAL(std::string("myname"), std::string(acc.username));
    BOOST_CHECK_EQUAL(false, ini.account[0].askpassword);
    BOOST_CHECK_EQUAL(std::string("secret"), std::string(acc.password));
    BOOST_CHECK_EQUAL(false, acc.askip);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"), std::string(acc.ip));

    acc = ini.account[1];

    BOOST_CHECK_EQUAL(ID_LIB_XUP,  acc.idlib);
    BOOST_CHECK_EQUAL(std::string("config2"), std::string(acc.accountname));
    BOOST_CHECK_EQUAL(true, acc.accountdefined);
    BOOST_CHECK_EQUAL(true, acc.askusername);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.username));
    BOOST_CHECK_EQUAL(true, acc.askpassword);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.password));
    BOOST_CHECK_EQUAL(true, acc.askip);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.ip));

    acc = ini.account[2];

    BOOST_CHECK_EQUAL(ID_LIB_AUTH,  acc.idlib);
    BOOST_CHECK_EQUAL(std::string("config3"), std::string(acc.accountname));
    BOOST_CHECK_EQUAL(true, acc.accountdefined);
    BOOST_CHECK_EQUAL(true, acc.askusername);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.username));
    BOOST_CHECK_EQUAL(true, acc.askpassword);
    BOOST_CHECK_EQUAL(std::string(""), std::string(acc.password));
    BOOST_CHECK_EQUAL(false, acc.askip);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"), std::string(acc.ip));
    BOOST_CHECK_EQUAL(3350,  acc.authport);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"), std::string(acc.authip));

    acc = ini.account[5];

    BOOST_CHECK_EQUAL(ID_LIB_AUTH, acc.idlib);
    BOOST_CHECK_EQUAL(std::string("config4"), std::string(acc.accountname));
    BOOST_CHECK_EQUAL(true, acc.accountdefined);

}

BOOST_AUTO_TEST_CASE(TestAuthentificationKeywordRecognition)
{
    BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("unknown"));
    BOOST_CHECK_EQUAL(AUTHID_TARGET_USER, authid_from_string(STRAUTHID_TARGET_USER));
    BOOST_CHECK_EQUAL(AUTHID_TARGET_PASSWORD, authid_from_string(STRAUTHID_TARGET_PASSWORD));
    BOOST_CHECK_EQUAL(AUTHID_HOST, authid_from_string(STRAUTHID_HOST));
    BOOST_CHECK_EQUAL(AUTHID_PASSWORD, authid_from_string(STRAUTHID_PASSWORD));
    BOOST_CHECK_EQUAL(AUTHID_AUTH_USER, authid_from_string(STRAUTHID_AUTH_USER));
    BOOST_CHECK_EQUAL(AUTHID_TARGET_DEVICE, authid_from_string(STRAUTHID_TARGET_DEVICE));
    BOOST_CHECK_EQUAL(AUTHID_TARGET_PORT, authid_from_string(STRAUTHID_TARGET_PORT));
    BOOST_CHECK_EQUAL(AUTHID_TARGET_PROTOCOL, authid_from_string(STRAUTHID_TARGET_PROTOCOL));
    BOOST_CHECK_EQUAL(AUTHID_REJECTED, authid_from_string(STRAUTHID_REJECTED));
    BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE, authid_from_string(STRAUTHID_OPT_MOVIE));
    BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE_PATH, authid_from_string(STRAUTHID_OPT_MOVIE_PATH));
    BOOST_CHECK_EQUAL(AUTHID_OPT_CLIPBOARD, authid_from_string(STRAUTHID_OPT_CLIPBOARD));
    BOOST_CHECK_EQUAL(AUTHID_OPT_DEVICEREDIRECTION, authid_from_string(STRAUTHID_OPT_DEVICEREDIRECTION));
    BOOST_CHECK_EQUAL(AUTHID_END_DATE_CNX, authid_from_string(STRAUTHID_END_DATE_CNX));
    BOOST_CHECK_EQUAL(AUTHID_MESSAGE, authid_from_string(STRAUTHID_MESSAGE));
    BOOST_CHECK_EQUAL(AUTHID_OPT_BITRATE, authid_from_string(STRAUTHID_OPT_BITRATE));
    BOOST_CHECK_EQUAL(AUTHID_OPT_FRAMERATE, authid_from_string(STRAUTHID_OPT_FRAMERATE));
    BOOST_CHECK_EQUAL(AUTHID_OPT_QSCALE, authid_from_string(STRAUTHID_OPT_QSCALE));
    BOOST_CHECK_EQUAL(AUTHID_OPT_CODEC_ID, authid_from_string(STRAUTHID_OPT_CODEC_ID));
    BOOST_CHECK_EQUAL(AUTHID_OPT_WIDTH, authid_from_string(STRAUTHID_OPT_WIDTH));
    BOOST_CHECK_EQUAL(AUTHID_OPT_HEIGHT, authid_from_string(STRAUTHID_OPT_HEIGHT));
    BOOST_CHECK_EQUAL(AUTHID_OPT_BPP, authid_from_string(STRAUTHID_OPT_BPP));
    BOOST_CHECK_EQUAL(AUTHID_PROXY_TYPE, authid_from_string(STRAUTHID_PROXY_TYPE));
    BOOST_CHECK_EQUAL(AUTHID_AUTHENTICATED, authid_from_string(STRAUTHID_AUTHENTICATED));
    BOOST_CHECK_EQUAL(AUTHID_SELECTOR, authid_from_string(STRAUTHID_SELECTOR));
    BOOST_CHECK_EQUAL(AUTHID_KEEPALIVE, authid_from_string(STRAUTHID_KEEPALIVE));
    BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("8899676"));
    BOOST_CHECK_EQUAL(AUTHID_DISPLAY_MESSAGE, authid_from_string(STRAUTHID_DISPLAY_MESSAGE));
    BOOST_CHECK_EQUAL(AUTHID_ACCEPT_MESSAGE, authid_from_string(STRAUTHID_ACCEPT_MESSAGE));
    BOOST_CHECK_EQUAL(AUTHID_AUTH_ERROR_MESSAGE, authid_from_string(STRAUTHID_AUTH_ERROR_MESSAGE));
    BOOST_CHECK_EQUAL(AUTHID_PROXY_TYPE, authid_from_string(STRAUTHID_PROXY_TYPE));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_OK, authid_from_string(STRAUTHID_TRANS_BUTTON_OK));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_CANCEL, authid_from_string(STRAUTHID_TRANS_BUTTON_CANCEL));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_HELP, authid_from_string(STRAUTHID_TRANS_BUTTON_HELP));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_CLOSE, authid_from_string(STRAUTHID_TRANS_BUTTON_CLOSE));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_REFUSED, authid_from_string(STRAUTHID_TRANS_BUTTON_REFUSED));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_LOGIN, authid_from_string(STRAUTHID_TRANS_LOGIN));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_USERNAME, authid_from_string(STRAUTHID_TRANS_USERNAME));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_PASSWORD, authid_from_string(STRAUTHID_TRANS_PASSWORD));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_TARGET, authid_from_string(STRAUTHID_TRANS_TARGET));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_DIAGNOSTIC, authid_from_string(STRAUTHID_TRANS_DIAGNOSTIC));
    BOOST_CHECK_EQUAL(AUTHID_TRANS_CONNECTION_CLOSED, authid_from_string(STRAUTHID_TRANS_CONNECTION_CLOSED));
    BOOST_CHECK_EQUAL(AUTHID_MODE_CONSOLE, authid_from_string(STRAUTHID_MODE_CONSOLE));
    BOOST_CHECK_EQUAL(AUTHID_VIDEO_QUALITY, authid_from_string(STRAUTHID_VIDEO_QUALITY));
    BOOST_CHECK_EQUAL(AUTHID_TIMEZONE, authid_from_string(STRAUTHID_TIMEZONE));
}

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
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestLogon
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "RDP/logon.hpp"


BOOST_AUTO_TEST_CASE(TestLogon)
{
    InfoPacket infoPacket;

	infoPacket.rdp5_support = 1;
    infoPacket.flags  = INFO_MOUSE;
	infoPacket.flags |= INFO_DISABLECTRLALTDEL;
	infoPacket.flags |= INFO_UNICODE;
	infoPacket.flags |= INFO_MAXIMIZESHELL;
	infoPacket.flags |= INFO_ENABLEWINDOWSKEY;
	infoPacket.flags |= INFO_LOGONNOTIFY;;
	infoPacket.flags |= ( (strlen((char *) infoPacket.Password ) > 0) * INFO_AUTOLOGON );
	infoPacket.flags |= ( infoPacket.rdp5_support != 0 ) * ( INFO_LOGONERRORS | INFO_NOAUDIOPLAYBACK );

    memcpy(infoPacket.Domain, "Domain_Test", strlen("Domain_Test"));
    infoPacket.cbDomain = strlen((char *) infoPacket.Domain);
    memcpy(infoPacket.UserName, "UserName_Test", strlen("UserName_Test"));
    infoPacket.cbUserName = strlen((char *) infoPacket.UserName);
    memcpy(infoPacket.Password, "Password_Test", strlen("Password_Test"));
    infoPacket.cbPassword = strlen((char *) infoPacket.Password);
    memcpy(infoPacket.AlternateShell, "Program_Test", strlen("Program_Test"));
    infoPacket.cbAlternateShell = strlen((char *) infoPacket.AlternateShell);
    memcpy(infoPacket.WorkingDir, "Directory_Test", strlen("Directory_Test"));
    infoPacket.cbWorkingDir = strlen((char *) infoPacket.WorkingDir);

    infoPacket.extendedInfoPacket.performanceFlags = PERF_DISABLE_WALLPAPER | 1 * ( PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS);

    BOOST_CHECK_EQUAL(infoPacket.rdp5_support, (uint32_t)1);
    BOOST_CHECK_EQUAL(infoPacket.flags, (uint32_t)590195);
    BOOST_CHECK_EQUAL(std::string((char *) infoPacket.Domain), std::string("Domain_Test") );
    BOOST_CHECK_EQUAL(infoPacket.cbDomain, (uint32_t)strlen((char *) infoPacket.Domain));
    BOOST_CHECK_EQUAL(std::string((char *) infoPacket.UserName), std::string("UserName_Test") );
    BOOST_CHECK_EQUAL(infoPacket.cbUserName, (uint32_t)strlen((char *) infoPacket.UserName));
    BOOST_CHECK_EQUAL(std::string((char *) infoPacket.Password), std::string("Password_Test") );
    BOOST_CHECK_EQUAL(infoPacket.cbPassword, (uint32_t)strlen((char *) infoPacket.Password));
    BOOST_CHECK_EQUAL(std::string((char *) infoPacket.AlternateShell), std::string("Program_Test") );
    BOOST_CHECK_EQUAL(infoPacket.cbAlternateShell, (uint32_t)strlen((char *) infoPacket.AlternateShell));
    BOOST_CHECK_EQUAL(std::string((char *) infoPacket.WorkingDir), std::string("Directory_Test") );
    BOOST_CHECK_EQUAL(infoPacket.cbWorkingDir, (uint32_t)strlen((char *) infoPacket.WorkingDir));
    BOOST_CHECK_EQUAL(infoPacket.extendedInfoPacket.performanceFlags, (uint32_t)7);

}

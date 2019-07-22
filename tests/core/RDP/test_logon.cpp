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

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/logon.hpp"


RED_AUTO_TEST_CASE(TestLogon)
{
    // TODO Really the test below is useless, we are testing assignment!
    // TODO Infopacket should be replaced by some constructor with parameters and test fixed
    InfoPacket infoPacket;

    infoPacket.rdp5_support = 1;

    size_t lg = strlen("Domain_Test");
    memcpy(infoPacket.Domain, "Domain_Test", lg);
    infoPacket.cbDomain = lg;

    lg = strlen("UserName_Test");
    memcpy(infoPacket.UserName, "UserName_Test", lg);
    infoPacket.cbUserName = lg;

    lg = strlen("Password_Test");
    memcpy(infoPacket.Password, "Password_Test", lg);
    infoPacket.cbPassword = lg;

    lg = strlen("Program_Test");
    memcpy(infoPacket.AlternateShell, "Program_Test", lg);
    infoPacket.cbAlternateShell = lg;

    lg = strlen("Directory_Test");
    memcpy(infoPacket.WorkingDir, "Directory_Test", lg);
    infoPacket.cbWorkingDir = lg;

    infoPacket.flags  = INFO_MOUSE;
    infoPacket.flags |= INFO_DISABLECTRLALTDEL;
    infoPacket.flags |= INFO_UNICODE;
    infoPacket.flags |= INFO_MAXIMIZESHELL;
    infoPacket.flags |= INFO_ENABLEWINDOWSKEY;
    infoPacket.flags |= INFO_LOGONNOTIFY;;
    infoPacket.flags |= ((infoPacket.Password[0]|infoPacket.Password[1]) != 0) * INFO_AUTOLOGON;
    infoPacket.flags |= (infoPacket.rdp5_support != 0) * (INFO_LOGONERRORS | INFO_NOAUDIOPLAYBACK);

    infoPacket.extendedInfoPacket.performanceFlags = PERF_DISABLE_WALLPAPER
                                                   | 1 * ( PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS);

    RED_CHECK_EQUAL(infoPacket.rdp5_support, static_cast<uint32_t>(1));
    RED_CHECK_EQUAL(infoPacket.flags, static_cast<uint32_t>
        (INFO_MOUSE
        |INFO_DISABLECTRLALTDEL
        |INFO_UNICODE
        |INFO_MAXIMIZESHELL
        |INFO_ENABLEWINDOWSKEY
        |INFO_LOGONNOTIFY
        |INFO_AUTOLOGON
        |INFO_LOGONERRORS
        |INFO_NOAUDIOPLAYBACK
        ));
    RED_CHECK_MEM("Domain_Test"_av, infoPacket.av_domain());
    RED_CHECK_MEM("UserName_Test"_av, infoPacket.av_user_anme());
    RED_CHECK_MEM("Password_Test"_av, infoPacket.av_password());
    RED_CHECK_MEM("Program_Test"_av, infoPacket.av_alternate_shell());
    RED_CHECK_MEM("Directory_Test"_av, infoPacket.av_working_directory());
    RED_CHECK_EQUAL(infoPacket.extendedInfoPacket.performanceFlags,
        (PERF_DISABLE_WALLPAPER|PERF_DISABLE_FULLWINDOWDRAG|PERF_DISABLE_MENUANIMATIONS));
}

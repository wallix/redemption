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
    InfoPacket infoPacket(
        /*.rdp5_support = */1,
        /*.cbDomain = */ "Domain_Test",
        /*.cbUserName = */ "UserName_Test",
        /*.cbPassword = */ "Password_Test",
        /*cbAlternateShell = */ "Program_Test",
        /*.cbWorkingDir = */ "Directory_Test",
        /*.performanceFlags = */ PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS);

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
        ));
    RED_CHECK_MEM("Domain_Test"_av, infoPacket.zDomain());
    RED_CHECK_MEM("UserName_Test"_av, infoPacket.zUserName());
    RED_CHECK_MEM("Password_Test"_av, infoPacket.zPassword());
    RED_CHECK_MEM("Program_Test"_av, infoPacket.zAlternateShell());
    RED_CHECK_MEM("Directory_Test"_av, infoPacket.zWorkingDirectory());
    RED_CHECK_EQUAL(infoPacket.extendedInfoPacket.performanceFlags,
        (PERF_DISABLE_WALLPAPER|PERF_DISABLE_FULLWINDOWDRAG|PERF_DISABLE_MENUANIMATIONS));
}

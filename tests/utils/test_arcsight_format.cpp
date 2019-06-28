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

   Unit test for bitmap class, compression performance

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/arcsight.hpp"
#include "utils/arcsight_format.hpp"
#include "main/version.hpp"


RED_AUTO_TEST_CASE(TestArcsight)
{
    ArcsightLogInfo arc_info;
    arc_info.name = "SESSION_CREATION";
    arc_info.ApplicationProtocol = "vnc";
    arc_info.WallixBastionStatus = "FAIL";
    arc_info.direction_flag = ArcsightLogInfo::Direction::SERVER_DST;

    std::string s;
    arcsight_format(s, arc_info, 0, "admin", "user1", "10.10.13.12", "", "", "t1");
    RED_CHECK_SMEM(s, "Jan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_CREATION|5|WallixBastionUser=admin WallixBastionAccount=user1 WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionSession_id= WallixBastionSessionType=t1 suser=admin duser=user1 src=10.10.13.12 dst= app=vnc WallixBastionStatus=FAIL"_av);

    arc_info.message = "long long\nmessage=|x\\y";
    s.clear();

    arcsight_format(s, arc_info, 0, "admin", "user1", "10.10.13.12", "", "id", "");
    RED_CHECK_SMEM(s, "Jan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|0|SESSION_CREATION|5|WallixBastionUser=admin WallixBastionAccount=user1 WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionSession_id=id WallixBastionSessionType=Neutral suser=admin duser=user1 src=10.10.13.12 dst= app=vnc WallixBastionStatus=FAIL msg=long long\\nmessage\\=|x\\\\y"_av);
}

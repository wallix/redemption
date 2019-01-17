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
   Copyright (C) Wallix 2015
   Author(s): Christophe Grosjean, Jonathan Poelen,
              Meng Tan, Raphael Zhou

*/

#define RED_TEST_MODULE TestRedirectionInfo
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/redirection_info.hpp"

RED_AUTO_TEST_CASE(TestRedirectionInfo)
{
    RedirectionInfo rinfo;
    rinfo.valid = true;
    rinfo.session_id = 3333;
    memcpy(rinfo.host, "machine.domaine.lan", sizeof("machine.domaine.lan"));
    memcpy(rinfo.username, "utilisateur", sizeof("utilisateur"));
    memcpy(rinfo.domain, "domaine", sizeof("domaine"));
    rinfo.smart_card_logon = true;

    rinfo.log(LOG_INFO, "test rinfo");

    RED_CHECK_EQUAL(rinfo.session_id, 3333u);
    rinfo = RedirectionInfo();
    RED_CHECK_EQUAL(rinfo.session_id, 0u);
}

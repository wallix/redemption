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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/log_siem.hpp"
#include "test_only/log_buffered.hpp"


RED_AUTO_TEST_CASE(TestLogProxySiem)
{
    ut::log_buffered log_buf;
    log_siem::target_connection("user", "sid", "host", 33);
    log_siem::set_psid("L33t");
    log_siem::incoming_connection("universe", 1234);
    log_siem::target_connection("user", "sid", "host", 33);
    log_siem::set_user("Banana");
    log_siem::target_connection("user", "sid", "host", 33);
    RED_CHECK(log_buf.buf() ==
        R"([rdpproxy] psid="42" user="" type="TARGET_CONNECTION" target="user" session_id="sid" host="host" port="33")""\n"
        R"([rdpproxy] psid="L33t" type="INCOMING_CONNECTION" src_ip="universe" src_port="1234")""\n"
        R"([rdpproxy] psid="L33t" user="" type="TARGET_CONNECTION" target="user" session_id="sid" host="host" port="33")""\n"
        R"([rdpproxy] psid="L33t" user="Banana" type="TARGET_CONNECTION" target="user" session_id="sid" host="host" port="33")""\n"
        ""_av
    );
}

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


RED_AUTO_TEST_CASE(TestLogSiem)
{
    tu::log_buffered log_buf;
    LOG_SIEM("test %s", "1");
    RED_CHECK(log_buf.buf() == "test 1\n");
}

RED_AUTO_TEST_CASE(TestLogProxySiem)
{
    tu::log_buffered log_buf;
    LOG_PROXY_SIEM("CAT", "test %s", "1");
    log_proxy::init("L33t", "universe", 1234);
    LOG_PROXY_SIEM("TAC", "test 2");
    log_proxy::set_user("Banana");
    LOG_PROXY_SIEM("ACT", "test 3");
    RED_CHECK(log_buf.buf() ==
        R"([rdpproxy] psid="42" user="" type="CAT" test 1)""\n"
        R"([rdpproxy] psid="L33t" type="INCOMING_CONNECTION" src_ip="universe" src_port="1234")""\n"
        R"([rdpproxy] psid="L33t" user="" type="TAC" test 2)""\n"
        R"([rdpproxy] psid="L33t" user="Banana" type="ACT" test 3)""\n"_av
    );
}

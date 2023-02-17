/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing.upper_order().svs to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/parse_server_message.hpp"
#include "core/log_id.hpp"


RED_AUTO_TEST_CASE(TestParseServerMessage)
{
    ParseServerMessage parse_server_message;

    RED_CHECK(parse_server_message.parse("Request=Hello\x01" "12345\x01" "67890"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "REQUEST");
    RED_REQUIRE(parse_server_message.parameters().size() == 3u);
    RED_CHECK(parse_server_message.parameters()[0] == "Hello"_av);
    RED_CHECK(parse_server_message.parameters()[1] == "12345"_av);
    RED_CHECK(parse_server_message.parameters()[2] == "67890"_av);

    RED_CHECK(parse_server_message.parse("Request2=Hello2\x01" "\x01" "678902"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "REQUEST2");
    RED_REQUIRE(parse_server_message.parameters().size() == 3u);
    RED_CHECK(parse_server_message.parameters()[0] == "Hello2"_av);
    RED_CHECK(parse_server_message.parameters()[1] == ""_av);
    RED_CHECK(parse_server_message.parameters()[2] == "678902"_av);

    RED_CHECK(parse_server_message.parse("Request=Hello\x01" "12345\x01"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "REQUEST");
    RED_REQUIRE(parse_server_message.parameters().size() == 3u);
    RED_CHECK(parse_server_message.parameters()[0] == "Hello"_av);
    RED_CHECK(parse_server_message.parameters()[1] == "12345"_av);
    RED_CHECK(parse_server_message.parameters()[2] == ""_av);

    RED_CHECK(parse_server_message.parse("Request"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "REQUEST");
    RED_REQUIRE(parse_server_message.parameters().size() == 0u);

    RED_CHECK(parse_server_message.parse("=TOTO"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "");
    RED_REQUIRE(parse_server_message.parameters().size() == 1u);
    RED_CHECK(parse_server_message.parameters()[0] == "TOTO"_av);

    RED_CHECK(parse_server_message.parse(
        "Request=0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "REQUEST");
    RED_REQUIRE(parse_server_message.parameters().size() == 8u);
    RED_CHECK(parse_server_message.parameters()[0] == "0"_av);
    RED_CHECK(parse_server_message.parameters()[1] == "1"_av);
    RED_CHECK(parse_server_message.parameters()[2] == "2"_av);
    RED_CHECK(parse_server_message.parameters()[3] == "3"_av);
    RED_CHECK(parse_server_message.parameters()[4] == "4"_av);
    RED_CHECK(parse_server_message.parameters()[5] == "5"_av);
    RED_CHECK(parse_server_message.parameters()[6] == "6"_av);
    RED_CHECK(parse_server_message.parameters()[7] == "7"_av);

    RED_CHECK(!parse_server_message.parse(
        "Request=0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"
        "0\x01""1\x01""2\x01""3\x01""4\x01""5\x01""6\x01""7\x01""8"_zv));

    RED_CHECK(parse_server_message.parse("GetWabSessionParameters"_zv));
    RED_CHECK(parse_server_message.upper_order().sv() == "GETWABSESSIONPARAMETERS");
    RED_REQUIRE(parse_server_message.parameters().size() == 0u);
}

RED_AUTO_TEST_CASE(TestExecuteLog6)
{
    bool result_for_unknown_key = execute_log6_if(
        "abc"_ascii_upper, {},
        [](LogId /*logid*/, KVLogList /*kvlist*/) { RED_CHECK(false); },
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(BUTTON_CLICKED)),
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED), "k"_av),
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED_2))
    );
    RED_CHECK(!result_for_unknown_key);

    std::string_view params[]{"abc"};
    LogId logid_result {};
    KVLog kvlog_result {};
    bool result_for_known_key = execute_log6_if(
        "EDIT_CHANGED"_ascii_upper, params,
        [&](LogId logid, KVLogList kvlist) {
            logid_result = logid;
            kvlog_result = kvlist[0];
        },
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(BUTTON_CLICKED)),
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED), "k"_av),
        executable_log6_if(EXECUTABLE_LOG6_ID_AND_NAME(EDIT_CHANGED_2))
    );
    RED_REQUIRE(result_for_known_key);
    RED_CHECK(logid_result == LogId::EDIT_CHANGED);
    RED_CHECK(kvlog_result.key == "k"_av);
    RED_CHECK(kvlog_result.value == "abc"_av);
}

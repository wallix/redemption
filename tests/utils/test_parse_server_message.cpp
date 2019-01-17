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

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE TestParseServerMessage
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/parse_server_message.hpp"

RED_AUTO_TEST_CASE(TestParseServerMessage)
{
    std::string              order;
    std::vector<std::string> parameters;

    RED_CHECK_EQUAL(parse_server_message(
            "Request=Hello\x01" "12345\x01" "67890",
            order,
            parameters
        ), true);
    RED_CHECK_EQUAL(order, "Request");
    RED_CHECK_EQUAL(parameters.size(), 3u);
    RED_CHECK_EQUAL(parameters[0], "Hello");
    RED_CHECK_EQUAL(parameters[1], "12345");
    RED_CHECK_EQUAL(parameters[2], "67890");
}

RED_AUTO_TEST_CASE(TestParseServerMessage2)
{
    std::string              order;
    std::vector<std::string> parameters;

    RED_CHECK_EQUAL(parse_server_message(
            "Request=Hello\x01" "\x01" "67890",
            order,
            parameters
        ), true);
    RED_CHECK_EQUAL(order, "Request");
    RED_CHECK_EQUAL(parameters.size(), 3u);
    RED_CHECK_EQUAL(parameters[0], "Hello");
    RED_CHECK_EQUAL(parameters[1], "");
    RED_CHECK_EQUAL(parameters[2], "67890");
}

RED_AUTO_TEST_CASE(TestParseServerMessage3)
{
    std::string              order;
    std::vector<std::string> parameters;

    RED_CHECK_EQUAL(parse_server_message(
            "Request=Hello\x01" "12345\x01",
            order,
            parameters
        ), true);
    RED_CHECK_EQUAL(order, "Request");
    RED_CHECK_EQUAL(parameters.size(), 3u);
    RED_CHECK_EQUAL(parameters[0], "Hello");
    RED_CHECK_EQUAL(parameters[1], "12345");
    RED_CHECK_EQUAL(parameters[2], "");
}

RED_AUTO_TEST_CASE(TestParseServerMessage4)
{
    std::string              order;
    std::vector<std::string> parameters;

    RED_CHECK_EQUAL(parse_server_message(
            "Request",
            order,
            parameters
        ), true);
    RED_CHECK_EQUAL(order, "Request");
    RED_CHECK_EQUAL(parameters.size(), 0u);
}

RED_AUTO_TEST_CASE(TestParseServerMessage5)
{
    std::string              order;
    std::vector<std::string> parameters;

    RED_CHECK_EQUAL(parse_server_message(
            "=TOTO",
            order,
            parameters
        ), false);
    RED_CHECK_EQUAL(order, "");
    RED_CHECK_EQUAL(parameters.size(), 1u);
    RED_CHECK_EQUAL(parameters[0], "TOTO");
}

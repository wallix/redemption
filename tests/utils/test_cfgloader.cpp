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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/cfgloader.hpp"

#include <sstream>
#include <string_view>

using namespace std::string_view_literals;



RED_AUTO_TEST_CASE(TestCfgloader)
{
    struct : ConfigurationHolder
    {
        int i = 0;
        void set_value(const char* section, const char* key, const char* value) override
        {
            switch (i++)
            {
                case 0:
                    RED_CHECK(section == "abc"sv);
                    RED_CHECK(key == "abc"sv);
                    RED_CHECK(value == "abc"sv);
                    break;
                case 1:
                    RED_CHECK(section == "abc"sv);
                    RED_CHECK(key == "vv"sv);
                    RED_CHECK(value == "plop"sv);
                    break;
                case 2:
                    RED_CHECK(section == "s"sv);
                    RED_CHECK(key == "aaa"sv);
                    RED_CHECK(value == "bbb"sv);
                    break;
                case 3:
                    RED_CHECK(section == "ss"sv);
                    RED_CHECK(key == "bbb"sv);
                    RED_CHECK(value == "aaa"sv);
                    break;
                case 4:
                    RED_CHECK(section == "ss"sv);
                    RED_CHECK(key == "val2"sv);
                    RED_CHECK(value == "1"sv);
                    break;
                case 5:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "v"sv);
                    RED_CHECK(value == "1"sv);
                    break;
                case 6:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "mac"sv);
                    RED_CHECK(value == "2"sv);
                    break;
                case 7:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "mac"sv);
                    RED_CHECK(value == "3"sv);
                    break;
                case 8:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "mac"sv);
                    RED_CHECK(value == "4"sv);
                    break;
                case 9:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "window"sv);
                    RED_CHECK(value == "5"sv);
                    break;
                case 10:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "window"sv);
                    RED_CHECK(value == "6"sv);
                    break;
                case 11:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "window"sv);
                    RED_CHECK(value == "7"sv);
                    break;
                case 12:
                    RED_CHECK(section == "a  a"sv);
                    RED_CHECK(key == "v3"sv);
                    RED_CHECK(value == "x"sv);
                    break;
                default:
                    RED_FAIL(i-1);
            }
        }
    } cfg;

    std::stringstream ss;
    ss <<
        "[abc]\n"
        "abc=abc\n"
        "\n"
        "\n"
        " #blah blah\n"
        "vv=  plop\n"
        "\n"
        "[s]\n"
        "aaa=bbb   \n"
        "\n"
        "[   ss  ]\n"
        "  bbb=aaa   \n"
        "\n"
        "val="
    ;
    ss.width(1024);
    ss.fill('x'); ss << "\n";
    ss <<
        "val2   =1\n"
        "[ a  a ]\n"
        "  v =  1 \n"
        "  mac =  2 \n\r"
        "  mac =  3 \n\r"
        "  mac =  4 \n\r"
        "  window =  5 \r\n"
        "  window =  6 \r\n"
        "  window =  7 \r\n"
        "v3=x"
    ;

    configuration_load(cfg, ss);
    RED_CHECK_EQ(cfg.i, 13);
}

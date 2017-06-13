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

#define RED_TEST_MODULE TestXXXXXXXXXX
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
// #define LOGPRINT

#include "utils/cfgloader.hpp"


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
                    RED_CHECK_EQ(section, "abc");
                    RED_CHECK_EQ(key, "abc");
                    RED_CHECK_EQ(value, "abc");
                    break;
                case 1:
                    RED_CHECK_EQ(section, "abc");
                    RED_CHECK_EQ(key, "vv");
                    RED_CHECK_EQ(value, "plop");
                    break;
                case 2:
                    RED_CHECK_EQ(section, "s");
                    RED_CHECK_EQ(key, "aaa");
                    RED_CHECK_EQ(value, "bbb");
                    break;
                case 3:
                    RED_CHECK_EQ(section, "ss");
                    RED_CHECK_EQ(key, "bbb");
                    RED_CHECK_EQ(value, "aaa");
                    break;
                case 4:
                    RED_CHECK_EQ(section, "ss");
                    RED_CHECK_EQ(key, "val2");
                    RED_CHECK_EQ(value, "1");
                    break;
                case 5:
                    RED_CHECK_EQ(section, "a  a");
                    RED_CHECK_EQ(key, "v");
                    RED_CHECK_EQ(value, "1");
                    break;
                case 6:
                    RED_CHECK_EQ(section, "a  a");
                    RED_CHECK_EQ(key, "v3");
                    RED_CHECK_EQ(value, "x");
                    break;
                default:
                    RED_CHECK(false);
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
        "v3=x"
    ;

    configuration_load(cfg, ss);
    RED_CHECK_EQ(cfg.i, 7);
}

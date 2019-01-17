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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE GraphicsApi
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "gdi/graphic_api.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TextMetrics)
{
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "abc");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(27, text.width);
    }
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "abcde");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(46, text.width);
    }
    {
        gdi::TextMetrics text(global_font_lato_light_16(), "Ay");
        RED_CHECK_EQUAL(21, text.height);
        RED_CHECK_EQUAL(20, text.width);
    }
}

RED_AUTO_TEST_CASE(MultiLineTextMetrics)
{
    std::string out;

    {
        gdi::MultiLineTextMetrics text(global_font_lato_light_16(),
            "abc efg fajshfkg kf gfjg hjgsj dj, fhsg h, sg, mshg szjh gkj,"
            " s hzgskhg shzktgs t lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli"
            " tyzkr tyzdkl yzdk,  ylktyzdlk dlktuh lkzhluzo huwory gzl",
            0, 300, out);
        RED_CHECK_EQUAL(105, text.height);
        RED_CHECK_EQUAL(299, text.width);
        RED_CHECK_EQUAL(
            "abc efg fajshfkg kf gfjg hjgsj dj, fhsg h,<br>"
            "sg, mshg szjh gkj, s hzgskhg shzktgs t<br>"
            "lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli<br>"
            "tyzkr tyzdkl yzdk, ylktyzdlk dlktuh<br>"
            "lkzhluzo huwory gzl",
            out);
    }
}

RED_AUTO_TEST_CASE(MultiLineTextMetricsEx)
{
    std::string out;

    {
        gdi::MultiLineTextMetricsEx text(global_font_lato_light_16(),
            "Unauthorized access to this system is forbidden and will be prosecuted by law. By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
            0, 300, out);
        RED_CHECK_EQUAL(126, text.height);
        RED_CHECK_EQUAL(286, text.width);
        RED_CHECK_EQUAL(
            "Unauthorized access to this system is<br>"
            "forbidden and will be prosecuted by<br>"
            "law. By accessing this system, you<br>"
            "agree that your actions may be<br>"
            "monitored if unauthorized usage is<br>"
            "suspected.",
            out);
    }
}

RED_AUTO_TEST_CASE(MultiLineTextMetricsEx1)
{
    std::string out;

    {
        gdi::MultiLineTextMetricsEx text(global_font_lato_light_16(),
            "Unauthorized access to this system is forbidden and will be prosecuted by law.<br>By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
            0, 300, out);
        RED_CHECK_EQUAL(126, text.height);
        RED_CHECK_EQUAL(286, text.width);
        RED_CHECK_EQUAL(
            "Unauthorized access to this system is<br>"
            "forbidden and will be prosecuted by<br>"
            "law.<br>"
            "By accessing this system, you agree<br>"
            "that your actions may be monitored if<br>"
            "unauthorized usage is suspected.",
            out);
    }
}

RED_AUTO_TEST_CASE(MultiLineTextMetricsEx2)
{
    std::string out;

    {
        gdi::MultiLineTextMetricsEx text(global_font_lato_light_16(),
            "Unauthorized access to this system is forbidden and will be prosecuted by law.<br><br>By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
            0, 300, out);
        RED_CHECK_EQUAL(147, text.height);
        RED_CHECK_EQUAL(286, text.width);
        RED_CHECK_EQUAL(
            "Unauthorized access to this system is<br>"
            "forbidden and will be prosecuted by<br>"
            "law.<br>"
            "<br>"
            "By accessing this system, you agree<br>"
            "that your actions may be monitored if<br>"
            "unauthorized usage is suspected.",
            out);
    }
}

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
#include "system/redemption_unit_tests.hpp"


#define LOGNULL

#include "gdi/graphic_api.hpp"
#include "core/font.hpp"

static Font font;

RED_AUTO_TEST_CASE(TextMetrics)
{
    {
        gdi::TextMetrics text(font, "abc");
        RED_CHECK_EQUAL(15, text.height);
        RED_CHECK_EQUAL(21, text.width);
    }
    {
        gdi::TextMetrics text(font, "abcde");
        RED_CHECK_EQUAL(15, text.height);
        RED_CHECK_EQUAL(35, text.width);
    }
    {
        gdi::TextMetrics text(font, "Ay");
        RED_CHECK_EQUAL(15, text.height);
        RED_CHECK_EQUAL(14, text.width);
    }
}

RED_AUTO_TEST_CASE(MultiLineTextMetrics)
{
    std::string out;

    {
        gdi::MultiLineTextMetrics text(font,
            "abc efg fajshfkg kf gfjg hjgsj dj, fhsg h, sg, mshg szjh gkj,"
            " s hzgskhg shzktgs t lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli"
            " tyzkr tyzdkl yzdk,  ylktyzdlk dlktuh lkzhluzo huwory gzl",
            300, out);
        RED_CHECK_EQUAL(75, text.height);
        RED_CHECK_EQUAL(294, text.width);
        RED_CHECK_EQUAL(
            "abc efg fajshfkg kf gfjg hjgsj dj, fhsg h,<br>"
            "sg, mshg szjh gkj, s hzgskhg shzktgs t<br>"
            "lurzywiurtyzlis uhtzsli uyzi tyz liuhtzli<br>"
            "tyzkr tyzdkl yzdk, ylktyzdlk dlktuh<br>lkzhluzo huwory gzl",
            out);
    }
}

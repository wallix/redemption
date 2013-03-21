/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2013
 *   Author(s): Christophe Grosjean
 *
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetClip
#include <boost/test/auto_unit_test.hpp>

// #define LOGNULL
#include <internal/widget2/widget_rect.hpp>

BOOST_AUTO_TEST_CASE(TraceWidgetClip)
{
    WidgetRect w1(NULL, Rect(0,0,100,100), 0, NULL);
    WidgetRect w2(NULL, Rect(80,20,50,50), &w1, NULL);
    WidgetRect w3(NULL, Rect(15,20,10,10), &w2, NULL);
    WidgetRect w4(NULL, Rect(0,0,15,20), &w2, NULL);
    WidgetRect w5(NULL, Rect(0,0,5,5), &w4, NULL);
    WidgetRect w6(NULL, Rect(-10,-10,15,15), &w2, NULL);

    Rect pscreen = w1.position_in_screen(Rect(0,0,w1.rect.cx,w1.rect.cy));
    BOOST_CHECK(w1.rect == pscreen);

    pscreen = w2.position_in_screen(Rect(0,0,w2.rect.cx,w2.rect.cy));
    BOOST_CHECK(Rect(80,20,20,50) == pscreen);

    pscreen = w3.position_in_screen(Rect(0,0,w3.rect.cx,w3.rect.cy));
    BOOST_CHECK(Rect(95,40,5,10) == pscreen);

    pscreen = w4.position_in_screen(Rect(0,0,w4.rect.cx,w4.rect.cy));
    BOOST_CHECK(Rect(80,20,15,20) == pscreen);

    pscreen = w5.position_in_screen(Rect(0,0,w5.rect.cx,w5.rect.cy));
    BOOST_CHECK(Rect(80,20,5,5) == pscreen);

    pscreen = w6.position_in_screen(Rect(0,0,w6.rect.cx,w6.rect.cy));
    BOOST_CHECK(Rect(80,20,5,5) == pscreen);

    pscreen = w2.position_in_screen(Rect(10,10,w2.rect.cx,w2.rect.cy));
    BOOST_CHECK(Rect(90,30,10,40) == pscreen);
}

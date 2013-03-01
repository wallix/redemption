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
#include <widget2/widget.hpp>
#include <widget2/widget_composite.hpp>

BOOST_AUTO_TEST_CASE(TraceWidgetClip)
{
    WidgetComposite w1(0, Rect(0,0,100,100), 0, Widget::TYPE_WND, 0);
    WidgetComposite w2(0, Rect(80,20,50,50), &w1, Widget::TYPE_WND, 0);
    Widget w3(0, Rect(15,20,10,10), &w2, Widget::TYPE_BUTTON, 0);
    WidgetComposite w4(0, Rect(0,0,15,20), &w2, Widget::TYPE_WND, 0);
    Widget w5(0, Rect(0,0,5,5), &w4, Widget::TYPE_BUTTON, 0);

    BOOST_CHECK(w1.rect == w1.get_clip_base());
    BOOST_CHECK(Rect(80,20,20,50) == w2.get_clip_base());
    BOOST_CHECK(Rect(95,40,5,10) == w3.get_clip_base());
    BOOST_CHECK(Rect(80,20,15,20) == w4.get_clip_base());
    BOOST_CHECK(Rect(80,20,5,5) == w5.get_clip_base());
}
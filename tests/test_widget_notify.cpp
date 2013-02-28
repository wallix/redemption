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
#define BOOST_TEST_MODULE TestWidgetNotify
#include <boost/test/auto_unit_test.hpp>

// #define LOGNULL
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <widget2/window.hpp>
#include <widget2/screen.hpp>
#include <widget2/edit.hpp>

struct TestNotify : NotifyApi
{
    std::ostringstream oss;

    virtual void notify(Widget * sender, EventType event)
    {
        BOOST_ASSERT(sender != 0);
        oss << "event: " << event
        << " -- id: " << sender->id
        << ", type: " << sender->type << std::endl;
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetFocus)
{
    TestNotify notify;
    WidgetScreen screen(0, 1000, 1000, &notify);
    Window win(0, Rect(30,30, 800, 600), &screen, &notify);
    win.has_focus = true;
    Widget w1(0, Rect(10, 40, 10, 10), &win, Widget::TYPE_BUTTON, &notify);
    WidgetEdit w2(0, Rect(50, 40, 10, 10), &win, &notify);
    Widget w3(0, Rect(100, 400, 10, 10), &win, Widget::TYPE_BUTTON, &notify);
    w1.id = 1;
    w2.id = 2;
    w3.id = 3;
    BOOST_CHECK(&w1 == w1.widget_at_pos(15,45));
    BOOST_CHECK(0 == screen.widget_at_pos(5, 20));
    BOOST_CHECK(&w1 == screen.widget_at_pos(45, 70));
    BOOST_CHECK(&w2 == screen.widget_at_pos(89, 70));
    BOOST_CHECK(&w3 == screen.widget_at_pos(133, 437));

    w2.has_focus = true;
    BOOST_CHECK(!w1.has_focus && w2.has_focus && !w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(133, 437);
        wevent->has_focus = true;
        wevent->notify(wevent, FOCUS_BEGIN);
    }
    BOOST_CHECK(!w1.has_focus && !w2.has_focus && w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(45, 70);
        wevent->has_focus = true;
        wevent->notify(wevent, FOCUS_BEGIN);
    }
    BOOST_CHECK(w1.has_focus && !w2.has_focus && !w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(89, 70);
        wevent->has_focus = true;
        wevent->notify(wevent, FOCUS_BEGIN);
    }
    BOOST_CHECK(!w1.has_focus && w2.has_focus && !w3.has_focus);

    BOOST_CHECK(notify.oss.str() ==
        "event: 0 -- id: 3, type: 3\n"
        "event: 1 -- id: 2, type: 5\n"
        "event: 0 -- id: 1, type: 3\n"
        "event: 1 -- id: 3, type: 3\n"
        "event: 0 -- id: 2, type: 5\n"
        "event: 1 -- id: 1, type: 3\n");

    notify.oss.str("");

    Keymap2 keymap;
    screen.send_event(KEYDOWN, 0, 0, &keymap);
    BOOST_CHECK(notify.oss.str() ==
        "event: 11 -- id: 2, type: 5\n");
}
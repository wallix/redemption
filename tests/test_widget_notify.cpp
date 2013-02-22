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
#include <sstream>
#include <widget2/window.hpp>

void widget_info(std::ostringstream& out, Widget* w, int depth = 0)
{
    std::string s(depth, ' ');
    out << s << "id: " << w->id << ", focus: " << w->has_focus << std::endl;
    if (w->type == Widget::TYPE_SCREEN || w->type == Widget::TYPE_WND){
        WidgetComposite* wc = static_cast<WidgetComposite*>(w);
        for (std::size_t i = 0; i < wc->child_list.size(); ++i){
            widget_info(out, wc->child_list[i], depth+2);
        }
    }
}

BOOST_AUTO_TEST_CASE(TestWidgetFocus)
{
    Window screen(0, 1000, 1000, 0, Widget::TYPE_SCREEN);
    Window win(0, 800, 600, 0);
    screen.addWidget(&win);
    win.set_position(30,30);
    Widget w1(0, 10, 10, 0, Widget::TYPE_BUTTON); w1.id = 1;
    Widget w2(0, 10, 10, 0, Widget::TYPE_BUTTON); w2.id = 2;
    Widget w3(0, 10, 10, 0, Widget::TYPE_BUTTON); w3.id = 3;
    win.addWidget(&w1);
    win.addWidget(&w2);
    win.addWidget(&w3);
    w1.set_position(10, 40);
    w2.set_position(50, 40);
    w3.set_position(100, 400);
    BOOST_CHECK(&w1 == w1.widget_at_pos(15,45));
    BOOST_CHECK(0 == screen.widget_at_pos(5, 20));
    BOOST_CHECK(&w1 == screen.widget_at_pos(45, 70));
    BOOST_CHECK(&w2 == screen.widget_at_pos(89, 70));
    BOOST_CHECK(&w3 == screen.widget_at_pos(133, 437));

#define WIDGET_CHECK(widget, strCmp)do{\
    std::ostringstream oss;\
    widget_info(oss, widget);\
    BOOST_CHECK_EQUAL(oss.str(), strCmp); }while(0)
    {
        Widget* wevent = screen.widget_at_pos(133, 437);
        wevent->notify(0, FOCUS_BEGIN);
        WIDGET_CHECK(&screen,
        "id: 0, focus: 1\n"
        "  id: 0, focus: 1\n"
        "    id: 1, focus: 0\n"
        "    id: 2, focus: 0\n"
        "    id: 3, focus: 1\n");
    }
    {
        Widget* wevent = screen.widget_at_pos(45, 70);
        wevent->notify(0, FOCUS_BEGIN);
        std::ostringstream oss;
        widget_info(oss, &screen);
        WIDGET_CHECK(&screen,
        "id: 0, focus: 1\n"
        "  id: 0, focus: 1\n"
        "    id: 1, focus: 1\n"
        "    id: 2, focus: 0\n"
        "    id: 3, focus: 0\n");
    }
    {
        Widget* wevent = screen.widget_at_pos(89, 70);
        wevent->notify(0, CLIC_BUTTON1_DOWN);
        std::ostringstream oss;
        widget_info(oss, &screen);
        WIDGET_CHECK(&screen,
        "id: 0, focus: 1\n"
        "  id: 0, focus: 1\n"
        "    id: 1, focus: 0\n"
        "    id: 2, focus: 1\n"
        "    id: 3, focus: 0\n");
    }
}
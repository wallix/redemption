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
    std::map<Widget*, const char *>& dico;
    std::ostringstream oss;

    TestNotify(std::map<Widget*, const char *>& dico_ref)
    : dico(dico_ref)
    {}

    virtual void notify(Widget * sender, Widget * receiver, EventType event)
    {
        BOOST_ASSERT(sender != 0);
        BOOST_ASSERT(receiver != 0);
        BOOST_ASSERT(this->dico.find(sender) != this->dico.end());
        BOOST_ASSERT(this->dico.find(receiver) != this->dico.end());
        oss << "event: " << event
        << "\n\tsender (" << this->dico[sender] << ") -- "
        << "id: " << sender->id
        << ", type: " << sender->type
        << "\n\treceiver (" << this->dico[receiver] << ") -- "
        << "id: " << receiver->id
        << ", type: " << receiver->type << std::endl;
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetFocus)
{
    WidgetScreen screen(0, 1000, 1000);
    Window win(0, 800, 600, 0);
    screen.addWidget(&win);
    win.set_position(30,30);
    win.has_focus = true;
    Widget w1(0, 10, 10, 0, Widget::TYPE_BUTTON); w1.id = 1;
    WidgetEdit w2(0, 10, 10, 0); w2.id = 2;
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


    std::map<Widget*, const char *> dico_widget_name;
    TestNotify notify(dico_widget_name);
    {
        typedef std::pair<Widget*, const char *> pair_t;
        pair_t widgets[] = {
            pair_t(&screen, "screen"),
            pair_t(&win, "win"),
            pair_t(&w1, "w1"),
            pair_t(&w2, "w2"),
            pair_t(&w3, "w3"),
        };
        for (size_t i = 0; i < sizeof(widgets)/sizeof(widgets[0]); ++i){
            dico_widget_name.insert(widgets[i]);
            widgets[i].first->add_notify(FOCUS_BEGIN, &notify);
            widgets[i].first->add_notify(FOCUS_END, &notify);
            widgets[i].first->add_notify(TEXT_CHANGED, &notify);
        }
    }

    w2.has_focus = true;
    BOOST_CHECK(!w1.has_focus && w2.has_focus && !w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(133, 437);
        wevent->focus();
    }
    BOOST_CHECK(!w1.has_focus && !w2.has_focus && w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(45, 70);
        wevent->focus();
    }
    BOOST_CHECK(w1.has_focus && !w2.has_focus && !w3.has_focus);
    {
        Widget* wevent = screen.widget_at_pos(89, 70);
        wevent->focus();
    }
    BOOST_CHECK(!w1.has_focus && w2.has_focus && !w3.has_focus);

    BOOST_CHECK(notify.oss.str() ==
        "event: 0\n"
        "\tsender (w3) -- id: 3, type: 3\n"
        "\treceiver (w3) -- id: 3, type: 3\n"
        "event: 1\n"
        "\tsender (w2) -- id: 2, type: 5\n"
        "\treceiver (w2) -- id: 2, type: 5\n"
        "event: 0\n"
        "\tsender (w1) -- id: 1, type: 3\n"
        "\treceiver (w1) -- id: 1, type: 3\n"
        "event: 1\n"
        "\tsender (w3) -- id: 3, type: 3\n"
        "\treceiver (w3) -- id: 3, type: 3\n"
        "event: 0\n"
        "\tsender (w2) -- id: 2, type: 5\n"
        "\treceiver (w2) -- id: 2, type: 5\n"
        "event: 1\n"
        "\tsender (w1) -- id: 1, type: 3\n"
        "\treceiver (w1) -- id: 1, type: 3\n");

    notify.oss.str("");

    Keymap2 keymap;
    screen.def_proc(KEYDOWN, 0, &keymap);
    BOOST_CHECK(notify.oss.str() ==
        "event: 11\n"
        "\tsender (w2) -- id: 2, type: 5\n"
        "\treceiver (w2) -- id: 2, type: 5\n"
        "event: 11\n"
        "\tsender (w2) -- id: 2, type: 5\n"
        "\treceiver (win) -- id: 0, type: 1\n"
        "event: 11\n"
        "\tsender (w2) -- id: 2, type: 5\n"
        "\treceiver (screen) -- id: 0, type: 0\n");
}
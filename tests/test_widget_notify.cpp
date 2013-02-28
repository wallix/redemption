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
#include <string>
#include <sstream>
#include <widget2/window.hpp>
#include <widget2/screen.hpp>
#include <widget2/edit.hpp>

struct TestNotify : NotifyApi
{
    std::ostringstream oss;

    virtual void notify(Widget * sender, NotifyApi::notify_event_t event)
    {
        BOOST_ASSERT(sender != 0);
        oss << "event: " << event
        << " -- id: " << sender->id
        << ", type: " << sender->type << std::endl;
    }
};

struct TestDraw : ModApi
{
    std::string s;

    virtual void begin_update()
    {
        s += "begin_update\n";
    }


    virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
    {
        s += "draw(RDPOpaqueRect)\n";
    }


    virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
    {
        s += "draw(RDPScrBlt)\n";
    }


    virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
    {
        s += "draw(RDPDestBlt)\n";
    }


    virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
    {
        s += "draw(RDPPatBlt)\n";
    }


    virtual void draw(const RDPLineTo& cmd, const Rect& clip)
    {
        s += "draw(RDPLineTo)\n";
    }


    virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
    {
        s += "draw(RDPGlyphIndex)\n";
    }


    virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
    {
        s += "draw(RDPMemBlt)\n";
    }


    virtual void end_update()
    {
        s += "end_update\n";
    }


    virtual void server_draw_text(uint16_t x, uint16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        s += "server_draw_text\n";
    }


    virtual void text_metrics(const char* text, int& width, int& height)
    {
        s += "text_metrics\n";
    }


};

struct TestWidget
{
    WidgetScreen screen;
    Window win;
    Widget w1;
    WidgetEdit w2;
    Widget w3;

    TestWidget(TestDraw * drawable=0, TestNotify * notify=0)
    : screen(drawable, 1000, 1000, notify)
    , win(drawable, Rect(30,30, 800, 600), &screen, notify)
    , w1(drawable, Rect(10, 40, 10, 10), &win, Widget::TYPE_BUTTON, notify)
    , w2(drawable, Rect(50, 40, 10, 10), &win, notify)
    , w3(drawable, Rect(100, 400, 10, 10), &win, Widget::TYPE_BUTTON, notify)
    {
        win.has_focus = true;
        w2.has_focus = true;
        w1.id = 1;
        w2.id = 2;
        w3.id = 3;
    }
};


BOOST_AUTO_TEST_CASE(TraceWidgetAtPos)
{
    TestWidget w;
    BOOST_CHECK(&w.w1 == w.w1.widget_at_pos(15,45));
    BOOST_CHECK(0 == w.screen.widget_at_pos(5, 20));
    BOOST_CHECK(&w.w1 == w.screen.widget_at_pos(45, 70));
    BOOST_CHECK(&w.w2 == w.screen.widget_at_pos(89, 70));
    BOOST_CHECK(&w.w3 == w.screen.widget_at_pos(133, 437));
}

BOOST_AUTO_TEST_CASE(TraceWidgetFocus)
{
    TestNotify notify;
    TestWidget w(0, &notify);

    BOOST_CHECK(!w.w1.has_focus && w.w2.has_focus && !w.w3.has_focus);
    {
        Widget* wevent = w.screen.widget_at_pos(133, 437);
        wevent->has_focus = true;
        wevent->notify(wevent->id, FOCUS_BEGIN);
    }
    BOOST_CHECK(!w.w1.has_focus && !w.w2.has_focus && w.w3.has_focus);
    {
        Widget* wevent = w.screen.widget_at_pos(45, 70);
        wevent->has_focus = true;
        wevent->notify(wevent->id, FOCUS_BEGIN);
    }
    BOOST_CHECK(w.w1.has_focus && !w.w2.has_focus && !w.w3.has_focus);
    {
        Widget* wevent = w.screen.widget_at_pos(89, 70);
        wevent->has_focus = true;
        wevent->notify(wevent->id, FOCUS_BEGIN);
    }
    BOOST_CHECK(!w.w1.has_focus && w.w2.has_focus && !w.w3.has_focus);

    BOOST_CHECK(notify.oss.str() ==
        "event: 0 -- id: 3, type: 3\n" //FOCUS_END
        "event: 1 -- id: 2, type: 5\n" //FOCUS_BEGIN
        "event: 0 -- id: 1, type: 3\n" //FOCUS_END
        "event: 1 -- id: 3, type: 3\n" //FOCUS_BEGIN
        "event: 0 -- id: 2, type: 5\n" //FOCUS_END
        "event: 1 -- id: 1, type: 3\n" //FOCUS_BEGIN
    );
}
#include <iostream>
BOOST_AUTO_TEST_CASE(TraceWidgetDraw)
{
    TestDraw drawable;
    TestWidget w(&drawable);

    w.screen.send_event(WM_DRAW, 0, 0, 0);

    BOOST_CHECK(drawable.s ==
        "begin_update\n"
        "draw(RDPOpaqueRect)\n"
        "begin_update\n"
        "draw(RDPOpaqueRect)\n"
        "begin_update\n"
        "draw(RDPOpaqueRect)\n"
        "end_update\n"
        "begin_update\n"
        "text_metrics\n"
        "server_draw_text\n"
        "end_update\n"
        "begin_update\n"
        "draw(RDPOpaqueRect)\n"
        "end_update\n"
        "end_update\n"
        "end_update\n"
    );
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestNotify notify;
    TestDraw drawable;
    TestWidget w(&drawable, &notify);

    Keymap2 keymap;
    w.screen.send_event(KEYDOWN, 0, 0, &keymap);
    BOOST_CHECK(notify.oss.str() ==
    "event: 11 -- id: 2, type: 5\n");

    BOOST_CHECK(drawable.s ==
    "begin_update\n"
    "text_metrics\n"
    "server_draw_text\n"
    "end_update\n"
    );
}
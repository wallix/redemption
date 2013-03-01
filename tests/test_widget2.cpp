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
#include <boost/lexical_cast.hpp>
#include <widget2/window.hpp>
#include <widget2/screen.hpp>
#include <widget2/edit.hpp>
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "png.hpp"
#include <stdio.h>

struct TestNotify : NotifyApi
{
    std::string s;

    virtual void notify(Widget * sender, NotifyApi::notify_event_t event)
    {
        BOOST_ASSERT(sender != 0);
        s += "event: ";
        s += boost::lexical_cast<std::string>(event);
        s += " -- id: ";
        s += boost::lexical_cast<std::string>(sender->id);
        s += ", type: ";
        s += boost::lexical_cast<std::string>(sender->type);
        s += '\n';
    }
};

struct TestDraw : ModApi
{
    RDPDrawable gd;

    TestDraw()
    : gd(1000,1000,true)
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
    {
        gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPBrushCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void draw(const RDPColCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void draw(const RDPGlyphCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(uint16_t x, uint16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {}


    virtual void text_metrics(const char* text, int& width, int& height)
    {}
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
        screen.bg_color = 100;
        win.bg_color = 1000;
        w1.bg_color = 10000;
        w2.bg_color = 100000;
        w3.bg_color = 1000000;
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

    BOOST_CHECK(notify.s ==
        "event: 0 -- id: 3, type: 3\n" //FOCUS_END
        "event: 1 -- id: 2, type: 5\n" //FOCUS_BEGIN
        "event: 0 -- id: 1, type: 3\n" //FOCUS_END
        "event: 1 -- id: 3, type: 3\n" //FOCUS_BEGIN
        "event: 0 -- id: 2, type: 5\n" //FOCUS_END
        "event: 1 -- id: 1, type: 3\n" //FOCUS_BEGIN
    );
}

inline bool check_sig(const uint8_t* data, std::size_t height, uint32_t len,
                      char * message, const char * shasig)
{
    uint8_t sig[20];
    SslSha1 sha1;
    for (size_t y = 0; y < (size_t)height; y++){
        sha1.update(data + y * len, len);
    }
    sha1.final(sig);

    if (memcmp(shasig, sig, 20)){
        sprintf(message, "Expected signature: \""
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x"
        "\\x%.2x\\x%.2x\\x%.2x\\x%.2x\"",
        sig[ 0], sig[ 1], sig[ 2], sig[ 3],
        sig[ 4], sig[ 5], sig[ 6], sig[ 7],
        sig[ 8], sig[ 9], sig[10], sig[11],
        sig[12], sig[13], sig[14], sig[15],
        sig[16], sig[17], sig[18], sig[19]);
        return false;
    }
    return true;
}

inline bool check_sig(Drawable & data, char * message, const char * shasig)
{
    return check_sig(data.data, data.height, data.rowsize, message, shasig);
}

BOOST_AUTO_TEST_CASE(TraceWidgetDraw)
{
    TestDraw drawable;
    TestWidget w(&drawable);
    Widget wid(&drawable, Rect(700, 500, 200, 200), &w.win, Widget::TYPE_BUTTON, 0);
    wid.bg_color = 10000000;

    w.screen.send_event(WM_DRAW, 0, 0, 0);
    //or w.screen.refresh(w.screen.rect);

    //std::FILE * file = fopen("/tmp/b.png", "w+");
    //dump_png24(file, drawable.gd.drawable.data, drawable.gd.drawable.width, //drawable.gd.drawable.height, drawable.gd.drawable.rowsize);
    //fclose(file);

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd9\xe1\x56\xed\xbc\x4b\xf5\x70\xe3\x97"
        "\x42\x54\x92\x91\x88\xad\xeb\x49\xc8\x83")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestNotify notify;
    TestDraw drawable;
    TestWidget w(&drawable, &notify);

    Keymap2 keymap;
    w.screen.send_event(KEYDOWN, 0, 0, &keymap);
    BOOST_CHECK(notify.s ==
        "event: 11 -- id: 2, type: 5\n");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7b\xb1\x56\x2c\x72\xf2\x50\x39\x15\xf0"
        "\x63\x27\xfd\x16\x84\x37\x52\xab\x3d\x42")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
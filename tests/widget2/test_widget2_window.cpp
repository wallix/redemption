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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetWindow
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/widget_rect.hpp"
#include "internal/widget2/window.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : ModApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, true)
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
    }

    virtual void draw(const RDPScrBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPDestBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPatBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPLineTo&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphIndex&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPBrushCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPColCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int, int, const char*, uint32_t, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void text_metrics(const char* , int& , int& )
    {
        BOOST_CHECK(false);
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetWindow)
{
    TestDraw drawable(800, 600);
    NotifyApi * notifier = NULL;
    Widget * parent = NULL;
    int id = 0;

    Window window(&drawable, Rect(30,40,500,400), parent, notifier, "Window 1");
    WidgetRect wrect1(&drawable, Rect(0,0,100,100),
                      &window, notifier, id++, YELLOW);
    WidgetRect wrect2(&drawable, Rect(0,100,100,100),
                      &window, notifier, id++, RED);
    WidgetRect wrect3(&drawable, Rect(100,100,100,100),
                      &window, notifier, id++, BLUE);
    WidgetRect wrect4(&drawable, Rect(300,300,100,100),
                      &window, notifier, id++, GREEN);
    WidgetRect wrect5(&drawable, Rect(700,-50,100,100),
                      &window, notifier, id++, WHITE);
    WidgetRect wrect6(&drawable, Rect(-50,550,100,100),
                      &window, notifier, id++, GREY);
    window.child_list.push_back(&wrect1);
    window.child_list.push_back(&wrect2);
    window.child_list.push_back(&wrect3);
    window.child_list.push_back(&wrect4);
    window.child_list.push_back(&wrect5);
    window.child_list.push_back(&wrect6);

    // ask to widget to redraw at position 150,500 and of size 800x600
    window.rdp_input_invalidate(Rect(150 + window.dx(),
                                     150 + window.dy(),
                                     window.cx(),
                                     window.cy()));

    //drawable.save_to_png("/tmp/window.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x92\x57\x70\xc1\xca\x27\x0f\xb3\x89\x2b"
        "\xb6\xad\xe7\x85\xdd\x6f\xc5\x66\xdb\x82")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at position 0,500 and of size 100x100
    window.rdp_input_invalidate(Rect(0 + window.dx(),
                                     500 + window.dy(),
                                     100,
                                     100));

    //drawable.save_to_png("/tmp/window2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x92\x57\x70\xc1\xca\x27\x0f\xb3\x89\x2b"
        "\xb6\xad\xe7\x85\xdd\x6f\xc5\x66\xdb\x82")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    window.rdp_input_invalidate(Rect(0 + window.dx(),
                                     0 + window.dy(),
                                     window.cx(),
                                     window.cy()));

    //drawable.save_to_png("/tmp/window3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x54\xb8\xf6\x10\x0f\xe5\x65\xb5\xc2\xff"
        "\xb1\xa1\xed\x6f\xf1\xb1\x09\x83\x23\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetWindow)
{
    struct TestNotify : NotifyApi
    {
        Widget * sender;
        notify_event_t event;

        TestNotify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget* sender, notify_event_t event,
                            long unsigned int, long unsigned int)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    ModApi * drawable = 0;
    Widget * parent = NULL;
    int id = 0;

    Window window(drawable, Rect(30,40,500,400), parent, &notifier, "Window 1");
    window.button_close.rdp_input_mouse(CLIC_BUTTON1_DOWN, 0,0,0);
    BOOST_CHECK(notifier.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    window.button_close.rdp_input_mouse(CLIC_BUTTON1_UP, 0,0,0);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
    BOOST_CHECK(notifier.sender == &window);
}

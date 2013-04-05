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
#define BOOST_TEST_MODULE TestWindowLogin
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_login.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : ModApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, true)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
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

    virtual void server_draw_text(int x, int y, const char* text, uint32_t fgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                FontChar *font_item = this->gd.get_font(this->font, uni[index]);
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            width -= 2;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWindowLogin)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget at position 0,0 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;
    int id = 0;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test1", id, "rec", "rec");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(window_login.rect);

    //drawable.save_to_png("/tmp/window_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x48\x75\x6d\x3e\x3f\xa2\x52\x25\xca\x80"
        "\x55\xd6\x65\xb5\x85\x21\x23\xa1\x30\xda")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin2)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test2");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x01\x9e\x9d\x0c\x14\x89\x2a\x56\x11\xea"
        "\xb8\x9b\x5f\xfb\xbf\xb3\x76\xcc\xfd\xac")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin3)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test3");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2c\xf5\xbb\x02\x87\x32\xac\x21\x73\xd7"
        "\x11\x6b\xee\x9a\x0d\x90\x19\x91\x9e\x5c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin4)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test4");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x18\x36\xdf\xa9\x0e\x78\x20\xff\xff\x0c"
        "\x47\x32\xf6\x8e\x4b\xa0\x2f\xda\x69\x9d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin5)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test5");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x80\x39\x54\x54\xc4\xab\x60\x5f\xcb\x6f"
        "\x5b\xbd\x21\x48\xf0\x72\x49\x0f\x82\x21")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLogin6)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at it's current position
    window_login.rdp_input_invalidate(Rect(0 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb9\x0f\x65\x96\xe2\xd0\x99\x91\xa2\x5b"
        "\x52\x46\x7c\x15\xea\x01\x48\xfa\x84\x87")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLoginClip)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    window_login.rdp_input_invalidate(Rect(20 + window_login.dx(),
                                      0 + window_login.dy(),
                                      window_login.cx(),
                                      window_login.cy()));

    //drawable.save_to_png("/tmp/window_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9c\xc6\xfb\xba\x72\x6a\xc2\x31\x20\x2e"
        "\x68\x7d\x7c\x97\x38\xcc\x9b\x8d\xda\x90")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowLoginClip2)
{
    TestDraw drawable(800, 600);

    // WindowLogin is a window_login widget of size 100x20 at position 10,7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowLogin window_login(&drawable, x, y, parent, notifier, "test6");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    window_login.rdp_input_invalidate(Rect(20 + window_login.dx(),
                                      5 + window_login.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/window_login8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xbd\xe4\x8e\xb6\x24\x84\x0a\xc1\xc8"
        "\x49\x43\x66\x4a\x90\x47\x8b\xc5\xea\x9e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    Widget * parent = NULL;
    struct Notify : NotifyApi {
        Widget * sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget* sender, notify_event_t event, long unsigned int param, long unsigned int param2)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    int16_t x = 10;
    int16_t y = 10;

    WindowLogin window_login(&drawable, x, y, parent, &notifier, "test6");


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_login.ok.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_login.ok.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == &window_login);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
}

// BOOST_AUTO_TEST_CASE(EventWidgetHelp)
// {
//     TestDraw drawable(800, 600);
//
//     WidgetComposite parent(&drawable, Rect(0,0,800,600), 0, 0);
//
//     int16_t x = 10;
//     int16_t y = 10;
//
//     WindowLogin window_login(&drawable, x, y, &parent, 0, "test6");
//     parent.child_list.push_back(&window_login);
//
//     parent.rdp_input_invalidate(parent.rect);
//
//     window_login.help.send_notify(NOTIFY_SUBMIT);
//     parent.rdp_input_invalidate(parent.rect);
//
//     drawable.save_to_png("/tmp/window_login-help.png");
//
//     char message[1024];
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x75\xbd\xe4\x8e\xb6\x24\x84\x0a\xc1\xc8"
//         "\x49\x43\x66\x4a\x90\x47\x8b\xc5\xea\x9e")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     window_login.window_help->send_notify(NOTIFY_SUBMIT);
//     parent.rdp_input_invalidate(parent.rect);
//
//     drawable.save_to_png("/tmp/window_login-help2.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x75\xbd\xe4\x8e\xb6\x24\x84\x0a\xc1\xc8"
//         "\x49\x43\x66\x4a\x90\x47\x8b\xc5\xea\x9e")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }


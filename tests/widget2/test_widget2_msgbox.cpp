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
#define BOOST_TEST_MODULE TestMessageBox
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/msgbox.hpp"
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

BOOST_AUTO_TEST_CASE(TraceMessageBox)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test1",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0, 0, wmsgbox.cx(), wmsgbox.cy()));


    //drawable.save_to_png("/tmp/msgbox.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x4e\x4c\x10\x52\x29\x59\x29\x04\xf1\x43"
        "\xfe\x01\xc7\x45\x7a\xbd\x8c\x3b\x9d\x69")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox2)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test2",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb1\x3e\xff\xc9\xde\x55\xcb\x5c\x58\xf6"
        "\xee\x48\x4a\x51\xdd\x80\xdf\x3c\x18\x0c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox3)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test3",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\x70\x42\x23\x50\x22\xba\xf6\xfa\x3b"
        "\x7f\x7e\x54\x4c\xee\xea\x82\x80\x72\x16")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox4)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test4",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x29\x0d\xa9\x2d\x89\x68\x0a\x55\x66\x7f"
        "\x49\x70\x7d\x0a\xef\xeb\x8e\x3c\xfb\xdb")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox5)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test5",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe4\xa7\x83\x3a\x37\x90\xa2\xaf\xe0\x98"
        "\xfe\xba\x08\x75\x08\x9c\xa7\xc7\xef\x3d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox6)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7a\x88\x3b\xa9\x83\x8b\xca\x07\xb4\x9e"
        "\x1a\x69\x4e\x9b\x81\x96\xce\xbf\xca\xb3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBoxClip)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmsgbox.rdp_input_invalidate(Rect(20 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png("/tmp/msgbox7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfa\x86\xa9\x1a\x2f\x5f\xa0\x54\xa2\x4d"
        "\x39\x45\x80\xc1\xc7\x34\x6a\xf4\xf9\x95")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBoxClip2)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    MessageBox wmsgbox(&drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmsgbox.rdp_input_invalidate(Rect(20 + wmsgbox.dx(),
                                      5 + wmsgbox.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/msgbox8.png");

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

    Widget2* parent = NULL;
    struct Notify : NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget2* sender, notify_event_t event, long unsigned int param, long unsigned int param2)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;

    MessageBox wmsgbox(&drawable, x, y, parent, &notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, bg_color, fg_color);


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wmsgbox.ok.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wmsgbox.ok.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == &wmsgbox);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}


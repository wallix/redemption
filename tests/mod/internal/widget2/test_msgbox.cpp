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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestMessageBox
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/msgbox.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
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

    virtual void draw(const RDPPatBlt& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPMem3Blt& cmd, const Rect& rect, const Bitmap& bmp)
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

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
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
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceMessageBox)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test1",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0, 0, wmsgbox.cx(), wmsgbox.cy()));


    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfb\x84\x4d\x48\x61\x0d\x08\x8d\x5f\xa0\x63\xb6\x8a\x5d\x42\x97\x37\x7a\x97\x7c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox2)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test2",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x49\x18\x0c\x49\xd8\xd7\x88\xbc\x1d\x18\x9f\xd9\xc9\x9a\x28\x57\xc0\x3e\x3a\x61")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox3)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test3",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x1d\xc2\x6b\x72\x14\x5f\xeb\x64\xfc\x1a"
        "\x69\x4f\xce\x55\xef\xef\xd1\x53\x66\xc7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox4)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test4",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf7\x26\x94\xdb\x18\x6a\x9b\xfd\xb6\x22"
        "\x53\xe2\x9e\x06\x51\xff\xc7\x91\xeb\x7d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox5)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test5",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xdf\xa5\xa3\xae\x1e\xa3\x6d\x41\x30\x01\x0f\xa6\x9b\x92\xbb\xbd\x09\x97\xd5\x16")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBox6)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x92\xde\xd8\x4e\x62\x0a\x59\xa6\x6a\xd1\x03\x0b\x82\x8c\xc5\xc2\xcd\xf3\x55\xb1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBoxClip)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmsgbox.rdp_input_invalidate(Rect(20 + wmsgbox.dx(),
                                      0 + wmsgbox.dy(),
                                      wmsgbox.cx(),
                                      wmsgbox.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa3\x90\xd4\x2b\x42\xb5\x24\xf1\x0d\x76\xb5\xe5\x9c\x40\xa0\x61\xb0\x07\xb0\x22")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceMessageBoxClip2)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmsgbox.rdp_input_invalidate(Rect(20 + wmsgbox.dx(),
                                      5 + wmsgbox.dy(),
                                      30,
                                      10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x41\x37\xe3\x2f\xb2\xfb\x1e\x6f\x6c\x9a"
        "\x93\x72\x4f\x8c\x7c\x90\xf3\x9d\x0d\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    WidgetScreen parent(drawable, 800, 600);
    struct Notify : NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget2* sender, notify_event_t event)
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

    MessageBox wmsgbox(drawable, x, y, parent, &notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wmsgbox.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                               wmsgbox.ok.dx(), wmsgbox.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wmsgbox.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                               wmsgbox.ok.dx(), wmsgbox.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &wmsgbox);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}


BOOST_AUTO_TEST_CASE(TraceMessageBoxOk)
{
    TestDraw drawable(800, 600);

    // MessageBox is a msgbox widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;

    MessageBox wmsgbox(drawable, x, y, parent, notifier, "test1",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", fg_color, bg_color);

    x = wmsgbox.ok.centerx();
    y = wmsgbox.ok.centery();

    wmsgbox.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, NULL);
    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0, 0, wmsgbox.cx(), wmsgbox.cy()));


    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x1c\x81\x45\xa8\x6d\x0c\x32\x07\xd6"
                   "\x79\x8a\xf5\x3a\xc7\x09\x1a\xcc\x4d\x56"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wmsgbox.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, NULL);
    // ask to widget to redraw at it's current position
    wmsgbox.rdp_input_invalidate(Rect(0, 0, wmsgbox.cx(), wmsgbox.cy()));


    //drawable.save_to_png(OUTPUT_FILE_PATH "msgbox10.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xa4\x62\x26\x93\x4d\x17\x15\x38\xa4\x29"
                   "\xe8\x8b\x24\xce\xab\x09\x40\xa8\x56\xc5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

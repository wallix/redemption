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
#define BOOST_TEST_MODULE TestWidgetLabel
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "internal/widget2/label.hpp"
#include "internal/widget2/screen.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test1", auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xe9\xd7\x17\x63\x3d\xe6\x1a\xc2\x59\xb6"
        "\x69\xdf\xab\x32\x42\xe6\x05\xb4\xa2\x5a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test2", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x28\xbf\x82\xf3\xe9\x32\xa1\x11\xfb\x37"
        "\x9a\x02\x43\x0f\xd9\x17\x95\xd3\x25\xc4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test3", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x14\x3a\xd8\x31\x4b\x03\x83\x6b\x4c\x3a"
        "\x92\x90\x01\xb3\x2a\x91\xa6\x43\x40\x85")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test4", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x43\xaf\x23\xac\x69\x21\xb2\xa7\xdd\xa6"
        "\xc4\x08\xd3\x6c\x4a\xa3\x3f\x3f\x6a\xa9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test5", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x2b\xd4\xd5\xfc\x58\x2e\x1e\xac\x95\x7f"
        "\x01\x81\x9a\x07\x96\xe9\xd3\x4a\xc7\xdd")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wlabel.rdp_input_invalidate(Rect(0 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xaf\xb6\xac\x53\x1e\x48\x0b\x03\xde\x66"
        "\xda\xa9\xe5\xab\x8b\x1a\x12\xd0\x0b\xfc")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabelClip)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.dx(),
                                     0 + wlabel.dy(),
                                     wlabel.cx(),
                                     wlabel.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x17\xfc\xdc\x5f\xae\x08\xab\xa7\x5b\xe3"
        "\x2d\xe7\x25\xdf\x37\xf9\x4f\xa2\x28\xb9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabelClip2)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wlabel.rdp_input_invalidate(Rect(20 + wlabel.dx(),
                                     5 + wlabel.dy(),
                                     30,
                                     10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "label8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xd1\xa8\x01\xd4\xcd\x55\xac\xae\xec\x4e"
        "\x0c\xa3\x23\xf3\xd7\x2c\xa5\x24\x96\x36")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabelEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(NULL)
        , event(0)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    Widget2& parent = widget_for_receive_event;
    NotifyApi * notifier = NULL;
    bool auto_resize = false;
    int16_t x = 0;
    int16_t y = 0;

    WidgetLabel wlabel(drawable, x, y, parent, notifier, "", auto_resize, 0, BLACK, WHITE);

    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 0, 0, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    wlabel.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_char('a');

    wlabel.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabelAndComposite)
{
    TestDraw drawable(800, 600);

    //WidgetLabel is a label widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetLabel wlabel1(drawable, 0,0, wcomposite, notifier,
                        "abababab", true, 0, YELLOW, BLACK);
    WidgetLabel wlabel2(drawable, 0,100, wcomposite, notifier,
                        "ggghdgh", true, 0, WHITE, BLUE);
    WidgetLabel wlabel3(drawable, 100,100, wcomposite, notifier,
                        "lldlslql", true, 0, BLUE, RED);
    WidgetLabel wlabel4(drawable, 300,300, wcomposite, notifier,
                        "LLLLMLLM", true, 0, PINK, DARK_GREEN);
    WidgetLabel wlabel5(drawable, 700,-10, wcomposite, notifier,
                        "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE);
    WidgetLabel wlabel6(drawable, -10,550, wcomposite, notifier,
                        "xxwwp", true, 0, ANTHRACITE, PALE_GREEN);

    wcomposite.add_widget(&wlabel1);
    wcomposite.add_widget(&wlabel2);
    wcomposite.add_widget(&wlabel3);
    wcomposite.add_widget(&wlabel4);
    wcomposite.add_widget(&wlabel5);
    wcomposite.add_widget(&wlabel6);

    //ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

   //drawable.save_to_png(OUTPUT_FILE_PATH "label9.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xcb\x89\x53\x0a\x5e\x9e\x97\xb3\x05\x56"
        "\xd8\xd6\x12\x24\x5f\xb7\x77\xe3\x14\xe5")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    //ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "label10.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x16\x9c\x03\xc9\xf5\x45\x42\x8e\x89\x5c"
        "\x8e\x51\x01\x40\xb6\xde\x3b\xc5\x7d\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

TODO("the entry point exists in module: it's rdp_input_invalidate"
     "je just have to change received values to widget messages")

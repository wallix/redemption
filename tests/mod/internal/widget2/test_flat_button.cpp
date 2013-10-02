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
#define BOOST_TEST_MODULE TestWidgetFlatButton
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_button.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
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

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test1", auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    //drawable.save_to_png(OUTPUT_FILE_PATH "button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x60\x57\x8c\x5a\x1d\x3a\xc9\x2e\x98\x95"
        "\x6f\x81\x4f\xc8\xd5\xf2\xd5\x6f\xa8\x13"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton2)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test2", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x6b\xec\x4f\x6b\xea\xae\x2a\x56\x03\xcd"
        "\xff\x9b\x8f\x40\xa1\xeb\x5a\xbf\xcd\xff")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton3)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test3", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8b\xcb\xc6\x89\xe0\xde\x93\x02\xac\x93"
        "\x40\x39\x12\x43\xcd\xf7\x82\x93\xe8\x01"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton4)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test4", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x3e\xed\x9c\xa1\xa0\xca\x17\xf7\x2c\x0a"
        "\x1d\x3d\x6c\x99\x55\x17\x22\xa4\xa2\xbd"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton5)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test5", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x36\x52\x07\x19\x62\xa9\xfc\x1b\x42\xda"
        "\xc3\x53\x57\x9a\xad\x70\x0e\xe0\xce\xeb"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton6)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x4f\x8e\xd9\x10\xa7\x87\x7c\xb1\xd3\x40"
        "\x07\x00\xbb\x4c\x65\x5a\x62\x7a\x3a\xe9"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonClip)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\xd2\x74\x4d\x32\xee\x05\x9f\xbc\x23"
        "\xa9\xac\x81\xd2\x86\xb9\xf2\x50\x16\xa0"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonClip2)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xdd\x3d\x3b\x98\xe5\x6d\x23\xae\x35\xa8"
        "\xbf\xda\x54\xad\xbc\x40\x1c\x71\x98\xbb"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf5\xcd\x48\xa3\x3b\xca\xa7\x0e\x26\x91"
        "\x28\x93\x16\xb2\x0b\x28\x65\x2e\x7e\xe7"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button10.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x30\xd3\xd4\x46\x90\xb6\xe4\xda\xf0\xf4"
        "\x59\x84\xeb\x22\xe3\x22\x2a\x1d\xab\xac"
       )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button11.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xf5\xcd\x48\xa3\x3b\xca\xa7\x0e\x26\x91"
        "\x28\x93\x16\xb2\x0b\x28\x65\x2e\x7e\xe7"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(0)
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

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {
        }
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget2& parent = widget_for_receive_event;
    bool auto_resize = false;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char('a');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char(' ');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetFlatButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetFlatButton wbutton1(drawable, 0,0, wcomposite, notifier,
                        "abababab", true, 0, YELLOW, BLACK);
    WidgetFlatButton wbutton2(drawable, 0,100, wcomposite, notifier,
                        "ggghdgh", true, 0, WHITE, RED);
    WidgetFlatButton wbutton3(drawable, 100,100, wcomposite, notifier,
                        "lldlslql", true, 0, BLUE, RED);
    WidgetFlatButton wbutton4(drawable, 300,300, wcomposite, notifier,
                        "LLLLMLLM", true, 0, PINK, DARK_GREEN);
    WidgetFlatButton wbutton5(drawable, 700,-10, wcomposite, notifier,
                        "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE);
    WidgetFlatButton wbutton6(drawable, -10,550, wcomposite, notifier,
                        "xxwwp", true, 0, DARK_GREY, PALE_GREEN);

    wcomposite.add_widget(&wbutton1);
    wcomposite.add_widget(&wbutton2);
    wcomposite.add_widget(&wbutton3);
    wcomposite.add_widget(&wbutton4);
    wcomposite.add_widget(&wbutton5);
    wcomposite.add_widget(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button12.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x24\x7b\x08\x32\xf4\x5e\xc9\x0f\xb1\x2a"
        "\x6e\x92\x95\x3a\x95\xe0\x16\x21\xd7\xef"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "button13.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x0a\x0e\x79\xd2\x3f\xfc\xd7\x2d\x3b\x95"
        "\x4e\xa5\x8c\x4b\x7e\xe8\xc0\xd5\xbd\xf8"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wcomposite.clear();
}


BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonFocus)
{
    TestDraw drawable(70, 40);

    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test7", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button14.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8e\xfa\x2a\xed\x3e\xe2\xa4\xac\xdb\xe3"
        "\xcd\x5e\xb4\xde\xd7\x71\xed\x72\x70\x16"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button15.png");

    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x4e\x56\x78\x23\xe6\x8f\xb2\xac\x4d\xe7"
    //     "\x7d\x10\xe9\x62\x46\x33\x6b\x92\x17\xe9"
    //    )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3b\xae\x86\x5e\xf7\xb4\xbe\x36\x09\x24"
                   "\x43\xc7\x04\x61\x4b\xac\x18\xce\x9f\xfb"
       )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button16.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x8e\xfa\x2a\xed\x3e\xe2\xa4\xac\xdb\xe3"
        "\xcd\x5e\xb4\xde\xd7\x71\xed\x72\x70\x16"
   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "button17.png");

    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x4e\x56\x78\x23\xe6\x8f\xb2\xac\x4d\xe7"
    //     "\x7d\x10\xe9\x62\x46\x33\x6b\x92\x17\xe9"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }

    if (!check_sig(drawable.gd.drawable, message,
                   "\x3b\xae\x86\x5e\xf7\xb4\xbe\x36\x09\x24"
                   "\x43\xc7\x04\x61\x4b\xac\x18\xce\x9f\xfb"
   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


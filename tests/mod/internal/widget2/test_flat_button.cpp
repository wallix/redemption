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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test1", auto_resize, id, fg_color, bg_color, fc_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    // drawable.save_to_png(OUTPUT_FILE_PATH "button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x22\xb2\x48\xa5\x9e\x42\xcb\x4d\x38\x7c"
                   "\x41\x1a\x59\x5a\xa5\xe3\x70\x8d\xe8\x00"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test2", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa2\x7c\xc5\x08\x48\x37\x41\x27\xed\x6a"
                   "\x02\xae\xc7\xad\xaf\x82\xbb\xba\x18\xe5"
                   )){
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test3", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x2a\xb3\x83\x3b\xc1\xca\x5a\x26\x5f\x1f"
                   "\x99\xc2\xe4\x29\xad\x54\xda\xe1\x17\xc6"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test4", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x99\x52\xc1\x9c\x18\x29\xce\x7d\x8f\x41"
                   "\x76\xd2\xda\xd8\x0b\x82\x98\x08\xd5\x40"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test5", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xc3\x04\x0e\xbc\x75\x79\xa9\xf3\x61\xba"
                   "\x38\x3f\x2e\x39\xae\x3d\x39\xc4\x2c\xf3"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x4a\x1a\x16\x53\xe4\x66\x41\x52\x03\xb6"
                   "\x93\xac\xfe\x4a\xc3\xe6\x67\x03\x59\xdf"
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
    int fc_color = WINBLUE;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xdc\x95\xcb\x86\xab\x12\x48\x0a\xd4\xbd"
                   "\x83\x49\xea\x97\x76\x3d\x06\xb5\xa7\xb5"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, fc_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf2\x68\xec\x64\x6a\xeb\xb7\xbd\x1b\x18"
                   "\x09\x0b\x23\x0d\x1e\xb1\xf6\xe9\xc9\x3c"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, fc_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x56\xc7\x70\x37\xd5\x53\xf4\x15\x1e\xe0"
                   "\xa0\x6b\x56\xe8\x92\x8a\x96\xd1\x66\x5f"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x7c\xd2\xea\xd3\xc0\xb9\x8b\xfb\xc5\x48"
                   "\x42\xf2\xc4\x24\xf7\xd6\x44\x23\xf2\x89"
       )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button11.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x56\xc7\x70\x37\xd5\x53\xf4\x15\x1e\xe0"
                   "\xa0\x6b\x56\xe8\x92\x8a\x96\xd1\x66\x5f"
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

    WidgetFlatButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize, 0, WHITE,
                             DARK_BLUE_BIS, WINBLUE);

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
                              "abababab", true, 0, YELLOW, BLACK, WINBLUE);
    WidgetFlatButton wbutton2(drawable, 0,100, wcomposite, notifier,
                              "ggghdgh", true, 0, WHITE, RED, WINBLUE);
    WidgetFlatButton wbutton3(drawable, 100,100, wcomposite, notifier,
                              "lldlslql", true, 0, BLUE, RED, WINBLUE);
    WidgetFlatButton wbutton4(drawable, 300,300, wcomposite, notifier,
                              "LLLLMLLM", true, 0, PINK, DARK_GREEN, WINBLUE);
    WidgetFlatButton wbutton5(drawable, 700,-10, wcomposite, notifier,
                              "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE, WINBLUE);
    WidgetFlatButton wbutton6(drawable, -10,550, wcomposite, notifier,
                              "xxwwp", true, 0, ANTHRACITE, PALE_GREEN, WINBLUE);

    wcomposite.add_widget(&wbutton1);
    wcomposite.add_widget(&wbutton2);
    wcomposite.add_widget(&wbutton3);
    wcomposite.add_widget(&wbutton4);
    wcomposite.add_widget(&wbutton5);
    wcomposite.add_widget(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button12.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x18\xe4\x82\xd0\xdf\x7a\x91\x84\xa7\xf9"
                   "\x7b\xdc\x23\x4d\x0e\x76\xb7\xf9\x32\xf2"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button13.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x93\x90\xf9\x17\xe0\x22\x00\xa6\x96\x8a"
                   "\x27\xc2\xca\x8b\x90\x41\x81\x92\x78\xbd"
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
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test7", auto_resize, id, fg_color, bg_color, fc_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button14.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb5\x54\x4b\x09\x1f\x05\xd6\x5f\x63\xcf"
                   "\x33\xef\x1b\xa1\x1a\x4e\xab\x72\x7a\x55"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button15.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xf7\xe1\x60\x46\x35\xe1\xaf\x36\x1b\x8f"
                   "\x22\x79\x89\x46\x7b\x35\xa9\xaf\xcc\xba"
       )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button16.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xb5\x54\x4b\x09\x1f\x05\xd6\x5f\x63\xcf"
                   "\x33\xef\x1b\xa1\x1a\x4e\xab\x72\x7a\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button17.png");


    if (!check_sig(drawable.gd.drawable, message,
                   "\xf7\xe1\x60\x46\x35\xe1\xaf\x36\x1b\x8f"
                   "\x22\x79\x89\x46\x7b\x35\xa9\xaf\xcc\xba"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}


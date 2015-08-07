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

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "font.hpp"
#include "internal/widget2/flat_button.hpp"
#include "internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test1", auto_resize, id,
                             fg_color, bg_color, fc_color, font, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x97\x74\x69\x41\x37\x9c\x8d\xa6\x89\x14"
        "\x88\x63\x79\x8e\x3f\xae\x3c\x22\xef\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test2", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\x01\x08\x71\xcc\x4f\xfb\x43\x6a\xec"
        "\xf7\xbd\x43\x10\xa0\x4b\x21\x44\x8a\x3f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test3", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x30\xfc\xe0\x34\xff\x78\x9b\x9e\x96\x6d"
        "\xf0\xa6\x9d\x15\xff\x0a\x1b\x10\xb6\xc0"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test4", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x6f\xb0\x8d\x93\xaf\xfb\x71\x11\x65\x4a"
        "\xf4\x68\x8f\xaa\xf0\x67\xb3\xf5\x10\x19"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test5", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x0a\x74\xd0\x5a\xe7\x29\x61\x18\x0c\x73"
        "\xef\x7d\xf3\x87\x9e\x79\xb4\x3d\x76\x08"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButton6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xca\x98\xb2\x78\x2e\xbc\xb4\xfb\xbf\x00"
        "\x7d\x3d\x38\x59\x77\x4e\xa6\x32\xaf\x0b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int fc_color = WINBLUE;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x65\x01\x76\x90\x19\x3f\x9d\x67\xa0\xae"
        "\xdc\x0e\xc9\xf8\x10\x5b\xae\xc4\xfb\x70"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id,
                             fg_color, bg_color, fc_color, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x3c\x4c\x75\x45\x7c\xfc\x6f\xc7\x43\x99"
        "\x28\x6e\xc2\x38\xcc\x14\x81\xf7\xee\x3c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id,
                             fg_color, bg_color, fc_color, font, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x63\xf6\xb9\xf7\xea\x5d\xaf\xaa\x5f\x9b"
        "\x4d\xd9\x59\xbc\x32\x86\x8c\x6f\x7b\x4f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button10.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x6b\x2b\x42\xf0\xd0\x72\xef\x60\xa5\x10"
        "\xf2\xb3\x20\x2e\x7f\xa2\x8f\xd0\xf2\xc0"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, nullptr);
    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button11.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x63\xf6\xb9\xf7\xea\x5d\xaf\xaa\x5f\x9b"
        "\x4d\xd9\x59\xbc\x32\x86\x8c\x6f\x7b\x4f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonEvent)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, nullptr)
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
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;
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

    Font font;

    WidgetFlatButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize, 0, WHITE,
                             DARK_BLUE_BIS, WINBLUE, font);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char('a');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char(' ');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetFlatButton is a button widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetFlatButton wbutton1(drawable, 0,0, wcomposite, notifier,
                              "abababab", true, 0, YELLOW, BLACK, WINBLUE, font);
    WidgetFlatButton wbutton2(drawable, 0,100, wcomposite, notifier,
                              "ggghdgh", true, 0, WHITE, RED, WINBLUE, font);
    WidgetFlatButton wbutton3(drawable, 100,100, wcomposite, notifier,
                              "lldlslql", true, 0, BLUE, RED, WINBLUE, font);
    WidgetFlatButton wbutton4(drawable, 300,300, wcomposite, notifier,
                              "LLLLMLLM", true, 0, PINK, DARK_GREEN, WINBLUE, font);
    WidgetFlatButton wbutton5(drawable, 700,-10, wcomposite, notifier,
                              "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE, WINBLUE, font);
    WidgetFlatButton wbutton6(drawable, -10,550, wcomposite, notifier,
                              "xxwwp", true, 0, ANTHRACITE, PALE_GREEN, WINBLUE, font);

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
    if (!check_sig(drawable.gd.impl(), message,
        "\xd2\xb7\x90\xff\xad\xe1\xd3\x5b\x20\x59"
        "\x14\x83\x8e\x0d\xbf\xd0\x9a\xd7\x3a\xe5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "button13.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x4b\xd7\x1c\xe2\x2b\x46\x34\xd7\xdb\x56"
        "\xd7\x64\x03\xba\xbf\xd7\xfd\xc9\x13\xa8"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wcomposite.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetFlatButtonFocus)
{
    TestDraw drawable(70, 40);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int fg_color = RED;
    int bg_color = YELLOW;
    int fc_color = WINBLUE;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetFlatButton wbutton(drawable, x, y, parent, notifier, "test7", auto_resize, id,
                             fg_color, bg_color, fc_color, font, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button14.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf4\xb4\xb3\xcd\x42\x49\xd9\x38\x38\xd8"
        "\x25\x94\x25\x37\x96\xe6\xea\x76\xca\x6f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.focus(Widget2::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button15.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xd1\x14\x95\x71\x12\x28\x07\xf5\x8d\x5a"
        "\x43\xbe\x26\x41\xd3\x0d\xfb\x13\x4f\xbf"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }



    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button16.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xf4\xb4\xb3\xcd\x42\x49\xd9\x38\x38\xd8"
        "\x25\x94\x25\x37\x96\xe6\xea\x76\xca\x6f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    wbutton.focus(Widget2::focus_reason_tabkey);

    wbutton.rdp_input_invalidate(wbutton.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "button17.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\xd1\x14\x95\x71\x12\x28\x07\xf5\x8d\x5a"
        "\x43\xbe\x26\x41\xd3\x0d\xfb\x13\x4f\xbf"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

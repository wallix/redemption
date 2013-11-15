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
#define BOOST_TEST_MODULE TestWidgetPassword
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/password.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, password_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dx(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x17\x50\x9f\x13\xed\xc3\x94\x32\x44\xde"
                   "\x0d\xe1\x60\x8c\x66\x82\x23\x81\x21\xfc"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x60\x58\xc8\x31\x30\xc3\x79\xa8\x6a\x5c"
                   "\x12\x1c\x52\xff\xde\x4a\x8b\xd5\x05\xb7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x93\x95\x0e\x86\x60\x2e\x7a\x9e\x04\xc6"
                   "\x5c\xac\x01\xcb\xf3\x45\xd5\x20\x02\xa9"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb2\x98\x00\xfd\x27\x76\x33\x9a\xeb\x36"
                   "\xde\x19\xce\xb5\xac\x24\xac\x16\x58\xc0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xca\xa2\x4c\xda\xb7\xc6\x0a\x44\xd2\x95"
                   "\x7f\x0c\x93\x7b\x94\x16\x05\xe3\x09\x5b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x47\x59\xa0\xd5\x17\x82\x9a\x89\x9e\x9d"
                   "\x05\x42\x6e\xb6\xf8\xed\x65\xec\x0b\xb2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf4\x55\x76\xa1\x8a\xfe\x53\x81\xc2\xea"
                   "\x0a\xa1\x86\x01\x60\xe9\xf0\x4a\x3d\x54"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    5 + wpassword.dy(),
                                    30,
                                    10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x91\xe4\x76\x91\x59\xf4\x56\x1a\xe2\x80"
        "\xb3\xf4\x6c\x7e\x56\x69\xe0\xdb\xa6\x70")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
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

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "abcdef", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x0e\x02\xe4\xe8\x81\xbe\x78\x2c\x64\x2b"
                   "\xce\xe1\x71\x54\xd4\xef\xf9\x2b\xcc\xc2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);
    bool    ctrl_alt_delete;

    keymap.event(0, 16, decoded_data, ctrl_alt_delete); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x49\xee\x68\xc3\x45\x89\x15\x0e\x2a"
                   "\x09\x1a\x77\xde\xab\x85\x0c\xcd\x48\x9c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17, decoded_data, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x03\x3d\x1c\x55\xd3\x26\x5f\x5f\x74\xa0"
                   "\x75\x8d\x62\x39\x14\xfd\x36\x0d\x5d\x01"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd7\x22\x2d\x70\x65\x02\x7b\xe2\x48\x46"
                   "\x74\x4b\xc3\xd6\x67\x89\x37\xed\x13\x97"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x03\x3d\x1c\x55\xd3\x26\x5f\x5f\x74\xa0"
                   "\x75\x8d\x62\x39\x14\xfd\x36\x0d\x5d\x01"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x49\xee\x68\xc3\x45\x89\x15\x0e\x2a"
                   "\x09\x1a\x77\xde\xab\x85\x0c\xcd\x48\x9c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd0\x52\xf3\x7b\x7d\x7a\xa7\xb3\x16\x5b"
                   "\xb7\xd3\xde\xe4\xd7\x46\xff\x36\xd1\xc8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x64\xbd\x3d\x98\x09\x70\xe3\x65\x26"
                   "\x1a\xf7\x82\x03\x25\xd1\x93\x03\x41\xef"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x2d\xb5\x25\x34\x8b\xa0\x68\xe1\x7b\x41"
                   "\xe1\x9f\x13\x3b\xe3\xe7\xd5\xe0\x46\x4e"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x0e\x02\xe4\xe8\x81\xbe\x78\x2c\x64\x2b"
                   "\xce\xe1\x71\x54\xd4\xef\xf9\x2b\xcc\xc2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x13\x76\xb5\x26\x1d\x0b\x8e\x9e\xd9\xcc"
                   "\xe2\xca\x61\xc2\x24\x2a\x25\x72\xc6\x3b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

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

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = 0;
    notifier.event = 0;
    widget_for_receive_event.sender = 0;
    widget_for_receive_event.event = 0;

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit-e11.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x04\x55\xc8\x7f\xf3\x3b\x5d\xda\x0b\xfd"
                   "\x86\xf6\x5f\x8b\xcd\x87\xa4\xd1\xa0\x99"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetPassword wpassword1(drawable, 0,0, 50, wcomposite, notifier,
                        "abababab", 4, YELLOW, BLACK);
    WidgetPassword wpassword2(drawable, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED);
    WidgetPassword wpassword3(drawable, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED);
    WidgetPassword wpassword4(drawable, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN);
    WidgetPassword wpassword5(drawable, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
    WidgetPassword wpassword6(drawable, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, DARK_GREY, PALE_GREEN);

    wcomposite.add_widget(&wpassword1);
    wcomposite.add_widget(&wpassword2);
    wcomposite.add_widget(&wpassword3);
    wcomposite.add_widget(&wpassword4);
    wcomposite.add_widget(&wpassword5);
    wcomposite.add_widget(&wpassword6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password-compo1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x74\xc1\x7c\xfc\xda\x78\x4d\x41\x32\xd9"
                   "\x4f\x57\xbd\x4d\xf2\xba\x36\x99\x5b\xce"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password-compo2.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x25\xd1\x91\xb9\x3e\xde\x79\x3d\x3a\x63"
                   "\xb4\x4d\x50\x92\xed\xc3\xd2\x7a\xc8\xa7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
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

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x49\xee\x68\xc3\x45\x89\x15\x0e\x2a"
                   "\x09\x1a\x77\xde\xab\x85\x0c\xcd\x48\x9c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd0\x52\xf3\x7b\x7d\x7a\xa7\xb3\x16\x5b"
                   "\xb7\xd3\xde\xe4\xd7\x46\xff\x36\xd1\xc8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x64\xbd\x3d\x98\x09\x70\xe3\x65\x26"
                   "\x1a\xf7\x82\x03\x25\xd1\x93\x03\x41\xef"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\x17\x22\xe0\xf0\xcd\xc0\x43\xfa\x0b"
                   "\x24\x69\x46\xaf\x15\x06\x06\x3c\x2e\x1f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x69\x1c\x6b\x24\xf6\x4c\xe9\xc5\x27\x46"
                   "\x4b\x40\xfe\x56\xdc\x15\x4a\xb0\x44\x08"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("******"), std::string(wpassword.show_text()));
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword2)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
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

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x49\xee\x68\xc3\x45\x89\x15\x0e\x2a"
                   "\x09\x1a\x77\xde\xab\x85\x0c\xcd\x48\x9c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd0\x52\xf3\x7b\x7d\x7a\xa7\xb3\x16\x5b"
                   "\xb7\xd3\xde\xe4\xd7\x46\xff\x36\xd1\xc8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x64\xbd\x3d\x98\x09\x70\xe3\x65\x26"
                   "\x1a\xf7\x82\x03\x25\xd1\x93\x03\x41\xef"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\x17\x22\xe0\xf0\xcd\xc0\x43\xfa\x0b"
                   "\x24\x69\x46\xaf\x15\x06\x06\x3c\x2e\x1f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd6\x88\x76\x17\x62\x0f\x0b\x75\x5a\x4f"
                   "\x68\x57\x81\x54\xac\x29\x60\xff\x32\x2c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x69\x1c\x6b\x24\xf6\x4c\xe9\xc5\x27\x46"
                   "\x4b\x40\xfe\x56\xdc\x15\x4a\xb0\x44\x08"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("******"), std::string(wpassword.show_text()));
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword3)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
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

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9e\x49\xee\x68\xc3\x45\x89\x15\x0e\x2a"
                   "\x09\x1a\x77\xde\xab\x85\x0c\xcd\x48\x9c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd0\x52\xf3\x7b\x7d\x7a\xa7\xb3\x16\x5b"
                   "\xb7\xd3\xde\xe4\xd7\x46\xff\x36\xd1\xc8"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x64\xbd\x3d\x98\x09\x70\xe3\x65\x26"
                   "\x1a\xf7\x82\x03\x25\xd1\x93\x03\x41\xef"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\x17\x22\xe0\xf0\xcd\xc0\x43\xfa\x0b"
                   "\x24\x69\x46\xaf\x15\x06\x06\x3c\x2e\x1f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd6\x88\x76\x17\x62\x0f\x0b\x75\x5a\x4f"
                   "\x68\x57\x81\x54\xac\x29\x60\xff\x32\x2c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3e\x17\x22\xe0\xf0\xcd\xc0\x43\xfa\x0b"
                   "\x24\x69\x46\xaf\x15\x06\x06\x3c\x2e\x1f"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    BStream decoded_data(256);
    bool    ctrl_alt_delete;

    keymap.event(0, 17, decoded_data, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x95\xde\x58\x8e\x03\x59\x09\xa5\x07\x9b"
                   "\xfe\xc2\x63\x95\x26\x27\x04\xdb\xcd\x9a"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurézlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("********"), std::string(wpassword.show_text()));
}

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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/password.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, 0x000000, font, password_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.x(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc8\x1f\x45\x66\x3e\x34\x8b\x06\x3c\xb6"
        "\xee\x82\x10\x46\x6a\x5a\x47\x18\x99\x87"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x1a\x26\x76\x95\xbe\x57\xdc\xb4\x1f\x65"
        "\x63\xba\x0f\x96\xbd\x6b\xe0\x6c\x2d\x1e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc8\xf2\xa4\x65\x27\xcb\x5d\x03\x43\xfa"
        "\xe3\xc6\x14\x3a\x82\x62\x3a\xab\xb5\xe5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password4.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf6\xb5\x23\x51\xdf\x86\x30\xe0\xf4\x10"
        "\x33\x8b\xd6\x27\x93\x0f\x34\x4f\x1d\x30"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xfc\x7e\x1a\x92\x3c\x21\x08\xdb\x42\x08"
        "\x6f\x64\x50\x7f\x59\xeb\x2e\x0e\x2e\x5b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password6.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x95\x39\x49\x96\x2c\xaf\xa5\xf6\xda\x7b"
        "\x7f\x44\x3c\xe4\x9b\x18\x15\xe0\x2f\x37"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc8\x9e\x4b\xb8\x2e\xf3\x1e\x49\x8c\x55"
        "\x27\x82\x2e\x3f\xea\x21\x87\x4e\x10\x86"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.x(),
                                    5 + wpassword.y(),
                                    30,
                                    10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x01\x5a\x19\xfb\xe0\x0c\xc1\xf4\x80\x76"
        "\x48\x8c\x49\xdf\x55\xd2\xa7\x60\xfa\x8f"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, &notifier, "abcdef", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xa4\x16\xd5\xb6\xcb\x65\x37\x92\x27\x22"
        "\x5a\x1e\xab\x58\x45\x3b\x75\x52\xcc\xfe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    bool    ctrl_alt_delete;

    keymap.event(0, 16, ctrl_alt_delete); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-1.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xdb\x84\x3f\x57\xbf\x39\xb8\x39\xa2"
        "\xcc\xbe\xea\xd0\xbc\xf0\x2c\x11\x4a\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = nullptr;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x64\x49\xed\x23\xbd\x5a\xbc\xd5\xc9\x76"
        "\x5a\x11\x28\x1f\x6e\x1f\x12\x0b\x8a\x5b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = nullptr;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xdd\x53\x49\x07\xcc\xf1\x15\x5b\x43\x07"
        "\x80\xa6\x7e\x3b\x9a\xda\xa2\xa1\x90\x76"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x64\x49\xed\x23\xbd\x5a\xbc\xd5\xc9\x76"
        "\x5a\x11\x28\x1f\x6e\x1f\x12\x0b\x8a\x5b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xdb\x84\x3f\x57\xbf\x39\xb8\x39\xa2"
        "\xcc\xbe\xea\xd0\xbc\xf0\x2c\x11\x4a\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x62\xef\x90\xb4\x8f\xda\xfd\x4b\x56\x4c"
        "\x84\x7f\x01\x54\xcf\x9e\xad\xc0\xfb\x4c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e7.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\xea\xed\xa0\x93\xda\x7e\xd0\x68\x16"
        "\x04\xbc\x0e\x21\x47\xfd\x5d\x46\xe7\x17"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e8.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x31\x2e\xaa\xc8\x8b\xcd\xde\xdd\xf5\xb3"
        "\xf8\x1a\x30\xd7\xf3\x9f\xb2\xdd\x87\x43"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e9.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xa4\x16\xd5\xb6\xcb\x65\x37\x92\x27\x22"
        "\x5a\x1e\xab\x58\x45\x3b\x75\x52\xcc\xfe"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e10.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xfe\x52\xca\x2b\xcf\xbf\xb2\xf7\x2a\xc2"
        "\x4e\x40\x85\x7e\x01\xcf\xaa\x56\x0f\x2c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable.gd, Rect(), *this, nullptr)
        {}

        void draw(const Rect&) override
        {}

        void notify(Widget2* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3, nullptr);
    BOOST_CHECK(widget_for_receive_event.sender == nullptr);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;
    widget_for_receive_event.sender = nullptr;
    widget_for_receive_event.event = 0;

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit-e11.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x54\x98\xf6\xc1\x37\x71\x20\xbd\xdf\x40"
        "\xf6\x93\x40\xde\x80\x3c\x05\xe9\x09\x90"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, Rect(0,0,800,600), parent, notifier);

    WidgetPassword wpassword1(drawable.gd, 0,0, 50, wcomposite, notifier,
                        "abababab", 4, YELLOW, BLACK, BLACK, font);
    WidgetPassword wpassword2(drawable.gd, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED, RED, font);
    WidgetPassword wpassword3(drawable.gd, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED, RED, font);
    WidgetPassword wpassword4(drawable.gd, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, font);
    WidgetPassword wpassword5(drawable.gd, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, font);
    WidgetPassword wpassword6(drawable.gd, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, font);

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
    if (!check_sig(drawable.gd.impl(), message,
        "\xeb\x4d\x10\xeb\xd8\x5a\x67\xe9\x82\x28"
        "\xc9\x8e\x90\x00\x3f\x86\x6d\x96\xcc\x04"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password-compo2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xfa\x2c\x22\x28\x37\xcb\x69\xe5\x1f\x17"
        "\xf7\xbf\x99\x78\x90\xa4\x0d\x7b\xd6\xe9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xdb\x84\x3f\x57\xbf\x39\xb8\x39\xa2"
        "\xcc\xbe\xea\xd0\xbc\xf0\x2c\x11\x4a\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x62\xef\x90\xb4\x8f\xda\xfd\x4b\x56\x4c"
        "\x84\x7f\x01\x54\xcf\x9e\xad\xc0\xfb\x4c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\xea\xed\xa0\x93\xda\x7e\xd0\x68\x16"
        "\x04\xbc\x0e\x21\x47\xfd\x5d\x46\xe7\x17"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xbd\x91\x1c\x44\x72\x2b\x53\x5f\xb9\x7d"
        "\x0e\x71\xf7\xe9\x41\xf2\x76\x9b\x68\xde"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x13\x24\x02\x11\x2d\x0f\x2a\x45\x93\x1a"
        "\x6e\xd3\xe6\x6d\xba\x81\x52\x5f\xc4\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), wpassword.get_text());
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword2)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xdb\x84\x3f\x57\xbf\x39\xb8\x39\xa2"
        "\xcc\xbe\xea\xd0\xbc\xf0\x2c\x11\x4a\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x62\xef\x90\xb4\x8f\xda\xfd\x4b\x56\x4c"
        "\x84\x7f\x01\x54\xcf\x9e\xad\xc0\xfb\x4c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\xea\xed\xa0\x93\xda\x7e\xd0\x68\x16"
        "\x04\xbc\x0e\x21\x47\xfd\x5d\x46\xe7\x17"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xbd\x91\x1c\x44\x72\x2b\x53\x5f\xb9\x7d"
        "\x0e\x71\xf7\xe9\x41\xf2\x76\x9b\x68\xde"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x68\xe7\x23\xc4\x37\x00\xb1\xf3\x09\xac"
        "\xa3\x9e\x1c\x48\x80\x86\xfd\x95\x26\xfc"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x13\x24\x02\x11\x2d\x0f\x2a\x45\x93\x1a"
        "\x6e\xd3\xe6\x6d\xba\x81\x52\x5f\xc4\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), wpassword.get_text());
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword3)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc4\xdb\x84\x3f\x57\xbf\x39\xb8\x39\xa2"
        "\xcc\xbe\xea\xd0\xbc\xf0\x2c\x11\x4a\x8a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = nullptr;
    notifier.event = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), wpassword.get_text());

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x62\xef\x90\xb4\x8f\xda\xfd\x4b\x56\x4c"
        "\x84\x7f\x01\x54\xcf\x9e\xad\xc0\xfb\x4c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xf9\xea\xed\xa0\x93\xda\x7e\xd0\x68\x16"
        "\x04\xbc\x0e\x21\x47\xfd\x5d\x46\xe7\x17"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e4.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xbd\x91\x1c\x44\x72\x2b\x53\x5f\xb9\x7d"
        "\x0e\x71\xf7\xe9\x41\xf2\x76\x9b\x68\xde"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x68\xe7\x23\xc4\x37\x00\xb1\xf3\x09\xac"
        "\xa3\x9e\x1c\x48\x80\x86\xfd\x95\x26\xfc"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xbd\x91\x1c\x44\x72\x2b\x53\x5f\xb9\x7d"
        "\x0e\x71\xf7\xe9\x41\xf2\x76\x9b\x68\xde"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);


    bool    ctrl_alt_delete;

    keymap.event(0, 17, ctrl_alt_delete); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, ctrl_alt_delete);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e7.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x19\x91\x4b\x51\x49\x89\x0f\xff\x38\x7c"
        "\x23\x69\xe4\x77\x7e\x48\x8d\x61\x3b\xd5"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    BOOST_CHECK_EQUAL(std::string("aurézlie"), wpassword.get_text());
}

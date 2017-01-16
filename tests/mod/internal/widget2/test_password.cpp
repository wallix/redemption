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
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

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

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test1", id,
                     fg_color, bg_color, 0x000000, font, password_pos, xtext, ytext);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.x(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xd8\xad\x77\x6e\x88\x33\xf8\x46\x94\xc2"
        "\x25\x7a\x6c\xd3\x2b\x23\x85\xc3\xaa\x37"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test2", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x82\xb6\x09\x58\x6d\xf1\x83\x79\x5f\x63"
        "\xad\x37\x1b\x44\xd2\x2a\x58\x80\x4b\xbb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test3", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xd8\x4e\x56\xf4\x75\x0c\x6e\x18\x91\xcd"
        "\x1d\xe9\x2b\x8a\x9c\x78\x4f\x93\x6b\xe4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test4", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

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
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test5", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.x(),
                                    0 + wpassword.y(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xa2\xad\x76\xa7\xa5\xb1\x12\xb9\xab\xc7"
        "\xcf\x69\xcb\xf2\xd9\xbf\x8b\x0b\xf5\x30"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

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
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

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
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable.gd, parent, notifier, "test6", id, fg_color, bg_color, 0x000000, font, 0);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

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

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, parent, &notifier, "abcdef", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x9d\xac\xdb\x03\xd9\x1a\x90\x59\xcb\x54"
        "\x05\x7c\x37\x17\x0e\x89\x61\x5c\x7d\xef"
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
        "\x99\x24\xc3\xf0\x32\x4f\xd0\x02\x58\x0d"
        "\xf1\xfe\x25\xee\x23\xf5\xc5\xa7\x0f\x45"
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
        "\x22\xe4\xaa\x93\x0f\x5a\xd2\x84\xdd\xb3"
        "\x6e\xcd\x3c\x3e\xec\x9e\x3d\x28\x04\xb4"
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
        "\x3d\x0e\x76\x9c\x94\x32\xc6\x5f\x1a\xbd"
        "\x19\x74\xc7\xb1\x86\x10\xa6\x9c\x24\xab"
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
        "\x22\xe4\xaa\x93\x0f\x5a\xd2\x84\xdd\xb3"
        "\x6e\xcd\x3c\x3e\xec\x9e\x3d\x28\x04\xb4"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e5.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x24\xc3\xf0\x32\x4f\xd0\x02\x58\x0d"
        "\xf1\xfe\x25\xee\x23\xf5\xc5\xa7\x0f\x45"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e6.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x60\x5f\x3b\x16\x88\x6f\x2a\x54\x7b\x90"
        "\xa4\x92\xbc\xf5\xef\xda\xd6\xab\x1a\x12"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e7.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\xb8\x86\xab\x4d\x6c\xb6\x02\x86\xb5\xef"
        "\xfb\x4a\x7c\x0d\x2f\x06\x26\x9c\x5e\xbe"
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
        "\xd3\xf2\xea\xb4\x7e\x2c\xab\xc0\x1d\x29"
        "\xd0\xbe\x2d\x76\x0e\x55\x7e\x60\x2e\x9f"
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
        "\x9d\xac\xdb\x03\xd9\x1a\x90\x59\xcb\x54"
        "\x05\x7c\x37\x17\x0e\x89\x61\x5c\x7d\xef"
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
        "\xe5\xff\x0a\x7d\xdc\xbd\xf3\x3e\xec\x3a"
        "\xaf\x0c\x30\x7d\x9e\xce\xfc\xc5\x6f\xcb"
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
        : Widget2(drawable.gd, *this, nullptr)
        {}

        void rdp_input_invalidate(Rect) override
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
        "\x6e\x4f\xf7\x07\x18\x16\x10\x89\xb3\xf4"
        "\xb1\x43\xb6\x45\x53\x82\x1d\x74\x7c\x9e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    WidgetComposite wcomposite(drawable.gd, parent, notifier);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetPassword wpassword1(drawable.gd, wcomposite, notifier,
                              "abababab", 4, YELLOW, BLACK, BLACK, font);
    Dimension dim = wpassword1.get_optimal_dim();
    wpassword1.set_wh(50, dim.h);
    wpassword1.set_xy(0, 0);

    WidgetPassword wpassword2(drawable.gd, wcomposite, notifier,
                              "ggghdgh", 2, WHITE, RED, RED, font);
    dim = wpassword2.get_optimal_dim();
    wpassword2.set_wh(50, dim.h);
    wpassword2.set_xy(0, 100);

    WidgetPassword wpassword3(drawable.gd, wcomposite, notifier,
                              "lldlslql", 1, BLUE, RED, RED, font);
    dim = wpassword3.get_optimal_dim();
    wpassword3.set_wh(50, dim.h);
    wpassword3.set_xy(100, 100);

    WidgetPassword wpassword4(drawable.gd, wcomposite, notifier,
                              "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN, font);
    dim = wpassword4.get_optimal_dim();
    wpassword4.set_wh(50, dim.h);
    wpassword4.set_xy(300, 300);

    WidgetPassword wpassword5(drawable.gd, wcomposite, notifier,
                              "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, font);
    dim = wpassword5.get_optimal_dim();
    wpassword5.set_wh(50, dim.h);
    wpassword5.set_xy(700, -10);

    WidgetPassword wpassword6(drawable.gd, wcomposite, notifier,
                              "xxwwp", 2, ANTHRACITE, PALE_GREEN, PALE_GREEN, font);
    dim = wpassword6.get_optimal_dim();
    wpassword6.set_wh(50, dim.h);
    wpassword6.set_xy(-10, 550);

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
        "\x27\x65\x28\x0e\x2c\x89\xf7\x46\xb7\x8d"
        "\x6c\xd5\x3f\x14\x0c\xb3\x37\x38\xd5\x50"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "password-compo2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\x2a\xe5\xd1\x8a\x45\x83\xc4\x16\x9f\xee"
        "\x5e\x3d\x1b\xfc\x27\xe2\x57\xe5\x37\x13"
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

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x24\xc3\xf0\x32\x4f\xd0\x02\x58\x0d"
        "\xf1\xfe\x25\xee\x23\xf5\xc5\xa7\x0f\x45"
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
        "\x60\x5f\x3b\x16\x88\x6f\x2a\x54\x7b\x90"
        "\xa4\x92\xbc\xf5\xef\xda\xd6\xab\x1a\x12"
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
        "\xb8\x86\xab\x4d\x6c\xb6\x02\x86\xb5\xef"
        "\xfb\x4a\x7c\x0d\x2f\x06\x26\x9c\x5e\xbe"
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
        "\x06\x52\x0b\x43\x8c\x4d\xbc\xf8\xbb\x7a"
        "\xd9\x57\xac\xa1\x36\x21\xbb\xe8\x21\xa9"
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
        "\x6d\xae\xd9\x53\x39\xa0\x35\x13\x2c\x14"
        "\x5d\x71\x6f\x65\x29\x11\xf1\x13\x34\x0b"
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

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x24\xc3\xf0\x32\x4f\xd0\x02\x58\x0d"
        "\xf1\xfe\x25\xee\x23\xf5\xc5\xa7\x0f\x45"
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
        "\x60\x5f\x3b\x16\x88\x6f\x2a\x54\x7b\x90"
        "\xa4\x92\xbc\xf5\xef\xda\xd6\xab\x1a\x12"
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
        "\xb8\x86\xab\x4d\x6c\xb6\x02\x86\xb5\xef"
        "\xfb\x4a\x7c\x0d\x2f\x06\x26\x9c\x5e\xbe"
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
        "\x06\x52\x0b\x43\x8c\x4d\xbc\xf8\xbb\x7a"
        "\xd9\x57\xac\xa1\x36\x21\xbb\xe8\x21\xa9"
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
        "\x79\x39\x0b\xd4\xad\xbc\x27\x90\xa0\xb8"
        "\x78\x9e\xcd\x89\xf9\x6c\x52\x37\x52\x45"
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
        "\x6d\xae\xd9\x53\x39\xa0\x35\x13\x2c\x14"
        "\x5d\x71\x6f\x65\x29\x11\xf1\x13\x34\x0b"
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

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable.gd, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF, 0x0000FF, font);
    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(cx, dim.h);
    wpassword.set_xy(x, y);

    wpassword.focus(Widget2::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x99\x24\xc3\xf0\x32\x4f\xd0\x02\x58\x0d"
        "\xf1\xfe\x25\xee\x23\xf5\xc5\xa7\x0f\x45"
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
        "\x60\x5f\x3b\x16\x88\x6f\x2a\x54\x7b\x90"
        "\xa4\x92\xbc\xf5\xef\xda\xd6\xab\x1a\x12"
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
        "\xb8\x86\xab\x4d\x6c\xb6\x02\x86\xb5\xef"
        "\xfb\x4a\x7c\x0d\x2f\x06\x26\x9c\x5e\xbe"
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
        "\x06\x52\x0b\x43\x8c\x4d\xbc\xf8\xbb\x7a"
        "\xd9\x57\xac\xa1\x36\x21\xbb\xe8\x21\xa9"
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
        "\x79\x39\x0b\xd4\xad\xbc\x27\x90\xa0\xb8"
        "\x78\x9e\xcd\x89\xf9\x6c\x52\x37\x52\x45"
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
        "\x06\x52\x0b\x43\x8c\x4d\xbc\xf8\xbb\x7a"
        "\xd9\x57\xac\xa1\x36\x21\xbb\xe8\x21\xa9"
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
        "\xe5\x14\x42\x63\xef\x49\xab\xad\x11\x4f"
        "\x82\x27\xf6\xe1\xed\x12\x65\xff\x45\x47"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.sender = nullptr;
    notifier.event = 0;

    BOOST_CHECK_EQUAL(std::string("aurézlie"), wpassword.get_text());
}

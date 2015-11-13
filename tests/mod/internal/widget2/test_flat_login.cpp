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
#define BOOST_TEST_MODULE TestFlatLogin
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "internal/widget2/flat_login.hpp"
#include "internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int id = 0;
    std::function<void(int)> notifier_language;

    FlatLogin flat_login(drawable, parent.cx(), parent.cy(), parent, notifier, "test1",
                         false, id, "rec", "rec", "Login", "Password", "", notifier_language, font);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xd7\xee\xfc\x34\x1d\xda\x31\xe9\xd7\x1c\x2d\x3b\x9a\xdd\x3a\xd2\x50\xb1\xe3\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestDraw drawable(800, 600);
    std::function<void(int)> notifier_language;
    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test2",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc3\xef\x12\xd7\x91\x67\x95\xb1\x51\x17\xd3\xc8\xff\x34\x0b\x8b\x8e\x0b\x3c\x3b"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin3)
{
    TestDraw drawable(800, 600);
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    std::function<void(int)> notifier_language;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);

    FlatLogin flat_login(drawable, 800, 600, parent, &notifier, "test3",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    flat_login.set_widget_focus(&flat_login.password_edit, Widget2::focus_reason_tabkey);

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    BOOST_CHECK(notifier.sender == &flat_login);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login3.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
            "\x6b\x5b\xc4\xb8\x08\x8d\xa6\x0b\x96\x8f\x59\x54\x64\x4d\x89\x8d\xeb\x6a\x47\x5c"
   //     "\xa3\xa6\x18\xbd\x9b\x21\xa0\xf1\xea\xfe\x00\x0d\x15\xa1\x71\xae\x0c\x04\xc7\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    notifier.sender = nullptr;
    notifier.event = 0;
    keymap.push_kevent(Keymap2::KEVENT_ESC); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    BOOST_CHECK(notifier.sender == &flat_login);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginHelp)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    std::function<void(int)> notifier_language;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test4",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x6d\x36\x8b\x6e\xd7\x5c\x83\xc2\x61\x82\xfe\xac\xe8\x73\xeb\xc7\x19\x76\x33\x60"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.centerx(), flat_login.helpicon.centery(), nullptr);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xa8\x54\x3e\xcc\x6d\x3b\x92\x6b\xfa\x47\xe2\x7c\x81\x02\x19\x9a\x22\xc9\x1a\x37"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    std::function<void(int)> notifier_language;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x72\xf7\x63\xf8\x23\xd7\x95\xfe\xf7\x1f\xc6\xb1\xdd\x85\x1a\x06\x7d\x8b\xd5\xb9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    std::function<void(int)> notifier_language;

    FlatLogin flat_login(drawable, 800, 600, parent, notifier, "test6",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      5 + flat_login.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login8.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
                   "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1"
                   "\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, font);
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    std::function<void(int)> notifier_language;

    FlatLogin flat_login(drawable, 800, 600, parent, &notifier, "test6",
                         false, 0, nullptr, nullptr, "Login", "Password", "", notifier_language, font);

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
}

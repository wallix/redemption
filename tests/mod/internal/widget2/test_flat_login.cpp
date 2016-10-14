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
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/flat_login.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x9c\xd2\x4b\x03\x86\x61\x50\xe1\x51\x3f\xcf\x6f\x34\x73\x71\x6f\x20\x40\x60\x81"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestDraw drawable(800, 600);
    WidgetFlatButton * extra_button = nullptr;
    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test2",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x04\x87\x3f\xf6\x80\xa6\x9c\x08\xb5\x29\x02\x6a\xda\xbc\xcf\x69\x32\x5e\x9f\x73"
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

        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    WidgetFlatButton * extra_button = nullptr;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test3",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

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
        "\xf5\xc0\x0e\x8e\x21\xcf\x97\xbf\x74\xaf\x9e\xfe\x8c\xf3\xaa\x03\x46\xd0\x4f\x8a"
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
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test4",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(0 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xc6\xdc\xe5\xc0\x76\x61\xdb\xc0\x7d\x75\xd3\xd5\xea\x89\x9d\xac\x14\x01\x61\x00"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.centerx(), flat_login.helpicon.centery(), nullptr);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help2.png");

    if (!check_sig(drawable.gd.impl(), message,
        "\xfa\xec\x2e\x07\x1b\x9d\xd5\x86\xbd\xcb\x6d\xad\x6f\x11\xeb\x1a\xbd\x05\x30\xdc"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.dx(),
                                      0 + flat_login.dy(),
                                      flat_login.cx(),
                                      flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x16\x8c\x6e\x95\xa5\x76\xf8\xbc\x50\x4b\xff\xf2\x12\x9a\xda\x1a\xe7\x03\xf2\x74"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

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

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);
}

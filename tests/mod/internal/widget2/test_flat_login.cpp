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

#define RED_TEST_MODULE TestFlatLogin
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "mod/internal/widget2/flat_login.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    RED_CHECK_SIG(drawable.gd, "\xe1\x2b\x7f\xd9\x2d\x5f\xf3\xcf\x05\xd4\x77\x40\x02\xb9\x8f\x66\x7e\xdb\xc6\xb7");
}

RED_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestDraw drawable(800, 600);
    WidgetFlatButton * extra_button = nullptr;
    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test2",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    RED_CHECK_SIG(drawable.gd, "\xb7\xe9\xfc\xaf\x84\xd8\x49\x7d\xb4\x23\x82\x34\xab\x4d\x74\xcf\x59\x67\x7c\x91");
}

RED_AUTO_TEST_CASE(TraceFlatLogin3)
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
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test3",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    flat_login.set_widget_focus(&flat_login.password_edit, Widget2::focus_reason_tabkey);

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK(notifier.sender == &flat_login);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login3.png");

    RED_CHECK_SIG(drawable.gd, "\x5d\x90\xcb\x7a\x0a\xe1\xa6\x4e\x36\x4a\x96\xc5\x3a\x13\x30\x47\x12\xf0\xe6\xef");

    notifier.sender = nullptr;
    notifier.event = 0;
    keymap.push_kevent(Keymap2::KEVENT_ESC); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK(notifier.sender == &flat_login);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(TraceFlatLoginHelp)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test4",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help1.png");

    RED_CHECK_SIG(drawable.gd, "\x5b\xf0\x1e\xc8\xa7\x15\x79\x55\x58\x91\x41\xde\xcc\x2f\xf3\xbc\x96\x83\xbe\xad");

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.x() + flat_login.helpicon.cx() / 2,
                               flat_login.helpicon.y() + flat_login.helpicon.cy() / 2, nullptr);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help2.png");

    RED_CHECK_SIG(drawable.gd, "\x33\xf1\x7f\x2b\x18\x67\x10\xe8\x72\x0c\x76\xb1\xf9\xe2\x40\xdc\x62\x16\x00\xbb");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    RED_CHECK_SIG(drawable.gd, "\x7f\xc9\xe7\xbb\x07\x22\x69\xb6\xc4\x0b\xf5\x35\xd6\x33\x27\xe0\xd9\x4d\x4c\xa4");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         5 + flat_login.y(),
                                         30,
                                         10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login8.png");

    RED_CHECK_SIG(drawable.gd, "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35");
}

RED_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

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

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
}

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


#include "mod/internal/widget/flat_login.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

constexpr const char * LOGON_MESSAGE = "Warning! Unauthorized access to this system is forbidden and will be prosecuted by law.";

RED_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    // drawable.save_to_png("flat_login.png");

    RED_CHECK_SIG(drawable.gd, "\x4d\xc2\xb2\x86\xc4\x61\xf8\x34\x7b\xfc\x7b\xe1\xb7\xaf\x61\x98\xaf\xff\x1c\x01");
}

RED_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestDraw drawable(800, 600);
    WidgetFlatButton * extra_button = nullptr;
    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test2",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login2.png");

    RED_CHECK_SIG(drawable.gd, "\x0c\xce\x0a\x55\xcb\xf4\xe6\xd2\xb7\x0d\xa3\x05\x5d\xa1\xb9\x93\xf0\xcd\xf8\x89");
}

RED_AUTO_TEST_CASE(TraceFlatLogin3)
{
    TestDraw drawable(800, 600);
    struct Notify : NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    WidgetFlatButton * extra_button = nullptr;

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test3",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    flat_login.set_widget_focus(&flat_login.password_edit, Widget::focus_reason_tabkey);

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

    // drawable.save_to_png("flat_login3.png");

    RED_CHECK_SIG(drawable.gd, "\xbd\xaa\xa1\x1a\x8a\x77\xc3\x69\xcd\xcf\x34\xd2\xd0\xae\xb7\x7a\x17\xf3\x52\xbc");

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

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test4",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login-help1.png");

    RED_CHECK_SIG(drawable.gd, "\x59\xd5\x91\xd2\xde\x6b\x2f\xee\x09\x62\x2e\xa4\x62\xcd\x03\x19\xfc\xaf\x78\x7c");

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.x() + flat_login.helpicon.cx() / 2,
                               flat_login.helpicon.y() + flat_login.helpicon.cy() / 2, nullptr);

    // drawable.save_to_png("flat_login-help2.png");

    RED_CHECK_SIG(drawable.gd, "\x12\xe3\xff\x10\x4c\x3f\xcd\xb1\x78\xbd\xb8\xd3\x36\xc8\x6b\xf6\x2b\x48\xff\x9d");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login7.png");

    RED_CHECK_SIG(drawable.gd, "\x4a\x92\xbf\xb8\x27\x62\x2a\x9f\x97\x3c\xef\xf5\xb8\xd7\x99\xc1\x27\x3d\x66\x9e");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         5 + flat_login.y(),
                                         30,
                                         10));

    // drawable.save_to_png("flat_login8.png");

    RED_CHECK_SIG(drawable.gd, "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35");
}

RED_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    struct Notify : NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
}

RED_AUTO_TEST_CASE(TraceFlatLogin4)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable.gd, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "",
                         "WARNING: Unauthorized access to this system is forbidden and will be prosecuted by law.<br><br>"
                             "By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
                         extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    // drawable.save_to_png("flat_login4.png");

    RED_CHECK_SIG(drawable.gd, "\xae\x9e\xab\x4a\x7d\x9e\x5e\x88\xd0\xd4\xd0\x57\x37\x57\x31\xbe\xcb\x7b\x86\x69");
}

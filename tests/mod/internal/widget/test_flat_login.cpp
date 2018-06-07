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

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login.png");

    RED_CHECK_SIG(drawable.gd, "\xaa\x5c\xb2\x4c\x8d\xe6\x93\xa9\xec\xf7\xfe\x69\xe5\xfa\xe7\xdf\xb9\xc6\x7e\x7b");
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
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login2.png");

    RED_CHECK_SIG(drawable.gd, "\x2e\x24\xc0\xac\x79\x2c\x69\x4e\xb5\x4e\x89\x3e\x4e\xc2\xc6\xd0\xe0\x77\x37\xfe");
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

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login3.png");

    RED_CHECK_SIG(drawable.gd, "\x97\xac\xd2\x50\x4f\x18\x42\xc9\xf4\xa3\x7f\x41\x1e\x38\x65\x68\xb7\xf8\xe0\x1b");

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
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help1.png");

    RED_CHECK_SIG(drawable.gd, "\x1c\xf6\xf1\x90\xd3\xe2\x2d\x73\xb6\x23\xab\x33\x7b\x5f\x56\xdc\xd6\x86\xe4\x0c");

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.x() + flat_login.helpicon.cx() / 2,
                               flat_login.helpicon.y() + flat_login.helpicon.cy() / 2, nullptr);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login-help2.png");

    RED_CHECK_SIG(drawable.gd, "\x1c\x34\xf9\x0a\x25\x28\x23\x49\x4f\x2d\x44\x2e\x58\xcc\x8b\x15\xa9\x7c\x75\x3a");
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
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
                         Translator{}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login7.png");

    RED_CHECK_SIG(drawable.gd, "\x21\x57\x7f\xbb\x1d\xcd\xfe\x01\xf7\x0a\xe1\x7b\xf8\x5e\x1e\x46\xf4\x33\xe9\x8e");
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
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, font,
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

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

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

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_login4.png");

    RED_CHECK_SIG(drawable.gd, "\x0b\x72\xf2\xb6\x17\x3f\xb0\x4c\x5c\x75\x81\x6a\xf9\x30\x35\x1c\xac\x68\xef\x2e");
}

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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_login.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

constexpr const char * LOGON_MESSAGE = "Warning! Unauthorized access to this system is forbidden and will be prosecuted by law.";

RED_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    // drawable.save_to_png("flat_login.png");

    RED_CHECK_SIG(drawable, "\x61\x60\x78\x34\xa8\x76\xfd\xe8\xdd\xf8\x4e\x7c\xa2\x00\x14\x8e\x14\x1e\xd0\x84");
}

RED_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestGraphic drawable(800, 600);
    WidgetFlatButton * extra_button = nullptr;

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test2",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login2.png");

    RED_CHECK_SIG(drawable, "\xcd\x3b\x99\x90\x5d\x98\x53\x51\x4e\xdd\xae\x58\x0c\x88\xe3\xbd\x14\xda\xda\x57");
}

RED_AUTO_TEST_CASE(TraceFlatLogin3)
{
    TestGraphic drawable(800, 600);
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


    // FlatLogin is a flat_login widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, &notifier, "test3",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
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

    RED_CHECK_SIG(drawable, "\xbc\x60\x55\x96\xd9\x27\xdd\xd2\x43\xca\xa5\x75\x3f\x1a\x8f\x03\x4b\xb3\x16\x79");

    notifier.sender = nullptr;
    notifier.event = 0;
    keymap.push_kevent(Keymap2::KEVENT_ESC); // enterto validate
    flat_login.rdp_input_scancode(0, 0, 0, 0, &keymap);

    RED_CHECK(notifier.sender == &flat_login);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(TraceFlatLoginHelp)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test4",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login-help1.png");

    RED_CHECK_SIG(drawable, "\x88\x8c\x57\x64\xc8\x73\x74\xf6\x89\xaf\x53\x28\x2b\xe6\xfa\xc5\x30\x37\x94\xf3");

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.x() + flat_login.helpicon.cx() / 2,
                               flat_login.helpicon.y() + flat_login.helpicon.cy() / 2, nullptr);

    // drawable.save_to_png("flat_login-help2.png");

    RED_CHECK_SIG(drawable, "\x3c\xc2\xc8\xcb\xf6\xb4\xa8\x3b\x22\xb3\xef\x54\x59\x5c\xc5\x37\xd8\x51\xb3\x10");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    // drawable.save_to_png("flat_login7.png");

    RED_CHECK_SIG(drawable, "\xb3\xa0\x84\x73\x75\xba\xa1\x7d\xc9\xdb\x2f\x72\x2f\x58\x62\xa6\xe2\x53\xf1\xe3");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         5 + flat_login.y(),
                                         30,
                                         10));

    // drawable.save_to_png("flat_login8.png");

    RED_CHECK_SIG(drawable, "\x31\x82\xdc\x89\xfd\xda\x77\xc1\xf9\xa1\x44\x23\xdb\xc5\x09\xae\xb9\xb7\x2b\x35");
}

RED_AUTO_TEST_CASE(EventWidgetOk)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, &notifier, "test6",
                         nullptr, nullptr, "Login", "Password", "", LOGON_MESSAGE, extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
}

RED_AUTO_TEST_CASE(TraceFlatLogin4)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "Login", "Password", "",
                         "WARNING: Unauthorized access to this system is forbidden and will be prosecuted by law.<br><br>"
                             "By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
                         extra_button, global_font_deja_vu_14(),
                         Translator{}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    // drawable.save_to_png("flat_login4.png");

    RED_CHECK_SIG(drawable, "\x20\x58\xbc\x63\xe4\x07\x75\xea\x32\xec\x7a\x1a\x92\xdd\x2a\xa9\xfa\x53\x89\xb9");
}

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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/flat_login.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/login/"


constexpr const char * LOGON_MESSAGE = "Warning! Unauthorized access to this system is forbidden and will be prosecuted by law.";

RED_AUTO_TEST_CASE(TraceFlatLogin)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, parent.cx(), parent.cy(), parent, notifier,
                         "test1", "rec", "rec", "",
                         "Login", "Password", "Target", "", LOGON_MESSAGE,
                         extra_button, false, global_font_deja_vu_14(),
                         Translator{Language::en}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_1.png");
}

RED_AUTO_TEST_CASE(TraceFlatLogin2)
{
    TestGraphic drawable(800, 600);
    WidgetFlatButton * extra_button = nullptr;

    // FlatLogin is a flat_login widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test2",
                         nullptr, nullptr, nullptr,
                         "Login", "Password", "Target", "",
                         LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(),
                         Translator{Language::en}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_2.png");
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
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, &notifier, "test3",
                         nullptr, nullptr, nullptr, "Login", "Password", "Target",
                         "", LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(), Translator{Language::en}, Theme{});

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

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_3.png");

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
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test4",
                         nullptr, nullptr, nullptr, "Login", "Password", "Target",
                         "", LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(), Translator{Language::en}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(Rect(flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_help_1.png");

    flat_login.rdp_input_mouse(MOUSE_FLAG_MOVE,
                               flat_login.helpicon.x() + flat_login.helpicon.cx() / 2,
                               flat_login.helpicon.y() + flat_login.helpicon.cy() / 2, nullptr);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_help_2.png");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, nullptr, "Login", "Password", "Target",
                         "", LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(), Translator{Language::en}, Theme{});

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         flat_login.y(),
                                         flat_login.cx(),
                                         flat_login.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_4.png");
}

RED_AUTO_TEST_CASE(TraceFlatLoginClip2)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                         nullptr, nullptr, nullptr, "Login", "Password", "Target",
                         "", LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(), Translator{Language::en}, Theme{});

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_login.rdp_input_invalidate(Rect(20 + flat_login.x(),
                                         5 + flat_login.y(),
                                         30,
                                         10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_5.png");
}

RED_AUTO_TEST_CASE(EventWidgetOk)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

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
                         nullptr, nullptr, nullptr, "Login", "Password", "Target",
                         "", LOGON_MESSAGE, extra_button, false,
                         global_font_deja_vu_14(), Translator{Language::en}, Theme{});

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
}

RED_AUTO_TEST_CASE(TraceFlatLogin4)
{
    TestGraphic drawable(800, 600);


    // FlatLogin is a flat_login widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    WidgetFlatButton * extra_button = nullptr;

    FlatLogin flat_login(drawable, 0, 0, parent.cx(), parent.cy(), parent, notifier, "test1",
                         "rec", "rec", "rec", "Login", "Password", "Target", "",
                         "WARNING: Unauthorized access to this system is forbidden and will be prosecuted by law.\n\n"
                             "By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.",
                         extra_button, false,
                         global_font_deja_vu_14(),
                         Translator{Language::en}, Theme{});

    // ask to widget to redraw at it's current position
    flat_login.rdp_input_invalidate(flat_login.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_6.png");
}

RED_AUTO_TEST_CASE(TraceFlatLogin_transparent_png_with_theme_color)
{
    TestGraphic drawable(800, 600);
    WidgetScreen parent(drawable,
                        800,
                        600,
                        global_font_deja_vu_14(),
                        nullptr,
                        Theme { });
    NotifyApi *notifier = nullptr;
    WidgetFlatButton *extra_button = nullptr;
    Theme colors;

    colors.global.enable_theme = true;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";

    FlatLogin flat_login(drawable,
                         0,
                         0,
                         parent.cx(),
                         parent.cy(),
                         parent,
                         notifier,
                         "test1",
                         "rec",
                         "rec",
                         "rec",
                         "Login",
                         "Password",
                         "Target",
                         "",
                         LOGON_MESSAGE,
                         extra_button,
                         false,
                         global_font_deja_vu_14(),
                         Translator{Language::en},
                         colors);

    flat_login.rdp_input_invalidate(flat_login.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_7.png");
}


RED_AUTO_TEST_CASE(TraceFlatLogin_target_field)
{
    TestGraphic drawable(800, 600);
    WidgetScreen parent(drawable,
                        800,
                        600,
                        global_font_deja_vu_14(),
                        nullptr,
                        Theme { });
    NotifyApi *notifier = nullptr;
    WidgetFlatButton *extra_button = nullptr;
    Theme colors;

    colors.global.enable_theme = true;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";

    FlatLogin flat_login(drawable,
                         0,
                         0,
                         parent.cx(),
                         parent.cy(),
                         parent,
                         notifier,
                         "test1",
                         "rec",
                         "rec",
                         "",
                         "Login",
                         "Password",
                         "Target",
                         "",
                         LOGON_MESSAGE,
                         extra_button,
                         true,
                         global_font_deja_vu_14(),
                         Translator{Language::en},
                         colors);

    flat_login.rdp_input_invalidate(flat_login.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "login_8.png");
}

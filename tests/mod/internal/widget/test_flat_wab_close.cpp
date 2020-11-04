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


#include "mod/internal/widget/flat_wab_close.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/wab_close/"


RED_AUTO_TEST_CASE(TraceFlatWabClose)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                "abc\ndef", "rec", "rec",
                                false, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_1.png");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
        "Lorem ipsum dolor sit amet, consectetur\n"
        "adipiscing elit. Nam purus lacus, luctus sit\n"
        "amet suscipit vel, posuere quis turpis. Sed\n"
        "venenatis rutrum sem ac posuere. Phasellus\n"
        "feugiat dui eu mauris adipiscing sodales.\n"
        "Mauris rutrum molestie purus, in tempor lacus\n"
        "tincidunt et. Sed eu ligula mauris, a rutrum\n"
        "est. Vestibulum in nunc vel massa condimentum\n"
        "iaculis nec in arcu. Pellentesque accumsan,\n"
        "quam sit amet aliquam mattis, odio purus\n"
        "porttitor tortor, sit amet tincidunt odio\n"
        "erat ut ligula. Fusce sit amet mauris neque.\n"
        "Sed orci augue, luctus in ornare sed,\n"
        "adipiscing et arcu.",
        nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // ask to widget to redraw at it's current position

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_2.png");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc\ndef",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_3.png");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc\ndef",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect().offset(20,0));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_4.png");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc\ndef",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.x(),
                                               5 + flat_wab_close.y(),
                                               30,
                                               10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_5.png");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseExit)
{
    struct Notify : NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, &notifier,
                                "abc\ndef", "tartempion", "caufield",
                                true, extra_message, global_font_deja_vu_14(), Theme(), Language::en);

    flat_wab_close.refresh_timeleft(183);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_7.png");

    flat_wab_close.refresh_timeleft(49);
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_8.png");

    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);
    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);

    RED_CHECK(notifier.sender == &flat_wab_close);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_8.png");

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_wab_close.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

RED_AUTO_TEST_CASE(TraceFlatWabClose_transparent_png_with_theme_color)
{
    TestGraphic drawable(800, 600);
    WidgetScreen parent(drawable,
                        800,
                        600,
                        global_font_deja_vu_14(),
                        nullptr,
                        Theme { });
    NotifyApi *notifier = nullptr;
    const char *extra_message = nullptr;
    Theme colors;

    colors.global.enable_theme = true;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";
    
    FlatWabClose flat_wab_close(drawable,
                                0,
                                0,
                                800,
                                600,
                                parent,
                                notifier,
                                "abc\ndef",
                                "rec",
                                "rec",
                                false,
                                extra_message,
                                global_font_deja_vu_14(),
                                colors,
                                Language::en);

    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "wab_close_10.png");
}

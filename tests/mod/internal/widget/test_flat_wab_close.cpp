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

#define RED_TEST_MODULE TestFlatWabClose
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/flat_wab_close.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceFlatWabClose)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                "abc<br>def", "rec", "rec",
                                false, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close.png");

    RED_CHECK_SIG(drawable, "\xdb\x65\x0f\x79\xfe\xd4\xee\x90\x78\xb7\x04\x01\xed\x5a\x83\x12\xe9\x03\x39\x1e");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
        "Lorem ipsum dolor sit amet, consectetur<br>"
        "adipiscing elit. Nam purus lacus, luctus sit<br>"
        "amet suscipit vel, posuere quis turpis. Sed<br>"
        "venenatis rutrum sem ac posuere. Phasellus<br>"
        "feugiat dui eu mauris adipiscing sodales.<br>"
        "Mauris rutrum molestie purus, in tempor lacus<br>"
        "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
        "est. Vestibulum in nunc vel massa condimentum<br>"
        "iaculis nec in arcu. Pellentesque accumsan,<br>"
        "quam sit amet aliquam mattis, odio purus<br>"
        "porttitor tortor, sit amet tincidunt odio<br>"
        "erat ut ligula. Fusce sit amet mauris neque.<br>"
        "Sed orci augue, luctus in ornare sed,<br>"
        "adipiscing et arcu.",
        nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // ask to widget to redraw at it's current position

//    drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close2.png");

    RED_CHECK_SIG(drawable, "\x46\xca\x13\x7f\x61\x03\x7c\xd1\xe0\x84\x8f\x11\x89\x09\x3c\xf0\xe1\x3b\xa6\xa1");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close3.png");

    RED_CHECK_SIG(drawable, "\x37\xca\xf1\xf0\x97\x55\x83\xa8\x76\x95\xd3\xd9\x6d\xa8\x18\xd3\x02\x9d\xd9\xd3");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect().offset(20,0));

    // drawable.save_to_png("flat_wab_close7.png");

    RED_CHECK_SIG(drawable, "\x4b\x63\x5e\x1b\x74\x99\x92\xed\xfd\xa8\xbe\xc9\xf0\x8e\x73\x34\xdc\xf9\x8a\x08");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestGraphic drawable(800, 600);


    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.x(),
                                               5 + flat_wab_close.y(),
                                               30,
                                               10));

    // drawable.save_to_png("flat_wab_close8.png");

    RED_CHECK_SIG(drawable, "\x5e\xd7\x4c\xb7\x03\x2f\xe1\x79\xbf\x92\x0b\x30\xd6\x9b\xe3\xe2\x5d\xbf\x7b\x75");
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
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable, 0, 0, 800, 600, parent, &notifier,
                                "abc<br>def", "tartempion", "caufield",
                                true, extra_message, global_font_deja_vu_14(), Theme(), Translation::EN);

    flat_wab_close.refresh_timeleft(183);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close-exit1.png");

    RED_CHECK_SIG(drawable, "\xa8\x5c\xaf\x51\x0d\x59\xfa\x8c\x38\x4e\xe7\x67\x21\x64\xc2\xf0\x8b\xa7\xf7\x1f");

    flat_wab_close.refresh_timeleft(49);
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close-exit2.png");
    RED_CHECK_SIG(drawable, "\x3b\xcb\x95\xcf\x2f\x9b\xb8\x4e\x45\xe2\x47\x6c\x50\x92\xa8\x64\xc9\xe3\x17\xf8");

    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);
    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);

    RED_CHECK(notifier.sender == &flat_wab_close);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);

    // drawable.save_to_png("flat_wab_close-exit3.png");
    RED_CHECK_SIG(drawable, "\x3b\xcb\x95\xcf\x2f\x9b\xb8\x4e\x45\xe2\x47\x6c\x50\x92\xa8\x64\xc9\xe3\x17\xf8");

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_wab_close.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

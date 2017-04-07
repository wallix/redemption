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

#define UNIT_TEST_MODULE TestFlatWabClose
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/flat_wab_close.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceFlatWabClose)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                "abc<br>def", "rec", "rec",
                                false, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close.png");

    RED_CHECK_SIG(drawable.gd, "\x67\xbb\x8b\xdd\x5e\x7a\xf3\xe0\x74\x9e\x35\xf0\xe2\x12\x93\x4c\xe2\xf2\x64\x46");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
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
        nullptr, nullptr, false, font, Theme(), Translation::EN);

    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // ask to widget to redraw at it's current position

//    drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close2.png");

    RED_CHECK_SIG(drawable.gd, "\x94\xa0\xce\xbc\x87\x28\x38\x2c\xf5\x84\x7f\xa3\x3b\xc7\x5d\xda\xde\x6c\x96\xe4");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close3.png");

    RED_CHECK_SIG(drawable.gd, "\x9d\x5b\x54\xd7\xd2\xa7\xce\x30\x3c\x69\x59\xb0\x28\xdb\xdf\x11\x37\x11\x0c\x92");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect().offset(20,0));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close7.png");

    RED_CHECK_SIG(drawable.gd, "\x4c\xee\xb4\x3b\xcf\x09\x0e\x37\x56\xb3\xb0\x2f\x37\x7e\x9d\xdc\xd5\xba\xd5\x27");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.x(),
                                               5 + flat_wab_close.y(),
                                               30,
                                               10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close8.png");

    RED_CHECK_SIG(drawable.gd, "\xbd\x03\x58\xe9\xe7\xa2\x9e\xa2\x8c\x57\xee\x6e\xf8\x7e\xca\xfa\x7f\x46\x0c\xf4");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseExit)
{
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);


    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, &notifier,
                                "abc<br>def", "tartempion", "caufield",
                                true, font, Theme(), Translation::EN);

    flat_wab_close.refresh_timeleft(183);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit1.png");

    RED_CHECK_SIG(drawable.gd, "\xec\xd0\x19\xe6\x90\x35\x2f\xb9\x54\x1c\xf7\x6f\x9d\x50\x2a\xd9\x71\x30\xe7\xd4");

    flat_wab_close.refresh_timeleft(49);
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit2.png");
    RED_CHECK_SIG(drawable.gd, "\x1f\x14\x7d\x66\x26\x2b\x6d\x5d\x3c\x73\x13\x8a\x1f\x2e\x7f\x1a\xe8\x9b\x73\xaa");

    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);
    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);

    RED_CHECK(notifier.sender == &flat_wab_close);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit3.png");
    RED_CHECK_SIG(drawable.gd, "\x1f\x14\x7d\x66\x26\x2b\x6d\x5d\x3c\x73\x13\x8a\x1f\x2e\x7f\x1a\xe8\x9b\x73\xaa");

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_wab_close.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

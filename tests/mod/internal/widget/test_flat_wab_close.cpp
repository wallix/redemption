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
#include "system/redemption_unit_tests.hpp"


#include "core/font.hpp"
#include "mod/internal/widget/flat_wab_close.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceFlatWabClose)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                "abc<br>def", "rec", "rec",
                                false, extra_message, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close.png");

    RED_CHECK_SIG(drawable.gd, "\xfe\xf7\x90\xd0\xca\x7a\xe6\x3b\x5c\x96\xff\x3b\xbc\xb6\x9c\xc4\x98\x01\xdb\x83");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

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
        nullptr, nullptr, false, extra_message, font, Theme(), Translation::EN);

    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // ask to widget to redraw at it's current position

//    drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close2.png");

    RED_CHECK_SIG(drawable.gd, "\xd3\x5d\xa6\x2c\xee\x6c\xb0\xd3\xe9\x03\xff\xb4\xec\x04\xfa\x0e\x0b\x68\xf4\x4a");
}

RED_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close3.png");

    RED_CHECK_SIG(drawable.gd, "\x3f\xd1\x8d\xf9\x44\x80\x73\xca\x96\x51\xca\x60\xed\x6d\xd4\xf4\x23\xb9\x8b\x75");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect().offset(20,0));

    // drawable.save_to_png("flat_wab_close7.png");

    RED_CHECK_SIG(drawable.gd, "\x5d\x62\x72\xa8\x8d\xc5\x95\xe8\xc2\x7a\xc8\x36\x01\x14\x0e\x5a\x67\xca\x0c\x41");
}

RED_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    nullptr, nullptr, false, extra_message, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.x(),
                                               5 + flat_wab_close.y(),
                                               30,
                                               10));

    // drawable.save_to_png("flat_wab_close8.png");

    RED_CHECK_SIG(drawable.gd, "\x6b\x68\x36\x6f\x2d\x72\x12\xec\xc7\x65\x51\x7e\xe1\xe6\x02\xab\x02\x29\x48\xbf");
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

    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    const char* extra_message = nullptr;

    FlatWabClose flat_wab_close(drawable.gd, 0, 0, 800, 600, parent, &notifier,
                                "abc<br>def", "tartempion", "caufield",
                                true, extra_message, font, Theme(), Translation::EN);

    flat_wab_close.refresh_timeleft(183);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close-exit1.png");

    RED_CHECK_SIG(drawable.gd, "\x67\x93\x48\x32\x7d\xd2\x55\x8e\x22\x66\x0d\xb2\x8f\x78\x7e\xa7\xed\xd8\xe4\x93");

    flat_wab_close.refresh_timeleft(49);
    flat_wab_close.rdp_input_invalidate(flat_wab_close.get_rect());

    // drawable.save_to_png("flat_wab_close-exit2.png");
    RED_CHECK_SIG(drawable.gd, "\xab\x76\x08\xc6\xe9\xb6\x36\xb1\x11\x01\xa0\xe5\x7e\xdf\xb7\xa7\x98\x89\xf6\xb1");

    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);
    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                   flat_wab_close.cancel.x() + 2,
                                   flat_wab_close.cancel.y() + 2, nullptr);

    RED_CHECK(notifier.sender == &flat_wab_close);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);

    // drawable.save_to_png("flat_wab_close-exit3.png");
    RED_CHECK_SIG(drawable.gd, "\xab\x76\x08\xc6\xe9\xb6\x36\xb1\x11\x01\xa0\xe5\x7e\xdf\xb7\xa7\x98\x89\xf6\xb1");

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_wab_close.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

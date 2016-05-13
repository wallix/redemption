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
#define BOOST_TEST_MODULE TestFlatWabClose
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/flat_wab_close.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatWabClose)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    int id = 0;

    try {
        FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def", id, "rec", "rec",
                                    false, font, Theme(), Translation::EN);

        // ask to widget to redraw at it's current position
        flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

        // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close.png");
    } catch (Error & e) {
        LOG(LOG_INFO, "e=%u", e.id);
    };

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xc3\x40\x75\x1e\x15\x45\xd9\x3b\x52\x70\xb7\xc1\x21\x69\x1d\x92\x52\xa8\x91\x36"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    try {
        FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
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
            0, nullptr, nullptr, false, font, Theme(), Translation::EN);

        flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);
    }
    catch(Error & e) {
        LOG(LOG_INFO, "error.id=%d", e.id);
        BOOST_CHECK(false);
    };

    // ask to widget to redraw at it's current position

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close2.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xbe\xfa\x1e\x30\xe1\x18\x09\x8e\x83\xa4\x46\x7e\xcd\x2e\x6f\x50\x49\x2c\xf0\x70"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabClose3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    0, nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close3.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xb6\xe8\xcd\x88\x8f\xb1\xdb\xc5\x0c\x44\x40\x79\x15\xe1\x33\xaf\x4b\xb9\xe5\x56"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabCloseClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    0, nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect.offset(20,0));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close7.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x36\x43\x1c\x13\xa6\x40\xe8\x4a\x3f\xbc\x6e\xa9\xd8\xb8\x24\x33\xd5\xd9\x68\x93"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabCloseClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, notifier,
                                    "abc<br>def",
                                    0, nullptr, nullptr, false, font, Theme(), Translation::EN);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_wab_close.rdp_input_invalidate(Rect(20 + flat_wab_close.dx(),
                                               5 + flat_wab_close.dy(),
                                               30,
                                               10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close8.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x12\x71\x2e\xd7\x8a\x95\x53\x23\x4c\x84\x0d\xce\xa2\x32\x3a\xc0\xc9\x48\x17\x4c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatWabCloseExit)
{
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        virtual void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatWabClose is a flat_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);

    FlatWabClose flat_wab_close(drawable, 800, 600, parent, &notifier,
                                "abc<br>def", 0, "tartempion", "caufield",
                                true, font, Theme(), Translation::EN);

    flat_wab_close.refresh_timeleft(183);

    // ask to widget to redraw at it's current position
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x4b\x4f\xaf\x73\xce\xb5\x65\x14\xbf\xa2\xdd\x59\xc2\x23\x54\x8a\xdc\x03\xd4\xf8"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_wab_close.refresh_timeleft(49);
    flat_wab_close.rdp_input_invalidate(flat_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x31\xd8\xbc\x23\xb4\xc6\xac\xc1\x33\x44\x4b\x76\x93\x82\x3a\x90\x70\x11\x88\xca"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                   flat_wab_close.cancel.centerx(),
                                   flat_wab_close.cancel.centery(), nullptr);
    flat_wab_close.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                   flat_wab_close.cancel.centerx(),
                                   flat_wab_close.cancel.centery(), nullptr);

    BOOST_CHECK(notifier.sender == &flat_wab_close);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_wab_close-exit3.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x31\xd8\xbc\x23\xb4\xc6\xac\xc1\x33\x44\x4b\x76\x93\x82\x3a\x90\x70\x11\x88\xca"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_wab_close.rdp_input_scancode(0, 0, 0, 0, &keymap);
}

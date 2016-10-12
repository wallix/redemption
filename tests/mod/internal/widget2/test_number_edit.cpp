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
#define BOOST_TEST_MODULE TestWidgetNumberEdit
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/number_edit.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(WidgetNumberEditEventPushChar)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, 800, 600, font, nullptr, Theme{});
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetNumberEdit wnumber_edit(drawable.gd, x, y, cx, parent, &notifier, "123456", 0, GREEN, RED, RED, font);

    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\x42\xfc\x5a\xf5\x42\x03\xb5\x65\x5d\x91\x56\x78\x50\x89\xfe\x8a\x21\x14\x9e\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push('a');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-1.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x42\xfc\x5a\xf5\x42\x03\xb5\x65\x5d\x91\x56\x78\x50\x89\xfe\x8a\x21\x14\x9e\x55"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == nullptr);
    BOOST_CHECK(notifier.event == 0);

    keymap.push('2');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.rect);
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-2.png");
    if (!check_sig(drawable.gd.impl(), message,
        "\x95\x97\x61\x74\x73\xb9\x2a\x51\x3d\x98\xaa\x3c\x2b\x74\x8f\x28\x31\x9c\xc5\x60"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wnumber_edit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
}

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

#define RED_TEST_MODULE TestWidgetNumberEdit
#include "system/redemption_unit_tests.hpp"

#define LOGNULL

#include "core/font.hpp"
#include "mod/internal/widget2/number_edit.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(WidgetNumberEditEventPushChar)
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

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetNumberEdit wnumber_edit(drawable.gd, parent, &notifier, "123456", 0, GREEN, RED, RED, font);
    Dimension dim = wnumber_edit.get_optimal_dim();
    wnumber_edit.set_wh(cx, dim.h);
    wnumber_edit.set_xy(x, y);

    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e1.png");
    RED_CHECK_SIG(drawable.gd, "\xb4\x77\xd9\x77\x1b\xbc\x9e\x63\x5d\x99\x6e\x22\xef\x69\xea\x87\xfa\x16\x52\x5c");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push('a');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-1.png");
    RED_CHECK_SIG(drawable.gd, "\xb4\x77\xd9\x77\x1b\xbc\x9e\x63\x5d\x99\x6e\x22\xef\x69\xea\x87\xfa\x16\x52\x5c");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push('2');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
//    drawable.save_to_png(OUTPUT_FILE_PATH "number_edit-e2-2.png");
    RED_CHECK_SIG(drawable.gd, "\xff\xd0\xe8\xbf\x21\x94\xdb\x15\x15\x32\x17\x1d\x89\x4d\x79\xae\xda\x97\xf7\x92");
    RED_CHECK(notifier.sender == &wnumber_edit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
}

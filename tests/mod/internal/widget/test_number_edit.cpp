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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/number_edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(WidgetNumberEditEventPushChar)
{
    TestGraphic drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetNumberEdit wnumber_edit(drawable, parent, &notifier, "123456", 0, GREEN, RED, RED, global_font_lato_light_16());
    Dimension dim = wnumber_edit.get_optimal_dim();
    wnumber_edit.set_wh(cx, dim.h);
    wnumber_edit.set_xy(x, y);

    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    // drawable.save_to_png("number_edit-e1.png");
    RED_CHECK_SIG(drawable, "\x1f\x0d\x7c\xd8\x43\x5b\x69\xe9\x20\x15\x32\x1c\x15\x2a\xa4\xd4\x90\x0a\x34\x6c");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push('a');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    // drawable.save_to_png("number_edit-e2-1.png");
    RED_CHECK_SIG(drawable, "\x1f\x0d\x7c\xd8\x43\x5b\x69\xe9\x20\x15\x32\x1c\x15\x2a\xa4\xd4\x90\x0a\x34\x6c");
    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);

    keymap.push('2');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    // drawable.save_to_png("number_edit-e2-2.png");
    RED_CHECK_SIG(drawable, "\x4b\x5a\xb9\x52\x13\x81\x8f\x35\x09\xa9\xf5\x64\x52\x8f\x24\x2c\x1f\xe0\x90\xb4");
    RED_CHECK(notifier.sender == &wnumber_edit);
    RED_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
}

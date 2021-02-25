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

#include "mod/internal/widget/number_edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/number_edit/"


RED_AUTO_TEST_CASE(WidgetNumberEditEventPushChar)
{
    TestGraphic drawable(800, 600);

    NotifyTrace notifier;

    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    // Widget* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetNumberEdit wnumber_edit(drawable, parent, &notifier, "123456", 0, GREEN, RED, RED, global_font_lato_light_16());
    Dimension dim = wnumber_edit.get_optimal_dim();
    wnumber_edit.set_wh(cx, dim.h);
    wnumber_edit.set_xy(x, y);

    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "number_edit_1.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push('a');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "number_edit_1.png");
    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    keymap.push('2');
    wnumber_edit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wnumber_edit.rdp_input_invalidate(wnumber_edit.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "number_edit_3.png");
    RED_CHECK(notifier.last_widget == &wnumber_edit);
    RED_CHECK(notifier.last_event == NOTIFY_TEXT_CHANGED);
}

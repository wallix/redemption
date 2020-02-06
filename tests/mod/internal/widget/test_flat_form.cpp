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


#include "mod/internal/widget/flat_form.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TestFlatForm)
{
    TestGraphic drawable(800, 600);

    // FlatWait is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;

    int flag = FlatForm::COMMENT_DISPLAY | FlatForm::TICKET_DISPLAY |
        FlatForm::DURATION_DISPLAY;

    flag += FlatForm::DURATION_MANDATORY;

    FlatForm form(drawable, 0, 0, 600, 150, parent, notifier, 0, global_font_lato_light_16(), colors, Translation::EN, flag);
    // ask to widget to redraw at it's current position
    form.move_xy(70, 70);
    form.rdp_input_invalidate(form.get_rect());

    // drawable.save_to_png("ticket_form.png");

    RED_CHECK_SIG(drawable, "\x81\x01\x09\x20\x44\x48\x48\xeb\xb4\x83\x0f\xd5\x66\xb9\x34\x49\xcd\xf5\x25\x57");
}

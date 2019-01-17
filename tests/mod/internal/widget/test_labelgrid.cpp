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

#define RED_TEST_MODULE TestWidgetLabelGrid
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/grid.hpp"
#include "mod/internal/widget/labelgrid.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceLabelGrid)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;

    const uint16_t line_number   = 5;
    const uint16_t column_number = 4;
    const uint16_t grid_border   = 2;

    WidgetLabelGrid wgrid(drawable, parent, notifier,
                          line_number, column_number,
                          PALE_BLUE, BLACK, LIGHT_BLUE, BLACK,
                          WINBLUE, WHITE, MEDIUM_BLUE, WHITE, global_font_lato_light_16(),
                          grid_border, id);
    wgrid.set_wh(640, 480);
    wgrid.set_xy(x, y);

    array_view_const_char const texts0[] = { "target_group"_av, "target"_av, "protocol"_av, "timeframe"_av };
    wgrid.add_line(texts0);
    array_view_const_char const texts1[] = { "win"_av, "admin@device"_av, "RDP"_av, "never"_av };
    wgrid.add_line(texts1);
    wgrid.add_line(texts1);
    wgrid.add_line(texts1);
    wgrid.set_selection(2);

    ColumnWidthStrategy column_width_strategies[] = {
        { 50, 150 }, { 150, 800 }, { 50, 150 }, { 50, 100 }
    };

    uint16_t row_height[line_number+4]      = { 0 };
    uint16_t column_width[column_number] = { 0 };

    compute_format(wgrid, column_width_strategies, row_height, column_width);
    apply_format(wgrid, row_height, column_width);


    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(0 + wgrid.x(),
                                    0 + wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    // drawable.save_to_png("/tmp/labelgrid.png");
    // char message[1024];
    // if (!check_sig(drawable.drawable, message,
    //                "\x47\x86\xd6\xd2\x1d\x47\xa2\x4e\xcf\x7b"
    //                "\x3f\xce\x8f\x0b\x25\x8b\xf7\x3b\xcf\x01")){
    //     RED_CHECK_MESSAGE(false, message);
    // }

    wgrid.has_focus = true;
    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(0 + wgrid.x(),
                                    0 + wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    // drawable.save_to_png("/tmp/labelgrid2.png");
    // if (!check_sig(drawable.drawable, message,
    //                "\x0f\xf6\x9f\xa5\xfb\x38\x4c\xb4\x8e\x66"
    //                "\x8e\x6d\x99\x64\x4e\x3c\x9c\x7b\xb6\xca")){
    //     RED_CHECK_MESSAGE(false, message);
    // }
}

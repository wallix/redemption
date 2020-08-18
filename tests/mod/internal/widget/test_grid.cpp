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
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "mod/internal/widget/label.hpp"
#include "mod/internal/widget/grid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/flat_button.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/grid/"

RED_AUTO_TEST_CASE(TraceWidgetGrid)
{
    TestGraphic drawable(800, 600);

    // WidgetLabel is a label widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;

    const uint16_t line_number   = 5;
    const uint16_t column_number = 4;
    const uint16_t grid_border   = 2;

    /* TODO
     * I believe users of this widget may wish to control text position and behavior inside rectangle
     * ie: text may be centered, aligned left, aligned right, or even upside down, etc
     * these possibilities (and others) are supported in RDPGlyphIndex */
    WidgetGrid wgrid(drawable, parent, notifier, line_number, column_number,
        PALE_BLUE, BLACK, LIGHT_BLUE, BLACK, WINBLUE, WHITE, MEDIUM_BLUE, WHITE,
        grid_border, id);
    wgrid.set_wh(640, 480);
    wgrid.set_xy(x, y);

    uint16_t   widget_count     = 0;

    for (uint16_t line_index = 0; line_index < line_number; line_index++) {
        for (uint16_t column_index = 0; column_index < column_number; column_index++) {
            char text[256];
            snprintf(text, sizeof(text), "Label %ux%u", unsigned(line_index), unsigned(column_index));
            std::unique_ptr<Widget> w;
            if ((line_index == 2) && (column_index == 3)) {
                w = std::make_unique<WidgetFlatButton>(
                    drawable, wgrid, notifier, text, id, WHITE, MEDIUM_BLUE, LIGHT_BLUE, 2,
                    global_font_lato_light_16(), 2, 2);
            }
            else {
                w = std::make_unique<WidgetLabel>(
                    drawable, wgrid, notifier, text, id, fg_color, bg_color,
                    global_font_lato_light_16());
            }

            Dimension dim = w->get_optimal_dim();
            w->set_wh(dim);

            wgrid.set_widget(line_index, column_index, std::move(w));
            widget_count++;
        }
    }

    ColumnWidthStrategy column_width_strategies[] = {
        { 50, 20 }, { 150, 30 }, { 50, 20 }, { 50, 30 }
    };

    uint16_t row_height[line_number] {};
    uint16_t column_width[column_number] {};
    bool     column_width_is_optimal[column_number] {};

    compute_format(wgrid, column_width_strategies, -1, row_height, column_width, column_width_is_optimal);
    apply_format(wgrid, row_height, column_width);

    wgrid.set_selection(2);

    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(wgrid.x(),
                                    wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "grid_1.png");

    wgrid.set_selection(4);

    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(0 + wgrid.x(),
                                    0 + wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "grid_2.png");

    uint16_t mouse_x = wgrid.x() + 50;
    uint16_t mouse_y = wgrid.get_widget(1, 0)->y();

    wgrid.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, mouse_x, mouse_y, nullptr);
    wgrid.rdp_input_mouse(MOUSE_FLAG_BUTTON1, mouse_x, mouse_y, nullptr);
    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(0 + wgrid.x(),
                                    0 + wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "grid_3.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wgrid.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    wgrid.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    wgrid.rdp_input_scancode(0,0,0,0, &keymap);
    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    wgrid.rdp_input_scancode(0,0,0,0, &keymap);

    // ask to widget to redraw at it's current position
    wgrid.rdp_input_invalidate(Rect(0 + wgrid.x(),
                                    0 + wgrid.y(),
                                    wgrid.cx(),
                                    wgrid.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "grid_4.png");

    wgrid.clear();
}

/* TODO
 * the entry point exists in module: it's rdp_input_invalidate
 * je just have to change received values to widget messages */

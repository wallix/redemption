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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "mod/internal/widget/widget.hpp"


class Font;

class WidgetScrollBar : public Widget
{
    bool horizontal;

    BGRColor fg_color;
    BGRColor bg_color;
    BGRColor focus_color;

    Font const & font;

    Rect left_or_top_button_rect;
    Rect right_or_bottom_button_rect;
    Rect scroll_bar_rect;
    Rect cursor_button_rect;

    int current_value = 0;
    int max_value     = 100;

    int step_value = 1;

    bool mouse_down = false;

    enum {
        BUTTON_NONE,

        BUTTON_LEFT_OR_TOP,
        BUTTON_CURSOR,
        BUTTON_RIGHT_OR_BOTTOM
    } selected_button = BUTTON_NONE;

    notify_event_t event;

    int old_mouse_x_or_y         = 0;
    int old_cursor_button_x_or_y = 0;

    uint16_t button_width_or_height = 0;

    bool rail_style = false;

public:
    WidgetScrollBar(gdi::GraphicApi & drawable, Widget& parent,
                    NotifyApi* notifier, bool horizontal,
                    int group_id, BGRColor fgcolor, BGRColor bgcolor, BGRColor focuscolor,
                    Font const & font, bool rail_style, int maxvalue = 100);

    unsigned int get_current_value() const;

    void set_current_value(unsigned int cv);

    void set_max_value(unsigned int maxvalue);

    // Widget

    void rdp_input_invalidate(Rect clip) override;

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using Widget::set_wh;

    Dimension get_optimal_dim() override;

    // RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

private:
    void compute_step_value();

    void update_cursor_button_rects();

    void update_rects();

    Dimension get_optimal_button_dim();
};

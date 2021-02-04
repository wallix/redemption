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
 *   Copyright (C) Wallix 2010-2021
 *   Author(s): Proxies Team
 */

#pragma once

#include "mod/internal/widget/widget.hpp"
#include "gdi/text_metrics.hpp"


class Font;

class WidgetVerticalScrollText : public Widget
{
public:
    WidgetVerticalScrollText(
        gdi::GraphicApi& drawable, Widget& parent,
        NotifyApi* notifier, int group_id, std::string text,
        BGRColor fgcolor, BGRColor bgcolor, BGRColor focuscolor,
        Font const& font, uint16_t xtext = 0, uint16_t ytext = 0); /*NOLINT*/

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using Widget::set_wh;

    Dimension get_optimal_dim() override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override;

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2* keymap) override;

    void rdp_input_invalidate(Rect clip) override;

private:
    int16_t compute_cursor_y(int current_value) const;

    RDPColor const fg_color;
    RDPColor const bg_color;
    RDPColor const focus_color;

    Font const & font;

    int16_t cursor_button_y = 0;

    int current_value = 0;
    int step = 0;

    int cursor_button_diff_y = 0;

    uint16_t x_text;
    uint16_t y_text;

    Dimension const button_dim;

    bool mouse_down = false;
    bool has_scroll = false;

    enum class ButtonType : unsigned char
    {
        None,

        Top,
        Cursor,
        Bottom,
    };

    ButtonType selected_button = ButtonType::None;

    std::string text;
    gdi::MultiLineTextMetrics line_metrics;
};

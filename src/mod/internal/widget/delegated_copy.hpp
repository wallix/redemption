/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "mod/internal/widget/widget.hpp"


class Font;

class WidgetDelegatedCopy : public Widget
{
public:
    WidgetDelegatedCopy(
        gdi::GraphicApi & drawable, Widget& delegated, NotifyApi& notifier,
        Color fgcolor, Color bgcolor, Color activecolor,
        Font const & font, int xicon, int yicon);

    Dimension get_optimal_dim() const override;

    static Dimension get_optimal_dim(Font const & font, int xicon, int yicon);

    void set_color(Color bg_color, Color fg_color) override;

    void rdp_input_mouse(int device_flags, int x, int y) override;

    void rdp_input_invalidate(Rect clip) override;

    static void draw(
        Rect clip, Rect rect, gdi::GraphicApi & drawable,
        Color fg, Color bg, int xicon, int yicon);

private:
    Color bg_color;
    Color fg_color;
    Color active_color;

    Dimension optimal_glyph_dim;
    int x_icon;
    int y_icon;

    bool is_active = false;
};

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
        BGRColor fgcolor, BGRColor bgcolor, BGRColor activecolor,
        Font const & font, int xicon, int yicon);

    Dimension get_optimal_dim() override;

    static Dimension get_optimal_dim(Font const & font, int xicon, int yicon);

    void set_color(BGRColor bg_color, BGRColor fg_color) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* /*unused*/) override;

    void rdp_input_invalidate(Rect clip) override;

    static void draw(
        Rect clip, Rect rect, gdi::GraphicApi & drawable,
        RDPColor fg, RDPColor bg, int xicon, int yicon);

private:
    RDPColor bg_color;
    RDPColor fg_color;
    RDPColor active_color;

    Dimension optimal_glyph_dim;
    int x_icon;
    int y_icon;

    bool is_active = false;
};

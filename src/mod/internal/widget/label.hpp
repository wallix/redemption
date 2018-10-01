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

#pragma once

#include "mod/internal/widget/widget.hpp"
#include "utils/sugar/array_view.hpp"


class Font;
namespace gdi
{
    class ColorCtx;
}

class WidgetLabel : public Widget
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];
    int initial_x_text;
    int x_text;
    int y_text;
    BGRColor bg_color;
    BGRColor fg_color;
    bool tool;

    int w_border;
    int h_border;

    Font const & font;

public:
    WidgetLabel(gdi::GraphicApi & drawable, Widget& parent,
                NotifyApi* notifier, array_view_const_char text,
                int group_id, BGRColor fgcolor, BGRColor bgcolor, Font const & font,
                int xtext = 0, int ytext = 0);

    WidgetLabel(gdi::GraphicApi & drawable, Widget& parent,
                NotifyApi* notifier, char const* text,
                int group_id, BGRColor fgcolor, BGRColor bgcolor, Font const & font,
                int xtext = 0, int ytext = 0);

    WidgetLabel(WidgetLabel const & other);

    ~WidgetLabel() override;

    void set_text(char const* text);
    void set_text(array_view_const_char text);

    const char * get_text() const;

    void rdp_input_invalidate(Rect clip) override;

    static void draw(Rect const clip, Rect const rect, gdi::GraphicApi& drawable,
                     char const* text, RDPColor fgcolor, RDPColor bgcolor, gdi::ColorCtx color_ctx,
                     Font const & font, int xtext, int ytext);

    Dimension get_optimal_dim() override;

    static Dimension get_optimal_dim(Font const & font, char const* text, int xtext, int ytext);

    bool shift_text(int pos_x);

    void set_color(BGRColor bg_color, BGRColor fg_color) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* /*unused*/) override;

    void auto_resize();
};

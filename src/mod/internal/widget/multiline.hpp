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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "mod/internal/widget/widget.hpp"

class Font;

class WidgetMultiLine : public Widget
{
public:
    WidgetMultiLine(gdi::GraphicApi & drawable, Widget& parent,
                    NotifyApi* notifier, const char * text,
                    int group_id,
                    BGRColor fgcolor, BGRColor bgcolor, Font const & font,
                    int xtext = 0, int ytext = 0);

    void set_text(const char * text);

    const char * get_line(size_t num) const;

    void rdp_input_invalidate(Rect clip) override;

    Dimension get_optimal_dim() override;

    BGRColor get_bg_color() const noexcept
    {
        return this->bg_color;
    }

private:
    static const size_t buffer_size = 1024;
    static const size_t max_line = 50;

    struct line_t
    {
        char * str;
        int cx;
    };

    char buffer[buffer_size];
    line_t lines[max_line];
    int x_text;
    int y_text;
    int cy_text;
    bool auto_resize;
    BGRColor bg_color;
    BGRColor fg_color;
    Font const & font;
};

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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *              Meng Tan
 */

#pragma once

#include "widget.hpp"
#include "multiline.hpp"

class WidgetTooltip : public Widget
{
public:
    WidgetTooltip(gdi::GraphicApi & drawable, Widget & parent,
                  NotifyApi* notifier, const char * text,
                  BGRColor fgcolor, BGRColor bgcolor, BGRColor border_color, Font const & font);

    ~WidgetTooltip() override;

    Dimension get_optimal_dim() override;

    void set_text(const char * text);

    void rdp_input_invalidate(Rect clip) override;

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using Widget::set_wh;

    void draw_border(const Rect clip);

private:
    uint w_border;
    uint h_border;
    WidgetMultiLine desc;
    BGRColor border_color;
};

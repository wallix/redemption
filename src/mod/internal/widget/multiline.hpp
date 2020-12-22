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
#include "gdi/text_metrics.hpp"

class WidgetMultiLine : public Widget
{
public:
    WidgetMultiLine(gdi::GraphicApi & drawable, Widget& parent,
                    NotifyApi* notifier, char const* text, int group_id,
                    BGRColor fgcolor, BGRColor bgcolor, Font const & font,
                    int xtext = 0, int ytext = 0); /*NOLINT*/

    void set_text(const char * text);
    void set_text(const char * text, unsigned max_width);
    void set_text(gdi::MultiLineTextMetrics&& line_metrics);

    void rdp_input_invalidate(Rect clip) override;

    Dimension get_optimal_dim() override;

    [[nodiscard]] BGRColor get_bg_color() const noexcept
    {
        return this->bg_color;
    }

private:
    gdi::MultiLineTextMetrics line_metrics;

    int x_text;
    int y_text;
    int cy_text;
    BGRColor bg_color;
    BGRColor fg_color;
    Font const & font;
};

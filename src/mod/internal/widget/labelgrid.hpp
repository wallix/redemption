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
  Copyright (C) Wallix 2014
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#pragma once

#include "mod/internal/widget/grid.hpp"

struct WidgetLabelGrid : public WidgetGrid
{
    static constexpr unsigned x_padding_label = 3;
    static constexpr unsigned y_padding_label = 1;

    Font const & font;

    WidgetLabelGrid(gdi::GraphicApi & drawable, Widget & parent,
               NotifyApi * notifier, uint16_t nb_lines, uint16_t nb_columns,
               BGRColor bg_color_1, BGRColor fg_color_1,
               BGRColor bg_color_2, BGRColor fg_color_2,
               BGRColor bg_color_focus, BGRColor fg_color_focus,
               BGRColor bg_color_selection, BGRColor fg_color_selection,
               Font const & font, uint16_t border = 0, int group_id = 0);

    ~WidgetLabelGrid() override;

    void clear() override;
    void clean_labels();

    void add_line(array_view<const array_view_const_char> entries);

    const char * get_cell_text(uint16_t row_index, uint16_t column_index);
};

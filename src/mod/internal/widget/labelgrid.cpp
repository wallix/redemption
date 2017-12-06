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

#include "mod/internal/widget/labelgrid.hpp"
#include "mod/internal/widget/label.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/log.hpp"

WidgetLabelGrid::WidgetLabelGrid(
    gdi::GraphicApi & drawable, Widget & parent,
    NotifyApi * notifier, uint16_t nb_lines, uint16_t nb_columns,
    BGRColor bg_color_1, BGRColor fg_color_1,
    BGRColor bg_color_2, BGRColor fg_color_2,
    BGRColor bg_color_focus, BGRColor fg_color_focus,
    BGRColor bg_color_selection, BGRColor fg_color_selection,
    Font const & font, uint16_t border, int group_id
)
    : WidgetGrid(
        drawable, parent, notifier, nb_lines, nb_columns,
        bg_color_1, fg_color_1, bg_color_2, fg_color_2,
        bg_color_focus, fg_color_focus,
        bg_color_selection, fg_color_selection, border, group_id)
    , toDelete()
    , font(font)
{}

WidgetLabelGrid::~WidgetLabelGrid()
{
    this->clean_labels();
}

void WidgetLabelGrid::clear()
{
    this->clean_labels();
    WidgetGrid::clear();
}

void WidgetLabelGrid::clean_labels()
{
    for (int i = 0; i < GRID_NB_COLUMNS_MAX; i++) {
        for (int j = 0; j < GRID_NB_ROWS_MAX; j++) {
            if (this->toDelete[i][j]) {
                if (this->widgets[i][j]) {
                    delete this->widgets[i][j];
                    this->widgets[i][j] = nullptr;
                }
            }
        }
    }
}

uint16_t WidgetLabelGrid::add_line(const char ** entries)
{
    assert(this->nb_rows <= GRID_NB_ROWS_MAX);
    for (int i = 0; i < this->nb_columns; i++) {
        bool odd = this->nb_rows & 1;
        WidgetLabel * label = new WidgetLabel(
            this->drawable, *this, this,
            entries[i], this->group_id,
            odd ? this->fg_color_1 : this->fg_color_2,
            odd ? this->bg_color_1 : this->bg_color_2,
            this->font, x_padding_label, y_padding_label
        );

        Dimension dim = label->get_optimal_dim();
        label->set_wh(dim);

        label->tool = true;
        this->set_widget(this->nb_rows, i, label);
        this->toDelete[i][this->nb_rows] = true;
    }
    return this->nb_rows++;
}

const char * WidgetLabelGrid::get_cell_text(uint16_t row_index, uint16_t column_index)
{
    const char * result = "";
    if (this->toDelete[column_index][row_index]) {
        WidgetLabel * label = static_cast<WidgetLabel*>(this->widgets[column_index][row_index]);
        result = label->get_text();
    }
    return result;
}

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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_LABELGRID_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_LABELGRID_HPP_

#include "log.hpp"
#include "widget.hpp"
#include "label.hpp"
#include "grid.hpp"

struct WidgetLabelGrid : public WidgetGrid {

    bool toDelete[GRID_NB_COLUMNS_MAX][GRID_NB_ROWS_MAX];

    WidgetLabelGrid(DrawApi & drawable, const Rect & rect, Widget2 & parent,
               NotifyApi * notifier, uint16_t nb_lines, uint16_t nb_columns,
               uint32_t bg_color_1, uint32_t fg_color_1,
               uint32_t bg_color_2, uint32_t fg_color_2,
               uint32_t bg_color_focus, uint32_t fg_color_focus,
               uint32_t bg_color_selection, uint32_t fg_color_selection,
               uint16_t border = 0, int group_id = 0)
        : WidgetGrid(drawable, rect, parent, notifier, nb_lines, nb_columns,
                     bg_color_1, fg_color_1, bg_color_2, fg_color_2,
                     bg_color_focus, fg_color_focus,
                     bg_color_selection, fg_color_selection, border, group_id)
        , toDelete()
    {
    }

    virtual ~WidgetLabelGrid() {
        this->clean_labels();
    }

    void clean_labels() {
        for (int i = 0; i < GRID_NB_COLUMNS_MAX; i++) {
            for (int j = 0; j < GRID_NB_ROWS_MAX; j++) {
                if (this->toDelete[i][j]) {
                    if (this->widgets[i][j]) {
                        delete this->widgets[i][j];
                        this->widgets[i][j] = NULL;
                    }
                }
            }
        }
    }

    void add_line(const char ** entries, uint16_t row_index) {
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        for (int i = 0; i < this->nb_columns; i++) {
            bool odd = row_index & 1;
            Widget2 * label = new WidgetLabel(this->drawable, 0, 0, *this, this,
                                              entries[i], true, this->group_id,
                                              odd ? this->fg_color_1 : this->fg_color_2,
                                              odd ? this->bg_color_1 : this->bg_color_2);
            this->set_widget(row_index, i, label);
            this->toDelete[i][row_index] = true;
        }
    }

};


#endif

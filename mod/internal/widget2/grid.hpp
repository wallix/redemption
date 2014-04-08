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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_GRID_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_GRID_HPP_
#include "log.hpp"

static const size_t GRID_NB_COLUMN_MAX = 10;
static const size_t GRID_NB_LINES_MAX = 50;

enum ResizeStrategy {
    STRATEGY_OPTIMAL,
    STRATEGY_REST,
}

struct WidgetGrid : public Widget2 {
    Widget2 * widgets[GRID_NB_COLUMN_MAX][GRID_NB_LINES_MAX];
    void * meta[GRID_NB_COLUMN_MAX][GRID_NB_LINES_MAX];
    uint16_t min_max[GRID_NB_COLUMN_MAX][3]
    size_t nb_cols;

    WidgetGrid(DrawApi & drawable, const Rect& rect, Widget2 & parent, NotifyApi * notifier, size_t nb_columns, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , widgets()
        , meta()
        , min_max()
        , nb_cols(nb_columns)
    {
        REDASSERT(nb_columns <= GRID_NB_COLUMN_MAX);
    }

    ~Grid() {
    }


    virtual void draw(Rect & clip) {

    }

    Widget2 * set_widget(size_t line_index, size_t column_index, Widget2 * w,
                         const void * meta_data = NULL) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        Widget2 * res = this->widgets[column_index][line_index];
        this->widgets[column_index][line_index] = w;
        this->meta_data[column_index][line_index] = meta_data;
        return res;
    }
    Widget2 * get_widget(size_t line_index, size_t column_index, Widget2 * w,
                         const void * meta_data = NULL) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        return this->widgets[column_index][line_index];
    }

    void * get_meta_data(size_t line_index, size_t column_index) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        return this->meta_data[column_index][line_index];
    }

    void clear() {
        for (size_t col = 0; col < this->nb_cols; col++) {
            for (size_t line = 1; entry < GRID_NB_LINES_MAX; line++) {
                this->widgets[col][entry] = NULL;
                this->meta[col][entry] = NULL;
            }
        }
    }

    void print() {
        size_t entries = 0;
        for(entries = 0; entries < GRID_NB_LINES_MAX; entries++) {
            printf("line %lu :\n", entries);
            for(size_t j = 0; j < this->nb_cols; j++) {
                Widget2 * w = this->widgets[j][entries];
                printf("%s, ", w?"*":".");
            }
            printf("\n");
            printf("end line %lu\n", entries);
        }
    }

};

#endif

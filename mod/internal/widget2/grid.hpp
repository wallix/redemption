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
#include "widget.hpp"

static const size_t GRID_NB_COLUMN_MAX = 10;
static const size_t GRID_NB_LINES_MAX = 50;

enum SizingStrategy {
    STRATEGY_OPTIMAL, // Use minimal, optimal and maximal triplet to determinate the width of a column.
    STRATEGY_WEIGHT,
};

enum SizingStrategyInfo {
    INFO_STRATEGY = 0,
    INFO_MAX = 1,
    INFO_WEIGHT = 1,
    INFO_MIN = 2,

    INFO_TOTAL = 3
};

struct WidgetGrid : public Widget2 {
    Widget2 * widgets[GRID_NB_COLUMN_MAX][GRID_NB_LINES_MAX];
    void * meta_data[GRID_NB_COLUMN_MAX][GRID_NB_LINES_MAX];
    uint16_t sizing_strategy[GRID_NB_COLUMN_MAX][INFO_TOTAL];
    size_t nb_cols;
    size_t nb_lines;

private:
    uint16_t column_width[GRID_NB_COLUMN_MAX];
    uint16_t line_height[GRID_NB_LINES_MAX];

public:
    uint32_t bg_color_1;
    uint32_t fg_color_1;

    uint32_t bg_color_2;
    uint32_t fg_color_2;

    uint32_t bg_color_focus;
    uint32_t fg_color_focus;

    uint32_t bg_color_selection;
    uint32_t fg_color_selection;

    uint16_t border;

    uint16_t selection_index;

    bool need_rearrange;

    struct difftimer {
        uint64_t t;

        difftimer(uint64_t start = 0)
            : t(start)
        {}

        uint64_t tick()
        {
            uint64_t ret = this->t;
            this->t = ustime();
            return this->t - ret;
        }

        void update()
        {
            this->t = ustime();
        }
    } click_interval;

    WidgetGrid(DrawApi & drawable, const Rect& rect, Widget2 & parent, NotifyApi * notifier, size_t nb_columns,
               uint32_t bg_color_1, uint32_t fg_color_1, uint32_t bg_color_2, uint32_t fg_color_2,
               uint32_t bg_color_focus, uint32_t fg_color_focus, uint32_t bg_color_selection, uint32_t fg_color_selection,
               uint16_t border = 0, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , widgets()
        , meta_data()
        , sizing_strategy()
        , nb_cols(nb_columns)
        , nb_lines(0)
        , column_width()
        , line_height()
        , bg_color_1(bg_color_1)
        , fg_color_1(fg_color_1)
        , bg_color_2(bg_color_2)
        , fg_color_2(fg_color_2)
        , bg_color_focus(bg_color_focus)
        , fg_color_focus(fg_color_focus)
        , bg_color_selection(bg_color_selection)
        , fg_color_selection(fg_color_selection)
        , border(border)
        , selection_index(static_cast<uint16_t>(-1u))
        , need_rearrange(false)
        , click_interval()
    {
        REDASSERT(nb_columns <= GRID_NB_COLUMN_MAX);
    }

    virtual ~WidgetGrid() {
    }

    void clear() {
        for (size_t col = 0; col < this->nb_cols; col++) {
            for (size_t line = 1; line < GRID_NB_LINES_MAX; line++) {
                this->widgets[col][line] = NULL;
                this->meta_data[col][line] = NULL;
            }
        }
    }

private:
    void compute_dimension() {
        memset(this->column_width, 0, sizeof(this->column_width));
        memset(this->line_height, 0, sizeof(this->line_height));

        unsigned line_index = 0;
        for (; line_index < GRID_NB_LINES_MAX; line_index++) {
            bool line_empty = true;
            for (unsigned column_index = 0; column_index < this->nb_cols; column_index++) {
                Widget2 * w = this->widgets[column_index][line_index];
                if (!w) {
                    continue;
                }
                line_empty = false;

                Dimension dim = w->get_optimal_dim();
                if (this->sizing_strategy[column_index][INFO_STRATEGY] == STRATEGY_OPTIMAL) {
                    if (this->column_width[column_index] < dim.w) {
                        this->column_width[column_index] = dim.w;
                    }
                }

                if (this->line_height[line_index] < dim.h) {
                    this->line_height[line_index] = dim.h;
                }
            }

            if (line_empty) {
                break;
            }
        }

        this->nb_lines = line_index;

        uint16_t used_width         = 0;
        uint16_t total_weight       = 0;
        uint16_t total_border_width = 0;
        for (unsigned column_index = 0; column_index < this->nb_cols; column_index++) {
            if (this->sizing_strategy[column_index][INFO_STRATEGY] == STRATEGY_OPTIMAL) {
                if (this->column_width[column_index] < this->sizing_strategy[column_index][INFO_MIN]) {
                    this->column_width[column_index] = this->sizing_strategy[column_index][INFO_MIN];
                }
                else if (this->column_width[column_index] > this->sizing_strategy[column_index][INFO_MAX]) {
                    this->column_width[column_index] = this->sizing_strategy[column_index][INFO_MAX];
                }

                used_width += this->column_width[column_index];
                total_border_width += (this->column_width[column_index] ? this->border * 2 : 0);
            }
            else {
                total_weight += this->sizing_strategy[column_index][INFO_WEIGHT];
                total_border_width += (this->sizing_strategy[column_index][INFO_WEIGHT] ? this->border * 2 : 0);
            }
        }

        if (!total_weight) {
            total_weight = 1;
        }

        uint16_t unused_width = this->rect.cx - total_border_width - used_width;
        for (unsigned column_index = 0; column_index < this->nb_cols; column_index++) {
            if (this->sizing_strategy[column_index][INFO_STRATEGY] == STRATEGY_WEIGHT) {
                this->column_width[column_index] = this->sizing_strategy[column_index][INFO_WEIGHT] * unused_width / total_weight;
            }
        }
    }

public:
    virtual void draw(const Rect & clip) {
        if (this->need_rearrange) {
            this->compute_dimension();

            this->need_rearrange = false;
        }

        for (unsigned line_index = 0; line_index < this->nb_lines; line_index++) {
            this->draw_line(line_index, clip);
        }
    }

    void draw_line(uint16_t line_index, const Rect & clip) {
        if (this->need_rearrange) {
            this->compute_dimension();

            this->need_rearrange = false;
        }

        bool odd = line_index & 1;

        uint16_t y = this->rect.y;
        for (uint16_t l_index = 0, l_count = std::min<uint16_t>(line_index, this->nb_lines);
             l_index < l_count; l_index++) {
            y += this->line_height[l_index] + this->border * 2;
        }

        uint32_t bg_color;
        uint32_t fg_color;

        if (this->selection_index == line_index) {
            bg_color = (this->has_focus ? this->bg_color_focus : this->bg_color_selection);
            fg_color = (this->has_focus ? this->fg_color_focus : this->fg_color_selection);
        }
        else {
            bg_color = (odd ? this->bg_color_1 : this->bg_color_2);
            fg_color = (odd ? this->fg_color_1 : this->fg_color_2);
        }

        uint16_t x = this->rect.x;

        Rect rectLine(x, y, this->rect.cx, this->line_height[line_index] + this->border * 2);
        this->drawable.draw(RDPOpaqueRect(rectLine, bg_color), clip);

        x += this->border;
        y += this->border;

        for (unsigned column_index = 0; column_index < this->nb_cols; column_index++) {
            Widget2 * w = this->widgets[column_index][line_index];
            Rect rectCell(x, y, this->column_width[column_index], this->line_height[line_index]);
            if (w) {
                w->set_xy(rectCell.x, rectCell.y);
                w->set_wh(rectCell.cx, rectCell.cy);

                w->set_color(bg_color, fg_color);

                Rect finalRect = clip.intersect(rectCell);
                if (!finalRect.isempty()) {
                    w->draw(finalRect);
                }
            }

            x += this->column_width[column_index] + this->border * 2;
        }
    }

    void * get_meta_data(size_t line_index, size_t column_index) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        return this->meta_data[column_index][line_index];
    }
    void * set_meta_data(size_t line_index, size_t column_index, void * meta_data) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        void * res = this->meta_data[column_index][line_index];
        this->meta_data[column_index][line_index] = meta_data;
        return res;
    }

    Widget2 * get_widget(size_t line_index, size_t column_index) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        return this->widgets[column_index][line_index];
    }
    Widget2 * set_widget(size_t line_index, size_t column_index, Widget2 * w,
                         void * meta_data = NULL) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT(line_index <= GRID_NB_LINES_MAX);
        Widget2 * res = this->widgets[column_index][line_index];
        this->widgets[column_index][line_index] = w;
        this->meta_data[column_index][line_index] = meta_data;

        this->need_rearrange = true;
        return res;
    }

    void set_sizing_strategy(size_t column_index, SizingStrategy sizing_strategy, uint16_t max_or_weight, uint16_t min = 0) {
        REDASSERT(column_index <= this->nb_cols);
        REDASSERT((sizing_strategy == STRATEGY_OPTIMAL) || (min == 0));

        this->sizing_strategy[column_index][INFO_STRATEGY] = sizing_strategy;
        this->sizing_strategy[column_index][INFO_MAX]      = max_or_weight;
        this->sizing_strategy[column_index][INFO_MIN]      = min;

        this->need_rearrange = true;
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        if (this->need_rearrange) {
            this->compute_dimension();

            this->need_rearrange = false;
        }


        for (unsigned line_index = 0; line_index < this->nb_lines; line_index++) {
            bool empty_line = true;
            for (unsigned column_index = 0; column_index < this->nb_cols; column_index++) {
                if (this->widgets[column_index][line_index]) {
                    empty_line = false;

                    if (this->widgets[column_index][line_index]->rect.contains_pt(x, y)) {
                        return this->widgets[column_index][line_index];
                    }
                }
            }

            if (empty_line) {
                break;
            }
        }
        return NULL;
    }

    void set_selected_index(uint16_t line_index) {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            if (this->need_rearrange) {
                this->compute_dimension();

                this->need_rearrange = false;
            }

            uint16_t y = this->rect.y;
            for (uint16_t line_index = 0; line_index < this->nb_lines; line_index++) {
                y += this->line_height[line_index] + this->border * 2;
                uint16_t x = this->rect.x;

                Rect rectLine(x, y, this->rect.cx, this->line_height[line_index] + this->border * 2);

                if (rectLine.contains_pt(x, y)) {
                    if (line_index != this->selection_index) {
                        this->click_interval.update();
                        this->set_selected_index(line_index);
                    }
                    else {
                        if (this->click_interval.tick() <= 700000LL) {
                            this->send_notify(NOTIFY_SUBMIT);
                            return ;
                        }
                    }
                }
            }
        }

        Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }
};

#endif

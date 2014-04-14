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

static const uint16_t GRID_NB_COLUMNS_MAX = 10;
static const uint16_t GRID_NB_ROWS_MAX    = 50;

/*
enum SizingStrategy {
    STRATEGY_OPTIMAL, // Use minimal, optimal and maximal triplet to determinate the width of a column.
    STRATEGY_WEIGHT,
};
*/

enum SizingStrategyInfo {
//    INFO_STRATEGY = 0,
    INFO_MAX = 1,
//    INFO_WEIGHT = 1,
    INFO_MIN = 2,

    INFO_TOTAL = 2
};

struct WidgetGrid : public Widget2 {
private:
    Widget2  * widgets[GRID_NB_COLUMNS_MAX][GRID_NB_ROWS_MAX];
    void     * meta_data[GRID_NB_COLUMNS_MAX][GRID_NB_ROWS_MAX];
    uint16_t   sizing_strategy[GRID_NB_COLUMNS_MAX][INFO_TOTAL];

    uint16_t nb_columns;
    uint16_t nb_rows;

    uint16_t column_width[GRID_NB_COLUMNS_MAX];
    uint16_t row_height[GRID_NB_ROWS_MAX];

    uint32_t bg_color_1;    // Odd
    uint32_t fg_color_1;

    uint32_t bg_color_2;    // Even
    uint32_t fg_color_2;

    uint32_t bg_color_focus;
    uint32_t fg_color_focus;

    uint32_t bg_color_selection;
    uint32_t fg_color_selection;

    uint16_t border;    // Width and height of cell's border.

    uint16_t selection_y;   // Index of seleted row.

    bool need_rearrange;

    struct difftimer {
        uint64_t t;

        difftimer(uint64_t start = 0)
            : t(start)
        {}

        uint64_t tick() {
            uint64_t ret = this->t;
            this->t = ustime();
            return this->t - ret;
        }

        void update() {
            this->t = ustime();
        }
    } click_interval;

public:
    WidgetGrid(DrawApi & drawable, const Rect & rect, Widget2 & parent, NotifyApi * notifier, uint16_t nb_columns,
               uint32_t bg_color_1, uint32_t fg_color_1, uint32_t bg_color_2, uint32_t fg_color_2,
               uint32_t bg_color_focus, uint32_t fg_color_focus, uint32_t bg_color_selection, uint32_t fg_color_selection,
               uint16_t border = 0, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , widgets()
        , meta_data()
        , sizing_strategy()
        , nb_columns(nb_columns)
        , nb_rows(0)
        , column_width()
        , row_height()
        , bg_color_1(bg_color_1)
        , fg_color_1(fg_color_1)
        , bg_color_2(bg_color_2)
        , fg_color_2(fg_color_2)
        , bg_color_focus(bg_color_focus)
        , fg_color_focus(fg_color_focus)
        , bg_color_selection(bg_color_selection)
        , fg_color_selection(fg_color_selection)
        , border(border)
        , selection_y(static_cast<uint16_t>(-1u))
        , need_rearrange(false)
        , click_interval()
    {
        REDASSERT(nb_columns <= GRID_NB_COLUMNS_MAX);
    }

    virtual ~WidgetGrid() {
    }

    void clear() {
        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            for (uint16_t row_index = 1; row_index < GRID_NB_ROWS_MAX; row_index++) {
                this->widgets[column_index][row_index]   = NULL;
                this->meta_data[column_index][row_index] = NULL;
            }
        }
    }

private:
    void do_rearrange() {
        memset(this->column_width, 0, sizeof(this->column_width));
        memset(this->row_height, 0, sizeof(this->row_height));

        uint16_t column_width_optimal[GRID_NB_COLUMNS_MAX] = { 0 };

        uint16_t row_index = 0;
        for (; row_index < GRID_NB_ROWS_MAX; row_index++) {
            bool row_empty = true;
            for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
                Widget2 * w = this->widgets[column_index][row_index];
                if (!w) {
                    continue;
                }
                row_empty = false;

                Dimension dim = w->get_optimal_dim();
                if (column_width_optimal[column_index] < dim.w) {
                    column_width_optimal[column_index] = dim.w;
                }
                // if (this->column_width[column_index] < dim.w) {
                //     this->column_width[column_index] = dim.w;
                // }

                if (this->row_height[row_index] < dim.h) {
                    this->row_height[row_index] = dim.h;
                }
            }

            if (row_empty) {
                break;
            }
        }
        this->nb_rows = row_index;


        TODO("Optiomize this");
        uint16_t unsatisfied_column_count = 0;
        // min
        int16_t unused_width = static_cast<int16_t>(this->rect.cx - this->border * 2 * this->nb_columns);
        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            this->column_width[column_index] = this->sizing_strategy[column_index][INFO_MIN];
            unused_width -= static_cast<int16_t>(this->sizing_strategy[column_index][INFO_MIN]);

            if (this->column_width[column_index] < std::min(column_width_optimal[column_index], this->sizing_strategy[column_index][INFO_MAX])) {
                unsatisfied_column_count++;
            }
        }
        // optimal
        while ((unused_width > 0) && (unsatisfied_column_count > 0)) {
            uint16_t part = unused_width / unsatisfied_column_count;
            if (!part) {
                break;
            }
            unsatisfied_column_count = 0;
            for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
                uint16_t optimal_max = std::min(column_width_optimal[column_index], this->sizing_strategy[column_index][INFO_MAX]);
                if (this->column_width[column_index] < optimal_max) {
                    uint16_t ajusted_part = std::min<uint16_t>(part, optimal_max - this->column_width[column_index]);
                    this->column_width[column_index] += ajusted_part;
                    unused_width -= ajusted_part;

                    if (this->column_width[column_index] < optimal_max) {
                        unsatisfied_column_count++;
                    }
                }
            }
        }
        // max
        unsatisfied_column_count = 0;
        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            if (this->column_width[column_index] < this->sizing_strategy[column_index][INFO_MAX]) {
                unsatisfied_column_count++;
            }
        }
        while ((unused_width > 0) && (unsatisfied_column_count > 0)) {
            uint16_t part = unused_width / unsatisfied_column_count;
            if (!part) {
                break;
            }
            unsatisfied_column_count = 0;
            for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
                if (this->column_width[column_index] < this->sizing_strategy[column_index][INFO_MAX]) {
                    uint16_t ajusted_part = std::min<uint16_t>(part, this->sizing_strategy[column_index][INFO_MAX] - this->column_width[column_index]);
                    this->column_width[column_index] += ajusted_part;
                    unused_width -= ajusted_part;

                    if (this->column_width[column_index] < this->sizing_strategy[column_index][INFO_MAX]) {
                        unsatisfied_column_count++;
                    }
                }
            }
        }
    }

public:
    virtual void draw(const Rect & clip) {
        if (this->need_rearrange) {
            this->do_rearrange();

            this->need_rearrange = false;
        }

        for (uint16_t row_index = 0; row_index < this->nb_rows; row_index++) {
            this->draw_row(row_index, clip);
        }
    }

    void draw_row(uint16_t row_index, const Rect & clip) {
        if (this->need_rearrange) {
            this->do_rearrange();

            this->need_rearrange = false;
        }

        bool odd = row_index & 1;

        uint32_t bg_color;
        uint32_t fg_color;

        if (this->selection_y == row_index) {
            bg_color = (this->has_focus ? this->bg_color_focus : this->bg_color_selection);
            fg_color = (this->has_focus ? this->fg_color_focus : this->fg_color_selection);
        }
        else {
            bg_color = (odd ? this->bg_color_1 : this->bg_color_2);
            fg_color = (odd ? this->fg_color_1 : this->fg_color_2);
        }

        uint16_t y = this->rect.y;
        for (uint16_t r_index = 0, r_count = std::min<uint16_t>(row_index, this->nb_rows);
             r_index < r_count; r_index++) {
            y += this->row_height[r_index] + this->border * 2;
        }

        uint16_t x = this->rect.x;
        Rect rectRow(x, y, this->rect.cx, this->row_height[row_index] + this->border * 2);
        this->drawable.draw(RDPOpaqueRect(rectRow, bg_color), clip);

        x += this->border;
        y += this->border;

        for (uint16_t column_index = 0; column_index < this->nb_columns; column_index++) {
            Widget2 * w = this->widgets[column_index][row_index];
            Rect rectCell(x, y, this->column_width[column_index], this->row_height[row_index]);
            if (w) {
                w->set_xy(rectCell.x, rectCell.y);
                w->set_wh(rectCell.cx, rectCell.cy);

                w->set_color(bg_color, fg_color);

                Rect destRect = clip.intersect(rectCell);
                if (!destRect.isempty()) {
                    w->draw(destRect);
                }
            }

            x += this->column_width[column_index] + this->border * 2;
        }
    }

    uint16_t get_column_width(uint16_t column_index) {
        REDASSERT(column_index < this->nb_columns);

        return this->column_width[column_index];
    }

    void * get_meta_data(uint16_t row_index, uint16_t column_index) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        return this->meta_data[column_index][row_index];
    }
    void * set_meta_data(uint16_t row_index, uint16_t column_index, void * meta_data) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        void * res = this->meta_data[column_index][row_index];
        this->meta_data[column_index][row_index] = meta_data;
        return res;
    }

    Widget2 * get_widget(uint16_t row_index, uint16_t column_index) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        return this->widgets[column_index][row_index];
    }
    Widget2 * set_widget(uint16_t row_index, uint16_t column_index, Widget2 * w,
                         void * meta_data = NULL) {
        REDASSERT(column_index <= this->nb_columns);
        REDASSERT(row_index <= GRID_NB_ROWS_MAX);
        Widget2 * res = this->widgets[column_index][row_index];
        this->widgets[column_index][row_index]   = w;
        this->meta_data[column_index][row_index] = meta_data;

        this->need_rearrange = true;
        return res;
    }

    void set_sizing_strategy(uint16_t column_index, /*SizingStrategy sizing_strategy, uint16_t max_or_weight, */
                             uint16_t min, uint16_t max) {
        REDASSERT(column_index <= this->nb_columns);
//        REDASSERT((sizing_strategy != STRATEGY_OPTIMAL) || (max_or_weight >= min));
        REDASSERT((max >= min));

//        this->sizing_strategy[column_index][INFO_STRATEGY] = sizing_strategy;
        this->sizing_strategy[column_index][INFO_MIN] = min;
        this->sizing_strategy[column_index][INFO_MAX] = max;

        this->need_rearrange = true;
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (this->need_rearrange) {
            this->do_rearrange();

            this->need_rearrange = false;
        }

        for (unsigned row_index = 0; row_index < this->nb_rows; row_index++) {
            bool empty_row = true;
            for (unsigned column_index = 0; column_index < this->nb_columns; column_index++) {
                if (this->widgets[column_index][row_index]) {
                    empty_row = false;

                    if (this->widgets[column_index][row_index]->rect.contains_pt(x, y)) {
                        return this->widgets[column_index][row_index];
                    }
                }
            }

            if (empty_row) {
                break;
            }
        }

        return NULL;
    }

    void get_selection(uint16_t & row_index, uint16_t & column_index) {
        row_index    = selection_y;
        column_index = static_cast<uint16_t>(-1);
    }
    void set_selection(uint16_t row_index, uint16_t column_index) {
        if (this->focus_flag == Widget2::IGNORE_FOCUS) {
            return;
        }

        if (row_index != this->selection_y) {
            uint16_t previous_selection_y = this->selection_y;
            this->selection_y = row_index;

            if (previous_selection_y < this->nb_rows) {
                this->draw_row(previous_selection_y, this->rect);
            }
            if (this->selection_y < this->nb_rows) {
                this->draw_row(this->selection_y, this->rect);
            }
        }
    }

    virtual void rdp_input_mouse(int device_flags, int mouse_x, int mouse_y, Keymap2 * keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            if (this->need_rearrange) {
                this->do_rearrange();

                this->need_rearrange = false;
            }

            uint16_t y = this->rect.y;
            for (uint16_t row_index = 0; row_index < this->nb_rows; row_index++) {
                uint16_t x = this->rect.x;
                Rect rectRow(x, y, this->rect.cx, this->row_height[row_index] + this->border * 2);

                if (rectRow.contains_pt(mouse_x, mouse_y)) {
                    if (row_index != this->selection_y) {
                        this->click_interval.update();
                        this->set_selection(row_index, static_cast<uint16_t>(-1));
                    }
                    else {
                        if (this->click_interval.tick() <= 700000LL) {
                            this->send_notify(NOTIFY_SUBMIT);
                            return;
                        }
                    }
                }

                y += this->row_height[row_index] + this->border * 2;
            }
        }

        Widget2::rdp_input_mouse(device_flags, mouse_x, mouse_y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2 * keymap) {
        if (keymap->nb_kevent_available() > 0) {
            if (this->need_rearrange) {
                this->do_rearrange();

                this->need_rearrange = false;
            }

            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->nb_rows > 1) {
                        this->set_selection(((this->selection_y > 0) ? this->selection_y - 1 : this->nb_rows - 1),
                                            static_cast<uint16_t>(-1));
                    }
                break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->nb_rows > 1) {
                        this->set_selection(((this->selection_y + 1 != this->nb_rows) ? this->selection_y + 1 : 0),
                                            static_cast<uint16_t>(-1));
                    }
                break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if ((this->nb_rows > 1) && (this->nb_rows - 1 != this->selection_y)) {
                        this->set_selection(this->nb_rows - 1, static_cast<uint16_t>(-1));
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if ((this->nb_rows > 1) && this->selection_y) {
                        this->set_selection(0, static_cast<uint16_t>(-1));
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    if (this->nb_rows) {
                        this->send_notify(NOTIFY_SUBMIT);
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }
};

#endif

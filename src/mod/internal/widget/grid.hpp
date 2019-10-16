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

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/internal/widget/widget.hpp"
#include "utils/difftimeval.hpp"
#include "utils/sugar/update_lock.hpp"

#include <vector>

#define COLUMN_WIDTH_CORRECTOR 6

struct WidgetGrid : public Widget
{
private:
    struct Widgets
    {
        uint16_t nb_rows;
        uint16_t nb_columns;
        std::vector<std::unique_ptr<Widget>> widgets;
        std::vector<uint16_t> column_and_row_height;

        Widgets(uint16_t nb_rows, uint16_t nb_columns)
        : nb_rows(nb_rows)
        , nb_columns(nb_columns)
        , widgets(nb_columns * nb_rows)
        , column_and_row_height(nb_columns + nb_rows)
        {}

        array_view<std::unique_ptr<Widget>> add_line()
        {
            for (uint16_t i = 0; i < this->nb_columns; ++i) {
                this->widgets.emplace_back();
            }
            this->column_and_row_height.emplace_back(0);
            ++this->nb_rows;
            return this->line(this->nb_rows-1);
        }

        [[nodiscard]] array_view<const std::unique_ptr<Widget>> line(uint16_t i) const
        {
            auto* p = &this->widgets[i * this->nb_columns];
            return {p, p + this->nb_columns};
        }

        array_view<std::unique_ptr<Widget>> line(uint16_t i)
        {
            auto* p = &this->widgets[i * this->nb_columns];
            return {p, p + this->nb_columns};
        }

        [[nodiscard]] array_view<const uint16_t> row_heights() const
        {
            return make_array_view(this->column_and_row_height).from_offset(nb_columns);
        }

        [[nodiscard]] array_view<const uint16_t> column_widths() const
        {
            return make_array_view(this->column_and_row_height).subarray(0, nb_columns);
        }

        array_view<uint16_t> row_heights()
        {
            return make_array_view(this->column_and_row_height).from_offset(nb_columns);
        }

        array_view<uint16_t> column_widths()
        {
            return make_array_view(this->column_and_row_height).subarray(0, nb_columns);
        }

        void clear()
        {
            this->widgets.clear();
            this->nb_rows = 0;
        }
    };

    Widgets widgets;

public:
    const BGRColor bg_color_1;    // Odd
    const BGRColor fg_color_1;

    const BGRColor bg_color_2;    // Even
    const BGRColor fg_color_2;

    const BGRColor bg_color_focus;
    const BGRColor fg_color_focus;

    const BGRColor bg_color_selection;
    const BGRColor fg_color_selection;

    const uint16_t border;    // Width and height of cell's border.

private:
    uint16_t selection_y = static_cast<uint16_t>(-1u);   // Index of seleted row.

    // TODO: see why grid object need a difftimer ?
    struct difftimer {
        std::chrono::microseconds t = std::chrono::microseconds::zero();

        std::chrono::microseconds tick() {
            std::chrono::microseconds ret = this->t;
            this->t = ustime();
            return this->t - ret;
        }

        void update() {
            this->t = ustime();
        }
    } click_interval {};

public:
    WidgetGrid(gdi::GraphicApi & drawable, Widget & parent,
               NotifyApi * notifier, uint16_t nb_rows, uint16_t nb_columns,
               BGRColor bg_color_1, BGRColor fg_color_1,
               BGRColor bg_color_2, BGRColor fg_color_2,
               BGRColor bg_color_focus, BGRColor fg_color_focus,
               BGRColor bg_color_selection, BGRColor fg_color_selection,
               uint16_t border = 0, int group_id = 0)
        : Widget(drawable, parent, notifier, group_id)
        , widgets(nb_rows, nb_columns)
        , bg_color_1(bg_color_1)
        , fg_color_1(fg_color_1)
        , bg_color_2(bg_color_2)
        , fg_color_2(fg_color_2)
        , bg_color_focus(bg_color_focus)
        , fg_color_focus(fg_color_focus)
        , bg_color_selection(bg_color_selection)
        , fg_color_selection(fg_color_selection)
        , border(border)
    {}

    virtual void clear()
    {
        this->widgets.clear();
        this->selection_y = static_cast<uint16_t>(-1);
    }

    void rdp_input_invalidate(Rect clip) override
    {
        Rect const rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            update_lock lock{this->drawable};
            for (uint16_t row_index = 0; row_index < this->widgets.nb_rows; ++row_index) {
                this->draw_row(row_index, rect_intersect);
            }
        }
    }

    void draw_row(uint16_t row_index, Rect const clip)
    {
        BGRColor bg_color;
        BGRColor fg_color;

        if (this->selection_y == row_index) {
            bg_color = (this->has_focus ? this->bg_color_focus : this->bg_color_selection);
            fg_color = (this->has_focus ? this->fg_color_focus : this->fg_color_selection);
        }
        else {
            const bool odd = row_index & 1;
            bg_color = (odd ? this->bg_color_1 : this->bg_color_2);
            fg_color = (odd ? this->fg_color_1 : this->fg_color_2);
        }

        auto row_heights = this->widgets.row_heights();

        uint16_t y = this->y();
        for (auto h : row_heights.subarray(0, std::min(row_index, this->widgets.nb_rows))) {
            y += h + this->border * 2;
        }

        uint16_t x = this->x();
        Rect rectRow(x, y, this->cx(), row_heights[row_index] + this->border * 2);
        this->drawable.draw(RDPOpaqueRect(rectRow, encode_color24()(bg_color)), clip, gdi::ColorCtx::depth24());

        x += this->border;
        y += this->border;

        auto column_width_it = this->widgets.column_widths().begin();
        for (auto&& w : this->widgets.line(row_index)) {
            Rect rectCell(x, y, *column_width_it, row_heights[row_index]);
            if (w) {
                w->set_xy(rectCell.x, rectCell.y);
                w->set_wh(rectCell.cx - COLUMN_WIDTH_CORRECTOR, rectCell.cy);

                w->set_color(bg_color, fg_color);

                Rect destRect = clip.intersect(rectCell);
                if (!destRect.isempty()) {
                    w->rdp_input_invalidate(destRect);
                }
            }

            x += *column_width_it + this->border * 2;
            ++column_width_it;
        }
    }

    [[nodiscard]] uint16_t get_column_width(uint16_t column_index) const
    {
        assert(column_index < this->widgets.nb_columns);

        return this->widgets.column_widths()[column_index];
    }

    void set_column_width(uint16_t column_index, uint16_t width)
    {
        assert(column_index < this->widgets.nb_columns);

        this->widgets.column_widths()[column_index] = width;
    }

    [[nodiscard]] uint16_t get_nb_rows() const
    {
        return this->widgets.nb_rows;
    }

    [[nodiscard]] uint16_t get_nb_columns() const
    {
        return this->widgets.nb_columns;
    }

    array_view<std::unique_ptr<Widget>> add_line()
    {
        return this->widgets.add_line();
    }

    [[nodiscard]] uint16_t get_row_height(uint16_t row_index) const
    {
        return this->widgets.row_heights()[row_index];
    }

    void set_row_height(uint16_t row_index, uint16_t height)
    {
        this->widgets.row_heights()[row_index] = height;
    }

    [[nodiscard]] Widget* get_widget(uint16_t row_index, uint16_t column_index) const
    {
        return this->widgets.line(row_index)[column_index].get();
    }

    std::unique_ptr<Widget> remove_widget(uint16_t row_index, uint16_t column_index)
    {
        return std::move(this->widgets.line(row_index)[column_index]);
    }

    void set_widget(uint16_t row_index, uint16_t column_index, std::unique_ptr<Widget>&& w)
    {
        this->widgets.line(row_index)[column_index] = std::move(w);
    }

    Widget* widget_at_pos(int16_t x, int16_t y) override
    {
        auto&& row_heights = this->widgets.row_heights();
        uint16_t wy = this->y();
        uint16_t const wx = this->x();
        for (uint16_t row_index = 0; row_index < this->widgets.nb_rows; row_index++) {
            Rect rectRow(wx, wy, this->cx(), row_heights[row_index] + this->border * 2);

            if (rectRow.contains_pt(x, y)) {
                for (auto& w : this->widgets.line(row_index)) {
                    if (w && w->get_rect().contains_pt(x, y)) {
                        return w.get();
                    }
                }
                break;
            }

            wy += row_heights[row_index] + this->border * 2;
        }

        return nullptr;
    }

    void get_selection(uint16_t & row_index, uint16_t & column_index) const
    {
        row_index    = this->selection_y;
        column_index = static_cast<uint16_t>(-1);
    }

    void set_selection(uint16_t row_index)
    {
        if (this->focus_flag == Widget::IGNORE_FOCUS) {
            return;
        }

        if (row_index != this->selection_y) {
            uint16_t previous_selection_y = this->selection_y;
            this->selection_y = row_index;

            update_lock lock{this->drawable};
            if (previous_selection_y < this->widgets.nb_rows) {
                this->draw_row(previous_selection_y, this->get_rect());
            }
            if (this->selection_y < this->widgets.nb_rows) {
                this->draw_row(this->selection_y, this->get_rect());
            }
        }
    }

    void refresh_selected()
    {
        if (this->selection_y < this->widgets.nb_rows) {
            update_lock lock{this->drawable};
            this->draw_row(this->selection_y, this->get_rect());
        }
    }

    void focus(int reason) override
    {
        (void)reason;
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            this->refresh_selected();
        }
    }

    void blur() override
    {
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            this->refresh_selected();
        }
    }

    void rdp_input_mouse(int device_flags, int mouse_x, int mouse_y, Keymap2 * keymap) override
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            uint16_t y = this->y();
            uint16_t const x = this->x();
            auto&& row_heights = this->widgets.row_heights();
            for (uint16_t row_index = 0; row_index < this->widgets.nb_rows; row_index++) {
                Rect rectRow(x, y, this->cx(), row_heights[row_index] + this->border * 2);

                if (rectRow.contains_pt(mouse_x, mouse_y)) {
                    if (row_index != this->selection_y) {
                        this->click_interval.update();
                        this->set_selection(row_index);
                    }
                    else {
                        if (this->click_interval.tick() <= std::chrono::microseconds(700000L)) {
                            this->send_notify(NOTIFY_SUBMIT);
                            return;
                        }
                    }

                    break;
                }

                y += row_heights[row_index] + this->border * 2;
            }
        }
        else if (device_flags == MOUSE_FLAG_MOVE) {
            Widget * wid = this->widget_at_pos(mouse_x, mouse_y);
            if (wid) {
                wid->rdp_input_mouse(device_flags, mouse_x, mouse_y, keymap);
            }
        }

        Widget::rdp_input_mouse(device_flags, mouse_x, mouse_y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2 * keymap) override
    {
        if (keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->widgets.nb_rows > 1) {
                        this->set_selection((this->selection_y > 0) ? this->selection_y - 1 : this->widgets.nb_rows - 1);
                    }
                break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->widgets.nb_rows > 1) {
                        this->set_selection((this->selection_y + 1 != this->widgets.nb_rows) ? this->selection_y + 1 : 0);
                    }
                break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->widgets.nb_rows > 1 && this->widgets.nb_rows - 1 != this->selection_y) {
                        this->set_selection(this->widgets.nb_rows - 1);
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if ((this->widgets.nb_rows > 1) && this->selection_y) {
                        this->set_selection(0);
                    }
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    if (this->widgets.nb_rows) {
                        this->send_notify(NOTIFY_SUBMIT);
                    }
                    break;
                default:
                    Widget::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }
};

struct ColumnWidthStrategy
{
    uint32_t min;
    uint32_t weight;
};

inline
void compute_format(WidgetGrid const& grid, ColumnWidthStrategy* column_width_strategies, int priority_column_index,
    uint16_t* row_height, uint16_t* column_width, bool* width_is_optimal)
{
    BufMaker<16, uint16_t> column_width_optimal_buffer;
    auto column_width_optimal = column_width_optimal_buffer.dyn_array(grid.get_nb_columns());
    std::fill(column_width_optimal.begin(), column_width_optimal.end(), 0);
    std::fill(row_height, row_height+grid.get_nb_rows(), 0);
    std::fill(width_is_optimal, width_is_optimal+grid.get_nb_columns(), false);

    for (uint16_t row_index = 0; row_index < grid.get_nb_rows(); row_index++) {
        for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
            Widget * w = grid.get_widget(row_index, column_index);
            if (!w) {
                continue;
            }

            Dimension dim = w->get_optimal_dim();
            if (column_width_optimal[column_index] < dim.w + 2 + COLUMN_WIDTH_CORRECTOR) {
                column_width_optimal[column_index] = dim.w + 2 + COLUMN_WIDTH_CORRECTOR;
            }

            if (row_height[row_index] < dim.h) {
                row_height[row_index] = dim.h;
            }
        }
    }

    for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
        if (column_width[column_index]) {
            break;
        }
        if (column_index == grid.get_nb_columns() - 1) {
            uint32_t const total_weight = [](WidgetGrid const& grid, ColumnWidthStrategy* column_width_strategies) -> uint32_t {
                    uint32_t total_weight = 0;

                    for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                        total_weight += column_width_strategies[column_index].weight;
                    }

                    return (total_weight ? total_weight : 1);
                }(grid, column_width_strategies);

            uint16_t unused_width = static_cast<int16_t>(grid.cx() - grid.border * 2 * grid.get_nb_columns());

            uint16_t const unused_width_saved = unused_width;

            for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                uint16_t const extra_width = unused_width_saved * column_width_strategies[column_index].weight / total_weight;

                column_width[column_index] += extra_width;

                unused_width -= extra_width;
            }
            column_width[grid.get_nb_columns() - 1] += unused_width;
        }
    }

    for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
        if (column_width[column_index] < column_width_optimal[column_index]) {
            break;
        }
        if (column_index == grid.get_nb_columns() - 1) {
            std::fill(width_is_optimal, width_is_optimal+grid.get_nb_columns(), true);

            return;
        }
    }

    if (priority_column_index == -1) {
        priority_column_index = [](WidgetGrid const& grid, array_view<uint16_t> column_width_optimal) {
                int      widest_columns = -1;
                uint16_t wider = 0;

                for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                    if (wider < column_width_optimal[column_index]) {
                        wider = column_width_optimal[column_index];
                        widest_columns = column_index;
                    }
                }

                return widest_columns;
            }(grid, column_width_optimal);
    }

    uint16_t const total_width_optimal = [](WidgetGrid const& grid, array_view<uint16_t> column_width_optimal) -> uint16_t {
            uint16_t total_width_optimal = 0;

            for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                total_width_optimal += column_width_optimal[column_index];
            }

            return total_width_optimal;
        }(grid, column_width_optimal);

    uint16_t unused_width = static_cast<int16_t>(grid.cx() - grid.border * 2 * grid.get_nb_columns());

    if (total_width_optimal <= unused_width)
    {
        for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
            column_width[column_index] = column_width_optimal[column_index];

            unused_width -= column_width_optimal[column_index];
        }

        uint32_t const total_weight = [](WidgetGrid const& grid, array_view<uint16_t> column_width_optimal) -> uint32_t {
                uint32_t total_weight = 0;

                for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                    total_weight += column_width_optimal[column_index];
                }

                return (total_weight ? total_weight : 1);
            }(grid, column_width_optimal);

        uint16_t const unused_width_saved = unused_width;

        for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
            uint16_t const extra_width = unused_width_saved * column_width_optimal[column_index] / total_weight;

            column_width[column_index] += extra_width;

            unused_width -= extra_width;
        }
        column_width[grid.get_nb_columns() - 1] += unused_width;

        std::fill(width_is_optimal, width_is_optimal+grid.get_nb_columns(), true);
    }
    else
    {
        for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
            column_width[column_index] = column_width_strategies[column_index].min;

            unused_width -= column_width_strategies[column_index].min;
        }

        if (column_width_optimal[priority_column_index] < column_width[priority_column_index] + unused_width) {
            unused_width += column_width[priority_column_index];

            column_width[priority_column_index] = column_width_optimal[priority_column_index];

            unused_width -= column_width_optimal[priority_column_index];

            if (unused_width) {
                for (bool exit_processing = false; !exit_processing; ) {
                    exit_processing = true;

                    uint32_t const partial_weight = [](WidgetGrid const& grid, array_view<uint16_t> column_width_optimal, uint16_t* column_width) -> uint32_t {
                            uint32_t partial_weight = 0;

                            for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                                if (column_width[column_index] < column_width_optimal[column_index]) {
                                    partial_weight += column_width_optimal[column_index];
                                }
                            }

                            return (partial_weight ? partial_weight : 1);
                        }(grid, column_width_optimal, column_width);

                    uint16_t const unused_width_saved = unused_width;

                    for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
                        if (column_width[column_index] < column_width_optimal[column_index]) {
                            uint16_t extra_width = unused_width_saved * column_width_optimal[column_index] / partial_weight;

                            column_width[column_index] += extra_width;

                            unused_width -= extra_width;

                            if (column_width[column_index] > column_width_optimal[column_index]) {
                                unused_width += (column_width[column_index] - column_width_optimal[column_index]);

                                column_width[column_index] = column_width_optimal[column_index];

                                exit_processing = false;
                            }
                        }
                    }
                }

                column_width[grid.get_nb_columns() - 1] += unused_width;
            }
        }
        else {
            column_width[priority_column_index] += unused_width;
        }

        for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
            width_is_optimal[column_index] = ((column_index == priority_column_index) ||
                (column_width[column_index] >= column_width_optimal[column_index]));
        }
    }
}

inline
void apply_format(WidgetGrid & grid, uint16_t * row_height, uint16_t * column_width)
{
    uint16_t height = 0;
    for (uint16_t row_index = 0; row_index < grid.get_nb_rows(); row_index++) {
        grid.set_row_height(row_index, row_height[row_index]);
        height += row_height[row_index] + grid.border * 2;
    }
    grid.set_wh(grid.cx(), height);
    for (uint16_t column_index = 0; column_index < grid.get_nb_columns(); column_index++) {
        grid.set_column_width(column_index, column_width[column_index]);
    }
}

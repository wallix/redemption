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

#include "mod/internal/widget/widget.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "utils/monotonic_clock.hpp"

#include <vector>
#include <chrono>
#include <memory>


struct WidgetGrid : public Widget
{
private:
    struct Widgets
    {
        Widgets(uint16_t nb_rows, uint16_t nb_columns);

        writable_array_view<std::unique_ptr<Widget>> add_line();

        [[nodiscard]] array_view<std::unique_ptr<Widget>> line(uint16_t i) const;
        writable_array_view<std::unique_ptr<Widget>> line(uint16_t i);

        [[nodiscard]] array_view<uint16_t> row_heights() const;
        [[nodiscard]] array_view<uint16_t> column_widths() const;

        writable_array_view<uint16_t> row_heights();
        writable_array_view<uint16_t> column_widths();

        void clear();

    private:
        friend class WidgetGrid;

        uint16_t nb_rows;
        uint16_t nb_columns;
        std::vector<std::unique_ptr<Widget>> widgets;
        std::vector<uint16_t> column_and_row_height;
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
    struct difftimer
    {
        MonotonicTimePoint t {};

        MonotonicTimePoint::duration tick();

        void update();
    } click_interval {};

public:
    WidgetGrid(gdi::GraphicApi & drawable, Widget & parent,
               NotifyApi * notifier, uint16_t nb_rows, uint16_t nb_columns,
               BGRColor bg_color_1, BGRColor fg_color_1,
               BGRColor bg_color_2, BGRColor fg_color_2,
               BGRColor bg_color_focus, BGRColor fg_color_focus,
               BGRColor bg_color_selection, BGRColor fg_color_selection,
               uint16_t border = 0, int group_id = 0); /*NOLINT*/

    void clear();

    void rdp_input_invalidate(Rect clip) override;

    [[nodiscard]] uint16_t get_column_width(uint16_t column_index) const;

    void set_column_width(uint16_t column_index, uint16_t width);

    [[nodiscard]] uint16_t get_nb_rows() const;
    [[nodiscard]] uint16_t get_nb_columns() const;

    writable_array_view<std::unique_ptr<Widget>> add_line();

    [[nodiscard]] uint16_t get_row_height(uint16_t row_index) const;

    void set_row_height(uint16_t row_index, uint16_t height);

    [[nodiscard]] Widget* get_widget(uint16_t row_index, uint16_t column_index) const;

    std::unique_ptr<Widget> remove_widget(uint16_t row_index, uint16_t column_index);

    void set_widget(uint16_t row_index, uint16_t column_index, std::unique_ptr<Widget>&& w);

    Widget* widget_at_pos(int16_t x, int16_t y) override;

    void get_selection(uint16_t & row_index, uint16_t & column_index) const;

    void set_selection(uint16_t row_index);

    void focus(int reason) override;

    void blur() override;

    void rdp_input_mouse(int device_flags, int mouse_x, int mouse_y, Keymap2 * keymap) override;

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2 * keymap) override;

private:
    void refresh_selected();
    void draw_row(uint16_t row_index, Rect const clip);
};

struct ColumnWidthStrategy
{
    uint32_t min;
    uint32_t weight;
};

void compute_format(
    WidgetGrid const& grid, ColumnWidthStrategy* column_width_strategies, int priority_column_index,
    uint16_t* row_height, uint16_t* column_width, bool* width_is_optimal);

void apply_format(WidgetGrid & grid, uint16_t * row_height, uint16_t * column_width);

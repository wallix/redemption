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
#include "gdi/text_metrics.hpp"


namespace
{
    struct WidgetLabelWithTooltip final : WidgetLabel
    {
        WidgetLabelWithTooltip(
            gdi::GraphicApi & drawable, WidgetTooltipShower & tooltip_shower,
            chars_view text, Color fgcolor, Color bgcolor, Font const & font,
            int xtext, int ytext)
        : WidgetLabel(drawable, text, fgcolor, bgcolor, font, xtext, ytext)
        , tooltip_shower(tooltip_shower)
        {}

        void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
        {
            if (device_flags == MOUSE_FLAG_MOVE && label_width > static_cast<int>(this->cx())) {
                this->tooltip_shower.show_tooltip(this->buffer, x, y, Rect(), this->get_rect());
            }
        }

        WidgetTooltipShower & tooltip_shower;
        int label_width = -1;
    };
} // anonymous namespace


WidgetLabelGrid::WidgetLabelGrid(
    gdi::GraphicApi & drawable,
    WidgetTooltipShower & tooltip_shower,
    WidgetEventNotifier onsubmit,
    uint16_t nb_lines, uint16_t nb_columns,
    Color bg_color_1, Color fg_color_1,
    Color bg_color_2, Color fg_color_2,
    Color bg_color_focus, Color fg_color_focus,
    Color bg_color_selection, Color fg_color_selection,
    Font const & font, uint16_t border
)
    : WidgetGrid(
        drawable, onsubmit, nb_lines, nb_columns,
        bg_color_1, fg_color_1, bg_color_2, fg_color_2,
        bg_color_focus, fg_color_focus,
        bg_color_selection, fg_color_selection, border)
    , font(font)
    , tooltip_shower(tooltip_shower)
{}

WidgetLabelGrid::~WidgetLabelGrid() = default;

void WidgetLabelGrid::add_line(array_view<chars_view> entries)
{
    auto line = this->WidgetGrid::add_line();

    bool const odd = this->get_nb_rows() & 1;
    auto const fg_color = odd ? this->fg_color_1 : this->fg_color_2;
    auto const bg_color = odd ? this->bg_color_1 : this->bg_color_2;
    auto const max_column = std::min(std::size_t(this->get_nb_columns()), entries.size());

    for (std::size_t i = 0; i < max_column; ++i) {
        auto label = std::make_unique<WidgetLabelWithTooltip>(
            this->drawable, tooltip_shower, entries[i],
            fg_color, bg_color, this->font, x_padding_label, y_padding_label
        );

        Dimension dim = label->get_optimal_dim();
        label->set_wh(dim);

        label->label_width = dim.w - x_padding_label * 2;
        line[i] = std::move(label);
    }
}

const char * WidgetLabelGrid::get_cell_text(uint16_t row_index, uint16_t column_index)
{
    const char * result = "";
    Widget * w = this->get_widget(row_index, column_index);
    if (w) {
        WidgetLabel * label = static_cast<WidgetLabel*>(w); /*NOLINT*/
        result = label->get_text();
    }
    return result;
}

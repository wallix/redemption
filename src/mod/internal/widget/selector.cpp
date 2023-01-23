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
 *              Meng Tan, Jennifer Inthavong
 *
 */

#include "mod/internal/widget/selector.hpp"
#include "utils/translation.hpp"
#include "utils/theme.hpp"
#include "utils/sugar/buf_maker.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/text_metrics.hpp"
#include "keyboard/keymap.hpp"

#include <cstring>


WidgetSelector::temporary_number_of_page::temporary_number_of_page(const char * s)
{
    size_t len = std::min(sizeof(this->buffer) - 3, strlen(s));
    this->buffer[0] = '/';
    memcpy(&this->buffer[1], s, len);
    this->buffer[len + 1] = '\0';
}

WidgetSelector::WidgetSelector(
    gdi::GraphicApi & drawable, CopyPaste & copy_paste,
    WidgetTooltipShower & tooltip_shower,
    const char * device_name,
    int16_t left, int16_t top, uint16_t width, uint16_t height,
    Events events,
    const char * current_page,
    const char * number_of_page,
    WidgetButton * extra_button,
    WidgetSelectorParams const & selector_params,
    Font const & font, Theme const & theme, Language lang,
    bool has_target_helpicon /* for unit test only */)
: WidgetParent(drawable, Focusable::Yes)
, tooltip_shower(*this)
, tooltip_shower_parent(tooltip_shower)
, onconnect(events.onconnect)
, oncancel(events.oncancel)
, onctrl_shift(events.onctrl_shift)
, less_than_800(width < 800)
, nb_columns(std::min(selector_params.nb_columns, WidgetSelectorParams::nb_max_columns))
, device_label(drawable, device_name,
               theme.global.fgcolor, theme.global.bgcolor, font)
, header_labels{
    WidgetLabel{
        drawable, selector_params.label[0],
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    },
    WidgetLabel{
        drawable, selector_params.label[1],
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    },
    WidgetLabel{
        drawable, selector_params.label[2],
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    }
}
, column_expansion_buttons{
    WidgetButton{
        drawable, "", [this]{
            this->priority_column_index = 0;
            this->rearrange();
            this->rdp_input_invalidate(this->get_rect());
        },
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 1, font, 6, 2
    },
    WidgetButton{
        drawable, "", [this]{
            this->priority_column_index = 1;
            this->rearrange();
            this->rdp_input_invalidate(this->get_rect());
        },
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 1, font, 6, 2
    },
    WidgetButton{
        drawable, "", [this]{
            this->priority_column_index = 2;
            this->rearrange();
            this->rdp_input_invalidate(this->get_rect());
        },
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 1, font, 6, 2
    }
}
, edit_filters{
    WidgetEdit{
        drawable, copy_paste, nullptr, events.onfilter,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    },
    WidgetEdit{
        drawable, copy_paste, nullptr, events.onfilter,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    },
    WidgetEdit{
        drawable, copy_paste, nullptr, events.onfilter,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    }
}
, selector_lines(drawable, tooltip_shower,
                 [this]{ this->ask_for_connection(); },
                 0, this->nb_columns,
                 theme.selector_line1.bgcolor,
                 theme.selector_line1.fgcolor,
                 theme.selector_line2.bgcolor,
                 theme.selector_line2.fgcolor,
                 theme.selector_focus.bgcolor,
                 theme.selector_focus.fgcolor,
                 theme.selector_selected.bgcolor,
                 theme.selector_selected.fgcolor,
                 font, 2)
//BEGIN WidgetPager
, first_page(drawable, "◀◂", events.onfirst_page,
             theme.global.fgcolor, theme.global.bgcolor,
             theme.global.focus_color, 2, font, 6, 2, true)
, prev_page(drawable, "◀", events.onprev_page,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
, current_page(drawable, copy_paste,
               current_page ? current_page : "XXXX",
               {events.oncurrent_page},
               theme.edit.fgcolor, theme.edit.bgcolor,
               theme.edit.focus_color, font, std::size_t(-1), 1, 1)
, number_page(drawable,
              number_of_page ? temporary_number_of_page(number_of_page).buffer : "/XXX",
              theme.global.fgcolor, theme.global.bgcolor, font)
, next_page(drawable, "▶", events.onnext_page,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
, last_page(drawable, "▸▶", events.onlast_page,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
//END WidgetPager
, logout(drawable, TR(trkeys::logout, lang), events.oncancel,
         theme.global.fgcolor, theme.global.bgcolor,
         theme.global.focus_color, 2, font, 6, 2)
, apply(drawable, TR(trkeys::filter, lang), events.onfilter,
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 2, font, 6, 2)
, connect(drawable, TR(trkeys::connect, lang), events.onconnect,
          theme.global.fgcolor, theme.global.bgcolor,
          theme.global.focus_color, 2, font, 6, 2)
// TODO button without notifier
, target_helpicon(drawable, "?", WidgetEventNotifier(),
                  theme.selector_label.fgcolor, theme.selector_label.bgcolor,
                  theme.global.focus_color, 1, font, 3, 0)
, tr(lang)
, bg_color(theme.global.bgcolor)
, font(font)
, left(left)
, top(top)
, extra_button(extra_button)
{
    this->impl = &composite_array;

    this->add_widget(this->device_label);

    for (int i = 0; i < this->nb_columns; i++) {
        this->weight[i] = selector_params.weight[i];
        this->label[i] = selector_params.label[i];
        this->add_widget(this->header_labels[i]);
        this->add_widget(this->edit_filters[i]);

        this->column_expansion_buttons[i].set_wh(8, 8);
    }

    this->add_widget(this->apply);
    this->add_widget(this->selector_lines, HasFocus::Yes);

    this->add_widget(this->first_page);
    this->add_widget(this->prev_page);
    this->add_widget(this->current_page);
    this->add_widget(this->number_page);
    this->add_widget(this->next_page);
    this->add_widget(this->last_page);
    this->add_widget(this->logout);
    this->add_widget(this->connect);

    if (has_target_helpicon) {
        this->target_helpicon.set_unfocusable();
        this->add_widget(this->target_helpicon);
    }

    if (extra_button) {
        this->add_widget(*extra_button);
    }

    this->move_size_widget(left, top, width, height);
}

void WidgetSelector::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->set_xy(left, top);
    this->set_wh(width, height);

    this->left = left;
    this->top  = top;

    Dimension dim = this->device_label.get_optimal_dim();
    this->device_label.set_wh(dim);
    this->device_label.set_xy(left + TEXT_MARGIN, top + VERTICAL_MARGIN);


    for (int i = 0; i < this->nb_columns; i++) {
        dim = this->header_labels[i].get_optimal_dim();
        this->header_labels[i].set_wh(dim);

        dim = this->edit_filters[i].get_optimal_dim();
        this->edit_filters[i].set_wh(dim);
    }

    dim = this->first_page.get_optimal_dim();
    this->first_page.set_wh(dim);

    dim = this->prev_page.get_optimal_dim();
    this->prev_page.set_wh(dim);


    dim = this->current_page.get_optimal_dim();
    this->current_page.set_wh(this->first_page.cy() + 2, dim.h);

    dim = this->number_page.get_optimal_dim();
    this->number_page.set_wh(dim);

    dim = this->next_page.get_optimal_dim();
    this->next_page.set_wh(dim);

    dim = this->last_page.get_optimal_dim();
    this->last_page.set_wh(dim);


    dim = this->logout.get_optimal_dim();
    this->logout.set_wh(dim);

    dim = this->apply.get_optimal_dim();
    this->apply.set_wh(dim);

    dim = this->connect.get_optimal_dim();
    this->connect.set_wh(dim);

    dim = this->target_helpicon.get_optimal_dim();
    this->target_helpicon.set_wh(dim);


    this->less_than_800 = (this->cx() < 800);

    this->selector_lines.set_wh(width - (this->less_than_800 ? 0 : 30),
        this->selector_lines.cy());
    std::fill(this->current_columns_width, this->current_columns_width + this->selector_lines.get_nb_columns(), 0);

    if (this->extra_button) {
        this->extra_button->set_xy(left + 60, top + height - 60);
    }

    this->rearrange();
}

Widget::Color WidgetSelector::get_bg_color() const
{
    return this->bg_color;
}

constexpr uint16_t COLUMN_EXPANSION_BUTTON_PLACE_HOLDER = 18;

void WidgetSelector::rearrange()
{
    ColumnWidthStrategy column_width_strategies[WidgetSelectorParams::nb_max_columns];
    bool column_width_is_optimal[WidgetSelectorParams::nb_max_columns];

    for (int i = 0; i < this->nb_columns; i++) {
        gdi::TextMetrics tm (this->font, this->header_labels[i].get_text());
        column_width_strategies[i] = { static_cast<uint16_t>(tm.width + 5 + COLUMN_EXPANSION_BUTTON_PLACE_HOLDER), this->weight[i] };

        column_width_is_optimal[i] = false;
    }

    BufMaker<128, uint16_t> rows_height_buffer;
    auto rows_height = rows_height_buffer.dyn_array(this->selector_lines.get_nb_rows());

    compute_format(this->selector_lines, column_width_strategies, this->priority_column_index,
                   rows_height.data(), this->current_columns_width, column_width_is_optimal);
    apply_format(this->selector_lines, rows_height.data(), this->current_columns_width);

    {
        // filter button position
        this->apply.set_xy(this->left + this->cx() - (this->apply.cx() + TEXT_MARGIN),
                            this->top + VERTICAL_MARGIN);
    }

    {
        // labels and filters position
        uint16_t offset = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
        uint16_t labels_y = this->device_label.ebottom() + HORIZONTAL_MARGIN;
        uint16_t filters_y = labels_y + this->header_labels[0].cy()
            + FILTER_SEPARATOR;

        for (std::size_t i = 0; i < this->nb_columns; ++i) {
            this->header_labels[i].set_wh(
                this->current_columns_width[i] + this->selector_lines.border * 2,
                this->header_labels[i].cy());
            this->header_labels[i].set_xy(this->left + offset, labels_y);
            this->edit_filters[i].set_xy(this->header_labels[i].x(), filters_y);
            this->edit_filters[i].set_wh(
                this->header_labels[i].cx() - ((i == this->nb_columns-1) ? 0 : FILTER_SEPARATOR),
                this->edit_filters[i].cy());
            offset += this->header_labels[i].cx();

            bool contains_widget = this->contains_widget(this->column_expansion_buttons[i]);
            if (column_width_is_optimal[i]) {
                if (contains_widget) {
                    this->remove_widget(this->column_expansion_buttons[i]);
                }
            }
            else {
                if (!contains_widget) {
                    this->add_widget(this->column_expansion_buttons[i]);
                }

                this->column_expansion_buttons[i].set_xy(this->left + offset - 15, labels_y + 5);
            }
        }

        WidgetLabel& target_header_label = this->header_labels[IDX_TARGET];

        this->target_helpicon.set_xy(target_header_label.x()
                                     + target_header_label.get_optimal_dim().w + 4,
                                     labels_y);
    }

    {
        // selector list position
        this->selector_lines.set_xy(this->left + (this->less_than_800 ? 0 : HORIZONTAL_MARGIN),
                                    this->edit_filters[0].ebottom() + FILTER_SEPARATOR);
    }
    {
        // Navigation buttons
        uint16_t nav_bottom_y = this->cy() - (this->connect.cy() + VERTICAL_MARGIN);
        this->connect.set_xy(this->connect.x(), this->top + nav_bottom_y);
        this->logout.set_xy(this->logout.x(), this->top + nav_bottom_y);

        uint16_t nav_top_y = this->connect.y() - (this->last_page.cy() + VERTICAL_MARGIN);
        this->last_page.set_xy(this->last_page.x(), nav_top_y);
        this->next_page.set_xy(this->next_page.x(), nav_top_y);
        this->number_page.set_xy(this->number_page.x(),
            nav_top_y + (this->next_page.cy() - this->number_page.cy()) / 2);
        this->current_page.set_xy(this->current_page.x(),
            nav_top_y + (this->next_page.cy() - this->current_page.cy()) / 2);
        this->prev_page.set_xy(this->prev_page.x(), nav_top_y);
        this->first_page.set_xy(this->first_page.x(), nav_top_y);

        uint16_t nav_offset_x = this->cx() - (this->last_page.cx() + TEXT_MARGIN);
        this->last_page.set_xy(this->left + nav_offset_x, this->last_page.y());

        nav_offset_x -= (this->next_page.cx() + NAV_SEPARATOR);
        this->next_page.set_xy(this->left + nav_offset_x, this->next_page.y());

        nav_offset_x -= (this->number_page.cx() + NAV_SEPARATOR);
        this->number_page.set_xy(this->left + nav_offset_x, this->number_page.y());

        nav_offset_x -= this->current_page.cx();
        this->current_page.set_xy(this->left + nav_offset_x, this->current_page.y());

        nav_offset_x -= (this->prev_page.cx() + NAV_SEPARATOR);
        this->prev_page.set_xy(this->left + nav_offset_x, this->prev_page.y());

        nav_offset_x -= (this->first_page.cx() + NAV_SEPARATOR);
        this->first_page.set_xy(this->left + nav_offset_x, this->first_page.y());

        int nav_w = this->last_page.eright() - this->first_page.x();
        this->connect.set_xy(this->last_page.eright() - nav_w/4 - this->connect.cx()/2,
            this->connect.y());
        this->logout.set_xy(this->first_page.x() + nav_w/4 - this->logout.cx()/2,
            this->logout.y());
    }
}

void WidgetSelector::ask_for_connection()
{
    this->onconnect();
}


void WidgetSelector::add_device(array_view<chars_view> entries)
{
    this->selector_lines.add_line(entries);
}

void WidgetSelector::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (keymap.last_kevent()) {
        case Keymap::KEvent::Esc:
            this->oncancel();
            break;

        case Keymap::KEvent::Ctrl:
        case Keymap::KEvent::Shift:
            if (this->extra_button
                && keymap.is_shift_pressed()
                && keymap.is_ctrl_pressed())
            {
                this->onctrl_shift();
            }
            break;

        default:
            WidgetParent::rdp_input_scancode(flags, scancode, event_time, keymap);
            break;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}

void WidgetSelector::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    if (device_flags == MOUSE_FLAG_MOVE && this->target_helpicon.get_rect().contains_pt(x, y)) {
        auto rect = this->get_rect();
        // exclude title bar when remoteapp
        rect.y += 30;
        rect.cy -= 30;
        this->tooltip_shower.show_tooltip(
            this->tr(trkeys::target_accurate_filter_help),
            x, y, rect, this->target_helpicon.get_rect()
        );
    }

    WidgetParent::rdp_input_mouse(device_flags, x, y);
}

void WidgetSelector::TooltipShower::show_tooltip(
    const char * text, int x, int y,
    Rect const preferred_display_rect,
    Rect const mouse_area)
{
    (void)preferred_display_rect;
    this->selector.tooltip_shower_parent.show_tooltip(text, x, y, this->selector.get_rect(), mouse_area);
}

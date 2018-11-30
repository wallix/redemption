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

WidgetSelector::temporary_number_of_page::temporary_number_of_page(const char * s)
{
    size_t len = std::min(sizeof(this->buffer) - 3, strlen(s));
    this->buffer[0] = '/';
    memcpy(&this->buffer[1], s, len);
    this->buffer[len + 1] = '\0';
}

WidgetSelector::WidgetSelector(
    gdi::GraphicApi & drawable,
    const char * device_name,
    int16_t left, int16_t top, uint16_t width, uint16_t height,
    Widget & parent, NotifyApi* notifier,
    const char * current_page,
    const char * number_of_page,
    WidgetFlatButton * extra_button,
    WidgetSelectorParams const & selector_params,
    Font const & font, Theme const & theme, Translation::language_t lang)
: WidgetParent(drawable, parent, notifier)
, less_than_800(width < 800)
, nb_columns(std::min(selector_params.nb_columns, WidgetSelectorParams::nb_max_columns))
, device_label(drawable, *this, nullptr, device_name,
               -10, theme.global.fgcolor, theme.global.bgcolor, font)
, header_labels{
    WidgetLabel{
        drawable, *this, nullptr, selector_params.label[0], -10,
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    },
    WidgetLabel{
        drawable, *this, nullptr, selector_params.label[1], -10,
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    },
    WidgetLabel{
        drawable, *this, nullptr, selector_params.label[2], -10,
        theme.selector_label.fgcolor,
        theme.selector_label.bgcolor, font, 5
    }
}
, edit_filters{
    WidgetEdit{
        drawable, *this, this,
        nullptr, -12,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    },
    WidgetEdit{
        drawable, *this, this,
        nullptr, -12,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    },
    WidgetEdit{
        drawable, *this, this,
        nullptr, -12,
        theme.edit.fgcolor, theme.edit.bgcolor,
        theme.edit.focus_color, font, std::size_t(-1), 1, 1
    }
}
, selector_lines(drawable,
                 *this, this, 0, this->nb_columns,
                 theme.selector_line1.bgcolor,
                 theme.selector_line1.fgcolor,
                 theme.selector_line2.bgcolor,
                 theme.selector_line2.fgcolor,
                 theme.selector_focus.bgcolor,
                 theme.selector_focus.fgcolor,
                 theme.selector_selected.bgcolor,
                 theme.selector_selected.fgcolor,
                 font, 2, -11)
//BEGIN WidgetPager
, first_page(drawable, *this, notifier, "◀◂", -15,
                theme.global.fgcolor, theme.global.bgcolor,
                theme.global.focus_color, 2, font, 6, 2, true)
, prev_page(drawable, *this, notifier, "◀", -15,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
, current_page(drawable, *this, notifier,
                current_page ? current_page : "XXXX", -15,
                theme.edit.fgcolor, theme.edit.bgcolor,
                theme.edit.focus_color, font, std::size_t(-1), 1, 1)
, number_page(drawable, *this, nullptr,
                number_of_page ? temporary_number_of_page(number_of_page).buffer
                : "/XXX", -100, theme.global.fgcolor,
                theme.global.bgcolor, font)
, next_page(drawable, *this, notifier, "▶", -15,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
, last_page(drawable, *this, notifier, "▸▶", -15,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2, true)
//END WidgetPager
, logout(drawable, *this, this, TR(trkeys::logout, lang), -16,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2)
, apply(drawable, *this, this, TR(trkeys::filter, lang), -12,
        theme.global.fgcolor, theme.global.bgcolor,
        theme.global.focus_color, 2, font, 6, 2)
, connect(drawable, *this, this, TR(trkeys::connect, lang), -18,
            theme.global.fgcolor, theme.global.bgcolor,
            theme.global.focus_color, 2, font, 6, 2)
, bg_color(theme.global.bgcolor)
, font(font)
, left(left)
, top(top)
, extra_button(extra_button)
{
    this->impl = &composite_array;

    this->add_widget(&this->device_label);

    for (int i = 0; i < this->nb_columns; i++) {
        this->base_len[i] = selector_params.base_len[i];
        this->add_widget(&this->header_labels[i]);
        this->add_widget(&this->edit_filters[i]);
    }

    this->add_widget(&this->apply);
    this->add_widget(&this->selector_lines);

    this->add_widget(&this->first_page);
    this->add_widget(&this->prev_page);
    this->add_widget(&this->current_page);
    this->add_widget(&this->number_page);
    this->add_widget(&this->next_page);
    this->add_widget(&this->last_page);
    this->add_widget(&this->logout);
    this->add_widget(&this->connect);

    if (extra_button) {
        this->add_widget(extra_button);
    }

    this->move_size_widget(left, top, width, height);
}

WidgetSelector::~WidgetSelector()
{
    this->clear();
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


    this->less_than_800 = (this->cx() < 800);

    this->selector_lines.set_wh(width - (this->less_than_800 ? 0 : 30),
        this->selector_lines.cy());

    if (this->extra_button) {
        this->extra_button->set_xy(left + 60, top + height - 60);
    }

    this->rearrange();
}

BGRColor WidgetSelector::get_bg_color() const
{
    return this->bg_color;
}

void WidgetSelector::rearrange()
{
    ColumnWidthStrategy column_width_strategies[WidgetSelectorParams::nb_max_columns];

    for (int i = 0; i < this->nb_columns; i++) {
        gdi::TextMetrics tm (this->font, this->header_labels[i].get_text());
        column_width_strategies[i] = { static_cast<uint16_t>(tm.width + 5), this->base_len[i]};
    };

    BufMaker<128, uint16_t> rows_height_buffer;
    auto rows_height = rows_height_buffer.dyn_array(this->selector_lines.get_nb_rows());
    uint16_t columns_width[WidgetSelectorParams::nb_max_columns] = { 0 };

    compute_format(this->selector_lines, column_width_strategies,
                   rows_height.data(), columns_width);
    apply_format(this->selector_lines, rows_height.data(), columns_width);


    {
        // filter button position
        this->apply.set_xy(this->left + this->cx() - (this->apply.cx() + TEXT_MARGIN),
                            this->top + VERTICAL_MARGIN);
    }

    {
        // labels and filters position
        uint16_t offset = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
        uint16_t labels_y = this->device_label.bottom() + HORIZONTAL_MARGIN;
        uint16_t filters_y = labels_y + this->header_labels[0].cy()
            + FILTER_SEPARATOR;

        for (int i = 0; i < this->nb_columns; i++) {
            this->header_labels[i].set_wh(
                columns_width[i] + this->selector_lines.border * 2,
                this->header_labels[i].cy());
            this->header_labels[i].set_xy(this->left + offset, labels_y);
            this->edit_filters[i].set_xy(this->header_labels[i].x(), filters_y);
            this->edit_filters[i].set_wh(
                this->header_labels[i].cx() - ((i == this->nb_columns-1) ? 0 : FILTER_SEPARATOR),
                this->edit_filters[i].cy());
            offset += this->header_labels[i].cx();
        }
    }
    {
        // selector list position
        this->selector_lines.set_xy(this->left + (this->less_than_800 ? 0 : HORIZONTAL_MARGIN),
                                    this->edit_filters[0].bottom() + FILTER_SEPARATOR);
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

        int nav_w = this->last_page.right() - this->first_page.x();
        this->connect.set_xy(this->last_page.right() - nav_w/4 - this->connect.cx()/2,
            this->connect.y());
        this->logout.set_xy(this->first_page.x() + nav_w/4 - this->logout.cx()/2,
            this->logout.y());
    }
}

void WidgetSelector::ask_for_connection()
{
    if (this->notifier) {
        this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
    }
}


void WidgetSelector::notify(Widget* widget, notify_event_t event)
{
    if ((widget->group_id == this->selector_lines.group_id) ||
        (widget->group_id == this->connect.group_id)) {
        if (NOTIFY_SUBMIT == event) {
            this->ask_for_connection();
        }
    }
    else if (widget->group_id == this->apply.group_id) {
        if (NOTIFY_SUBMIT == event || NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
    }
    else if (widget->group_id == this->logout.group_id) {
        if (NOTIFY_SUBMIT == event) {
            if (this->notifier) {
                this->notifier->notify(widget, NOTIFY_CANCEL);
            }
        }
    }
    else {
        WidgetParent::notify(widget, event);
    }
}

void WidgetSelector::add_device(array_view<const array_view_const_char> entries)
{
    this->selector_lines.add_line(entries);
}

void WidgetSelector::rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
{
    if (keymap->nb_kevent_available() > 0){
        switch (keymap->top_kevent()){
        case Keymap2::KEVENT_ESC:
            keymap->get_kevent();
            this->send_notify(NOTIFY_CANCEL);
            break;
        default:
            WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
            break;
        }
    }
}

void WidgetSelector::show_tooltip(
    Widget * widget, const char * text, int x, int y,
    Rect const preferred_display_rect)
{
    WidgetParent::show_tooltip(widget, text, x, y,
        (preferred_display_rect.isempty() ? this->get_rect() : preferred_display_rect));
}

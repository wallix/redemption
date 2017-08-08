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

#pragma once

#include "composite.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "number_edit.hpp"
#include "image.hpp"
#include "labelgrid.hpp"
// #include "headergrid.hpp"


#include "flat_button.hpp"
#include "utils/translation.hpp"
#include "utils/theme.hpp"
#include "gdi/graphic_api.hpp"



using FlatSelectorModVariables = vcfg::variables<
    vcfg::var<cfg::globals::auth_user,                  vcfg::accessmode::ask | vcfg::accessmode::set | vcfg::accessmode::get>,
    vcfg::var<cfg::context::selector,                   vcfg::accessmode::ask | vcfg::accessmode::set>,
    vcfg::var<cfg::context::target_protocol,            vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_device,              vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::globals::target_user,                vcfg::accessmode::ask | vcfg::accessmode::get>,
    vcfg::var<cfg::context::password,                   vcfg::accessmode::ask>,
    vcfg::var<cfg::context::selector_current_page,      vcfg::accessmode::is_asked | vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_number_of_pages,   vcfg::accessmode::is_asked | vcfg::accessmode::get>,
    vcfg::var<cfg::context::selector_device_filter,     vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_group_filter,      vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_lines_per_page,    vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector_proto_filter,      vcfg::accessmode::get | vcfg::accessmode::set>,
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::globals::host,                       vcfg::accessmode::get>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::font,                                vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class GridSelector : public WidgetParent
{
public:
    struct GridSelectorParams {
        const char * label_names[GRID_NB_COLUMNS_MAX];
        uint16_t base_len[GRID_NB_COLUMNS_MAX];
        uint16_t nb_columns;
    };

    CompositeArray composite_array;

    bool less_than_800;
    const uint16_t nb_columns;

    WidgetLabel device_label;

    WidgetLabel * header_label[GRID_NB_COLUMNS_MAX];
    WidgetEdit * edit_filter[GRID_NB_COLUMNS_MAX];


    WidgetLabelGrid selector_lines;

    WidgetFlatButton first_page;
    WidgetFlatButton prev_page;

    WidgetNumberEdit current_page;
    WidgetLabel number_page;
    WidgetFlatButton next_page;
    WidgetFlatButton last_page;
    WidgetFlatButton logout;
    WidgetFlatButton apply;
    WidgetFlatButton connect;

    BGRColor bg_color;

    Font const & font;

    int16_t left;
    int16_t top;

public:
    struct temporary_number_of_page {
        char buffer[15];

        explicit temporary_number_of_page(const char * s)
        {
            size_t len = std::min(sizeof(this->buffer) - 3, strlen(s));
            this->buffer[0] = '/';
            memcpy(&this->buffer[1], s, len);
            this->buffer[len + 1] = '\0';
        }
    };

    enum {
        IDX_TARGETGROUP,
        IDX_TARGET,
        IDX_PROTOCOL,
        IDX_CLOSETIME
    };

    enum {
        HORIZONTAL_MARGIN = 15,
        VERTICAL_MARGIN = 10,
        TEXT_MARGIN = 20,
        FILTER_SEPARATOR = 5,
        NAV_SEPARATOR = 15
    };

    WidgetFlatButton * extra_button;

    uint16_t base_len[GRID_NB_COLUMNS_MAX] = {0};



    GridSelector(gdi::GraphicApi & drawable,
                 const char * device_name,
                 int16_t left, int16_t top, uint16_t width, uint16_t height,
                 Widget & parent, NotifyApi* notifier,
                 const char * current_page,
                 const char * number_of_page,
                 WidgetFlatButton * extra_button,
                 uint16_t nb_columns, FlatSelectorModVariables * params,
                 Font const & font, Theme const & theme, Translation::language_t lang)
    : WidgetParent(drawable, parent, notifier)
    , less_than_800(width < 800)
    , nb_columns((nb_columns > 3) ? nb_columns : 3)
    , device_label(drawable, *this, nullptr, device_name,
                    -10, theme.global.fgcolor, theme.global.bgcolor, font)
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
                    theme.edit.focus_color, font, -1, 1, 1)
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

        const char * entries[GRID_NB_COLUMNS_MAX];
        entries[0] = "Authorization";
        entries[1] = "Target";
        entries[2] = "Protocol";
        entries[3] = "empty";
        entries[4] = "empty";
        entries[5] = "empty";
        entries[6] = "empty";
        entries[7] = "empty";
        entries[8] = "empty";
        entries[9] = "empty";

        const uint16_t base_len[GRID_NB_COLUMNS_MAX] = {200, 640, 80, 80, 80, 80, 80, 80, 80, 80};

        for (int i = 0; i < this->nb_columns; i++) {
            this->header_label[i] = new WidgetLabel(drawable, *this, nullptr, entries[i], -10,
                         theme.selector_label.fgcolor,
                         theme.selector_label.bgcolor, font, 5);

            this->edit_filter[i] = new WidgetEdit(drawable, *this, this,
                              nullptr, -12,
                              theme.edit.fgcolor, theme.edit.bgcolor,
                              theme.edit.focus_color, font, -1, 1, 1);

            this->base_len[i] = base_len[i];

            this->add_widget(this->header_label[i]);
            this->add_widget(this->edit_filter[i]);
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

        ~GridSelector() override {
        this->clear();
    }

//     void add_column(const char * column_name, int column_base_len) {
//         this->header_column.add_column(column_name, column_base_len);
//     }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_xy(left, top);
        this->set_wh(width, height);

        this->left = left;
        this->top  = top;

        Dimension dim = this->device_label.get_optimal_dim();
        this->device_label.set_wh(dim);
        this->device_label.set_xy(left + TEXT_MARGIN, top + VERTICAL_MARGIN);


        for (int i = 0; i < this->nb_columns; i++) {
            dim = this->header_label[i]->get_optimal_dim();
            this->header_label[i]->set_wh(dim);

            dim = this->edit_filter[i]->get_optimal_dim();
            this->edit_filter[i]->set_wh(dim);
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

    BGRColor get_bg_color() const override {
        return this->bg_color;
    }


private:
    void rearrange() {

        ColumnWidthStrategy column_width_strategies[GRID_NB_COLUMNS_MAX];

        for (int i = 0; i < this->nb_columns; i++) {
            gdi::TextMetrics tm (this->font, this->header_label[i]->get_text());
            column_width_strategies[i] = { static_cast<uint16_t>(tm.width + 5), this->base_len[i]};
        };

        uint16_t rows_height[GRID_NB_ROWS_MAX]      = { 0 };
        uint16_t columns_width[GRID_NB_COLUMNS_MAX] = { 0 };

        compute_format(this->selector_lines, column_width_strategies,
                       rows_height, columns_width);
        apply_format(this->selector_lines, rows_height, columns_width);


        {
            // filter button position
            this->apply.set_xy(this->left + this->cx() - (this->apply.cx() + TEXT_MARGIN),
                               this->top + VERTICAL_MARGIN);
        }

        {
            // labels and filters position
            uint16_t offset = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
            uint16_t labels_y = this->device_label.bottom() + HORIZONTAL_MARGIN;
            uint16_t filters_y = labels_y + this->header_label[0]->cy()
                + FILTER_SEPARATOR;

            for (int i = 0; i < this->nb_columns; i++) {
                this->header_label[i]->set_wh(
                    columns_width[i] + this->selector_lines.border * 2,
                    this->header_label[i]->cy());
                this->header_label[i]->set_xy(this->left + offset, labels_y);
                this->edit_filter[i]->set_xy(this->header_label[i]->x(), filters_y);
                this->edit_filter[i]->set_wh(
                    this->header_label[i]->cx() - FILTER_SEPARATOR,
                    this->edit_filter[i]->cy());
                offset += this->header_label[i]->cx();
            }

           (void)offset;
        }
        {
            // selector list position
            this->selector_lines.set_xy(this->left + (this->less_than_800 ? 0 : HORIZONTAL_MARGIN),
                                        this->edit_filter[0]->bottom() + FILTER_SEPARATOR);
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

public:
    void ask_for_connection() {
        if (this->notifier) {
            this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
        }
    }


    void notify(Widget* widget, notify_event_t event) override {
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

    void add_device(const char ** entries)
    {
        //for (int i = 0; i < this->nb_columns; i++) {
            this->selector_lines.add_line(entries);
        //}
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
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

    void show_tooltip(Widget * widget, const char * text, int x, int y,
                      Rect const preferred_display_rect, int iter) override {
        WidgetParent::show_tooltip(widget, text, x, y,
            (preferred_display_rect.isempty() ? this->get_rect() : preferred_display_rect),
            iter);
    }

};



class WidgetSelectorFlat : public WidgetParent
{
public:
    CompositeArray composite_array;

    bool less_than_800;

    WidgetLabel device_label;
    WidgetLabel target_group_label;
    WidgetLabel target_label;
    WidgetLabel protocol_label;

    WidgetLabelGrid selector_lines;

    WidgetEdit filter_target_group;
    WidgetEdit filter_target;
    WidgetEdit filter_protocol;
    //BEGIN WidgetPager

    WidgetFlatButton first_page;
    WidgetFlatButton prev_page;

    WidgetNumberEdit current_page;
    WidgetLabel number_page;
    WidgetFlatButton next_page;
    WidgetFlatButton last_page;
    //END WidgetPager
    WidgetFlatButton logout;
    WidgetFlatButton apply;
    WidgetFlatButton connect;

    BGRColor bg_color;

    Font const & font;

    int16_t left;
    int16_t top;

public:
    struct temporary_number_of_page {
        char buffer[15];

        explicit temporary_number_of_page(const char * s)
        {
            size_t len = std::min(sizeof(this->buffer) - 3, strlen(s));
            this->buffer[0] = '/';
            memcpy(&this->buffer[1], s, len);
            this->buffer[len + 1] = '\0';
        }
    };

    enum {
        IDX_TARGETGROUP,
        IDX_TARGET,
        IDX_PROTOCOL,
        IDX_CLOSETIME
    };

    enum {
        HORIZONTAL_MARGIN = 15,
        VERTICAL_MARGIN = 10,
        TEXT_MARGIN = 20,
        FILTER_SEPARATOR = 5,
        NAV_SEPARATOR = 15
    };

    WidgetFlatButton * extra_button;


public:
    WidgetSelectorFlat(gdi::GraphicApi & drawable,
                        const char * device_name,
                        // TODO use Rect
                        int16_t left, int16_t top, uint16_t width, uint16_t height,
                        Widget & parent, NotifyApi* notifier,
                        const char * current_page, const char * number_of_page,
                        const char * filter_target_group, const char * filter_target,
                        const char * filter_protocol,
                        WidgetFlatButton * extra_button,
                        Font const & font, Theme const & theme, Translation::language_t lang)
        : WidgetParent(drawable, parent, notifier)
        , less_than_800(width < 800)
        , device_label(drawable, *this, nullptr, device_name,
                       -10, theme.global.fgcolor, theme.global.bgcolor, font)
        , target_group_label(drawable, *this, nullptr, TR(trkeys::authorization, lang),
                             -10, theme.selector_label.fgcolor,
                             theme.selector_label.bgcolor, font, 5)
        , target_label(drawable, *this, nullptr, TR(trkeys::target, lang), -10,
                       theme.selector_label.fgcolor,
                       theme.selector_label.bgcolor, font, 5)
        , protocol_label(drawable, *this, nullptr, TR(trkeys::protocol, lang), -10,
                         theme.selector_label.fgcolor,
                         theme.selector_label.bgcolor, font, 5)
        , selector_lines(drawable,
                         *this, this, 0, 3,
                         theme.selector_line1.bgcolor,
                         theme.selector_line1.fgcolor,
                         theme.selector_line2.bgcolor,
                         theme.selector_line2.fgcolor,
                         theme.selector_focus.bgcolor,
                         theme.selector_focus.fgcolor,
                         theme.selector_selected.bgcolor,
                         theme.selector_selected.fgcolor,
                         font, 2, -11)
        , filter_target_group(drawable, *this, this,
                              filter_target_group?filter_target_group:nullptr, -12,
                              theme.edit.fgcolor, theme.edit.bgcolor,
                              theme.edit.focus_color, font, -1, 1, 1)
        , filter_target(drawable, *this, this, filter_target?filter_target:nullptr,
                        -12, theme.edit.fgcolor, theme.edit.bgcolor,
                        theme.edit.focus_color, font, -1, 1, 1)
        , filter_protocol(drawable, *this, this,
                          filter_protocol?filter_protocol:nullptr, -12,
                          theme.edit.fgcolor, theme.edit.bgcolor,
                          theme.edit.focus_color, font, -1, 1, 1)
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
                       theme.edit.focus_color, font, -1, 1, 1)
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
        this->add_widget(&this->target_group_label);
        this->add_widget(&this->target_label);
        this->add_widget(&this->protocol_label);
        this->add_widget(&this->filter_target_group);
        this->add_widget(&this->filter_target);
        this->add_widget(&this->filter_protocol);
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

    ~WidgetSelectorFlat() override {
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_xy(left, top);
        this->set_wh(width, height);

        this->left = left;
        this->top  = top;

        Dimension dim = this->device_label.get_optimal_dim();
        this->device_label.set_wh(dim);
        this->device_label.set_xy(left + TEXT_MARGIN, top + VERTICAL_MARGIN);


        dim = this->target_group_label.get_optimal_dim();
        this->target_group_label.set_wh(dim);

        dim = this->target_label.get_optimal_dim();
        this->target_label.set_wh(dim);

        dim = this->protocol_label.get_optimal_dim();
        this->protocol_label.set_wh(dim);


        dim = this->filter_target_group.get_optimal_dim();
        this->filter_target_group.set_wh(dim);

        dim = this->filter_target.get_optimal_dim();
        this->filter_target.set_wh(dim);

        dim = this->filter_protocol.get_optimal_dim();
        this->filter_protocol.set_wh(dim);


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

    BGRColor get_bg_color() const override {
        return this->bg_color;
    }

private:
    void rearrange() {
        gdi::TextMetrics tm1(this->font, this->target_group_label.get_text());
        int target_group_min_width = tm1.width + 5;
        gdi::TextMetrics tm2(this->font, this->target_label.get_text());
        int target_min_width = tm2.width + 5;
        gdi::TextMetrics tm(this->font, this->protocol_label.get_text());
        int protocol_min_width = tm.width + 5;

        ColumnWidthStrategy column_width_strategies[] = {
            { static_cast<uint16_t>(target_group_min_width), 200 },
            { static_cast<uint16_t>(target_min_width), 64000 },
            { static_cast<uint16_t>(protocol_min_width), 80 }
        };

        uint16_t rows_height[GRID_NB_ROWS_MAX]      = { 0 };
        uint16_t columns_width[GRID_NB_COLUMNS_MAX] = { 0 };

        compute_format(this->selector_lines, column_width_strategies,
                       rows_height, columns_width);
        apply_format(this->selector_lines, rows_height, columns_width);

        {
            // filter button position
            this->apply.set_xy(this->left + this->cx() - (this->apply.cx() + TEXT_MARGIN),
                               this->top + VERTICAL_MARGIN);
        }

        {
            // labels and filters position
            uint16_t offset = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
            uint16_t labels_y = this->device_label.bottom() + HORIZONTAL_MARGIN;
            uint16_t filters_y = labels_y + this->target_group_label.cy()
                + FILTER_SEPARATOR;
            // target group
            this->target_group_label.set_wh(
                columns_width[IDX_TARGETGROUP] + this->selector_lines.border * 2,
                this->target_group_label.cy());
            this->target_group_label.set_xy(this->left + offset, labels_y);
            this->filter_target_group.set_xy(this->target_group_label.x(), filters_y);
            this->filter_target_group.set_wh(
                this->target_group_label.cx() - FILTER_SEPARATOR,
                this->filter_target_group.cy());
            offset += this->target_group_label.cx();

            // target
            this->target_label.set_wh(
                columns_width[IDX_TARGET] + this->selector_lines.border * 2,
                this->target_label.cy());
            this->target_label.set_xy(this->left + offset, labels_y);
            this->filter_target.set_xy(this->target_label.x(), filters_y);
            this->filter_target.set_wh(
                this->target_label.cx() - FILTER_SEPARATOR,
                this->filter_target.cy());
            offset += this->target_label.cx();

            // protocol
            this->protocol_label.set_wh(
                columns_width[IDX_PROTOCOL] + this->selector_lines.border * 2,
                this->protocol_label.cy());
            this->protocol_label.set_xy(this->left + offset, labels_y);
            this->filter_protocol.set_xy(this->protocol_label.x(), filters_y);
            this->filter_protocol.set_wh(
                this->protocol_label.cx(),
                this->filter_protocol.cy());
            offset += this->protocol_label.cx();

            (void)offset;
        }
        {
            // selector list position
            this->selector_lines.set_xy(this->left + (this->less_than_800 ? 0 : HORIZONTAL_MARGIN),
                                        this->filter_target_group.bottom() + FILTER_SEPARATOR);
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

public:
    void ask_for_connection() {
        if (this->notifier) {
            this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
        }
    }


    void notify(Widget* widget, notify_event_t event) override {
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

    void add_device(const char * device_group, const char * target_label,
                    const char * protocol)
    {
        const char * texts[] = { device_group, target_label, protocol };
        this->selector_lines.add_line(texts);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
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

    void show_tooltip(Widget * widget, const char * text, int x, int y,
                      Rect const preferred_display_rect, int iter) override {
        WidgetParent::show_tooltip(widget, text, x, y,
            (preferred_display_rect.isempty() ? this->get_rect() : preferred_display_rect),
            iter);
    }
};

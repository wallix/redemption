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
 *              Meng Tan
 *
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_SELECTOR2_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_SELECTOR2_HPP

#include "composite.hpp"
#include "multiline.hpp"
#include "label.hpp"
#include "edit.hpp"
#include "number_edit.hpp"
#include "image.hpp"
#include "region.hpp"
#include "difftimeval.hpp"
#include "labelgrid.hpp"


#include "flat_button.hpp"
#include "translation.hpp"
#include "theme.hpp"

class WidgetSelectorFlat2 : public WidgetParent
{
public:

    int bg_color;
    const bool less_than_800;
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

    CompositeArray composite_array;

public:
    struct temporary_number_of_page {
        char buffer[15];

        temporary_number_of_page(const char * s)
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
public:
    WidgetSelectorFlat2(DrawApi& drawable, const char * device_name, uint16_t width,
                        uint16_t height, Widget2 & parent, NotifyApi* notifier,
                        const char * current_page, const char * number_of_page,
                        const char * filter_target_group, const char * filter_target,
                        const char * filter_protocol, Inifile & ini)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , bg_color(ini.theme.global.bgcolor)
        , less_than_800(this->rect.cx < 800)
        , device_label(drawable, TEXT_MARGIN, VERTICAL_MARGIN, *this, NULL, device_name,
                       true, -10, ini.theme.global.fgcolor, ini.theme.global.bgcolor)
        , target_group_label(drawable, 0, 0, *this, NULL, TR("target_group", ini), true,
                              -10, ini.theme.selector_label.fgcolor,
                             ini.theme.selector_label.bgcolor, 5)
        , target_label(drawable, 0, 0, *this, NULL, TR("target", ini), true, -10,
                       ini.theme.selector_label.fgcolor,
                       ini.theme.selector_label.bgcolor, 5)
        , protocol_label(drawable, 0, 0, *this, NULL, TR("protocol", ini), true, -10,
                         ini.theme.selector_label.fgcolor,
                         ini.theme.selector_label.bgcolor, 5)
        , selector_lines(drawable, Rect(0, 0, width - (this->less_than_800 ? 0 : 30), 1),
                         *this, this, 0, 3,
                         ini.theme.selector_line1.bgcolor,
                         ini.theme.selector_line1.fgcolor,
                         ini.theme.selector_line2.bgcolor,
                         ini.theme.selector_line2.fgcolor,
                         ini.theme.selector_focus.bgcolor,
                         ini.theme.selector_focus.fgcolor,
                         ini.theme.selector_selected.bgcolor,
                         ini.theme.selector_selected.fgcolor,
                         2, -11)
        , filter_target_group(drawable, 0, 0, 120, *this, this,
                              filter_target_group?filter_target_group:0, -12,
                              ini.theme.edit.fgcolor, ini.theme.edit.bgcolor,
                              ini.theme.edit.focus_color, -1, 1, 1)
        , filter_target(drawable, 0, 0, 340, *this, this, filter_target?filter_target:0,
                        -12, ini.theme.edit.fgcolor, ini.theme.edit.bgcolor,
                        ini.theme.edit.focus_color, -1, 1, 1)
        , filter_protocol(drawable, 0, 0, 110, *this, this,
                          filter_protocol?filter_protocol:0, -12,
                          ini.theme.edit.fgcolor, ini.theme.edit.bgcolor,
                          ini.theme.edit.focus_color, -1, 1, 1)
          //BEGIN WidgetPager
        , first_page(drawable, 0, 0, *this, notifier, "◀◂", true, -15,
                     ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                     ini.theme.global.focus_color, 6, 2, true)
        , prev_page(drawable, 0, 0, *this, notifier, "◀", true, -15,
                    ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                    ini.theme.global.focus_color, 6, 2, true)
        , current_page(drawable, 0, 0, this->first_page.cy(), *this, notifier,
                       current_page ? current_page : "XXXX", -15,
                       ini.theme.edit.fgcolor, ini.theme.edit.bgcolor,
                       ini.theme.edit.focus_color, -1, 1, 1)
        , number_page(drawable, 0, 0, *this, NULL,
                      number_of_page ? temporary_number_of_page(number_of_page).buffer
                      : "/XXX", true, -100, ini.theme.global.fgcolor,
                      ini.theme.global.bgcolor)
        , next_page(drawable, 0, 0, *this, notifier, "▶", true, -15,
                    ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                    ini.theme.global.focus_color, 6, 2, true)
        , last_page(drawable, 0, 0, *this, notifier, "▸▶", true, -15,
                    ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                    ini.theme.global.focus_color, 6, 2, true)
          //END WidgetPager
        , logout(drawable, 0, 0, *this, this, TR("logout", ini), true, -16,
                 ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                 ini.theme.global.focus_color, 6, 2)
        , apply(drawable, 0, 0, *this, this, TR("filter", ini), true, -12,
                ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                ini.theme.global.focus_color, 6, 2)
        , connect(drawable, 0, 0, *this, this, TR("connect", ini), true, -18,
                  ini.theme.global.fgcolor, ini.theme.global.bgcolor,
                  ini.theme.global.focus_color, 6, 2)
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

        this->rearrange();
    }

    virtual ~WidgetSelectorFlat2()
    {
        this->clear();
    }

    virtual int get_bg_color() const {
        return this->bg_color;
    }

    void rearrange() {
        int target_group_min_width = 0;
        int target_min_width = 0;
        int protocol_min_width = 0;
        int h = 0;
        this->drawable.text_metrics(this->target_group_label.get_text(), target_group_min_width, h);
        this->drawable.text_metrics(this->target_label.get_text(), target_min_width, h);
        this->drawable.text_metrics(this->protocol_label.get_text(), protocol_min_width, h);
        target_group_min_width += 5;
        target_min_width += 5;
        protocol_min_width += 5;

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
            this->apply.set_button_y(VERTICAL_MARGIN);
            this->apply.set_button_x(this->cx() - (this->apply.cx() + TEXT_MARGIN));
        }

        {
            // labels and filters position
            uint16_t offset = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
            uint16_t labels_y = this->device_label.ly() + HORIZONTAL_MARGIN;
            uint16_t filters_y = labels_y + this->target_group_label.cy()
                + FILTER_SEPARATOR;
            // target group
            this->target_group_label.rect.cx = columns_width[IDX_TARGETGROUP] +
                this->selector_lines.border * 2;
            this->target_group_label.rect.x = offset;
            this->target_group_label.rect.y = labels_y;
            this->filter_target_group.set_edit_x(this->target_group_label.dx());
            this->filter_target_group.set_edit_cx(this->target_group_label.cx() -
                                                  FILTER_SEPARATOR);
            this->filter_target_group.set_edit_y(filters_y);
            offset += this->target_group_label.rect.cx;

            // target
            this->target_label.rect.cx = columns_width[IDX_TARGET] +
                this->selector_lines.border * 2;
            this->target_label.rect.x = offset;
            this->target_label.rect.y = labels_y;
            this->filter_target.set_edit_x(this->target_label.dx());
            this->filter_target.set_edit_cx(this->target_label.cx() - FILTER_SEPARATOR);
            this->filter_target.set_edit_y(filters_y);
            offset += this->target_label.rect.cx;

            // protocol
            this->protocol_label.rect.cx = columns_width[IDX_PROTOCOL] +
                this->selector_lines.border * 2;
            this->protocol_label.rect.x = offset;
            this->protocol_label.rect.y = labels_y;
            this->filter_protocol.set_edit_x(this->protocol_label.dx());
            this->filter_protocol.set_edit_cx(this->protocol_label.cx());
            this->filter_protocol.set_edit_y(filters_y);
            offset += this->protocol_label.rect.cx;
        }
        {
            // selector list position
            this->selector_lines.rect.x = this->less_than_800 ? 0 : HORIZONTAL_MARGIN;
            this->selector_lines.rect.y = this->filter_target_group.ly() + FILTER_SEPARATOR;
        }
        {
            // Navigation buttons
            uint16_t nav_bottom_y = this->cy() - (this->connect.cy() + VERTICAL_MARGIN);
            this->connect.set_button_y(nav_bottom_y);
            this->logout.set_button_y(nav_bottom_y);

            uint16_t nav_top_y = this->connect.dy() - (this->last_page.cy() + VERTICAL_MARGIN);
            this->last_page.set_button_y(nav_top_y);
            this->next_page.set_button_y(nav_top_y);
            this->number_page.rect.y = nav_top_y + (this->next_page.cy() - this->number_page.cy()) / 2;
            this->current_page.set_edit_y(nav_top_y + (this->next_page.cy() - this->current_page.cy()) / 2);
            this->prev_page.set_button_y(nav_top_y);
            this->first_page.set_button_y(nav_top_y);

            uint16_t nav_offset_x = this->cx() - (this->last_page.cx() + TEXT_MARGIN);
            this->last_page.set_button_x(nav_offset_x);

            nav_offset_x -= (this->next_page.cx() + NAV_SEPARATOR);
            this->next_page.set_button_x(nav_offset_x);

            nav_offset_x -= (this->number_page.cx() + NAV_SEPARATOR);
            this->number_page.rect.x = nav_offset_x;

            nav_offset_x -= this->current_page.cx();
            this->current_page.set_edit_x(nav_offset_x);

            nav_offset_x -= (this->prev_page.cx() + NAV_SEPARATOR);
            this->prev_page.set_button_x(nav_offset_x);

            nav_offset_x -= (this->first_page.cx() + NAV_SEPARATOR);
            this->first_page.set_button_x(nav_offset_x);

            int nav_w = this->last_page.lx() - this->first_page.dx();
            this->connect.set_button_x(this->last_page.lx() - nav_w/4 - this->connect.cx()/2);
            this->logout.set_button_x(this->first_page.dx() + nav_w/4 - this->logout.cx()/2);
        }
    }

    void ask_for_connection() {
        if (this->notifier) {
            this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
        }
    }


    virtual void notify(Widget2* widget, notify_event_t event)
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

    void add_device(const char * device_group, const char * target_label,
                    const char * protocol)
    {
        const char * texts[] = { device_group, target_label, protocol };
        this->selector_lines.add_line(texts);
    }
};

#endif

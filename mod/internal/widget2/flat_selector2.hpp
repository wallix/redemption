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
// #include "radio_list.hpp"

enum {
    NONE = 0x00,
    TICKET_VISIBLE = 0x01,
    TICKET_MANDATORY = 0x03,
    COMMENT_VISIBLE = 0x04,
    COMMENT_MANDATORY = 0x0c
};

class WidgetSelectorFlat2 : public WidgetParent
{
public:

    Theme & theme;
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
    //WidgetPager pager;


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
        IDX_CLOSETIME,
    };

    enum {
        SELECTOR_MARGIN = 15,
        SEPARATOR_SPACE = 5,
    };
public:
    WidgetSelectorFlat2(DrawApi& drawable, const char * device_name, uint16_t width,
                        uint16_t height, Widget2 & parent, NotifyApi* notifier,
                        const char * current_page, const char * number_of_page,
                        const char * filter_target_group, const char * filter_target,
                        const char * filter_protocol, Inifile & ini, int ticcom_flag = 0)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , theme(ini.theme)
        , less_than_800(this->rect.cx < 800)
        , device_label(drawable, 20, 10, *this, NULL, device_name, true, -10,
                       this->theme.global.fgcolor, this->theme.global.bgcolor)
        , target_group_label(drawable, 0, 0, *this, NULL, TR("target_group", ini), true,
                              -10, this->theme.selector_label.fgcolor,
                             this->theme.selector_label.bgcolor, 5)
        , target_label(drawable, 0, 0, *this, NULL, TR("target", ini), true, -10,
                       this->theme.selector_label.fgcolor,
                       this->theme.selector_label.bgcolor, 5)
        , protocol_label(drawable, 0, 0, *this, NULL, TR("protocol", ini), true, -10,
                         this->theme.selector_label.fgcolor,
                         this->theme.selector_label.bgcolor, 5)
        , selector_lines(drawable, Rect(0, 0, width - (this->less_than_800 ? 0 : 30), 1),
                         *this, this, 0, 3,
                         this->theme.selector_line1.bgcolor,
                         this->theme.selector_line1.fgcolor,
                         this->theme.selector_line2.bgcolor,
                         this->theme.selector_line2.fgcolor,
                         this->theme.selector_focus.bgcolor,
                         this->theme.selector_focus.fgcolor,
                         this->theme.selector_selected.bgcolor,
                         this->theme.selector_selected.fgcolor,
                         2, -11)
        , filter_target_group(drawable, 0, 0, 120, *this, this, filter_target_group?filter_target_group:0,
                        -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                        this->theme.edit.focus_color, -1, 1, 1)
        , filter_target(drawable, 145, 0, 340, *this, this, filter_target?filter_target:0,
                        -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                        this->theme.edit.focus_color, -1, 1, 1)
        , filter_protocol(drawable, 495, 0, 110, *this, this, filter_protocol?filter_protocol:0,
                       -12, this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                       this->theme.edit.focus_color, -1, 1, 1)
          //BEGIN WidgetPager
        , first_page(drawable, 0, 0, *this, notifier, "◀◂", true, -15,
                     this->theme.global.fgcolor, this->theme.global.bgcolor,
                     this->theme.global.focus_color, 6, 2, true)
        , prev_page(drawable, 0, 0, *this, notifier, "◀", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
        , current_page(drawable, 0, 0, this->first_page.cy(), *this, notifier,
                       current_page ? current_page : "XXXX", -15,
                       this->theme.edit.fgcolor, this->theme.edit.bgcolor,
                       this->theme.edit.focus_color, -1, 1, 1)
        , number_page(drawable, 0, 0, *this, NULL,
                      number_of_page ? temporary_number_of_page(number_of_page).buffer
                      : "/XXX", true, -100, this->theme.global.fgcolor,
                      this->theme.global.bgcolor)
        , next_page(drawable, 0, 0, *this, notifier, "▶", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
        , last_page(drawable, 0, 0, *this, notifier, "▸▶", true, -15,
                    this->theme.global.fgcolor, this->theme.global.bgcolor,
                    this->theme.global.focus_color, 6, 2, true)
          //END WidgetPager
        , logout(drawable, 0, 0, *this, this, TR("logout", ini), true, -16,
                 this->theme.global.fgcolor, this->theme.global.bgcolor,
                 this->theme.global.focus_color, 6, 2)
        , apply(drawable, 0, 0, *this, this, TR("filter", ini), true, -12,
                this->theme.global.fgcolor, this->theme.global.bgcolor,
                this->theme.global.focus_color, 6, 2)
        , connect(drawable, 0, 0, *this, this, TR("connect", ini), true, -18,
                  this->theme.global.fgcolor, this->theme.global.bgcolor,
                  this->theme.global.focus_color, 6, 2)
    {
        this->impl = new CompositeTable;

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


        // this->add_widget(&this->radiolist);

        // radiolist.add_elem("Alphabetical");
        // radiolist.add_elem("Last Used First");
        // radiolist.add_elem("Most Used First");
        // radiolist.select(0);

        this->rearrange();

    }


    virtual ~WidgetSelectorFlat2()
    {
        this->clear();
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
            this->apply.set_button_y(this->device_label.dy());
            this->apply.set_button_x(this->cx() - (this->apply.cx() + 20));
        }

        uint16_t offset = this->less_than_800 ? 0 : SELECTOR_MARGIN;
        {
            // labels and filters position
            this->target_group_label.rect.cx = columns_width[IDX_TARGETGROUP] +
                this->selector_lines.border * 2;
            this->target_group_label.rect.x = offset;
            this->target_group_label.rect.y = this->device_label.ly() + 15;
            this->filter_target_group.set_edit_x(this->target_group_label.dx());
            this->filter_target_group.set_edit_cx(this->target_group_label.cx() -
                                                  SEPARATOR_SPACE);
            this->filter_target_group.set_edit_y(this->target_group_label.ly() +
                                                 SEPARATOR_SPACE);
            offset += this->target_group_label.rect.cx;


            this->target_label.rect.cx = columns_width[IDX_TARGET] +
                this->selector_lines.border * 2;
            this->target_label.rect.x = offset;
            this->target_label.rect.y = this->target_group_label.rect.y;
            this->filter_target.set_edit_x(this->target_label.dx());
            this->filter_target.set_edit_cx(this->target_label.cx() - SEPARATOR_SPACE);
            this->filter_target.set_edit_y(this->target_label.ly() + SEPARATOR_SPACE);
            offset += this->target_label.rect.cx;


            this->protocol_label.rect.cx = columns_width[IDX_PROTOCOL] +
                this->selector_lines.border * 2;
            this->protocol_label.rect.x = offset;
            this->protocol_label.rect.y = this->target_group_label.rect.y;
            this->filter_protocol.set_edit_x(this->protocol_label.dx());
            this->filter_protocol.set_edit_cx(this->protocol_label.cx() -
                                              SEPARATOR_SPACE);
            this->filter_protocol.set_edit_y(this->protocol_label.ly() +
                                             SEPARATOR_SPACE);
            offset += this->protocol_label.rect.cx;
        }
        {
            // selector list position
            this->selector_lines.rect.x = this->less_than_800 ? 0 : SELECTOR_MARGIN;
            this->selector_lines.rect.y = this->filter_target_group.ly() +
                SEPARATOR_SPACE;
        }


        {
            // Navigation buttons
            this->connect.set_button_y(this->cy() - (this->connect.cy() + 10));
            this->logout.set_button_y(this->connect.dy());

            this->last_page.set_button_y(this->connect.dy() - (this->last_page.cy() + 10));
            this->next_page.set_button_y(this->last_page.dy());
            this->number_page.rect.y = this->last_page.dy() +
                (this->next_page.cy() - this->number_page.cy()) / 2;
            this->current_page.set_edit_y(this->last_page.dy() +
                                          (this->next_page.cy() - this->current_page.cy()) / 2);
            this->prev_page.set_button_y(this->last_page.dy());
            this->first_page.set_button_y(this->last_page.dy());


            this->last_page.set_button_x(this->cx() - (this->last_page.cx() + 20));
            this->next_page.set_button_x(this->last_page.dx() - (this->next_page.cx() + 15));
            this->number_page.rect.x = this->next_page.dx() - (this->number_page.cx() + 15);
            this->current_page.set_edit_x(this->number_page.dx() - (this->current_page.cx()));
            this->prev_page.set_button_x(this->current_page.dx() - (this->prev_page.cx() + 15));
            this->first_page.set_button_x(this->prev_page.dx() - (this->first_page.cx() + 15));

            int nav_w = this->last_page.lx() - this->first_page.dx();

            this->connect.set_button_x(this->last_page.lx() - nav_w/4 - this->connect.cx()/2);
            this->logout.set_button_x(this->first_page.dx() + nav_w/4 - this->logout.cx()/2);
        }
    }


    virtual void draw(const Rect& clip)
    {
        this->impl->draw(clip);
        this->draw_inner_free(clip.intersect(this->rect), this->theme.global.bgcolor);
    }

    void ask_for_connection() {
        if (this->notifier) {
            this->notifier->notify(&this->connect, NOTIFY_SUBMIT);
        }
    }


    virtual void notify(Widget2* widget, notify_event_t event)
    {
        if (widget->group_id == this->selector_lines.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->ask_for_connection();
            }
        }
        else if (widget->group_id == this->apply.group_id) {
            if (NOTIFY_SUBMIT == event) {
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
        else if (widget->group_id == this->connect.group_id) {
            if (NOTIFY_SUBMIT == event) {
                this->ask_for_connection();
            }
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        // if (device_flags == MOUSE_FLAG_MOVE) {
        //     if (this->selector_lines.col != COLUMN_UNKNOWN) {
        //         Widget2 * w = this->widget_at_pos(x, y);
        //         if (w != &this->selector_lines) {
        //             this->selector_lines.col = COLUMN_UNKNOWN;
        //         }
        //     }
        // }
        WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
    }

    void add_device(const char * device_group, const char * target_label,
                    const char * protocol, const char * close_time)
    {
        const char * texts[] = { device_group, target_label, protocol };
        this->selector_lines.add_line(texts);
    }


    virtual void draw_inner_free(const Rect& clip, int bg_color) {
        Region region;
        region.rects.push_back(clip);

        this->impl->draw_inner_free(clip, bg_color, region);

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }

};

#endif

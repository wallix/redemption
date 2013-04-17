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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_HPP

#include "widget_composite.hpp"
#include "multiline.hpp"
#include "button.hpp"
#include "label.hpp"
#include "edit.hpp"
#include <modcontext.hpp>
#include <region.hpp>
//#include "pager.hpp"
//#include "selectline.hpp"

class WidgetSelectLine : public Widget
{
public:
    uint current_index;
    int current_bg_color;
    int bg_color;
    int bg_color2;
    int current_fg_color;
    int fg_color;
    int fg_color2;
    int h_text;
    std::vector<WidgetLabel*> labels;

    WidgetSelectLine(ModApi* drawable, const Rect& rect,
                     Widget* parent, NotifyApi* notifier, int id = 0,
                     int bgcolor = WABGREEN, int bgcolor2 = GREEN,
                     int current_bgcolor = DARK_GREEN, int fgcolor = BLACK,
                     int fgcolor2 = BLACK, int current_fgcolor = BLACK)
    : Widget(drawable, rect, parent, notifier, id)
    , current_index(-1u)
    , current_bg_color(current_bgcolor)
    , bg_color(bgcolor)
    , bg_color2(bgcolor2)
    , current_fg_color(current_fgcolor)
    , fg_color(fgcolor)
    , fg_color2(fgcolor2)
    , h_text(0)
    , labels()
    {
        if (this->drawable) {
            int w;
            this->drawable->text_metrics("Lp", w, this->h_text);
        }
    }

    virtual ~WidgetSelectLine()
    {
        for (size_t i = 0; i < this->labels.size(); ++i) {
            delete this->labels[i];
        }
    }

    virtual void notify(Widget* widget, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (event == CLIC_BUTTON1_DOWN) {
            int y = ((int)param2 - this->dy()) / widget->cy();
            if (y >= 0 && (uint)y != this->current_index) {
                int old_current_index = this->current_index;
                this->set_current_index(y);
                this->send_notify(NOTIFY_SELECTION_CHANGED,
                                  old_current_index, this->current_index);
            }
        } else {
            Widget::notify(widget, event, param, param2);
        }
    }

    void add_line(const char * line)
    {
        bool b = this->labels.size() & 1;
        WidgetLabel * label = new WidgetLabel(
            this->drawable,
            0, this->labels.size() * this->h_text,
            this, NULL, line, false, 0,
            b ? this->bg_color : this->bg_color2,
            b ? this->fg_color : this->fg_color2,
            1, 1
        );
        label->rect.cx = this->cx();
        label->rect.cy = this->h_text;
        this->labels.push_back(label);
    }

    void clear()
    {
        for (size_t i = 0; i < this->labels.size(); ++i) {
            delete this->labels[i];
        }
        this->labels.clear();
    }

    void set_current_index(uint idx)
    {
        if (this->current_index < this->labels.size()) {
            this->labels[this->current_index]->bg_color = this->current_index & 1
            ? this->bg_color : this->bg_color2;
            this->labels[this->current_index]->fg_color = this->current_index & 1
            ? this->fg_color : this->fg_color2;
        }
        this->current_index = idx;
        this->labels[idx]->bg_color = this->current_bg_color;
        this->labels[idx]->fg_color = this->current_fg_color;
    }

    virtual void draw(const Rect& clip)
    {
        Rect new_clip = clip.intersect(this->rect);
        std::size_t size = this->labels.size();

        for (std::size_t i = 0; i < size; ++i) {
            Widget *w = this->labels[i];
            Rect rect = new_clip.intersect(w->rect);

            if (!rect.isempty()) {
                w->refresh(rect);
            }
        }
    }
};

class WidgetSelector : public WidgetComposite
{
public:
    WidgetLabel device_label;
    WidgetLabel device_group_label;
    WidgetLabel account_device_label;
    WidgetLabel protocol_label;
    WidgetLabel close_time_label;
    WidgetSelectLine device_group_lines;
    WidgetSelectLine account_device_lines;
    WidgetSelectLine protocol_lines;
    WidgetSelectLine close_time_lines;
    WidgetEdit filter_device_group;
    WidgetEdit filter_account_device;
    WidgetButton logout;
    WidgetButton apply;
    WidgetButton connect;
    //WidgetPager pager;

    ModContext & context;

public:
    WidgetSelector(ModContext & context, ModApi* drawable,
                   uint16_t width, uint16_t height, NotifyApi* notifier)
    : WidgetComposite(drawable, Rect(0,0,width,height), NULL, notifier)
    , device_label(drawable, 20, 10, this, NULL, "???", true, -10, GREY, BLACK)
    , device_group_label(drawable, 20, 0, this, NULL, "Device Group", true, -11, GREY, BLACK)
    , account_device_label(drawable, 150, 0, this, NULL, "Account Device", true, -12, GREY, BLACK)
    , protocol_label(drawable, 500, 0, this, NULL, "Protocol", true, -13, GREY, BLACK)
    , close_time_label(drawable, 620, 0, this, NULL, "Close Time", true, -14, GREY, BLACK)
    , device_group_lines(drawable, Rect(20,0,120,1), this, NULL, -15,
                         WABGREEN, GREEN, DARK_GREEN, BLACK, BLACK, BLACK)
    , account_device_lines(drawable, Rect(150,0,340,1), this, NULL, -16,
                           WABGREEN, GREEN, DARK_GREEN, BLACK, BLACK, BLACK)
    , protocol_lines(drawable, Rect(500,0,110,1), this, NULL, -17,
                     WABGREEN, GREEN, DARK_GREEN, BLACK, BLACK, BLACK)
    , close_time_lines(drawable, Rect(620,0,150,1), this, NULL, -18,
                       WABGREEN, GREEN, DARK_GREEN, BLACK, BLACK, BLACK)
    , filter_device_group(drawable, 20, 0, 120, this, this, NULL, -19, WHITE, BLACK, -1, 1, 1)
    , filter_account_device(drawable, 150, 0, 340, this, this, NULL, -20, WHITE, BLACK, -1, 1, 1)
    , logout(drawable, 0, 0, this, this, "Logout", true, -21, WHITE, BLACK, 8, 4)
    , apply(drawable, 0, 0, this, this, "Appy", true, -22, WHITE, BLACK, 8, 4)
    , connect(drawable, 0, 0, this, this, "Connect", true, -23, WHITE, BLACK, 8, 4)
    , context(context)
    {
        this->child_list.push_back(&this->device_group_label);
        this->child_list.push_back(&this->account_device_label);
        this->child_list.push_back(&this->protocol_label);
        this->child_list.push_back(&this->close_time_label);
        this->child_list.push_back(&this->device_group_lines);
        this->child_list.push_back(&this->account_device_lines);
        this->child_list.push_back(&this->protocol_lines);
        this->child_list.push_back(&this->close_time_lines);
        this->child_list.push_back(&this->filter_device_group);
        this->child_list.push_back(&this->filter_account_device);
        this->child_list.push_back(&this->logout);
        this->child_list.push_back(&this->apply);
        this->child_list.push_back(&this->connect);
        //this->child_list.push_back(&this->pager);

        this->device_group_label.rect.y = this->device_label.cy() + this->device_label.dy() + 5;
        this->account_device_label.rect.y = this->device_group_label.dy();
        this->protocol_label.rect.y = this->device_group_label.dy();
        this->close_time_label.rect.y = this->device_group_label.dy();
        this->filter_device_group.set_edit_y(this->device_group_label.dy() + this->device_group_label.cy() + 5);
        this->filter_account_device.set_edit_y(this->filter_device_group.dy());
        this->device_group_lines.rect.y = this->filter_device_group.dy() + this->filter_device_group.cy() + 5;
        this->account_device_lines.rect.y = this->device_group_lines.dy();
        this->protocol_lines.rect.y = this->device_group_lines.dy();
        this->close_time_lines.rect.y = this->device_group_lines.dy();

        this->device_group_lines.rect.x = this->device_group_label.dx();
        this->account_device_lines.rect.x = this->account_device_label.dx();
        this->protocol_lines.rect.x = this->protocol_label.dx();
        this->close_time_lines.rect.x = this->close_time_label.dx();

        this->connect.set_button_y(this->cy() - (this->logout.cy() + 10));
        this->apply.set_button_y(this->connect.dy());
        this->logout.set_button_y(this->connect.dy());

        this->connect.set_button_x(this->cx() - (this->connect.cx() + 20));
        this->apply.set_button_x(this->connect.dx() - (this->apply.cx() + 15));
        this->logout.set_button_x(this->apply.dx() - (this->logout.cx() + 15));

        this->device_group_lines.rect.cy = this->apply.dy() - (this->device_group_lines.dy() + 10);
        this->account_device_lines.rect.cy = this->device_group_lines.cy();
        this->protocol_lines.rect.cy = this->device_group_lines.cy();
        this->close_time_lines.rect.cy = this->device_group_lines.cy();
    }

    virtual ~WidgetSelector()
    {}

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        Rect new_clip = clip.intersect(this->rect);
        Region region;
        region.rects.push_back(new_clip);

        for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
            Rect rect = new_clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable->draw(RDPOpaqueRect(region.rects[i], GREY),
                                 region.rects[i]);
        }
    }

    virtual void notify(Widget* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (widget == &this->device_group_lines
         || widget == &this->account_device_lines
         || widget == &this->protocol_lines
         || widget == &this->close_time_lines
        ) {
            if (NOTIFY_SUBMIT == event) {
                this->send_notify(NOTIFY_SUBMIT);
            } else {
                this->set_index_list(static_cast<WidgetSelectLine*>(widget)->current_index);
            }
        }
        else if (widget == &this->apply
         || (NOTIFY_SUBMIT == event
          && (widget == &this->filter_account_device
           || widget == &this->filter_device_group
        ))) {
            this->send_notify(NOTIFY_SUBMIT, 2);
        }
        else if (widget == &this->connect) {
            this->send_notify(NOTIFY_SUBMIT);
        }
    }

    void set_index_list(int idx)
    {
        this->device_group_lines.set_current_index(idx);
        this->account_device_lines.set_current_index(idx);
        this->protocol_lines.set_current_index(idx);
        this->close_time_lines.set_current_index(idx);
    }

    void add_device(const char * device_group, const char * account_device,
                    const char * protocol, const char * close_time)
    {
        this->device_group_lines.add_line(device_group);
        this->account_device_lines.add_line(account_device);
        this->protocol_lines.add_line(protocol);
        this->close_time_lines.add_line(close_time);
    }

    void refresh_context()
    {
        char * groups = this->context.get(STRAUTHID_TARGET_USER);
        char * targets = this->context.get(STRAUTHID_TARGET_DEVICE);
        char * protocols = this->context.get(STRAUTHID_TARGET_PROTOCOL);
        char * endtimes = this->context.get(STRAUTHID_END_TIME);

        for (size_t index = 0 ; index < 50 ; index++){
            size_t size_groups = proceed_item(groups);
            size_t size_targets = proceed_item(targets);
            size_t size_protocols = proceed_item(protocols);
            size_t size_endtimes = proceed_item(endtimes, ';');

            char c_group = groups[size_groups];
            char c_target = targets[size_targets];
            char c_protocol = protocols[size_protocols];
            char c_endtime = endtimes[size_endtimes];

            groups[size_groups] = '\0';
            targets[size_targets] = '\0';
            protocols[size_protocols] = '\0';
            endtimes[size_endtimes] = '\0';

            this->add_device(groups, targets, protocols, endtimes);

            groups[size_groups] = c_group;
            targets[size_targets] = c_target;
            protocols[size_protocols] = c_protocol;
            endtimes[size_endtimes] = c_endtime;


            if (c_group    == '\n' || !c_group
            ||  c_target   == '\n' || !c_target
            ||  c_protocol == '\n' || !c_protocol
            ||  c_endtime  == '\n' || !c_endtime
            ){
                break;
            }

            groups += size_groups + 1;
            targets += size_targets + 1;
            protocols += size_protocols + 1;
            endtimes += size_endtimes + 1;
        }
    }


    static inline size_t proceed_item(const char * list, char sep = ' ')
    {
        const char * p = list;
        while (*p != sep && *p != '\n' && *p){
            p++;
        }
        return p - list;
    }
};

#endif
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
//#include "pager.hpp"
//#include "selectline.hpp"

class WidgetSelectLine : public Widget
{
public:
    uint current_index;
    std::vector<WidgetLabel*> labels;

    WidgetSelectLine(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier, int id = 0)
    : Widget(drawable, rect, parent, notifier, id)
    {}

    virtual ~WidgetSelectLine()
    {
        for (size_t i = 0; i < this->labels.size(); ++i) {
            delete this->labels[i];
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

public:
    WidgetSelector(ModApi* drawable, uint16_t width, uint16_t height, Widget* parent, NotifyApi* notifier)
    : WidgetComposite(drawable, Rect(0,0,width,height), parent, notifier)
    , device_group_label(drawable, 0, 0, this, NULL, "???", false, -10, BLACK, WABGREEN)
    , device_group_label(drawable, 0, 0, this, NULL, "Device Group", false, -11, BLACK, WABGREEN)
    , account_device_label(drawable, 0, 0, this, NULL, "Account Device", false, -12, BLACK, WABGREEN)
    , protocol_label(drawable, 0, 0, this, NULL, "Protocol", false, -13, BLACK, WABGREEN)
    , close_time_label(drawable, 0, 0, this, NULL, "Close Time", false, -14, BLACK, WABGREEN)
    , device_group_lines(drawable, Rect(0,0,0,0), this, -15, GREEN)
    , account_device_lines(drawable, Rect(0,0,0,0), this, -16, GREEN)
    , protocol_lines(drawable, Rect(0,0,0,0), this, -17, GREEN)
    , close_time_lines(drawable, Rect(0,0,0,0), this, -18, GREEN)
    , filter_device_group(drawable, 0, 0, 0, this, this, NULL, -19, WHITE, BLACK, -1, 1, 1)
    , filter_account_device(drawable, 0, 0, 0, this, this, NULL, -20, WHITE, BLACK, -1, 1, 1)
    , logout(drawable, 0, 0, this, this, "Logout", true, -21, WHITE, BLACK, 6, 2)
    , apply(drawable, 0, 0, this, this, "Appy", true, -22, WHITE, BLACK, 6, 2)
    , connect(drawable, 0, 0, this, this, "Connect", true, -23, WHITE, BLACK, 6, 2)
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
    }

    virtual ~WidgetSelector()
    {}

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
         || NOTIFY_SUBMIT == event
         && (widget == &this->filter_account_device
          || widget == &this->filter_device_group
        )) {
            this->send_notify(NOTIFY_SUBMIT, 2);
        }
        else if (widget == this->connect) {
            this->send_notify(NOTIFY_SUBMIT);
        }
    }

private:
    void set_index_list(int idx)
    {
        this->device_group_lines.current_index = idx;
        this->account_device_lines.current_index = idx;
        this->protocol_lines.current_index = idx;
        this->close_time_lines.current_index = idx;
    }
};

#endif
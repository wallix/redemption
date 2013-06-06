/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SELECTOR_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "selector.hpp"
#include "widget2_internal_mod.hpp"

class SelectorMod : public InternalMod, public NotifyApi
{
    WidgetSelector selector;
    int current_page;
    int number_page;
    int lines_per_page;

    Inifile & ini;

    struct temporary_buffer {
        char buffer[16];
    };

    struct temporary_login {
        char buffer[256];

        temporary_login(Inifile& ini) {
            buffer[0] = 0;
            sprintf(buffer, "%s@%s", ini.globals.auth_user, ini.globals.host);
        }
    };

public:
    SelectorMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    , selector(this, temporary_login(ini).buffer, width, height, this,
               ini.context_get_value(AUTHID_SELECTOR_CURRENT_PAGE,
                                     temporary_buffer().buffer, sizeof(temporary_buffer)),
               ini.context_get_value(AUTHID_SELECTOR_NUMBER_OF_PAGES,
                                     temporary_buffer().buffer, sizeof(temporary_buffer)),
               ini.context_get_value(AUTHID_SELECTOR_GROUP_FILTER, NULL, 0),
               ini.context_get_value(AUTHID_SELECTOR_DEVICE_FILTER, NULL, 0)
              )
    , current_page(atoi(this->selector.current_page.get_text()))
    , number_page(atoi(this->selector.number_page.get_text()+1))
    , ini(ini)
    {
        {
            char buffer[16];
            ini.context_get_value(AUTHID_SELECTOR_LINES_PER_PAGE, buffer, sizeof(buffer));
            this->lines_per_page = atoi(buffer);
        }
        this->refresh_device();

        //BEGIN TEST
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
//         this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        //END TEST

        this->selector.set_widget_focus(&this->selector.device_lines);
        this->screen.set_widget_focus(&this->selector);

//         this->ini.globals.context.selector_lines_per_page = 5;
//         this->lines_per_page = 5;
//         this->signal = BACK_EVENT_REFRESH;
//         this->event.set();

        this->selector.refresh(this->selector.rect);
    }

    virtual ~SelectorMod()
    {
    }

    void ask_page()
    {
        this->ini.context_ask(AUTHID_SELECTOR);
        char buffer[32];
        sprintf(buffer, "%u", (unsigned int)this->current_page);
        this->ini.context_set_value(AUTHID_SELECTOR_CURRENT_PAGE, buffer);
        this->ini.context_set_value(AUTHID_SELECTOR_GROUP_FILTER,
                                    this->selector.filter_device.get_text());
        this->ini.context_set_value(AUTHID_SELECTOR_DEVICE_FILTER,
                                    this->selector.filter_target.get_text());
        this->ini.context_ask(AUTHID_TARGET_USER);
        this->ini.context_ask(AUTHID_TARGET_DEVICE);
        this->ini.context_ask(AUTHID_SELECTOR);
        this->signal = BACK_EVENT_REFRESH;
        this->event.set();
    }

    virtual void notify(Widget2* widget, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (NOTIFY_CANCEL == event) {
            this->ini.context_ask(AUTHID_AUTH_USER);
            this->ini.context_ask(AUTHID_PASSWORD);
            this->ini.context_ask(AUTHID_TARGET_USER);
            this->ini.context_ask(AUTHID_TARGET_DEVICE);
            this->ini.context_ask(AUTHID_SELECTOR);
            this->signal = BACK_EVENT_NEXT;
            this->event.set();
        }
        else if (NOTIFY_SUBMIT == event) {
            if (widget == &this->selector.connect
             || widget->group_id == this->selector.device_lines.group_id) {
                char buffer[1024];
                sprintf(buffer, "%s:%s",
                        this->selector.target_lines.get_current_index(),
                        this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0));
                this->ini.parse_username(buffer);
                this->signal = BACK_EVENT_NEXT;
                this->event.set();
            }
            else if (widget->group_id == this->selector.apply.group_id) {
                this->ask_page();
            }
            else if (widget == &this->selector.first_page) {
                if (this->current_page > 1) {
                    this->current_page = 1;
                    this->ask_page();
                }
            }
            else if (widget == &this->selector.prev_page) {
                if (this->current_page > 1) {
                    --this->current_page;
                    this->ask_page();
                }
            }
            else if (widget == &this->selector.current_page) {
                int page = atoi(this->selector.current_page.get_text());
                if (page != this->current_page && page <= this->number_page) {
                    this->current_page = page;
                    this->ask_page();
                }
            }
            else if (widget == &this->selector.next_page) {
                if (this->current_page < this->number_page) {
                    ++this->current_page;
                    this->ask_page();
                }
            }
            else if (widget == &this->selector.last_page) {
                if (this->current_page < this->number_page) {
                    this->current_page = this->number_page;
                    this->ask_page();
                }
            }
        }
    }

    virtual void refresh_context(Inifile& ini)
    {
        char buffer[16];

        ini.context_get_value(AUTHID_SELECTOR_LINES_PER_PAGE, buffer, sizeof(buffer));
        this->lines_per_page = atoi(buffer);

        ini.context_get_value(AUTHID_SELECTOR_CURRENT_PAGE, buffer, sizeof(buffer));
        this->selector.current_page.set_text(buffer);
        this->current_page = atoi(buffer);

        ini.context_get_value(AUTHID_SELECTOR_NUMBER_OF_PAGES, buffer, sizeof(buffer));
        this->selector.number_page.set_text(WidgetSelector::temporary_number_of_page(buffer).buffer);
        this->number_page = atoi(buffer);

        uint16_t cy = this->selector.device_lines.cy();

        this->selector.device_lines.clear();
        this->selector.target_lines.clear();
        this->selector.protocol_lines.clear();
        this->selector.close_time_lines.clear();

        this->refresh_device();

        this->selector.refresh(Rect(
            this->selector.device_lines.dx(),
            this->selector.device_lines.dy(),
            this->selector.close_time_lines.dx() + this->selector.close_time_lines.cx() - this->selector.device_lines.dx(),
            std::max(cy, this->selector.device_lines.cy())
        ));
        this->selector.current_page.refresh(this->selector.current_page.rect);
        this->selector.number_page.refresh(this->selector.number_page.rect);
    }

    void refresh_device()
    {
        char * groups    = const_cast<char *>(this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0));
        char * targets   = const_cast<char *>(this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0));
        char * protocols = const_cast<char *>(this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0));
        char * endtimes  = const_cast<char *>((const char *)this->ini.globals.context.end_time);

        for (int index = 0 ; index < this->lines_per_page ; index++) {
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

            this->selector.add_device(groups, targets, protocols, endtimes);

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

        this->selector.target_lines.init_current_index(0);
        this->selector.device_lines.init_current_index(0);
        this->selector.close_time_lines.init_current_index(0);
        this->selector.protocol_lines.init_current_index(0);
    }

    static inline size_t proceed_item(const char * list, char sep = ' ')
    {
        const char * p = list;
        while (*p != sep && *p != '\n' && *p){
            p++;
        }
        return p - list;
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {}

    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        return this->signal;
    }
};

#endif

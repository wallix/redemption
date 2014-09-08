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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_SELECTOR2_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_SELECTOR2_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_selector2.hpp"
#include "internal_mod.hpp"

class FlatSelector2Mod : public InternalMod, public NotifyApi
{
    WidgetSelectorFlat2 selector;
    int current_page;
    int number_page;

    Inifile & ini;

    struct temporary_login {
        char buffer[256];

        temporary_login(Inifile& ini) {
            this->buffer[0] = 0;
            snprintf(this->buffer, sizeof(this->buffer),
                     "%s@%s", ini.globals.auth_user.get_cstr(), ini.globals.host.get_cstr());
        }
    };

public:
    FlatSelector2Mod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, &ini)
        , selector(*this, temporary_login(ini).buffer, width, height, this->screen, this,
                   ini.context.selector_current_page.get_value(),
                   ini.context.selector_number_of_pages.get_value(),
                   ini.context.selector_group_filter.get_cstr(),
                   ini.context.selector_device_filter.get_cstr(),
                   ini.context.selector_proto_filter.get_cstr(),
                   ini)
        , current_page(atoi(this->selector.current_page.get_text()))
        , number_page(atoi(this->selector.number_page.get_text()+1))
        , ini(ini)
    {
        this->selector.set_widget_focus(&this->selector.selector_lines, Widget2::focus_reason_tabkey);
        this->screen.add_widget(&this->selector);
        this->screen.set_widget_focus(&this->selector, Widget2::focus_reason_tabkey);

        uint16_t available_height = (this->selector.first_page.dy() - 10) - this->selector.selector_lines.dy();
        int w, h = 0;
        this->text_metrics("Lp", w, h);
        uint16_t line_height = h + 2 * this->selector.selector_lines.border;

        this->ini.context.selector_lines_per_page.set(available_height / line_height);
        this->ask_page();
        this->selector.refresh(this->selector.rect);
    }

    virtual ~FlatSelector2Mod()
    {
        this->screen.clear();
    }

    void ask_page()
    {
    	this->ini.context.selector_current_page.set((uint32_t)this->current_page);
        this->ini.context_set_value(AUTHID_SELECTOR_GROUP_FILTER,
                                    this->selector.filter_target_group.get_text());
        this->ini.context_set_value(AUTHID_SELECTOR_DEVICE_FILTER,
                                    this->selector.filter_target.get_text());
        this->ini.context_set_value(AUTHID_SELECTOR_PROTO_FILTER,
                                    this->selector.filter_protocol.get_text());
        this->ini.context_ask(AUTHID_TARGET_USER);
        this->ini.context_ask(AUTHID_TARGET_DEVICE);
        this->ini.context_ask(AUTHID_SELECTOR);
        this->event.signal = BACK_EVENT_REFRESH;
        this->event.set();
    }

    virtual void notify(Widget2* widget, notify_event_t event)
    {
        if (NOTIFY_CANCEL == event) {
            this->ini.context_ask(AUTHID_AUTH_USER);
            this->ini.context_ask(AUTHID_PASSWORD);
            this->ini.context.selector.set(false);
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();
        }
        else if (NOTIFY_SUBMIT == event) {
            if (widget == &this->selector.connect) {
                char buffer[1024];
                uint16_t row_index = 0;
                uint16_t column_index = 0;
                this->selector.selector_lines.get_selection(row_index, column_index);
                const char * target = this->selector.selector_lines.get_cell_text(row_index, WidgetSelectorFlat2::IDX_TARGET);
                snprintf(buffer, sizeof(buffer), "%s:%s",
                         target, this->ini.globals.auth_user.get_cstr());
                this->ini.parse_username(buffer);


                this->event.signal = BACK_EVENT_NEXT;
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
                if (page != this->current_page) {
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

	this->current_page = ini.context.selector_current_page.get();
	snprintf(buffer, sizeof(buffer), "%u", this->current_page);
	this->selector.current_page.set_text(buffer);

	this->number_page = ini.context.selector_number_of_pages.get();
	snprintf(buffer, sizeof(buffer), "%u", this->number_page);
	this->selector.number_page.set_text(WidgetSelectorFlat2::temporary_number_of_page(buffer).buffer);


        // uint16_t cy = this->selector.selector_lines.cy();

        this->selector.selector_lines.clear();

        this->refresh_device();

        this->selector.refresh(this->selector.rect);

        this->selector.current_page.refresh(this->selector.current_page.rect);
        this->selector.number_page.refresh(this->selector.number_page.rect);
        this->event.reset();
    }

    void refresh_device()
    {
        char * groups    = const_cast<char *>(this->ini.globals.target_user.get_cstr());
        char * targets   = const_cast<char *>(this->ini.globals.target_device.get_cstr());
        char * protocols = const_cast<char *>(this->ini.context.target_protocol.get_cstr());
        for (unsigned index = 0; index < this->ini.context.selector_lines_per_page.get();
             index++) {
            size_t size_groups = proceed_item(groups, '\x01');
            if (!size_groups)
                break;
            size_t size_targets = proceed_item(targets, '\x01');
            size_t size_protocols = proceed_item(protocols, '\x01');

            char c_group = groups[size_groups];
            char c_target = targets[size_targets];
            char c_protocol = protocols[size_protocols];

            groups[size_groups] = '\0';
            targets[size_targets] = '\0';
            protocols[size_protocols] = '\0';

            this->selector.add_device(groups, targets, protocols);

            groups[size_groups] = c_group;
            targets[size_targets] = c_target;
            protocols[size_protocols] = c_protocol;

            if (c_group    == '\n' || !c_group
                ||  c_target   == '\n' || !c_target
                ||  c_protocol == '\n' || !c_protocol
                ){
                break;
            }

            groups += size_groups + 1;
            targets += size_targets + 1;
            protocols += size_protocols + 1;

        }

        if (this->selector.selector_lines.get_nb_rows() == 0) {
            this->selector.selector_lines.tab_flag = Widget2::IGNORE_TAB;
            this->selector.selector_lines.focus_flag = Widget2::IGNORE_FOCUS;
            this->selector.add_device("", TR("no_results", this->ini), "");
        } else {
            this->selector.selector_lines.tab_flag = Widget2::NORMAL_TAB;
            this->selector.selector_lines.focus_flag = Widget2::NORMAL_FOCUS;
            this->selector.selector_lines.set_selection(0, static_cast<uint16_t>(-1));
            this->selector.set_widget_focus(&this->selector.selector_lines, Widget2::focus_reason_tabkey);
        }
        this->selector.rearrange();
    }

    static inline size_t proceed_item(const char * list, char sep = ' ')
    {
        const char * p = list;
        while (*p != sep && *p != '\n' && *p){
            p++;
        }
        return p - list;
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap)
    {
        if (&this->selector.selector_lines == this->selector.current_focus
            && keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_LEFT_ARROW:
                keymap->get_kevent();
                if (this->current_page > 1) {
                    --this->current_page;
                    this->ask_page();
                }
                else if (this->current_page == 1 && this->number_page > 1) {
                    this->current_page = this->number_page;
                    this->ask_page();
                }
                break;
            case Keymap2::KEVENT_RIGHT_ARROW:
                keymap->get_kevent();
                if (this->current_page < this->number_page) {
                    ++this->current_page;
                    this->ask_page();
                }
                else if (this->current_page == this->number_page && this->number_page > 1) {
                    this->current_page = 1;
                    this->ask_page();
                }
                break;
            default:
                this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
        else {
            this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }


    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }
};

#endif

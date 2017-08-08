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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan, Jennifer Inthavong
 */


#pragma once

#include "core/front_api.hpp"
#include "configs/config.hpp"
#include "configs/config_access.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/internal_mod.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget/flat_selector.hpp"
#include "mod/internal/widget/language_button.hpp"




class FlatSelectorMod : public LocallyIntegrableMod, public NotifyApi
{
    LanguageButton language_button;
//     WidgetSelectorFlat selector;
    Selector selector;


    int current_page;
    int number_page;

    FlatSelectorModVariables vars;

    CopyPaste copy_paste;

    bool waiting_for_next_module = false;

    struct temporary_login {
        char buffer[256];

        explicit temporary_login(FlatSelectorModVariables vars) {
            this->buffer[0] = 0;
            snprintf(this->buffer, sizeof(this->buffer),
                     "%s@%s",
                     vars.get<cfg::globals::auth_user>().c_str(),
                     vars.get<cfg::globals::host>().c_str());
        }
    };

    int selector_lines_per_page_saved = 0;




public:
    FlatSelectorMod(FlatSelectorModVariables vars, FrontAPI & front, uint16_t width, uint16_t height, Rect const widget_rect, ClientExecute & client_execute)
        : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
        , language_button(vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->selector, front, front, this->font(), this->theme())

//         , selector(
//             front, temporary_login(vars).buffer,
//             widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
//             this->screen, this,
//             vars.is_asked<cfg::context::selector_current_page>()
//                 ? "" : configs::make_zstr_buffer(vars.get<cfg::context::selector_current_page>()).get(),
//             vars.is_asked<cfg::context::selector_number_of_pages>()
//                 ? "" : configs::make_zstr_buffer(vars.get<cfg::context::selector_number_of_pages>()).get(),
//             vars.get<cfg::context::selector_group_filter>().c_str(),
//             vars.get<cfg::context::selector_device_filter>().c_str(),
//             vars.get<cfg::context::selector_proto_filter>().c_str(),
//             &this->language_button,
//             vars.get<cfg::font>(),
//             vars.get<cfg::theme>(),
//             language(vars))

        , selector(front, temporary_login(vars).buffer,
            widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
            this->screen, this,
            vars.is_asked<cfg::context::selector_current_page>()
                ? "" : configs::make_zstr_buffer(vars.get<cfg::context::selector_current_page>()).get(),
            vars.is_asked<cfg::context::selector_number_of_pages>()
                ? "" : configs::make_zstr_buffer(vars.get<cfg::context::selector_number_of_pages>()).get(),
            &this->language_button,
            3, &vars,
            vars.get<cfg::font>(),
            vars.get<cfg::theme>(),
            language(vars))

        , current_page(atoi(this->selector.current_page.get_text()))
        , number_page(atoi(this->selector.number_page.get_text()+1))
        , vars(vars)
        , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    {
        this->selector.set_widget_focus(&this->selector.selector_lines, Widget::focus_reason_tabkey);
        this->screen.add_widget(&this->selector);
        this->screen.set_widget_focus(&this->selector, Widget::focus_reason_tabkey);

        uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
        gdi::TextMetrics tm(this->vars.get<cfg::font>(), "Édp");
        uint16_t line_height = tm.height + 2 * (
                                this->selector.selector_lines.border
                             +  this->selector.selector_lines.y_padding_label);

        this->selector_lines_per_page_saved = available_height / line_height;
        this->vars.set_acl<cfg::context::selector_lines_per_page>(this->selector_lines_per_page_saved);
        this->ask_page();
        this->selector.rdp_input_invalidate(this->selector.get_rect());
    }

    ~FlatSelectorMod() override {
        this->screen.clear();
    }

    void ask_page()
    {
        this->vars.set_acl<cfg::context::selector_current_page>(static_cast<unsigned>(this->current_page));


        this->vars.set_acl<cfg::context::selector_group_filter>(this->selector.edit_filter[0]->get_text());
        this->vars.set_acl<cfg::context::selector_device_filter>(this->selector.edit_filter[1]->get_text());
        this->vars.set_acl<cfg::context::selector_proto_filter>(this->selector.edit_filter[2]->get_text());


//         this->vars.set_acl<cfg::context::selector_group_filter>(this->selector.filter_target_group.get_text());
//         this->vars.set_acl<cfg::context::selector_device_filter>(this->selector.filter_target.get_text());
//         this->vars.set_acl<cfg::context::selector_proto_filter>(this->selector.filter_protocol.get_text());


        this->vars.ask<cfg::globals::target_user>();
        this->vars.ask<cfg::globals::target_device>();
        this->vars.ask<cfg::context::selector>();
        this->event.signal = BACK_EVENT_REFRESH;
//        this->event.set();
        this->event.set_trigger_time(wait_obj::NOW);
    }

    void notify(Widget* widget, notify_event_t event) override {
        switch (event) {
        case NOTIFY_CANCEL: {
            if (this->waiting_for_next_module) {
                LOG(LOG_INFO, "FlatSelector2Mod::notify: NOTIFY_CANCEL - Waiting for next module.");
                return;
            }

            this->vars.ask<cfg::globals::auth_user>();
            this->vars.ask<cfg::context::password>();
            this->vars.set<cfg::context::selector>(false);
            this->event.signal = BACK_EVENT_NEXT;
//            this->event.set();
            this->event.set_trigger_time(wait_obj::NOW);

            this->waiting_for_next_module = true;
        } break;
        case NOTIFY_SUBMIT: {
            if (this->waiting_for_next_module) {
                LOG(LOG_INFO, "FlatSelector2Mod::notify: NOTIFY_SUBMIT - Waiting for next module.");
                return;
            }

            if (widget == &this->selector.connect) {
                char buffer[1024] = {};
                uint16_t row_index = 0;
                uint16_t column_index = 0;
                this->selector.selector_lines.get_selection(row_index, column_index);
                const char * target = this->selector.selector_lines.get_cell_text(row_index, WidgetSelectorFlat::IDX_TARGET);
                const char * groups = this->selector.selector_lines.get_cell_text(row_index, WidgetSelectorFlat::IDX_TARGETGROUP);
                snprintf(buffer, sizeof(buffer), "%s:%s:%s",
                         target, groups, this->vars.get<cfg::globals::auth_user>().c_str());
                this->vars.set_acl<cfg::globals::auth_user>(buffer);
                this->vars.ask<cfg::globals::target_user>();
                this->vars.ask<cfg::globals::target_device>();
                this->vars.ask<cfg::context::target_protocol>();

                this->event.signal = BACK_EVENT_NEXT;
//                this->event.set();
                this->event.set_trigger_time(wait_obj::NOW);

                this->waiting_for_next_module = true;
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
        } break;
        case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT: {
            if (this->copy_paste) {
                copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(widget), event);
            }
        } break;
        default:;
        }
    }

    void refresh_context() override {
        char buffer[16];

        this->current_page = vars.get<cfg::context::selector_current_page>();
        snprintf(buffer, sizeof(buffer), "%d", this->current_page);
        this->selector.current_page.set_text(buffer);

        this->number_page = vars.get<cfg::context::selector_number_of_pages>();
        snprintf(buffer, sizeof(buffer), "%d", this->number_page);
        this->selector.number_page.set_text(WidgetSelectorFlat::temporary_number_of_page(buffer).buffer);

        this->selector.selector_lines.clear();

        this->refresh_device();

        this->selector.rdp_input_invalidate(this->selector.get_rect());

        this->selector.current_page.rdp_input_invalidate(this->selector.current_page.get_rect());
        this->selector.number_page.rdp_input_invalidate(this->selector.number_page.get_rect());
//        this->event.reset();
        this->event.reset_trigger_time();
    }

private:
    void refresh_device()
    {
        char * groups    = const_cast<char *>(this->vars.get<cfg::globals::target_user>().c_str());
        char * targets   = const_cast<char *>(this->vars.get<cfg::globals::target_device>().c_str());
        char * protocols = const_cast<char *>(this->vars.get<cfg::context::target_protocol>().c_str());
        for (unsigned index = 0; index < this->vars.get<cfg::context::selector_lines_per_page>();
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

            //this->selector.add_device(groups, targets, protocols);

            const char * texts[] = { groups, targets, protocols,  "", "", "", "", "", "", ""};
            this->selector.add_device(texts);

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
            this->selector.selector_lines.tab_flag = Widget::IGNORE_TAB;
            this->selector.selector_lines.focus_flag = Widget::IGNORE_FOCUS;

            const char * texts[] = { "", TR(trkeys::no_results, language(this->vars)), "", "", "", "", "", "", "", ""};
            this->selector.add_device(texts);

            //this->selector.add_device("", TR(trkeys::no_results, language(this->vars)), "", "");
        } else {
            this->selector.selector_lines.tab_flag = Widget::NORMAL_TAB;
            this->selector.selector_lines.focus_flag = Widget::NORMAL_FOCUS;
            this->selector.selector_lines.set_selection(0);
            this->selector.set_widget_focus(&this->selector.selector_lines, Widget::focus_reason_tabkey);
        }
        this->selector.move_size_widget(
            this->selector.x(),
            this->selector.y(),
            this->selector.cx(),
            this->selector.cy());
    }

public:
    static inline size_t proceed_item(const char * list, char sep = ' ')
    {
        const char * p = list;
        while (*p != sep && *p != '\n' && *p){
            p++;
        }
        return p - list;
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap) override {
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

        LocallyIntegrableMod::rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void draw_event(time_t now, gdi::GraphicApi & gapi) override {
        LocallyIntegrableMod::draw_event(now, gapi);

        if (!this->copy_paste && this->event.is_waked_up_by_time()) {
            this->copy_paste.ready(this->front);
        }

//        this->event.reset();
        this->event.reset_trigger_time();
    }

    bool is_up_and_running() override { return true; }

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk, length, flags);

        if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
            this->copy_paste.send_to_mod_channel(chunk, flags);
        }
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override {
        this->selector.move_size_widget(left, top, width, height);

        uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
        gdi::TextMetrics tm(this->vars.get<cfg::font>(), "Édp");
        uint16_t line_height = tm.height + 2 * (
                                this->selector.selector_lines.border
                             +  this->selector.selector_lines.y_padding_label);

        int selector_lines_per_page = available_height / line_height;
        if (this->selector_lines_per_page_saved != selector_lines_per_page) {
            this->selector_lines_per_page_saved = selector_lines_per_page;

            this->vars.set_acl<cfg::context::selector_lines_per_page>(available_height / line_height);
            this->ask_page();
            this->selector.rdp_input_invalidate(this->selector.get_rect());
        }
    }
};

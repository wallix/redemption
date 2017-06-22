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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */


#pragma once

#include "configs/config.hpp"
#include "configs/config_access.hpp"
#include "core/front_api.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "utils/timeout.hpp"
#include "widget2/flat_wait.hpp"
#include "widget2/language_button.hpp"
#include "widget2/screen.hpp"

using FlatWaitModVariables = vcfg::variables<
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::context::comment,                    vcfg::accessmode::set>,
    vcfg::var<cfg::context::duration,                   vcfg::accessmode::set>,
    vcfg::var<cfg::context::ticket,                     vcfg::accessmode::set>,
    vcfg::var<cfg::context::waitinforeturn,             vcfg::accessmode::set>,
    vcfg::var<cfg::context::duration_max,               vcfg::accessmode::get>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::font,                                vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                               vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;

class FlatWaitMod : public LocallyIntegrableMod, public NotifyApi
{
    LanguageButton language_button;
    FlatWait wait_widget;

    FlatWaitModVariables vars;
    Timeout timeout;

    CopyPaste copy_paste;

public:
    FlatWaitMod(FlatWaitModVariables vars, FrontAPI & front, uint16_t width, uint16_t height, Rect const widget_rect,
                const char * caption, const char * message, time_t now, ClientExecute & client_execute,
                bool showform = false, uint32_t flag = 0)
        : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
        , language_button(vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->wait_widget, front, front, this->font(), this->theme())
        , wait_widget(front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy, this->screen, this, caption, message, 0,
                      &this->language_button,
                      vars.get<cfg::font>(),
                      vars.get<cfg::theme>(),
                      language(vars),
                      showform, flag, vars.get<cfg::context::duration_max>()
                      )
        , vars(vars)
        , timeout(now, 600)
        , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
    {
        this->screen.add_widget(&this->wait_widget);
        if (this->wait_widget.hasform) {
            this->wait_widget.set_widget_focus(&this->wait_widget.form, Widget2::focus_reason_tabkey);
        }
        else {
            this->wait_widget.set_widget_focus(&this->wait_widget.goselector, Widget2::focus_reason_tabkey);
        }
        this->screen.set_widget_focus(&this->wait_widget, Widget2::focus_reason_tabkey);
        this->screen.rdp_input_invalidate(this->screen.get_rect());
    }

    ~FlatWaitMod() override {
        this->screen.clear();
    }

    void notify(Widget2 * sender, notify_event_t event) override {
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            case NOTIFY_TEXT_CHANGED: this->confirm(); break;
            case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
                if (this->copy_paste) {
                    copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit *>(sender), event);
                };
                break;
            default:;
        }
    }

private:
    void confirm()
    {
        this->vars.set_acl<cfg::context::waitinforeturn>("confirm");
        this->vars.set_acl<cfg::context::comment>(this->wait_widget.form.comment_edit.get_text());
        this->vars.set_acl<cfg::context::ticket>(this->wait_widget.form.ticket_edit.get_text());
        this->vars.set_acl<cfg::context::duration>(this->wait_widget.form.duration_edit.get_text());
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }
    // TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
    void accepted()
    {
        this->vars.set_acl<cfg::context::waitinforeturn>("backselector");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    // TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
    void refused()
    {
        this->vars.set_acl<cfg::context::waitinforeturn>("exit");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    void draw_event(time_t now, gdi::GraphicApi & gapi) override {
        LocallyIntegrableMod::draw_event(now, gapi);

        if (!this->copy_paste && event.waked_up_by_time) {
            this->copy_paste.ready(this->front);
        }

        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->refused();
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            this->event.set(1000000);
            break;
        case Timeout::TIMEOUT_INACTIVE:
            this->event.reset();
            break;
        }
    }

    bool is_up_and_running() override { return true; }

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream& chunk, size_t length, uint32_t flags) override {
        LocallyIntegrableMod::send_to_mod_channel(front_channel_name, chunk, length, flags);

        if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
            this->copy_paste.send_to_mod_channel(chunk, flags);
        }
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override {
        this->wait_widget.move_size_widget(left, top, width, height);
    }
};

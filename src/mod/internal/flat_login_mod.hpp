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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan,
 *              Jennifer Inthavong
 */


#pragma once

#include "main/version.hpp"
#include "configs/config_access.hpp"
#include "core/front_api.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "mod/internal/widget/flat_login.hpp"
#include "mod/internal/widget/language_button.hpp"
#include "mod/internal/widget/notify_api.hpp"
#include "utils/timeout.hpp"

# include <chrono>


using FlatLoginModVariables = vcfg::variables<
    vcfg::var<cfg::context::password,                   vcfg::accessmode::set>,
    vcfg::var<cfg::globals::auth_user,                  vcfg::accessmode::set>,
    vcfg::var<cfg::context::selector,                   vcfg::accessmode::ask>,
    vcfg::var<cfg::context::target_protocol,            vcfg::accessmode::ask>,
    vcfg::var<cfg::globals::target_device,              vcfg::accessmode::ask>,
    vcfg::var<cfg::globals::target_user,                vcfg::accessmode::ask>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::font,                                vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                               vcfg::accessmode::get>,
    vcfg::var<cfg::context::opt_message,                vcfg::accessmode::get>,
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>,
    vcfg::var<cfg::globals::authentication_timeout,     vcfg::accessmode::get>,
    vcfg::var<cfg::debug::mod_internal,                 vcfg::accessmode::get>
>;


class FlatLoginMod : public LocallyIntegrableMod, public NotifyApi
{
    LanguageButton language_button;

    FlatLogin login;
    Timeout timeout;

    CopyPaste copy_paste;

    FlatLoginModVariables vars;

public:
    FlatLoginMod(
        FlatLoginModVariables vars,
        char const * username, char const * password,
        FrontAPI & front, uint16_t width, uint16_t height, Rect const widget_rect, time_t now,
        ClientExecute & client_execute
    )
        : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
        , language_button(
            vars.get<cfg::client::keyboard_layout_proposals>().c_str(),
            this->login, front, front, this->font(), this->theme())
        , login(
            front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
            this->screen, this, "Redemption " VERSION,
            nullptr, nullptr,
            TR(trkeys::login, language(vars)),
            TR(trkeys::password, language(vars)),
            vars.get<cfg::context::opt_message>().c_str(),
            &this->language_button,
            this->font(), Translator(language(vars)), this->theme())
        , timeout(now, vars.get<cfg::globals::authentication_timeout>().count())
        , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
        , vars(vars)
    {
        if (vars.get<cfg::globals::authentication_timeout>().count()) {
            LOG(LOG_INFO, "LoginMod: Ending session in %u seconds",
                static_cast<unsigned>(vars.get<cfg::globals::authentication_timeout>().count()));
        }
        this->screen.add_widget(&this->login);

        this->login.login_edit.set_text(username);
        this->login.password_edit.set_text(password);

        this->screen.set_widget_focus(&this->login, Widget::focus_reason_tabkey);

        this->login.set_widget_focus(&this->login.login_edit, Widget::focus_reason_tabkey);
        if (username[0] != 0){
            this->login.set_widget_focus(&this->login.password_edit, Widget::focus_reason_tabkey);
        }

        this->screen.rdp_input_invalidate(this->screen.get_rect());
    }

    ~FlatLoginMod() override {
        this->screen.clear();
    }

    void notify(Widget* sender, notify_event_t event) override {
        switch (event) {
        case NOTIFY_SUBMIT:
            LOG(LOG_INFO, "asking for selector");
            this->vars.set_acl<cfg::globals::auth_user>(this->login.login_edit.get_text());
            this->vars.ask<cfg::context::selector>();
            this->vars.ask<cfg::globals::target_user>();
            this->vars.ask<cfg::globals::target_device>();
            this->vars.ask<cfg::context::target_protocol>();
            this->vars.set_acl<cfg::context::password>(this->login.password_edit.get_text());
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set_trigger_time(wait_obj::NOW);
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set_trigger_time(wait_obj::NOW);
            break;
        case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT:
            if (this->copy_paste) {
                copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event);
            }
            break;
        default:;
        }
    }

    void draw_event(time_t now, gdi::GraphicApi & gapi) override {
        LocallyIntegrableMod::draw_event(now, gapi);

        if (!this->copy_paste && this->event.is_waked_up_by_time()) {
            this->copy_paste.ready(this->front);
        }

        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set_trigger_time(wait_obj::NOW);
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            this->event.set_trigger_time(200000);
            break;
        case Timeout::TIMEOUT_INACTIVE:
            this->event.reset_trigger_time();
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
        this->login.move_size_widget(left, top, width, height);
    }
};

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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */


#pragma once

#include "configs/config.hpp"
#include "configs/config_access.hpp"
#include "core/front_api.hpp"
#include "mod/internal/locally_integrable_mod.hpp"
#include "utils/translation.hpp"
#include "mod/internal/widget2/flat_interactive_target.hpp"
#include "mod/internal/widget2/language_button.hpp"
#include "mod/internal/widget2/screen.hpp"

using InteractiveTargetModVariables = vcfg::variables<
    vcfg::var<cfg::globals::target_user,                vcfg::accessmode::is_asked | vcfg::accessmode::set | vcfg::accessmode::get>,
    vcfg::var<cfg::context::target_password,            vcfg::accessmode::is_asked | vcfg::accessmode::set>,
    vcfg::var<cfg::context::target_host,                vcfg::accessmode::is_asked | vcfg::accessmode::set>,
    vcfg::var<cfg::globals::target_device,              vcfg::accessmode::get>,
    vcfg::var<cfg::context::display_message,            vcfg::accessmode::set>,
    vcfg::var<cfg::translation::language,               vcfg::accessmode::get>,
    vcfg::var<cfg::font,                                vcfg::accessmode::get>,
    vcfg::var<cfg::theme,                               vcfg::accessmode::get>,
    vcfg::var<cfg::client::keyboard_layout_proposals,   vcfg::accessmode::get>
>;

class InteractiveTargetMod : public LocallyIntegrableMod, public NotifyApi
{
    bool ask_device;
    bool ask_login;
    bool ask_password;

    LanguageButton language_button;
    FlatInteractiveTarget challenge;

    InteractiveTargetModVariables vars;

public:
    InteractiveTargetMod(InteractiveTargetModVariables vars, FrontAPI & front, uint16_t width, uint16_t height, Rect const widget_rect, ClientExecute & client_execute)
        : LocallyIntegrableMod(front, width, height, vars.get<cfg::font>(), client_execute, vars.get<cfg::theme>())
        , ask_device(vars.is_asked<cfg::context::target_host>())
        , ask_login(vars.is_asked<cfg::globals::target_user>())
        , ask_password((this->ask_login || vars.is_asked<cfg::context::target_password>()))
        , language_button(vars.get<cfg::client::keyboard_layout_proposals>().c_str(), this->challenge, front, front, this->font(), this->theme())
        , challenge(
            front, widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
            this->screen, this,
            this->ask_device, this->ask_login, this->ask_password,
            vars.get<cfg::theme>(),
            TR(trkeys::target_info_required, language(vars)),
            TR(trkeys::device, language(vars)), vars.get<cfg::globals::target_device>().c_str(),
            TR(trkeys::login, language(vars)), vars.get<cfg::globals::target_user>().c_str(),
            TR(trkeys::password, language(vars)),
            vars.get<cfg::font>(),
            &this->language_button)
        , vars(vars)
    {
        this->screen.add_widget(&this->challenge);
        this->challenge.password_edit.set_text("");
        this->screen.set_widget_focus(&this->challenge,
                                      Widget2::focus_reason_tabkey);
        if (this->ask_device) {
            this->challenge.set_widget_focus(&this->challenge.device_edit,
                                             Widget2::focus_reason_tabkey);
        }
        else if (this->ask_login) {
            this->challenge.set_widget_focus(&this->challenge.login_edit,
                                             Widget2::focus_reason_tabkey);
        }
        else {
            this->challenge.set_widget_focus(&this->challenge.password_edit,
                                             Widget2::focus_reason_tabkey);
        }
        this->screen.rdp_input_invalidate(this->screen.get_rect());
    }

    ~InteractiveTargetMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        (void)sender;
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            default: ;
        }
    }

private:
    // TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
    void accepted()
    {
        if (this->ask_device) {
            this->vars.set_acl<cfg::context::target_host>(this->challenge.device_edit.get_text());
        }
        if (this->ask_login) {
            this->vars.set_acl<cfg::globals::target_user>(this->challenge.login_edit.get_text());
        }
        if (this->ask_password) {
            this->vars.set_acl<cfg::context::target_password>(this->challenge.password_edit.get_text());
        }
        this->vars.set_acl<cfg::context::display_message>(true);
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    // TODO ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod
    void refused()
    {
        this->vars.set_acl<cfg::context::target_password>("");
        this->vars.set_acl<cfg::context::display_message>(false);
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    void draw_event(time_t now, gdi::GraphicApi & gapi) override {
        LocallyIntegrableMod::draw_event(now, gapi);

        this->event.reset();
    }

    bool is_up_and_running() override { return true; }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override {
        this->challenge.move_size_widget(left, top, width, height);
    }
};

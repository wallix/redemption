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
 *              Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_INTERACTIVE_TARGET_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_INTERACTIVE_TARGET_MOD_HPP

#include "translation.hpp"
#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_interactive_target.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"
#include "config_access.hpp"


using InteractiveTargetModVariables = vcfg::variables<
    vcfg::var<cfg::globals::target_user,     vcfg::wait | vcfg::write | vcfg::read>,
    vcfg::var<cfg::context::target_password, vcfg::wait | vcfg::write>,
    vcfg::var<cfg::context::target_host,     vcfg::wait | vcfg::write>,
    vcfg::var<cfg::globals::target_device,   vcfg::read>,
    vcfg::var<cfg::context::display_message, vcfg::write>,
    vcfg::var<cfg::translation::language>,
    vcfg::var<cfg::font>,
    vcfg::var<cfg::theme>
>;

class InteractiveTargetMod : public InternalMod, public NotifyApi
{
    bool ask_device;
    bool ask_login;
    bool ask_password;

    FlatInteractiveTarget challenge;

    InteractiveTargetModVariables vars;

public:
    InteractiveTargetMod(InteractiveTargetModVariables vars, FrontAPI & front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, vars.get<cfg::font>(), vars.get<cfg::theme>())
        , ask_device(vars.is_asked<cfg::context::target_host>())
        , ask_login(vars.is_asked<cfg::globals::target_user>())
        , ask_password((this->ask_login || vars.is_asked<cfg::context::target_password>()))
        , challenge(*this, width, height, this->screen, this, 0,
                    this->ask_device, this->ask_login, this->ask_password,
                    vars.get<cfg::theme>(),
                    TR("target_info_required", language(vars)),
                    TR("device", language(vars)),
                    vars.get<cfg::globals::target_device>().c_str(),
                    TR("login", language(vars)),
                    vars.get<cfg::globals::target_user>().c_str(),
                    TR("password", language(vars)),
                    vars.get<cfg::font>())
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
        this->screen.refresh(this->screen.rect);
    }

    ~InteractiveTargetMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            default: ;
        }
    }

private:
    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
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
        this->vars.set_acl<cfg::context::display_message>("True");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void refused()
    {
        this->vars.set_acl<cfg::context::target_password>("");
        this->vars.set_acl<cfg::context::display_message>("False");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    void draw_event(time_t now) override {
        this->event.reset();
    }

};

#endif

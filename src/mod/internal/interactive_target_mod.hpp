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

#include <functional>

class InteractiveTargetMod : public InternalMod, public NotifyApi
{
    using accepted_notify_type = std::function<void(char const * device, char const * login, char const * password)>;
    using refused_notify_type = std::function<void()>;

    accepted_notify_type accepted_notify;
    refused_notify_type refused_notify;

    bool ask_device;
    bool ask_login;
    bool ask_password;

    FlatInteractiveTarget challenge;

public:
    InteractiveTargetMod(
        std::function<void(char const * device, char const * login, char const * password)> accepted_notify,
        std::function<void()> refused_notify,
        FrontAPI & front, uint16_t width, uint16_t height,
        Translator const & tr, Font const & font, Theme const & theme,
        bool ask_device, bool ask_login, bool ask_password,
        const char * device_str, const char * login_str
    )
        : InternalMod(front, width, height, font, theme)
        , accepted_notify(std::move(accepted_notify))
        , refused_notify(std::move(refused_notify))
        , ask_device(ask_device)
        , ask_login(ask_login)
        , ask_password(this->ask_login || ask_password)
        , challenge(*this, width, height, this->screen, this, 0,
                    this->ask_device, this->ask_login, this->ask_password,
                    theme, tr("target_info_required"),
                    tr("device"), device_str,
                    tr("login"), login_str,
                    tr("password"), font)
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
        this->accepted_notify(
            this->ask_device ? this->challenge.device_edit.get_text() : nullptr,
            this->ask_login ? this->challenge.login_edit.get_text() : nullptr,
            this->ask_password ? this->challenge.password_edit.get_text() : nullptr
        );
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void refused()
    {
        this->refused_notify();
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    void draw_event(time_t now) override {
        this->event.reset();
    }

};

#endif

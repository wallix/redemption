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

class InteractiveTargetMod : public InternalMod, public NotifyApi
{
    bool ask_device;
    bool ask_login;
    bool ask_password;

    FlatInteractiveTarget challenge;

    Inifile & ini;

public:
    InteractiveTargetMod(Inifile & ini, FrontAPI & front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, ini.font, &ini)
        , ask_device(ini.context_is_asked(AUTHID_TARGET_HOST))
        , ask_login(ini.context_is_asked(AUTHID_TARGET_USER))
        , ask_password((this->ask_login || ini.context_is_asked(AUTHID_TARGET_PASSWORD)))
        , challenge(*this, width, height, this->screen, this, 0,
                    ini.context_is_asked(AUTHID_TARGET_HOST),
                    ini.context_is_asked(AUTHID_TARGET_USER),
                    ini.context_is_asked(AUTHID_TARGET_PASSWORD),
                    ini.theme, TR("target_info_required", ini),
                    TR("device", ini), ini.context_get_value(AUTHID_TARGET_DEVICE),
                    TR("login", ini), ini.context_get_value(AUTHID_TARGET_USER),
                    TR("password", ini), ini.font)
        , ini(ini)
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

    virtual ~InteractiveTargetMod()
    {
        this->screen.clear();
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {
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
            this->ini.context_set_value(AUTHID_TARGET_HOST,
                                        this->challenge.device_edit.get_text());
        }
        if (this->ask_login) {
            this->ini.context_set_value(AUTHID_TARGET_USER,
                                        this->challenge.login_edit.get_text());
        }
        if (this->ask_password) {
            this->ini.context_set_value(AUTHID_TARGET_PASSWORD,
                                        this->challenge.password_edit.get_text());
        }
        this->ini.context_set_value(AUTHID_DISPLAY_MESSAGE, "True");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void refused()
    {
        this->ini.context_set_value(AUTHID_TARGET_PASSWORD, "");
        this->ini.context_set_value(AUTHID_DISPLAY_MESSAGE, "False");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }

};

#endif

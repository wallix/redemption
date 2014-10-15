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

#ifndef REDEMPTION_MOD_INTERNAL_INTERACTIVE_PASSWORD_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_INTERACTIVE_PASSWORD_MOD_HPP

#include "translation.hpp"
#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_interactive_password.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"

class InteractivePasswordMod : public InternalMod, public NotifyApi
{
    FlatInteractivePassword challenge;

    Inifile & ini;

public:
    InteractivePasswordMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, &ini)
        , challenge(*this, width, height, this->screen, this,
                    0 , 0, ini.theme, TR("authentication_required", ini),
                    TR("login", ini), ini.context_get_value(AUTHID_TARGET_USER),
                    TR("password", ini))
        , ini(ini)
    {
        this->screen.add_widget(&this->challenge);
        this->challenge.password_edit.set_text("");
        this->screen.set_widget_focus(&this->challenge,
                                      Widget2::focus_reason_tabkey);
        this->challenge.set_widget_focus(&this->challenge.password_edit,
                                         Widget2::focus_reason_tabkey);
        this->screen.refresh(this->screen.rect);
    }

    virtual ~InteractivePasswordMod()
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
        this->ini.context_set_value(AUTHID_TARGET_PASSWORD,
                                    this->challenge.password_edit.get_text());
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

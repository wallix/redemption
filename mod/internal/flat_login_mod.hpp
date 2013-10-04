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

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_LOGIN_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_LOGIN_MOD_HPP

#include "version.hpp"
#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_login.hpp"
#include "internal_mod.hpp"
#include "widget2/notify_api.hpp"
#include "translation.hpp"

class FlatLoginMod : public InternalMod, public NotifyApi
{
    FlatLogin login;

public:
    Inifile & ini;

    FlatLoginMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height)
        , login(*this, width, height, this->screen, this, "Redemption " VERSION,
                ini.account.username[0] != 0,
                0, 0, 0, WHITE, DARK_BLUE_BIS,
                TR("login", &ini),
                TR("password", &ini)
                )
        , ini(ini)
    {
        this->screen.add_widget(&this->login);

        this->login.login_edit.set_text(this->ini.account.username);
        this->login.password_edit.set_text(this->ini.account.password);

        this->screen.set_widget_focus(&this->login);

        this->login.set_widget_focus(&this->login.login_edit);
        if (ini.account.username[0] != 0){
            this->login.set_widget_focus(&this->login.password_edit);
        }

        this->screen.refresh(this->screen.rect);
    }

    virtual ~FlatLoginMod()
    {
        this->screen.clear();
    }
    virtual void notify(Widget2* sender, notify_event_t event)
    {
        switch (event) {
        case NOTIFY_SUBMIT:
            this->ini.parse_username(this->login.login_edit.label.buffer);
            this->ini.context_set_value(AUTHID_PASSWORD, this->login.password_edit.get_text());
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        default:
            break;
        }
    }

    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }
};

#endif

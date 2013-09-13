/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#ifndef REDEMPTION_MOD_INTERNAL_LOGIN_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_LOGIN_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "widget2/window_login.hpp"
#include "widget2/image.hpp"
#include "internal_mod.hpp"
#include "widget2/notify_api.hpp"

class LoginMod : public InternalMod, public NotifyApi
{
    WindowLogin window_login;
    WidgetImage image;

public:
    Inifile & ini;

public:
    LoginMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height)
        , window_login(*this, 0, 0, this->screen, this, VERSION,
                       ini.account.username[0] != 0,
                       0, 0, 0, BLACK, GREY,
                       ini.translation.button_ok.get().c_str(),
                       ini.translation.button_cancel.get().c_str(),
                       ini.translation.button_help.get().c_str(),
                       ini.translation.login.get().c_str(),
                       ini.translation.password.get().c_str())
        , image(*this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, this->screen, NULL)
        , ini(ini)
    {
        this->screen.add_widget(&this->image);
        this->screen.add_widget(&this->window_login);

        this->window_login.set_xy((width - this->window_login.cx()) / 2,
                                  (height - this->window_login.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        this->window_login.login_edit.set_text(this->ini.account.username);
        this->window_login.password_edit.set_text(this->ini.account.password);

        this->window_login.set_widget_focus(&this->window_login.login_edit);
        if (ini.account.username[0] != 0){
            this->window_login.set_widget_focus(&this->window_login.password_edit);
        }

        this->screen.set_widget_focus(&this->window_login);

        this->screen.refresh(this->screen.rect);

    }

    virtual ~LoginMod()
    {
        this->screen.clear();
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {
        switch (event) {
        case NOTIFY_SUBMIT:
            this->ini.parse_username(this->window_login.login_edit.label.buffer);
            this->ini.context_set_value(AUTHID_PASSWORD, this->window_login.password_edit.get_text());
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

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {
    }

    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }
};

#endif

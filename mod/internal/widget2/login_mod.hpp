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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_MOD_INTERNAL_WIDGET2_LOGIN_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_WIDGET2_LOGIN_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "window_login.hpp"
#include "widget2_image.hpp"
#include "widget2_internal_mod.hpp"

class LoginMod : public InternalMod, public NotifyApi
{
    WindowLogin window_login;
    WidgetImage image;

public:
    Inifile & ini;

public:
    LoginMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    , window_login(this, 0, 0, &this->screen, this, VERSION, 0, 0, 0, BLACK, GREY,
                   ini.translation.button_ok.c_str(),
                   ini.translation.button_cancel.c_str(),
                   ini.translation.button_help.c_str(),
                   ini.translation.login.c_str(),
                   ini.translation.password.c_str())
    , image(this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, &this->screen, NULL)
    , ini(ini)
    {
        this->screen.child_list.push_back(&this->image);
        this->screen.child_list.push_back(&this->window_login);

        this->window_login.set_xy((width - this->window_login.cx()) / 2,
                                  (height - this->window_login.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        if (this->ini.context_is_asked(AUTHID_TARGET_USER)
        ||  this->ini.context_is_asked(AUTHID_TARGET_DEVICE)){
            if (this->ini.context_is_asked(AUTHID_AUTH_USER)){
                this->ini.account.username[0] = 0;
            }
            else {
                strncpy(this->ini.account.username,
                    this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0),
                    sizeof(this->ini.account.username));
                this->ini.account.username[sizeof(this->ini.account.username) - 1] = 0;
            }
        }
        else if (this->ini.context_is_asked(AUTHID_AUTH_USER)) {
            this->ini.account.username[0] = 0;
        }
        else {
            TODO("check this! Assembling parts to get user login with target is not obvious"
                 "method used below il likely to show @: if target fields are empty")
            char buffer[256];
            snprintf( buffer, 256, "%s@%s:%s%s%s"
                    , this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
                    , this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , (this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ?
                           this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0) : "")
                    , (this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ? ":" : "")
                    , this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0)
                    );
            strcpy(this->ini.account.username, buffer);
        }

        if (this->ini.account.password[0]) {
            this->window_login.password_edit.set_text(this->ini.account.username);
        }

        Widget2 * focus_in_window;
        if (this->ini.account.username[0]) {
            focus_in_window = &this->window_login.password_edit;
            this->window_login.login_edit.set_text(this->ini.account.username);
        } else {
            focus_in_window = &this->window_login.login_edit;
        }

        this->window_login.set_widget_focus(focus_in_window);
        this->screen.set_widget_focus(&this->window_login);

        this->screen.refresh(this->screen.rect);
    }

    virtual ~LoginMod()
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        switch (event) {
            case NOTIFY_SUBMIT:
                this->ini.parse_username(this->window_login.login_edit.label.buffer);
                this->ini.context_set_value(AUTHID_PASSWORD, this->window_login.password_edit.buffer);
                this->mod_event(BACK_EVENT_NEXT);
                break;
            case NOTIFY_CANCEL:
                this->mod_event(BACK_EVENT_STOP);
                break;
            default:
                break;
        }
    }

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {}

    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        return this->signal;
    }
};

#endif

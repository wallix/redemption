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
#include "screen.hpp"
#include "window_login.hpp"
#include "image.hpp"
#include "internal_mod.hpp"

class LoginMod : public InternalMod, public NotifyApi
{
    WidgetScreen screen;
    WindowLogin window_login;
    WidgetImage image;

public:
    Inifile & ini;

public:
    LoginMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
    : InternalMod(front, width, height)
    , screen(this, width, height)
    , window_login(this, 0, 0, &this->screen, this, "???")
    , image(this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, &this->screen, NULL)
    , ini(ini)
    {
        this->screen.child_list.push_back(&this->window_login);
        this->screen.child_list.push_back(&this->image);

        this->window_login.set_xy((width - this->window_login.cx()) / 2,
                                  (height - this->window_login.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        const char * auth_user = this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0);

        if (this->ini.context_is_asked(AUTHID_TARGET_USER)
        ||  this->ini.context_is_asked(AUTHID_TARGET_DEVICE)){
            if (*auth_user){
                this->window_login.login_edit.label.set_text(auth_user);
                strncpy(this->ini.account.username, auth_user, sizeof(this->ini.account.username));
            } else {
                this->ini.account.username[0] = 0;
            }
        }
        else if (this->ini.context_is_asked(AUTHID_AUTH_USER)) {
            this->ini.account.username[0] = 0;
        }
        else if (0 == *auth_user) {
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
            this->window_login.login_edit.label.set_text(buffer);
            strncpy(this->ini.account.username, auth_user, sizeof(this->ini.account.username));
        }

        if (this->ini.account.username[0]) {
            this->screen.widget_with_focus = &this->window_login.password_edit;
        } else {
            this->screen.widget_with_focus = &this->window_login.login_edit;
        }

        this->screen.refresh(this->screen.rect);
    }

    virtual ~LoginMod()
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        switch (event) {
            case NOTIFY_SUBMIT:
                if (0 == strcmp(this->window_login.login_edit.label.buffer, this->ini.globals.translation.login)){
                    this->ini.parse_username(this->window_login.login_edit.label.buffer);
                }
                else if (0 == strcmp(this->window_login.password_edit.buffer, this->ini.globals.translation.password)){
                    this->ini.context_set_value(AUTHID_PASSWORD, this->window_login.password_edit.buffer);
                }
                this->mod_event(BACK_EVENT_NEXT);
                break;
            case NOTIFY_CANCEL:
                this->mod_event(BACK_EVENT_STOP);
                break;
            default:
                break;
        }
    }

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->screen.rdp_input_invalidate(r);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {}

    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        return this->signal;
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        TODO("bgcolor <-> fgcolor")
        this->front.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }
};

#endif
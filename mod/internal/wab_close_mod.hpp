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

#ifndef REDEMPTION_MOD_INTERNAL_WAB_CLOSE_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_WAB_CLOSE_MOD_HPP

#include "front_api.hpp"
#include "mod_api.hpp"
#include "widget2/window_wab_close.hpp"
#include "widget2/image.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"


class WabCloseMod : public InternalMod, public NotifyApi
{
    Inifile & ini;
    WindowWabClose window_close;
    WidgetImage image;
    Timeout timeout;

private:
    struct temporary_text {
        char text[255];

        temporary_text(Inifile& ini)
        {
            if (ini.context.selector.get()
                && !ini.context.selector.is_asked()) {
                snprintf(text,sizeof(text),"Selector");
            }
            else {
                if (::strlen(ini.globals.target_application.get_cstr())) {
                    snprintf(text, sizeof(text), "%s",
                             ini.globals.target_application.get_cstr());
                }
                else {
                    snprintf(text, sizeof(text), "%s@%s",
                             ini.globals.target_user.get_cstr(),
                             ini.globals.target_device.get_cstr());
                }
            }
        }
    };

public:
    WabCloseMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height, time_t now)
    : InternalMod(front, width, height)
    , ini(ini)
    , window_close(*this, 0, 0, &this->screen, this,
                   ini.context.auth_error_message.c_str(), 0,
                   (ini.context_is_asked(AUTHID_AUTH_USER)
                    || ini.context_is_asked(AUTHID_TARGET_DEVICE)) ?
                    NULL : ini.globals.auth_user.get_cstr(),
                   (ini.context_is_asked(AUTHID_TARGET_USER)
                    || ini.context_is_asked(AUTHID_TARGET_DEVICE)) ?
                    NULL : temporary_text(ini).text,
                   BLACK, GREY
    )
    , image(*this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, &this->screen, NULL)
    , timeout(Timeout(now, ini.globals.close_timeout))
    {
        LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds", ini.globals.close_timeout);
        this->screen.child_list.push_back(&this->image);
        this->screen.child_list.push_back(&this->window_close);

        this->window_close.set_xy((width - this->window_close.cx()) / 2,
                                  (height - this->window_close.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        this->window_close.set_widget_focus(&this->window_close.cancel);
        this->screen.set_widget_focus(&this->window_close);

        this->screen.refresh(this->screen.rect);
    }

    virtual ~WabCloseMod()
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (NOTIFY_CANCEL == event) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
        }
    }

    virtual void draw_event(time_t now)
    {
        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            this->event.set(1000000);
            break;
        default:
            this->event.reset();
            break;
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0 && keymap->top_kevent() == Keymap2::KEVENT_ESC){
            keymap->get_kevent();
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
        }
        else {
            this->window_close.rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        TODO("bgcolor <-> fgcolor")
        this->front.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }
};

#endif

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
 *
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_WAB_CLOSE_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_WAB_CLOSE_MOD_HPP

#include "front_api.hpp"
#include "mod_api.hpp"
#include "widget2/flat_wab_close.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"

class FlatWabCloseMod : public InternalMod, public NotifyApi
{
    Inifile & ini;
    FlatWabClose close_widget;
    Timeout timeout;

private:
    bool showtimer;
    struct temporary_text {
        char text[255];

        temporary_text(Inifile& ini)
        {
            if (ini.context.selector.get()
                && !ini.context.selector.is_asked()) {
                snprintf(text, sizeof(text), "%s", TR("selector", ini));
            }
            else {
                TODO("target_application only used for user message, the two branches of alternative should be unified et message prepared by sesman")
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
    FlatWabCloseMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height, time_t now, bool showtimer = false)
    : InternalMod(front, width, height)
    , ini(ini)
    , close_widget(*this, width, height, this->screen, this,
                   ini.context.auth_error_message.c_str(), 0,
                   (ini.context_is_asked(AUTHID_AUTH_USER)
                    || ini.context_is_asked(AUTHID_TARGET_DEVICE)) ?
                    NULL : ini.globals.auth_user.get_cstr(),
                   (ini.context_is_asked(AUTHID_TARGET_USER)
                    || ini.context_is_asked(AUTHID_TARGET_DEVICE)) ?
                    NULL : temporary_text(ini).text,
                   WHITE, DARK_BLUE_BIS,
                   showtimer,
                   ini
                   )
    , timeout(Timeout(now, ini.globals.close_timeout))
    , showtimer(showtimer)
    {
        LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds", ini.globals.close_timeout);
        this->screen.add_widget(&this->close_widget);
        this->close_widget.set_widget_focus(&this->close_widget.cancel);
        this->screen.set_widget_focus(&this->close_widget);

        this->screen.refresh(this->screen.rect);
    }

    virtual ~FlatWabCloseMod()
    {
        this->screen.clear();
    }

    virtual void notify(Widget2* sender, notify_event_t event)
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
            if (this->showtimer) {
                this->close_widget.refresh_timeleft(this->timeout.timeleft_sec(now));
            }
            this->event.set(200000);
            break;
        default:
            this->event.reset();
            break;
        }
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        TODO("bgcolor <-> fgcolor");
        this->front.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }
};

#endif

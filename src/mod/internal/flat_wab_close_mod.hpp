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
#include "timeout.hpp"

class FlatWabCloseMod : public InternalMod, public NotifyApi
{
    FlatWabClose     close_widget;
    TimeoutT<time_t> timeout;

private:
    bool showtimer;
    struct temporary_text {
        char text[255];

        explicit temporary_text(Inifile & ini)
        {
            if (ini.get<cfg::context::module>() == "selector") {
                snprintf(text, sizeof(text), "%s", TR("selector", ini));
            }
            else {
                TODO("target_application only used for user message, the two branches of alternative should be unified et message prepared by sesman")
                if (!ini.get<cfg::globals::target_application>().empty()) {
                    snprintf(text, sizeof(text), "%s",
                             ini.get<cfg::globals::target_application>().c_str());
                }
                else {
                    snprintf(text, sizeof(text), "%s@%s",
                             ini.get<cfg::globals::target_user>().c_str(),
                             ini.get<cfg::globals::target_device>().c_str());
                }
            }
        }
    };

public:
    FlatWabCloseMod(Inifile & ini, FrontAPI & front, uint16_t width, uint16_t height, time_t now, bool showtimer = false)
        : InternalMod(front, width, height, ini.get<cfg::font>(), ini.get<cfg::theme>())
        , close_widget(*this, width, height, this->screen, this,
                       ini.get<cfg::context::auth_error_message>().c_str(), 0,
                       (ini.is_asked<cfg::globals::auth_user>()
                        || ini.is_asked<cfg::globals::target_device>()) ?
                       nullptr : ini.get<cfg::globals::auth_user>().c_str(),
                       (ini.is_asked<cfg::globals::auth_user>()
                        || ini.is_asked<cfg::globals::target_device>()) ?
                       nullptr : temporary_text(ini).text,
                       showtimer, ini)
        , timeout(now, ini.get<cfg::globals::close_timeout>())
        , showtimer(showtimer)
    {
        LOG(LOG_INFO, "WabCloseMod: Ending session in %u seconds", ini.get<cfg::globals::close_timeout>());
        this->front.set_mod_palette(BGRPalette::classic_332());

        this->screen.add_widget(&this->close_widget);
        this->close_widget.set_widget_focus(&this->close_widget.cancel, Widget2::focus_reason_tabkey);
        this->screen.set_widget_focus(&this->close_widget, Widget2::focus_reason_tabkey);

        this->screen.refresh(this->screen.rect);
    }

    ~FlatWabCloseMod() override {
        this->screen.clear();
    }

    void notify(Widget2* sender, notify_event_t event) override {
        if (NOTIFY_CANCEL == event) {
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
        }
    }

    void draw_event(time_t now) override {
        switch(this->timeout.check(now)) {
        case TimeoutT<time_t>::TIMEOUT_REACHED:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        case TimeoutT<time_t>::TIMEOUT_NOT_REACHED:
            if (this->showtimer) {
                this->close_widget.refresh_timeleft(this->timeout.timeleft(now));
            }
            this->event.set(200000);
            break;
        default:
            this->event.reset();
            break;
        }
    }
};

#endif

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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_WAIT_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_WAIT_MOD_HPP

#include "translation.hpp"
#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_wait.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"

class FlatWaitMod : public InternalMod, public NotifyApi
{
    FlatWait wait_widget;

    Inifile & ini;
    Timeout timeout;

public:
    FlatWaitMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height,
                  const char * caption, const char * message, time_t now)
        : InternalMod(front, width, height, &ini)
        , wait_widget(*this, width, height, this->screen, this, caption, message,
                      0, ini.theme, true)
        , ini(ini)
        , timeout(Timeout(now, 60))
    {
        this->screen.add_widget(&this->wait_widget);
        if (this->wait_widget.hasform) {
            this->wait_widget.set_widget_focus(&this->wait_widget.form, Widget2::focus_reason_tabkey);
        }
        else {
            this->wait_widget.set_widget_focus(&this->wait_widget.goselector, Widget2::focus_reason_tabkey);
        }
        this->screen.set_widget_focus(&this->wait_widget, Widget2::focus_reason_tabkey);
        this->screen.refresh(this->screen.rect);
    }

    virtual ~FlatWaitMod()
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
        this->ini.context_set_value(AUTHID_DISPLAY_MESSAGE,
                                    "True");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
    void refused()
    {
        this->ini.context_set_value(AUTHID_DISPLAY_MESSAGE,
                                    "False");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    virtual void draw_event(time_t now)
    {
        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->refused();
            break;
        case Timeout::TIMEOUT_NOT_REACHED:
            this->event.set(1000000);
            break;
        default:
            this->event.reset();
            break;
        }
    }

};

#endif

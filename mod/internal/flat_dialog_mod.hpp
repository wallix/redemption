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

#ifndef REDEMPTION_MOD_INTERNAL_FLAT_DIALOG_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_FLAT_DIALOG_MOD_HPP

#include "translation.hpp"
#include "front_api.hpp"
#include "config.hpp"
#include "widget2/flat_dialog.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"

class FlatDialogMod : public InternalMod, public NotifyApi
{
    FlatDialog dialog_widget;

    Inifile & ini;
    Timeout timeout;

public:
    FlatDialogMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height,
                  const char * caption, const char * message, const char * cancel_text, time_t now)
    : InternalMod(front, width, height)
    , dialog_widget(*this, width, height, this->screen, this, caption, message,
                    0, TR("OK", &ini), cancel_text, WHITE, DARK_BLUE_BIS)
    , ini(ini)
    , timeout(Timeout(now, ini.debug.pass_dialog_box))
    {
        this->screen.add_widget(&this->dialog_widget);
        this->dialog_widget.set_widget_focus(&this->dialog_widget.ok);
        this->screen.set_widget_focus(&this->dialog_widget);
        this->screen.refresh(this->screen.rect);
    }

    virtual ~FlatDialogMod()
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
    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod");
    void accepted()
    {
        this->ini.context_set_value(
            (this->dialog_widget.cancel
            ? AUTHID_ACCEPT_MESSAGE : AUTHID_DISPLAY_MESSAGE), "True");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod");
    void refused()
    {
        this->ini.context_set_value(
            (this->dialog_widget.cancel
            ? AUTHID_ACCEPT_MESSAGE : AUTHID_DISPLAY_MESSAGE), "False");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    virtual void draw_event(time_t now)
    {
        switch(this->timeout.check(now)) {
        case Timeout::TIMEOUT_REACHED:
            this->accepted();
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

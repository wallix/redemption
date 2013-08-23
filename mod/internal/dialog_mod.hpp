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

#ifndef REDEMPTION_MOD_INTERNAL_DIALOG_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_DIALOG_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "widget2/window_dialog.hpp"
#include "widget2/msgbox.hpp"
#include "widget2/screen.hpp"
#include "internal_mod.hpp"

class DialogMod : public InternalMod, public NotifyApi
{
    WindowDialog window_dialog;

    Inifile & ini;
    Timeout timeout;
    // time_t timeout;

public:
    DialogMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height,
              const char * caption, const char * message, const char * cancel_text, time_t now)
    : InternalMod(front, width, height)
    , window_dialog(*this, 0, 0, &this->screen, this, caption, message, 0, "Ok", cancel_text, BLACK, GREY, BLACK, GREY)
    , ini(ini)
    // , timeout(ini.debug.pass_dialog_box?(now + ini.debug.pass_dialog_box):0)
    , timeout(Timeout(now, ini.debug.pass_dialog_box))
    {
        this->screen.child_list.push_back(&this->window_dialog);

        this->window_dialog.ok.border_top_left_color = WHITE;
        if (this->window_dialog.cancel) {
            this->window_dialog.cancel->border_top_left_color = WHITE;
        }

        this->window_dialog.set_xy((width - this->window_dialog.cx()) / 2,
                                   (height - this->window_dialog.cy()) / 2);

        this->window_dialog.set_widget_focus(&this->window_dialog.ok);
        this->screen.set_widget_focus(&this->window_dialog);
        this->screen.refresh(this->screen.rect);
    }

    virtual ~DialogMod()
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            default: ;
        }
    }

private:
    void accepted()
    {
        TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
        this->ini.context_set_value(
            (this->window_dialog.cancel
            ? AUTHID_ACCEPT_MESSAGE
            : AUTHID_DISPLAY_MESSAGE),
            "True");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    void refused()
    {
        TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod")
        this->ini.context_set_value(
            (this->window_dialog.cancel
            ? AUTHID_ACCEPT_MESSAGE
            : AUTHID_DISPLAY_MESSAGE),
            "False");
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    virtual void draw_event()
    {
        // if (this->timeout) {
        //     if (this->now > this->timeout) {
        //         this->accepted();
        //     }
        //     else {
        //         this->event.set(1000000);
        //     }
        // }
        // else {
        //     this->event.reset();
        // }
        switch(this->timeout.check(this->now)) {
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

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

};

#endif

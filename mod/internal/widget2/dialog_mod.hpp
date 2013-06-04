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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_MOD_INTERNAL_WIDGET2_DIALOG_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_WIDGET2_DIALOG_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "window_dialog.hpp"
#include "msgbox.hpp"
#include "widget2_screen.hpp"
#include "widget2_internal_mod.hpp"

class DialogMod : public InternalMod, public NotifyApi
{
    WidgetScreen screen;
    WindowDialog window_dialog;

    Inifile & ini;

public:
    DialogMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height,
              const char * caption, const char * message, const char * cancel_text)
    : InternalMod(front, width, height)
    , screen(this, width, height)
    , window_dialog(this, 0, 0, &this->screen, this, caption, message, 0, "Ok", cancel_text, BLACK, GREY, BLACK, WHITE)
    , ini(ini)
    {
        this->screen.child_list.push_back(&this->window_dialog);

        this->window_dialog.set_xy((width - this->window_dialog.cx()) / 2,
                                   (height - this->window_dialog.cy()) / 2);

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
        this->ini.context_set_value(
            (this->window_dialog.cancel
            ? AUTHID_ACCEPT_MESSAGE
            : AUTHID_DISPLAY_MESSAGE),
            "True");
        this->signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    void refused()
    {
        this->ini.context_set_value(
            (this->window_dialog.cancel
            ? AUTHID_ACCEPT_MESSAGE
            : AUTHID_DISPLAY_MESSAGE),
            "False");
        this->signal = BACK_EVENT_NEXT;
        this->event.set();
    }

public:
    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        return this->signal;
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->window_dialog.rdp_input_invalidate(r);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_ESC:
                case Keymap2::KEVENT_RIGHT_ARROW:
                    keymap->get_kevent();
                    this->refused();
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->accepted();
                    break;
                default:
                    break;
            }
        }
        (void)param1;
        (void)param2;
        (void)param3;
        (void)param4;
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        TODO("bgcolor <-> fgcolor")
        this->front.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }
};

#endif

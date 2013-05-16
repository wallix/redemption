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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP

// #include "widget.hpp"
#include "front.hpp"
#include "mod_api.hpp"
#include "internal/internal_mod.hpp"
#include "window_wab_close.hpp"
#include "image.hpp"
#include "screen.hpp"
// #include "../close.hpp"

//TODO("Old CloseMod Test")
TODO("CloseMod")


class widget2_mod : public internal_mod, public NotifyApi
{
    WidgetScreen screen;
    WindowWabClose window_close;
    WidgetImage image;

    ModContext & context;
    Inifile & ini;


private:
    struct temporary_text {
        char text[255];

        temporary_text(Inifile& ini)
        {
            snprintf(text, sizeof(text), "%s@%s",
                     ini.globals.target_user,
                     ini.globals.target_device);
        }
    };

public:
    widget2_mod(ModContext& context, Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
    : internal_mod(front, width, height)
    , screen(this, width, height)
    , window_close(this, 0, 0, &this->screen, this, "End of connection", 0,
                   context.is_asked(_STRAUTHID_AUTH_USER) ? NULL : ini.globals.auth_user,
                   context.is_asked(_STRAUTHID_TARGET_USER) ||context.is_asked(_STRAUTHID_TARGET_DEVICE) ? NULL : temporary_text(ini).text,
                   BLACK, GREY
                  )
    , image(this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, &this->screen, NULL)
    , context(context)
    , ini(ini)
    {
        this->screen.child_list.push_back(&this->window_close);
        this->screen.child_list.push_back(&this->image);

        this->screen.widget_with_focus = &this->window_close;

        this->window_close.set_xy((width - this->window_close.cx()) / 2,
                                  (height - this->window_close.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        this->screen.refresh(this->screen.rect);
    }

    virtual ~widget2_mod()
    {}

    virtual void notify(Widget2* sender, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (event == NOTIFY_CANCEL || (event == NOTIFY_SUBMIT && sender == &window_close)) {
            this->signal = BACK_EVENT_STOP;
            this->event.set();
        }
    }

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
        this->window_close.rdp_input_invalidate(r);
        //this->image.rdp_input_invalidate(r.intersect(this->image));
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0 && keymap->top_kevent() == Keymap2::KEVENT_ENTER){
            keymap->get_kevent();
            this->signal = BACK_EVENT_STOP;
            this->event.set();
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
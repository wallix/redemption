/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen, Raphael Zhou
*/

#ifndef REDEMPTION_MOD_INTERNAL_WIDGETTESTMOD_HPP
#define REDEMPTION_MOD_INTERNAL_WIDGETTESTMOD_HPP

//#include "translation.hpp"
#include "front_api.hpp"
#include "config.hpp"
//#include "widget2/flat_dialog.hpp"
//#include "widget2/screen.hpp"
#include "internal_mod.hpp"
#include "widget2/tab.hpp"

class WidgetTestMod : public InternalMod, public NotifyApi
{
    Inifile & ini;

    WidgetTabDPDefault drawing_policy;
    WidgetTab          tab;

public:
    WidgetTestMod(Inifile & ini, FrontAPI & front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, &ini)
        , ini(ini)
        , drawing_policy(*this)
        , tab(*this, drawing_policy, 0, 0, width, height, this->screen, this, 0, ini.theme.global.fgcolor, ini.theme.global.bgcolor)
    {
/*
        this->screen.add_widget(&this->dialog_widget);
        this->dialog_widget.set_widget_focus(&this->dialog_widget.ok);
        this->screen.set_widget_focus(&this->dialog_widget);
*/

        size_t tab_0_index = static_cast<size_t>(-1);
        size_t tab_1_index = static_cast<size_t>(-1);

        tab_0_index = this->tab.add_item("First tab");
        tab_1_index = this->tab.add_item("Second tab");

//        this->tab.set_current_item(tab_0_index);

        this->screen.add_widget(&this->tab);

        //this->screen.refresh(this->screen.rect);

LOG(LOG_INFO, "WidgetTestMod: rdp_input_invalidate, x=%u y=%u cx=%u cy=%u",
    this->screen.rect.x, this->screen.rect.y, this->screen.rect.cx, this->screen.rect.cy);
        this->screen.rdp_input_invalidate(this->screen.rect);
    this->begin_update();
    this->end_update();

/*
        if (this->dialog_widget.challenge) {
            this->dialog_widget.set_widget_focus(this->dialog_widget.challenge);
            // this->ini.to_send_set.insert(AUTHID_AUTHENTICATION_CHALLENGE);
        }
*/
    }

    virtual ~WidgetTestMod()
    {
        this->screen.clear();
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {
/*
        switch (event) {
            case NOTIFY_SUBMIT: this->accepted(); break;
            case NOTIFY_CANCEL: this->refused(); break;
            default: ;
        }
*/
    }

/*
private:
    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod");
    void accepted()
    {
        if (this->dialog_widget.challenge) {
            this->ini.context_set_value(AUTHID_PASSWORD,
                                        this->dialog_widget.challenge->get_text());
        }
        else {
            this->ini.context_set_value((this->dialog_widget.cancel
                                         ? AUTHID_ACCEPT_MESSAGE
                                         : AUTHID_DISPLAY_MESSAGE),
                                        "True");
        }
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }

    TODO("ugly. The value should be pulled by authentifier when module is closed instead of being pushed to it by mod");
    void refused()
    {
        if (!this->dialog_widget.challenge) {
            this->ini.context_set_value((this->dialog_widget.cancel
                                         ? AUTHID_ACCEPT_MESSAGE
                                         : AUTHID_DISPLAY_MESSAGE),
                                        "False");
        }
        this->event.signal = BACK_EVENT_NEXT;
        this->event.set();
    }
*/

public:
    virtual void draw_event(time_t now)
    {
/*
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
*/
    }
};

#endif  // #ifndef REDEMPTION_MOD_INTERNAL_WIDGETTESTMOD_HPP

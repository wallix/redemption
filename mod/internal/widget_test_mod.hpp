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
#include "widget2/edit.hpp"
#include "widget2/flat_button.hpp"
#include "widget2/tab.hpp"

class WidgetTestMod : public InternalMod, public NotifyApi
{
    Inifile & ini;

    WidgetTabDPDefault drawing_policy;
    WidgetTab          tab;

    WidgetEdit        * wedit_on_first_tab;
    WidgetFlatButton  * wbutton_on_first_tab;

    WidgetEdit        * wedit_on_screen;

public:
    WidgetTestMod(Inifile & ini, FrontAPI & front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height, &ini)
        , ini(ini)
        , drawing_policy(*this)
        , tab(*this, drawing_policy, 30, 30, width - 60, height - 260, this->screen, this, 0, ini.theme.global.fgcolor, ini.theme.global.bgcolor)
        , wedit_on_first_tab(NULL)
        , wbutton_on_first_tab(NULL)
    {
        this->screen.add_widget(&this->tab);

        size_t tab_0_index = static_cast<size_t>(-1);
        size_t tab_1_index = static_cast<size_t>(-1);

        tab_0_index = this->tab.add_item("First tab");
        tab_1_index = this->tab.add_item("Second tab");
(void)tab_1_index;


        NotifyApi * notifier = NULL;
        int         group_id = 0;
        int         fg_color = RED;
        int         bg_color = YELLOW;

        bool auto_resize = true;
        int  focuscolor  = LIGHT_YELLOW;
        int  xtext       = 4;
        int  ytext       = 1;

        WidgetParent & parent_item = this->tab.get_item(tab_0_index);

        notifier  = &parent_item;

        this->wedit_on_first_tab = new WidgetEdit(*this, 11, 20, 30, parent_item,
            notifier, "", group_id, BLACK, WHITE, WHITE);
        this->tab.add_widget(tab_0_index, wedit_on_first_tab);

        this->wbutton_on_first_tab = new WidgetFlatButton(*this, 10, 42, parent_item,
            notifier, "Button on First tab", auto_resize, group_id, fg_color, bg_color,
            focuscolor, xtext, ytext);
        this->tab.add_widget(tab_0_index, wbutton_on_first_tab);


        this->wedit_on_screen = new WidgetEdit(*this, 11, 620, 30, this->screen,
            this, "", group_id, BLACK, WHITE, WHITE);
        this->screen.add_widget(this->wedit_on_screen);

        this->screen.set_widget_focus(&this->tab, Widget2::focus_reason_tabkey);

        WidgetParent & wp = this->tab.get_item(tab_0_index);
        wp.set_widget_focus(this->wedit_on_first_tab, Widget2::focus_reason_tabkey);
        this->tab.child_has_focus = true;

        this->screen.refresh(this->screen.rect);
    }

    virtual ~WidgetTestMod()
    {
        this->screen.clear();

        delete this->wedit_on_screen;

        delete this->wbutton_on_first_tab;
        delete this->wedit_on_first_tab;
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
        this->event.reset();
    }
};

#endif  // #ifndef REDEMPTION_MOD_INTERNAL_WIDGETTESTMOD_HPP

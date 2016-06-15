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

#include "internal_mod.hpp"

#include "widget2/flat_button.hpp"


class LanguagePicker : public NotifyApi
{
    std::vector<std::string> texts = {"fr", "encffvg", "ru"};
    int selected_index = 0;
    WidgetLabel * label = nullptr;

public:
    LanguagePicker() = default;

    void attach_label(WidgetLabel * label) {
        if ((this->label = label)) {
            this->selected_index = 0;
            this->label->set_text(this->texts[this->selected_index].c_str());
        }
    }

    void notify(Widget2 * sender, notify_event_t event) override {
        if (event == NOTIFY_SUBMIT) {
            this->selected_index = (this->selected_index +1) % texts.size();
            if (this->label) {
                this->label->set_text(this->texts[this->selected_index].c_str());
            }
        }
    }


};


//void set_text(const char * text)


class WidgetTestMod : public InternalMod, public NotifyApi {

    LanguagePicker language_picker;
    WidgetFlatButton wbutton_selector_language;

public:
    WidgetTestMod(FrontAPI & front, uint16_t width, uint16_t height, Font const & font, Theme const & theme)
    : InternalMod(front, width, height, font, theme)
    , wbutton_selector_language(front, 300, 300, this->screen, &this->language_picker, "ab", true, 5, YELLOW, RED, GREEN, font){
        this->screen.add_widget(&this->wbutton_selector_language);
        this->language_picker.attach_label(&this->wbutton_selector_language.label);
        this->screen.set_widget_focus(&this->wbutton_selector_language, Widget2::focus_reason_tabkey);
        this->screen.refresh(this->screen.rect);
    }

    ~WidgetTestMod() override {
        this->screen.clear();


    }

    void notify(Widget2 * sender, notify_event_t event) override {}

public:
    void draw_event(time_t now, gdi::GraphicApi & drawable) override {
        this->event.reset();
    }
};

#endif  // #ifndef REDEMPTION_MOD_INTERNAL_WIDGETTESTMOD_HPP

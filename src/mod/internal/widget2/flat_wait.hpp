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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#pragma once

#include "composite.hpp"
#include "flat_button.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "widget2_rect.hpp"
#include "utils/theme.hpp"
#include "group_box.hpp"
#include "flat_form.hpp"
#include "utils/translation.hpp"
#include "flat_button.hpp"
#include "gdi/graphic_api.hpp"

class FlatWait : public WidgetParent
{
private:
    CompositeArray composite_array;

    WidgetGroupBox     groupbox;
    WidgetMultiLine    dialog;
public:
    FlatForm           form;
    WidgetFlatButton   goselector;
private:
    WidgetFlatButton   exit;
    WidgetFlatButton * extra_button;

public:
    bool hasform;

private:
    BGRColor_ bg_color;

public:
    FlatWait(gdi::GraphicApi & drawable, int16_t left, int16_t top, int16_t width, int16_t height,
             Widget2 & parent, NotifyApi* notifier,
             const char* caption, const char * text, int group_id,
             WidgetFlatButton * extra_button,
             Font const & font, Theme const & theme, Translation::language_t lang,
             bool showform = false, int required = FlatForm::NONE)
        : WidgetParent(drawable, parent, notifier, group_id)
        , groupbox(drawable, *this, nullptr, caption,
                   theme.global.fgcolor, theme.global.bgcolor, font)
        , dialog(drawable, this->groupbox, nullptr, text, -10,
                 theme.global.fgcolor, theme.global.bgcolor, font, 10, 2)
        , form(drawable, *this, this, -20, font, theme, lang, required)
        , goselector(drawable, this->groupbox, this, TR("back_selector", lang), -12,
                     theme.global.fgcolor, theme.global.bgcolor,
                     theme.global.focus_color, 2, font, 6, 2)
        , exit(drawable, this->groupbox, this, TR("exit", lang), -11,
               theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, 2, font,
               6, 2)
        , extra_button(extra_button)
        , hasform(showform)
        , bg_color(theme.global.bgcolor)
    {
        this->impl = &composite_array;

        this->groupbox.add_widget(&this->dialog);

        if (showform) {
            this->groupbox.add_widget(&this->form);
        }

        this->groupbox.add_widget(&this->goselector);
        this->groupbox.add_widget(&this->exit);

        this->add_widget(&this->groupbox);

        if (extra_button) {
            this->add_widget(extra_button);
        }

        this->move_size_widget(left, top, width, height);
    }

    ~FlatWait() override {
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_xy(left, top);
        this->set_wh(width, height);

        this->groupbox.set_xy(left, top);
        this->groupbox.set_wh(width, height);

        if (this->hasform) {
            this->form.set_wh(width - 80, 150);
            this->form.set_xy(left, top);
        }

        int y = 20;

        Dimension dim = this->dialog.get_optimal_dim();
        this->dialog.set_wh(dim);
        this->dialog.set_xy(left + 30, top + y + 10);

        y = this->dialog.y() + this->dialog.cy() + 20;

        if (this->hasform) {
            this->form.move_size_widget(left + 40, y, this->form.cx(), this->form.cy());

            y = this->form.bottom() + 10;
        }

        dim = this->exit.get_optimal_dim();
        this->exit.set_wh(dim);
        this->exit.set_xy(left + width - 40 - this->exit.cx(), y);

        dim = this->goselector.get_optimal_dim();
        this->goselector.set_wh(dim);
        this->goselector.set_xy(this->exit.x() - (this->goselector.cx() + 10), y);

        y += this->goselector.cy() + 20;

        this->groupbox.set_wh(this->groupbox.cx(), y - top);
        this->groupbox.move_xy(0, (height - (y - top)) / 2);

        if (this->extra_button) {
            this->extra_button->set_xy(left + 60, top + height - 60);
        }
    }

    BGRColor_ get_bg_color() const override {
        return this->bg_color;
    }

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
        if ((event == NOTIFY_CANCEL) ||
            ((event == NOTIFY_SUBMIT) && (widget == &this->exit))) {
            this->send_notify(NOTIFY_CANCEL);
        }
        else if ((event == NOTIFY_SUBMIT) && (widget == &this->goselector)) {
            this->send_notify(NOTIFY_SUBMIT);
        }
        else if ((event == NOTIFY_SUBMIT) && (widget->group_id == this->form.group_id)) {
            this->send_notify(NOTIFY_TEXT_CHANGED);
        }
        else if (NOTIFY_COPY == event || NOTIFY_CUT == event || NOTIFY_PASTE == event) {
            if (this->notifier) {
                this->notifier->notify(widget, event);
            }
        }
        else {
            WidgetParent::notify(widget, event);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_ESC:
                keymap->get_kevent();
                this->send_notify(NOTIFY_SUBMIT);
                break;
            default:
                WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
    }
};

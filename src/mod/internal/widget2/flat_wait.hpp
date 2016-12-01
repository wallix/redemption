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
public:

    WidgetGroupBox groupbox;
    int bg_color;

    WidgetMultiLine dialog;
    FlatForm form;
    WidgetFlatButton goselector;
    WidgetFlatButton exit;
    bool hasform;
    CompositeArray composite_array;

private:
    WidgetFlatButton * extra_button;

public:
    FlatWait(gdi::GraphicApi & drawable, int16_t left, int16_t top, int16_t width, int16_t height,
             Widget2 & parent, NotifyApi* notifier,
             const char* caption, const char * text, int group_id,
             WidgetFlatButton * extra_button,
             Font const & font, Theme const & theme, Translation::language_t lang,
             bool showform = false, int required = FlatForm::NONE)
        : WidgetParent(drawable, Rect(left, top, width, height), parent, notifier, group_id)
        , groupbox(drawable, 0, 0, width, height, *this, nullptr, caption,
                   theme.global.fgcolor, theme.global.bgcolor, font)
        , bg_color(theme.global.bgcolor)
        , dialog(drawable, 0, 0, this->groupbox, nullptr, text, true, -10,
                 theme.global.fgcolor, theme.global.bgcolor, font, 10, 2)
        , form(drawable, left, top, width - 80, 150, *this, this, -20, font, theme, lang, required)
        , goselector(drawable, 0, 0, this->groupbox, this, TR("back_selector", lang), true, -12,
                     theme.global.fgcolor, theme.global.bgcolor,
                     theme.global.focus_color, font, 6, 2)
        , exit(drawable, 0, 0, this->groupbox, this, TR("exit", lang), true, -11,
               theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, font,
               6, 2)
        , hasform(showform)
        , extra_button(extra_button)
    {
        this->impl = &composite_array;
        // this->groupbox.add_widget(&this->title);
        this->groupbox.add_widget(&this->dialog);

        // int total_width = this->dialog.cx();
        // total_width = std::min(width + total_width, 2 * width);
        // total_width = total_width / 2;
        // int total_width = width - 80;
        // int total_height = this->dialog.cy() + this->goselector.cy() + 20;
        // if (showform) {
        //     total_height += this->form.cy();
        // }
        // int starty = (height - total_height) / 2;
        const int starty = 20;
        int y = starty;
        this->dialog.set_x(left + 30); // dialog has 10 margin.
        // this->dialog.rect.x = (this->cx() - total_width) / 2;
        this->dialog.set_y(top + y + 10);

        y = this->dialog.y() + this->dialog.cy() + 20;

        if (showform) {
            this->groupbox.add_widget(&this->form);
            this->form.move_xy(40, y - top);
            y = this->form.bottom() + 10;
        }

        this->groupbox.add_widget(&this->goselector);
        this->groupbox.add_widget(&this->exit);

        this->exit.set_x(left + width - 40 - this->exit.cx());
        this->goselector.set_x(this->exit.x() - (this->goselector.cx() + 10));

        this->goselector.set_y(y);
        this->exit.set_y(y);

        y += this->goselector.cy() + 20;
        this->groupbox.set_cy(y - top);
        this->groupbox.move_xy(0, (height - (y - top)) / 2);
        this->add_widget(&this->groupbox);

        if (extra_button) {
            this->add_widget(extra_button);
            extra_button->set_x(left + 60);
            extra_button->set_y(top + height - 60);
        }
    }

    ~FlatWait() override {
        this->clear();
    }

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) {
        this->set_x(left);
        this->set_y(top);
        this->set_cx(width);
        this->set_cy(height);

        this->groupbox.set_x(left);
        this->groupbox.set_y(top);
        this->groupbox.set_cx(width);
        this->groupbox.set_cy(height);

        this->dialog.set_x(left);
        this->dialog.set_y(top);

        this->form.set_x(left);
        this->form.set_y(top);
        this->form.set_cx(width - 80);

        const int starty = 20;
        int y = starty;
        this->dialog.set_x(left + 30); // dialog has 10 margin.
        // this->dialog.rect.x = (this->cx() - total_width) / 2;
        this->dialog.set_y(top + y + 10);

        y = this->dialog.y() + this->dialog.cy() + 20;

        if (this->hasform) {
            this->form.move_size_widget(left + 40, y, this->form.cx(), this->form.cy());
            y = this->form.bottom() + 10;
        }

        this->exit.set_x(left + width - 40 - this->exit.cx());
        this->goselector.set_x(this->exit.x() - (this->goselector.cx() + 10));

        this->goselector.set_y(y);
        this->exit.set_y(y);

        y += this->goselector.cy() + 20;
        this->groupbox.set_cy(y - top);
        this->groupbox.move_xy(0, (height - (y - top)) / 2);

        if (this->extra_button) {
            this->extra_button->set_x(left + 60);
            this->extra_button->set_y(top + height - 60);
        }
    }

    int get_bg_color() const override {
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



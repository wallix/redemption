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
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_WAIT_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_WAIT_HPP

#include "composite.hpp"
#include "flat_button.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "widget2_rect.hpp"
#include "theme.hpp"
#include "group_box.hpp"
#include "flat_form.hpp"
#include "translation.hpp"

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

    FlatWait(DrawApi& drawable, int16_t width, int16_t height,
             Widget2 & parent, NotifyApi* notifier,
             const char* caption, const char * text, int group_id, Inifile & ini,
             Theme & theme, bool showform = false, int required = FlatForm::NONE)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier, group_id)
        , groupbox(drawable, 0, 0, width, height, *this, NULL, caption, -6,
                   theme.global.fgcolor, theme.global.bgcolor, ini.font)
        , bg_color(theme.global.bgcolor)
        , dialog(drawable, 0, 0, this->groupbox, NULL, text, true, -10,
                 theme.global.fgcolor, theme.global.bgcolor, ini.font, 10, 2)
        , form(drawable, width - 80, 150, *this, this, -20, ini, theme, required)
        , goselector(drawable, 0, 0, this->groupbox, this, TR("back_selector", ini), true, -12,
                     theme.global.fgcolor, theme.global.bgcolor,
                     theme.global.focus_color, ini.font, 6, 2)
        , exit(drawable, 0, 0, this->groupbox, this, TR("exit", ini), true, -11,
               theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, ini.font,
               6, 2)
        , hasform(showform)
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
        int starty = 20;
        int y = starty;
        this->dialog.rect.x = 30; // dialog has 10 margin.
        // this->dialog.rect.x = (this->cx() - total_width) / 2;
        this->dialog.rect.y = y + 10;

        y = this->dialog.dy() + this->dialog.cy() + 20;

        if (showform) {
            this->groupbox.add_widget(&this->form);
            this->form.move_xy(40, y);
            y = this->form.ly() + 10;
        }

        this->groupbox.add_widget(&this->goselector);
        this->groupbox.add_widget(&this->exit);

        this->exit.set_button_x(width - 40 - this->exit.cx());
        this->goselector.set_button_x(this->exit.dx() - (this->goselector.cx() + 10));

        this->goselector.set_button_y(y);
        this->exit.set_button_y(y);

        y += this->goselector.cy() + 20;
        this->groupbox.rect.cy = y;
        this->groupbox.move_xy(0, (height - y) / 2);
        this->add_widget(&this->groupbox);
    }

    virtual ~FlatWait() {
        this->clear();
    }

    virtual int get_bg_color() const {
        return this->bg_color;
    }

    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event) {
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
        else {
            WidgetParent::notify(widget, event);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
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

#endif

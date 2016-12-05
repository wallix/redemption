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
 *   Author(s): Christophe Grosjean, Raphael Zhou
 */

#pragma once

#include "edit.hpp"
#include "edit_valid.hpp"
#include "label.hpp"
#include "password.hpp"
#include "multiline.hpp"
#include "image.hpp"
#include "composite.hpp"
#include "flat_button.hpp"
#include "utils/translation.hpp"
#include "utils/theme.hpp"
#include "core/defines.hpp"
#include "gdi/graphic_api.hpp"

class FlatVNCAuthentification : public WidgetParent
{
public:
    WidgetLabel     message_label;
    WidgetLabel     password_label;
    WidgetEditValid password_edit;
    WidgetImage     img;

    int fgcolor;
    int bgcolor;

    CompositeArray composite_array;

    FlatVNCAuthentification(gdi::GraphicApi & drawable, uint16_t width, uint16_t height,
                            Widget2 & parent, NotifyApi* notifier, const char * password,
                            Theme const & theme, const char * label_text_message,
                            const char * label_text_password, Font const & font)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , message_label(drawable, 0, 0, *this, nullptr, label_text_message, true, -13,
                        theme.global.fgcolor, theme.global.bgcolor, font)
        , password_label(drawable, 0, 0, *this, nullptr, label_text_password, true, -13,
                         theme.global.fgcolor, theme.global.bgcolor, font)
        , password_edit(drawable, 0, 0, 400, *this, this, password, -14,
                        theme.edit.fgcolor, theme.edit.bgcolor,
                        theme.edit.focus_color, font, nullptr, false, -1u, 1, 1, true)
        // , img(drawable, 0, 0, theme.global.logo_path, *this, nullptr, -10)
        , img(drawable, 0, 0,
              theme.global.logo ? theme.global.logo_path :
              SHARE_PATH "/" LOGIN_WAB_BLUE, *this, nullptr, -10)
        , fgcolor(theme.global.fgcolor)
        , bgcolor(theme.global.bgcolor)
    {
        this->impl = &composite_array;

        this->add_widget(&this->img);
        this->add_widget(&this->message_label);
        this->add_widget(&this->password_label);
        this->add_widget(&this->password_edit);

        // Center bloc positionning
        // Login and Password boxes
        int cbloc_w = std::max<int>(this->message_label.cx(),
            this->password_label.cx() + this->password_edit.cx() + 10);
        int cbloc_h = this->message_label.cy() +
            std::max(this->password_label.cy(), this->password_edit.cy()) + 20;

        int x_cbloc = (width  - cbloc_w) / 2;
        int y_cbloc = (height - cbloc_h) / 3;

        this->message_label.set_xy((width  - this->message_label.cx()) / 2, y_cbloc);

        this->password_label.set_xy(x_cbloc, y_cbloc + this->message_label.cy() + 20);
        this->password_edit.set_xy(x_cbloc + this->password_label.cx() + 10,
            y_cbloc + this->message_label.cy() + 20);

        this->password_label.set_y(this->password_label.y() + (this->password_edit.cy() - this->password_label.cy()) / 2);

        int bottom_height = (height - cbloc_h) / 2;
        int bbloc_h = this->img.cy()/* + 10 + this->version_label.rect.cy*/;
        int y_bbloc = ((bbloc_h + 10) > (bottom_height / 2))
            ?(height - (bbloc_h + 10))
            :(height/2 + cbloc_h/2 + bottom_height/2);
        this->img.set_xy((width - this->img.cx()) / 2, y_bbloc);
        if (this->img.y() + this->img.cy() > height) {
            this->img.set_y(0);
        }
    }

    ~FlatVNCAuthentification() override {
        this->clear();
    }

    int get_bg_color() const override {
        return this->bgcolor;
    }

    void notify(Widget2* widget, NotifyApi::notify_event_t event) override {
        if ((widget == &this->password_edit)
             && event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_SUBMIT);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
            case Keymap2::KEVENT_ESC:
                keymap->get_kevent();
                this->send_notify(NOTIFY_CANCEL);
                break;
            default:
                WidgetParent::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
    }
};



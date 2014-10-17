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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_INTERACTIVE_PASSWORD_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_FLAT_INTERACTIVE_PASSWORD_HPP

#include "edit.hpp"
#include "edit_valid.hpp"
#include "label.hpp"
#include "password.hpp"
#include "multiline.hpp"
#include "widget2_rect.hpp"
#include "composite.hpp"
#include "flat_button.hpp"
#include "translation.hpp"
#include "theme.hpp"


class FlatInteractivePassword : public WidgetParent
{
public:
    WidgetLabel     caption_label;
    WidgetLabel     login_label;
    WidgetLabel     login;
    WidgetLabel     password_label;
    WidgetEditValid password_edit;
    WidgetRect separator;

    int fgcolor;
    int bgcolor;

    CompositeArray composite_array;

    FlatInteractivePassword(DrawApi& drawable, uint16_t width, uint16_t height,
                            Widget2 & parent, NotifyApi* notifier,
                            int group_id, const char * password,
                            Theme & theme, const char* caption,
                            const char * text_login,
                            const char * login_str,
                            const char * text_password)
        : WidgetParent(drawable, Rect(0, 0, width, height), parent, notifier)
        , caption_label(drawable, 0, 0, *this, NULL, caption, true, -13,
                        theme.global.fgcolor, theme.global.bgcolor)
        , login_label(drawable, 0, 0, *this, NULL, text_login, true, -13,
                      theme.global.fgcolor, theme.global.bgcolor)
        , login(drawable, 0, 0, *this, NULL, login_str, true, -13,
                theme.global.fgcolor, theme.global.bgcolor)
        , password_label(drawable, 0, 0, *this, NULL, text_password, true, -13,
                         theme.global.fgcolor, theme.global.bgcolor)
        , password_edit(drawable, 0, 0, 400, *this, this, password, -14,
                        theme.edit.fgcolor, theme.edit.bgcolor,
                        theme.edit.focus_color, -1u, 1, 1, true)
        , separator(drawable, Rect(0, 0, width, 2), *this, this, -12,
                    theme.global.separator_color)
        , fgcolor(theme.global.fgcolor)
        , bgcolor(theme.global.bgcolor)
    {
        this->impl = &composite_array;

        this->add_widget(&this->caption_label);
        this->add_widget(&this->login_label);
        this->add_widget(&this->login);
        this->add_widget(&this->password_label);
        this->add_widget(&this->password_edit);
        this->add_widget(&this->separator);

        // Center bloc positionning
        // Login and Password boxes
        int cbloc_w = std::max<int>(this->caption_label.rect.cx,
            this->password_label.rect.cx + this->password_edit.rect.cx + 20);
        cbloc_w = std::max<int>(cbloc_w,
                                this->login_label.rect.cx + this->login.rect.cx + 20);
        int cbloc_h = this->login_label.rect.cy + this->caption_label.rect.cy + 20 + 30 +
            std::max(this->password_label.rect.cy, this->password_edit.rect.cy) + 20;

        int x_cbloc = (width  - cbloc_w) / 2;
        int y_cbloc = (height - cbloc_h) / 3;

        int margin_w = std::max<int>(this->password_label.rect.cx,
                                     this->login_label.rect.cx);

        this->caption_label.set_xy((width - this->caption_label.rect.cx) / 2, y_cbloc);
        this->separator.rect.cx = cbloc_w;

        this->separator.set_xy(x_cbloc, this->caption_label.ly() + 20);

        this->login_label.set_xy(x_cbloc,
                                 this->separator.ly() + 20);
        this->login.set_xy(x_cbloc + margin_w + 20,
                           this->separator.ly() + 20);
        this->password_label.set_xy(x_cbloc,
                                    this->login_label.ly() + 20);
        this->password_edit.set_xy(x_cbloc + margin_w + 20,
                                   this->login_label.ly() + 20);

        this->password_label.rect.y += (this->password_edit.cy() - this->password_label.cy()) / 2;
    }

    virtual ~FlatInteractivePassword()
    {
        this->clear();
    }

    virtual int get_bg_color() const {
        return this->bgcolor;
    }

    virtual void notify(Widget2* widget, NotifyApi::notify_event_t event)
    {
        if ((widget == &this->password_edit)
             && event == NOTIFY_SUBMIT) {
            this->send_notify(NOTIFY_SUBMIT);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
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

#endif

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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WINDOW_LOGIN_HPP

#include "edit.hpp"
#include "label.hpp"
#include "button.hpp"
#include "password.hpp"
#include "window.hpp"

class WindowLogin : public Window
{
public:
    WidgetEdit login_edit;
    WidgetLabel login_label;
    WidgetPassword password_edit;
    WidgetLabel password_label;
    WidgetButton ok;

    WindowLogin(ModApi* drawable, int16_t x, int16_t y, Widget* parent, NotifyApi* notifier, const char* caption, int id = 0, int bgcolor = DARK_WABGREEN)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, id)
    , login_edit(drawable, 0, 0, 100, this, NULL, NULL, -10, WHITE, BLACK)
    , login_label(drawable, 10, 0, this, NULL, "login:", true, -11, WHITE, BLACK)
    , password_edit(drawable, 0, 0, 100, this, NULL, NULL, -12, WHITE, BLACK)
    , password_label(drawable, 10, 0, this, NULL, "password:", true, -13, WHITE, BLACK)
    , ok(drawable, 0, 0, this, this, "Ok", true, -14, WHITE, BLACK, 4)
    {
        this->child_list.push_back(&this->login_edit);
        this->child_list.push_back(&this->login_label);
        this->child_list.push_back(&this->password_edit);
        this->child_list.push_back(&this->password_label);
        this->child_list.push_back(&this->ok);

        x = this->dx() + std::max(this->login_label.cx(), this->password_label.cx()) + 20;
        this->login_edit.rect.x = x;
        this->login_edit.label.rect.x = x;
        this->password_edit.rect.x = x;
        this->password_edit.rect.x = x;
        this->rect.cx = x + std::max(this->login_edit.cx(), this->password_edit.cx()) + 10;
        this->ok.set_button_x(this->rect.cx - this->ok.cx() - 10);

        this->resize_titlebar();

        y = this->dy() + this->titlebar.cy() + this->login_label.cy();
        this->login_label.rect.y = y;
        this->login_edit.rect.y = y;
        this->login_edit.label.rect.y = this->login_edit.dy() + this->login_edit.label.y_text;
        y += this->login_label.cy() * 2;
        this->password_label.rect.y = y;
        this->password_edit.rect.y = y;
        this->password_edit.rect.y = this->password_edit.dy() + this->password_edit.y_text;
        y += this->password_label.cy() + 10;
        this->ok.set_button_y(y);
        this->rect.cy = y + this->ok.cy() + 5 - this->dy();
    }

    virtual void notify(Widget* widget, notify_event_t event,
                        long unsigned int param, long unsigned int param2)
    {
        if (widget == &this->ok) {
            this->send_notify(NOTIFY_SUBMIT);
        }
    }

    virtual ~WindowLogin()
    {}
};

#endif
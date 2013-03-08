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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_LOGIN_HPP_)
#define REDEMPTION_MOD_WIDGET2_WINDOW_LOGIN_HPP_

#include "window_box.hpp"
#include "edit.hpp"
#include "button.hpp"

class WindowLogin : public WindowBox
{
public:
    WidgetEdit login;
    WidgetEdit pass;

public:
    enum {
        NOTIFY_FOCUS_BEGIN = ::NOTIFY_FOCUS_BEGIN,
        NOTIFY_FOCUS_END = ::NOTIFY_FOCUS_END,
        NOTIFY_SUBMIT = ::NOTIFY_SUBMIT,
        NOTIFY_CANCEL = ::NOTIFY_CANCEL,
        NOTIFY_USERNAME_EDIT = 100,
        NOTIFY_PASSWORD_EDIT,
        NOTIFY_USERNAME_SUBMIT,
        NOTIFY_PASSWORD_SUBMIT,
    };

    WindowLogin(ModApi* drawable, int x, int y, Widget* parent, NotifyApi* notifier, int id = 0)
    : WindowBox(drawable, Rect(x, y, 270, 95), parent, notifier, "Connection", id)
    , login(drawable, Rect(10,20,250,20), this, 0, 0, 0, 2)
    , pass(drawable, Rect(10,45,250,20), this, 0, 0, 0, 3)
    {}

    virtual void notify(int id, EventType event)
    {
        if (id == 2) {
            this->notify_self(event == WIDGET_SUBMIT
            ? NOTIFY_USERNAME_SUBMIT
            : NOTIFY_USERNAME_EDIT);
        } else if (id == 3) {
            this->notify_self(event == WIDGET_SUBMIT
            ? NOTIFY_PASSWORD_SUBMIT
            : NOTIFY_PASSWORD_EDIT);
        } else {
            this->WindowBox::notify(id, event);
        }
    }

    virtual ~WindowLogin()
    {}
};

#endif
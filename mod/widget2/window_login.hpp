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

#include "window.hpp"
#include "edit.hpp"

class WindowLogin : public Window
{
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
        //NOTIFY_PASSWORD_SUBMIT,
    };

    WindowLogin(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier)
    : Window(drawable, rect, parent, notifier)
    , login(drawable, Rect(0,0,100,15), this, 0)
    , pass(drawable, Rect(0,0,100,15), this, 0)
    {}

    virtual void notify(int id, EventType event)
    {
        if (event == WIDGET_SUBMIT){
            this->ok();
        } else {
            this->Window::notify(id, event);
        }
    }

    void ok()
    {
        this->notify_self(NOTIFY_SUBMIT);
    }
};

#endif
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

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_BOX_HPP)
#define REDEMPTION_MOD_WIDGET2_WINDOW_BOX_HPP

#include "button.hpp"
#include "window.hpp"

class WindowBox : public Window
{
public:
    enum {
        NOTIFY_FOCUS_BEGIN = ::NOTIFY_FOCUS_BEGIN,
        NOTIFY_FOCUS_END = ::NOTIFY_FOCUS_END,
        NOTIFY_SUBMIT = ::NOTIFY_SUBMIT,
        NOTIFY_CANCEL = ::NOTIFY_CANCEL,
        //NOTIFY_USERNAME_EDIT = 100,
        //NOTIFY_PASSWORD_EDIT,
        //NOTIFY_USERNAME_SUBMIT,
        //NOTIFY_PASSWORD_SUBMIT,
    };

    WidgetButton submit;
    WidgetButton cancel;

    WindowBox(ModApi* drawable, const Rect & rect, Widget* parent, NotifyApi* notifier, const char * caption, int id = 0)
    : Window(drawable, rect, parent, notifier, caption, id)
    , submit(drawable, Rect(), this, 0, "submit", 0)
    , cancel(drawable, Rect(), this, 0, "cancel", 1)
    {
        int cx = std::max(this->cancel.context_text->cx, this->submit.context_text->cx) + 8;
        int cy = std::max(this->cancel.context_text->cy, this->submit.context_text->cy) + 6;
        int y = this->rect.cy - cy - 5;
        this->cancel.rect.cx = cx;
        this->cancel.rect.cy = cy;
        this->cancel.rect.y = y;
        this->cancel.rect.x = this->rect.cx - cx - 5;
        this->cancel.x_text = (cx - this->cancel.context_text->cx) / 2;
        this->cancel.y_text = (cy - this->cancel.context_text->cy) / 2;
        this->submit.rect.cx = cx;
        this->submit.rect.cy = cy;
        this->submit.rect.y = y;
        this->submit.rect.x = this->cancel.rect.x - cx - 10;
        this->submit.x_text = (cx - this->submit.context_text->cx) / 2;
        this->submit.y_text = (cy - this->submit.context_text->cy) / 2;
    }

    virtual ~WindowBox()
    {}

    virtual void notify(int id, EventType event)
    {
        if (event == WIDGET_SUBMIT){
            if (id == this->cancel.id) {
                this->notify_self(NOTIFY_CANCEL);
            } else if (id == this->submit.id) {
                this->notify_self(NOTIFY_SUBMIT);
            }
        } else {
            this->Window::notify(id, event);
        }
    }
};

#endif
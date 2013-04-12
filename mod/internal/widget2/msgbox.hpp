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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_MSGBOX_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_MSGBOX_HPP

#include "window.hpp"
#include "multiline.hpp"

class MessageBox : public Window
{
public:
    WidgetMultiLine msg;
    WidgetButton ok;

    MessageBox(ModApi* drawable, int16_t x, int16_t y, Widget* parent,
                     NotifyApi* notifier, const char * caption, const char * text,
                     int id = 0, int bgcolor = BLACK, int fgcolor = WHITE)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, id)
    , msg(drawable, 0, 0, this, NULL, text, true, -10, bgcolor, fgcolor, 10, 2)
    , ok(drawable, 0,0, this, this, "Ok", true, -11, bgcolor, fgcolor, 6, 2)
    {
        this->child_list.push_back(&this->msg);
        this->child_list.push_back(&this->ok);

        this->rect.cx = std::max<int>(this->msg.cx(), this->ok.cx() + 20);
        this->msg.rect.x += (this->cx() - this->msg.cx()) / 2;

        this->resize_titlebar();

        this->rect.cy = this->titlebar.cy() + this->msg.cy() + this->ok.cy() + 10;
        this->msg.rect.y += this->titlebar.cy() + 5;
        this->ok.set_button_x(this->dx() + this->cx() - this->ok.cx() - 10);
        this->ok.set_button_y(this->dy() + this->cy() - this->ok.cy() - 5);
    }

    virtual ~MessageBox()
    {}

    virtual void notify(Widget* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (this->notifier) {
            if (widget == &this->ok) {
                this->send_notify(NOTIFY_CANCEL);
            } else {
                Window::notify(widget, event, param, param2);
            }
        }
    }
};

#endif
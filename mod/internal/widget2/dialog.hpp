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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_DIALOG_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_DIALOG_HPP

#include "window.hpp"
#include "multiline.hpp"

class WidgetDialog : public Window
{
public:
    WidgetMultiLine lines;
    WidgetButton ok;

    WidgetDialog(ModApi* drawable, int16_t x, int16_t y, Widget* parent, NotifyApi* notifier, const char * caption, const char * text, int id = 0, int bgcolor = BLACK, int fgcolor = WHITE)
    : Window(drawable, Rect(x,y,1,1), parent, notifier, caption, bgcolor, id)
    , lines(drawable, 10, 10 + this->titlebar.cx(), this, NULL, text, true, -10, bgcolor, fgcolor)
    , ok(drawable, 0,0, this, this, "Ok", true, -11, bgcolor, fgcolor, 5, 1)
    {
        this->rect.cx = this->lines.cx() + 20;
        this->resize_titlebar();
        this->rect.cy = this->titlebar.cy() + this->lines.cy() + this->ok.cy() + 30;
        this->lines.rect.y += this->titlebar.cy();
        this->ok.rect.x = this->dx() + this->rect.cx - this->ok.cx() - 10;
        this->ok.rect.y = this->dy() + this->rect.cy - this->ok.cy() - 10;
        this->ok.label.rect.x = this->ok.dx() + 2;
        this->ok.label.rect.y = this->ok.dy() + 2;
        this->child_list.push_back(&this->lines);
        this->child_list.push_back(&this->ok);
    }

    virtual ~WidgetDialog()
    {}

    virtual void notify(Widget* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (this->notifier && widget == &ok) {
            this->send_notify(NOTIFY_CANCEL);
        }
    }
};

#endif
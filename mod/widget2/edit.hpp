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

#if !defined(REDEMPTION_MOD_WIDGET2_EDIT_HPP_)
#define REDEMPTION_MOD_WIDGET2_EDIT_HPP_

#include "widget.hpp"
#include <keymap2.hpp>

class WidgetEdit : public Widget
{
public:
    WidgetEdit(ModApi * drawable, const Rect& rect, Widget * parent, NotifyApi * notifier)
    : Widget(drawable, rect, parent, Widget::TYPE_EDIT, notifier)
    {}

    virtual void draw(const Rect& rect, const Rect& clip_screen)
    {
        this->Widget::draw(rect, clip_screen);
        int w,h;
        this->drawable->text_metrics("", w,h);
        this->drawable->server_draw_text(
            clip_screen.x, clip_screen.y,
            "", 0, 0, clip_screen
        );
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == KEYDOWN)
        {
            if (keymap->top_kevent() == Keymap2::KEVENT_ENTER){
                this->notify_parent(WIDGET_SUBMIT);
            } else {
                this->notify_self(NOTIFY_TEXT_CHANGED);
                //this->notify_parent(this, WM_DRAW);
                this->refresh(Rect(0,0,10,10));
            }
        }
    }
};

#endif
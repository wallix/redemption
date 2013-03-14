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

#if !defined(REDEMPTION_MOD_WIDGET2_BUTTON_HPP)
#define REDEMPTION_MOD_WIDGET2_BUTTON_HPP

#include "label.hpp"

class WidgetButton : public WidgetLabel
{
public:
    char buffer[256];
    int state;
    bool is_down;

    WidgetButton(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier, const char * text, int id = 0, int xtext = 0, int ytext = 0)
    : WidgetLabel(drawable, rect, parent, notifier, text, id, xtext, ytext)
    , state(0)
    , is_down(false)
    {
        this->type = Widget::TYPE_BUTTON;
    }

    virtual ~WidgetButton()
    {}

    virtual void send_event(EventType event, int param, int param2, Keymap2* keymap)
    {
        switch (event) {
            case CLIC_BUTTON1_DOWN:
                this->is_down = true;
                this->refresh(this->rect);
                break;
            case CLIC_BUTTON1_UP:
                if (this->is_down) {
                    this->is_down = false;
                    this->notify_self(NOTIFY_SUBMIT);
                    this->notify_parent(WIDGET_SUBMIT);
                    this->refresh(this->rect);
                }
                break;
            case FOCUS_END:
                if (this->is_down) {
                    this->is_down = false;
                    this->refresh(this->rect);
                }
                break;
            default:
                this->Widget::send_event(event, param, param2, keymap);
                break;
        }
    }
};

#endif
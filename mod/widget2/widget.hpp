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

#if !defined(REDEMPTION_MOD_WIDGET2_WIDGET_HPP_)
#define REDEMPTION_MOD_WIDGET2_WIDGET_HPP_

#include <vector>
#include "notify_api.hpp"
#include "mod_api.hpp"
#include "notify_api.hpp"
#include <rect.hpp>


#include <iostream>

class Keymap2;

enum EventType {
    FOCUS_BEGIN,
    FOCUS_END,
    KEYDOWN,
    KEYUP,
    CLIC_BUTTON1_UP,
    CLIC_BUTTON1_DOWN,
    CLIC_BUTTON2_UP,
    CLIC_BUTTON2_DOWN,
    CLIC_BUTTON3_UP,
    CLIC_BUTTON3_DOWN,
    WM_DRAW,
    TEXT_CHANGED,
    BUTTON_PRESSED,
};

enum NotifyEventType {
    NOTIFY_FOCUS_BEGIN,
    NOTIFY_FOCUS_END,
    NOTIFY_TEXT_CHANGED = TEXT_CHANGED,
    NOTIFY_BUTTON_PRESSED = BUTTON_PRESSED,
    NOTIFY_SUBMIT,
    NOTIFY_CANCEL,
};

class Widget
{
public:
    enum WidgetType {
        TYPE_SCREEN  = 0,
        TYPE_WND     = 1,
        TYPE_BITMAP  = 2,
        TYPE_BUTTON  = 3,
        TYPE_IMAGE   = 4,
        TYPE_EDIT    = 5,
        TYPE_LABEL   = 6,
    };

public:
    Widget * parent;
    ModApi * drawable;
    NotifyApi * notifier;
    Rect rect;
    int type;
    int id;
    int bg_color;
    bool has_focus;

public:
    Widget(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , rect(rect)
    , type(type)
    , id(0)
    , bg_color(0)
    , has_focus(false)
    {
        if (this->parent)
            this->parent->attach_widget(this);
    }

    virtual ~Widget()
    {
        if (this->parent)
            this->parent->detach_widget(this);
    }

protected:
    virtual void attach_widget(Widget *)
    {}

    virtual void detach_widget(Widget *)
    {}

public:
    virtual void draw(const Rect& clip)
    {
        this->drawable->draw(RDPOpaqueRect(this->rect, this->bg_color), clip);
    }

    void refresh(const Rect & rect)
    {
        if (!rect.isempty() && this->drawable){
            this->drawable->begin_update();
            this->draw(rect);
            this->drawable->end_update();
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == WM_DRAW){
            this->refresh(this->rect);
        }
    }

    void notify_self(NotifyApi::notify_event_t event)
    {
        if (this->notifier)
            this->notifier->notify(this, event);
    }

    void notify_parent(EventType event)
    {
        if (this->parent)
            this->parent->notify(this->id, event);
    }

    virtual void notify(int id, EventType event)
    {
        (void)id;
        this->notify_self(event);
        this->notify_parent(event);
    }

    virtual Widget * widget_at_pos(int x, int y)
    {
        if (this->rect.contains_pt(x, y))
            return this;
        return 0;
    }

    int dx() const
    {
        return this->rect.x;
    }

    int dy() const
    {
        return this->rect.y;
    }

    uint16_t cx() const
    {
        return this->rect.cx;
    }

    uint16_t cy() const
    {
        return this->rect.cy;
    }

    virtual Widget * widget_focused()
    {
        return 0; ///TODO
    }
};

#endif
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

class Keymap2;

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
    bool has_focus;

public:
    Widget(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , rect(rect)
    , type(type)
    , id(0)
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
    virtual void draw(const Rect& rect)
    {
        (void)rect;
    }

    virtual void redraw(const Rect & rect)
    {
        if (!rect.isempty() && this->drawable){
            this->drawable->begin_update();
            this->draw(rect);
            this->drawable->begin_update();
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {}

protected:
    void notify_self(EventType event)
    {
        if (this->notifier)
            this->notifier->notify(this, event);
    }

    void notify_parent(Widget * w, EventType event)
    {
        if (this->parent)
            this->parent->notify(w, event);
    }

    static void notify_to(Widget * w, EventType type)
    {
        w->notify_self(type);
    }

public:
    virtual void notify(Widget * w, EventType event)
    {
        this->notify_self(event);
        this->notify_parent(w, event);
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
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

#include "notifyapi.hpp"
#include <rect.hpp>

class DrawAPI;

class Widget : NotifyAPI
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
    int type;
    int id;
    Rect rect;
    bool has_focus;
    DrawAPI * drawable;
    Widget * parent;

public:
    Widget(DrawAPI * drawable, int width, int height, Widget * parent, int type)
    : type(type)
    , id(0)
    , rect(0,0,width,height)
    , has_focus(false)
    , drawable(drawable)
    , parent(parent)
    {}

    void draw(const Rect& rect)
    {
        this->do_draw(rect);
    }

    virtual void notify(int id, EventType event)
    {
        (void)id;
        if (event == CLIC_BUTTON1_DOWN || event == FOCUS_BEGIN)
        {
            this->focus();
            this->propagate_event(FOCUS_BEGIN);
        }
        else
        {
            this->propagate_event(event);
        }
    }

    void propagate_event(EventType event)
    {
        if (this->parent)
            this->parent->notify(this->id, event);
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

    virtual Widget * widget_focused()
    {
        return 0;
    }

    virtual void focus()
    { this->has_focus = true; }

    virtual void blur()
    { this->has_focus = false;}

    void set_position(int x, int y)
    {
        this->rect.x = x;
        this->rect.y = y;
    }

protected:
    virtual void do_draw(const Rect&) {}; ///TODO virtual pure = 0
};

#endif
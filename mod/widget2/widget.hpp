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

    struct notify_event {
        EventType type;
        NotifyApi * notify;

        notify_event(EventType type, NotifyApi * notify)
        : type(type)
        , notify(notify)
        {}

        void send_notify(Widget * sender, Widget * receiver)
        {
            this->notify->notify(sender, receiver, this->type);
        }
    };

public:
    int type;
    int id;
    Rect rect;
    bool has_focus;
    ModApi * drawable;
    std::vector<notify_event> notifies;
    Widget * parent;

public:
    Widget(ModApi * drawable, int width, int height, Widget * parent, int type)
    : type(type)
    , id(0)
    , rect(0,0,width,height)
    , has_focus(false)
    , drawable(drawable)
    , parent(parent)
    {}

    void add_notify(EventType type, NotifyApi * notify)
    {
        notifies.push_back(notify_event(type, notify));
    }

    NotifyApi * detach_notify(EventType type, NotifyApi * notify)
    {
        NotifyApi * ret = 0;
        for (std::size_t i = 0; i != notifies.size(); ++i) {
            if (notifies[i].type == type && notifies[i].notify == notify) {
                ret = notifies[i].notify;
                notifies[i] = notifies[notifies.size()-1];
                notifies.pop_back();
                break;
            }
        }
        return ret;
    }

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

    virtual void def_proc(EventType event, int param, Keymap2 * keymap)
    {}

    virtual void notify(Widget * w, EventType event)
    {
        this->notify_self(w, event);
        this->notify_parent(w, event);
    }

    void notify_self(Widget * w, EventType event)
    {
        for (std::size_t i = 0; i != notifies.size(); ++i) {
            if (notifies[i].type == event) {
                notifies[i].send_notify(w, this);
            }
        }
    }

    void notify_parent(Widget * w, EventType event)
    {
        if (this->parent)
            this->parent->notify(w, event);
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

    virtual void focus()
    {
        this->has_focus = true;
        this->notify(this, FOCUS_BEGIN);
    }

    virtual void blur()
    {
        this->has_focus = false;
        this->notify(this, FOCUS_END);
    }

    void set_position(int x, int y)
    {
        this->rect.x = x;
        this->rect.y = y;
    }
};

#endif
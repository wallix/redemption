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
    WIDGET_SUBMIT
};

enum NotifyEventType {
    NOTIFY_FOCUS_BEGIN,
    NOTIFY_FOCUS_END,
    NOTIFY_TEXT_CHANGED = TEXT_CHANGED,
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
    struct screen_position {
        Rect rect;
        uint16_t x;
        uint16_t y;

        screen_position(const Rect & rect)
        : rect(rect)
        , x(rect.x)
        , y(rect.y)
        {}
    };

    screen_position get_clip_base()
    {
        screen_position ret(this->rect);
        for (Widget * p = this->parent; p; p = p->parent){
            ret.rect = ret.rect.intersect(p->rect.cx, p->rect.cy);
            ret.rect.x += p->rect.x;
            ret.rect.y += p->rect.y;
            ret.x += p->rect.x;
            ret.y += p->rect.y;
        }
        return ret;
    }

    virtual void draw(const Rect& rect, uint16_t x_screen, uint16_t y_screen, const Rect& clip_screen)
    {
        (void)x_screen;
        (void)y_screen;
        this->drawable->draw(
            RDPOpaqueRect(
                rect.offset(clip_screen.x, clip_screen.y),
                this->bg_color
            ), clip_screen);
    }

    void refresh(const Rect & rect)
    {
        if (!rect.isempty() && this->drawable){
            screen_position sp = this->get_clip_base();
            if (sp.rect.cx && sp.rect.cy && rect.x < sp.rect.cx && rect.y < sp.rect.cy){
                this->drawable->begin_update();
                this->draw(rect, sp.x, sp.y, sp.rect);
                this->drawable->end_update();
            }
        }
    }

    void refresh_child(Widget * w, const Rect & rect, uint16_t x_screen, uint16_t y_screen, const Rect & clip_screen)
    {
        if (!w->rect.isempty() && w->drawable){
            Rect new_clip = clip_screen.intersect(
                Rect(clip_screen.x + rect.x + w->rect.x,
                     clip_screen.y + rect.y + w->rect.y,
                     rect.x + rect.cx,
                     rect.y + rect.cy)
            );
            if (new_clip.cx && new_clip.cy && rect.x < new_clip.cx && rect.y < new_clip.cy){
                w->drawable->begin_update();
                w->draw(rect, x_screen + w->rect.x, y_screen + w->rect.y, new_clip);
                w->drawable->end_update();
            }
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == WM_DRAW){
            this->refresh(Rect(0,0,this->rect.cx, this->rect.cy));
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
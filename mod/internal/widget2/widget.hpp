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
#include <callback.hpp>

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
    WIDGET_SUBMIT,
    WIDGET_CANCEL
};

enum NotifyEventType {
    NOTIFY_FOCUS_BEGIN = FOCUS_BEGIN,
    NOTIFY_FOCUS_END = FOCUS_END,
    NOTIFY_TEXT_CHANGED = TEXT_CHANGED,
    NOTIFY_SUBMIT = WIDGET_SUBMIT,
    NOTIFY_CANCEL = WIDGET_CANCEL,
    NOTIFY_SELECTION_CHANGED,
};

class Widget2 : public RdpInput, public NotifyApi
{
public:
    enum OptionTab {
        IGNORE_TAB = 0,
        NORMAL_TAB = 1,
        DELEGATE_CONTROL_TAB = 2,
        NO_DELEGATE_CHILD_TAB = 4,
        REWIND_TAB = 8,
        REWIND_BACKTAB = 16
    };

public:
    Widget2 * parent;
    ModApi * drawable;
    NotifyApi * notifier;
    Rect rect;
    int group_id;
    //int tab_flag;
    bool has_focus;

public:
    Widget2(ModApi * drawable, const Rect& rect, Widget2 * parent, NotifyApi * notifier, int group_id = 0)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , rect(Rect(rect.x + (parent ? parent->dx() : 0),
                rect.y + (parent ? parent->dy() : 0),
                rect.cx,
                rect.cy
    ))
    , group_id(group_id)
    //, tab_flag(NORMAL_TAB)
    , has_focus(false)
    {}

    virtual ~Widget2()
    {}

    virtual void draw(const Rect& clip) = 0;

    // NOTE move to Widget2Composite ?
    //void refresh(const Rect & rect)
    //{
    //    if (!rect.isempty() && this->drawable){
    //        Widget2::screen_position sp = this->position_in_screen();
    //        Rect clip = sp.clip.intersect(Rect(sp.x + rect.x, sp.y + rect.y, rect.cx, rect.cy));
    //        if (clip.isempty()) {
    //            return ;
    //        }
    //        int dx = clip.x - sp.x;
    //        int dy = clip.y - sp.y;
    //        Rect new_rect = Rect(dx,
    //                             dy,
    //                             clip.cx - (rect.x - dx),
    //                             clip.cy - (rect.y - dy)
    //                            );
    //        if (!new_rect.isempty()){
    //            this->drawable->begin_update();
    //            this->draw(new_rect, sp.x, sp.y, clip.x, clip.y);
    //            this->drawable->end_update();
    //        }
    //    }
    //}

    void refresh(const Rect& clip)
    {
        if (this->drawable) {
            if (!clip.isempty()){
                this->drawable->begin_update();
                this->draw(clip);
                this->drawable->end_update();
            }
        }
    }

    // External world can generate 4 kind of events
    // - keyboard event (scancode)
    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
    }

    // - mouve event (mouse moves or a button went up or down)
    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
    }

    // - synchronisation of capslock, numlock, etc state.
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
    }

    // - part of screen should be redrawn
    virtual void rdp_input_invalidate(const Rect & r)
    {
        this->refresh(r);
    }

    void send_notify(NotifyApi::notify_event_t event,
                     unsigned long param = 0, unsigned long param2 = 0)
    {
        if (this->notifier)
            this->notifier->notify(this, event, param, param2);
    }

    virtual void notify(Widget2 * widget, NotifyApi::notify_event_t event,
                        unsigned long param, unsigned long param2)
    {
        (void)widget;
        this->send_notify(event, param, param2);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        if (this->rect.contains_pt(x, y))
            return this;
        return 0;
    }

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->rect.x = x;
        this->rect.y = y;
    }

    virtual void set_wh(uint16_t w, uint16_t h)
    {
        this->rect.cx = w;
        this->rect.cy = h;
    }

    virtual bool focus(Widget2 * old_focused)
    {
        (void)old_focused;
        this->send_notify(NOTIFY_FOCUS_BEGIN);
        this->has_focus = true;
        return true;
    }

    virtual void blur()
    {
        this->send_notify(NOTIFY_FOCUS_END);
        this->has_focus = false;
    }

    ///Return x position in it's screen
    int16_t dx() const
    {
        return this->rect.x;
    }

    ///Return y position in it's screen
    int16_t dy() const
    {
        return this->rect.y;
    }

    ///Return width
    uint16_t cx() const
    {
        return this->rect.cx;
    }

    ///Return height
    uint16_t cy() const
    {
        return this->rect.cy;
    }

    ///Return x position in it's parent
    int16_t px() const
    {
        return this->parent ? this->dx() - this->parent->dx() : this->dx();
    }

    ///Return y position in it's parent
    int16_t py() const
    {
        return this->parent ? this->dy() - this->parent->dy() : this->dy();
    }

    virtual Widget2 * widget_focused()
    {
        return this->has_focus ? this : 0;
    }
};

#endif

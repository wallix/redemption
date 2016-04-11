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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET_HPP_)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET_HPP_

#include <vector>

#include "notify_api.hpp"
#include "mod/mod_api.hpp"
#include "utils/rect.hpp"
#include "core/callback.hpp"
#include "core/RDP/pointer.hpp"

struct Keymap2;

enum NotifyEventType {
    NOTIFY_FOCUS_BEGIN,
    NOTIFY_FOCUS_END,
    NOTIFY_TEXT_CHANGED,
    NOTIFY_SUBMIT,
    NOTIFY_CANCEL,
    NOTIFY_SELECTION_CHANGED,
    NOTIFY_COPY,
    NOTIFY_PASTE,
    NOTIFY_CUT,
};

class Widget2 : public RdpInput, public NotifyApi
{
public:
    TODO("using several booleans may be easier to read than flags")
    enum OptionTab {
        IGNORE_TAB = 0x00,
        NORMAL_TAB = 0x02
    };

    enum OptionFocus {
        IGNORE_FOCUS = 0x00,
        NORMAL_FOCUS = 0x01
        // FORCE_FOCUS  = 0x04
    };

public:
    Widget2 & parent;
    mod_api & drawable;
    NotifyApi * notifier;
    Rect rect;
    int group_id;
    int tab_flag;
    int focus_flag;
    int pointer_flag;
    bool has_focus;
    int notify_value;

public:
    Widget2(mod_api & drawable, const Rect& rect, Widget2 & parent, NotifyApi * notifier, int group_id = 0)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , rect(Rect(rect.x + ((&parent != this) ? parent.dx() : 0),
                rect.y + ((&parent != this) ? parent.dy() : 0),
                rect.cx,
                rect.cy
                ))
    , group_id(group_id)
    , tab_flag(NORMAL_TAB)
    , focus_flag(NORMAL_FOCUS)
    , pointer_flag(Pointer::POINTER_NORMAL)
    , has_focus(false)
    , notify_value(0) {}

    ~Widget2() override {}

    virtual bool next_focus()
    {
        return false;
    }

    virtual bool previous_focus()
    {
        return false;
    }

    virtual void draw(const Rect& clip) = 0;

    void refresh(const Rect& clip)
    {
        if (!clip.isempty()){
            this->drawable.begin_update();
            this->draw(clip);
            this->drawable.end_update();
        }
    }

    bool is_root() {
        // The root widget is defined as the parent of itself (screen widget only)
        return (&this->parent == this);
    }

    virtual void show_tooltip(Widget2 * widget, const char * text, int x, int y, int iter = 10) {
        if (iter > 0) {
            this->parent.show_tooltip(widget, text, x, y, iter - 1);
        }
    }
    void hide_tooltip() {
        this->show_tooltip(this, nullptr, 0, 0);
    }

    Widget2 * last_widget_at_pos(int16_t x, int16_t y) {
        // recursive
        // Widget2 * w = this->widget_at_pos(x, y);
        // if (w && (w != this)) {
        //     return w->last_widget_at_pos(x, y);
        // }
        // return this;

        // loop
        Widget2 * w = this;
        int count = 10;
        while (w->widget_at_pos(x, y)
               && (w != w->widget_at_pos(x, y))
               && (--count > 0)) {
            w = w->widget_at_pos(x, y);
        }
        return w;
    }

    // External world can generate 4 kind of events
    // - keyboard event (scancode)
    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override {
    }

    // - mouve event (mouse moves or a button went up or down)
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
    }

    // - synchronisation of capslock, numlock, etc state.
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
    }

    // - part of screen should be redrawn
    void rdp_input_invalidate(const Rect & r) override {
        this->refresh(r);
    }

    void send_notify(NotifyApi::notify_event_t event)
    {
        if (this->notifier)
            this->notifier->notify(this, event);
    }

    void notify(Widget2 * w, NotifyApi::notify_event_t event) override {
        if (this->notifier)
            this->notifier->notify(this, event);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        if (this->rect.contains_pt(x, y))
            return this;
        return nullptr;
    }

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->rect.x = x;
        this->rect.y = y;
    }

    virtual void set_wh(int16_t w, int16_t h)
    {
        this->rect.cx = w;
        this->rect.cy = h;
    }

    virtual void set_color(uint32_t bg_color, uint32_t fg_color) {
    }

    enum {
          focus_reason_tabkey
        , focus_reason_backtabkey
        , focus_reason_mousebutton1
    };
    virtual void focus(int reason)
    {
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            this->refresh(this->rect);
        }
    }

    virtual void blur()
    {
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            this->refresh(this->rect);
        }
    }

    virtual Dimension get_optimal_dim() {
        return Dimension(0, 0);
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

    ///Return dx()+cx()
    int16_t lx() const
    {
        return this->rect.x + this->rect.cx;
    }

    ///Return dy()+cy()
    int16_t ly() const
    {
        return this->rect.y + this->rect.cy;
    }

    int16_t centerx() const
    {
        return this->rect.x + this->rect.cx / 2;
    }

    int16_t centery() const
    {
        return this->rect.y + this->rect.cy / 2;
    }

    ///Return x position in it's parent
    int16_t px() const
    {
        return this->dx() - this->parent.dx();
    }

    ///Return y position in it's parent
    int16_t py() const
    {
        return this->dy() - this->parent.dy();
    }
};

#endif

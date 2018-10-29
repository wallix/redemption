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
#pragma once

#include "mod/internal/widget/notify_api.hpp"
#include "utils/rect.hpp"
#include "utils/colors.hpp"
#include "core/callback.hpp"
#include "core/RDP/rdp_pointer.hpp"

struct Keymap2;
namespace gdi
{
    class GraphicApi;
} // namespace gdi

enum NotifyEventType
{
    NOTIFY_FOCUS_BEGIN,
    NOTIFY_FOCUS_END,
    NOTIFY_TEXT_CHANGED,
    NOTIFY_SUBMIT,
    NOTIFY_CANCEL,
    NOTIFY_SELECTION_CHANGED,
    NOTIFY_COPY,
    NOTIFY_PASTE,
    NOTIFY_CUT,
    NOTIFY_HSCROLL,
    NOTIFY_VSCROLL,
};

class Widget : public RdpInput, public NotifyApi
{
public:
    // TODO using several booleans may be easier to read than flags
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
    Widget & parent;
    gdi::GraphicApi & drawable;
    NotifyApi * notifier;

private:
    Rect rect;

public:
    int group_id;
    int tab_flag;
    int focus_flag;
    int pointer_flag;
    bool has_focus;
    int notify_value;

protected:
    Widget(Widget const & other)
    : parent(other.parent)
    , drawable(other.drawable)
    , notifier(other.notifier)
    , rect(other.rect)
    , group_id(other.group_id)
    , tab_flag(other.tab_flag)
    , focus_flag(other.focus_flag)
    , pointer_flag(other.pointer_flag)
    , has_focus(other.has_focus)
    , notify_value(other.notify_value)
    {}

public:
    Widget(gdi::GraphicApi & drawable, Widget & parent, NotifyApi * notifier, int group_id = 0)
    : parent(parent)
    , drawable(drawable)
    , notifier(notifier)
    , group_id(group_id)
    , tab_flag(NORMAL_TAB)
    , focus_flag(NORMAL_FOCUS)
    , pointer_flag(Pointer::POINTER_NORMAL)
    , has_focus(false)
    , notify_value(0)
    {}

    virtual bool next_focus()
    {
        return false;
    }

    virtual bool previous_focus()
    {
        return false;
    }

    bool is_root()
    {
        // The root widget is defined as the parent of itself (screen widget only)
        return (&this->parent == this);
    }

    virtual void show_tooltip(
        Widget * widget, const char * text, int x, int y,
        Rect preferred_display_rect)
    {
        if (!this->is_root()) {
            this->parent.show_tooltip(widget, text, x, y, preferred_display_rect);
        }
    }

    void hide_tooltip()
    {
        this->show_tooltip(this, nullptr, 0, 0, Rect(0, 0, 0, 0));
    }

    Widget * last_widget_at_pos(int16_t x, int16_t y)
    {
        Widget * w = this;
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
    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * /*keymap*/) override {
        (void)param1;
        (void)param2;
        (void)param3;
        (void)param4;
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override
    {
        (void)unicode;
        (void)flag;
    }

    // - mouve event (mouse moves or a button went up or down)
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * /*keymap*/) override {
        (void)device_flags;
        (void)x;
        (void)y;
    }

    // - synchronisation of capslock, numlock, etc state.
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        (void)time;
        (void)device_flags;
        (void)param1;
        (void)param2;
    }

    void refresh(Rect clip) override
    {
        this->rdp_input_invalidate(clip);
    }

    void send_notify(NotifyApi::notify_event_t event)
    {
        if (this->notifier) {
            this->notifier->notify(this, event);
        }
    }

    void notify(Widget * /*sender*/, NotifyApi::notify_event_t event) override
    {
        if (this->notifier) {
            this->notifier->notify(this, event);
        }
    }

    virtual Widget * widget_at_pos(int16_t x, int16_t y)
    {
        return (this->rect.contains_pt(x, y)) ? this : nullptr;
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

    void set_wh(Dimension dim)
    {
        this->set_wh(dim.w, dim.h);
    }

    virtual void set_color(BGRColor bg_color, BGRColor fg_color)
    {
        (void)bg_color;
        (void)fg_color;
    }

    virtual void move_xy(int16_t x, int16_t y)
    {
        this->set_xy(this->rect.x + x, this->rect.y + y);
    }

    enum {
          focus_reason_tabkey
        , focus_reason_backtabkey
        , focus_reason_mousebutton1
    };
    virtual void focus(int reason)
    {
        (void)reason;
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            this->rdp_input_invalidate(this->rect);
        }
    }

    virtual void blur()
    {
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            this->rdp_input_invalidate(this->rect);
        }
    }

    virtual Dimension get_optimal_dim()
    {
        return Dimension(0, 0);
    }

    ///Return x position in it's screen
    int16_t x() const
    {
        return this->rect.x;
    }

    ///Return y position in it's screen
    int16_t y() const
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

    ///Return x()+cx()
    int16_t right() const
    {
        return this->rect.right();
    }

    ///Return y()+cy()
    int16_t bottom() const
    {
        return this->rect.bottom();
    }

    Rect get_rect() const {
        return this->rect;
    }

    virtual const Pointer* get_pointer() const
    {
        return nullptr;
    }
};

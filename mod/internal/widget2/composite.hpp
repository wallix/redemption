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

#if !defined(REDEMPTION_MOD_WIDGET2_WIDGET_COMPOSITE_HPP_)
#define REDEMPTION_MOD_WIDGET2_WIDGET_COMPOSITE_HPP_

#include <vector>
#include <algorithm>
#include "widget.hpp"
#include "keymap2.hpp"
#include <region.hpp>

class CompositeInterface {
public:
    virtual ~CompositeInterface() {}
    virtual void add_widget(Widget2 * w) = 0;
    virtual void remove_widget(Widget2 * w) = 0;
    virtual void clear() = 0;
    virtual void move_xy(int16_t x, int16_t y) = 0;
    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) = 0;
    virtual Widget2 * next_focus(Widget2 * current_focus) = 0;
    virtual Widget2 * previous_focus(Widget2 * current_focus) = 0;
    virtual void draw(const Rect& clip) = 0;
    virtual void draw_inner_free(const Rect& clip, int bg_color, Region & region) = 0;
};


class WidgetParent : public Widget2 {
    Widget2 * pressed;
protected:
    CompositeInterface * impl;
public:
    Widget2 * current_focus;

    WidgetParent(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                 NotifyApi * notifier, int group_id = 0)
        : Widget2(drawable, rect, parent, notifier, group_id)
        , pressed(NULL)
        , impl(NULL)
        , current_focus(NULL)
    {
    }

    virtual ~WidgetParent() {
        if (this->impl) {
            delete this->impl;
            this->impl = NULL;
        }
    }

    void set_widget_focus(Widget2 * new_focused)
    {
        if (this->current_focus) {
            this->current_focus->blur();
        }
        this->current_focus = new_focused;
        this->current_focus->focus();
    }

    virtual void focus()
    {
        if (!this->has_focus){
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
            if (this->current_focus) {
                this->current_focus->focus();
            }
            this->refresh(this->rect);
        }
    }

    virtual void blur()
    {
        if (this->has_focus){
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            if (this->current_focus) {
                this->current_focus->blur();
            }
            this->refresh(this->rect);
        }
    }
    virtual void add_widget(Widget2 * w) {
        this->impl->add_widget(w);
    }
    virtual void remove_widget(Widget2 * w) {
        this->impl->remove_widget(w);
    }
    virtual void clear() {
        this->impl->clear();
    }

    virtual bool next_focus() {
        Widget2 * next = this->impl->next_focus(this->current_focus);
        if (next) {
            this->set_widget_focus(next);
            return true;
        }
        return false;
    }

    virtual bool previous_focus() {
        Widget2 * previous = this->impl->previous_focus(this->current_focus);
        if (previous) {
            this->set_widget_focus(previous);
            return true;
        }
        return false;
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (!this->rect.contains_pt(x, y))
            return 0;
        if (this->current_focus) {
            if (this->current_focus->rect.contains_pt(x, y)) {
                return this->current_focus;
            }
        }
        return this->impl->widget_at_pos(x, y);
    }

    virtual void rdp_input_scancode(long param1, long param2, long param3,
                                    long param4, Keymap2 * keymap)
    {
        if (keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()) {
            case Keymap2::KEVENT_TAB:
                //std::cout << ("tab") << '\n';
                keymap->get_kevent();
                this->next_focus();
                break;
            case Keymap2::KEVENT_BACKTAB:
                //std::cout << ("backtab") << '\n';
                keymap->get_kevent();
                this->previous_focus();
                break;
            default:
                if (this->current_focus)
                    this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
            }
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        Widget2 * w = this->widget_at_pos(x, y);

        // Mouse clic release
        // w could be null if mouse is located at an empty space
        if (device_flags == MOUSE_FLAG_BUTTON1) {
            if (this->pressed
                && (w != this->pressed)) {
                this->pressed->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }
        if (w){
            // get focus when mouse clic
            if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
                this->pressed = w;
                if ((w->focus_flag != IGNORE_FOCUS) && (w != this->current_focus)){
                    this->set_widget_focus(w);
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
        else {
            Widget2::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

};


class CompositeTable : public CompositeInterface {
    enum {
        AUTOSIZE = 256
    };

    Widget2 * child_list[AUTOSIZE];
    size_t size;

public:
    CompositeTable()
        : size(0)
    {
    }

    void add_widget(Widget2 * w) {
        if (this->size >= AUTOSIZE)
            return;
        this->child_list[this->size] = w;
        this->size++;
    }

    void remove_widget(Widget2 * w) {
        bool found = false;
        for (size_t i = 0; i < this->size; ++i) {
            if (!found) {
                if (w == this->child_list[i]) {
                    found = true;
                    this->child_list[i] = NULL;
                }
            }
            else {
                this->child_list[i-1] = this->child_list[i];
            }
        }
        if (found) {
            this->child_list[this->size] = NULL;
            this->size--;
        }
    }
    void clear() {
        this->size = 0;
    };

    void move_xy(int16_t x, int16_t y) {
        for (size_t i = 0, max = this->size; i < max; ++i) {
            Widget2 * w = this->child_list[i];
            w->set_xy(x + w->dx(), y + w->dy());
        }
    }
    Widget2 * widget_at_pos(int16_t x, int16_t y) {
        Widget2 * ret = 0;
        for (size_t i = 0; i < this->size && ret == 0; ++i){
            if (this->child_list[i]->rect.contains_pt(x, y)){
                ret = this->child_list[i];
            }
        }
        return ret;
    }

    void draw(const Rect& clip)
    {
        for (size_t i = 0; i < this->size; ++i) {
            Widget2 *w = this->child_list[i];
            w->refresh(clip.intersect(w->rect));
        }
    }

    void draw_inner_free(const Rect& clip, int bg_color, Region & region)
    {
        for (size_t i = 0; i < this->size; ++i) {
            Rect rect = clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }
    }
    size_t find(Widget2 * w) {
        size_t pos = 0;
        for (pos = 0; pos < this->size; ++pos) {
            if (this->child_list[pos] == w) {
                break;
            }
        }
        return (pos == this->size)?0:pos;
    }
    size_t next(size_t n) {
        return (n >= this->size - 1)?0:(n+1);
    }
    size_t prev(size_t n) {
        return (n == 0)?(this->size - 1):(n-1);
    }

    Widget2 * next_focus(Widget2 * current_focus) {
        Widget2 * current = current_focus;
        size_t current_pos = this->find(current);
        size_t next = this->next(current_pos);
        while (((this->child_list[next]->tab_flag == Widget2::IGNORE_TAB)
                || (this->child_list[next]->focus_flag == Widget2::IGNORE_FOCUS))
               && (next != current_pos)) {
            next = this->next(next);
        }
        if (next != current_pos) {
            return this->child_list[next];
        }
        return NULL;
    }
    Widget2 * previous_focus(Widget2 * current_focus) {
        Widget2 * current = current_focus;
        size_t current_pos = this->find(current);
        size_t prev = this->prev(current_pos);
        while (((this->child_list[prev]->tab_flag == Widget2::IGNORE_TAB)
                || (this->child_list[prev]->focus_flag == Widget2::IGNORE_FOCUS))
               && (prev != current_pos)) {
            prev = this->prev(prev);
        }
        if (prev != current_pos) {
            return this->child_list[prev];
        }
        return NULL;
    }

};


// WidgetComposite is a WidgetParent and use Delegation to an implementation of CompositeInterface
class WidgetComposite: public WidgetParent {

public:

    WidgetComposite(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                    NotifyApi * notifier, int group_id = 0)
    : WidgetParent(drawable, rect, parent, notifier, group_id)
    {
        this->impl = new CompositeTable;
    }

    ~WidgetComposite() {
    }


    virtual void draw(const Rect& clip) {
        Rect new_clip = clip.intersect(this->rect);
        this->impl->draw(new_clip);
    }

    virtual void draw_inner_free(const Rect& clip, int bg_color) {
        Region region;
        region.rects.push_back(clip);

        this->impl->draw_inner_free(clip, bg_color, region);

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }


};
#endif

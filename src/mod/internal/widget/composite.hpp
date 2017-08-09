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
 *              Meng Tan, Raphael Zhou
 */

#pragma once

#include "widget.hpp"
#include "keyboard/keymap2.hpp"
#include "utils/colors.hpp"
#include "utils/log.hpp"
#include "utils/region.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

inline void fill_region(gdi::GraphicApi & drawable, const SubRegion & region, BGRColor bg_color) {
    for (Rect const & rect : region.rects) {
        drawable.draw(RDPOpaqueRect(rect, encode_color24()(bg_color)), rect, gdi::ColorCtx::depth24());
    }
}

class CompositeContainer {
public:
    virtual ~CompositeContainer() = default;

    enum { invalid_iterator = 0 };

    typedef void * iterator;

    virtual iterator add(Widget * w) = 0;
    virtual void remove(const Widget * w) = 0;

    virtual Widget * get(iterator iter) const = 0;

    virtual iterator get_first() = 0;
    virtual iterator get_last() = 0;

    virtual iterator get_previous(iterator iter, bool loop = false) = 0;
    virtual iterator get_next(iterator iter, bool loop = false) = 0;

    virtual iterator find(const Widget * w) = 0;

    virtual void clear() = 0;
};

class CompositeArray : public CompositeContainer {
    enum {
        MAX_CHILDREN_COUNT = 256
    };

    Widget * child_table[MAX_CHILDREN_COUNT];
    size_t    children_count;

public:
    CompositeArray() : child_table(), children_count(0) {
        for (int i = 0; i < MAX_CHILDREN_COUNT; i++) {
            REDASSERT(!child_table[i]);
        }
    }

    iterator add(Widget * w) override {
        REDASSERT(w);
        REDASSERT(this->children_count < MAX_CHILDREN_COUNT);
        this->child_table[this->children_count] = w;
        return static_cast<iterator>(&this->child_table[this->children_count++]);
    }
    void remove(const Widget * w) override {
        REDASSERT(w);
        REDASSERT(this->children_count);
        auto last = this->child_table + this->children_count;
        auto it = std::find(&this->child_table[0], last, w);
        REDASSERT(it != last);
        if (it != last) {
            auto new_last = std::copy(it+1, last, it);
            *new_last = nullptr;
            this->children_count--;
        }
    }

    Widget * get(iterator iter) const override {
        return *(static_cast<Widget **>(iter));
    }

    iterator get_first() override {
        if (!this->children_count) {
            return reinterpret_cast<iterator>(invalid_iterator);
        }

        return static_cast<iterator>(&this->child_table[0]);
    }
    iterator get_last() override {
        if (!this->children_count) {
            return reinterpret_cast<iterator>(invalid_iterator);
        }

        return static_cast<iterator>(&this->child_table[this->children_count - 1]);
    }

    iterator get_previous(iterator iter, bool loop = false) override {
        if (iter == this->get_first()) {
            if (loop) {
                iterator last;
                if ((last = this->get_last()) != iter) {
                    return last;
                }
            }
            return reinterpret_cast<iterator>(invalid_iterator);
        }

        return (static_cast<Widget **>(iter)) - 1;
    }
    iterator get_next(iterator iter, bool loop = false) override {
        if (iter == this->get_last()) {
            if (loop) {
                iterator frist;
                if ((frist = this->get_first()) != iter) {
                    return frist;
                }
            }
            return reinterpret_cast<iterator>(invalid_iterator);
        }

        return (static_cast<Widget **>(iter)) + 1;
    }

    iterator find(const Widget * w) override {
        for (size_t i = 0; i < this->children_count; i++) {
            if (this->child_table[i] == w) {
                return static_cast<iterator>(&this->child_table[i]);
            }
        }

        return reinterpret_cast<iterator>(invalid_iterator);
    }

    void clear()  override {
        this->children_count = 0;
    }
};  // class CompositeArray

class WidgetParent : public Widget {
    Widget * pressed;

    BGRColor bg_color;

protected:
    CompositeContainer * impl;

public:
    Widget * current_focus;

    WidgetParent(gdi::GraphicApi & drawable, Widget & parent,
                 NotifyApi * notifier, int group_id = 0)
        : Widget(drawable, parent, notifier, group_id)
        , pressed(nullptr)
        , bg_color(BLACK)
        , impl(nullptr)
        , current_focus(nullptr) {}

    ~WidgetParent() override {}

    void set_widget_focus(Widget * new_focused, int reason) {
        REDASSERT(new_focused);
        if (new_focused != this->current_focus) {
            if (this->current_focus) {
                this->current_focus->blur();
            }
            this->current_focus = new_focused;
        }

        this->current_focus->focus(reason);
    }

    void focus(int reason) override {
        const bool tmp_has_focus = this->has_focus;
        if (!this->has_focus) {
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);

            if (reason == focus_reason_tabkey) {
                this->current_focus = this->get_next_focus(nullptr, false);
            }
            else if (reason == focus_reason_backtabkey) {
                this->current_focus = this->get_previous_focus(nullptr, false);
            }
        }
        if (this->current_focus) {
            this->current_focus->focus(reason);
        }
        if (!tmp_has_focus) {
            this->rdp_input_invalidate(this->get_rect());
        }
    }
    void blur() override {
        if (this->has_focus) {
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
        }
        if (this->current_focus) {
            this->current_focus->blur();
        }
        this->rdp_input_invalidate(this->get_rect());
    }

    Widget * get_next_focus(Widget * w, bool loop) {
        CompositeContainer::iterator iter;
        if (!w) {
            REDASSERT(!loop);
            if ((iter = this->impl->get_first()) == reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
                return nullptr;
            }

            w = this->impl->get(iter);
            if ((w->tab_flag != Widget::IGNORE_TAB) && (w->focus_flag != Widget::IGNORE_FOCUS)) {
                return w;
            }
        }
        else {
            iter = this->impl->find(w);
            REDASSERT(iter != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator));
        }

        CompositeContainer::iterator future_focus_iter;
        while ((future_focus_iter = this->impl->get_next(iter, loop)) != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * future_focus_w = this->impl->get(future_focus_iter);
            if ((future_focus_w->tab_flag != Widget::IGNORE_TAB) && (future_focus_w->focus_flag != Widget::IGNORE_FOCUS)) {
                return future_focus_w;
            }

            if (future_focus_w == w) {
                break;
            }

            iter = future_focus_iter;
        }

        return nullptr;
    }
    Widget * get_previous_focus(Widget * w, bool loop) {
        CompositeContainer::iterator iter;
        if (!w) {
            REDASSERT(!loop);
            if ((iter = this->impl->get_last()) == reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
                return nullptr;
            }

            w = this->impl->get(iter);
            if ((w->tab_flag != Widget::IGNORE_TAB) && (w->focus_flag != Widget::IGNORE_FOCUS)) {
                return w;
            }
        }
        else {
            iter = this->impl->find(w);
            REDASSERT(iter != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator));
        }

        CompositeContainer::iterator future_focus_iter;
        while ((future_focus_iter = this->impl->get_previous(iter, loop)) != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * future_focus_w = this->impl->get(future_focus_iter);
            if ((future_focus_w->tab_flag != Widget::IGNORE_TAB) && (future_focus_w->focus_flag != Widget::IGNORE_FOCUS)) {
                return future_focus_w;
            }

            if (future_focus_w == w) {
                break;
            }

            iter = future_focus_iter;
        }

        return nullptr;
    }

    virtual void add_widget(Widget * w) {
        this->impl->add(w);

        if (!this->current_focus &&
            (w->tab_flag != Widget::IGNORE_TAB) && (w->focus_flag != Widget::IGNORE_FOCUS)) {
            this->current_focus = w;
        }
    }
    virtual void remove_widget(Widget * w) {
        if (this->current_focus == w) {
            Widget * future_focus_w;
            if ((future_focus_w = this->get_next_focus(w, false)) != nullptr) {
                this->current_focus = future_focus_w;
            }
            else if ((future_focus_w = this->get_previous_focus(w, false)) != nullptr) {
                this->current_focus = future_focus_w;
            }
            else {
                this->current_focus = nullptr;
            }
        }

        this->impl->remove(w);
    }
    virtual void clear() {
        this->impl->clear();

        this->current_focus = nullptr;
    }

    virtual void invalidate_children(Rect clip) {
        CompositeContainer::iterator iter_w_current = this->impl->get_first();
        while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * w = this->impl->get(iter_w_current);
            REDASSERT(w);

            Rect newr = clip.intersect(w->get_rect());

            if (!newr.isempty()) {
                w->rdp_input_invalidate(newr);
            }

            iter_w_current = this->impl->get_next(iter_w_current);
        }
    }

    virtual void refresh_children(Rect clip) {
        CompositeContainer::iterator iter_w_current = this->impl->get_first();
        while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * w = this->impl->get(iter_w_current);
            REDASSERT(w);

            Rect newr = clip.intersect(w->get_rect());

            if (!newr.isempty()) {
                w->refresh(newr);
            }

            iter_w_current = this->impl->get_next(iter_w_current);
        }
    }

    virtual void draw_inner_free(Rect clip, BGRColor bg_color) {
        SubRegion region;
        region.rects.push_back(clip.intersect(this->get_rect()));

        CompositeContainer::iterator iter_w_current = this->impl->get_first();
        while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * w = this->impl->get(iter_w_current);
            REDASSERT(w);

            Rect rect_widget = clip.intersect(w->get_rect());
            if (!rect_widget.isempty()) {
                region.subtract_rect(rect_widget);
            }

            iter_w_current = this->impl->get_next(iter_w_current);
        }

        ::fill_region(this->drawable, region, bg_color);
    }

    //virtual void hide_child(Rect clip, BGRColor bg_color) {
    //    SubRegion region;
    //
    //    CompositeContainer::iterator iter_w_current = this->impl->get_first();
    //    while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
    //        Widget * w = this->impl->get(iter_w_current);
    //        REDASSERT(w);
    //
    //        Rect rect_widget = clip.intersect(w->rect);
    //        if (!rect_widget.isempty()) {
    //            region.add_rect(rect_widget);
    //        }
    //
    //        iter_w_current = this->impl->get_next(iter_w_current);
    //    }
    //
    //    if (!region.rects.empty()) {
    //        ::fill_region(this->drawable, region, bg_color);
    //    }
    //}

    virtual BGRColor get_bg_color() const {
        return this->bg_color;
    }

    virtual void set_bg_color(BGRColor color) {
        this->bg_color = color;
    }

    void move_xy(int16_t x, int16_t y) override {
        this->set_xy(this->x() + x, this->y() + y);

        this->move_children_xy(x, y);
    }

    void move_children_xy(int16_t x, int16_t y) {
        CompositeContainer::iterator iter_w_first = this->impl->get_first();
        if (iter_w_first != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            CompositeContainer::iterator iter_w_current = iter_w_first;
            do {
                Widget * w = this->impl->get(iter_w_current);
                REDASSERT(w);
                w->move_xy(x, y);

                iter_w_current = this->impl->get_next(iter_w_current);
            }
            while ((iter_w_current != iter_w_first) &&
                   (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)));
        }
    }

    bool next_focus() override {
        if (this->current_focus) {
            if (this->current_focus->next_focus()) {
                return true;
            }

            Widget * future_focus_w = this->get_next_focus(this->current_focus, false);

            if (future_focus_w) {
                this->set_widget_focus(future_focus_w, focus_reason_tabkey);

                return true;
            }

            this->current_focus->blur();
            this->current_focus = this->get_next_focus(nullptr, false);
            REDASSERT(this->current_focus);
        }

        return false;
    }
    bool previous_focus() override {
        if (this->current_focus) {
            if (this->current_focus->previous_focus()) {
                return true;
            }

            Widget * future_focus_w = this->get_previous_focus(this->current_focus, false);

            if (future_focus_w) {
                this->set_widget_focus(future_focus_w, focus_reason_backtabkey);

                return true;
            }

            this->current_focus->blur();
            this->current_focus = this->get_previous_focus(nullptr, false);
            REDASSERT(this->current_focus);
        }

        return false;
    }

    Widget * widget_at_pos(int16_t x, int16_t y) override {
        if (!this->get_rect().contains_pt(x, y)) {
            return nullptr;
        }
        if (this->current_focus) {
            if (this->current_focus->get_rect().contains_pt(x, y)) {
                return this->current_focus;
            }
        }
        // Foreground widget is the last in the list.
        CompositeContainer::iterator iter_w_current = this->impl->get_last();
        while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
            Widget * w = this->impl->get(iter_w_current);
            REDASSERT(w);
            if (w->get_rect().contains_pt(x, y)) {
                return w;
            }

            iter_w_current = this->impl->get_previous(iter_w_current);
        }

        return nullptr;
    }

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->draw_inner_free(rect_intersect, this->get_bg_color());
            this->invalidate_children(rect_intersect);

            this->drawable.end_update();
        }
    }

    void refresh(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->draw_inner_free(rect_intersect, this->get_bg_color());
            this->refresh_children(rect_intersect);

            this->drawable.end_update();
        }
    }

    void rdp_input_scancode(long param1, long param2, long param3,
                            long param4, Keymap2 * keymap) override {
        if (!keymap->nb_kevent_available()) {
            if (this->current_focus) {
                this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
        }

        while (keymap->nb_kevent_available() > 0) {
            uint32_t nb_kevent = keymap->nb_kevent_available();
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
                if (this->current_focus) {
                    this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
                }
                break;
            }
            if (nb_kevent == keymap->nb_kevent_available()) {
                // this is to prevent infinite loop if the kevent is not consummed
                keymap->get_kevent();
            }
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        if (this->current_focus) {
            this->current_focus->rdp_input_unicode(unicode, flag);
        }
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
        Widget * w = this->widget_at_pos(x, y);

        // Mouse clic release
        // w could be null if mouse is located at an empty space
        if (device_flags == MOUSE_FLAG_BUTTON1) {
            if (this->pressed
                && (w != this->pressed)) {
                this->pressed->rdp_input_mouse(device_flags, x, y, keymap);
            }
            this->pressed = nullptr;
        }
        if (w) {
            // get focus when mouse clic
            if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
                this->pressed = w;
                if (/*(*/w->focus_flag != IGNORE_FOCUS/*) && (w != this->current_focus)*/) {
                    this->set_widget_focus(w, focus_reason_mousebutton1);
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
        else {
            Widget::rdp_input_mouse(device_flags, x, y, keymap);
        }
        if (device_flags == MOUSE_FLAG_MOVE && this->pressed) {
            this->pressed->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }
};

// WidgetComposite is a WidgetParent and use Delegation to an implementation of CompositeInterface
class WidgetComposite: public WidgetParent {
    CompositeArray composite_array;

public:
    WidgetComposite(gdi::GraphicApi & drawable, Widget & parent,
                    NotifyApi * notifier, int group_id = 0)
    : WidgetParent(drawable, parent, notifier, group_id) {
        this->impl = & composite_array;
    }

    ~WidgetComposite() override {}
};

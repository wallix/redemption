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

#include "mod/internal/widget/composite.hpp"
#include "keyboard/keymap.hpp"
#include "utils/region.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"


void fill_region(gdi::GraphicApi & drawable, const SubRegion & region, Widget::Color bg_color)
{
    for (Rect const & rect : region.rects) {
        drawable.draw(RDPOpaqueRect(rect, bg_color), rect, gdi::ColorCtx::depth24());
    }
}


CompositeArray::CompositeArray()
: capacity(checked_int(std::size(fixed_table)))
, p(fixed_table)
{}

CompositeArray::~CompositeArray()
{
    dealloc_table();
}

void CompositeArray::dealloc_table()
{
    if (p != fixed_table) {
        delete[] p;
    }
}

Widget** CompositeArray::begin()
{
    return p;
}

Widget** CompositeArray::end()
{
    return p + count;
}

void CompositeArray::add(Widget & w)
{
    if (REDEMPTION_UNLIKELY(count == capacity)) {
        const std::size_t new_size = std::size_t(capacity) * 4;
        auto* new_array = new Widget*[new_size];
        std::copy(p, p + count, new_array);
        dealloc_table();
        p = new_array;
        capacity = checked_int(new_size);
    }

    p[count] = &w;
    count++;
}

void CompositeArray::remove(Widget const & w)
{
    for (int i = 0; i < count; ++i) {
        if (p[i] == &w) {
            std::copy(p + i + 1, p + count, p + i);
            --count;
            break;
        }
    }
}

Widget** CompositeArray::find(Widget const & w)
{
    for (int i = 0; i < this->count; ++i) {
        if (p[i] == &w) {
            return p + i;
        }
    }

    return end();
}

void CompositeArray::clear()
{
    this->count = 0;
}


WidgetComposite::WidgetComposite(gdi::GraphicApi & drawable, Focusable focusable)
    : Widget(drawable, focusable)
    , pressed(nullptr)
    , bg_color(BLACK)
    , current_focus(nullptr)
{}

WidgetComposite::~WidgetComposite() = default;

void WidgetComposite::set_widget_focus(Widget & new_focused, int reason)
{
    if (&new_focused != this->current_focus) {
        if (this->current_focus) {
            this->current_focus->blur();
        }
        this->current_focus = &new_focused;
    }

    this->current_focus->focus(reason);
}

void WidgetComposite::focus(int reason)
{
    const bool tmp_has_focus = this->has_focus;
    if (!this->has_focus) {
        this->has_focus = true;

        if (reason == focus_reason_tabkey) {
            this->current_focus = this->get_next_focus(nullptr);
        }
        else if (reason == focus_reason_backtabkey) {
            this->current_focus = this->get_previous_focus(nullptr);
        }
    }
    if (this->current_focus) {
        this->current_focus->focus(reason);
    }
    if (!tmp_has_focus) {
        this->rdp_input_invalidate(this->get_rect());
    }
}

void WidgetComposite::blur()
{
    if (this->has_focus) {
        this->has_focus = false;
    }
    if (this->current_focus) {
        this->current_focus->blur();
    }
    this->rdp_input_invalidate(this->get_rect());
}

Widget * WidgetComposite::get_next_focus(Widget * w)
{
    Widget** it = this->widgets.begin();
    Widget** last = this->widgets.end();

    if (!w) {
        if (it == last) {
            return nullptr;
        }

        w = *it;
        if (w->focusable == Focusable::Yes) {
            return w;
        }
    }
    else {
        it = this->widgets.find(*w);

        if (it == last) {
            return nullptr;
        }
    }

    while (++it != last) {
        Widget* future_focus_w = *it;
        if (future_focus_w->focusable == Focusable::Yes) {
            return future_focus_w;
        }
    }

    return nullptr;
}

Widget * WidgetComposite::get_previous_focus(Widget * w)
{
    Widget** it = this->widgets.begin();
    Widget** last = this->widgets.end();

    if (!w) {
        if (it == last) {
            return nullptr;
        }

        w = *(last - 1);
        if (w->focusable == Focusable::Yes) {
            return w;
        }

        it = last;
    }
    else {
        it = this->widgets.find(*w);

        if (it == last) {
            return nullptr;
        }
    }

    Widget** first = this->widgets.begin();
    if (it != first) {
        do {
            --it;
            Widget* future_focus_w = *it;
            if (future_focus_w->focusable == Focusable::Yes) {
                return future_focus_w;
            }
        } while (it != first);
    }

    return nullptr;
}

void WidgetComposite::init_focus()
{
    this->has_focus = true;
    if (this->current_focus) {
        this->current_focus->init_focus();
    }
}

void WidgetComposite::add_widget(Widget & w, HasFocus has_focus)
{
    this->widgets.add(w);

    if (w.focusable == Focusable::Yes && (has_focus == HasFocus::Yes || !this->current_focus)) {
        this->current_focus = &w;
    }
}

void WidgetComposite::remove_widget(Widget & w)
{
    if (this->current_focus == &w) {
        Widget * future_focus_w = this->get_next_focus(&w);
        if (not future_focus_w) {
            future_focus_w = this->get_previous_focus(&w);
        }
        this->current_focus = future_focus_w;
    }

    this->widgets.remove(w);
}

bool WidgetComposite::contains_widget(Widget & w)
{
    return this->widgets.find(w) != this->widgets.end();
}

void WidgetComposite::clear_widget()
{
    this->widgets.clear();

    this->current_focus = nullptr;
}

void WidgetComposite::invalidate_children(Rect clip)
{
    for (Widget* w : this->widgets) {
        Rect rect = clip.intersect(w->get_rect());
        if (!rect.isempty()) {
            w->rdp_input_invalidate(rect);
        }
    }
}

void WidgetComposite::draw_inner_free(Rect clip, Color bg_color)
{
    SubRegion region;
    region.add_rect(clip.intersect(this->get_rect()));

    for (Widget* w : this->widgets) {
        Rect rect = clip.intersect(w->get_rect());
        if (!rect.isempty()) {
            region.subtract_rect(rect);
        }
    }

    ::fill_region(this->drawable, region, bg_color);
}

void WidgetComposite::move_xy(int16_t x, int16_t y)
{
    this->set_xy(this->x() + x, this->y() + y);

    this->move_children_xy(x, y);
}

void WidgetComposite::move_children_xy(int16_t x, int16_t y)
{
    for (Widget* w : this->widgets) {
        w->move_xy(x, y);
    }
}

bool WidgetComposite::next_focus()
{
    if (this->current_focus) {
        if (this->current_focus->next_focus()) {
            return true;
        }

        Widget * future_focus_w = this->get_next_focus(this->current_focus);

        if (future_focus_w) {
            this->set_widget_focus(*future_focus_w, focus_reason_tabkey);

            return true;
        }

        this->current_focus->blur();
        this->current_focus = this->get_next_focus(nullptr);
        assert(this->current_focus);
    }

    return false;
}

bool WidgetComposite::previous_focus()
{
    if (this->current_focus) {
        if (this->current_focus->previous_focus()) {
            return true;
        }

        Widget * future_focus_w = this->get_previous_focus(this->current_focus);

        if (future_focus_w) {
            this->set_widget_focus(*future_focus_w, focus_reason_backtabkey);

            return true;
        }

        this->current_focus->blur();
        this->current_focus = this->get_previous_focus(nullptr);
        assert(this->current_focus);
    }

    return false;
}

Widget * WidgetComposite::widget_at_pos(int16_t x, int16_t y)
{
    if (!this->get_rect().contains_pt(x, y)) {
        return nullptr;
    }

    if (this->current_focus) {
        if (this->current_focus->get_rect().contains_pt(x, y)) {
            return this->current_focus;
        }
    }

    Widget** first = this->widgets.begin();
    Widget** last = this->widgets.end();

    // Foreground widget is the last in the list.
    if (first != last) {
        do {
            --last;
            Widget* w = *last;
            if (w->get_rect().contains_pt(x, y)) {
                return w;
            }
        } while (first != last);
    }

    return nullptr;
}

void WidgetComposite::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->draw_inner_free(rect_intersect, this->get_bg_color());
        this->invalidate_children(rect_intersect);
    }
}

void WidgetComposite::rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap)
{
    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
    switch (keymap.last_kevent()){
        case Keymap::KEvent::Tab:
            this->next_focus();
            break;

        case Keymap::KEvent::BackTab:
            this->previous_focus();
            break;

        default:
            if (this->current_focus) {
                this->current_focus->rdp_input_scancode(flags, scancode, event_time, keymap);
            }
            break;
    }
    REDEMPTION_DIAGNOSTIC_POP()
}

void WidgetComposite::rdp_input_unicode(KbdFlags flag, uint16_t unicode)
{
    if (this->current_focus) {
        this->current_focus->rdp_input_unicode(flag, unicode);
    }
}

void WidgetComposite::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
        x = this->old_mouse_x;
        y = this->old_mouse_y;
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }

    Widget * w = this->widget_at_pos(x, y);

    // Mouse clic release
    // w could be null if mouse is located at an empty space
    if (device_flags == MOUSE_FLAG_BUTTON1) {
        if (this->pressed
            && (w != this->pressed)) {
            this->pressed->rdp_input_mouse(device_flags, x, y);
        }
        this->pressed = nullptr;
    }
    if (w) {
        // get focus when mouse clic
        if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
            this->pressed = w;
            if (w->focusable == Focusable::Yes) {
                this->set_widget_focus(*w, focus_reason_mousebutton1);
            }
        }
        w->rdp_input_mouse(device_flags, x, y);
    }
    else {
        Widget::rdp_input_mouse(device_flags, x, y);
    }
    if (device_flags == MOUSE_FLAG_MOVE && this->pressed) {
        this->pressed->rdp_input_mouse(device_flags, x, y);
    }
}

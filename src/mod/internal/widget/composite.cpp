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
#include "keyboard/keymap2.hpp"
#include "utils/log.hpp"
#include "utils/region.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

void fill_region(gdi::GraphicApi & drawable, const SubRegion & region, BGRColor bg_color)
{
    for (Rect const & rect : region.rects) {
        drawable.draw(RDPOpaqueRect(rect, encode_color24()(bg_color)), rect, gdi::ColorCtx::depth24());
    }
}


CompositeArray::CompositeArray() = default;

CompositeArray::iterator CompositeArray::add(Widget * w)
{
    assert(w);
    assert(this->children_count < MAX_CHILDREN_COUNT);
    this->child_table[this->children_count] = w;
    return static_cast<iterator>(&this->child_table[this->children_count++]);
}

void CompositeArray::remove(const Widget * w)
{
    assert(w);
    assert(this->children_count);
    auto last = this->child_table + this->children_count;
    auto it = std::find(&this->child_table[0], last, w);
    assert(it != last);
    if (it != last) {
        auto new_last = std::copy(it+1, last, it);
        *new_last = nullptr;
        this->children_count--;
    }
}

Widget * CompositeArray::get(iterator iter) const
{
    return *(static_cast<Widget **>(iter));
}

CompositeArray::iterator CompositeArray::get_first()
{
    if (!this->children_count) {
        return reinterpret_cast<iterator>(invalid_iterator);
    }

    return static_cast<iterator>(&this->child_table[0]);
}

CompositeArray::iterator CompositeArray::get_last()
{
    if (!this->children_count) {
        return reinterpret_cast<iterator>(invalid_iterator);
    }

    return static_cast<iterator>(&this->child_table[this->children_count - 1]);
}

CompositeArray::iterator CompositeArray::get_previous(iterator iter)
{
    if (iter == this->get_first()) {
        return reinterpret_cast<iterator>(invalid_iterator);
    }

    return (static_cast<Widget **>(iter)) - 1;
}

CompositeArray::iterator CompositeArray::get_next(iterator iter)
{
    if (iter == this->get_last()) {
        return reinterpret_cast<iterator>(invalid_iterator);
    }

    return (static_cast<Widget **>(iter)) + 1;
}

CompositeArray::iterator CompositeArray::find(const Widget * w)
{
    for (size_t i = 0; i < this->children_count; i++) {
        if (this->child_table[i] == w) {
            return static_cast<iterator>(&this->child_table[i]);
        }
    }

    return reinterpret_cast<iterator>(invalid_iterator);
}

void CompositeArray::clear()
{
    this->children_count = 0;
}


WidgetParent::WidgetParent(
    gdi::GraphicApi & drawable, Widget & parent, NotifyApi * notifier, int group_id
)
    : Widget(drawable, parent, notifier, group_id)
    , pressed(nullptr)
    , bg_color(BLACK)
    , impl(nullptr)
    , current_focus(nullptr)
{}

WidgetParent::~WidgetParent() = default;

void WidgetParent::set_widget_focus(Widget * new_focused, int reason)
{
    assert(new_focused);
    if (new_focused != this->current_focus) {
        if (this->current_focus) {
            this->current_focus->blur();
        }
        this->current_focus = new_focused;
    }

    this->current_focus->focus(reason);
}

void WidgetParent::focus(int reason)
{
    const bool tmp_has_focus = this->has_focus;
    if (!this->has_focus) {
        this->has_focus = true;
        this->send_notify(NOTIFY_FOCUS_BEGIN);

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

void WidgetParent::blur()
{
    if (this->has_focus) {
        this->has_focus = false;
        this->send_notify(NOTIFY_FOCUS_END);
    }
    if (this->current_focus) {
        this->current_focus->blur();
    }
    this->rdp_input_invalidate(this->get_rect());
}

Widget * WidgetParent::get_next_focus(Widget * w)
{
    CompositeContainer::iterator iter;
    if (!w) {
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
        assert(iter != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator));
    }

    CompositeContainer::iterator future_focus_iter;
    while ((future_focus_iter = this->impl->get_next(iter)) != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
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

Widget * WidgetParent::get_previous_focus(Widget * w)
{
    CompositeContainer::iterator iter;
    if (!w) {
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
        assert(iter != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator));
    }

    CompositeContainer::iterator future_focus_iter;
    while ((future_focus_iter = this->impl->get_previous(iter)) != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
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

void WidgetParent::add_widget(Widget * w)
{
    this->impl->add(w);

    if (!this->current_focus &&
        (w->tab_flag != Widget::IGNORE_TAB) && (w->focus_flag != Widget::IGNORE_FOCUS)) {
        this->current_focus = w;
    }
}

void WidgetParent::remove_widget(Widget * w)
{
    if (this->current_focus == w) {
        Widget * future_focus_w;
        if ((future_focus_w = this->get_next_focus(w)) != nullptr) {
            this->current_focus = future_focus_w;
        }
        else if ((future_focus_w = this->get_previous_focus(w)) != nullptr) {
            this->current_focus = future_focus_w;
        }
        else {
            this->current_focus = nullptr;
        }
    }

    this->impl->remove(w);
}

void WidgetParent::clear()
{
    this->impl->clear();

    this->current_focus = nullptr;
}

void WidgetParent::invalidate_children(Rect clip)
{
    CompositeContainer::iterator iter_w_current = this->impl->get_first();
    while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
        Widget * w = this->impl->get(iter_w_current);
        assert(w);

        Rect newr = clip.intersect(w->get_rect());

        if (!newr.isempty()) {
            w->rdp_input_invalidate(newr);
        }

        iter_w_current = this->impl->get_next(iter_w_current);
    }
}

void WidgetParent::refresh_children(Rect clip)
{
    CompositeContainer::iterator iter_w_current = this->impl->get_first();
    while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
        Widget * w = this->impl->get(iter_w_current);
        assert(w);

        Rect newr = clip.intersect(w->get_rect());

        if (!newr.isempty()) {
            w->refresh(newr);
        }

        iter_w_current = this->impl->get_next(iter_w_current);
    }
}

void WidgetParent::draw_inner_free(Rect clip, BGRColor bg_color)
{
    SubRegion region;
    region.add_rect(clip.intersect(this->get_rect()));

    CompositeContainer::iterator iter_w_current = this->impl->get_first();
    while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
        Widget * w = this->impl->get(iter_w_current);
        assert(w);

        Rect rect_widget = clip.intersect(w->get_rect());
        if (!rect_widget.isempty()) {
            region.subtract_rect(rect_widget);
        }

        iter_w_current = this->impl->get_next(iter_w_current);
    }

    ::fill_region(this->drawable, region, bg_color);
}

//void hide_child(Rect clip, BGRColor bg_color) {
//    SubRegion region;
//
//    CompositeContainer::iterator iter_w_current = this->impl->get_first();
//    while (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
//        Widget * w = this->impl->get(iter_w_current);
//        assert(w);
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

void WidgetParent::move_xy(int16_t x, int16_t y)
{
    this->set_xy(this->x() + x, this->y() + y);

    this->move_children_xy(x, y);
}

void WidgetParent::move_children_xy(int16_t x, int16_t y)
{
    CompositeContainer::iterator iter_w_first = this->impl->get_first();
    if (iter_w_first != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)) {
        CompositeContainer::iterator iter_w_current = iter_w_first;
        do {
            Widget * w = this->impl->get(iter_w_current);
            assert(w);
            w->move_xy(x, y);

            iter_w_current = this->impl->get_next(iter_w_current);
        }
        while ((iter_w_current != iter_w_first) &&
                (iter_w_current != reinterpret_cast<CompositeContainer::iterator>(CompositeContainer::invalid_iterator)));
    }
}

bool WidgetParent::next_focus()
{
    if (this->current_focus) {
        if (this->current_focus->next_focus()) {
            return true;
        }

        Widget * future_focus_w = this->get_next_focus(this->current_focus);

        if (future_focus_w) {
            this->set_widget_focus(future_focus_w, focus_reason_tabkey);

            return true;
        }

        this->current_focus->blur();
        this->current_focus = this->get_next_focus(nullptr);
        assert(this->current_focus);
    }

    return false;
}

bool WidgetParent::previous_focus()
{
    if (this->current_focus) {
        if (this->current_focus->previous_focus()) {
            return true;
        }

        Widget * future_focus_w = this->get_previous_focus(this->current_focus);

        if (future_focus_w) {
            this->set_widget_focus(future_focus_w, focus_reason_backtabkey);

            return true;
        }

        this->current_focus->blur();
        this->current_focus = this->get_previous_focus(nullptr);
        assert(this->current_focus);
    }

    return false;
}

Widget * WidgetParent::widget_at_pos(int16_t x, int16_t y)
{
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
        assert(w);
        if (w->get_rect().contains_pt(x, y)) {
            return w;
        }

        iter_w_current = this->impl->get_previous(iter_w_current);
    }

    return nullptr;
}

void WidgetParent::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();

        this->draw_inner_free(rect_intersect, this->get_bg_color());
        this->invalidate_children(rect_intersect);

        this->drawable.end_update();
    }
}

void WidgetParent::refresh(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();

        this->draw_inner_free(rect_intersect, this->get_bg_color());
        this->refresh_children(rect_intersect);

        this->drawable.end_update();
    }
}

void WidgetParent::rdp_input_scancode(
    long param1, long param2, long param3,
    long param4, Keymap2 * keymap)
{
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

void WidgetParent::rdp_input_unicode(uint16_t unicode, uint16_t flag)
{
    if (this->current_focus) {
        this->current_focus->rdp_input_unicode(unicode, flag);
    }
}

void WidgetParent::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
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

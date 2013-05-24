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

#if !defined(REDEMPTION_MOD_WIDGET2_WIDGET_COMPOSITE_HPP_)
#define REDEMPTION_MOD_WIDGET2_WIDGET_COMPOSITE_HPP_

#include <vector>
#include <algorithm>
#include "widget.hpp"
#include "keymap2.hpp"
#include <region.hpp>

class WidgetComposite : public Widget2
{
    typedef std::vector<Widget2*>::iterator position_t;
public:
    std::vector<Widget2*> child_list;

    WidgetComposite(ModApi * drawable, const Rect& rect, Widget2 * parent,
                    NotifyApi * notifier, int group_id = 0)
    : Widget2(drawable, rect, parent, notifier, group_id)
    , child_list()
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
    }

    virtual ~WidgetComposite()
    {
        for (size_t i = 0; i < this->child_list.size(); ++i) {
            if (this->child_list[i]->parent == this) {
                this->child_list[i]->parent = 0;
            }
        }
    }

public:
    virtual void set_xy(int16_t x, int16_t y)
    {
        int16_t xx = x - this->dx();
        int16_t yy = y - this->dy();
        for (size_t i = 0, max = this->child_list.size(); i < max; ++i) {
            Widget2 * w = this->child_list[i];
            w->set_xy(xx + w->dx(), yy + w->dy());
        }
        Widget2::set_xy(x, y);
    }

    virtual void set_wh(uint16_t width, uint16_t height)
    {
        for (size_t i = 0, max = this->child_list.size(); i < max; ++i) {
            Widget2 * w = this->child_list[i];
            w->set_wh(width, height);
        }
        Widget2::set_wh(width, height);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        if (!this->rect.contains_pt(x, y))
            return 0;
        Widget2 * ret = this->child_at_pos(x, y);
        return ret ? ret : this;
    }

    virtual Widget2 * widget_focused()
    {
        Widget2 * ret = this->direct_child_focused();
        if (ret)
        {
            Widget2 * tmp = ret->widget_focused();
            if (tmp)
                ret = tmp;
        }
        return ret;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (this->rect.contains_pt(x, y)) {
            Widget2 * w = this->child_at_pos(x,y);
            if (w) {
                w->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }
    }

    //BEGIN focus manager @{
    position_t next_position_of_widget_with_focus()
    {
        return std::find(this->child_list.begin(), this->child_list.end(),
                         this->widget_with_focus);
    }

    virtual bool next_focus()
    {
        struct focus_manager {
            static position_t next_in(position_t first, position_t last)
            {
                for (; first < last; ++first) {
                    if ((*first)->tab_flag & NORMAL_TAB) {
                        std::cout << ("normal tab") << std::endl;
                        break ;
                    }
                    if ((*first)->tab_flag & DELEGATE_CONTROL_TAB) {
                        std::cout << ("delegate tab") << std::endl;
                        if ((*first)->next_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        struct Trace {
            Trace() { std::cout << ("[ next_focus") << std::endl; }
            ~Trace() { std::cout << ("] next_focus") << std::endl; }
        } t;

        if (this->widget_with_focus != NULL && this->widget_with_focus != this) {
            if (this->widget_with_focus->next_focus()) {
                return true;
            }
        }

       std::cout << (typeid(*this).name()) << std::endl;
        std::cout << "tab_flag: " << (tab_flag) << std::endl;
        position_t pos = this->next_position_of_widget_with_focus();
        if (pos != this->child_list.end()) {
            std::cout << ("find yes") << std::endl;
            position_t pos2 = focus_manager::next_in(pos+1, this->child_list.end());
            bool ok = (pos2 != this->child_list.end());
            if (!ok) {
                std::cout << ("to end") << std::endl;
                pos2 = focus_manager::next_in(this->child_list.begin(), pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                std::cout << ("set widget_with_focus") << std::endl;
                if (this->widget_with_focus == *pos2) {
                    std::cout << ("!!!!!! error switch with self") << std::endl;
                }
                this->switch_focus_with(*pos2);
                std::cout << ("ok") << std::endl;
                return true;
            }
            else std::cout << ("to begin, not find") << std::endl;
        } else {
            std::cout << ("find no") << std::endl;
            pos = focus_manager::next_in(this->child_list.begin(), this->child_list.end());
            if (pos != this->child_list.end()) {
                std::cout << ("set widget_with_focus 2") << std::endl;
                this->switch_focus_with(*pos);
                std::cout << ("ok") << std::endl;
                return true;
            } else  std::cout << ("nada") << std::endl;
        }

        if ((!this->tab_flag & NO_DELEGATE_PARENT) && this->parent) {
            this->parent->next_focus();
        }
        return false;
    }

    position_t previous_position_of_widget_with_focus()
    {
        position_t first = this->child_list.begin();
        position_t last = this->child_list.end();
        --first;
        while (--last != first && *last != this->widget_with_focus) {
        }
        return last;
    }

    virtual bool previous_focus()
    {
        struct focus_manager {
            static position_t previous_in(position_t first, position_t last)
            {
                for (; first != last; --first) {
                    if ((*first)->tab_flag & NORMAL_TAB) {
                        std::cout << ("normal tab") << std::endl;
                        break ;
                    }
                    if ((*first)->tab_flag & DELEGATE_CONTROL_TAB) {
                        std::cout << ("delegate tab") << std::endl;
                        if ((*first)->previous_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        struct Trace {
            Trace() { std::cout << ("[ previous_focus") << std::endl; }
            ~Trace() { std::cout << ("] previous_focus") << std::endl; }
        } t;

        if (this->widget_with_focus != NULL && this->widget_with_focus != this) {
            if (this->widget_with_focus->previous_focus()) {
                return true;
            }
        }

       std::cout << (typeid(*this).name()) << std::endl;
        std::cout << "tab_flag: " << (tab_flag) << std::endl;
        position_t pos = this->previous_position_of_widget_with_focus();
        if (pos != this->child_list.begin()-1) {
            std::cout << ("find yes") << std::endl;
            position_t pos2 = focus_manager::previous_in(pos-1, this->child_list.begin()-1);
            bool ok = (pos2 != this->child_list.begin()-1);
            if (!ok) {
                std::cout << ("to end") << std::endl;
                pos2 = focus_manager::previous_in(this->child_list.end()-1, pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                std::cout << ("set widget_with_focus") << std::endl;
                if (this->widget_with_focus == *pos2) {
                    std::cout << ("!!!!!! error switch with self") << std::endl;
                }
                this->switch_focus_with(*pos2);
                std::cout << ("ok") << std::endl;
                return true;
            }
            else std::cout << ("to begin, not find") << std::endl;
        } else {
            std::cout << ("find no") << std::endl;
            pos = focus_manager::previous_in(this->child_list.end()-1, this->child_list.begin()-1);
            if (pos != this->child_list.begin()-1) {
                std::cout << ("set widget_with_focus 2") << std::endl;
                this->switch_focus_with(*pos);
                std::cout << ("ok") << std::endl;
                return true;
            } else  std::cout << ("nada") << std::endl;
        }

        if ((!this->tab_flag & NO_DELEGATE_PARENT) && this->parent) {
            this->parent->previous_focus();
        }
        return false;
    }
    //END focus manager @}

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this->widget_with_focus != NULL && this->widget_with_focus != this) {
            this->widget_with_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
        else {
            Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }

    bool detach_widget(Widget2 * widget, bool active_previous_widget = false)
    {
        for (size_t i = 0; i < this->child_list.size(); ++i) {
            if (this->child_list[i] == widget) {
                if (widget->parent == this) {
                    widget->parent = 0;
                }
                this->child_list[i] = this->child_list[this->child_list.size()-1];
                this->child_list.pop_back();

                if (active_previous_widget) {
                    if (this->old_widget_with_focus == widget) {
                        this->old_widget_with_focus = NULL;
                    }
                    if (this->widget_with_focus == widget) {
                        this->widget_with_focus = this->old_widget_with_focus;
                        if (this->widget_with_focus) {
                            this->widget_with_focus->focus(widget);
                        }
                    }
                }
                return true;
            }
        }
        return false;
    }

    virtual void notify(Widget2* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (event == NOTIFY_FOCUS_BEGIN) {
            Widget2 * old = this->widget_with_focus;
            this->widget_with_focus = widget;
            if (this->parent && this->parent->widget_with_focus != this) {
                this->focus(old);
            }
        }
        else {
            Widget2::notify(widget, event, param, param2);
        }
    }

#if 0
    static bool switch_focus(Widget2 * old_focus, Widget2 * new_focus)
    {
        bool res = true;
        new_focus->has_focus = true;
        new_focus->notify_self(NOTIFY_FOCUS_BEGIN);
        new_focus->notify_parent(FOCUS_BEGIN);
        return res;
    }

    bool _control_childs_tab(Widget2 * old, std::size_t n, OptionTab dtab)
    {
        Widget2 * w = this->child_list[n];
        if (w->tab_flag == NORMAL_TAB) {
            this->switch_focus(old, w);
            return true;
        } else if (
            w->tab_flag & DELEGATE_CONTROL_TAB
            && w->type & TYPE_WND
            && static_cast<WidgetComposite*>(w)->control_tab(dtab, old)
        ) {
            return true;
        }
        return false;
    }

    bool _control_tab_impl(Widget2 * old, std::size_t n, std::size_t last)
    {
        for (; n < last; ++n) {
            if (this->_control_childs_tab(old, n, REWIND_TAB)) {
                return true;
            }
        }
        return false;
    }

    bool _control_backtab_impl(Widget2 * old, std::size_t n, std::size_t last)
    {
        for (; n >= last; --n) {
            if (this->_control_childs_tab(old, n, REWIND_BACKTAB)) {
                return true;
            }
        }
        return false;
    }

    bool control_tab(OptionTab dtab, Widget2 * pold = 0)
    {
        size_t size = this->child_list.size();
        if (size != 0) {
            size_t idx = this->has_focus ? this->direct_idx_focused() : -1u;
            bool nidx = (idx == -1u);
            if (nidx)
                idx = 0;
            Widget2 * w = this->child_list[idx];
            Widget2 * old = pold ? pold : w;
            if (w->tab_flag & DELEGATE_CONTROL_TAB) {
                if (static_cast<WidgetComposite*>(w)->control_tab(dtab, old)) {
                    return true;
                }
            }

            if (dtab == REWIND_TAB
            ? this->_control_tab_impl(old, idx+1, size)
            : this->_control_backtab_impl(old, idx-1, 0)) {
                return true;
            }

            if (!pold && (dtab == REWIND_TAB
            ? this->_control_tab_impl(old, 0, idx)
            : this->_control_backtab_impl(old, size-1, idx+1))) {
                return true;
            }

            return !pold && !this->parent
            && (dtab == REWIND_TAB
            ? this->focus_on_first()
            : this->focus_on_last());
        }
        return false;
    }

    bool _focus_on_impl(OptionTab dtab, std::size_t n)
    {
        Widget2 * w = this->child_list[n];
        if (w->tab_flag == NORMAL_TAB) {
            w->has_focus = true;
            w->notify_self(NOTIFY_FOCUS_BEGIN);
            w->notify_parent(FOCUS_BEGIN);
            return true;
        } else if (w->tab_flag & DELEGATE_CONTROL_TAB) {
            WidgetComposite* wi = static_cast<WidgetComposite*>(w);
            if (dtab == REWIND_TAB? wi->focus_on_first():wi->focus_on_last()) {
                return true;
            }
        }
        return false;
    }

    bool focus_on_first()
    {
        size_t size = this->child_list.size();
        for (size_t n = 0; n < size; ++n) {
            if (this->_focus_on_impl(REWIND_TAB, n)) {
                return true;
            }
        }
        return false;
    }

    bool focus_on_last()
    {
        size_t size = this->child_list.size();
        while (0 < size--) {
            if (this->_focus_on_impl(REWIND_BACKTAB, size)) {
                return true;
            }
        }
        return false;
    }

    void send_event_to_children(EventType event, int param, int param2, Keymap2 * keymap)
    {
        for (std::size_t i = 0; i < this->child_list.size(); ++i) {
            if (this->child_list[i]->has_focus)
                this->child_list[i]->send_event(event, param, param2, keymap);
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == WM_DRAW){
            this->refresh(Rect(0,0,this->rect.cx, this->rect.cy));
        } else if (event == KEYDOWN && this->tab_flag != IGNORE_TAB) {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_TAB:
                    this->control_tab(REWIND_TAB);
                    break;
                case Keymap2::KEVENT_BACKTAB:
                    this->control_tab(REWIND_BACKTAB);
                    break;
                default:
                    this->send_event_to_children(event, param, param2, keymap);
                    break;
            }
        } else {
            this->send_event_to_children(event, param, param2, keymap);
        }
    }

    virtual void notify(int group_id, EventType event)
    {
        if (event == FOCUS_BEGIN){
            for (std::size_t i = 0; i < this->child_list.size(); ++i)
            {
                Widget2 * wchild = this->child_list[i];
                if (wchild->has_focus && wchild->id != group_id) {
                    wchild->has_focus = false;
                    wchild->notify_self(NOTIFY_FOCUS_END);
                }
            }
            if (false == this->has_focus) {
                this->has_focus = true;
                this->notify_parent(FOCUS_BEGIN);
            }
        } else {
            this->Widget2::notify(group_id, event);
        }
    }
#endif

    virtual void draw(const Rect& clip)
    {
        Rect new_clip = clip.intersect(this->rect);
        std::size_t size = this->child_list.size();

        for (std::size_t i = 0; i < size; ++i) {
            Widget2 *w = this->child_list[i];
            w->refresh(new_clip.intersect(w->rect));
        }
    }

    void draw_inner_free(const Rect& clip, int bg_color)
    {
        Region region;
        region.rects.push_back(clip);

        for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
            Rect rect = clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable->draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }

protected:
    Widget2 * direct_child_focused() const
    {
        for (std::size_t i = 0; i < this->child_list.size(); ++i)
        {
            if (this->child_list[i]->has_focus)
                return this->child_list[i];
        }
        return 0;
    }

    size_t direct_idx_focused() const
    {
        for (std::size_t i = 0; i < this->child_list.size(); ++i)
        {
            if (this->child_list[i]->has_focus)
                return i;
        }
        return -1;
    }

    Widget2 * get_child_by_group_id(int group_id) const
    {
        for (size_t i = 0; i < this->child_list.size(); i++)
        {
            struct Widget2 * w = this->child_list[i];
            if (w->group_id == group_id)
                return w;
        }
        return 0;
    }

    virtual Widget2 * child_at_pos(int16_t x, int16_t y)
    {
        Widget2 * ret = 0;
        std::size_t size = this->child_list.size();
        for (std::size_t i = 0; i < size && ret == 0; ++i)
        {
            ret = this->child_list[i]->widget_at_pos(x, y);
        }
        return ret;
    }
};

#endif

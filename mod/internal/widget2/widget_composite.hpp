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
#include "widget.hpp"
#include <region.hpp>
#include "keymap2.hpp"

class WidgetComposite : public Widget
{
public:
    std::vector<Widget*> child_list;

    WidgetComposite(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier, int id = 0)
    : Widget(drawable, rect, parent, type, notifier, id)
    , child_list()
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
    }

    virtual ~WidgetComposite()
    {
        for (size_t i = 0; i < this->child_list.size(); ++i) {
            this->child_list[i]->parent = 0;
        }
    }

protected:
    virtual void attach_widget(Widget * widget)
    {
        this->child_list.push_back(widget);
    }

    virtual void detach_widget(Widget * widget)
    {
        size_t size = this->child_list.size();
        for (size_t i = 0; i < size; ++i) {
            if (this->child_list[i] == widget){
                this->child_list[i] = this->child_list[size - 1];
                this->child_list.resize(size - 1);
                widget->parent = 0;
                return ;
            }
        }
    }

public:
    virtual Widget * widget_at_pos(int x, int y)
    {
        if (!this->rect.contains_pt(x, y))
            return 0;
        Widget* ret = 0;
        x -= this->dx();
        y -= this->dy();
        for (std::size_t i = 0; i < this->child_list.size() && ret == 0; ++i)
        {
            ret = this->child_list[i]->widget_at_pos(x, y);
        }
        return ret;
    }

    virtual Widget* widget_focused()
    {
        Widget* ret =  this->direct_child_focused();
        if (ret)
        {
            Widget* tmp = ret->widget_focused();
            if (tmp)
                ret = tmp;
        }
        return ret;
    }

    static bool switch_focus(Widget * old_focus, Widget * new_focus)
    {
        bool res = true;
        new_focus->has_focus = true;
        new_focus->notify_self(NOTIFY_FOCUS_BEGIN);
        new_focus->notify_parent(FOCUS_BEGIN);
        return res;
    }

    bool _control_childs_tab(Widget * old, std::size_t n, OptionTab dtab)
    {
        Widget * w = this->child_list[n];
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

    bool _control_tab_impl(Widget * old, std::size_t n, std::size_t last)
    {
        for (; n < last; ++n) {
            if (this->_control_childs_tab(old, n, REWIND_TAB)) {
                return true;
            }
        }
        return false;
    }

    bool _control_backtab_impl(Widget * old, std::size_t n, std::size_t last)
    {
        for (; n >= last; --n) {
            if (this->_control_childs_tab(old, n, REWIND_BACKTAB)) {
                return true;
            }
        }
        return false;
    }

    bool control_tab(OptionTab dtab, Widget * pold = 0)
    {
        size_t size = this->child_list.size();
        if (size != 0) {
            size_t idx = this->has_focus ? this->direct_idx_focused() : -1u;
            bool nidx = (idx == -1u);
            if (nidx)
                idx = 0;
            Widget * w = this->child_list[idx];
            Widget * old = pold ? pold : w;
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
        Widget * w = this->child_list[n];
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

    virtual void notify(int id, EventType event)
    {
        if (event == FOCUS_BEGIN){
            for (std::size_t i = 0; i < this->child_list.size(); ++i)
            {
                Widget * wchild = this->child_list[i];
                if (wchild->has_focus && wchild->id != id) {
                    wchild->has_focus = false;
                    wchild->notify_self(NOTIFY_FOCUS_END);
                }
            }
            if (false == this->has_focus) {
                this->has_focus = true;
                this->notify_parent(FOCUS_BEGIN);
            }
        } else {
            this->Widget::notify(id, event);
        }
    }

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        Region region;
        region.rects.push_back(rect);
        Rect clip(xclip, yclip, rect.cx, rect.cy);
        for (std::size_t i = 0; i < this->child_list.size(); ++i) {
            Widget *w = this->child_list[i];
            Rect tmp = rect.intersect(w->rect);
            if (!tmp.isempty()){
                region.subtract_rect(tmp);
                if (w->drawable) {
                    Rect wrect(tmp.x - w->rect.x, tmp.y - w->rect.y, tmp.cx, tmp.cy);
                    if (!wrect.isempty()){
                        Rect new_clip = clip.intersect(Rect(
                            x + w->rect.x + wrect.x,
                            y + w->rect.y + wrect.y,
                            wrect.cx,
                            wrect.cy
                        ));
                        if (!new_clip.isempty()) {
                            w->drawable->begin_update();
                            w->draw(wrect, x + w->rect.x, y + w->rect.y, new_clip.x, new_clip.y);
                            w->drawable->end_update();
                        }
                    }
                }
            }
        }
        for (size_t i = 0, max = region.rects.size(); i < max; ++i) {
            this->drawable->draw(
                RDPOpaqueRect(
                    region.rects[i].offset(x,y),
                    this->bg_color
                ), clip
            );
        }
    }

protected:
    Widget* direct_child_focused() const
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

    Widget* get_child_by_id(int id) const
    {
        for (size_t i = 0; i < this->child_list.size(); i++)
        {
            struct Widget * w = this->child_list[i];
            if (w->id == id)
                return w;
        }
        return 0;
    }
};

#endif

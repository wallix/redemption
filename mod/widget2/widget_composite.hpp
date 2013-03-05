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

    ~WidgetComposite()
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
        for (size_t i = 0; i < this->child_list.size(); ++i) {
            if (this->child_list[i] == widget){
                this->child_list[i] = this->child_list[this->child_list.size() - 1];
                this->child_list.resize(this->child_list.size() - 1);
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

    bool switch_focus(Widget * old_focus, Widget * new_focus)
    {
        bool res = true;
        old_focus->has_focus = false;
        old_focus->notify_self(NOTIFY_FOCUS_END);
        new_focus->has_focus = true;
        new_focus->notify_self(NOTIFY_FOCUS_BEGIN);
        return res;
    }

    static bool control_tab(Widget * w)
    {
        if (w->type == TYPE_WND) {
            WidgetComposite * win = static_cast<WidgetComposite*>(w);
            size_t idx = win->direct_idx_focused();
            if (idx != -1u) {
                size_t size = win->child_list.size();
                for (size_t n = idx + 1; n < size; ++n) {
                    Widget * w = win->child_list[n];
                    if (w->tab_flag == NORMAL_TAB) {
                        win->switch_focus(win->child_list[idx], w);
                        return true;
                    } else if (w->tab_flag == DELEGATE_CONTROL_TAB) {
                        if (win->control_tab(w)) {
                            return true;
                        }
                    }
                }
            }
        } else {

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
//         } else if (event == KEYDOWN) {
//             size_t idx = this->direct_idx_focused();
//             if (idx != -1u) {
//                 for (size_t n = 0; n < idx; ++n) {
//                     Widget * w = this->child_list[n];
//                     if (w->tab_flag == NORMAL_TAB) {
//                         idx = n;
//                         break;
//                     } else if (w->tab_flag == DELEGATE_CONTROL_TAB) {
//                         w->send_event(event, param, param2, keymap);
//                         break;
//                     }
//                 }
//                 size_t idx_next = idx+1 == this->child_list.size() ? 0 : idx+1;
//
//             }
//             switch (keymap->top_kevent()) {
//                 case Keymap2::KEVENT_TAB:
//
//                     break;
//                 case Keymap2::KEVENT_BACKTAB:
//
//                     break;
//                 default:
//                     this->send_event_to_children(event, param, param2, keymap);
//                     break;
//             }
        } else {
            this->send_event_to_children(event, param, param2, keymap);
        }
    }

    virtual void notify(int id, EventType event)
    {
        if (event == FOCUS_BEGIN && this->has_focus == true){
            this->notify_self(event);
        } else {
            this->Widget::notify(id, event);
        }
    }

    virtual void draw(const Rect& rect, uint16_t x_screen, uint16_t y_screen, const Rect& clip_screen)
    {
        Rect clip = rect.intersect(Rect(
            0,0, clip_screen.cx, clip_screen.cy
        ));
        Region region;
        region.rects.push_back(clip);
        for (std::size_t i = 0; i < this->child_list.size(); ++i) {
            Widget *p = this->child_list[i];
            Rect tmp = clip.intersect(p->rect);
            if (!tmp.isempty()){
                region.subtract_rect(tmp);
                this->refresh_child(p, Rect(0,0,tmp.cx,tmp.cy), x_screen, y_screen, clip_screen);
            }
        }
        for (size_t i = 0, max = region.rects.size(); i < max; ++i) {
            this->Widget::draw(region.rects[i], x_screen, y_screen, clip_screen);
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
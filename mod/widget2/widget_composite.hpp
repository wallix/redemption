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

#include "widget.hpp"
#include <vector>

class WidgetComposite : public Widget
{
public:
    std::vector<Widget*> child_list;

    WidgetComposite(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier)
    : Widget(drawable, rect, parent, type, notifier)
    , child_list()
    {}

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

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        for (std::size_t i = 0; i < this->child_list.size(); ++i)
        {
            if (this->child_list[i]->has_focus)
                this->child_list[i]->send_event(event, param, param2, keymap);
        }
    }

    virtual void notify(Widget* w, EventType event)
    {
        if (event == FOCUS_BEGIN && this->has_focus == true){
            this->notify_self(event);
        } else {
            this->Widget::notify(w, event);
        }
    }

    virtual void redraw(const Rect & clip)
    {
        this->draw(clip);
        this->notify(this, WM_DRAW);

        size_t count = this->child_list.size();
        for (size_t i = 0; i < count; i++) {
            Widget * b = this->child_list[i];
            b->redraw(b->rect.wh());
        }
    }

    void addWidget(Widget* w) ///TODO
    {
        this->child_list.push_back(w);
        w->parent = this;
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
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

#if !defined(REDEMPTION_MOD_WIDGET2_WINDOW_HPP_)
#define REDEMPTION_MOD_WIDGET2_WINDOW_HPP_

#include "widget_composite.hpp"
#include "label.hpp"
#include <keymap2.hpp>

class Window : public WidgetComposite
{
public:
    WidgetLabel titlebar;

    Window(ModApi * drawable, const Rect& rect, Widget * parent, NotifyApi * notifier, const char * caption, int id = 0)
    : WidgetComposite(drawable, rect, parent, Widget::TYPE_WND, notifier, id)
    , titlebar(drawable, Rect(0,0, rect.cx, 15), this, 0, caption, -1)
    {}

    virtual ~Window()
    {}

protected:
    bool switch_focus(Widget * old_focus, Widget * new_focus)
    {
        bool res = true;
        old_focus->has_focus = false;
        old_focus->notify_self(NOTIFY_FOCUS_END);
        new_focus->has_focus = true;
        new_focus->notify_self(NOTIFY_FOCUS_BEGIN);
        return res;
    }

public:
    virtual void send_event(EventType event, int param, int param2, Keymap2* keymap)
    {
        if (event == KEYDOWN) {
            size_t idx = this->direct_idx_focused();
            if (idx == -1u && !this->child_list.empty()){
                idx = 0;
            }
            if (idx != -1u){
                size_t size = this->child_list.size();
                Widget * w = this->child_list[idx];
                switch (keymap->top_kevent()) {
                    case Keymap2::KEVENT_TAB:
                        for (size_t i = (idx+1)%size; i != idx; i = (i+1)%size){
                            if (this->switch_focus(w, this->child_list[i])){
                                break;
                            }
                        }
                        break;
                    case Keymap2::KEVENT_BACKTAB:
                        for (size_t i = (idx-1)%size; i != idx; i = (i-1)%size){
                            if (this->switch_focus(w, this->child_list[i])){
                                break;
                            }
                        }
                        break;
                    default:
                        w->send_event(event, param, param2, keymap);
                        break;
                }
            }
        } else {
            this->WidgetComposite::send_event(event, param, param2, keymap);
        }
    }

    virtual void notify(int id, EventType event)
    {
        if (event == FOCUS_BEGIN){
            for (std::size_t i = 0; i < this->child_list.size(); ++i)
            {
                Widget * wchild = this->child_list[i];
                if (wchild->has_focus && wchild->id != id){
                    wchild->has_focus = false;
                    wchild->notify_self(NOTIFY_FOCUS_END);
                }
            }
            if (false == this->has_focus){
                this->has_focus = true;
                this->notify_parent(FOCUS_BEGIN);
            }
        } else {
            this->Widget::notify(id, event);
        }
    }
};

#endif
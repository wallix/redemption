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

TODO("child_list should not be directly accessed");

class WidgetComposite : public Widget2
{
    typedef std::vector<Widget2*>::iterator position_t;
public:
    std::vector<Widget2*> child_list;

    WidgetComposite(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                    NotifyApi * notifier, int group_id = 0)
    : Widget2(drawable, rect, parent, notifier, group_id)
    , child_list()
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
    }

    virtual ~WidgetComposite()
    {
        if (!this->child_list.empty()) {
            throw Error(ERR_WIDGET_INVALID_COMPOSITE_DESTROY);
        }
    }

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

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y)
    {
        if (!this->rect.contains_pt(x, y))
            return 0;
        if (this->current_focus) {
            if (this->current_focus->rect.contains_pt(x, y)) {
                return this->current_focus;
            }
        }

        Widget2 * ret = 0;
        std::size_t size = this->child_list.size();
        for (std::size_t i = 0; i < size && ret == 0; ++i){
            if (this->child_list[i]->rect.contains_pt(x, y)){
                ret = this->child_list[i];
            }
        }
        return ret;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (this->rect.contains_pt(x, y)) {
            Widget2 * w = this->widget_at_pos(x,y);
            if (w) {
                w->rdp_input_mouse(device_flags, x, y, keymap);
            }
        }
    }

    //BEGIN focus manager @{
    position_t next_position_of_current_focus()
    {
        return std::find(this->child_list.begin(), this->child_list.end(),
                         this->current_focus);
    }

    virtual Widget2 * search_focus() {
        Widget2 * ret = 0;
        std::size_t size = this->child_list.size();
        for (std::size_t i = 0; i < size && ret == 0; ++i){
            if (this->child_list[i]->has_focus)
                ret = this->child_list[i];
        }
        return ret;
    }

    virtual bool next_focus()
    {
        struct focus_manager {
            static position_t next_in(position_t first, position_t last)
            {
                for (; first < last; ++first) {
                    if ((*first)->tab_flag & NORMAL_TAB) {
                        break ;
                    }
                    if ((*first)->tab_flag & DELEGATE_CONTROL_TAB) {
                        if ((*first)->next_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        if (this->current_focus != NULL && this->current_focus != this) {
            if (this->current_focus->next_focus()) {
                return true;
            }
        }

        position_t pos = this->next_position_of_current_focus();
        if (pos != this->child_list.end()) {
            position_t pos2 = focus_manager::next_in(pos+1, this->child_list.end());
            bool ok = (pos2 != this->child_list.end());
            if (!ok) {
                pos2 = focus_manager::next_in(this->child_list.begin(), pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                if (this->current_focus != *pos2) {
                    this->switch_focus_with(*pos2);
                }
                return true;
            }
        } else {
            pos = focus_manager::next_in(this->child_list.begin(), this->child_list.end());
            if (pos != this->child_list.end()) {
                this->switch_focus_with(*pos);
                return true;
            }
        }
        if ((!this->tab_flag & NO_DELEGATE_PARENT) && (&this->parent != this)) {
            this->parent.next_focus();
        }
        return false;
    }

    position_t previous_position_of_current_focus()
    {
        position_t first = this->child_list.begin();
        position_t last = this->child_list.end();
        --first;
        while (--last != first && *last != this->current_focus) {
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
                        break ;
                    }
                    if ((*first)->tab_flag & DELEGATE_CONTROL_TAB) {
                        if ((*first)->previous_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        if (this->current_focus != NULL && this->current_focus != this) {
            if (this->current_focus->previous_focus()) {
                return true;
            }
        }

        position_t pos = this->previous_position_of_current_focus();
        if (pos != this->child_list.begin()-1) {
            position_t pos2 = focus_manager::previous_in(pos-1, this->child_list.begin()-1);
            bool ok = (pos2 != this->child_list.begin()-1);
            if (!ok) {
                pos2 = focus_manager::previous_in(this->child_list.end()-1, pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                if (this->current_focus != *pos2) {
                    this->switch_focus_with(*pos2);
                }
                return true;
            }
        } else {
            pos = focus_manager::previous_in(this->child_list.end()-1, this->child_list.begin()-1);
            if (pos != this->child_list.begin()-1) {
                this->switch_focus_with(*pos);
                return true;
            }
        }

        if ((!this->tab_flag & NO_DELEGATE_PARENT) && (&this->parent != this)) {
            this->parent.previous_focus();
        }
        return false;
    }
    //END focus manager @}

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this->current_focus != NULL && this->current_focus != this) {
            this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
        else {
            Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }



    virtual void notify(Widget2* widget, notify_event_t event, long unsigned int param, long unsigned int param2)
    {
        if (event == NOTIFY_FOCUS_BEGIN) {
            this->current_focus = widget;
        }
        else {
            Widget2::notify(widget, event, param, param2);
        }
    }

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
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }
    }

protected:
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
};

#endif

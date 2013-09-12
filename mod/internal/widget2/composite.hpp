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
class CompositeInterface {
public:
    virtual ~CompositeInterface() {}
    virtual void add_widget_compo(Widget2 * w) = 0;
    virtual void remove_widget_compo(Widget2 * w) = 0;
    virtual void clear_compo() = 0;
    virtual void set_xy_compo(int16_t x, int16_t y) = 0;
    virtual Widget2 * widget_at_pos_compo(int16_t x, int16_t y) = 0;
    virtual bool next_focus_compo(WidgetParent * thiswidget) = 0;
    virtual bool previous_focus_compo(WidgetParent * thiswidget) = 0;
    virtual void draw_compo(const Rect& clip) = 0;
    virtual void draw_inner_free_compo(const Rect& clip, int bg_color, Region & region) = 0;
};

// CompositeVector is an Implementation of CompositeInterface
class CompositeVector : public CompositeInterface
{
    typedef std::vector<Widget2*>::iterator position_t;
    std::vector<Widget2*> child_list;
public:
    CompositeVector()
        : child_list()
    {
    }

    virtual ~CompositeVector()
    {
        if (!this->child_list.empty()) {
            throw Error(ERR_WIDGET_INVALID_COMPOSITE_DESTROY);
        }
    }

    virtual void add_widget_compo(Widget2 * w) {
        this->child_list.push_back(w);
    }
    virtual void remove_widget_compo(Widget2 * w) {
        std::size_t size = this->child_list.size();
        if (size != 0 && this->child_list[size - 1] == w) {
            this->child_list.pop_back();
        }
    }

    virtual void clear_compo() {
        this->child_list.clear();
    }

    virtual void set_xy_compo(int16_t xx, int16_t yy)
    {
        for (size_t i = 0, max = this->child_list.size(); i < max; ++i) {
            Widget2 * w = this->child_list[i];
            w->set_xy(xx + w->dx(), yy + w->dy());
        }
    }

    virtual Widget2 * widget_at_pos_compo(int16_t x, int16_t y)
    {
        Widget2 * ret = 0;
        std::size_t size = this->child_list.size();
        for (std::size_t i = 0; i < size && ret == 0; ++i){
            if (this->child_list[i]->rect.contains_pt(x, y)){
                ret = this->child_list[i];
            }
        }
        return ret;
    }

    //BEGIN focus manager @{
    position_t next_position_of_current_focus(Widget2 * w)
    {
        return std::find(this->child_list.begin(), this->child_list.end(),
                         w);
    }

    virtual bool next_focus_compo(WidgetParent * thiswidget)
    {
        struct focus_manager {
            static position_t next_in(position_t first, position_t last)
            {
                for (; first < last; ++first) {
                    if ((*first)->tab_flag & Widget2::NORMAL_TAB) {
                        break ;
                    }
                    if ((*first)->tab_flag & Widget2::DELEGATE_CONTROL_TAB) {
                        if ((*first)->next_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        if (thiswidget->current_focus != NULL && thiswidget->current_focus != thiswidget) {
            if (thiswidget->current_focus->next_focus()) {
                return true;
            }
        }

        position_t pos = this->next_position_of_current_focus(thiswidget->current_focus);
        if (pos != this->child_list.end()) {
            position_t pos2 = focus_manager::next_in(pos+1, this->child_list.end());
            bool ok = (pos2 != this->child_list.end());
            if (!ok) {
                pos2 = focus_manager::next_in(this->child_list.begin(), pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                if (thiswidget->current_focus != *pos2) {
                    thiswidget->switch_focus_with(*pos2);
                }
                return true;
            }
        } else {
            pos = focus_manager::next_in(this->child_list.begin(), this->child_list.end());
            if (pos != this->child_list.end()) {
                thiswidget->switch_focus_with(*pos);
                return true;
            }
        }
        if ((!thiswidget->tab_flag & Widget2::NO_DELEGATE_PARENT) && (&thiswidget->parent != thiswidget)) {
            thiswidget->parent.next_focus();
        }
        return false;
    }

    position_t previous_position_of_current_focus(Widget2 * w)
    {
        position_t first = this->child_list.begin();
        position_t last = this->child_list.end();
        --first;
        while (--last != first && *last != w) {
        }
        return last;
    }

    virtual bool previous_focus_compo(WidgetParent * thiswidget)
    {
        struct focus_manager {
            static position_t previous_in(position_t first, position_t last)
            {
                for (; first != last; --first) {
                    if ((*first)->tab_flag & Widget2::NORMAL_TAB) {
                        break ;
                    }
                    if ((*first)->tab_flag & Widget2::DELEGATE_CONTROL_TAB) {
                        if ((*first)->previous_focus()) {
                            break ;
                        }
                    }
                }
                return first;
            }
        };

        if (thiswidget->current_focus != NULL && thiswidget->current_focus != thiswidget) {
            if (thiswidget->current_focus->previous_focus()) {
                return true;
            }
        }

        position_t pos = this->previous_position_of_current_focus(thiswidget->current_focus);
        if (pos != this->child_list.begin()-1) {
            position_t pos2 = focus_manager::previous_in(pos-1, this->child_list.begin()-1);
            bool ok = (pos2 != this->child_list.begin()-1);
            if (!ok) {
                pos2 = focus_manager::previous_in(this->child_list.end()-1, pos);
                ok = (pos2 != pos);
            }
            if (ok) {
                if (thiswidget->current_focus != *pos2) {
                    thiswidget->switch_focus_with(*pos2);
                }
                return true;
            }
        } else {
            pos = focus_manager::previous_in(this->child_list.end()-1, this->child_list.begin()-1);
            if (pos != this->child_list.begin()-1) {
                thiswidget->switch_focus_with(*pos);
                return true;
            }
        }

        if ((!thiswidget->tab_flag & Widget2::NO_DELEGATE_PARENT) && (&thiswidget->parent != thiswidget)) {
            thiswidget->parent.previous_focus();
        }
        return false;
    }
    //END focus manager @}



    virtual void draw_compo(const Rect& clip)
    {
        std::size_t size = this->child_list.size();

        for (std::size_t i = 0; i < size; ++i) {
            Widget2 *w = this->child_list[i];
            w->refresh(clip.intersect(w->rect));
        }
    }

    void draw_inner_free_compo(const Rect& clip, int bg_color, Region & region)
    {
        for (std::size_t i = 0, size = this->child_list.size(); i < size; ++i) {
            Rect rect = clip.intersect(this->child_list[i]->rect);

            if (!rect.isempty()) {
                region.subtract_rect(rect);
            }
        }

    }


};

// WidgetComposite is a WidgetParent and use Delegation to an implementation of CompositeInterface
class WidgetComposite: public WidgetParent {
    CompositeInterface * impl;

public:

    WidgetComposite(DrawApi & drawable, const Rect& rect, Widget2 & parent,
                             NotifyApi * notifier, int group_id = 0)
    : WidgetParent(drawable, rect, parent, notifier, group_id)
    {
        this->tab_flag = DELEGATE_CONTROL_TAB;
        this->impl = new CompositeVector;
    }

    ~WidgetComposite() {
        if (this->impl) {
            delete this->impl;
            this->impl = NULL;
        }
    }

    virtual void add_widget(Widget2 * w) {
        this->impl->add_widget_compo(w);
    }
    virtual void remove_widget(Widget2 * w) {
        this->impl->remove_widget_compo(w);
    }
    virtual void clear() {
        this->impl->clear_compo();
    }

    virtual void set_xy(int16_t x, int16_t y) {
        int16_t xx = x - this->dx();
        int16_t yy = y - this->dy();
        this->impl->set_xy_compo(xx, yy);
        WidgetParent::set_xy(x, y);
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (!this->rect.contains_pt(x, y))
            return 0;
        if (this->current_focus) {
            if (this->current_focus->rect.contains_pt(x, y)) {
                return this->current_focus;
            }
        }
        return this->impl->widget_at_pos_compo(x, y);
    }

    virtual bool next_focus() {
        return this->impl->next_focus_compo(this);
    }

    virtual bool previous_focus() {
        return this->impl->previous_focus_compo(this);
    }

    virtual void draw(const Rect& clip) {
        Rect new_clip = clip.intersect(this->rect);
        this->impl->draw_compo(new_clip);
    }

    virtual void draw_inner_free(const Rect& clip, int bg_color) {
        Region region;
        region.rects.push_back(clip);

        this->impl->draw_inner_free_compo(clip, bg_color, region);

        for (std::size_t i = 0, size = region.rects.size(); i < size; ++i) {
            this->drawable.draw(RDPOpaqueRect(region.rects[i], bg_color), region.rects[i]);
        }

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

};
#endif

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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP

#include "composite.hpp"

#include <typeinfo>

class FocusPropagation {
    class minivector {
        Widget2* data[32];
        size_t data_size;

    public:
        minivector()
        : data_size(0)
        {}

        void clear()
        { this->data_size = 0; }

        size_t size() const
        { return this->data_size; }

        void push_back(Widget2 * w)
        { this->data[this->data_size++] = w; }

        Widget2 * & operator[](size_t n)
        { return data[n]; }
    };

    minivector focus_parents;
    minivector new_focus_parents;

public:
    FocusPropagation()
    {}

    void active_focus(Widget2 * screen, Widget2 * new_focused, int policy = 0)
    {
        Widget2 * w2 = new_focused->parent;
        if (0 == w2) {
            return ;
        }

        this->new_focus_parents.clear();
        this->focus_parents.clear();
        Widget2 * w = screen;
        while (w->widget_with_focus && w->widget_with_focus->has_focus && w->focus_flag != Widget2::FORCE_FOCUS) {
            this->focus_parents.push_back(w);
            w = w->widget_with_focus;
        }
        if (w != screen && Widget2::FORCE_FOCUS != w->focus_flag) {
            Widget2 ** last = &this->focus_parents[this->focus_parents.size()];
            this->new_focus_parents.push_back(new_focused);
            switch (new_focused->focus_flag) {
                case Widget2::NORMAL_FOCUS:
                    while (w2) {
                        this->new_focus_parents.push_back(w2);
                        if (std::find<>(&this->focus_parents[0], last, w2) != last) {
                            for (size_t n = this->new_focus_parents.size() - 1; n > 0; --n) {
                                if (this->new_focus_parents[n]->widget_with_focus != this->new_focus_parents[n-1]) {
                                    this->new_focus_parents[n]->switch_focus_with(this->new_focus_parents[n-1], policy);
                                }
                                else if (false == this->new_focus_parents[n]->has_focus) {
                                    this->new_focus_parents[n-1]->focus(0, policy);
                                }
                            }
                            break;
                        }
                        w2 = w2->parent;
                    }
                    break;
                default:
                    break;
            }
        }
    }
};

class WidgetScreen : public WidgetComposite
{
    Widget2 * widget_pressed;
    FocusPropagation focus_propagation;

public:
    WidgetScreen(DrawApi& drawable, uint16_t width, uint16_t height, NotifyApi * notifier = NULL)
    : WidgetComposite(drawable, Rect(0, 0, width, height), NULL, notifier)
    , widget_pressed(0)
    {
    }

    virtual ~WidgetScreen()
    {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        Widget2 * tmp_widget_with_focus = this->widget_with_focus;
        bool same_window = this->widget_with_focus && this->widget_with_focus->rect.contains_pt(x, y);
        Widget2 * w = same_window ? this->widget_with_focus : this->child_at_pos(x, y);
        if (device_flags == MOUSE_FLAG_BUTTON1) {
            if (this->widget_pressed && w != this->widget_pressed) {
                this->widget_pressed->rdp_input_mouse(device_flags, x, y, keymap);
            }
            else {
                this->widget_pressed = 0;
            }
        }
        if (w) {
            if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
                this->widget_pressed = w;
                focus_propagation.active_focus(this, w, 2);
                if (!same_window && tmp_widget_with_focus->focus_flag != Widget2::FORCE_FOCUS) {
                    w->refresh(tmp_widget_with_focus->rect);
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this != this->widget_with_focus) {
            this->widget_with_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }

        for (uint32_t n = keymap->nb_kevent_available(); n ; --n) {
            keymap->get_kevent();
        }
    }

    virtual void draw(const Rect& clip)
    {
        this->WidgetComposite::draw(clip);
        this->WidgetComposite::draw_inner_free(clip, BLACK);
    }
};

#endif

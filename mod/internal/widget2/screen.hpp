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

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_SCREEN_HPP

#include "widget_composite.hpp"

class WidgetScreen : public WidgetComposite
{
    Widget2 * widget_pressed;

public:
    WidgetScreen(ModApi * drawable, uint16_t width, uint16_t height, NotifyApi * notifier = NULL)
    : WidgetComposite(drawable, Rect(0, 0, width, height), NULL, notifier)
    , widget_pressed(0)
    {
    }

    virtual ~WidgetScreen()
    {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        Widget2 * w = this->child_at_pos(x, y);
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
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        Widget2 * wfocus = this->widget_with_focus ? this->widget_with_focus : this;
        if (keymap->nb_char_available() > 0) {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_TAB:
                    keymap->get_kevent();
                    while (false == wfocus->next_focus() && wfocus->parent && wfocus->tab_flag != NO_DELEGATE_CHILD_TAB) {
                        wfocus = wfocus->parent;
                    }
                    break;
                case Keymap2::KEVENT_BACKTAB:
                    keymap->get_kevent();
                    while (false == wfocus->previous_focus() && wfocus->parent && wfocus->tab_flag != NO_DELEGATE_CHILD_TAB) {
                        wfocus = wfocus->parent;
                    }
                    break;
                default:
                    break;
            }
        }

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
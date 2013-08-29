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

class WidgetScreen : public WidgetComposite
{
public:
    WidgetScreen(DrawApi& drawable, uint16_t width, uint16_t height, NotifyApi * notifier = NULL)
    : WidgetComposite(drawable, Rect(0, 0, width, height), NULL, notifier)
    {
    }

    virtual ~WidgetScreen()
    {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        Widget2 * w = this->widget_at_pos(x, y);
        if (w){
            if (device_flags & MOUSE_FLAG_BUTTON1) {
                if ((w->focus_flag != IGNORE_FOCUS) && (w != this->current_focus)){
                    if (this->current_focus) {
                        this->current_focus->blur(); 
                    }
                    this->current_focus = w;
                    this->current_focus->focus();
                }
            }
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this->current_focus) {
            this->current_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
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

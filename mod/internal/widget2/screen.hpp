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
public:
    WidgetScreen(ModApi * drawable, uint16_t width, uint16_t height, NotifyApi * notifier = NULL)
    : WidgetComposite(drawable, Rect(0, 0, width, height), NULL, notifier)
    {
    }

    virtual ~WidgetScreen()
    {}

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (Widget2 * w = this->child_at_pos(x, y)) {
            w->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (this == this->widget_with_focus)
            return ;
        std::cout << "widget_with_focus: " << (widget_with_focus) << std::endl;
        this->widget_with_focus->rdp_input_scancode(param1, param2, param3, param4, keymap);
    }
};

#endif
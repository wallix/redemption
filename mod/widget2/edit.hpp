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

#if !defined(REDEMPTION_MOD_WIDGET2_EDIT_HPP_)
#define REDEMPTION_MOD_WIDGET2_EDIT_HPP_

#include "widget.hpp"

class WidgetEdit : public Widget
{
public:
    WidgetEdit(ModApi* drawable, int width, int height, Widget* parent)
    : Widget(drawable, width, height, parent, Widget::TYPE_EDIT)
    {}

    virtual void draw(const Rect & rect)
    {
        (void)rect;
    }

    virtual void def_proc(EventType event, int param, Keymap2* keymap)
    {
        std::cout << "event: " << event << std::endl;
        if (event == KEYDOWN)
        {
            this->notify(this, TEXT_CHANGED);
        }
    }
};

#endif
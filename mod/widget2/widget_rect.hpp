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

#if !defined(REDEMPTION_MOD_WIDGET2_WIDGET_RECT_HPP)
#define REDEMPTION_MOD_WIDGET2_WIDGET_RECT_HPP

#include "widget.hpp"

class WidgetRect : public Widget
{
public:
    int color;

public:
    WidgetRect(ModApi * drawable, const Rect& rect, Widget * parent, NotifyApi * notifier, int id = 0, int color = BLACK)
    : Widget(drawable, rect, parent, type, notifier, id)
    , color(color)
    {
    }

    virtual void draw(const Rect& clip)
    {
        screen_position s = this->position_in_screen();
        this->drawable->draw(
            RDPOpaqueRect(
                Rect(this->rect.x + s.x, this->rect.y + s.y, s.clip.cx, s.clip.cy),
                this->color
            ), s.clip
        );
    }
};

#endif

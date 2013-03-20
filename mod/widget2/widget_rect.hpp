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
    int bg_color;
    int fg_color;

public:
    WidgetRect(ModApi * drawable, const Rect& rect, Widget * parent, int type, NotifyApi * notifier, int id = 0, int bgcolor = BLACK, int fgcolor = WHITE)
    : Widget(drawable, rect, parent, type, notifier, id)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    {
    }

    void draw_rect(const Rect& rect, int16_t x, int16_t y, const Rect & clip)
    {
        this->drawable->draw(
            RDPOpaqueRect(
                rect.offset(x,y),
                this->bg_color
            ), clip
        );
    }

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        this->draw_rect(rect, x, y, Rect(xclip, yclip, rect.cx, rect.cy));
    }
};

#endif
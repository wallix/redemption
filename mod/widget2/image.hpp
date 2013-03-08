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

#if !defined(REDEMPTION_MOD_WIDGET2_IMAGE_HPP)
#define REDEMPTION_MOD_WIDGET2_IMAGE_HPP

#include "widget.hpp"

class WidgetImage : public Widget
{
    Bitmap bmp;

public:
    WidgetImage(ModApi* drawable, int x, int y, const char * filename, Widget* parent, NotifyApi* notifier, int id = 0)
    : Widget(drawable, Rect(), parent, TYPE_IMAGE, notifier, id)
    , bmp(filename)
    {
        this->rect.x = x;
        this->rect.y = y;
        this->rect.cx = this->bmp.cx;
        this->rect.cy = this->bmp.cy;
    }

    virtual ~WidgetImage()
    {}

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        this->drawable->draw(
            RDPMemBlt(
                0,
                rect.offset(x, y),
                0xCC,
                xclip - x,
                yclip - y,
                0
            ),
            Rect(xclip, yclip, rect.cx, rect.cy),
            this->bmp
        );
    }
};

#endif
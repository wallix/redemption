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

#include"widget.hpp"

class WidgetImage : public Widget
{
    Bitmap bmp;

public:
    WidgetImage(ModApi* drawable, int x, int y, const char * filename, Widget* parent, NotifyApi* notifier, int id = 0)
    : Widget(drawable, Rect(), parent, notifier, id)
    , bmp(filename)
    {
        this->rect.x = x;
        this->rect.y = y;
        this->rect.cx = this->bmp.cx;
        this->rect.cy = this->bmp.cy;
    }

    virtual ~WidgetImage()
    {}

    virtual void draw(const Rect& clip)
    {
        Rect screen_clip = this->position_in_screen(clip);
        this->drawable->draw(
            RDPMemBlt(
                0,
                Rect(std::max<int16_t>(screen_clip.x, 0),
                     std::max<int16_t>(screen_clip.y, 0),
                     screen_clip.cx,
                     screen_clip.cy),
                0xCC,
                (screen_clip.x < 0 ? -screen_clip.x : 0) + clip.x,
                (screen_clip.y < 0 ? -screen_clip.y : 0) + clip.y,
                0
            ),
            clip.offset(this->rect.x, this->rect.y).intersect(this->rect),
            this->bmp
        );
    }
};

#endif
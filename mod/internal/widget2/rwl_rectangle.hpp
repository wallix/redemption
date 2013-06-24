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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_MOD_INTERNAL_WIDGET2_RWL_RECTANGLE_HPP
#define REDEMPTION_MOD_INTERNAL_WIDGET2_RWL_RECTANGLE_HPP

#include "widget2_composite.hpp"
#include "draw_api.hpp"

class RwlRectangle : public WidgetComposite
{
public:
    struct Style {
        struct Borders {
            struct Border {
                int type;
                int color;
                uint16_t size;
            };
            Border top;
            Border right;
            Border bottom;
            Border left;
        };
        Borders borders;
        Borders inactive_borders;
        Borders focus_borders;
        Borders inactive_focus_borders;
        int color;
    };

    Style style;

public:
    RwlRectangle(DrawApi* drawable, Widget2* parent, NotifyApi* notifier,
                 const Style& basestyle, int group_id = 0)
    : WidgetComposite(drawable, Rect(), parent, notifier, group_id)
    , style(basestyle)
    {}

    virtual ~RwlRectangle()
    {}

    void set_style(const Style& newstyle)
    {
        this->rect.cx -= this->style.borders.left.size + this->style.borders.right.size;
        this->rect.cy -= this->style.borders.top.size + this->style.borders.bottom.size;
        this->style = newstyle;
        this->rect.cx += this->style.borders.left.size + this->style.borders.right.size;
        this->rect.cy += this->style.borders.top.size + this->style.borders.bottom.size;
    }

    virtual void draw(const Rect& clip)
    {
        this->drawable->draw(
            RDPOpaqueRect(
                clip,
                this->style.color
            ), this->inner_clip()
        );
    }

    Rect inner_clip()
    {
        return Rect(
            this->rect.x + this->style.borders.left.size,
            this->rect.y + this->style.borders.top.size,
            this->rect.cx - (this->style.borders.left.size + this->style.borders.right.size),
            this->rect.cy - (this->style.borders.top.size + this->style.borders.bottom.size)
        );
    }
};

class RwlImage : public RwlRectangle
{
    Bitmap bmp;

public:
    RwlImage(DrawApi* drawable, Widget2* parent, NotifyApi* notifier,
             const char * filename, const Style& basestyle, int group_id = 0)
    : RwlRectangle(drawable, parent, notifier, basestyle, group_id)
    , bmp(filename)
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;

        this->rect.cx = this->bmp.cx;
        this->rect.cy = this->bmp.cy;
        this->rect.cx += this->style.borders.left.size + this->style.borders.right.size;
        this->rect.cy += this->style.borders.top.size + this->style.borders.bottom.size;
    }

    virtual ~RwlImage()
    {}

    virtual void draw(const Rect& clip)
    {
        Rect inner_clip = this->inner_clip();
        Rect newclip = inner_clip.intersect(clip);
        int16_t mx = std::max<int16_t>(newclip.x, 0);
        int16_t my = std::max<int16_t>(newclip.y, 0);
        this->drawable->draw(
            RDPMemBlt(
                0,
                Rect(mx, my, newclip.cx, newclip.cy),
                0xCC,
                mx - this->dx(),
                my - this->dy(),
                0
            ),
            inner_clip,
            this->bmp
        );
    }
};



#endif
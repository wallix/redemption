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
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_WIDGET2_TOOLTIP_HPP)
#define REDEMPTION_MOD_WIDGET2_TOOLTIP_HPP

#include "widget.hpp"
#include "multiline.hpp"

class WidgetTooltip : public Widget2
{
    WidgetMultiLine desc;

public:
    WidgetTooltip(DrawApi & drawable, int16_t x, int16_t y, Widget2 & parent,
                  NotifyApi* notifier, const char * text,
                  int fgcolor = BLACK, int bgcolor = 0x9fffff
                  )
        : Widget2(drawable, Rect(x, y, 100, 100), parent, notifier, 0)
        , desc(WidgetMultiLine(drawable, 0, 0, *this, this, text, true, 0, fgcolor, bgcolor, 4, 1))
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;
        this->rect.cx = this->desc.rect.cx;
        this->rect.cy = this->desc.rect.cy;

    }
    virtual ~WidgetTooltip()
    {
    }

    void set_text(const char * text)
    {
        this->desc.set_text(text);
        this->rect.cx = this->desc.rect.cx;
        this->rect.cy = this->desc.rect.cy;
    }

    virtual void draw(const Rect& clip)
    {
        this->desc.draw(clip);
        this->draw_border(clip);
    }

    int get_tooltip_cx() {
        return this->rect.cx;
    }
    int get_tooltip_cy() {
        return this->rect.cy;
    }

    void set_tooltip_xy(int x, int y) {
        this->rect.x = x;
        this->rect.y = y;
        this->desc.rect.x = x;
        this->desc.rect.y = y;
    }

    void draw_border(const Rect& clip)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx() - 1, 1
        )), BLACK), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), BLACK), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
        )), BLACK), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx() - 1, 1
        )), BLACK), this->rect);
    }
};


// class WidgetTooltip : public Widget2
// {
// public:
//     static const size_t buffer_size = 256;

//     char buffer[buffer_size];
//     int bg_color;
//     int fg_color;
//     bool auto_resize;


// public:
//     WidgetTooltip(DrawApi & drawable, int16_t x, int16_t y, Widget2 & parent,
//                   NotifyApi* notifier, const char * text,
//                   int fgcolor = BLACK, int bgcolor = YELLOW
//                   )
//         : Widget2(drawable, Rect(x, y, 100, 100), parent, notifier, 0)
//         , bg_color(bgcolor)
//         , fg_color(fgcolor)
//     {
//         this->tab_flag = IGNORE_TAB;
//         this->focus_flag = IGNORE_FOCUS;
//         this->set_text(text);
//     }

//     virtual ~WidgetTooltip()
//     {
//     }

//     void set_text(const char * text)
//     {
//         this->buffer[0] = 0;
//         if (text) {
//             const size_t max = std::min(buffer_size - 1, strlen(text));
//             memcpy(this->buffer, text, max);
//             this->buffer[max] = 0;
//             int w = 0;
//             int h = 0;
//             this->drawable.text_metrics(this->buffer, w, h);
//             this->rect.cx = w + 6;
//             this->rect.cy = h + 3;
//         }
//     }

//     const char * get_text() const
//     {
//         return this->buffer;
//     }

//     virtual void draw(const Rect& clip)
//     {
//         this->drawable.draw(RDPOpaqueRect(this->rect, this->bg_color), clip);
//         this->drawable.server_draw_text(this->rect.x + 2,
//                                         this->rect.y + 2,
//                                         this->get_text(),
//                                         this->fg_color,
//                                         this->bg_color,
//                                         this->rect.intersect(clip)
//                                         );
//         this->draw_border(clip);
//     }

//     void draw_border(const Rect& clip)
//     {
//         //top
//         this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
//             this->dx(), this->dy(), this->cx() - 1, 1
//         )), BLACK), this->rect);
//         //left
//         this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
//             this->dx(), this->dy() + 1, 1, this->cy() - 2
//         )), BLACK), this->rect);
//         //right
//         this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
//             this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
//         )), BLACK), this->rect);
//         //bottom
//         this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
//             this->dx(), this->dy() + this->cy() - 1, this->cx() - 1, 1
//         )), BLACK), this->rect);
//     }

// };

#endif

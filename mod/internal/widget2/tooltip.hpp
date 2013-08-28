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


class WidgetTooltip : public Widget2
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];
    int bg_color;
    int fg_color;
    bool auto_resize;


public:
    WidgetTooltip(DrawApi & drawable, int16_t x, int16_t y, Widget2* parent,
                  NotifyApi* notifier, const char * text,
                  int fgcolor = BLACK, int bgcolor = YELLOW
                  )
        : Widget2(drawable, Rect(x, y, 100, 100), parent, notifier, 0)
        , bg_color(bgcolor)
        , fg_color(fgcolor)
    {
        this->set_text(text);
    }

    virtual ~WidgetTooltip()
    {
    }

    void set_text(const char * text)
    {
        this->buffer[0] = 0;
        if (text) {
            const size_t max = std::min(buffer_size - 1, strlen(text));
            memcpy(this->buffer, text, max);
            this->buffer[max] = 0;
            int w = 0;
            int h = 0;
            this->drawable.text_metrics(this->buffer, w, h);
            this->rect.cx = w;
            this->rect.cy = h;
        }
    }

    const char * get_text() const
    {
        return this->buffer;
    }

    virtual void draw(const Rect& clip)
    {
        this->drawable.draw(RDPOpaqueRect(this->rect, this->bg_color), clip);
        this->drawable.server_draw_text(this->rect.x,
                                        this->rect.y,
                                        this->get_text(),
                                        this->fg_color,
                                        this->bg_color,
                                        this->rect.intersect(clip)
                                        );
    }

};

#endif

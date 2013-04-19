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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_MULTILINE_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_MULTILINE_HPP

#include "widget.hpp"

class WidgetMultiLine : public Widget2
{
public:
    static const size_t buffer_size = 1024;
    static const size_t max_line = 50;

    struct line_t {
        char * str;
        int cx;
    };
    char buffer[buffer_size];
    line_t lines[max_line];
    int x_text;
    int y_text;
    int cy_text;
    bool auto_resize;
    int bg_color;
    int fg_color;

public:
    WidgetMultiLine(ModApi* drawable, int16_t x, int16_t y, Widget2* parent,
                    NotifyApi* notifier, const char * text,
                    bool auto_resize = true,
                    int id = 0, int bgcolor = BLACK, int fgcolor = WHITE,
                    int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, id)
    , x_text(xtext)
    , y_text(ytext)
    , cy_text(0)
    , auto_resize(auto_resize)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    {
        this->rect.cx = 0;
        this->rect.cy = 0;
        this->set_text(text);
    }

    virtual ~WidgetMultiLine()
    {
    }

    void set_text(const char * text)
    {
        const char * str = 0;
        char * pbuf = this->buffer;
        line_t * line = this->lines;
        do {
            str = strstr(text, "<br>");
            size_t size = std::min<size_t>(str ? (str-text) : strlen(text), &this->buffer[this->buffer_size-1]-pbuf);
            memcpy(pbuf, text, size);
            line->str = pbuf;
            pbuf += size;
            text += size + 4;
            *pbuf = '\0';
            ++pbuf;
            if (this->drawable) {
                int h;
                this->drawable->text_metrics(line->str, line->cx, h);
                if (h > this->cy_text)
                    this->cy_text = h;
                if (this->auto_resize) {
                    if (line->cx > this->rect.cx)
                        this->rect.cx = line->cx;
                    if (h > this->rect.cy)
                        this->rect.cy = h;
                }
            }
            ++line;
        } while (str && pbuf < &this->buffer[this->buffer_size] && line != &this->lines[this->max_line-1]);

        line->str = 0;

        if (this->auto_resize) {
            this->rect.cx += this->x_text * 2;
            this->rect.cy = (this->rect.cy + this->y_text * 2) * (line - &this->lines[0]);
        }
    }

    const char * get_line(size_t num) const
    {
        if (num >= this->max_line)
            return 0;
        return this->lines[num].str;
    }

    virtual void draw(const Rect& clip)
    {
        int dy = this->dy();
        this->drawable->draw(RDPOpaqueRect(clip, this->bg_color), this->rect);
        for (line_t * line = this->lines; line->str; ++line) {
            dy += this->y_text;
            this->drawable->server_draw_text(this->x_text + this->dx(),
                                             dy,
                                             line->str,
                                             this->fg_color,
                                             clip.intersect(Rect(this->dx(),
                                                                 dy,
                                                                 this->cx(),
                                                                 this->cy_text))
            );
            dy += this->y_text + this->cy_text;
        }
    }
};

#endif
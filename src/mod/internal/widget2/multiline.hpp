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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once
#include "widget.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

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
    Font const & font;

public:
    WidgetMultiLine(gdi::GraphicApi & drawable, int16_t x, int16_t y, Widget2& parent,
                    NotifyApi* notifier, const char * text,
                    bool auto_resize, int group_id,
                    int fgcolor, int bgcolor, Font const & font,
                    int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x, y, 1, 1), parent, notifier, group_id)
    , x_text(xtext)
    , y_text(ytext)
    , cy_text(0)
    , auto_resize(auto_resize)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , font(font)
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;

        this->set_cx(0);
        this->set_cy(0);
        this->set_text(text);
    }

    void set_text(const char * text)
    {
        if (this->auto_resize) {
            this->set_cx(0);
            this->set_cy(0);
        }

        const char * str = nullptr;
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
            gdi::TextMetrics tm(this->font, line->str);
            line->cx = tm.width;
            if (tm.height > this->cy_text){
                this->cy_text = tm.height;
            }
            if (this->auto_resize) {
                if (line->cx > this->cx()){
                    this->set_cx(line->cx);
                }
                if (tm.height > this->cy()){
                    this->set_cy(tm.height);
                }
            }
            ++line;
        } while (str && pbuf < &this->buffer[this->buffer_size] && line != &this->lines[this->max_line-1]);

        line->str = nullptr;

        if (this->auto_resize) {
            this->set_cx(this->cx() + this->x_text * 2);
            this->set_cy((this->cy() + this->y_text * 2) * (line - &this->lines[0]));
        }
    }

    const char * get_line(size_t num) const
    {
        if (num >= this->max_line)
            return nullptr;
        return this->lines[num].str;
    }

    void draw(const Rect& clip) override {
        int dy = this->y() + this->y_text;
        this->drawable.draw(RDPOpaqueRect(clip, this->bg_color), this->get_rect());
        for (line_t * line = this->lines; line->str; ++line) {
            dy += this->y_text;
            gdi::server_draw_text(this->drawable
                                 , this->font
                                 , this->x_text + this->x()
                                 , dy
                                 , line->str
                                 , this->fg_color
                                 , this->bg_color
                                 , clip.intersect(Rect(this->x()
                                 , dy
                                 , this->cx()
                                 , this->cy_text))
            );
            dy += this->y_text + this->cy_text;
        }
    }
};


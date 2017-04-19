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
#include "utils/colors.hpp"

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
    BGRColor_ bg_color;
    BGRColor_ fg_color;
    Font const & font;

public:
    WidgetMultiLine(gdi::GraphicApi & drawable, Widget2& parent,
                    NotifyApi* notifier, const char * text,
                    int group_id,
                    BGRColor_ fgcolor, BGRColor_ bgcolor, Font const & font,
                    int xtext = 0, int ytext = 0)
    : Widget2(drawable, parent, notifier, group_id)
    , x_text(xtext)
    , y_text(ytext)
    , cy_text(0)
    , auto_resize(false)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , font(font)
    {
        this->tab_flag   = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;

        this->set_text(text);
    }

    void set_text(const char * text)
    {
        if (this->auto_resize) {
            this->set_wh(0, 0);
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
                uint16_t w = this->cx();
                if (line->cx > w){
                    w = line->cx;
                }
                uint16_t h = this->cy();
                if (tm.height > h){
                    h = tm.height;
                }
                this->set_wh(w, h);
            }
            ++line;
        } while (str && pbuf < &this->buffer[this->buffer_size] && line != &this->lines[this->max_line-1]);

        line->str = nullptr;

        if (this->auto_resize) {
            uint16_t w = this->cx();
            uint16_t h = this->cy();
            this->set_wh(w + this->x_text * 2,
                         (h + this->y_text * 2) * (line - &this->lines[0]));
        }
    }

    const char * get_line(size_t num) const
    {
        if (num >= this->max_line)
            return nullptr;
        return this->lines[num].str;
    }

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            int dy = this->y() + this->y_text;
            this->drawable.draw(RDPOpaqueRect(rect_intersect, encode_color24()(this->bg_color)), this->get_rect(), gdi::ColorCtx::depth24());
            for (line_t * line = this->lines; line->str; ++line) {
                dy += this->y_text;
                gdi::server_draw_text(this->drawable
                                     , this->font
                                     , this->x_text + this->x()
                                     , dy
                                     , line->str
                                     , encode_color24()(this->fg_color)
                                     , encode_color24()(this->bg_color)
                                     , gdi::ColorCtx::depth24()
                                     , rect_intersect.intersect(
                                                Rect(this->x(),
                                                     dy,
                                                     this->cx(),
                                                     this->cy_text
                                            ))
                    );
                dy += this->y_text + this->cy_text;
            }

            this->drawable.end_update();
        }
    }

    Dimension get_optimal_dim() override {
        uint16_t max_line_width  = 0;
        uint16_t max_line_height = 0;
        line_t * line = this->lines;
        for (; line->str; ++line) {
            gdi::TextMetrics tm(this->font, line->str);
            if (max_line_width < tm.width){
                max_line_width = tm.width;
            }
            if (max_line_height < tm.height){
                max_line_height = tm.height;
            }
        }
        return Dimension(max_line_width + this->x_text * 2,
            (max_line_height + this->y_text * 2) * (line - &this->lines[0]));
    }
};

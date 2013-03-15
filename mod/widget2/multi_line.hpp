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

#if !defined(REDEMPTION_MOD_WIDGET2_MULTI_LINE_HPP)
#define REDEMPTION_MOD_WIDGET2_MULTI_LINE_HPP

#include <vector>
#include "widget.hpp"
#include "label.hpp"

///TODO : public Widget
class WidgetMultiLine : public Widget
{
public:
    std::vector<WidgetLabel*> child_list;
    int bg_color2;

    WidgetMultiLine(ModApi* drawable, const Rect& rect, Widget* parent, NotifyApi* notifier, const char * message, int id = 0, int bgcolor1 = 0, int bgcolor2 = 0)
    : Widget(drawable, rect, parent, Widget::TYPE_LABEL | Widget::TYPE_MULTIPLE, notifier, id)
    , child_list()
    , bg_color2(bgcolor2)
    {
        this->bg_color = bgcolor1;
        this->tab_flag = NORMAL_TAB;
        int id_child = 0;
        size_t n = 0;
        int addy = 25;
        int y = 0;
        for (;;) {
            if (message[n] == 0 || (message[n] == '<' && message[n+1] && message[n+1] == 'b' && message[n+2] && message[n+2] == 'r' && message[n+3] && message[n+3] == '>')) {
                if (n) {
                    WidgetLabel * w = new WidgetLabel(drawable, Rect(0, y, rect.cx, 25), this, 0, 0, id_child++, 2);
                    w->bg_color = id_child & 1 ? bgcolor1 : bgcolor2;
                    size_t max = std::min(n, WidgetLabel::buffer_size-1);
                    memcpy(w->buffer, message, max);
                    w->buffer[max] = 0;
                    if (drawable) {
                        w->context_text = drawable->create_context_text(w->buffer);
                        addy = w->context_text->cy + 1;
                        w->rect.cy = w->context_text->cy;
                    }
                    this->child_list.push_back(w);
                }
                if (message[n] == 0) {
                    break;
                }
                message += n + 4;
                n = 0;
                y += addy;
            } else {
                ++n;
            }
        }
    }

    virtual ~WidgetMultiLine()
    {}

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        int dx = x < 0 ? -x : 0;
        int dy = y < 0 ? -y : 0;
        for (size_t i = 0; i < this->child_list.size(); ++i) {
            WidgetLabel & w = *this->child_list[i];
            Rect clipsep = rect.intersect(w.rect.offset(-dx, -dy));
            if (!clipsep.isempty()) {
                clipsep.x -= w.rect.x;
                clipsep.y -= w.rect.y;
                w.draw(clipsep, x+dx, y + w.rect.y + dy, xclip, yclip + w.rect.y);
            }
        }
    }
};

#endif
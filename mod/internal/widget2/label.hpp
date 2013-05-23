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

#if !defined(REDEMPTION_MOD_WIDGET2_LABEL_HPP)
#define REDEMPTION_MOD_WIDGET2_LABEL_HPP

#include "widget.hpp"

class WidgetLabel : public Widget2
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];
    int x_text;
    int y_text;
    int bg_color;
    int fg_color;
    bool auto_resize;

public:
    WidgetLabel(ModApi* drawable, int16_t x, int16_t y, Widget2* parent,
                NotifyApi* notifier, const char * text, bool auto_resize = true,
                int group_id = 0, int fgcolor = BLACK, int bgcolor = WHITE,
                int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
    , x_text(xtext)
    , y_text(ytext)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , auto_resize(auto_resize)
    {
        this->rect.cx = 0;
        this->rect.cy = 0;
        this->set_text(text);
    }

    virtual ~WidgetLabel()
    {
    }

    void set_text(const char * text)
    {
        this->buffer[0] = 0;
        if (text) {
            const size_t max = std::min(buffer_size - 1, strlen(text));
            memcpy(this->buffer, text, max);
            this->buffer[max] = 0;
            if (this->auto_resize && this->drawable) {
                int w,h;
                this->drawable->text_metrics(this->buffer, w,h);
                this->rect.cx = this->x_text * 2 + w;
                this->rect.cy = this->y_text * 2 + h;
                if (max == 1) {
                    this->rect.cx -= 2;
                }
            }
        }
    }

    const char * get_text() const
    {
        return this->buffer;
    }

    virtual void draw(const Rect& clip)
    {
        this->drawable->draw(RDPOpaqueRect(clip, this->bg_color), this->rect);
        this->drawable->server_draw_text(this->x_text + this->dx(),
                                         this->y_text + this->dy(),
                                         this->get_text(),
                                         this->fg_color,
                                         this->bg_color,
                                         this->rect.intersect(clip)
                                        );
    }
};

#endif

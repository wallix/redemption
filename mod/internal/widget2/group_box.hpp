/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#ifndef REDEMPTION_MOD_WIDGET2_GROUP_BOX_HPP
#define REDEMPTION_MOD_WIDGET2_GROUP_BOX_HPP

#include "composite.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"

class WidgetGroupBox : public WidgetParent
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];

    int bg_color;
    int fg_color;

    CompositeArray composite_array;

public:
    WidgetGroupBox( DrawApi & drawable, int16_t x, int16_t y
                  , uint16_t cx, uint16_t cy, Widget2 & parent
                  , NotifyApi * notifier, const char * text
                  , int group_id, int fgcolor, int bgcolor)
    : WidgetParent(drawable, Rect(x, y, cx, cy), parent, notifier)
    , bg_color(bgcolor)
    , fg_color(fgcolor) {
        this->impl = &composite_array;

        this->set_text(text);
    }

    virtual ~WidgetGroupBox() {
        this->clear();
    }

    virtual void draw(const Rect & clip) {
        Rect rect_intersect = clip.intersect(this->rect);
        WidgetParent::draw_inner_free(rect_intersect, this->bg_color);

        // Box.
        const uint16_t border           = 6;
        const uint16_t text_margin      = 6;
        const uint16_t text_indentation = border + text_margin + 4;
        const uint16_t x_offset         = 1;

        int w, h, tmp;
        this->drawable.text_metrics("bp", tmp, h);
        this->drawable.text_metrics(this->buffer, w, tmp);

        BStream deltaPoints(256);

        deltaPoints.out_sint16_le(border - (text_indentation - text_margin + 1));
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(this->rect.cy - h / 2 - border);

        deltaPoints.out_sint16_le(this->rect.cx - border * 2 + x_offset);
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(-(this->rect.cy - h / 2 - border));    // OK

        deltaPoints.out_sint16_le(-(this->rect.cx - border * 2 - w - text_indentation + x_offset));
        deltaPoints.out_sint16_le(0);

        deltaPoints.mark_end();
        deltaPoints.rewind();

        RDPPolyline polyline_box( this->rect.x + text_indentation - text_margin
                                , this->rect.y + h / 2
                                , 0x0D, 0, this->fg_color, 5, deltaPoints);
        this->drawable.draw(polyline_box, clip);


        // Label.
        this->drawable.server_draw_text( this->rect.x + text_indentation
                                       , this->rect.y
                                       , this->buffer
                                       , this->fg_color
                                       , this->bg_color
                                       , rect_intersect
                                       );

        WidgetParent::draw_children(rect_intersect);
    }

    virtual int get_bg_color() const {
        return this->bg_color;
    }
    virtual void move_xy(int16_t x, int16_t y) {
        this->rect.x += x;
        this->rect.y += y;
        this->WidgetParent::move_xy(x,y);
    }
    virtual void set_xy(int16_t x, int16_t y) {
        this->move_xy(x - this->rect.x, y - this->rect.y);
    }
    const char * get_text() const {
        return this->buffer;
    }
    void set_text(const char * text) {
        this->buffer[0] = 0;
        if (text) {
            const size_t max = std::min(buffer_size - 1, strlen(text));
            memcpy(this->buffer, text, max);
            this->buffer[max] = 0;
        }
    }
};

#endif  // #ifndef REDEMPTION_MOD_WIDGET2_GROUP_BOX_HPP

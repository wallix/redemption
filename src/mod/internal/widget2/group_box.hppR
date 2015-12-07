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
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"

class WidgetGroupBox : public WidgetParent
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];

    int bg_color;
    int fg_color;

    CompositeArray composite_array;

    Font const & font;

public:
    WidgetGroupBox( DrawApi & drawable, int16_t x, int16_t y
                  , uint16_t cx, uint16_t cy, Widget2 & parent
                  , NotifyApi * notifier, const char * text
                  , int group_id, int fgcolor, int bgcolor, Font const & font)
    : WidgetParent(drawable, Rect(x, y, cx, cy), parent, notifier)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , font(font) {
        this->impl = &composite_array;

        this->set_text(text);
    }

    ~WidgetGroupBox() override {
        this->clear();
    }

    void draw(const Rect & clip) override {
        Rect rect_intersect = clip.intersect(this->rect);
        WidgetParent::draw_inner_free(rect_intersect, this->bg_color);

        // Box.
        const uint16_t border           = 6;
        const uint16_t text_margin      = 6;
        const uint16_t text_indentation = border + text_margin + 4;

        int w, h, tmp;
        this->drawable.text_metrics(this->font, "bp", tmp, h);
        this->drawable.text_metrics(this->font, this->buffer, w, tmp);

        auto gcy = this->rect.cy - h / 2 - border;
        auto gcx = this->rect.cx - border * 2 + 1;
        auto px = this->rect.x + border - 1;
        auto wlabel = text_margin * 2 + w;
        auto x = px;
        auto y = this->rect.y + h / 2;
        auto cx = text_indentation - text_margin - border + 2;
        auto cy = 1;
        this->drawable.draw(RDPOpaqueRect(Rect(x, y, cx, cy), this->fg_color), clip);
        cx = gcx;
        this->drawable.draw(RDPOpaqueRect(Rect(x, y+gcy, cx+1, cy), this->fg_color), clip);
        x += wlabel + 4;
        cx -= wlabel + 4;
        this->drawable.draw(RDPOpaqueRect(Rect(x, y, cx, cy), this->fg_color), clip);
        x = px;
        cy = gcy;
        cx = 1;
        this->drawable.draw(RDPOpaqueRect(Rect(x, y, cx, cy), this->fg_color), clip);
        x += gcx;
        this->drawable.draw(RDPOpaqueRect(Rect(x, y, cx, cy), this->fg_color), clip);

        // Label.
        this->drawable.server_draw_text( this->font
                                       , this->rect.x + text_indentation
                                       , this->rect.y
                                       , this->buffer
                                       , this->fg_color
                                       , this->bg_color
                                       , rect_intersect
                                       );

        WidgetParent::draw_children(rect_intersect);
    }

    int get_bg_color() const override {
        return this->bg_color;
    }
    void move_xy(int16_t x, int16_t y) {
        this->rect.x += x;
        this->rect.y += y;
        this->WidgetParent::move_xy(x,y);
    }
    void set_xy(int16_t x, int16_t y) override {
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

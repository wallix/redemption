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

#pragma once

#include "widget.hpp"
#include "multiline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"

class WidgetTooltip : public Widget2
{
    uint w_border;
    uint h_border;
    WidgetMultiLine desc;
    BGRColor border_color;

public:
    WidgetTooltip(gdi::GraphicApi & drawable, Widget2 & parent,
                  NotifyApi* notifier, const char * text,
                  BGRColor fgcolor, BGRColor bgcolor, BGRColor border_color, Font const & font)
        : Widget2(drawable, parent, notifier, 0)
        , w_border(10)
        , h_border(10)
        , desc(drawable, *this, this, text, 0, fgcolor, bgcolor, font, 0, 0)
        , border_color(border_color)
    {
        this->tab_flag   = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;
    }

    ~WidgetTooltip() override {
    }

    Dimension get_optimal_dim() override {
        Dimension dim = this->desc.get_optimal_dim();

        dim.w += 2 * this->w_border;
        dim.h += 2 * this->h_border;

        return dim;
    }

    void set_text(const char * text)
    {
        this->desc.set_text(text);
        Dimension dim = this->desc.get_optimal_dim();
        this->desc.set_wh(dim);

        this->set_wh(this->desc.cx() + 2 * w_border,
                     this->desc.cy() + 2 * h_border);
    }

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->drawable.draw(RDPOpaqueRect(this->get_rect(), encode_color24()(this->desc.bg_color)), rect_intersect, gdi::ColorCtx::depth24());
            this->desc.rdp_input_invalidate(rect_intersect);
            this->draw_border(rect_intersect);

            this->drawable.end_update();
        }
    }

    void set_xy(int16_t x, int16_t y) override {
        Widget2::set_xy(x, y);
        this->desc.set_xy(x + w_border, y + h_border);
    }

    void set_wh(uint16_t w, uint16_t h) override {
        Widget2::set_wh(w, h);
        this->desc.set_wh(w -  2 * w_border, h - 2 * h_border);
    }

    using Widget2::set_wh;

    void draw_border(const Rect clip)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y(), this->cx() - 1, 1
        )), encode_color24()(this->border_color)), clip, gdi::ColorCtx::depth24());
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + 1, 1, this->cy() - 2
        )), encode_color24()(this->border_color)), clip, gdi::ColorCtx::depth24());
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x() + this->cx() - 1, this->y(), 1, this->cy()
        )), encode_color24()(this->border_color)), clip, gdi::ColorCtx::depth24());
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->x(), this->y() + this->cy() - 1, this->cx() - 1, 1
        )), encode_color24()(this->border_color)), clip, gdi::ColorCtx::depth24());
    }
};

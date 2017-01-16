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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#pragma once

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "mod/internal/widget2/widget.hpp"
#include "utils/sugar/cast.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/log.hpp"

class WidgetLabel : public Widget2
{
public:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];
    int initial_x_text;
    int x_text;
    int y_text;
    uint32_t bg_color;
    uint32_t fg_color;
    bool tool;

    int w_border;
    int h_border;

    Font const & font;

public:
    WidgetLabel(gdi::GraphicApi & drawable, Widget2& parent,
                NotifyApi* notifier, const char * text,
                int group_id, uint32_t fgcolor, uint32_t bgcolor, Font const & font,
                int xtext = 0, int ytext = 0)
    : Widget2(drawable, parent, notifier, group_id)
    , initial_x_text(xtext)
    , x_text(xtext)
    , y_text(ytext)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , tool(false)
    , w_border(x_text)
    , h_border(y_text)
    , font(font)
    {
        this->tab_flag = IGNORE_TAB;
        this->focus_flag = IGNORE_FOCUS;
        this->set_text(text);
    }

    ~WidgetLabel() override {
    }

    void set_text(const char * text)
    {
        this->buffer[0] = 0;
        if (text) {
            const size_t remain_n = buffer_size - 1;
            const size_t n = strlen(text);
            const size_t max = ((remain_n >= n) ? n :
                                ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));
            memcpy(this->buffer, text, max);
            this->buffer[max] = 0;
        }
    }

    const char * get_text() const
    {
        return this->buffer;
    }

    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->draw(rect_intersect, this->get_rect(), this->drawable, this->buffer,
                this->fg_color, this->bg_color, this->font, this->x_text, this->y_text);

            this->drawable.end_update();
        }
    }

    static void draw(Rect const clip, Rect const rect, gdi::GraphicApi& drawable,
                     char const* text, uint32_t fgcolor, uint32_t bgcolor,
                     Font const & font, int xtext, int ytext) {
        drawable.draw(RDPOpaqueRect(rect, bgcolor), clip, gdi::ColorCtx::depth24());
        gdi::server_draw_text(drawable,
                              font,
                              xtext + rect.x,
                              ytext + rect.y,
                              text,
                              fgcolor,
                              bgcolor,
                              gdi::ColorCtx::depth24(),
                              rect.intersect(clip)
                              );
    }

    Dimension get_optimal_dim() override {
        gdi::TextMetrics tm(this->font, (this->buffer[0] ? this->buffer : "Édp"));
        return Dimension(tm.width + this->x_text * 2, tm.height + this->y_text * 2);
    }

    static Dimension get_optimal_dim(Font const & font, char const* text, int xtext, int ytext) {
        char buffer[buffer_size];

        buffer[0] = 0;
        if (text) {
            const size_t remain_n = buffer_size - 1;
            const size_t n = strlen(text);
            const size_t max = ((remain_n >= n) ? n :
                                ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));
            memcpy(buffer, text, max);
            buffer[max] = 0;
        }

        gdi::TextMetrics tm(font, (buffer[0] ? buffer : "Édp"));
        return Dimension(tm.width + xtext * 2, tm.height + ytext * 2);
    }

    bool shift_text(int pos_x) {
        bool res = true;
        if (pos_x + this->x_text > this->cx() - 4) {
            this->x_text = this->cx() - pos_x - 4;
        }
        else if (pos_x + this->x_text < this->w_border) {
            this->x_text = this->w_border - pos_x;
        }
        else {
            res = false;
        }
        return res;
    }

    void set_color(uint32_t bg_color, uint32_t fg_color) override {
        this->bg_color = bg_color;
        this->fg_color = fg_color;
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2*) override {
        if (this->tool) {
            if (device_flags == MOUSE_FLAG_MOVE) {
                // TODO: tm.height unused ?
                gdi::TextMetrics tm(this->font, this->buffer);
                if (tm.width > this->cx()) {
                    this->show_tooltip(this, this->buffer, x, y, Rect(0, 0, 0, 0));
                }
            }
        }
    }

    void auto_resize() {
        Dimension dim = this->get_optimal_dim();
        this->set_wh(dim);
    }
};

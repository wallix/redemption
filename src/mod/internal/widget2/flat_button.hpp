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

#include "widget.hpp"
#include "label.hpp"
#include "keyboard/keymap2.hpp"
#include "gdi/graphic_api.hpp"

class WidgetFlatButton : public Widget2
{
private:
    static const size_t buffer_size = 256;

    char buffer[buffer_size];

    bool auto_resize_;

    int x_text;
    int y_text;

public:
    int state;
    notify_event_t event;
    int fg_color;
    int bg_color;
    int focus_color;
    bool logo;

    Font const & font;

    Rect label_rect;

    WidgetFlatButton(gdi::GraphicApi & drawable, int16_t x, int16_t y, Widget2& parent,
                     NotifyApi* notifier, const char * text, bool auto_resize,
                     int group_id, int fgcolor, int bgcolor,
                     int focuscolor, Font const & font, int xtext = 0, int ytext = 0,
                     bool logo = false /*, notify_event_t notify_event = NOTIFY_SUBMIT*/)
    : Widget2(drawable, Rect(x,y,1,1), parent, notifier, group_id)
    , auto_resize_(auto_resize)
    , x_text(xtext)
    , y_text(ytext)
    , state(0)
    , event(NOTIFY_SUBMIT)
    , fg_color(fgcolor)
    , bg_color(bgcolor)
    , focus_color(focuscolor)
    , logo(logo)
    , font(font)
    {
        this->label_rect.x  = this->x() + 1;
        this->label_rect.y  = this->y() + 1;
        this->label_rect.cx = 1;
        this->label_rect.cy = 1;

        this->set_text(text);

        this->set_cx(this->label_rect.cx + 3);
        this->set_cy(this->label_rect.cy + 3);
    }

    ~WidgetFlatButton() override {}

    void set_x(int16_t x) override {
        Widget2::set_x(x);
        this->label_rect.x = x + 1;
    }

    void set_y(int16_t y) override {
        Widget2::set_y(y);
        this->label_rect.y = y + 1;
    }

    void set_cx(uint16_t cx) override {
        Widget2::set_cx(cx);
        this->label_rect.cx = cx - 3;
    }

    void set_cy(uint16_t cy) override {
        Widget2::set_cy(cy);
        this->label_rect.cy = cy - 3;
    }

    void set_text(char const* text) {
        this->buffer[0] = 0;
        if (text) {
            const size_t remain_n = buffer_size - 1;
            const size_t n = strlen(text);
            const size_t max = ((remain_n >= n) ? n :
                                ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));
            memcpy(this->buffer, text, max);
            this->buffer[max] = 0;
            if (this->auto_resize_) {
                Dimension dm = WidgetLabel::get_optimal_dim(this->buffer, this->font, this->x_text, this->y_text);

                this->label_rect.cx = dm.w;
                this->label_rect.cy = dm.h;

                this->set_cx(this->label_rect.cx + 3);
                this->set_cy(this->label_rect.cy + 3);
            }
        }
    }

    void draw(const Rect& clip) override
    {
        this->draw(clip, this->get_rect(), this->drawable, this->logo, this->has_focus,
            this->buffer, this->fg_color, this->bg_color, this->focus_color,
            this->label_rect, this->state, this->font, this->x_text, this->y_text);
    }

    static void draw(Rect const& clip, Rect const& rect, gdi::GraphicApi& drawable,
                     bool logo, bool has_focus, char const* text,
                     uint32_t fgcolor, uint32_t bgcolor, uint32_t focuscolor,
                     Rect const& optional_label_rect, int state, Font const& font, int xtext, int ytext) {
        uint32_t fg_color = fgcolor;
        uint32_t bg_color = bgcolor;

        Rect label_rect = optional_label_rect;
        if (label_rect.isempty()) {
            label_rect = rect;
            label_rect.x++;
            label_rect.y++;
            label_rect.cx -= 3;
            label_rect.cy -= 3;
        }

        // Label color
        if (logo) {
            if (has_focus) {
                fg_color = focuscolor;
            }
            else {
                fg_color = fgcolor;
            }
        }
        else {
            if (has_focus) {
                bg_color = focuscolor;
            }
            else {
                bg_color = bgcolor;
            }
        }
        // background
        drawable.draw(RDPOpaqueRect(clip.intersect(rect), bg_color), rect);

        if (state & 1)  {
            Rect temp_rect = label_rect.offset(1, 1);
            temp_rect.cx--;
            temp_rect.cy--;
            WidgetLabel::draw(clip, temp_rect, drawable, text, fg_color, bg_color, font, xtext, ytext);
        }
        else {
            WidgetLabel::draw(clip, label_rect, drawable, text, fg_color, bg_color, font, xtext, ytext);
        }

        if (logo)
            return;

        // border
        //top
        drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              rect.x, rect.y, rect.cx - 2, 2
              )), fg_color), rect);
        //left
        drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              rect.x, rect.y + 2, 2, rect.cy - 2
              )), fg_color), rect);
        //right
        drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              rect.x + rect.cx - 2, rect.y, 2, rect.cy
              )), fg_color), rect);
        //bottom
        drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
              rect.x, rect.y + rect.cy - 2, rect.cx, 2
              )), fg_color), rect);
    }

    void swap_border_color()
    {
        this->drawable.begin_update();
        this->draw(this->get_rect());
        this->drawable.end_update();
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN) && (this->state & 1) == 0) {
            this->state |= 1;
            this->swap_border_color();
        }
        else if (device_flags == MOUSE_FLAG_BUTTON1 && this->state & 1) {
            this->state &= ~1;
            this->swap_border_color();
            if (this->get_rect().contains_pt(x, y)) {
                this->send_notify(this->event);
            }
        }
        else
            this->Widget2::rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(this->event);
                    break;
                case Keymap2::KEVENT_KEY:
                    if (keymap->get_char() == ' ') {
                        this->send_notify(this->event);
                    }
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }

    Dimension get_optimal_dim() override {
        gdi::TextMetrics tm(this->font, this->buffer);
        return Dimension(tm.width + 2 * this->x_text + 4, tm.height + 2 * this->y_text + 4);
    }

    static Dimension get_optimal_dim(Font const& font, char const* text, int xtext = 0, int ytext = 0) {
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

        gdi::TextMetrics tm(font, buffer);
        return Dimension(tm.width + 2 * xtext + 4, tm.height + 2 * ytext + 4);
    }
};

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

#include "label.hpp"
#include "widget.hpp"
#include "edit.hpp"
#include "keyboard/keymap2.hpp"
#include "gdi/graphic_api.hpp"
#include "cxx/cxx.hpp"

class WidgetPassword : public WidgetEdit {
public:
    WidgetLabel masked_text;

    int w_char;
    int h_char;

    WidgetPassword(gdi::GraphicApi & drawable,
                   Widget2& parent, NotifyApi* notifier, const char * text,
                   int group_id, BGRColor_ fgcolor, BGRColor_ bgcolor, BGRColor_ focus_color, Font const & font,
                   std::size_t edit_position = -1, int xtext = 0, int ytext = 0)
        : WidgetEdit(drawable, parent, notifier, text,
                     group_id, fgcolor, bgcolor, focus_color, font, edit_position, xtext, ytext)
        , masked_text(drawable, *this, nullptr, text, 0, fgcolor, bgcolor, font,
                      xtext, ytext)
    {
        this->set_masked_text();

        gdi::TextMetrics tm(font, "*");
        this->w_char = tm.width;
        this->h_char = tm.height;
        this->h_char -= 1;
    }

    Dimension get_optimal_dim() override {
        Dimension dim = this->masked_text.get_optimal_dim();

        dim.w += 2;
        dim.h += 2;

        return dim;
    }

    void set_masked_text() {
        char buff[WidgetLabel::buffer_size];
        for (size_t n = 0; n < this->num_chars; ++n) {
            buff[n] = '*';
        }
        buff[this->num_chars] = 0;
        this->masked_text.set_text(buff);
    }

    void set_xy(int16_t x, int16_t y) override {
        WidgetEdit::set_xy(x, y);
        this->masked_text.set_xy(x + 1, y + 1);
    }

    void set_wh(uint16_t w, uint16_t h) override {
        WidgetEdit::set_wh(w, h);
        this->masked_text.set_wh(w - 2, h - 2);
    }

    using WidgetEdit::set_wh;

    void set_text(const char * text) override {
        WidgetEdit::set_text(text);
        this->set_masked_text();
    }

    void insert_text(const char* text) override {
        WidgetEdit::insert_text(text);
        this->set_masked_text();
        this->rdp_input_invalidate(this->get_rect());
    }

    //const char * show_text() {
    //    return this->masked_text.buffer;
    //}

//    void draw(const Rect clip) override {
    void rdp_input_invalidate(Rect clip) override {
        Rect rect_intersect = clip.intersect(this->get_rect());

        if (!rect_intersect.isempty()) {
            this->drawable.begin_update();

            this->masked_text.rdp_input_invalidate(rect_intersect);
            if (this->has_focus) {
                this->draw_cursor(this->get_cursor_rect());
                if (this->draw_border_focus) {
                    this->draw_border(rect_intersect, this->focus_color);
                }
                else {
                    this->draw_border(rect_intersect, this->label.bg_color);
                }
            }
            else {
                this->draw_border(rect_intersect, this->label.bg_color);
            }

            this->drawable.end_update();
        }
    }
    void update_draw_cursor(Rect old_cursor) override {
        this->drawable.begin_update();
        this->masked_text.rdp_input_invalidate(old_cursor);
        this->draw_cursor(this->get_cursor_rect());
        this->drawable.end_update();
    }


    Rect get_cursor_rect() const override {
        return Rect(this->masked_text.x_text + this->edit_pos * this->w_char + this->x() + 2,
                    this->masked_text.y_text + this->masked_text.y(),
                    1,
                    this->h_char);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            Rect old_cursor_rect = this->get_cursor_rect();
            size_t e = this->edit_pos;
            if (x <= this->x() + this->masked_text.x_text + this->w_char/2) {
                this->edit_pos = 0;
                this->edit_buffer_pos = 0;
            }
            else if (x >= int(this->x() + this->masked_text.x_text + this->w_char * this->num_chars)) {
                if (this->edit_pos < this->num_chars) {
                    this->edit_pos = this->num_chars;
                    this->edit_buffer_pos = this->buffer_size;
                }
            }
            else {

                 //      dx
                 // <---------->
                 //           x
                 // <------------------->
                 //     -x_text
                 //     <------>             screen
                 // +-------------------------------------------------------------
                 // |                        editbox
                 // |           +--------------------------------+
                 // |   {.......|.......X................}       |
                 // |           +--------------------------------+
                 // |   <--------------->
                 // |   (x - dx - x_text)
                 // |

                this->edit_pos = std::min<size_t>((x - this->x() - this->masked_text.x_text - this->w_char/2) / this->w_char, this->num_chars-1);
                this->edit_buffer_pos = UTF8GetPos(reinterpret_cast<uint8_t*>(&this->label.buffer[0]), this->edit_pos);
            }
            if (e != this->edit_pos) {
                this->update_draw_cursor(old_cursor_rect);
            }
        } else {
            Widget2::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3,
                                    long int param4, Keymap2* keymap) override {
        if (keymap->nb_kevent_available() > 0){
            uint32_t kevent = keymap->top_kevent();
            WidgetEdit::rdp_input_scancode(param1, param2, param3, param4, keymap);
            switch (kevent) {
            case Keymap2::KEVENT_BACKSPACE:
            case Keymap2::KEVENT_DELETE:
            case Keymap2::KEVENT_KEY:
                this->set_masked_text();
                REDEMPTION_CXX_FALLTHROUGH;
            case Keymap2::KEVENT_LEFT_ARROW:
            case Keymap2::KEVENT_UP_ARROW:
            case Keymap2::KEVENT_RIGHT_ARROW:
            case Keymap2::KEVENT_DOWN_ARROW:
            case Keymap2::KEVENT_END:
            case Keymap2::KEVENT_HOME:
                this->masked_text.shift_text(this->edit_pos * this->w_char);

                this->rdp_input_invalidate(this->get_rect());
                break;
            default:
                break;
            }
        }
    }
};

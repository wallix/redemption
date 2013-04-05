/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1 of the License, or
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
 *   Copyright (C) Wallix 1010-2013
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_HPP

#include "label.hpp"
#include <keymap2.hpp>

class WidgetEdit : public Widget {
public:
    WidgetLabel label;
    size_t buffer_size;
    size_t num_chars;
    size_t edit_buffer_pos;
    size_t edit_pos;
    size_t cursor_px_pos;
    int w_text;
    int h_text;

    WidgetEdit(ModApi* drawable, int16_t x, int16_t y, uint16_t cx,
               Widget* parent, NotifyApi* notifier, const char * text,
               int id = 0, int bgcolor = BLACK, int fgcolor = WHITE,
               std::size_t edit_position = -1, int xtext = 0, int ytext = 0)
    : Widget(drawable, Rect(x,y,cx,1), parent, notifier, id)
    , label(drawable, 0, 0, this, 0, text, false, 0, bgcolor, fgcolor, xtext+1, ytext)
    , w_text(0)
    , h_text(0)
    {
        if (text) {
            this->buffer_size = strlen(text);
            this->num_chars = UTF8Len(this->label.buffer);
            this->edit_pos = std::min(this->num_chars, edit_position);
            this->edit_buffer_pos = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos);
            this->cursor_px_pos = 0;
            if (this->drawable) {
                char c = this->label.buffer[this->edit_buffer_pos];
                this->label.buffer[this->edit_buffer_pos] = 0;
                this->drawable->text_metrics(this->label.buffer, this->w_text, this->h_text);
                this->cursor_px_pos = this->w_text;
                if (this->edit_pos) {
                    this->cursor_px_pos += 2;
                }
                this->label.buffer[this->edit_buffer_pos] = c;
                int w, h;
                this->drawable->text_metrics(&this->label.buffer[this->edit_buffer_pos], w, h);
                this->w_text += w;
            }
        } else {
            this->buffer_size = 0;
            this->num_chars = 0;
            this->edit_buffer_pos = 0;
            this->edit_pos = 0;
            this->cursor_px_pos = 0;
        }
        if (this->drawable) {
            int w;
            this->drawable->text_metrics("Lp", w, this->h_text);
        }
        this->rect.cy = this->h_text + this->label.y_text * 2;
        this->label.rect.cx = this->rect.cx;
        this->label.rect.cy = this->rect.cy;
        ++this->label.rect.x;
        ++this->label.rect.y;
        this->rect.cx += 2;
        this->rect.cy += 2;
    }

    virtual ~WidgetEdit()
    {}

    void set_edit_x(int x)
    {
        this->rect.x = x;
        this->label.rect.x = x + 1;
    }

    void set_edit_y(int y)
    {
        this->rect.y = y;
        this->label.rect.y = y + 1;
    }

    void set_edit_cx(int w)
    {
        this->rect.cx = w;
        this->label.rect.cx = w - 2;
    }

    void set_edit_cy(int h)
    {
        this->rect.cy = h;
        this->label.rect.cy = h - 2;
    }

    virtual void draw(const Rect& clip)
    {
        this->label.draw(clip);
        this->draw_cursor(clip);

        //top
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx(), 1
        )), 0x888888), this->rect);
        //left
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), 0x888888), this->rect);
        //right
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy() + 1, 1, this->cy() - 2
        )), 0x888888), this->rect);
        //bottom
        this->drawable->draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), 0x888888), this->rect);
    }

    void draw_cursor(const Rect& clip)
    {
        Rect cursor_clip = clip.intersect(
            Rect(this->label.x_text + this->cursor_px_pos + this->label.dx() - 1,
                 this->label.y_text + this->label.dy(),
                 1,
                 this->h_text)
        );
        if (!cursor_clip.isempty()) {
            this->drawable->draw(
                RDPOpaqueRect(
                    cursor_clip,
                    0x888888
                ), this->rect
            );
        }
    }

    void increment_edit_pos()
    {
        this->edit_pos++;
        size_t n = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 1);
        if (this->drawable) {
            char c = this->label.buffer[this->edit_buffer_pos + n];
            this->label.buffer[this->edit_buffer_pos + n] = 0;
            int w;
            this->drawable->text_metrics(this->label.buffer + this->edit_buffer_pos, w, this->h_text);
            this->cursor_px_pos += w;
            if (this->num_chars > 0)
                this->cursor_px_pos += 2;
            this->label.buffer[this->edit_buffer_pos + n] = c;
        }
        this->edit_buffer_pos += n;
    }

    size_t utf8len_current_char()
    {
        size_t len = 1;
        while (this->label.buffer[this->edit_buffer_pos + len] >> 6 == 2){
            ++len;
        }
        return len;
    }

    void decrement_edit_pos()
    {
        size_t len = 1;
        while (this->edit_buffer_pos - len - 1 >= 0 && this->label.buffer[this->edit_buffer_pos - len - 1] >> 6 == 2){
            ++len;
        }
        this->edit_pos--;
        if (this->drawable) {
            char c = this->label.buffer[this->edit_buffer_pos];
            this->label.buffer[this->edit_buffer_pos] = 0;
            int w;
            this->drawable->text_metrics(this->label.buffer + this->edit_buffer_pos - len, w, this->h_text);
            this->cursor_px_pos -= w;
            this->label.buffer[this->edit_buffer_pos] = c;
            if (this->num_chars > 0)
                this->cursor_px_pos -= 2;
        }
        this->edit_buffer_pos -= len;
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == CLIC_BUTTON1_DOWN) {
            if (x <= this->dx() + this->label.x_text) {
                this->edit_pos = 0;
                this->edit_buffer_pos = 0;
                this->cursor_px_pos = 0;
            }
            else if (x >= int(this->cursor_px_pos + this->label.dx() + this->label.x_text)) {
                if (this->edit_pos < this->num_chars) {
                    this->edit_pos = this->num_chars;
                    this->edit_buffer_pos = this->buffer_size;
                    this->cursor_px_pos = this->w_text;
                }
            } else {
                TODO("move cursor")
            }
        } else {
            Widget::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        if (keymap->nb_kevent_available() > 0){
            switch (keymap->top_kevent()){
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        this->decrement_edit_pos();
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        this->increment_edit_pos();
                    }
                    break;
                case Keymap2::KEVENT_BACKSPACE:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        this->num_chars--;
                        size_t pxtmp = this->cursor_px_pos;
                        this->decrement_edit_pos();
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->w_text -= pxtmp - this->cursor_px_pos;
                    }
                    break;
                case Keymap2::KEVENT_DELETE:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        size_t len = this->utf8len_current_char();
                        char c = this->label.buffer[this->edit_buffer_pos + len];
                        this->label.buffer[this->edit_buffer_pos + len] = 0;
                        int w;
                        this->drawable->text_metrics(this->label.buffer + this->edit_buffer_pos, w, this->h_text);
                        this->w_text -= w + 2;
                        this->label.buffer[this->edit_buffer_pos + len] = c;
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->num_chars--;
                    }
                    break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        this->edit_pos = this->num_chars;
                        this->edit_buffer_pos = this->buffer_size;
                        this->cursor_px_pos = this->w_text;
                        if (this->edit_pos) {
                            this->cursor_px_pos += 2;
                        }
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    this->edit_pos = 0;
                    this->edit_buffer_pos = 0;
                    this->cursor_px_pos = 0;
                    break;
                case Keymap2::KEVENT_KEY:
                    if (this->num_chars < WidgetLabel::buffer_size - 5) {
                        uint32_t c = keymap->get_char();
                        UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0, c, WidgetLabel::buffer_size - 1 - this->edit_buffer_pos);
                        size_t tmp = this->edit_buffer_pos;
                        size_t pxtmp = this->cursor_px_pos;
                        this->increment_edit_pos();
                        this->w_text += this->cursor_px_pos - pxtmp;
                        this->buffer_size += this->edit_buffer_pos - tmp;
                        this->num_chars++;
                        this->send_notify(NOTIFY_TEXT_CHANGED);
                    }
                    keymap->get_kevent();
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_SUBMIT);
                    break;
                default:
                    break;
            }
        }
    }
};

#endif

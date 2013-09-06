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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_EDIT_HPP

#include "widget.hpp"
#include "label.hpp"
#include <keymap2.hpp>

class WidgetEdit : public Widget2 {
public:
    WidgetLabel label;
    size_t buffer_size;
    size_t num_chars;
    size_t edit_buffer_pos;
    size_t edit_pos;
    size_t cursor_px_pos;
    int w_text;
    int h_text;
    int cursor_color;
    int border_top_left_color;
    int border_right_bottom_color;
    int border_right_bottom_color_inner;
    bool drawall;

    WidgetEdit(DrawApi& drawable, int16_t x, int16_t y, uint16_t cx,
               Widget2 & parent, NotifyApi* notifier, const char * text,
               int group_id = 0, int fgcolor = BLACK, int bgcolor = WHITE,
               std::size_t edit_position = -1, int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,cx,1), parent, notifier, group_id)
    , label(drawable, 0, 0, *this, 0, text, false, 0, fgcolor, bgcolor, xtext, ytext)
    , w_text(0)
    , h_text(0)
    , cursor_color(0x888888)
    , border_top_left_color(0x444444)
    , border_right_bottom_color(0xEEEEEE)
    , border_right_bottom_color_inner(0x888888)
    , drawall(false)
    {
        if (text) {
            this->buffer_size = strlen(text);
            this->num_chars = UTF8Len(this->label.buffer);
            this->edit_pos = std::min(this->num_chars, edit_position);
            this->edit_buffer_pos = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos);
            this->cursor_px_pos = 0;
            char c = this->label.buffer[this->edit_buffer_pos];
            this->label.buffer[this->edit_buffer_pos] = 0;
            this->drawable.text_metrics(this->label.buffer, this->w_text, this->h_text);
            this->cursor_px_pos = this->w_text;
            this->label.buffer[this->edit_buffer_pos] = c;
            int w, h;
            this->drawable.text_metrics(&this->label.buffer[this->edit_buffer_pos], w, h);
            this->w_text += w;
        } else {
            // this->drawable.text_metrics("abc", this->w_text, this->h_text);
            // LOG(LOG_INFO, "LOGIN_EDIT::constructor() w_text: %u, h_text: %u", this->w_text, this->h_text);
            this->buffer_size = 0;
            this->num_chars = 0;
            this->edit_buffer_pos = 0;
            this->edit_pos = 0;
            this->cursor_px_pos = 0;
        }

        int w = 0;
        this->drawable.text_metrics("Lp", w, this->h_text);
        this->rect.cy = this->h_text + this->label.y_text * 2;
        this->label.rect.cx = this->rect.cx;
        this->label.rect.cy = this->rect.cy;
        ++this->label.rect.x;
        ++this->label.rect.y;
        this->rect.cx += 2;
        this->rect.cy += 2;
        --this->h_text;
    }

    virtual ~WidgetEdit()
    {}

    void set_text(const char * text/*, int position = 0*/)
    {
        this->label.buffer[0] = 0;
        this->buffer_size = 0;
        this->num_chars = 0;
        if (text && *text) {
            this->buffer_size = std::min(WidgetLabel::buffer_size - 1, strlen(text));
            memcpy(this->label.buffer, text, this->buffer_size);
            this->label.buffer[this->buffer_size] = 0;
            this->drawable.text_metrics(this->label.buffer, this->w_text, this->h_text);
            if (this->label.auto_resize) {
                this->rect.cx = this->label.x_text * 2 + this->w_text;
                this->rect.cy = this->label.y_text * 2 + this->h_text;
                if (this->buffer_size == 1) {
                    this->rect.cx -= 2;
                }
            }
            this->num_chars = UTF8Len(this->label.buffer);
        }
        this->edit_pos = this->num_chars;
        this->edit_buffer_pos = this->buffer_size;
        this->cursor_px_pos = this->w_text;
    }

    const char * get_text() const
    {
        return this->label.get_text();
    }

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

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->set_edit_x(x);
        this->set_edit_y(y);
    }

    virtual void draw(const Rect& clip)
    {
        this->label.draw(clip);
        if (this->has_focus) {
            this->draw_cursor(this->get_cursor_rect());
        }
        this->draw_border(clip);
    }

    void draw_border(const Rect& clip)
    {
        //top
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy(), this->cx() - 1, 1
        )), this->border_top_left_color), this->rect);
        //left
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + 1, 1, this->cy() - 2
        )), this->border_top_left_color), this->rect);
        //right
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 1, this->dy(), 1, this->cy()
        )), this->border_right_bottom_color), this->rect);
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + this->cx() - 2, this->dy() + 1, 1, this->cy() - 3
        )), this->border_right_bottom_color_inner), this->rect);
        //bottom
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx(), this->dy() + this->cy() - 1, this->cx(), 1
        )), this->border_right_bottom_color), this->rect);
        this->drawable.draw(RDPOpaqueRect(clip.intersect(Rect(
            this->dx() + 1, this->dy() + this->cy() - 2, this->cx() - 2, 1
        )), this->border_right_bottom_color_inner), this->rect);
    }

    Rect get_cursor_rect() const
    {
        return Rect(this->label.x_text + this->cursor_px_pos + this->label.dx() + 1,
                    this->label.y_text + this->label.dy(),
                    1,
                    this->h_text);
    }

    void draw_cursor(const Rect& clip)
    {
        if (!clip.isempty()) {
            this->drawable.draw(RDPOpaqueRect(clip, this->cursor_color), this->rect);
        }
    }

    void increment_edit_pos()
    {
        this->edit_pos++;
        size_t n = UTF8GetPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 1);
        char c = this->label.buffer[this->edit_buffer_pos + n];
        this->label.buffer[this->edit_buffer_pos + n] = 0;
        int w;
        this->drawable.text_metrics(this->label.buffer + this->edit_buffer_pos, w, this->h_text);
        this->cursor_px_pos += w;
        this->label.buffer[this->edit_buffer_pos + n] = c;
        this->edit_buffer_pos += n;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }
    }

    size_t utf8len_current_char()
    {
        size_t len = 1;
        while ((this->label.buffer[this->edit_buffer_pos + len] & 0xC0) == 0x80){
            ++len;
        }
        return len;
    }

    void decrement_edit_pos()
    {
        size_t len = 1;
        while (this->edit_buffer_pos - len >= 0 &&
               ((this->label.buffer[this->edit_buffer_pos - len] & 0xC0) == 0x80)){
            ++len;
        }

        this->edit_pos--;
        char c = this->label.buffer[this->edit_buffer_pos];
        this->label.buffer[this->edit_buffer_pos] = 0;
        int w;
        this->drawable.text_metrics(this->label.buffer + this->edit_buffer_pos - len, w, this->h_text);
        this->cursor_px_pos -= w;
        this->label.buffer[this->edit_buffer_pos] = c;
        this->edit_buffer_pos -= len;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }
    }

    void update_draw_cursor(Rect old_cursor)
    {
        this->drawable.begin_update();
        if (this->drawall) {
            this->drawall = false;
            this->label.draw(this->rect);
        }
        else {
            this->label.draw(old_cursor);
        }
        this->draw_cursor(this->get_cursor_rect());
        this->drawable.end_update();
    }

    void move_to_last_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = this->num_chars;
        this->edit_buffer_pos = this->buffer_size;
        this->cursor_px_pos = this->w_text;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }

        this->update_draw_cursor(old_cursor_rect);
    }

    void move_to_first_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = 0;
        this->edit_buffer_pos = 0;
        this->cursor_px_pos = 0;

        if (this->label.shift_text(this->cursor_px_pos)) {
            this->drawall = true;
        }

        this->update_draw_cursor(old_cursor_rect);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            if (x <= this->dx() + this->label.x_text) {
                if (this->edit_pos) {
                    this->move_to_first_character();
                }
            }
            else if (x >= this->w_text + this->dx() + this->label.x_text) {
                if (this->edit_pos < this->num_chars) {
                    this->move_to_last_character();
                }
            }
            else {
                Rect old_cursor_rect = this->get_cursor_rect();
                int xx = this->dx() + this->label.x_text;
                size_t e = this->edit_pos;
                this->edit_pos = 0;
                this->edit_buffer_pos = 0;
                size_t len = this->utf8len_current_char();
                while (this->edit_buffer_pos < this->buffer_size) {
                    char c = this->label.buffer[this->edit_buffer_pos + len];
                    this->label.buffer[this->edit_buffer_pos + len] = 0;
                    int w, h;
                    this->drawable.text_metrics(this->label.buffer + this->edit_buffer_pos, w, h);
                    this->label.buffer[this->edit_buffer_pos + len] = c;
                    xx += w;
                    if (xx >= x) {
                        xx -= w;
                        break;
                    }
                    len = this->utf8len_current_char();
                    this->edit_buffer_pos += len;
                    ++this->edit_pos;
                }
                this->cursor_px_pos = xx - (this->dx() + this->label.x_text);
                if (e != this->edit_pos) {
                    this->update_draw_cursor(old_cursor_rect);
                }
            }
        } else {
            Widget2::rdp_input_mouse(device_flags, x, y, keymap);
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
                        Rect old_cursor_rect = this->get_cursor_rect();
                        this->decrement_edit_pos();
                        this->update_draw_cursor(old_cursor_rect);
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        Rect old_cursor_rect = this->get_cursor_rect();
                        this->increment_edit_pos();
                        this->update_draw_cursor(old_cursor_rect);
                    }
                    break;
                case Keymap2::KEVENT_BACKSPACE:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        this->num_chars--;
                        size_t pxtmp = this->cursor_px_pos;
                        Rect crect = this->get_cursor_rect();
                        size_t ebpos = this->edit_buffer_pos;
                        this->decrement_edit_pos();
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->buffer_size += this->edit_buffer_pos - ebpos;
                        this->drawable.begin_update();
                        this->drawable.draw(RDPOpaqueRect(crect, 0x888888), this->rect);
                        this->draw_cursor(this->get_cursor_rect());
                        this->label.draw(Rect(
                            this->dx() + this->cursor_px_pos + this->label.x_text + 3,
                            this->dy() + this->label.y_text + 1,
                            this->w_text - this->cursor_px_pos,
                            this->h_text
                        ));
                        this->drawable.end_update();
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
                        this->drawable.text_metrics(this->label.buffer + this->edit_buffer_pos, w, this->h_text);
                        this->label.buffer[this->edit_buffer_pos + len] = c;
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0);
                        this->buffer_size -= len;
                        this->num_chars--;
                        this->drawable.begin_update();
                        this->label.draw(Rect(
                            this->dx() + this->cursor_px_pos + this->label.x_text + 3,
                            this->dy() + this->label.y_text + 1,
                            this->w_text - this->cursor_px_pos,
                            this->h_text
                        ));
                        this->draw_cursor(this->get_cursor_rect());
                        this->drawable.end_update();
                        this->w_text -= w;
                    }
                    break;
                case Keymap2::KEVENT_END:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        this->move_to_last_character();
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    keymap->get_kevent();
                    if (this->edit_pos) {
                        this->move_to_first_character();
                    }
                    break;
                case Keymap2::KEVENT_KEY:
                    if (this->num_chars < WidgetLabel::buffer_size - 5) {
                        uint32_t c = keymap->get_char();
                        UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer + this->edit_buffer_pos), 0, c, WidgetLabel::buffer_size - 1 - this->edit_buffer_pos);
                        size_t tmp = this->edit_buffer_pos;
                        size_t pxtmp = this->cursor_px_pos;
                        this->increment_edit_pos();
                        this->buffer_size += this->edit_buffer_pos - tmp;
                        this->num_chars++;
                        this->send_notify(NOTIFY_TEXT_CHANGED);
                        this->w_text += this->cursor_px_pos - pxtmp;
                        this->update_draw_cursor(Rect(
                            this->dx() + pxtmp + this->label.x_text + 1,
                            this->dy() + this->label.y_text + 1,
                            this->w_text - pxtmp,
                            this->h_text
                        ));
                    }
                    keymap->get_kevent();
                    break;
                case Keymap2::KEVENT_ENTER:
                    keymap->get_kevent();
                    this->send_notify(NOTIFY_SUBMIT);
                    break;
                default:
                    Widget2::rdp_input_scancode(param1, param2, param3, param4, keymap);
                    break;
            }
        }
    }
};

#endif

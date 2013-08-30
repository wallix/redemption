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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_PASSWORD_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_PASSWORD_HPP

#include "label.hpp"
#include "widget.hpp"
#include <keymap2.hpp>

class WidgetPassword : public Widget2
{
public:
    static const size_t buffer_size = 256;
    char buffer[buffer_size];
    char display_pass[buffer_size];
    size_t buf_size;
    size_t buf_pos;
    size_t num_chars;
    size_t edit_pos;
    int w_char;
    int h_char;
    int x_text;
    int y_text;
    int bg_color;
    int fg_color;
    bool auto_resize;
    int cursor_color;
    int border_top_left_color;
    int border_right_bottom_color;
    int border_right_bottom_color_inner;

    WidgetPassword(DrawApi& drawable, int16_t x, int16_t y, uint16_t cx,
                   Widget2* parent, NotifyApi* notifier, const char * text,
                   int group_id = 0, int fgcolor = BLACK, int bgcolor = WHITE,
                   std::size_t edit_position = -1, int xtext = 0, int ytext = 0)
    : Widget2(drawable, Rect(x,y,cx,1), parent, notifier, group_id)
    , buf_size(0)
    , buf_pos(0)
    , num_chars(0)
    , edit_pos(0)
    , w_char(0)
    , h_char(0)
    , x_text(xtext+1)
    , y_text(ytext)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , auto_resize(false)
    , cursor_color(0x888888)
    , border_top_left_color(0x444444)
    , border_right_bottom_color(0xEEEEEE)
    , border_right_bottom_color_inner(0x888888)
    {
        this->buffer[0] = 0;
        this->display_pass[0] = 0;
        if (text && *text) {
            this->buf_size = std::min(buffer_size - 1, strlen(text));
            memcpy(this->buffer, text, this->buf_size);
            this->buffer[this->buf_size] = 0;


            this->num_chars = UTF8Len(text);
            this->edit_pos = std::min(this->num_chars, edit_position);
            this->buf_pos = UTF8GetPos(reinterpret_cast<uint8_t *>(this->buffer), this->edit_pos);
            for (size_t n = 0; n < this->num_chars; ++n) {
                this->display_pass[n] = '*';
            }
            this->display_pass[this->num_chars] = 0;
        }
        this->rect.cy = 0;
        this->drawable.text_metrics("*", this->w_char, this->h_char);
        this->rect.cy = this->y_text * 2 + this->h_char;
        this->rect.cy += 2;
        this->rect.cx += 2;
        --this->h_char;
    }

    virtual ~WidgetPassword()
    {}

    void set_text(const char * text)
    {
        this->buffer[0] = 0;
        this->display_pass[0] = 0;
        this->edit_pos = 0;
        this->buf_pos = 0;
        this->buf_size = 0;
        if (text) {
            this->buf_size = std::min(buffer_size - 1, strlen(text));
            memcpy(this->buffer, text, this->buf_size);
            this->buffer[this->buf_size] = 0;
            this->edit_pos = 0;
            this->buf_pos = 0;
            size_t len = UTF8Len(this->buffer);
            if (this->auto_resize) {
                this->rect.cx = this->x_text * 2 + this->w_char * len;
                if (this->buf_size == 1) {
                    this->rect.cx -= 2;
                }
            }
            for (size_t i = 0; i < len; ++i) {
                this->display_pass[i] = '*';
            }
            this->display_pass[len] = 0;
        }
    }

    void set_password_x(int x)
    {
        this->rect.x = x;
    }

    void set_password_y(int y)
    {
        this->rect.y = y;
    }

    virtual void draw(const Rect& clip)
    {
        LOG(LOG_INFO, "LOGIN_PASSWORD::label_draw() h_text: %u", this->h_char);
        this->draw_text(clip);

        if (this->has_focus) {
            this->draw_cursor(this->get_cursor_rect());
        }

        this->draw_border(clip);
    }

    void draw_text(const Rect& clip)
    {
        this->drawable.draw(RDPOpaqueRect(clip, this->bg_color), this->rect);
        this->drawable.server_draw_text(this->x_text + this->dx(),
                                         this->y_text + this->dy() + 1,
                                         this->display_pass,
                                         this->fg_color,
                                         this->bg_color,
                                         this->rect.intersect(clip)
        );
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
        return Rect(this->x_text + this->edit_pos * this->w_char + this->dx() + 1,
                    this->y_text + this->dy() + 1,
                    1,
                    this->h_char);
    }

    void draw_cursor(const Rect& clip)
    {
        if (!clip.isempty()) {
            this->drawable.draw(RDPOpaqueRect(clip, this->cursor_color), this->rect);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        if (device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            if (x <= this->dx() + this->x_text + this->w_char/2) {
                this->move_to_first_character();
            }
            else if (x >= int(this->dx() + this->x_text + this->w_char * this->num_chars)) {
                if (this->edit_pos < this->num_chars) {
                    this->move_to_last_character();
                }
            }
            else {
                Rect old_cursor_rect = this->get_cursor_rect();
                size_t e = this->edit_pos;
                this->edit_pos = std::min<size_t>((x - this->dx() + this->x_text - this->w_char/2) / this->w_char, this->num_chars-1);
                this->buf_pos = UTF8GetPos(reinterpret_cast<uint8_t*>(&this->buffer[0]), this->edit_pos);
                if (e != this->edit_pos) {
                    this->update_draw_cursor(old_cursor_rect);
                }
            }
        } else {
            Widget2::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    size_t utf8len_current_char()
    {
        size_t len = 1;
        while ((this->buffer[this->buf_pos + len] & 0xC0) == 0x80){
            ++len;
        }
        return len;
    }

    size_t utf8len_prevent_char()
    {
/*
        size_t len = 2;
LOG(LOG_INFO, "buf_pos=%d", this->buf_pos);
        while ((this->buffer[this->buf_pos - len] & 0xC0) == 0x80){
            ++len;
        }
        return len-1;
*/
        size_t len = 1;
//LOG(LOG_INFO, "buf_pos=%d", this->buf_pos);
        while ((this->buffer[this->buf_pos - len] & 0xC0) == 0x80){
            ++len;
        }
        return len;
    }

    void update_draw_cursor(Rect old_cursor)
    {
        this->drawable.begin_update();
        this->draw_cursor(this->get_cursor_rect());
        this->draw(old_cursor);
        this->drawable.end_update();
    }

    void move_to_last_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = this->num_chars;
        this->buf_pos = this->buf_size;
        this->update_draw_cursor(old_cursor_rect);
    }

    void move_to_first_character()
    {
        Rect old_cursor_rect = this->get_cursor_rect();
        this->edit_pos = 0;
        this->buf_pos = 0;
        this->update_draw_cursor(old_cursor_rect);
    }

    void remove_last_character()
    {
        --this->num_chars;
        this->display_pass[this->num_chars] = 0;
        this->drawable.begin_update();
        this->drawable.draw(
            RDPOpaqueRect(this->rect, this->bg_color),
            Rect(
                this->dx() + this->num_chars * this->w_char + this->x_text,
                this->dy() + this->y_text + 1,
                this->w_char,
                this->h_char
            )
        );
        this->drawable.end_update();
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
                        --this->edit_pos;
                        this->buf_pos -= this->utf8len_prevent_char();
                        this->update_draw_cursor(old_cursor_rect);
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        Rect old_cursor_rect = this->get_cursor_rect();
                        this->buf_pos += this->utf8len_current_char();
                        ++this->edit_pos;
                        this->update_draw_cursor(old_cursor_rect);
                    }
                    break;
                case Keymap2::KEVENT_BACKSPACE:
                    keymap->get_kevent();
                    if (this->edit_pos > 0) {
                        Rect old_cursor = this->get_cursor_rect();
                        --this->edit_pos;
//                        size_t d = this->utf8len_current_char();
                        size_t d = this->utf8len_prevent_char();
                        std::memmove(&this->buffer[this->buf_pos-d],
                                     &this->buffer[this->buf_pos],
                                     this->buf_size - this->buf_pos + 1);
                        this->buf_size -= d;
                        this->buf_pos -= d;
                        this->remove_last_character();
                        this->update_draw_cursor(old_cursor);
                    }
                    break;
                case Keymap2::KEVENT_DELETE:
                    keymap->get_kevent();
                    if (this->edit_pos < this->num_chars) {
                        Rect old_cursor = this->get_cursor_rect();
                        size_t d = this->utf8len_current_char();
                        std::memmove(&this->buffer[this->buf_pos],
                                     &this->buffer[this->buf_pos+d],
                                     this->buf_size - this->buf_pos + 1);
                        this->buf_size -= d;
                        this->remove_last_character();
                        this->update_draw_cursor(old_cursor);
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
                    this->move_to_first_character();
                    break;
                case Keymap2::KEVENT_KEY:
                    if (this->num_chars < this->buffer_size - 5) {
                        uint32_t c = keymap->get_char();
                        UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->buffer + this->buf_pos), 0, c, this->buffer_size - 1 - this->buf_pos);
                        ++this->edit_pos;
                        this->display_pass[this->num_chars] = '*';
                        ++this->num_chars;
                        this->display_pass[this->num_chars] = '\0';
                        size_t d = this->utf8len_current_char();
                        this->buf_size += d;
                        this->buf_pos += d;
                        this->send_notify(NOTIFY_TEXT_CHANGED);
                        this->drawable.begin_update();
                        this->draw_text(Rect(
                            this->dx() + this->x_text + (this->edit_pos - 1) * this->w_char,
                            this->dy() + this->y_text + 1,
                            this->w_char * this->num_chars,
                            this->h_char
                        ));
                        this->draw_cursor(this->get_cursor_rect());
                        this->drawable.end_update();
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

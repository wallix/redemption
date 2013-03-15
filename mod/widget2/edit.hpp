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

#if !defined(REDEMPTION_MOD_WIDGET2_EDIT_HPP_)
#define REDEMPTION_MOD_WIDGET2_EDIT_HPP_

#include "label.hpp"
#include <keymap2.hpp>

class WidgetEdit : public Widget
{
public:
    WidgetLabel label;
    size_t buffer_size;
    size_t num_chars;
    size_t edit_buffer_pos;
    size_t edit_pos;
    size_t cursor_px_pos;
    size_t prev_cursor_px_pos;
    int h_text;

public:
    WidgetEdit(ModApi * drawable, const Rect& rect, Widget * parent, NotifyApi * notifier, const char * text, size_t edit_position, int id = 0, int xtext = 0, int ytext = 0)
    : Widget(drawable, rect, parent, Widget::TYPE_EDIT, notifier, id)
    , label(drawable, rect, this, 0, text, 0, xtext, ytext)
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
                int w;
                this->drawable->text_metrics(this->label.buffer, w, this->h_text);
                this->label.buffer[this->edit_buffer_pos] = c;
                this->cursor_px_pos = w;
            }
        } else {
            this->buffer_size = 0;
            this->num_chars = 0;
            this->edit_buffer_pos = 0;
            this->edit_pos = 0;
            this->cursor_px_pos = 0;
        }
    }

    virtual ~WidgetEdit()
    {
    }

    virtual void draw(const Rect& rect, int16_t x, int16_t y, int16_t xclip, int16_t yclip)
    {
        Rect clip_screen(xclip, yclip, rect.cx, rect.cy);
        //this->clear_cursor(x, y, clip_screen);
        this->label.draw(rect, x, y, xclip, yclip);
        this->draw_cursor(x, y, clip_screen);
    }

    void draw_cursor(int16_t x_screen, int16_t y_screen, const Rect& clip_screen)
    {
        this->drawable->draw(RDPOpaqueRect(Rect(
            x_screen + this->cursor_px_pos,
            y_screen,
            2,
            this->h_text
        ), ~this->bg_color), clip_screen);
    }

    //void clear_cursor(int16_t x_screen, int16_t y_screen, const Rect& clip_screen)
    //{
    //    this->drawable->draw(RDPOpaqueRect(Rect(
    //        x_screen + this->prev_cursor_px_pos,
    //        y_screen,
    //        2,
    //        this->h_text
    //    ), this->bg_color), clip_screen);
    //}

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
            this->label.buffer[this->edit_buffer_pos + n] = c;
        }
        this->edit_buffer_pos += n;
    }

    void decrement_edit_pos()
    {
        size_t len = 1;
        while (this->label.buffer[this->edit_buffer_pos - len] >> 6 == 2){
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
        }
        this->edit_buffer_pos -= len;
    }

    void reload_context_text()
    {
        if (this->drawable) {
            delete this->label.context_text;
            this->label.context_text = this->drawable->create_context_text(this->label.buffer);
        }
    }

    void refresh_pos_cursor(size_t l, size_t r)
    {
        if (this->drawable) {
            screen_position sp = this->position_in_screen();
            Rect clip = sp.clip.intersect(Rect(sp.x + l + this->label.x_text, sp.y + this->label.y_text, r - l + 2, this->h_text));
            if (!clip.isempty()) {
                //this->clear_cursor(sp.x, sp.y, clip);
                this->label.draw(Rect(r + this->label.x_text, this->label.y_text, 2, this->h_text), sp.x, sp.y, clip.x, clip.y);
                this->draw_cursor(sp.x, sp.y, clip);
            }
        }
    }

    virtual void send_event(EventType event, int param, int param2, Keymap2 * keymap)
    {
        if (event == KEYDOWN)
        {
            switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_ENTER:
                    this->notify_parent(WIDGET_SUBMIT);
                    break;
                case Keymap2::KEVENT_LEFT_ARROW:
                case Keymap2::KEVENT_UP_ARROW:
                    if (this->edit_pos > 0) {
                        this->prev_cursor_px_pos = this->cursor_px_pos;
                        this->decrement_edit_pos();
                        this->refresh(this->rect.wh());
                        this->refresh_pos_cursor(this->cursor_px_pos, this->prev_cursor_px_pos);
                    }
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                case Keymap2::KEVENT_DOWN_ARROW:
                    if (this->edit_pos < (int)this->num_chars) {
                        this->prev_cursor_px_pos = this->cursor_px_pos;
                        this->increment_edit_pos();
                        this->refresh(this->rect.wh());
                        this->refresh_pos_cursor(this->prev_cursor_px_pos, this->cursor_px_pos);
                    }
                    break;
                case Keymap2::KEVENT_BACKSPACE:
                    if ((this->num_chars > 0) && (this->edit_pos > 0)) {
                        std::size_t size = this->label.context_text->cx - this->cursor_px_pos;
                        this->decrement_edit_pos();
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos);
                        this->num_chars--;
                        this->reload_context_text();
                        this->refresh(Rect(this->cursor_px_pos + this->label.x_text, this->label.y_text, size, this->h_text));
                    }
                    break;
                case Keymap2::KEVENT_DELETE:
                    if (this->num_chars > 0 && this->edit_pos < this->num_chars) {
                        UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos);
                        this->num_chars--;
                        std::size_t size = this->label.context_text->cx - this->cursor_px_pos - 2;
                        this->reload_context_text();
                        this->refresh(Rect(this->cursor_px_pos + this->label.x_text + 2, this->label.y_text, size, this->h_text));
                    }
                    break;
                case Keymap2::KEVENT_END:
                    if (this->edit_pos < this->num_chars) {
                        this->edit_pos = this->num_chars;
                        this->edit_buffer_pos = this->buffer_size;
                        if (this->label.context_text) {
                            this->prev_cursor_px_pos = this->cursor_px_pos;
                            this->cursor_px_pos += this->label.context_text->cx;
                            this->refresh_pos_cursor(this->prev_cursor_px_pos, this->cursor_px_pos);
                        }
                    }
                    break;
                case Keymap2::KEVENT_HOME:
                    if (this->edit_pos > 0) {
                        this->edit_pos = 0;
                        this->edit_buffer_pos = 0;
                        this->prev_cursor_px_pos = this->cursor_px_pos;
                        this->cursor_px_pos = 0;
                        this->refresh_pos_cursor(this->cursor_px_pos, this->prev_cursor_px_pos);
                    }
                    break;
                case Keymap2::KEVENT_KEY:
                    {
                        uint32_t c = keymap->top_char();
                        if (this->num_chars < 120) {
                            UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->label.buffer), this->edit_pos, c, 255);
                            this->prev_cursor_px_pos = this->cursor_px_pos;
                            size_t tmp = this->edit_buffer_pos;
                            this->increment_edit_pos();
                            this->buffer_size += tmp;
                            this->num_chars++;
                            this->reload_context_text();
                            this->notify_self(NOTIFY_TEXT_CHANGED);
                            this->refresh(Rect(this->prev_cursor_px_pos, 0, this->cursor_px_pos-this->prev_cursor_px_pos+2, this->h_text));
                        }
                    }
                    break;
                default:
                    this->notify_parent(event);
                    break;
            }
        } else {
            this->Widget::send_event(event, param, param2, keymap);
        }
    }
};

#endif
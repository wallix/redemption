/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni

*/

#if !defined(__MOD_INTERNAL_WIDGET_EDIT__)
#define __MOD_INTERNAL_WIDGET_EDIT__

#include "widget.hpp"
#include "internal/internal_mod.hpp"

struct widget_edit : public Widget {

    char buffer[256];

    widget_edit(internal_mod * mod, const Rect & r, Widget * parent, int id, int tab_stop, const char * caption, int pointer, int edit_pos)
    : Widget(mod, r.cx, r.cy, parent, WND_TYPE_EDIT) {

        assert(type == WND_TYPE_EDIT);

        this->rect.x = r.x;
        this->rect.y = r.y;
        this->tab_stop = tab_stop;
        this->id = id;
        this->buffer[0] = 0;
        if (caption){
            strncpy(this->buffer, caption, 255);
            this->buffer[255] = 0;
        }
        this->pointer = pointer;
        this->edit_pos = edit_pos;
        this->caption1 = 0;
    }

    ~widget_edit() {
    }

    virtual void draw(const Rect & clip)
    {

        Rect r(0, 0, this->rect.cx, this->rect.cy);
        const Rect scr_r = this->to_screen_rect(r);
        const Region region = this->get_visible_region(&this->mod->screen, this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->mod->draw_edit(scr_r,
                this->password_char,
                this->buffer,
                this->edit_pos,
                this->has_focus,
                region_clip);
        }
    }

    virtual void def_proc(const int msg, int const param1, int const param2, Keymap2 * keymap)
    {
        int n;
        if (msg == WM_KEYDOWN) {
            if ((keymap->top_kevent() == Keymap2::KEVENT_LEFT_ARROW)
            || (keymap->top_kevent() == Keymap2::KEVENT_UP_ARROW)){
                if (this->edit_pos > 0) {
                    this->edit_pos--;
                    this->refresh(this->rect.wh());
                }
            }
            if ((keymap->top_kevent() == Keymap2::KEVENT_RIGHT_ARROW)
            || (keymap->top_kevent() == Keymap2::KEVENT_DOWN_ARROW)){
                if (this->edit_pos < (int)mbstowcs(0, this->buffer, 0)) {
                    this->edit_pos++;
                    this->refresh(this->rect.wh());
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_BACKSPACE)) {
                n = mbstowcs(0, this->buffer, 0);
                if (n > 0) {
                    if (this->edit_pos > 0) {
                        this->edit_pos--;
                        remove_char_at(this->buffer, 255, this->edit_pos);
                        this->refresh(this->rect.wh());
                    }
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_DELETE)) {
                n = mbstowcs(0, this->buffer, 0);
                if (n > 0) {
                    if (this->edit_pos < n) {
                        remove_char_at(this->buffer, 255, this->edit_pos);
                        this->refresh(this->rect.wh());
                    }
                }
            }
            else if (keymap->top_kevent() == Keymap2::KEVENT_END) {
                keymap->get_kevent();
                n = mbstowcs(0, this->buffer, 0);
                if (this->edit_pos < n) {
                    this->edit_pos = n;
                    this->refresh(this->rect.wh());
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_HOME)) {
                if (this->edit_pos > 0) {
                    this->edit_pos = 0;
                    this->refresh(this->rect.wh());
                }
            }
            else {
                if (keymap->top_kevent() == Keymap2::KEVENT_KEY){
                    wchar_t c = keymap->top_char();
                    int num_chars = mbstowcs(0, this->buffer, 0);
                    if ((this->edit_pos >= num_chars) || (this->edit_pos < 0)) {
                        this->edit_pos = num_chars;
                    }

                    if (num_chars < 120) {
                        wchar_t wstr[num_chars + 16];
                        mbstowcs(wstr, this->buffer, num_chars + 1);
                        // make room by moving the end
                        for (int i = (num_chars - 1); i >= this->edit_pos; i--) {
                            wstr[i + 1] = wstr[i];
                        }
                        // store char at the right place
                        wstr[this->edit_pos] = c;
                        wstr[num_chars + 1] = 0;
                        TODO("check man page and perform checks for conversion failure cases")
                        wcstombs(this->buffer, wstr, 255);
                        this->edit_pos++;
                        this->refresh(this->rect.wh());
                    }
                }
            }
        }
    }
};

#endif

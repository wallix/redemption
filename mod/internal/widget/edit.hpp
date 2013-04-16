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

#ifndef _REDEMPTION_MOD_INTERNAL_WIDGET_EDIT_HPP_
#define _REDEMPTION_MOD_INTERNAL_WIDGET_EDIT_HPP_

#include "widget.hpp"

struct widget_edit : public Widget {

    char buffer[256];

    widget_edit(mod_api * mod, const Rect & r, Widget * parent, int id, int tab_stop, const char * caption, int pointer, int edit_pos)
    : Widget(mod, r, parent, WND_TYPE_EDIT) {

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
        const Region region = this->get_visible_region(this, this->parent, scr_r);

        for (size_t ir = 0 ; ir < region.rects.size() ; ir++){
            const Rect region_clip = region.rects[ir].intersect(this->to_screen_rect(clip));

            this->draw_edit(scr_r,
                this->password_char,
                this->buffer,
                this->edit_pos,
                this->has_focus,
                region_clip);
        }
    }

    void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus, const Rect & clip){
//        this->mod->draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), DARK_GREEN), clip);
        this->mod->draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), WHITE), clip);
        this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
        this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
        this->mod->draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), WHITE), clip);
        this->mod->draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);

        /* draw text */
        char text[255];
        const size_t len = strlen(buffer);
        if (password_char != 0) {
            memset(text, password_char, len);
            text[len] = 0;
        }
        else {
            memcpy(text, buffer, len+1);
        }

        this->mod->server_draw_text(r.x + 4, r.y + 2, text, WHITE, BLACK, clip);

        /* draw xor box(cursor) */
        if (has_focus) {
            UTF8TruncateAtPos(text, std::min<unsigned>(edit_pos, 255));
            int width = 0; int height = 0;
            TODO("As we are just looking for the end of bounding box to draw cursor, calling text_metrics is overkill."
                 "It would need some simpler function only computing width")
            this->mod->text_metrics(text, width, height);
            this->mod->draw(
                RDPOpaqueRect(Rect(r.x + 4 + width, r.y + 3, 2, r.cy - 6), BLACK),
                clip);
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
                if (this->edit_pos < (int)UTF8Len(this->buffer)) {
                    this->edit_pos++;
                    this->refresh(this->rect.wh());
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_BACKSPACE)) {
                n = UTF8Len(this->buffer);
                if ((n > 0) && (this->edit_pos > 0)) {
                    this->edit_pos--;
                    UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->buffer), this->edit_pos);
                    this->refresh(this->rect.wh());
                }
            }
            else if ((keymap->top_kevent() == Keymap2::KEVENT_DELETE)) {
                n = UTF8Len(this->buffer);
                if (n > 0 && this->edit_pos < n) {
                    UTF8RemoveOneAtPos(reinterpret_cast<uint8_t *>(this->buffer), this->edit_pos);
                    this->refresh(this->rect.wh());
                }
            }
            else if (keymap->top_kevent() == Keymap2::KEVENT_END) {
                n = UTF8Len(this->buffer);
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
                    uint32_t c = keymap->top_char();
                    int num_chars = UTF8Len(this->buffer);
                    if ((this->edit_pos >= num_chars) || (this->edit_pos < 0)) {
                        this->edit_pos = num_chars;
                    }

                    if (num_chars < 120) {
                        UTF8InsertOneAtPos(reinterpret_cast<uint8_t *>(this->buffer), this->edit_pos, c, 255);
                        this->edit_pos++;
                        this->refresh(this->rect.wh());
                    }
                }
            }
        }
    }
};

#endif

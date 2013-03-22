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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

*/

#ifndef _REDEMPTION_MOD_MOD_API_HPP_
#define _REDEMPTION_MOD_MOD_API_HPP_

#include"RDP/RDPGraphicDevice.hpp"

enum {
    BUTTON_STATE_UP   = 0,
    BUTTON_STATE_DOWN = 1,
};


struct mod_api : public RDPGraphicDevice {

    virtual void mod_event(int event_id) = 0;

    virtual void begin_update() = 0;
    virtual void end_update() = 0;

    void server_draw_dragging_rect(const Rect & r, const Rect & clip)
    {
        this->begin_update();

        RDPBrush brush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");

        // draw rectangles by drawing each edge top/bottom/left/right
        // 0x66 = xor -> pat_blt( ... 0x5A ...
        // 0xAA = noop -> pat_blt( ... 0xFB ...
        // 0xCC = copy -> pat_blt( ... 0xF0 ...
        // 0x88 = and -> pat_blt( ...  0xC0 ...

        this->draw(RDPPatBlt(Rect(r.x, r.y, r.cx, 5), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x, r.y + (r.cy - 5), r.cx, 5), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x, r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, brush), clip);
        this->draw(RDPPatBlt(Rect(r.x + (r.cx - 5), r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, brush), clip);
        this->end_update();
    }

    void draw_button(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip){

        int bevel = (state == BUTTON_STATE_DOWN)?1:0;

        this->draw(RDPOpaqueRect(r, GREY), clip);
        if (state == BUTTON_STATE_DOWN) {
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        } else {
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), WHITE), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), WHITE), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        }

        TODO("There is probably a way to move text_metrics into server_draw_text, or something similar")
        int w = 0;
        int h = 0;
        this->text_metrics(caption, w, h);
        this->server_draw_text(
            r.x + r.cx / 2 - w / 2 + bevel,
            r.y + r.cy / 2 - h / 2 + bevel,
            caption, GREY, BLACK, clip);
        // focus rect
        if (has_focus) {
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + r.cx - 6 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + r.cy - 6 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
        }
    }

    void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus, const Rect & clip){
//        this->draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), DARK_GREEN), clip);
        this->draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
        this->draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), WHITE), clip);
        this->draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);

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

        this->server_draw_text(r.x + 4, r.y + 2, text, WHITE, BLACK, clip);

        /* draw xor box(cursor) */
        if (has_focus) {
            UTF8TruncateAtPos(text, std::min<unsigned>(edit_pos, 255));
            int width = 0; int height = 0;
            TODO("As we are just looking for the end of bounding box to draw cursor, calling text_metrics is overkill."
                 "It would need some simpler function only computing width")
            this->text_metrics(text, width, height);
            this->draw(
                RDPOpaqueRect(Rect(r.x + 4 + width, r.y + 3, 2, r.cy - 6), BLACK),
                clip);
        }
    }

    virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) = 0;
    virtual void text_metrics(const char * text, int & width, int & height) = 0;
};

#endif

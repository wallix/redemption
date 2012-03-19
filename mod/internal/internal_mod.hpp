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

#if !defined(__INTERNAL_MOD_HPP__)
#define __INTERNAL_MOD_HPP__

#include "modcontext.hpp"
#include "internal/widget.hpp"
#include "client_mod.hpp"

struct internal_mod : public client_mod {
    public:
    widget_screen screen;
    int dragging;
    Rect dragging_rect;
    int draggingdx; // distance between mouse and top angle of dragged window
    int draggingdy; // distance between mouse and top angle of dragged window
    struct Widget* dragging_window;
    RDPBrush brush;

    internal_mod(FrontAPI & front, uint16_t width, uint16_t height)
            : client_mod(front), screen(this, width, height, 24)
    {
        /* dragging info */
        this->dragging = 0;
        this->event = event;
        // dragging_rect is (0,0,0,0)
        this->draggingdx = 0; // distance between mouse and top angle of dragged window
        this->draggingdy = 0; // distance between mouse and top angle of dragged window
        this->dragging_window = 0;
    }

    void server_draw_dragging_rect(const Rect & r, const Rect & clip)
    {
        this->front.begin_update();

        RDPBrush brush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");

        // draw rectangles by drawing each edge top/bottom/left/right
        // 0x66 = xor -> pat_blt( ... 0x5A ...
        // 0xAA = noop -> pat_blt( ... 0xFB ...
        // 0xCC = copy -> pat_blt( ... 0xF0 ...
        // 0x88 = and -> pat_blt( ...  0xC0 ...

        this->front.draw(RDPPatBlt(Rect(r.x, r.y, r.cx, 5), 0x5A, BLACK, WHITE, this->brush), clip);
        this->front.draw(RDPPatBlt(Rect(r.x, r.y + (r.cy - 5), r.cx, 5), 0x5A, BLACK, WHITE, this->brush), clip);
        this->front.draw(RDPPatBlt(Rect(r.x, r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, this->brush), clip);
        this->front.draw(RDPPatBlt(Rect(r.x + (r.cx - 5), r.y + 5, 5, r.cy - 10), 0x5A, BLACK, WHITE, this->brush), clip);
        this->front.end_update();
    }


    virtual ~internal_mod()
    {
//        this->screen.delete_all_childs();
    }

    size_t nb_windows()
    {
        return this->screen.child_list.size();
    }


    virtual void front_resize() {
        this->screen.rect.cx = this->front.get_front_width();
        this->screen.rect.cy = this->front.get_front_height();
        this->screen.bpp     = this->front.get_front_bpp();
    }

    const Rect & get_screen_rect() const
    {
        return this->screen.rect;
    }

    Widget * window(int i)
    {
        return this->screen.child_list[i];
    }

    widget_screen * get_screen_wdg(){
        return &(this->screen);
    }


    void draw_window(const Rect & r, uint32_t bgcolor, const char * caption, bool has_focus, const Rect & clip){

        // Window surface and border
        this->front.draw(RDPOpaqueRect(r, bgcolor), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), WHITE), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), WHITE), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + r.cy - 2, r.cx - 2, 1), DARK_GREY), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x+r.cx-2, r.y + 1, 1, r.cy), DARK_GREY), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), BLACK), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), BLACK), clip);

        // Title bar
        this->front.draw(RDPOpaqueRect(Rect(r.x + 3, r.y + 3, r.cx - 5, 18),
                          has_focus?WABGREEN:DARK_GREY), clip);

        this->front.server_draw_text(r.x + 4, r.y + 4, caption,
                has_focus?WABGREEN:DARK_GREY,
                has_focus?WHITE:BLACK, clip);
    }

    void draw_combo(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip)
    {
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, r.cy), GREY), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 3, r.cy - 3), WHITE), clip);
        if (has_focus) {
            this->front.draw(RDPOpaqueRect(Rect(r.x + 3, r.y + 3, (r.cx - 6) - 18, r.cy - 5), DARK_WABGREEN), clip);
        }
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), DARK_GREY), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), DARK_GREY), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy- 1, r.cx, 1), WHITE), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), BLACK), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), BLACK), clip);
        this->front.server_draw_text(r.x + 4, r.y + 3, caption, has_focus?DARK_WABGREEN:WHITE, has_focus?WHITE:BLACK, clip);
        this->draw_button(Rect(r.x + r.cx - 20, r.y + 2, 18, r.cy - 4), "", state, false, clip);
    }

    void draw_button(const Rect & r, const char * caption, int state, bool has_focus, const Rect & clip){

        int bevel = (state == BUTTON_STATE_DOWN)?1:0;

        this->front.draw(RDPOpaqueRect(r, GREY), clip);
        if (state == BUTTON_STATE_DOWN) {
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, r.cx - 2, 1), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + 1, 1, r.cy - 2), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + (r.cx - 2), r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        } else {
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), WHITE), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), WHITE), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + 1, r.y + (r.cy - 2), r.cx - 1, 1), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect((r.x + r.cx) - 2, r.y + 1, 1, r.cy - 1), DARK_GREY), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x, r.y + (r.cy - 1), r.cx, 1), BLACK), clip);
            this->front.draw(RDPOpaqueRect(Rect(r.x + (r.cx - 1), r.y, 1, r.cy), BLACK), clip);
        }

        TODO("There is probably a way to move text_metrics into server_draw_text, or something similar")
        int w = 0;
        int h = 0;
        this->front.text_metrics(caption, w, h);
        this->front.server_draw_text(
            r.x + r.cx / 2 - w / 2 + bevel,
            r.y + r.cy / 2 - h / 2 + bevel,
            caption, GREY, BLACK, clip);
        // focus rect
        if (has_focus) {
            this->front.draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->front.draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->front.draw(
                RDPPatBlt(Rect(r.x + r.cx - 6 + bevel, r.y + 3 + bevel, 2, r.cy - 8),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
            this->front.draw(
                RDPPatBlt(Rect(r.x + 3 + bevel, r.y + r.cy - 6 + bevel, r.cx - 8, 2),
                    0xF0, GREY, BLACK,
                    RDPBrush(r.x, r.y, 3, 0xaa, (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55")), clip);
        }
    }

    void draw_edit(const Rect & r, char password_char, char * buffer, size_t edit_pos, bool has_focus, const Rect & clip){
        this->front.draw(RDPOpaqueRect(Rect(r.x+1, r.y+1, r.cx - 3, r.cy - 3), DARK_GREEN), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, r.cx, 1), BLACK), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y, 1, r.cy), BLACK), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x, r.y + r.cy - 1, r.cx, 1), WHITE), clip);
        this->front.draw(RDPOpaqueRect(Rect(r.x + r.cx - 1, r.y, 1, r.cy), WHITE), clip);

        /* draw text */
        char text[255];
        wchar_t wtext[255];

        if (password_char != 0) {
            int i = mbstowcs(0, buffer, 0);
            memset(text, password_char, i);
            text[i] = 0;
            this->front.server_draw_text(r.x + 4, r.y + 2, text, DARK_GREEN, LIGHT_GREEN, clip);
        }
        else {
            this->front.server_draw_text(r.x + 4, r.y + 2, buffer, DARK_GREEN, LIGHT_GREEN, clip);
        }
        /* draw xor box(cursor) */
        if (has_focus) {
            if (password_char != 0) {
                for (size_t index = 0; index < edit_pos; index++) {
                    if (index >= 255) {
                        break;
                    }
                    wtext[index] = password_char;
                }
                wtext[edit_pos] = 0;
                wcstombs(text, wtext, 255);
            } else {
                mbstowcs(wtext, buffer, 255);
                wtext[edit_pos] = 0;
                wcstombs(text, wtext, 255);
            }

            int w = 0;
            int h = 0;
            this->front.text_metrics(text, w, h);
            this->front.draw(
                RDPOpaqueRect(Rect(r.x + 4 + w, r.y + 3, 2, r.cy - 6), PALE_GREEN),
                clip);
        }
    }



    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }

    virtual void rdp_input_invalidate(const Rect & rect) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap) = 0;
    virtual void rdp_input_scancode(long param1, long param2, long device_flags, long param4, const Keymap * keymap, const key_info* ki) = 0;

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // module got an internal event (like incoming data) and want to sent it outside
    virtual BackEvent_t draw_event()
    {
        return BACK_EVENT_NONE;
    }
};

#endif

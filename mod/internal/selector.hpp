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
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   RDP Secondary Connection Device Selector

*/

#if !defined(__SELECTOR_HPP__)
#define __SELECTOR_HPP__

#include <stdio.h>

struct selector_mod : public internal_mod {
    int signal;
    size_t focus_line;
    enum {
        FOCUS_ON_FILTER = 0,
        FOCUS_ON_GRID = 1,
        FOCUS_ON_LOGOUT = 2,
        FOCUS_ON_CANCEL = 3,
        FOCUS_ON_CONNECT = 4,
        MAX_FOCUS_ITEM = 5
    };
    unsigned focus_item;
    unsigned state;
    size_t showed;
    size_t total;
    uint32_t color[3];
    selector_mod(wait_obj * event, ModContext & context, Front & front):
            internal_mod(front), signal(0), focus_line(0), 
            focus_item(FOCUS_ON_LOGOUT), 
            state(BUTTON_STATE_UP),
            showed(100), total(1000)
    {
        this->color[0] = RED;
        this->color[1] = GREEN;
        this->color[2] = BLUE;
        this->event = event;
        this->event->set();
    }

    virtual ~selector_mod()
    {
    }

    virtual void rdp_input_invalidate(const Rect & rect)
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap)
    {
    }

    virtual void rdp_input_scancode(long param1, long param2, long flags, long time, const Keymap * keymap, const key_info* ki)
    {
        LOG(LOG_INFO, "param1=%u param2=%u flags=%x time=%u", param1, param2, flags, time);
        if (flags & 0xC000){ // KEYUP
            switch (param1){
                case 15:
                    this->focus_item = (this->focus_item + 1) % MAX_FOCUS_ITEM;
                    this->event->set();
                break;
                case 57: // SPACE
                case 28: // ENTER
                    this->state = BUTTON_STATE_UP;
                    this->event->set();
                break;
                default:
                break;
            }
        }
        else { // KEYPRESSED
            switch (param1){
            case 80: // ARROW_DOWN
                this->focus_line = (this->focus_line + 1) % 10;
                this->event->set();
            break;
            case 72: // ARROW_UP
                this->focus_line = (this->focus_line + 9) % 10;
                this->event->set();
            break;
            case 57: // SPACE
            case 28: // ENTER
                this->state = BUTTON_STATE_DOWN;
                this->event->set();
            break;
            default:
            break;
            }
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int draw_event()
    {
        this->draw(this->screen.rect);
        this->event->reset();
        return this->signal;
    }


    void draw_background(){
        this->opaque_rect(RDPOpaqueRect(this->screen.rect, WHITE));
    }

    void draw_login(){
        this->server_draw_text(30, 30, "Current user: cgr@10.10.4.13", WHITE, BLACK);
    }

    void draw_filter(){
        this->server_draw_text(30, 60, "Filter:", WHITE, BLACK);
        this->draw_edit(Rect(70, 60, 200, 20), 0, "*", 1, false);
        char buffer[256];
        sprintf(buffer, "Results: %u/%u", this->showed, this->total);
        this->server_draw_text(280, 60,  buffer, WHITE, BLACK);
    }

    void draw_array(){
        for (size_t line = 0 ; line < 10 ; line++){
            Rect rect(20, 100 + line * 20, this->screen.rect.cx-40, 19);
            uint32_t c = this->color[line%2];
            if (line == this->focus_line){
                c = this->color[2];
            }
            this->opaque_rect(RDPOpaqueRect(rect, c));
            this->server_draw_text(rect.x + 10, rect.y + 2, "account@device", c, BLACK);
        }
        this->server_draw_text(this->screen.rect.cx-240, 320, "|<<   <   1/10   >  >>|", WHITE, BLACK);
    }

    void draw_buttons(){
        Rect r(this->screen.rect.cx-240, this->screen.rect.cy- 100, 60, 25);
        this->draw_button(r, "Logout", 
            (this->focus_item == FOCUS_ON_LOGOUT)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_LOGOUT);
        this->draw_button(r.offset(70,0), "Cancel", 
            (this->focus_item == FOCUS_ON_CANCEL)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_CANCEL);
        this->draw_button(r.offset(140,0), "Connect", 
            (this->focus_item == FOCUS_ON_CONNECT)?this->state:BUTTON_STATE_UP,
            this->focus_item == FOCUS_ON_CONNECT);
    }


    void draw(const Rect & clip)
    {
        this->server_begin_update();
        this->server_set_clip(clip);

        this->draw_background();
        this->draw_login();
        this->draw_filter();
        this->draw_array();
        this->draw_buttons();

        this->server_end_update();
    }

};

#endif

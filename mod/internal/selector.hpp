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

   Use (implemented) basic RDP orders to draw some known test pattern

*/

#if !defined(__SELECTOR_HPP__)
#define __SELECTOR_HPP__

struct selector_mod : public internal_mod {
    size_t focus_line;
    uint32_t color[3];
    selector_mod(wait_obj * event, ModContext & context, Front & front):
            internal_mod(front), focus_line(0)
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
        if (flags & 0xC000){ // UP
        }
        else { // DOWN
            switch (param1){
            case 80:
                this->focus_line = (this->focus_line + 1) % 10;
                this->event->set();
            break;
            case 72:
                this->focus_line = (this->focus_line + 9) % 10;
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
        this->draw();
        this->event->reset();
        return 0;
    }


    void draw()
    {
        this->server_begin_update();
        this->server_set_clip(this->screen.rect);

        this->opaque_rect(RDPOpaqueRect(this->screen.rect, WHITE));
        this->server_draw_text(30, 30, "Current user: cgr@10.10.4.13", WHITE, BLACK);

        for (size_t line = 0 ; line < 10 ; line++){
            Rect rect(20, 100 + line * 20, this->screen.rect.cx-40, 19);
            uint32_t c = this->color[line%2];
            if (line == this->focus_line){
                c = this->color[2];
            }
            this->opaque_rect(RDPOpaqueRect(rect, c));
            this->server_draw_text(rect.x + 10, rect.y + 2, "account@device", c, BLACK);
        }

        this->server_end_update();
    }

};

#endif

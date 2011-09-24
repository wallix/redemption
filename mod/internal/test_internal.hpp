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

#if !defined(__TEST_INTERNAL_HPP__)
#define __TEST_INTERNAL_HPP__


struct test_internal_mod : public internal_mod {
    test_internal_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front):
            internal_mod(keys, key_flags, keymap, front)
    {
      this->mod_bpp = this->get_front_bpp();
        this->event = event;
        this->event->set();
    }

    virtual ~test_internal_mod()
    {
    }

    virtual void invalidate(const Rect & rect)
    {
        if (!rect.isempty()) {
            this->server_begin_update();
            Rect & r = this->screen.rect;
            this->input_event(WM_INVALIDATE,
                ((r.x & 0xffff) << 16) | (r.y & 0xffff),
                ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
                0, 0, this->key_flags, this->keys);

            /* draw any child windows in the area */
            for (size_t i = 0; i < this->nb_windows(); i++) {
                Widget *b = this->window(i);
                Rect r2 = rect.intersect(b->rect.wh());
                if (!r2.isempty()) {
                    b->refresh_clip(r2);
                }
            }
            this->server_end_update();
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y)
    {
        LOG(LOG_INFO, "input mouse");

        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->input_event(WM_MOUSEMOVE, x, y, 0, 0, this->key_flags, this->keys);
            this->front.mouse_x = x;
            this->front.mouse_y = y;

        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->input_event(
                WM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->input_event(
                WM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->input_event(
                WM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
            this->input_event(WM_BUTTON4DOWN, x, y, 0, 0, this->key_flags, this->keys);
            this->input_event(WM_BUTTON4UP, x, y, 0, 0, this->key_flags, this->keys);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
            this->input_event(WM_BUTTON5DOWN, x, y, 0, 0, this->key_flags, this->keys);
            this->input_event(WM_BUTTON5UP, x, y, 0, 0, this->key_flags, this->keys);
        }
    }

    virtual void rdp_input_scancode(int msg, long param1, long param2, long param3, long param4, const int key_flags, const int (& keys)[256], struct key_info* ki){
        LOG(LOG_INFO, "scan code");
        if (ki != 0) {
            this->input_event(msg, ki->chr, ki->sym, param1, param3, key_flags, keys);
        }
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        LOG(LOG_INFO, "overloaded by subclasses");
        return;
    }

    // event from front (keyboard or mouse event)
    int input_event(const int msg, const long param1, const long param2, const long param3, const long param4, const int key_flags, const int (& keys)[256])
    {
        return 0;
    }

    // event from back end (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int draw_event()
    {
        this->event->reset();
        this->draw();
        return 0;
    }

    void draw()
    {
        this->front.send_global_palette(this->palette332);

        this->server_begin_update();
//        this->color_cache(this->palette332, 0);
//        this->color_cache(this->palette332, 1);
//        this->color_cache(this->palette332, 2);
//        this->color_cache(this->palette332, 3);
//        this->color_cache(this->palette332, 4);
//        this->color_cache(this->palette332, 5);

//        #include "trace2008.cpp"
        #include "vnctest.cpp"

// {
//     RDPPatBlt cmd(Rect(381, 702, 10, 16), 240, 0xffffff, 0xd6d3ce,
//          RDPBrush(5, 7, 3, 170,            (const uint8_t *)"\x55\xaa\x55\xaa\x55\xaa\x55"));
//     this->front.pat_blt(cmd, Rect(0, 0, 1280, 800));
// }
//     {
//         RDPBrushCache cmd1(9, 1, 8, 8, 0, 8,(const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55");
//         this->front.orders->send(cmd1);
//         RDPBrush brush(5, 7, 0x81, 9);
//         RDPPatBlt cmd(Rect(381, 702, 10, 16), 240, 0xffffff, 0xd6d3ce, brush );
//         this->front.orders->send(cmd, Rect(0, 0, 1280, 800));
//     }
        this->server_end_update();
    }

};

#endif

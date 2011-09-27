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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Martin Potier
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Bouncer test, high level API

*/

#ifndef __BOUNCER2_HPP__
#define __BOUNCER2_HPP__

#include "client_mod.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include <unistd.h>

struct bouncer2_mod : public internal_mod {

    private:
        wait_obj * event;
        int speedx;
        int speedy;
        Rect * dancing_rect;
    public:

    bouncer2_mod(wait_obj * back_event, int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front) :
        internal_mod(keys, key_flags, keymap, front), event(back_event), speedx(10), speedy(10), dancing_rect(NULL)
    {
        this->server_begin_update();
        this->opaque_rect(RDPOpaqueRect(this->screen.rect, 0x00FF00));
        this->server_end_update();

        this->dancing_rect = new Rect(0,0,100,100);

        // Using µsec set
        this->event->set(33333);
    }

    ~bouncer2_mod()
    {}

    virtual void rdp_input_invalidate(const Rect & rect)
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

    // This should come from FRONT!
    int input_event(const int msg, const long x, const long y, const long param4, const long param5, const int key_flags, const int (& keys)[256])
    {
        // Get x% of the screen cx and cy
        int scarex = this->screen.rect.cx / 5;
        int scarey = this->screen.rect.cx / 5;
        Rect scareZone(this->dancing_rect->getCenteredX() - (scarex / 2),this->dancing_rect->getCenteredY() - (scarey / 2),scarex,scarey);

        // Calculating new speedx and speedy, if cube encounters a moving mouse pointer, it flees
        if (scareZone.rect_contains_pt(x,y)) {
            if (((this->dancing_rect->getCenteredX() - x) < scarex) && this->dancing_rect->getCenteredX() > x) {
                this->speedx = 2;
            } else if (((x - this->dancing_rect->getCenteredX()) < scarex) && x > this->dancing_rect->getCenteredX()) {
                this->speedx = -2;
            }
            if (((this->dancing_rect->getCenteredY() - y) < scarey) && this->dancing_rect->getCenteredY() > y) {
                this->speedy = 2;
            } else if (((y - this->dancing_rect->getCenteredY()) < scarey) && y > this->dancing_rect->getCenteredY()) {
                this->speedy = -2;
            }
        }
        return 0;
    }

    // This should come from BACK!
    virtual int draw_event()
    {
//        this->server_begin_update();
//        this->opaque_rect(RDPOpaqueRect(this->screen.rect, 0x00FF00));
//        this->server_end_update();
        // Creating a new RDP Order: OpaqueRect
        //RDPOpaqueRect white_rect(Rect(0, 0, 10, 10), 0xFFFFFF);
        //RDPOpaqueRect black_rect(Rect(0, 0, 10, 10), 0x000000);

        // Calculating new speedx and speedy
        if (this->dancing_rect->x <= 0 && this->speedx < 0) {
            this->speedx = -this->speedx;
        } else if (this->dancing_rect->x + this->dancing_rect->cx >= this->screen.rect.cx && this->speedx > 0) {
            this->speedx = -this->speedx;
        }
        if (this->dancing_rect->y <= 0 && this->speedy < 0) {
            this->speedy = -this->speedy;
        } else if (this->dancing_rect->y + this->dancing_rect->cy >= this->screen.rect.cy && this->speedy > 0) {
            this->speedy = -this->speedy;
        }

        // Saving old rect position
        Rect oldrect = this->dancing_rect->offset(0,0);

        // Setting the new position
        this->dancing_rect->x += this->speedx;
        this->dancing_rect->y += this->speedy;

        // Drawing the RECT
        this->server_begin_update();
        this->opaque_rect(RDPOpaqueRect(*this->dancing_rect, 0x0000FF));
        this->server_end_update();

        // And erase
        this->server_begin_update();
        this->wipe(oldrect, *this->dancing_rect, 0x00FF00);
        this->server_end_update();

        // Final with setting next idle time
        this->event->set(33333); // 0.3s is 30fps
        return 0;
    }

    void wipe(Rect oldrect, Rect newrect, int color) {
        // new RectIterator
        struct RectIt : public Rect::RectIterator {
            int color;
            bouncer2_mod & b;

            RectIt(int color, bouncer2_mod & b) : color(color), b(b)
            {}

            void callback(const Rect & a) {
                b.opaque_rect(RDPOpaqueRect(a, color));
            }
        } it(color, *this);

        // Use my iterator
        oldrect.difference(newrect, it);
    }
};

#endif

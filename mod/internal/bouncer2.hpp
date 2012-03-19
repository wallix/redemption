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

    bouncer2_mod(wait_obj * back_event, FrontAPI & front) :
        internal_mod(front), event(back_event), speedx(10), speedy(10), dancing_rect(NULL)
    {

        this->front.begin_update();
        this->front.draw(RDPOpaqueRect(this->get_screen_rect(), 0x00FF00), this->get_front_rect());
        this->front.end_update();

        this->dancing_rect = new Rect(0,0,100,100);

        // Using µsec set
        this->event->set(33333);
    }

    ~bouncer2_mod()
    {}

    virtual void rdp_input_invalidate(const Rect & rect)
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, const Keymap * keymap)
    {
    }

    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, const Keymap * keymap, const key_info* ki)
    {
        this->interaction();
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
    }

    int interaction()
    {
        // Get x% of the screen cx and cy
        long x = this->front.mouse_x;
        long y = this->front.mouse_y;
        int scarex = this->get_screen_rect().cx / 5;
        int scarey = this->get_screen_rect().cx / 5;
        Rect scareZone(this->dancing_rect->getCenteredX() - (scarex / 2),this->dancing_rect->getCenteredY() - (scarey / 2),scarex,scarey);

        // Calculating new speedx and speedy, if cube encounters a moving mouse pointer, it flees
        if (scareZone.contains_pt(x,y)) {
            if (((this->dancing_rect->getCenteredX() - x) < scarex)
            && this->dancing_rect->getCenteredX() > x) {
                this->speedx = 2;
            } else if (((x - this->dancing_rect->getCenteredX()) < scarex)
            && x > this->dancing_rect->getCenteredX()) {
                this->speedx = -2;
            }
            if (((this->dancing_rect->getCenteredY() - y) < scarey)
            && this->dancing_rect->getCenteredY() > y) {
                this->speedy = 2;
            } else if (((y - this->dancing_rect->getCenteredY()) < scarey) && y > this->dancing_rect->getCenteredY()) {
                this->speedy = -2;
            }
        }
        return 0;
    }

    // This should come from BACK!
    virtual BackEvent_t draw_event()
    {
//        this->front.begin_update();
//        this->opaque_rect(RDPOpaqueRect(this->get_screen_rect(), 0x00FF00), this->get_front_rect());
//        this->front.end_update();
        // Creating a new RDP Order: OpaqueRect
        //RDPOpaqueRect white_rect(Rect(0, 0, 10, 10), 0xFFFFFF);
        //RDPOpaqueRect black_rect(Rect(0, 0, 10, 10), 0x000000);

        this->interaction();
        // Calculating new speedx and speedy
        if (this->dancing_rect->x <= 0 && this->speedx < 0) {
            this->speedx = -this->speedx;
        } else if (this->dancing_rect->x + this->dancing_rect->cx >= this->get_screen_rect().cx && this->speedx > 0) {
            this->speedx = -this->speedx;
        }
        if (this->dancing_rect->y <= 0 && this->speedy < 0) {
            this->speedy = -this->speedy;
        } else if (this->dancing_rect->y + this->dancing_rect->cy >= this->get_screen_rect().cy && this->speedy > 0) {
            this->speedy = -this->speedy;
        }

        // Saving old rect position
        Rect oldrect = this->dancing_rect->offset(0,0);

        // Setting the new position
        this->dancing_rect->x += this->speedx;
        this->dancing_rect->y += this->speedy;

        // Drawing the RECT
        this->front.begin_update();
        this->front.draw(RDPOpaqueRect(*this->dancing_rect, 0x0000FF), this->get_front_rect());
        this->front.end_update();

        // And erase
        this->front.begin_update();
        this->wipe(oldrect, *this->dancing_rect, 0x00FF00, this->get_front_rect());
        this->front.end_update();

        // Final with setting next idle time
        this->event->set(33333); // 0.3s is 30fps
        return BACK_EVENT_NONE;
    }

    void wipe(Rect oldrect, Rect newrect, int color, const Rect & clip) {
        // new RectIterator
        struct RectIt : public Rect::RectIterator {
            int color;
            bouncer2_mod & b;
            const Rect & clip;

            RectIt(int color, bouncer2_mod & b, const Rect & clip)
            : color(color), b(b), clip(clip)
            {}

            void callback(const Rect & a) {
                b.front.draw(RDPOpaqueRect(a, color), this->clip);
            }
        } it(color, *this, clip);

        // Use my iterator
        oldrect.difference(newrect, it);
    }
};

#endif

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
#include "NewRDPOrders.hpp"
#include <unistd.h>

struct bouncer2_mod : public internal_mod {

    private:
        wait_obj * event;
        int speedx;
        int speedy;
        Rect * dancing_rect;
    public:

    bouncer2_mod(wait_obj * back_event, int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front) :
        internal_mod(keys, key_flags, keymap, front), event(back_event), speedx(5), speedy(5), dancing_rect(NULL)
    {
        printf("Pouwap !\n");

        this->server_begin_update();
        this->opaque_rect(RDPOpaqueRect(this->screen.rect, 0x00FF00));
        this->server_end_update();
        
        this->dancing_rect = new Rect(0,0,40,30);

        this->event->set();
    }

    ~bouncer2_mod()
    {}

    // This should come from FRONT!
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // This should come from BACK!
    virtual int mod_signal()
    {
        this->server_begin_update();
        this->opaque_rect(RDPOpaqueRect(this->screen.rect, 0x00FF00));
        this->server_end_update();
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
        
        usleep(100000);

        return 0;
    }

    void wipe(Rect oldrect, Rect newrect, int color) {
        this->opaque_rect(RDPOpaqueRect(newrect, color));
    }
};

#endif

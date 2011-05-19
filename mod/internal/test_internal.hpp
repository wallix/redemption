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
    Front & front;
    test_internal_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front, Session * session):
            internal_mod(keys, key_flags, keymap, front),
            front(front)
    {
        this->mod_bpp = 16;
        this->event = event;
        this->event->set();
    }

    virtual ~test_internal_mod()
    {
    }

    // event from front (keyboard or mouse event)
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // event from back en (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int mod_signal()
    {
        uint16_t cache_id;
        uint8_t cache_idx;

        this->event->reset();
        this->front.begin_update();
        this->front.orders->opaque_rect(Rect(0, 0, 1024, 768), color_encode(YELLOW, this->get_front_bpp(), this->palette332), Rect(0, 0, 1024, 768));

	// #include "../../trace19.inc"

        this->front.end_update();
        return 0;
    }

};

#endif

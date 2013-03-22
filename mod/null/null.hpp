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
   Author(s): Christophe Grosjean

*/

#ifndef _REDEMPTION_MOD_NULL_NULL_HPP_
#define _REDEMPTION_MOD_NULL_NULL_HPP_

#include "client_mod.hpp"

// Null module receive every event and does nothing. It allow session code to always have a receiving module active, thus avoidind to test that so back_end is available.

struct null_mod : public client_mod {

    null_mod(FrontAPI & front) : client_mod(front, 0, 0)
    {

    }

    virtual ~null_mod()
    {
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        return;
    }

    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
        return;
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        return;
    }

    virtual void rdp_input_invalidate(const Rect & r)
    {
        return;
    }

    // management of module originated event ("data received from server")
    // return non zero if module is "finished", 0 if it's still running
    // the null module never finish and accept any incoming event
    virtual BackEvent_t draw_event()
    {
        return BACK_EVENT_NONE;
    }

};

#endif

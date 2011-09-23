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

*/

#if !defined(__NULL_HPP__)
#define __NULL_HPP__

#include "client_mod.hpp"
#include "session.hpp"

// Null module receive every event and does nothing. It allow session code to always have a receiving module active, thus avoidind to test that so back_end is available.

struct null_mod : public client_mod {

    null_mod(int (& keys)[256], int & key_flags, Keymap * &keymap,
             struct ModContext & context, Front & front)
            : client_mod(keys, key_flags, keymap, front)
    {

    }

    virtual ~null_mod()
    {
    }

    // module received an event from client
    virtual int input_event(const int msg, const long x, const long y, const long param4, const long param5, const int key_flags, const int (& keys)[256])
    {
        return 0;
    }

    // management of module originated event ("data received from server")
    // return non zero if module is "finished", 0 if it's still running
    // the null module never finish and accept any incoming event
    virtual int draw_event()
    {
        return 0;
    }

    virtual void scancode(long param1, long param2, long param3, long param4){
        param1 = param1 % 128;
        int msg = WM_KEYUP;
        this->keys[param1] = 1 | param3;
        if ((param3 & KBD_FLAG_UP) == 0) { /* 0x8000 */
            /* key down */
            msg = WM_KEYDOWN;
            switch (param1) {
            case 58:
                this->key_flags ^= 4;
                break; /* caps lock */
            case 69:
                this->key_flags ^= 2;
                break; /* num lock */
            case 70:
                this->key_flags ^= 1;
                break; /* scroll lock */
            default:
                ;
            }
        }
        if (&this->keymap != NULL)
        {
            struct key_info* ki = this->keymap->get_key_info_from_scan_code(
                            param3,
                            param1,
                            this->keys,
                            this->key_flags);
            if (ki != 0) {
                this->input_event(msg, ki->chr, ki->sym, param1, param3, this->key_flags, this->keys);
            }
        }
        if (msg == WM_KEYUP){
            this->keys[param1] = 0;
        }
    }

};

#endif

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

#if !defined(__CLI_HPP__)
#define __CLI_HPP__

#include "../transitory/transitory.hpp"

struct cli_mod : public transitory_mod {

    cli_mod(
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        struct ModContext & context, Front & front)
            : transitory_mod(keys, key_flags, keymap, context, front)
    {

    }

    virtual ~cli_mod()
    {
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
                this->input_event(msg, ki->chr, ki->sym, param1, param3);
            }
        }
        if (msg == WM_KEYUP){
            this->keys[param1] = 0;
        }
    }

};

#endif

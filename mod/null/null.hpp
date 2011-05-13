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
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // management of module originated event ("data received from server")
    // return non zero if module is "finished", 0 if it's still running
    // the null module never finish and accept any incoming event
    virtual int mod_signal()
    {
        return 0;
    }

};

#endif

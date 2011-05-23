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

struct bouncer2_mod : public internal_mod {

    private:
    public:

    bouncer2_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front) :
        internal_mod(keys, key_flags, keymap, front)
    {
        front.begin_update();
        printf("Pouwap !\n");
        front.end_update();
        
    }

    ~bouncer2_mod()
    {}

    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    virtual int mod_signal()
    {
        printf("Canard\n");
        return 0;
    }
};

#endif

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

   This class implement abstract callback used by server_rdp layer
   it is used to avoid explicit dependency between rdp_server layer
   and session (to enable to use server_rdp layer in tests).

*/

#ifndef __CALLBACK_HPP__
#define __CALLBACK_HPP__

struct Callback
{
    virtual int callback(int msg, long param1, long param2, long param3, long param4) = 0;
};


#endif

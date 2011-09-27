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

   This class implement abstract callback used by front layer
   it is used to avoid explicit dependency between front layer
   and session (to enable to use front layer in tests).

*/

#ifndef __CALLBACK_HPP__
#define __CALLBACK_HPP__
#include "channel_list.hpp"

struct Callback
{
    virtual void send_to_mod_channel(const McsChannelItem & front_channel, uint8_t * data, size_t length, size_t chunk_size, uint32_t flags)
    {
        LOG(LOG_INFO, "overloaded by subclass");
    }
    virtual void set_key_flags(int key_flags) = 0;
    virtual int get_key_flags() = 0;
    virtual void rdp_input_scancode(long param1, long param2, long param3, long param4, const int key_flags, const int (& keys)[256], struct key_info* ki) = 0;
    virtual void rdp_input_mouse(int device_flags, int x, int y, const int key_flags, const int (& keys)[256]) = 0;
    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) = 0;
    virtual void rdp_input_invalidate(const Rect & r) = 0;
    virtual const Keymap * get_keymap() = 0;
    virtual const int  (& get_keys())[256] = 0;

};


#endif

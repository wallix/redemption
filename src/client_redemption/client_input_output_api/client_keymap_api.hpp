/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include <string_view>


class ClientKeyLayoutAPI {

public:

    virtual ~ClientKeyLayoutAPI() = default;

    virtual void update_keylayout(int LCID) = 0;

    virtual void key_event(int flag, int key, std::string_view text) = 0;

    virtual uint16_t get_scancode() = 0;

    virtual uint16_t get_flag() = 0;
};

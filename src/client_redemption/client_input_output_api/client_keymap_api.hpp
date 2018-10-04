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


#include "client_redemption/client_config/client_redemption_config.hpp"
// #include "client_redemption/client_config/client_redemption_config.hpp"
// #include "client_redemption/mod_wrapper/client_callback.hpp"



class ClientKeyLayoutAPI {

public:

    ClientKeyLayoutAPI() = default;

    virtual ~ClientKeyLayoutAPI() = default;

    virtual void update_keylayout(const int /*LCID*/) = 0;

    virtual void setCustomKeyCode(const int /*qtKeyID*/, const int /*scanCode*/, const std::string & /*ASCII8*/, const int /*extended*/) {}

    virtual void init(const int /*flag*/, const int /*key*/, std::string const& /*text*/) {}

    virtual int get_scancode() {
        return 0;
    }

    virtual int get_flag() {
        return 0;
    }

    virtual void clearCustomKeyCode() {}

    virtual KeyCustomDefinition get_key_info(int , std::string const& ) {
        KeyCustomDefinition key_info;
        return key_info;
    }


};

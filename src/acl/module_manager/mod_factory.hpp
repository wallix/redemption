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
  Copyright (C) Wallix 2019
  Author(s): Christophe Grosjean

    ModFactory : Factory class used to instanciate BackEnd modules

*/

#pragma once

#include "core/session_reactor.hpp"
#include "core/client_info.hpp"
#include "mod/internal/bouncer2_mod.hpp"

class ModFactory
{
    SessionReactor & session_reactor;
    ClientInfo & client_info;

public:
    ModFactory(SessionReactor & session_reactor, ClientInfo & client_info)
        : session_reactor(session_reactor)
        , client_info(client_info)
    {
    }

    auto create_mod_bouncer() -> mod_api*
    {
        auto new_mod = new Bouncer2Mod(
                            this->session_reactor,
                            this->client_info.screen_info.width,
                            this->client_info.screen_info.height);
        return new_mod;
    }
};

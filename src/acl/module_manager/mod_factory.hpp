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
#include "core/front_api.hpp" // for FrontAPI
#include "gdi/graphic_api.hpp"
#include "configs/config.hpp"

#include "mod/internal/bouncer2_mod.hpp"
#include "mod/internal/replay_mod.hpp"

class ModFactory
{
    SessionReactor & session_reactor;
    ClientInfo & client_info;
    FrontAPI & front;
    gdi::GraphicApi & graphics;
    Inifile & ini;

public:
    ModFactory(SessionReactor & session_reactor, ClientInfo & client_info, FrontAPI & front, gdi::GraphicApi & graphics, Inifile & ini)
        : session_reactor(session_reactor)
        , client_info(client_info)
        , front(front)
        , graphics(graphics)
        , ini(ini)
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

    auto create_mod_replay() -> mod_api*
    {
            auto new_mod = new ReplayMod(
                this->session_reactor,
                this->graphics, this->front,
                [this]{
                    auto movie_path = this->ini.get<cfg::video::replay_path>().as_string()
                                    + this->ini.get<cfg::globals::target_user>();
                    if (movie_path.size() < 5u 
                    || !std::equal(movie_path.end() - 5u, movie_path.end(), ".mwrm")) {
                        movie_path += ".mwrm";
                    }
                    return movie_path;
                }().c_str(),
                this->client_info.screen_info.width,
                this->client_info.screen_info.height,
                this->ini.get_mutable_ref<cfg::context::auth_error_message>(),
                !this->ini.get<cfg::mod_replay::on_end_of_data>(),
                this->ini.get<cfg::mod_replay::replay_on_loop>(),
                this->ini.get<cfg::video::play_video_with_corrupted_bitmap>(),
                to_verbose_flags(this->ini.get<cfg::debug::capture>())
            );
        return new_mod;
    }

};

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

#include "acl/mod_pack.hpp"

#include "core/session_reactor.hpp" // for SessionReactor
#include "core/client_info.hpp"     // for ClientInfo
#include "core/front_api.hpp"       // for FrontAPI
#include "gdi/graphic_api.hpp"      // for GraphicApi
#include "core/font.hpp"            // for Font
#include "configs/config.hpp"       // for Inifile
#include "RAIL/client_execute.hpp"
#include "utils/strutils.hpp"

// Modules
#include "mod/internal/bouncer2_mod.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "mod/internal/selector_mod.hpp"
#include "mod/internal/close_mod.hpp"
#include "mod/internal/interactive_target_mod.hpp"
#include "mod/internal/dialog_mod.hpp"
#include "mod/internal/wait_mod.hpp"
#include "mod/internal/transition_mod.hpp"
#include "mod/internal/login_mod.hpp"
#include "mod/xup/xup.hpp"
#include "acl/module_manager/create_mod_xup.hpp"

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "utils/translation.hpp"

class ModFactory
{
    ModWrapper & mod_wrapper;
    SessionReactor & session_reactor;
    SesmanInterface & sesman;
    TopFdContainer & fd_events_;
    GraphicFdContainer & graphic_fd_events_;
    TimerContainer & timer_events_;
    GraphicEventContainer & graphic_events_;
    GraphicTimerContainer & graphic_timer_events_;
    ClientInfo & client_info;
    FrontAPI & front;
    gdi::GraphicApi & graphics;
    Inifile & ini;
    Font & glyphs;
    const Theme & theme;
    ClientExecute & rail_client_execute;
    

public:
    ModFactory(ModWrapper & mod_wrapper, 
               SessionReactor & session_reactor,
               SesmanInterface & sesman,
               TopFdContainer & fd_events_,
               GraphicFdContainer & graphic_fd_events_,
               TimerContainer & timer_events_,
               GraphicEventContainer & graphic_events_,
               GraphicTimerContainer & graphic_timer_events_,
               ClientInfo & client_info, FrontAPI & front, gdi::GraphicApi & graphics, Inifile & ini, Font & glyphs, const Theme & theme, ClientExecute & rail_client_execute)
        : mod_wrapper(mod_wrapper)
        , session_reactor(session_reactor)
        , sesman(sesman)
        , fd_events_(fd_events_)
        , graphic_fd_events_(graphic_fd_events_)
        , timer_events_(timer_events_)
        , graphic_events_(graphic_events_)
        , graphic_timer_events_(graphic_timer_events_)
        , client_info(client_info)
        , front(front)
        , graphics(graphics)
        , ini(ini)
        , glyphs(glyphs)
        , theme(theme)
        , rail_client_execute(rail_client_execute)
    {
    }

    auto create_mod_bouncer() -> ModPack
    {
        auto new_mod = new Bouncer2Mod(
                            this->session_reactor,
                            this->graphic_timer_events_,
                            this->sesman,
                            this->front,
                            this->client_info.screen_info.width,
                            this->client_info.screen_info.height);
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_mod_replay() -> ModPack
    {
            auto new_mod = new ReplayMod(
                this->session_reactor,
                this->graphic_timer_events_,
                this->sesman,
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
        return {new_mod, nullptr, nullptr, nullptr};
    }
    
    auto create_widget_test_mod() -> ModPack
    {
        auto new_mod = new WidgetTestMod(
            this->session_reactor,
            this->graphic_timer_events_,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_test_card_mod() -> ModPack
    {
        auto new_mod = new TestCardMod(
            this->session_reactor,
            this->graphic_events_,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs,
            false
        ); 
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_selector_mod() -> ModPack
    {
        auto new_mod = new SelectorMod(
            this->ini,
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_close_mod() -> ModPack
    {
        bool back_to_selector = false;
        std::string auth_error_message = this->ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(this->ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message,
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            true,
            back_to_selector
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_close_mod_back_to_selector() -> ModPack
    {
        bool back_to_selector = true;
        std::string auth_error_message = this->ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(this->ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message,
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            true,
            back_to_selector
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_interactive_target_mod() -> ModPack
    {
        auto new_mod = new InteractiveTargetMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        ); 
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_valid_message_mod() -> ModPack
    {
        const char * message = this->ini.get<cfg::context::message>().c_str();
        const char * button = TR(trkeys::refused, language(this->ini));
        const char * caption = "Information";
        auto new_mod = new DialogMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            caption,
            message,
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_display_message_mod() -> ModPack
    {
        const char * message = this->ini.get<cfg::context::message>().c_str();
        const char * button = nullptr;
        const char * caption = "Information";
        auto new_mod = new DialogMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            caption,
            message,
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }
    
    auto create_dialog_challenge_mod() -> ModPack
    {
        const char * message = this->ini.get<cfg::context::message>().c_str();
        const char * button = nullptr;
        const char * caption = "Challenge";
        ChallengeOpt challenge = CHALLENGE_HIDE;
        if (this->ini.get<cfg::context::authentication_challenge>()) {
            challenge = CHALLENGE_ECHO;
        }
        this->ini.ask<cfg::context::authentication_challenge>();
        this->ini.ask<cfg::context::password>();
        auto new_mod = new DialogMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            caption,
            message,
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            challenge
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }

    auto create_wait_info_mod() -> ModPack
    {
        LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
        const char * message = this->ini.get<cfg::context::message>().c_str();
        const char * caption = TR(trkeys::information, language(this->ini));
        bool showform = this->ini.get<cfg::context::showform>();
        uint flag = this->ini.get<cfg::context::formflag>();
        auto new_mod = new WaitMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            caption,
            message,
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            showform,
            flag
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }
    
    auto create_transition_mod() -> ModPack
    {
        auto new_mod = new TransitionMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }
    
    auto create_login_mod() -> ModPack
    {
        char username[255]; // should use string
        username[0] = 0;
        LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
        if (!this->ini.is_asked<cfg::globals::auth_user>()){
            if (this->ini.is_asked<cfg::globals::target_user>()
             || this->ini.is_asked<cfg::globals::target_device>()){
                utils::strlcpy(
                    username,
                    this->ini.get<cfg::globals::auth_user>().c_str(),
                    sizeof(username));
            }
            else {
                // TODO check this! Assembling parts to get user login with target is not obvious method used below il likely to show @: if target fields are empty
                snprintf( username, sizeof(username), "%s@%s:%s%s%s"
                        , this->ini.get<cfg::globals::target_user>().c_str()
                        , this->ini.get<cfg::globals::target_device>().c_str()
                        , this->ini.get<cfg::context::target_protocol>().c_str()
                        , (!this->ini.get<cfg::context::target_protocol>().empty() ? ":" : "")
                        , this->ini.get<cfg::globals::auth_user>().c_str()
                        );
            }

            username[sizeof(username) - 1] = 0;
        }

        auto new_mod = new LoginMod(
            this->ini,
            this->session_reactor,
            this->timer_events_,
            this->graphic_events_,
            username,
            "", // password
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.cs_monitor.get_widget_rect(
                this->client_info.screen_info.width,
                this->client_info.screen_info.height
            )),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }    
    
    auto create_xup_mod(unique_fd & client_sck) -> ModPack
    {
        const char * name = "XUP Target";

        auto new_mod = new XupModWithSocket(
            this->mod_wrapper,
            this->ini,
            name,
            std::move(client_sck),
            this->ini.get<cfg::debug::mod_xup>(),
            nullptr,
            this->session_reactor,
            this->graphic_fd_events_,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            safe_int(this->ini.get<cfg::context::opt_bpp>())
        );
        return {new_mod, nullptr, nullptr, nullptr};
    }    
};

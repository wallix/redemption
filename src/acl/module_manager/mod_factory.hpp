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

#include "utils/timebase.hpp"
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
#include "core/events.hpp"

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "utils/translation.hpp"
#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
#include "acl/module_manager/acl_new_line_converter.hpp"

class ModFactory
{
    ModWrapper & mod_wrapper;
    TimeBase & time_base;
    AuthApi & sesman;
    EventContainer & events;
    ClientInfo & client_info;
    FrontAPI & front;
    gdi::GraphicApi & graphics;
    Inifile & ini;
    Font & glyphs;
    Theme & theme;
    ClientExecute & rail_client_execute;
    Keymap2 & keymap;
    FileSystemLicenseStore file_system_license_store{ app_path(AppPath::License).to_string() };
    Random & gen;
    TimeObj & timeobj;
    CryptoContext & cctx;
    std::array<uint8_t, 28> server_auto_reconnect_packet {};

public:
    ModFactory(ModWrapper & mod_wrapper,
               TimeBase & time_base,
               AuthApi & sesman,
               EventContainer & events,
               ClientInfo & client_info,
               FrontAPI & front,
               gdi::GraphicApi & graphics,
               Inifile & ini,
               Font & glyphs,
               Theme & theme,
               ClientExecute & rail_client_execute,
               Keymap2 & keymap,
               Random & gen,
               TimeObj & timeobj,
               CryptoContext & cctx
        )
        : mod_wrapper(mod_wrapper)
        , time_base(time_base)
        , sesman(sesman)
        , events(events)
        , client_info(client_info)
        , front(front)
        , graphics(graphics)
        , ini(ini)
        , glyphs(glyphs)
        , theme(theme)
        , rail_client_execute(rail_client_execute)
        , keymap(keymap)
        , gen(gen)
        , timeobj(timeobj)
        , cctx(cctx)
    {
    }


    auto create_mod(ModuleIndex target_module) -> ModPack
    {
        LOG(LOG_INFO, "New Module: %s", get_module_name(target_module));

        switch (target_module)
        {
        case MODULE_INTERNAL_BOUNCER2:
        {
            auto mod_pack = this->create_mod_bouncer();
            mod_pack.enable_osd = true;
            return mod_pack;
        }
        case MODULE_INTERNAL_TEST:
            return this->create_mod_replay();
        case MODULE_INTERNAL_WIDGETTEST:
            return this->create_widget_test_mod();
        case MODULE_INTERNAL_CARD:
            return this->create_test_card_mod();
        case MODULE_INTERNAL_SELECTOR:
            return this->create_selector_mod();
        case MODULE_INTERNAL_CLOSE:
            return this->create_close_mod();
        case MODULE_INTERNAL_CLOSE_BACK:
            return this->create_close_mod_back_to_selector();
        case MODULE_INTERNAL_TARGET:
            return this->create_interactive_target_mod();
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
            return this->create_valid_message_mod();
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
            return this->create_display_message_mod();
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
            return this->create_dialog_challenge_mod();
        case MODULE_INTERNAL_WAIT_INFO:
            return this->create_wait_info_mod();
        case MODULE_TRANSITORY:
            return this->create_transition_mod();
        case MODULE_INTERNAL_TRANSITION:
            return this->create_transition_mod();
        case MODULE_INTERNAL_LOGIN:
            return this->create_login_mod();
        case MODULE_RDP:
        {
            auto mod_pack = this->create_rdp_mod();
            mod_pack.enable_osd = true;
            mod_pack.connected = true;
            return mod_pack;
        }
        case MODULE_VNC:
        {
            auto mod_pack = this->create_vnc_mod();
            mod_pack.enable_osd = true;
            mod_pack.connected = true;
            return mod_pack;
        }
        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception %u", unsigned(target_module));
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }

    auto create_mod_bouncer() -> ModPack
    {
        auto new_mod = new Bouncer2Mod(
                            this->time_base,
                            this->mod_wrapper,
                            this->events,
                            this->front,
                            this->client_info.screen_info.width,
                            this->client_info.screen_info.height);
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_mod_replay() -> ModPack
    {
            auto new_mod = new ReplayMod(this->graphics, this->front,
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
                timeval{0, 0}, timeval{0, 0}, 0,
                this->ini.get<cfg::mod_replay::replay_on_loop>(),
                this->ini.get<cfg::video::play_video_with_corrupted_bitmap>(),
                to_verbose_flags(this->ini.get<cfg::debug::capture>())
            );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_widget_test_mod() -> ModPack
    {
        auto new_mod = new WidgetTestMod(
            this->time_base,
            this->mod_wrapper,
            this->events,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_test_card_mod() -> ModPack
    {
        auto new_mod = new TestCardMod(
            this->mod_wrapper,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs,
            false
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_selector_mod() -> ModPack
    {
        auto new_mod = new SelectorMod(
            this->ini,
            this->ini,
            this->time_base,
            this->events,
            this->sesman,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_close_mod() -> ModPack
    {
        LOG(LOG_INFO, "----------------------- create_close_mod() -> ModPack -----------------");

        bool back_to_selector = false;
        std::string auth_error_message = this->ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(this->ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message,
            this->ini,
            this->time_base,
            this->events,
            this->mod_wrapper, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            back_to_selector
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_close_mod_back_to_selector() -> ModPack
    {

        LOG(LOG_INFO, "----------------------- create_close_mod_back_to_selector() -> ModPack -----------------");

        bool back_to_selector = true;
        std::string auth_error_message = this->ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(this->ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message,
            this->ini,
            this->time_base,
            this->events,
            this->mod_wrapper, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            back_to_selector
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_interactive_target_mod() -> ModPack
    {
        auto new_mod = new InteractiveTargetMod(
            this->ini,
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_valid_message_mod() -> ModPack
    {
        AclNewLineConverter message{this->ini.get<cfg::context::message>()};
        const char * button = TR(trkeys::refused, language(this->ini));
        const char * caption = "Information";
        auto new_mod = new DialogMod(
            this->ini,
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            message.zstring().c_str(),
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_display_message_mod() -> ModPack
    {
        AclNewLineConverter message{this->ini.get<cfg::context::message>()};
        const char * button = nullptr;
        const char * caption = "Information";
        auto new_mod = new DialogMod(
            this->ini,
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            message.zstring().c_str(),
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_dialog_challenge_mod() -> ModPack
    {
        AclNewLineConverter message{this->ini.get<cfg::context::message>()};
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
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            message.zstring().c_str(),
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            challenge
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_wait_info_mod() -> ModPack
    {
        LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
        const char * caption = TR(trkeys::information, language(this->ini));
        AclNewLineConverter message{this->ini.get<cfg::context::message>()};
        bool showform = this->ini.get<cfg::context::showform>();
        uint flag = this->ini.get<cfg::context::formflag>();
        auto new_mod = new WaitMod(
            this->ini,
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            message.zstring().c_str(),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            showform,
            flag
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_transition_mod() -> ModPack
    {
        auto new_mod = new TransitionMod(
            this->ini,
            this->time_base,
            this->events,
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
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
            this->time_base,
            this->events,
            username,
            "", // password
            this->graphics, this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        return {new_mod, nullptr, nullptr, nullptr, false, false, nullptr};
    }

    auto create_rdp_mod() -> ModPack
    {
        auto new_mod_pack = create_mod_rdp(this->mod_wrapper,
            this->ini,
            this->mod_wrapper.get_graphics(),
            this->front,
            this->client_info,
            this->rail_client_execute,
            this->keymap.key_flags,
            this->glyphs, this->theme,
            this->time_base,
            this->events,
            this->sesman,
            this->file_system_license_store,
            this->gen,
            this->timeobj,
            this->cctx,
            this->server_auto_reconnect_packet);
        return new_mod_pack;
    }

    auto create_vnc_mod() -> ModPack
    {

        auto new_mod_pack = create_mod_vnc(this->mod_wrapper, this->ini,
            mod_wrapper.get_graphics(), this->front, this->client_info,
            this->rail_client_execute, this->keymap.key_flags,
            this->glyphs, this->theme,
            this->time_base,
            this->events,
            this->sesman,
            this->timeobj
        );
        return new_mod_pack;
    }
};

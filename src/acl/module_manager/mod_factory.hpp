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

#include "acl/mod_wrapper.hpp"
#include "acl/mod_pack.hpp"
#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"

#include "configs/config.hpp"
#include "RAIL/client_execute.hpp"
#include "utils/strutils.hpp"
#include "utils/redirection_info.hpp"
#include "utils/translation.hpp"
#include "utils/load_theme.hpp"

// Modules
#include "mod/internal/bouncer2_mod.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/internal/widget_test_mod.hpp"
#include "mod/internal/test_card_mod.hpp"
#include "mod/internal/selector_mod.hpp"
#include "mod/internal/close_mod.hpp"
#include "mod/internal/interactive_target_mod.hpp"
#include "mod/internal/dialog_mod.hpp"
#include "mod/internal/dialog_mod2.hpp"
#include "mod/internal/wait_mod.hpp"
#include "mod/internal/transition_mod.hpp"
#include "mod/internal/login_mod.hpp"


class ModFactory
{
    struct NoMod final : null_mod
    {
        bool is_up_and_running() const override { return false; }
    };

    SocketTransport * psocket_transport = nullptr;
    ModuleName current_mod = ModuleName::UNKNOWN;

    EventContainer & events;
    ClientInfo const& client_info;
    FrontAPI & front;
    gdi::GraphicApi & graphics;
    Inifile & ini;
    Font const & glyphs;
    Keymap & keymap;
    Random & gen;
    CryptoContext & cctx;
    std::array<uint8_t, 28> server_auto_reconnect_packet {};
    NoMod no_mod;
    FileSystemLicenseStore file_system_license_store{ app_path(AppPath::License).to_string() };

    Theme theme;
    ClientExecute rail_client_execute;
    ModWrapper mod_wrapper;
    RedirectionInfo redir_info;

public:
    ModFactory(EventContainer & events,
               CRef<TimeBase> time_base,
               CRef<ClientInfo> client_info,
               FrontAPI & front,
               gdi::GraphicApi & graphics,
               CRef<BGRPalette> palette,
               CRef<Font> glyphs,
               Inifile & ini,
               Keymap & keymap,
               Random & gen,
               CryptoContext & cctx
        )
        : events(events)
        , client_info(client_info)
        , front(front)
        , graphics(graphics)
        , ini(ini)
        , glyphs(glyphs)
        , keymap(keymap)
        , gen(gen)
        , cctx(cctx)
        , rail_client_execute(
            time_base, graphics, front,
            this->client_info.window_list_caps,
            ini.get<cfg::debug::mod_internal>() & 1)
        , mod_wrapper(
            no_mod, time_base, palette, graphics,
            client_info, glyphs, rail_client_execute, ini)
    {
        ::load_theme(theme, ini);
    }

    ~ModFactory()
    {
        if (&this->mod() != &this->no_mod){
            delete &this->mod();
        }
    }

public:
    RedirectionInfo& get_redir_info() noexcept
    {
        return redir_info;
    }

    ModuleName mod_name() const noexcept
    {
        return current_mod;
    }

    mod_api& mod() noexcept
    {
        return mod_wrapper.get_mod();
    }

    mod_api const& mod() const noexcept
    {
        return mod_wrapper.get_mod();
    }

    [[nodiscard]] SocketTransport* mod_sck_transport() const noexcept
    {
        return psocket_transport;
    }

    Callback& callback() noexcept
    {
        return mod_wrapper.get_callback();
    }

    void display_osd_message(std::string_view message,
                             gdi::OsdMsgUrgency omu = gdi::OsdMsgUrgency::NORMAL)
    {
        return mod_wrapper.display_osd_message(message, omu);
    }

    void set_time_close(MonotonicTimePoint t)
    {
        mod_wrapper.set_time_close(t);
    }

    bool is_connected() const noexcept
    {
        return this->psocket_transport;
    }

    void disconnect()
    {
        if (&mod() != &no_mod) {
            try {
                mod().disconnect();
            }
            catch (Error const& e) {
                LOG(LOG_ERR, "disconnect raised exception %d", static_cast<int>(e.id));
            }

            delete &mod();
            psocket_transport = nullptr;
            current_mod = ModuleName::UNKNOWN;
            mod_wrapper.set_mod(no_mod, nullptr, false);
        }
    }

private:
    void set_mod(ModuleName name, ModPack mod_pack, bool enable_osd)
    {
        this->keymap.reset_decoded_keys();

        this->mod_wrapper.clear_osd_message(false);

        if (&this->mod() != &this->no_mod){
            delete &this->mod();
        }

        this->current_mod = name;
        this->psocket_transport = mod_pack.psocket_transport;
        rail_client_execute.enable_remote_program(client_info.remote_program);
        mod_wrapper.set_mod(*mod_pack.mod, mod_pack.winapi, enable_osd);

        mod_pack.mod->init();
    }

public:
    void create_mod_bouncer()
    {
        auto new_mod = new Bouncer2Mod(
            this->graphics,
            this->events,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height);
        set_mod(ModuleName::bouncer2, {new_mod, nullptr, nullptr}, true);
    }

    void create_mod_replay()
    {
        auto new_mod = new ReplayMod(
            this->events,
            this->graphics,
            this->front,
            str_concat(
                this->ini.get<cfg::mod_replay::replay_path>().as_string(),
                this->ini.get<cfg::globals::target_user>(),
                ".mwrm"_av
            ),
            // this->client_info.screen_info.width,
            // this->client_info.screen_info.height,
            this->ini.get_mutable_ref<cfg::context::auth_error_message>(),
            !this->ini.get<cfg::mod_replay::on_end_of_data>(),
            this->ini.get<cfg::mod_replay::replay_on_loop>(),
            this->ini.get<cfg::video::play_video_with_corrupted_bitmap>(),
            safe_cast<FileToGraphicVerbose>(this->ini.get<cfg::debug::capture>())
        );
        set_mod(ModuleName::autotest, {new_mod, nullptr, nullptr}, false);
    }

    void create_widget_test_mod()
    {
        auto new_mod = new WidgetTestMod(
            this->graphics,
            this->events,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::widgettest, {new_mod, nullptr, nullptr}, true);
    }

    void create_test_card_mod()
    {
        auto new_mod = new TestCardMod(
            this->graphics,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->glyphs,
            false
        );
        set_mod(ModuleName::card, {new_mod, nullptr, nullptr}, false);
    }

    void create_selector_mod()
    {
        auto new_mod = new SelectorMod(
            this->ini,
            this->mod_wrapper.get_graphics(),
            this->mod_wrapper,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::selector, {new_mod, nullptr, nullptr}, false);
    }

    void create_close_mod()
    {
        LOG(LOG_INFO, "----------------------- create_close_mod() -----------------");

        bool back_to_selector = false;
        set_mod(ModuleName::close, this->_create_close_mod(back_to_selector), false);
    }

    void create_close_mod_back_to_selector()
    {
        LOG(LOG_INFO, "----------------------- create_close_mod_back_to_selector() -----------------");

        bool back_to_selector = true;
        set_mod(ModuleName::close_back, this->_create_close_mod(back_to_selector), false);
    }

private:
    ModPack _create_close_mod(bool back_to_selector)
    {
        zstring_view auth_error_message = this->ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(this->ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message.c_str(),
            this->ini,
            this->events,
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            back_to_selector
        );
        return {new_mod, nullptr, nullptr};
    }

public:
    void create_interactive_target_mod()
    {
        auto new_mod = new InteractiveTargetMod(
            this->ini,
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::interactive_target, {new_mod, nullptr, nullptr}, false);
    }

    void create_valid_message_mod()
    {
        const char * button = TR(trkeys::refused, language(this->ini));
        const char * caption = "Information";
        set_mod(ModuleName::valid, this->_create_dialog(button, caption, NO_CHALLENGE), false);
    }

    void create_display_message_mod()
    {
        const char * button = nullptr;
        const char * caption = "Information";
        set_mod(ModuleName::confirm, this->_create_dialog(button, caption, NO_CHALLENGE), false);
    }

    void create_dialog_challenge_mod()
    {
        const char * button = nullptr;
        const char * caption = "Challenge";
        const ChallengeOpt challenge = this->ini.get<cfg::context::authentication_challenge>()
            ? CHALLENGE_ECHO
            : CHALLENGE_HIDE;
        set_mod(ModuleName::challenge, this->_create_dialog(button, caption, challenge), false);
    }

    void create_display_link_mod()
    {
        const char * caption = "URL Redirection";
        const char * link_label = "Copy to clipboard: ";
        // return this->_create_dialog(button, caption, NO_CHALLENGE);
        auto new_mod = new DialogMod2(
            this->ini,
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            this->ini.get<cfg::context::message>().c_str(),
            this->ini.get<cfg::context::display_link>().c_str(),
            link_label,
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::link_confirm, {new_mod, nullptr, nullptr}, false);
    }

private:
    ModPack _create_dialog(const char * button, const char * caption, ChallengeOpt challenge)
    {
        auto new_mod = new DialogMod(
            this->ini,
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            this->ini.get<cfg::context::message>().c_str(),
            button,
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            challenge
        );
        return {new_mod, nullptr, nullptr};
    }

public:
    void create_wait_info_mod()
    {
        LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Wait Info Message'");
        const char * caption = TR(trkeys::information, language(this->ini));
        bool showform = this->ini.get<cfg::context::showform>();
        uint flag = this->ini.get<cfg::context::formflag>();
        auto new_mod = new WaitMod(
            this->ini,
            this->events,
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            caption,
            this->ini.get<cfg::context::message>().c_str(),
            this->rail_client_execute,
            this->glyphs,
            this->theme,
            showform,
            flag
        );
        set_mod(ModuleName::waitinfo, {new_mod, nullptr, nullptr}, false);
    }

    void create_transition_mod()
    {
        auto new_mod = new TransitionMod(
            TR(trkeys::wait_msg, language(this->ini)),
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::transitory, {new_mod, nullptr, nullptr}, false);
    }

    void create_login_mod()
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
                        , this->ini.get<cfg::context::target_protocol>().empty() ? "" : ":"
                        , this->ini.get<cfg::globals::auth_user>().c_str()
                        );
            }

            username[sizeof(username) - 1] = 0;
        }

        auto new_mod = new LoginMod(
            this->ini,
            this->events,
            username,
            "", // password
            this->graphics,
            this->front,
            this->client_info.screen_info.width,
            this->client_info.screen_info.height,
            this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
            this->rail_client_execute,
            this->glyphs,
            this->theme
        );
        set_mod(ModuleName::waitinfo, {new_mod, nullptr, nullptr}, false);
    }

    void create_rdp_mod(
        SessionLogApi& session_log,
        PerformAutomaticReconnection perform_automatic_reconnection
    )
    {
        auto mod_pack = create_mod_rdp(
            this->mod_wrapper.get_graphics(),
            this->mod_wrapper,
            this->redir_info,
            this->ini,
            this->front,
            this->client_info,
            this->rail_client_execute,
            this->keymap.locks(),
            this->glyphs, this->theme,
            this->events,
            session_log,
            this->file_system_license_store,
            this->gen,
            this->cctx,
            this->server_auto_reconnect_packet,
            perform_automatic_reconnection);
        set_mod(ModuleName::RDP, mod_pack, true);
    }

    void create_vnc_mod(SessionLogApi& session_log)
    {
        auto mod_pack = create_mod_vnc(
            this->mod_wrapper.get_graphics(),
            this->ini,
            this->front, this->client_info,
            this->rail_client_execute,
            this->keymap.layout(),
            this->keymap.locks(),
            this->glyphs, this->theme,
            this->events,
            session_log);
        set_mod(ModuleName::VNC, mod_pack, true);
    }
};

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

#include "acl/module_manager/mod_factory.hpp"
#include "acl/mod_pack.hpp"

#include "configs/config.hpp"
#include "utils/strutils.hpp"
#include "utils/translation.hpp"
#include "utils/load_theme.hpp"
#include "mod/null/null.hpp"
#include "keyboard/keymap.hpp"
#include "core/client_info.hpp"

// Modules
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"
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

namespace
{
    struct NoMod final : null_mod
    {
        bool is_up_and_running() const override { return false; }
    };

    inline NoMod no_mod;
}

ModFactory::ModFactory(
    EventContainer & events,
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
, file_system_license_store{ app_path(AppPath::License).to_string() }
, rail_client_execute(
    time_base, graphics, front,
    this->client_info.window_list_caps,
    ini.get<cfg::debug::mod_internal>() & 1)
, mod_wrapper(
    no_mod, time_base, palette, graphics,
    client_info, glyphs, rail_client_execute, ini)
{
    ::load_theme(theme, ini);

    this->rail_client_execute.enable_remote_program(this->client_info.remote_program);
}

ModFactory::~ModFactory()
{
    if (&this->mod() != &no_mod){
        delete &this->mod();
    }
}

void ModFactory::disconnect()
{
    if (&mod() != &no_mod) {
        try {
            mod().disconnect();
        }
        catch (Error const& e) {
            LOG(LOG_ERR, "disconnect raised exception %d", static_cast<int>(e.id));
        }

        psocket_transport = nullptr;
        current_mod = ModuleName::UNKNOWN;
        auto * previous_mod = &mod();
        mod_wrapper.set_mod(no_mod, nullptr, false);
        delete previous_mod;
    }
}

static ModPack mod_pack_from_widget(mod_api* mod)
{
    return {mod, nullptr, nullptr, false};
}

struct ModFactory::D
{
    static void set_mod(ModFactory& self, ModuleName name, ModPack mod_pack, bool enable_osd)
    {
        self.keymap.reset_decoded_keys();

        self.mod_wrapper.clear_osd_message(false);

        if (&self.mod() != &no_mod){
            delete &self.mod();
        }

        self.current_mod = name;
        self.psocket_transport = mod_pack.psocket_transport;
        self.rail_client_execute.enable_remote_program(self.client_info.remote_program);
        self.mod_wrapper.set_mod(*mod_pack.mod, mod_pack.winapi, enable_osd);

        if (self.rail_client_execute.is_rail_enabled() && !self.rail_client_execute.is_ready()) {
            bool can_resize_hosted_desktop = mod_pack.can_resize_hosted_desktop
                                          && self.ini.get<cfg::remote_program::allow_resize_hosted_desktop>()
                                          && !self.ini.get<cfg::globals::is_rec>();
            self.rail_client_execute.ready(*mod_pack.mod, self.glyphs, can_resize_hosted_desktop);
        }
    }

    static ModPack create_close_mod(ModFactory& self, bool back_to_selector)
    {
        zstring_view auth_error_message = self.ini.get<cfg::context::auth_error_message>();
        if (auth_error_message.empty()) {
            auth_error_message = TR(trkeys::connection_ended, language(self.ini));
        }

        auto new_mod = new CloseMod(
            auth_error_message.c_str(),
            self.ini,
            self.events,
            self.graphics,
            self.client_info.screen_info.width,
            self.client_info.screen_info.height,
            self.rail_client_execute.adjust_rect(self.client_info.get_widget_rect()),
            self.rail_client_execute,
            self.glyphs,
            self.theme,
            back_to_selector
        );
        return mod_pack_from_widget(new_mod);
    }

    static ModPack create_dialog(
        ModFactory& self, const char* button, const char* caption,
        ChallengeOpt challenge)
    {
        auto new_mod = new DialogMod(
            self.ini,
            self.graphics,
            self.front,
            self.client_info.screen_info.width,
            self.client_info.screen_info.height,
            self.rail_client_execute.adjust_rect(self.client_info.get_widget_rect()),
            caption,
            self.ini.get<cfg::context::message>().c_str(),
            button,
            self.rail_client_execute,
            self.glyphs,
            self.theme,
            copy_paste(self),
            challenge
        );
        return mod_pack_from_widget(new_mod);
    }

    static CopyPaste& copy_paste(ModFactory& self)
    {
        if (!self.copy_paste_ptr) {
            const auto verbosity = self.ini.get<cfg::debug::mod_internal>();
            self.copy_paste_ptr = std::make_unique<CopyPaste>(verbosity != 0);
            self.copy_paste_ptr->ready(self.front);
        }
        return *self.copy_paste_ptr;
    }
};

void ModFactory::create_mod_bouncer()
{
    auto new_mod = new Bouncer2Mod(
        this->graphics,
        this->events,
        this->client_info.screen_info.width,
        this->client_info.screen_info.height);
    D::set_mod(*this, ModuleName::bouncer2, mod_pack_from_widget(new_mod), true);
}

void ModFactory::create_mod_replay()
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
    D::set_mod(*this, ModuleName::autotest, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_widget_test_mod()
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
    D::set_mod(*this, ModuleName::widgettest, mod_pack_from_widget(new_mod), true);
}

void ModFactory::create_test_card_mod()
{
    auto new_mod = new TestCardMod(
        this->graphics,
        this->client_info.screen_info.width,
        this->client_info.screen_info.height,
        this->glyphs,
        false
    );
    D::set_mod(*this, ModuleName::card, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_selector_mod()
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
        this->theme,
        D::copy_paste(*this)
    );
    D::set_mod(*this, ModuleName::selector, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_close_mod()
{
    LOG(LOG_INFO, "----------------------- create_close_mod() -----------------");

    bool back_to_selector = false;
    D::set_mod(*this, ModuleName::close, D::create_close_mod(*this, back_to_selector), false);
}

void ModFactory::create_close_mod_back_to_selector()
{
    LOG(LOG_INFO, "----------------------- create_close_mod_back_to_selector() -----------------");

    bool back_to_selector = true;
    D::set_mod(*this, ModuleName::close_back, D::create_close_mod(*this, back_to_selector), false);
}

void ModFactory::create_interactive_target_mod()
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
        this->theme,
        D::copy_paste(*this)
    );
    D::set_mod(*this, ModuleName::interactive_target, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_valid_message_mod()
{
    const char * button = TR(trkeys::refused, language(this->ini));
    const char * caption = "Information";
    auto mod_pack = D::create_dialog(*this, button, caption, NO_CHALLENGE);
    D::set_mod(*this, ModuleName::valid, mod_pack, false);
}

void ModFactory::create_display_message_mod()
{
    const char * button = nullptr;
    const char * caption = "Information";
    auto mod_pack = D::create_dialog(*this, button, caption, NO_CHALLENGE);
    D::set_mod(*this, ModuleName::confirm, mod_pack, false);
}

void ModFactory::create_dialog_challenge_mod()
{
    const char * button = nullptr;
    const char * caption = "Challenge";
    const ChallengeOpt challenge = this->ini.get<cfg::context::authentication_challenge>()
        ? CHALLENGE_ECHO
        : CHALLENGE_HIDE;
    auto mod_pack = D::create_dialog(*this, button, caption, challenge);
    D::set_mod(*this, ModuleName::challenge, mod_pack, false);
}

void ModFactory::create_display_link_mod()
{
    const char * caption = "URL Redirection";
    const char * link_label = "Copy to clipboard: ";
    // return D::create_dialog(*this, button, caption, NO_CHALLENGE);
    auto new_mod = new DialogMod2(
        this->ini,
        this->graphics,
        this->client_info.screen_info.width,
        this->client_info.screen_info.height,
        this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
        caption,
        this->ini.get<cfg::context::message>().c_str(),
        this->ini.get<cfg::context::display_link>().c_str(),
        link_label,
        this->rail_client_execute,
        this->glyphs,
        this->theme,
        D::copy_paste(*this)
    );
    D::set_mod(*this, ModuleName::link_confirm, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_wait_info_mod()
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
        D::copy_paste(*this),
        showform,
        flag
    );
    D::set_mod(*this, ModuleName::waitinfo, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_transition_mod()
{
    auto new_mod = new TransitionMod(
        TR(trkeys::wait_msg, language(this->ini)),
        this->graphics,
        this->client_info.screen_info.width,
        this->client_info.screen_info.height,
        this->rail_client_execute.adjust_rect(this->client_info.get_widget_rect()),
        this->rail_client_execute,
        this->glyphs,
        this->theme
    );
    D::set_mod(*this, ModuleName::transitory, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_login_mod()
{
    LOG(LOG_INFO, "ModuleManager::Creation of internal module 'Login'");
    char username[255]; // should use string
    username[0] = 0;
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
        this->theme,
        D::copy_paste(*this)
    );
    D::set_mod(*this, ModuleName::waitinfo, mod_pack_from_widget(new_mod), false);
}

void ModFactory::create_rdp_mod(
    SessionLogApi& session_log,
    PerformAutomaticReconnection perform_automatic_reconnection
)
{
    this->copy_paste_ptr.reset();
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
    D::set_mod(*this, ModuleName::RDP, mod_pack, true);
}

void ModFactory::create_vnc_mod(SessionLogApi& session_log)
{
    this->copy_paste_ptr.reset();
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
    D::set_mod(*this, ModuleName::VNC, mod_pack, true);
}

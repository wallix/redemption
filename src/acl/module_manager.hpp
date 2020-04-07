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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#pragma once

#include "acl/end_session_warning.hpp"

#include "acl/module_manager/mod_factory.hpp"
#include "acl/auth_api.hpp"
#include "acl/file_system_license_store.hpp"
#include "acl/module_manager/enums.hpp"
#include "configs/config.hpp"
#include "core/log_id.hpp"
#include "core/session_reactor.hpp"
#include "front/front.hpp"

#include "mod/internal/rail_module_host_mod.hpp"

#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/xup/xup.hpp"

#include "transport/socket_transport.hpp"

#include "core/session_reactor.hpp"
#include "acl/mod_wrapper.hpp"
#include "acl/acl_serializer.hpp"

#include "acl/connect_to_target_host.hpp"

class rdp_api;
class AuthApi;
class ReportMessageApi;

class ModuleManager
{
    ModFactory & mod_factory;

public:

    Inifile& ini;
    SessionReactor& session_reactor;
    TopFdContainer& fd_events_;
    GraphicFdContainer & graphic_fd_events_;
    TimerContainer& timer_events_;
    GraphicEventContainer& graphic_events_;
    SesmanInterface & sesman;
    CryptoContext & cctx;

    FileSystemLicenseStore file_system_license_store{ app_path(AppPath::License).to_string() };

private:
    FrontAPI & front;
    Keymap2 & keymap;
    ClientInfo & client_info;
    ClientExecute & rail_client_execute;
    Random & gen;
    TimeObj & timeobj;
    ReportMessageApi & report_message;
    AuthApi & authentifier;

    std::array<uint8_t, 28> server_auto_reconnect_packet {};

public:


    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        new_mod = 0x1,
    };

private:

    Font & glyphs;
    Theme & theme;

public:
    ModuleManager(ModFactory & mod_factory,
                  SessionReactor& session_reactor,
                  TopFdContainer& fd_events_,
                  GraphicFdContainer & graphic_fd_events_,
                  TimerContainer& timer_events_,
                  GraphicEventContainer& graphic_events_,
                  SesmanInterface & sesman,
                  FrontAPI & front, Keymap2 & keymap, ClientInfo & client_info, ClientExecute & rail_client_execute, Font & glyphs, Theme & theme, Inifile & ini, CryptoContext & cctx, Random & gen, TimeObj & timeobj, ReportMessageApi & report_message, AuthApi & authentifier)
        : mod_factory(mod_factory)
        , ini(ini)
        , session_reactor(session_reactor)
        , fd_events_(fd_events_)
        , graphic_fd_events_(graphic_fd_events_)
        , timer_events_(timer_events_)
        , graphic_events_(graphic_events_)
        , sesman(sesman)
        , cctx(cctx)
        , front(front)
        , keymap(keymap)
        , client_info(client_info)
        , rail_client_execute(rail_client_execute)
        , gen(gen)
        , timeobj(timeobj)
        , report_message(report_message)
        , authentifier(authentifier)
        , verbose(static_cast<Verbose>(ini.get<cfg::debug::auth>()))
        , glyphs(glyphs)
        , theme(theme)
    {
    }

    ~ModuleManager()
    {
    }

public:

    void new_mod(ModWrapper & mod_wrapper, ModuleIndex target_module)
    {
        switch (target_module)
        {
        case MODULE_INTERNAL_BOUNCER2:
        {
            auto mod_pack = mod_factory.create_mod_bouncer();
            mod_pack.enable_osd = true;
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_TEST:
        {
            auto mod_pack = mod_factory.create_mod_replay();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_WIDGETTEST:
        {
            auto mod_pack = mod_factory.create_widget_test_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_CARD:
        {
            auto mod_pack = mod_factory.create_test_card_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_WIDGET_SELECTOR:
        {
            auto mod_pack = mod_factory.create_selector_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_CLOSE:
        {
            auto mod_pack = mod_factory.create_close_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_CLOSE_BACK:
        {
            auto mod_pack = mod_factory.create_close_mod_back_to_selector();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_TARGET:
        {
            auto mod_pack = mod_factory.create_interactive_target_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
        {
            auto mod_pack = mod_factory.create_valid_message_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
        {
            auto mod_pack = mod_factory.create_display_message_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
        {
            auto mod_pack = mod_factory.create_dialog_challenge_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_WAIT_INFO:
        {
            auto mod_pack = mod_factory.create_wait_info_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_TRANSITION:
        {
            auto mod_pack = mod_factory.create_transition_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;
        case MODULE_INTERNAL_WIDGET_LOGIN: 
        {
            auto mod_pack = mod_factory.create_login_mod();
            mod_wrapper.set_mod(mod_pack);
        }
        break;

        case MODULE_XUP: {
            auto mod_pack = mod_factory.create_xup_mod();
            mod_pack.enable_osd = true;
            mod_pack.connected = true;
            mod_wrapper.set_mod(mod_pack);
            this->ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
            break;
        }

        case MODULE_RDP:
        {
            // %%% auto mod = mod_factory.create_mod_rdp(); %%%
            // %%% mod_wrapper.set_mod(mod, nullptr, nullptr);

                auto mod_pack = mod_factory.create_rdp_mod();
                mod_pack.enable_osd = true;
                mod_pack.connected = true;
                mod_wrapper.set_mod(mod_pack);
        }
        break;

        case MODULE_VNC:
        {
                auto mod_pack = mod_factory.create_vnc_mod();
                mod_pack.enable_osd = true;
                mod_pack.connected = true;

                mod_wrapper.set_mod(mod_pack);
        }
        break;

        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception %u", target_module);
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }
};

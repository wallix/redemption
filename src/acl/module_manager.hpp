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
#include "acl/module_manager/create_module_rdp.hpp"
#include "acl/module_manager/create_module_vnc.hpp"

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

    std::array<uint8_t, 28> server_auto_reconnect_packet {};

    ModuleIndex old_target_module = MODULE_UNKNOWN;

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
                  FrontAPI & front, Keymap2 & keymap, ClientInfo & client_info, ClientExecute & rail_client_execute, Font & glyphs, Theme & theme, Inifile & ini, CryptoContext & cctx, Random & gen, TimeObj & timeobj)
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
        , verbose(static_cast<Verbose>(ini.get<cfg::debug::auth>()))
        , glyphs(glyphs)
        , theme(theme)
    {
    }

    ~ModuleManager()
    {
    }

public:

    void new_mod(ModWrapper & mod_wrapper, ModuleIndex target_module, AuthApi & authentifier, ReportMessageApi & report_message)
    {
        LOG(LOG_INFO, "New_mod: target_module=%s (was %s)", 
                get_module_name(target_module), get_module_name(this->old_target_module));

        if (target_module != MODULE_INTERNAL_TRANSITION) {
            LOG(LOG_INFO, "----------> new_mod <--------");
            LOG(LOG_INFO, "target_module=%s(%d)",
                get_module_name(target_module), target_module);
        }

        this->rail_client_execute.enable_remote_program(this->client_info.remote_program);

        switch (target_module) {
        case MODULE_INTERNAL_CLOSE:
            log_proxy::set_user("");
            break;
        case MODULE_INTERNAL_WIDGET_LOGIN:
            log_proxy::set_user("");
            break;
        default:
            log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
            break;
        }

        mod_wrapper.connected = false;

        if (this->old_target_module != target_module) {
            this->front.must_be_stop_capture();

            switch (this->old_target_module){
            case MODULE_XUP: authentifier.delete_remote_mod(); break;
            case MODULE_RDP: authentifier.delete_remote_mod(); break;
            case MODULE_VNC: authentifier.delete_remote_mod(); break;
            default:;
            }

            switch (target_module){
            case MODULE_XUP: authentifier.new_remote_mod(); break;
            case MODULE_RDP: authentifier.new_remote_mod(); break;
            case MODULE_VNC: authentifier.new_remote_mod(); break;
            default:;
            }
        }
        this->old_target_module = target_module;

//        if ((target_module == MODULE_INTERNAL_WIDGET_SELECTOR)
//        && (acl.get_inactivity_timeout() != this->ini.get<cfg::globals::session_timeout>().count())) {
//            acl.update_inactivity_timeout();
//        }

        mod_wrapper.show_osd_flag = false;

        switch (target_module)
        {
        case MODULE_INTERNAL_BOUNCER2:
            mod_wrapper.set_mod(mod_factory.create_mod_bouncer());
        break;
        case MODULE_INTERNAL_TEST:
            mod_wrapper.set_mod(mod_factory.create_mod_replay());
        break;
        case MODULE_INTERNAL_WIDGETTEST:
            mod_wrapper.set_mod(mod_factory.create_widget_test_mod());
        break;
        case MODULE_INTERNAL_CARD:
            mod_wrapper.set_mod(mod_factory.create_test_card_mod());
        break;
        case MODULE_INTERNAL_WIDGET_SELECTOR:
            mod_wrapper.set_mod(mod_factory.create_selector_mod());
        break;
        case MODULE_INTERNAL_CLOSE:
            mod_wrapper.set_mod(mod_factory.create_close_mod());
        break;
        case MODULE_INTERNAL_CLOSE_BACK:
            mod_wrapper.set_mod(mod_factory.create_close_mod_back_to_selector());
        break;
        case MODULE_INTERNAL_TARGET:
            mod_wrapper.set_mod(mod_factory.create_interactive_target_mod());
        break;
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
            mod_wrapper.set_mod(mod_factory.create_valid_message_mod());
        break;
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
            mod_wrapper.set_mod(mod_factory.create_display_message_mod());
        break;
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
            mod_wrapper.set_mod(mod_factory.create_dialog_challenge_mod());
        break;
        case MODULE_INTERNAL_WAIT_INFO:
            mod_wrapper.set_mod(mod_factory.create_wait_info_mod());
        break;
        case MODULE_INTERNAL_TRANSITION:
            mod_wrapper.set_mod(mod_factory.create_transition_mod());
        break;
        case MODULE_INTERNAL_WIDGET_LOGIN: 
            mod_wrapper.set_mod(mod_factory.create_login_mod());
        break;

        case MODULE_XUP: {
            unique_fd client_sck = connect_to_target_host(
                    ini, session_reactor,
                    report_message, trkeys::authentification_x_fail);

            auto mod_pack = mod_factory.create_xup_mod(client_sck);
            mod_wrapper.set_mod(mod_pack);
            this->ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
            mod_wrapper.connected = true;
            mod_wrapper.show_osd_flag = true;
            break;
        }

        case MODULE_RDP:
        {
            // %%% auto mod = mod_factory.create_mod_rdp(); %%%
            // %%% mod_wrapper.set_mod(mod, nullptr, nullptr);

            try {
                auto mod_pack = create_mod_rdp(mod_wrapper,
                    authentifier, report_message, this->ini,
                    mod_wrapper.get_graphics(), this->front, this->client_info,
                    this->rail_client_execute, this->keymap.key_flags,
                    this->glyphs, this->theme,
                    this->session_reactor, this->fd_events_, this->graphic_fd_events_, this->timer_events_, this->graphic_events_,
                    this->sesman,
                    this->file_system_license_store,
                    this->gen, this->timeobj, this->cctx,
                    this->server_auto_reconnect_packet);

                mod_wrapper.set_mod(mod_pack);

                if (ini.get<cfg::globals::bogus_refresh_rect>() &&
                    ini.get<cfg::globals::allow_using_multiple_monitors>() &&
                    (client_info.cs_monitor.monitorCount > 1)) {
                    mod_wrapper.get_mod()->rdp_suppress_display_updates();
                    mod_wrapper.get_mod()->rdp_allow_display_updates(0, 0,
                        client_info.screen_info.width, client_info.screen_info.height);
                }
                mod_wrapper.get_mod()->rdp_input_invalidate(Rect(0, 0, client_info.screen_info.width, client_info.screen_info.height));
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'RDP' suceeded");
                ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
                mod_wrapper.show_osd_flag = true;
                mod_wrapper.connected = true;
            }
            catch (...) {
                report_message.log6(LogId::SESSION_CREATION_FAILED, this->session_reactor.get_current_time(), {});
                this->front.must_be_stop_capture();

                throw;
            }
                
        }
        break;

        case MODULE_VNC:
            try {
                auto mod_pack = create_mod_vnc(mod_wrapper, authentifier, report_message, this->ini,
                    mod_wrapper.get_graphics(), this->front, this->client_info,
                    this->rail_client_execute, this->keymap.key_flags,
                    this->glyphs, this->theme,
                    this->session_reactor,
                    this->graphic_fd_events_,
                    this->timer_events_,
                    this->graphic_events_,
                    this->sesman,
                    this->timeobj
                );

                mod_wrapper.set_mod(mod_pack);
                LOG(LOG_INFO, "ModuleManager::Creation of new mod 'VNC' suceeded");
                ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
                mod_wrapper.show_osd_flag = true;
                mod_wrapper.connected = true;
            }
            catch (...) {
                report_message.log6(LogId::SESSION_CREATION_FAILED, this->session_reactor.get_current_time(), {});
                throw;
            }
            break;

        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception");
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }
};

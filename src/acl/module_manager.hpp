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
#include "gdi/protected_graphics.hpp"

#include "mod/internal/rail_module_host_mod.hpp"

#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "mod/xup/xup.hpp"

#include "transport/socket_transport.hpp"

#include "utils/load_theme.hpp"
#include "utils/netutils.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/update_lock.hpp"
#include "utils/log_siem.hpp"
#include "utils/fileutils.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "acl/module_manager/enums.hpp"
#include "core/back_event_t.hpp"

#include "core/session_reactor.hpp"
#include "acl/mod_wrapper.hpp"
#include "acl/acl_serializer.hpp"

class rdp_api;
class AuthApi;
class ReportMessageApi;

class ModuleManager
{
    ModFactory & mod_factory;

private:
    bool connected{false};

public:
    bool is_connected() {
        return this->connected;
    }

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

    EndSessionWarning & end_session_warning;
    Font & glyphs;
    Theme & theme;

public:
    ModuleManager(EndSessionWarning & end_session_warning,
                  ModFactory & mod_factory,
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
        , end_session_warning(end_session_warning)
        , glyphs(glyphs)
        , theme(theme)
    {
    }

    ~ModuleManager()
    {
    }

private:
    void set_mod(ModWrapper & mod_wrapper, not_null_ptr<mod_api> mod, rdp_api* rdpapi, windowing_api* winapi)
    {
        while (this->keymap.nb_char_available()) {
            this->keymap.get_char();
        }
        while (this->keymap.nb_kevent_available()) {
            this->keymap.get_kevent();
        }

        mod_wrapper.clear_osd_message();

        mod_wrapper.set_mod(mod.get());

        mod_wrapper.rail_module_host_mod_ptr = nullptr;
        mod_wrapper.rdpapi = rdpapi;
        mod_wrapper.winapi = winapi;
    }

public:

    void new_mod_internal_close(ModWrapper & mod_wrapper, AuthApi & authentifier)
    {
        LOG(LOG_INFO, "New_mod: target_module=MODULE_INTERNAL_CLOSE");
        this->rail_client_execute.enable_remote_program(this->client_info.remote_program);
        log_proxy::set_user("");
        this->connected = false;
        if (this->old_target_module != MODULE_INTERNAL_CLOSE) {
            this->front.must_be_stop_capture();
            auto is_remote_mod = [](int mod_type){
                return
                    (mod_type == MODULE_XUP)
                 || (mod_type == MODULE_RDP)
                 || (mod_type == MODULE_VNC);
            };

            if (is_remote_mod(this->old_target_module)) {
                authentifier.delete_remote_mod();
            }
        }
        this->old_target_module = MODULE_INTERNAL_CLOSE;
        this->set_mod(mod_wrapper, mod_factory.create_close_mod(), nullptr, nullptr);
    }


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
        case MODULE_INTERNAL_WIDGET_LOGIN:
            log_proxy::set_user("");
            break;
        default:
            log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
            break;
        }

        this->connected = false;

        if (this->old_target_module != target_module) {
            this->front.must_be_stop_capture();

            auto is_remote_mod = [](int mod_type){
                return
                    (mod_type == MODULE_XUP)
                 || (mod_type == MODULE_RDP)
                 || (mod_type == MODULE_VNC);
            };

            if (is_remote_mod(this->old_target_module)) {
                authentifier.delete_remote_mod();
            }

            if (is_remote_mod(target_module)) {
                authentifier.new_remote_mod();
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
            this->set_mod(mod_wrapper, mod_factory.create_mod_bouncer(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_TEST:
            this->set_mod(mod_wrapper, mod_factory.create_mod_replay(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_WIDGETTEST:
            this->set_mod(mod_wrapper, mod_factory.create_widget_test_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_CARD:
            this->set_mod(mod_wrapper, mod_factory.create_test_card_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_WIDGET_SELECTOR:
            this->set_mod(mod_wrapper, mod_factory.create_selector_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_CLOSE:
            this->set_mod(mod_wrapper, mod_factory.create_close_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_CLOSE_BACK:
            this->set_mod(mod_wrapper, mod_factory.create_close_mod_back_to_selector(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_TARGET:
            this->set_mod(mod_wrapper, mod_factory.create_interactive_target_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:
            this->set_mod(mod_wrapper, mod_factory.create_valid_message_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:
            this->set_mod(mod_wrapper, mod_factory.create_display_message_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_DIALOG_CHALLENGE:
            this->set_mod(mod_wrapper, mod_factory.create_dialog_challenge_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_WAIT_INFO:
            this->set_mod(mod_wrapper, mod_factory.create_wait_info_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_TRANSITION:
            this->set_mod(mod_wrapper, mod_factory.create_transition_mod(), nullptr, nullptr);
        break;
        case MODULE_INTERNAL_WIDGET_LOGIN: 
            this->set_mod(mod_wrapper, mod_factory.create_login_mod(), nullptr, nullptr);
        break;

        case MODULE_XUP: {
            unique_fd client_sck = this->connect_to_target_host(
                    report_message, trkeys::authentification_x_fail);

            this->set_mod(mod_wrapper, mod_factory.create_xup_mod(client_sck), nullptr, nullptr);
            this->ini.get_mutable_ref<cfg::context::auth_error_message>().clear();
            this->connected = true;
            mod_wrapper.show_osd_flag = true;
            break;
        }

        case MODULE_RDP:
        {
            this->create_mod_rdp(mod_wrapper,
                authentifier, report_message, this->ini,
                mod_wrapper.get_graphics(), this->front, this->client_info,
                this->rail_client_execute, this->keymap.key_flags,
                this->server_auto_reconnect_packet);
                
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
            this->connected = true;
        }
        break;

        case MODULE_VNC:
            this->create_mod_vnc(mod_wrapper,
                authentifier, report_message, this->ini,
                mod_wrapper.get_graphics(), this->front, this->client_info,
                this->rail_client_execute, this->keymap.key_flags);
                mod_wrapper.show_osd_flag = true;
            break;

        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception");
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }

public:
    [[nodiscard]] rdp_api* get_rdp_api(ModWrapper & mod_wrapper) const {
        return mod_wrapper.rdpapi;
    }

    ModuleIndex next_module()
    {
        auto & module_cstr = this->ini.get<cfg::context::module>();
        auto module_id = get_module_id(module_cstr);
        LOG(LOG_INFO, "----------> ACL next_module : %s %u <--------", module_cstr, unsigned(module_id));

        if (this->connected && ((module_id == MODULE_RDP)||(module_id == MODULE_VNC))) {
            if (this->ini.get<cfg::context::auth_error_message>().empty()) {
                this->ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(this->ini)));
            }
            return MODULE_INTERNAL_CLOSE;
        }
        if (module_id == MODULE_INTERNAL)
        {
            auto module_id = get_internal_module_id_from_target(this->ini.get<cfg::context::target_host>());
            LOG(LOG_INFO, "===========> %s (from target)", get_module_name(module_id));
            return module_id;
        }
        if (module_id == MODULE_UNKNOWN)
        {
            LOG(LOG_INFO, "===========> UNKNOWN MODULE (closing)");
            return MODULE_INTERNAL_CLOSE;
        }
        return module_id;
    }

private:
    unique_fd connect_to_target_host(ReportMessageApi& report_message, trkeys::TrKey const& authentification_fail)
    {
        auto throw_error = [this, &report_message](char const* error_message, int id) {
            LOG_PROXY_SIEM("TARGET_CONNECTION_FAILED",
                R"(target="%s" host="%s" port="%u" reason="%s")",
                this->ini.get<cfg::globals::target_user>(),
                this->ini.get<cfg::context::target_host>(),
                this->ini.get<cfg::context::target_port>(),
                error_message);

            report_message.log6(LogId::CONNECTION_FAILED, this->session_reactor.get_current_time(), {});

            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::target_fail, language(this->ini)));

            LOG(LOG_ERR, "%s", (id == 1)
                ? "Failed to connect to remote TCP host (1)"
                : "Failed to connect to remote TCP host (2)");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        };

        LOG_PROXY_SIEM("TARGET_CONNECTION",
            R"(target="%s" host="%s" port="%u")",
            this->ini.get<cfg::globals::target_user>(),
            this->ini.get<cfg::context::target_host>(),
            this->ini.get<cfg::context::target_port>());

        const char * ip = this->ini.get<cfg::context::target_host>().c_str();
        char ip_addr[256] {};
        in_addr s4_sin_addr;
        if (auto error_message = resolve_ipv4_address(ip, s4_sin_addr)) {
            // TODO: actually this is DNS Failure or invalid address
            throw_error(error_message, 1);
        }

        snprintf(ip_addr, sizeof(ip_addr), "%s", inet_ntoa(s4_sin_addr));

        char const* error_message = nullptr;
        unique_fd client_sck = ip_connect(ip, this->ini.get<cfg::context::target_port>(), &error_message);

        if (!client_sck.is_open()) {
            throw_error(error_message, 2);
        }

        this->ini.set<cfg::context::auth_error_message>(TR(authentification_fail, language(this->ini)));
        this->ini.set<cfg::context::ip_target>(ip_addr);

        return client_sck;
    }


    void create_mod_rdp(ModWrapper & mod_wrapper,
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo client_info,
        ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags,
        std::array<uint8_t, 28>& server_auto_reconnect_packet);

    void create_mod_vnc(ModWrapper & mod_wrapper,
        AuthApi& authentifier, ReportMessageApi& report_message,
        Inifile& ini, gdi::GraphicApi & drawable, FrontAPI& front, ClientInfo const& client_info,
        ClientExecute& rail_client_execute, Keymap2::KeyFlags key_flags);
};

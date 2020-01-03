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
#include "acl/time_before_closing.hpp"
#include "acl/acl_serializer.hpp"

class rdp_api;
class AuthApi;
class ReportMessageApi;

class ModuleManager
{
    ModFactory & mod_factory;

public:
    bool last_module{false};
    bool connected{false};

    bool is_connected() {
        return this->connected;
    }

    Inifile& ini;
    SessionReactor& session_reactor;
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

    int validator_fd = -1;

private:

    EndSessionWarning & end_session_warning;
    Font & glyphs;
    Theme & theme;

public:
    ModuleManager(EndSessionWarning & end_session_warning, ModFactory & mod_factory, SessionReactor& session_reactor, FrontAPI & front, Keymap2 & keymap, ClientInfo & client_info, ClientExecute & rail_client_execute, Font & glyphs, Theme & theme, Inifile & ini, CryptoContext & cctx, Random & gen, TimeObj & timeobj)
        : mod_factory(mod_factory)
        , ini(ini)
        , session_reactor(session_reactor)
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
    void set_mod(ModWrapper & mod_wrapper, not_null_ptr<mod_api> mod, rdp_api* rdpapi = nullptr, windowing_api* winapi = nullptr)
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

    bool check_acl(Acl & acl,
        AuthApi & authentifier, ReportMessageApi & report_message, ModWrapper & mod_wrapper,
        time_t now, BackEvent_t & signal, BackEvent_t & front_signal, bool & has_user_activity)
    {
        // LOG(LOG_DEBUG, "================> ACL check: now=%u, signal=%u, front_signal=%u",
        //  static_cast<unsigned>(now), static_cast<unsigned>(signal), static_cast<unsigned>(front_signal));
        if (signal == BACK_EVENT_STOP) {
            // here, this->last_module should be false only when we are in login box
            return false;
        }

        if (this->last_module) {
            // at a close box (this->last_module is true),
            // we are only waiting for a stop signal
            // and Authentifier should not exist anymore.
            return true;
        }

        const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            const char * message = TR(trkeys::session_out_time, language(this->ini));
            this->invoke_close_box(mod_wrapper, ini.get<cfg::globals::enable_close_box>(), message, signal, authentifier, report_message);

            return true;
        }

        // Close by rejeted message received
        if (!this->ini.get<cfg::context::rejected>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
            LOG(LOG_INFO, "Close by Rejected message received : %s",
                this->ini.get<cfg::context::rejected>());
            this->ini.set_acl<cfg::context::rejected>("");
            this->invoke_close_box(mod_wrapper, ini.get<cfg::globals::enable_close_box>(), nullptr, signal, authentifier, report_message);
            return true;
        }

        // Keep Alive
        if (acl.acl_serial.keepalive.check(now, this->ini)) {
            this->invoke_close_box(mod_wrapper, ini.get<cfg::globals::enable_close_box>(),
                TR(trkeys::miss_keepalive, language(this->ini)),
                signal, authentifier, report_message
            );
            return true;
        }

        // Inactivity management

        if (acl.acl_serial.inactivity.check_user_activity(now, has_user_activity)) {
            this->invoke_close_box(mod_wrapper, ini.get<cfg::globals::enable_close_box>(),
                TR(trkeys::close_inactivity, language(this->ini)),
                signal, authentifier, report_message
            );
            return true;
        }

        // Manage module (refresh or next)
        if (this->ini.changed_field_size()) {
            if (this->connected) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                acl.acl_serial.send_acl_data();
            }
            else if (signal == BACK_EVENT_REFRESH || signal == BACK_EVENT_NEXT) {
                acl.acl_serial.remote_answer = false;
                acl.acl_serial.send_acl_data();
                if (signal == BACK_EVENT_NEXT) {
                    mod_wrapper.remove_mod();
                    this->new_mod(mod_wrapper, MODULE_INTERNAL_TRANSITION, authentifier, report_message);
                }
            }
            if (signal == BACK_EVENT_REFRESH) {
                signal = BACK_EVENT_NONE;
            }
        }
        else if (acl.acl_serial.remote_answer
        || (signal == BACK_EVENT_RETRY_CURRENT)
        || (front_signal == BACK_EVENT_NEXT)) {
            acl.acl_serial.remote_answer = false;
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
            }
            else if ((signal == BACK_EVENT_NEXT)
                    || (signal == BACK_EVENT_RETRY_CURRENT)
                    || (front_signal == BACK_EVENT_NEXT)) {
                if ((signal == BACK_EVENT_NEXT)
                    || (front_signal == BACK_EVENT_NEXT)) {
                    LOG(LOG_INFO, "===========> MODULE_NEXT");
                }
                else {
                    assert(signal == BACK_EVENT_RETRY_CURRENT);

                    LOG(LOG_INFO, "===========> MODULE_RETRY_CURRENT");
                }

                ModuleIndex next_state
                    = (signal == BACK_EVENT_NEXT || front_signal == BACK_EVENT_NEXT)
                    ? this->next_module() : MODULE_RDP;

                front_signal = BACK_EVENT_NONE;

                if (next_state == MODULE_TRANSITORY) {
                    acl.acl_serial.remote_answer = false;

                    return true;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    this->invoke_close_box(mod_wrapper, ini.get<cfg::globals::enable_close_box>(), nullptr, signal, authentifier, report_message);
                    return true;
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    acl.acl_serial.keepalive.stop();
                }
                if (mod_wrapper.get_mod()) {
                    mod_wrapper.get_mod()->disconnect();
                }
                mod_wrapper.remove_mod();
                try {
                    this->new_mod(mod_wrapper, next_state, authentifier, report_message);
                }
                catch (Error const& e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        // TODO : see STRMODULE_TRANSITORY
                        this->ini.set_acl<cfg::context::module>("transitory");

                        signal = BACK_EVENT_NEXT;

                        acl.acl_serial.remote_answer = false;

                        authentifier.disconnect_target();

                        acl.acl_serial.report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return true;
                    }

                    if ((e.id == ERR_RDP_SERVER_REDIR) 
                    && this->ini.get<cfg::mod_rdp::server_redirection_support>()) {
                        acl.acl_serial.server_redirection_target();
                        acl.acl_serial.remote_answer = true;
                        signal = BACK_EVENT_NEXT;
                        return true;
                    }

                    throw;
                }
                if (!acl.acl_serial.keepalive.is_started() && this->connected) {
                    acl.acl_serial.keepalive.start(now);
                }
            }
            else
            {
                if (!this->ini.get<cfg::context::disconnect_reason>().empty()) {
                    acl.acl_serial.manager_disconnect_reason = this->ini.get<cfg::context::disconnect_reason>();
                    this->ini.get_mutable_ref<cfg::context::disconnect_reason>().clear();

                    this->ini.set_acl<cfg::context::disconnect_reason_ack>(true);
                }
                else if (!this->ini.get<cfg::context::auth_command>().empty()) {
                    if (!::strcasecmp(this->ini.get<cfg::context::auth_command>().c_str(),
                                        "rail_exec")) {
                        const uint16_t flags                = this->ini.get<cfg::context::auth_command_rail_exec_flags>();
                        const char*    original_exe_or_file = this->ini.get<cfg::context::auth_command_rail_exec_original_exe_or_file>().c_str();
                        const char*    exe_or_file          = this->ini.get<cfg::context::auth_command_rail_exec_exe_or_file>().c_str();
                        const char*    working_dir          = this->ini.get<cfg::context::auth_command_rail_exec_working_dir>().c_str();
                        const char*    arguments            = this->ini.get<cfg::context::auth_command_rail_exec_arguments>().c_str();
                        const uint16_t exec_result          = this->ini.get<cfg::context::auth_command_rail_exec_exec_result>();
                        const char*    account              = this->ini.get<cfg::context::auth_command_rail_exec_account>().c_str();
                        const char*    password             = this->ini.get<cfg::context::auth_command_rail_exec_password>().c_str();

                        rdp_api* rdpapi = this->get_rdp_api(mod_wrapper);

                        if (!exec_result) {
                            //LOG(LOG_INFO,
                            //    "RailExec: "
                            //        "original_exe_or_file=\"%s\" "
                            //        "exe_or_file=\"%s\" "
                            //        "working_dir=\"%s\" "
                            //        "arguments=\"%s\" "
                            //        "flags=%u",
                            //    original_exe_or_file, exe_or_file, working_dir, arguments, flags);

                            if (rdpapi) {
                                rdpapi->auth_rail_exec(flags, original_exe_or_file, exe_or_file, working_dir, arguments, account, password);
                            }
                        }
                        else {
                            //LOG(LOG_INFO,
                            //    "RailExec: "
                            //        "exec_result=%u "
                            //        "original_exe_or_file=\"%s\" "
                            //        "flags=%u",
                            //    exec_result, original_exe_or_file, flags);

                            if (rdpapi) {
                                rdpapi->auth_rail_exec_cancel(flags, original_exe_or_file, exec_result);
                            }
                        }
                    }

                    this->ini.get_mutable_ref<cfg::context::auth_command>().clear();
                }
            }
        }

        // LOG(LOG_INFO, "connect=%s check=%s", this->connected?"Y":"N", check()?"Y":"N");

        if (this->connected) {
            // AuthCHANNEL CHECK
            // if an answer has been received, send it to
            // rdp serveur via mod (should be rdp module)
            // TODO Check if this->mod is RDP MODULE
            if (this->ini.get<cfg::mod_rdp::auth_channel>()[0]) {
                // Get sesman answer to AUTHCHANNEL_TARGET
                if (!this->ini.get<cfg::context::auth_channel_answer>().empty()) {
                    // If set, transmit to auth_channel channel
                    mod_wrapper.get_mod()->send_auth_channel_data(this->ini.get<cfg::context::auth_channel_answer>().c_str());
                    // Erase the context variable
                    this->ini.get_mutable_ref<cfg::context::auth_channel_answer>().clear();
                }
            }

            // CheckoutCHANNEL CHECK
            // if an answer has been received, send it to
            // rdp serveur via mod (should be rdp module)
            // TODO Check if this->mod is RDP MODULE
            if (this->ini.get<cfg::mod_rdp::checkout_channel>()[0]) {
                // Get sesman answer to AUTHCHANNEL_TARGET
                if (!this->ini.get<cfg::context::pm_response>().empty()) {
                    // If set, transmit to auth_channel channel
                    mod_wrapper.get_mod()->send_checkout_channel_data(this->ini.get<cfg::context::pm_response>().c_str());
                    // Erase the context variable
                    this->ini.get_mutable_ref<cfg::context::pm_response>().clear();
                }
            }

            if (!this->ini.get<cfg::context::rd_shadow_type>().empty()) {
                mod_wrapper.get_mod()->create_shadow_session(this->ini.get<cfg::context::rd_shadow_userdata>().c_str(),
                    this->ini.get<cfg::context::rd_shadow_type>().c_str());

                this->ini.get_mutable_ref<cfg::context::rd_shadow_type>().clear();
            }
        }

        return true;
    }


    void new_mod(ModWrapper & mod_wrapper, ModuleIndex target_module, AuthApi & authentifier, ReportMessageApi & report_message)
    {
        if (target_module != MODULE_INTERNAL_TRANSITION) {
            LOG(LOG_INFO, "----------> ACL new_mod <--------");
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

        if ((target_module == MODULE_INTERNAL_WIDGET_SELECTOR)
        && (report_message.get_inactivity_timeout() != this->ini.get<cfg::globals::session_timeout>().count())) {
            report_message.update_inactivity_timeout();
        }


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
            break;
        }

        case MODULE_RDP:
            this->create_mod_rdp(mod_wrapper,
                authentifier, report_message, this->ini,
                mod_wrapper.get_graphics(), this->front, this->client_info,
                this->rail_client_execute, this->keymap.key_flags,
                this->server_auto_reconnect_packet);
            break;

        case MODULE_VNC:
            this->create_mod_vnc(mod_wrapper,
                authentifier, report_message, this->ini,
                mod_wrapper.get_graphics(), this->front, this->client_info,
                this->rail_client_execute, this->keymap.key_flags);
            break;

        default:
            LOG(LOG_INFO, "ModuleManager::Unknown backend exception");
            throw Error(ERR_SESSION_UNKNOWN_BACKEND);
        }
    }

    [[nodiscard]] rdp_api* get_rdp_api(ModWrapper & mod_wrapper) const {
        return mod_wrapper.rdpapi;
    }

    void invoke_close_box(ModWrapper & mod_wrapper,
        bool enable_close_box,
        const char * auth_error_message, BackEvent_t & signal,
        AuthApi & authentifier, ReportMessageApi & report_message)
    {
        LOG(LOG_INFO, "----------> ACL invoke_close_box <--------");
        this->last_module = true;
        if (auth_error_message) {
            this->ini.set<cfg::context::auth_error_message>(auth_error_message);
        }
        if (mod_wrapper.has_mod()) {
            try {
                mod_wrapper.get_mod()->disconnect();
            }
            catch (Error const& e) {
                LOG(LOG_INFO, "MMIni::invoke_close_box exception = %u!", e.id);
            }
        }

        mod_wrapper.remove_mod();
        if (enable_close_box) {
            this->new_mod(mod_wrapper, MODULE_INTERNAL_CLOSE, authentifier, report_message);
            signal = BACK_EVENT_NONE;
        }
        else {
            signal = BACK_EVENT_STOP;
        }
    }

    ModuleIndex next_module()
    {
        auto & module_cstr = this->ini.get<cfg::context::module>();
        auto module_id = get_module_id(module_cstr);
        LOG(LOG_INFO, "----------> ACL next_module : %s %u <--------", module_cstr, unsigned(module_id));

        if (this->connected && ((module_id == MODULE_RDP)||(module_id == MODULE_VNC))) {
            LOG(LOG_INFO, "===========> Connection close asked by admin while connected");
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

    void check_module() 
    {
        if (this->ini.get<cfg::context::forcemodule>() && !this->is_connected()) {
            this->session_reactor.set_next_event(BACK_EVENT_NEXT);
            this->ini.set<cfg::context::forcemodule>(false);
            // Do not send back the value to sesman.
        }
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

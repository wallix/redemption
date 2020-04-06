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
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou, Meng Tan
*/

#include "acl/end_session_warning.hpp"

#include "acl/module_manager/mod_factory.hpp"
#include "core/session.hpp"

#include "acl/authentifier.hpp"
#include "acl/module_manager.hpp"
#include "acl/sesman.hpp"
#include "capture/capture.hpp"
#include "configs/config.hpp"
#include "core/session_reactor.hpp"
#include "core/set_server_redirection_target.hpp"
#include "front/front.hpp"
#include "mod/mod_api.hpp"
#include "transport/socket_transport.hpp"
#include "transport/ws_transport.hpp"
#include "utils/file.hpp"
#include "utils/genfstat.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "utils/log_siem.hpp"
#include "utils/load_theme.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <unistd.h>

#include "acl/mod_wrapper.hpp"

namespace
{

class Session
{

    enum class SessionState : int {
        SESSION_STATE_INCOMING,
        SESSION_STATE_RUNNING,
        SESSION_STATE_BACKEND_CLEANUP,
        SESSION_STATE_CLOSE_BOX
    };

    struct Select
    {
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;
        timeval timeout;
        bool want_write = false;

        Select(timeval timeout)
        : timeout{timeout}
        {
//            LOG(LOG_INFO, "set initial timeout at %u s %u us", this->timeout.tv_sec, this->timeout.tv_usec);
            io_fd_zero(this->rfds);
            io_fd_zero(this->wfds);
        }

        int select(timeval now)
        {
            timeval timeoutastv = {0,0};
            const timeval & ultimatum = this->timeout;
            const timeval & starttime = now;
            if (ultimatum > starttime) {
                timeoutastv = to_timeval(std::chrono::seconds(ultimatum.tv_sec) + std::chrono::microseconds(ultimatum.tv_usec)
                    - std::chrono::seconds(starttime.tv_sec) - std::chrono::microseconds(starttime.tv_usec));
            }

//            LOG(LOG_INFO, "Waiting on select: now=%d.%d timeout=%d.%d timeout in %u s %u us", 
//                now.tv_sec, now.tv_usec, this->timeout.tv_sec, this->timeout.tv_usec, 
//                timeoutastv.tv_sec, timeoutastv.tv_usec);
            return ::select(
                this->max + 1, &this->rfds,
                this->want_write ? &this->wfds : nullptr,
                nullptr, &timeoutastv);
        }

        void set_timeout(timeval next_timeout)
        {
//            LOG(LOG_INFO, "set timeout at %u s %u us", next_timeout.tv_sec, next_timeout.tv_usec);
            this->timeout = next_timeout;
        }

        timeval get_timeout()
        {
            return this->timeout;
        }

        bool is_set_for_writing(int fd){
            bool res = io_fd_isset(fd, this->wfds);
//            if (res){
//                LOG(LOG_INFO, "is set for writing fd=%u", fd);
//            }
            return res;
        }

        bool is_set_for_reading(int fd){
            bool res = io_fd_isset(fd, this->rfds);
//            if (res){
//                LOG(LOG_INFO, "is set for reading fd=%u", fd);
//            }
            return res;
        }

        void set_read_sck(int sck)
        {
//            LOG(LOG_INFO, "--> set for reading fd=%u", sck);
            this->max = prepare_fds(sck, this->max, this->rfds);
        }

        void set_write_sck(int sck)
        {
//            LOG(LOG_INFO, "--> set for writing fd=%u", sck);
            if (!this->want_write) {
                this->want_write = true;
                io_fd_zero(this->wfds);
            }
            this->max = prepare_fds(sck, this->max, this->wfds);
        }
    };

    Inifile & ini;

    time_t      perf_last_info_collect_time = 0;
    const pid_t perf_pid = getpid();
    File        perf_file = nullptr;

    static const time_t select_timeout_tv_sec = 3;

private:
    void start_acl_running(std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, Authentifier & authentifier, Fstat & fstat)
    {
        // authentifier never opened or closed by me (close box)
        // now is authentifier start time
        acl = std::make_unique<Acl>(
            ini, Session::acl_connect(ini.get<cfg::globals::authfile>(), (strcmp(ini.get<cfg::globals::host>().c_str(), "127.0.0.1") == 0)), now.tv_sec, cctx, rnd, fstat
        );
        const auto sck = acl->auth_trans.sck;
        fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
        authentifier.set_acl_serial(&acl->acl_serial);
    }

private:
    int end_session_exception(Error const& e, Authentifier & authentifier, Inifile & ini) {
        if ((e.id == ERR_SESSION_PROBE_LAUNCH)
        ||  (e.id == ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE)
        ||  (e.id == ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS)
        ||  (e.id == ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED)
        ||  (e.id == ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG)
        ||  (e.id == ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG)) {
            if (ini.get<cfg::mod_rdp::session_probe_on_launch_failure>() ==
                    SessionProbeOnLaunchFailure::retry_without_session_probe)
            {
                LOG(LOG_INFO, "====> Retry without session probe");
                ini.get_mutable_ref<cfg::mod_rdp::enable_session_probe>() = false;
                return 2;
            }
            if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
            }
            else {
                ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));
            }
            return 1;
        }
        else if (e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) {
            LOG(LOG_INFO, "====> Retry SESSION_PROBE_DISCONNECTION_RECONNECTION");
            return true;
        }
        else if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
            LOG(LOG_INFO, "====> Retry AUTOMATIC_RECONNECTION_REQUIRED");
            ini.set<cfg::context::perform_automatic_reconnection>(true);
            return 2;
        }
        else if (e.id == ERR_RAIL_NOT_ENABLED) {
            LOG(LOG_INFO, "====> Retry witout Native remoteapp capability");
            ini.get_mutable_ref<cfg::mod_rdp::use_native_remoteapp_capability>() = false;
            return 2;
        }
        else if (e.id == ERR_RDP_SERVER_REDIR){
            if (ini.get<cfg::mod_rdp::server_redirection_support>()) {
                LOG(LOG_INFO, "====> Retry Server redirection");
                set_server_redirection_target(ini, authentifier);
                return 2;
            }
            else {
                LOG(LOG_ERR, "Start Session Failed: forbidden redirection = %s", e.errmsg());
                if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                    this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                }
                return 1;
            }
        }
        else if (e.id == ERR_SESSION_CLOSE_ENDDATE_REACHED){
            LOG(LOG_INFO, "check_acl: close by enddate");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::session_out_time, language(this->ini)));
            return 1;
        }
        else if (e.id == ERR_SESSION_CLOSE_REJECTED_BY_ACL_MESSAGE){
            // Close by rejeted message received
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
            LOG(LOG_INFO, "check_acl: close by Rejected message received : %s", this->ini.get<cfg::context::rejected>());
            this->ini.set_acl<cfg::context::rejected>("");
            return 1;
        }
        else if (e.id == ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED) {
            LOG(LOG_INFO, "check_acl: close by Missed keepalive");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::miss_keepalive, language(this->ini)));
            return 1;
        }
        else if (e.id == ERR_SESSION_CLOSE_USER_INACTIVITY) {
            LOG(LOG_INFO, "check_acl: close by user Inactivity");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::close_inactivity, language(this->ini)));
            return 1;
        }
        else if (e.id == ERR_SESSION_CLOSE_MODULE_NEXT) {
            LOG(LOG_INFO, "check_acl: next module is close box");
            return 1;
        }
        LOG(LOG_ERR, "Start Session Failed = %s", e.errmsg());
        if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
        }
        return 1;
    }

private:
    void rt_display(Inifile & ini, ModWrapper & mod_wrapper, Front & front)
    {
        if (ini.check_from_acl()) {
            auto const rt_status = front.set_rt_display(ini.get<cfg::video::rt_display>());

            if (ini.get<cfg::client::enable_osd_4_eyes>()) {
                Translator tr(language(ini));
                if (rt_status != Capture::RTDisplayResult::Unchanged) {
                    std::string message = tr((rt_status==Capture::RTDisplayResult::Enabled)
                        ?trkeys::enable_rt_display
                        :trkeys::disable_rt_display
                            ).to_string();

                    bool is_disable_by_input = true;
                    if (message != mod_wrapper.get_message()) {
                        mod_wrapper.clear_osd_message();
                    }
                    if (!message.empty()) {
                        mod_wrapper.set_message(std::move(message), is_disable_by_input);
                        mod_wrapper.draw_osd_message();
                    }
                }
            }

            if (this->ini.get<cfg::context::forcemodule>() && !mod_wrapper.is_connected()) {
                this->ini.set<cfg::context::forcemodule>(false);
                // Do not send back the value to sesman.
            }
        }
    }


    bool front_up_and_running(std::unique_ptr<Acl> & acl, timeval & now,
                              const time_t start_time, Inifile& ini,
                              ModuleManager & mm, ModWrapper & mod_wrapper,
                              EndSessionWarning & end_session_warning,
                              Front & front,
                              Authentifier & authentifier,
                              ClientExecute & rail_client_execute)
    {
        if (ini.check_from_acl()) {
            if (ini.get<cfg::client::force_bitmap_cache_v2_with_am>() &&
                ini.get<cfg::context::is_wabam>()) {
                front.force_using_cache_bitmap_r2();
            }
        }

        if (ini.get<cfg::globals::enable_osd>()) {
            const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
            if (enddate && mod_wrapper.is_up_and_running()) {
                LOG(LOG_INFO, "--------------------- End Session OSD Warning");
                std::string mes = end_session_warning.update_osd_state(
                    language(ini), start_time, static_cast<time_t>(enddate), now.tv_sec);
                if (!mes.empty()) {
                    bool is_disable_by_input = true;
                    if (mes != mod_wrapper.get_message()) {
                        mod_wrapper.clear_osd_message();
                    }
                    mod_wrapper.set_message(std::move(mes), is_disable_by_input);
                    mod_wrapper.draw_osd_message();
                }
            }
        }

        if (!acl->keepalive.is_started() && mod_wrapper.is_connected()) {
            acl->keepalive.start(now.tv_sec);
        }

        // There are modified fields to send to sesman
        if (ini.changed_field_size()) {
            LOG(LOG_INFO, "check_acl: data to send to sesman");
            if (mod_wrapper.is_connected()) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                acl->acl_serial.send_acl_data();
                return true;
            }
            BackEvent_t signal = mod_wrapper.get_mod()->get_mod_signal();
            switch (signal){
            case BACK_EVENT_STOP:
            break;
            case BACK_EVENT_NONE:
            break;
            case BACK_EVENT_NEXT:
                acl->acl_serial.remote_answer = false;
                acl->acl_serial.send_acl_data();
                mod_wrapper.remove_mod();
                LOG(LOG_INFO, "New_mod: MODULE_INTERNAL_TRANSITION (was %s)", get_module_name(mod_wrapper.old_target_module));
                rail_client_execute.enable_remote_program(front.client_info.remote_program);
                log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
                mod_wrapper.connected = false;
                if (mod_wrapper.old_target_module != MODULE_INTERNAL_TRANSITION) {
                    front.must_be_stop_capture();
                    switch (mod_wrapper.old_target_module){
                    case MODULE_XUP: authentifier.delete_remote_mod(); break;
                    case MODULE_RDP: authentifier.delete_remote_mod(); break;
                    case MODULE_VNC: authentifier.delete_remote_mod(); break;
                    default:;
                    }
                }
                mod_wrapper.old_target_module = MODULE_INTERNAL_TRANSITION;
                mm.new_mod(mod_wrapper, MODULE_INTERNAL_TRANSITION);
            break;
            case BACK_EVENT_REFRESH:
                acl->acl_serial.remote_answer = false;
                acl->acl_serial.send_acl_data();
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
                return true;
            break;
            }
            return true;
        }

        if (acl->acl_serial.remote_answer) {
            BackEvent_t signal = mod_wrapper.get_mod()->get_mod_signal();
            LOG(LOG_INFO, "check_acl remote_answer signal=%s", signal_name(signal));
            acl->acl_serial.remote_answer = false;

            switch (signal){
            default:
            case BACK_EVENT_NONE:
                LOG(LOG_INFO, "Back Event NONE");
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
            return true;
            case BACK_EVENT_NEXT:
            {
                LOG(LOG_INFO, "Remote Answer NEXT MODULE");
                auto & module_cstr = ini.get<cfg::context::module>();
                auto next_state = get_module_id(module_cstr);
                LOG(LOG_INFO, "----------> ACL next_module : %s %u <--------", module_cstr, unsigned(next_state));

                switch (next_state){
                case MODULE_TRANSITORY: // NO MODULE CHANGE INFO YET, ASK MORE FROM ACL
                {
                    LOG(LOG_INFO, "check_acl TRANSITORY signal=%s", signal_name(signal));
                    acl->acl_serial.remote_answer = false;
                    mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NEXT);
                    return true;
                }
                break;
                case MODULE_RDP:
                {
                    LOG(LOG_INFO, "New_mod: MODULE_RDP (was %s)", get_module_name(mod_wrapper.old_target_module));
                    if (mod_wrapper.is_connected()) {
                        if (ini.get<cfg::context::auth_error_message>().empty()) {
                            ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(ini)));
                        }
                        throw Error(ERR_SESSION_CLOSE_MODULE_NEXT);
                    }
                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                    log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
                    mod_wrapper.connected = false;

                    if (mod_wrapper.old_target_module != MODULE_RDP) {
                        front.must_be_stop_capture();
                        switch (mod_wrapper.old_target_module){
                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                        default:;
                        }
                        authentifier.new_remote_mod();
                    }
                    mod_wrapper.old_target_module = MODULE_RDP;
                    mm.new_mod(mod_wrapper, MODULE_RDP);
                }
                break;
                case MODULE_VNC:
                {
                    LOG(LOG_INFO, "New_mod: MODULE_VNC (was %s)", get_module_name(mod_wrapper.old_target_module));
                    if (mod_wrapper.is_connected()) {
                        if (ini.get<cfg::context::auth_error_message>().empty()) {
                            ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(ini)));
                        }
                        throw Error(ERR_SESSION_CLOSE_MODULE_NEXT);
                    }

                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                    log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());

                    mod_wrapper.connected = false;
                    if (mod_wrapper.old_target_module != MODULE_VNC) {
                        front.must_be_stop_capture();
                        switch (mod_wrapper.old_target_module){
                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                        default:;
                        }
                        authentifier.new_remote_mod();
                    }
                    mod_wrapper.old_target_module = MODULE_VNC;
                    mm.new_mod(mod_wrapper, MODULE_VNC);
                }
                break;
                case MODULE_INTERNAL:
                {
                    next_state = get_internal_module_id_from_target(ini.get<cfg::context::target_host>());
                    LOG(LOG_INFO, "New_mod (internal from target): %s (was %s)", 
                            get_module_name(next_state), get_module_name(mod_wrapper.old_target_module));
                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                    log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
                    mod_wrapper.connected = false;
                    if (mod_wrapper.old_target_module != MODULE_INTERNAL) {
                        front.must_be_stop_capture();
                        switch (mod_wrapper.old_target_module){
                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                        default:;
                        }
                    }
                    mod_wrapper.old_target_module = next_state;
                    mm.new_mod(mod_wrapper, next_state);
                }
                break;
                case MODULE_UNKNOWN:
                {
                    LOG(LOG_INFO, "===========> UNKNOWN MODULE (closing)");
                    throw Error(ERR_SESSION_CLOSE_MODULE_NEXT);
                }
                case MODULE_INTERNAL_CLOSE:
                {
                    throw Error(ERR_SESSION_CLOSE_MODULE_NEXT);
                }
                case MODULE_INTERNAL_CLOSE_BACK:
                {
                    LOG(LOG_INFO, "New_mod: MODULE_INTERNAL_CLOSE_BACK (was %s)", get_module_name(mod_wrapper.old_target_module));
                    acl->keepalive.stop();
                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                    log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());

                    mod_wrapper.connected = false;

                    if (mod_wrapper.old_target_module != MODULE_INTERNAL_CLOSE_BACK) {
                        front.must_be_stop_capture();

                        switch (mod_wrapper.old_target_module){
                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                        default:;
                        }
                    }
                    mod_wrapper.old_target_module = MODULE_INTERNAL_CLOSE_BACK;
                    mm.new_mod(mod_wrapper, MODULE_INTERNAL_CLOSE_BACK);
                }
                break;
                default:
                {
                    LOG(LOG_INFO, "New_mod (default): target_module=%s (was %s)", 
                            get_module_name(next_state), get_module_name(mod_wrapper.old_target_module));

                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                    switch (next_state) {
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
                    if (mod_wrapper.old_target_module != next_state) {
                        front.must_be_stop_capture();

                        switch (mod_wrapper.old_target_module){
                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                        default:;
                        }
                    }
                    mod_wrapper.old_target_module = next_state;
                    mm.new_mod(mod_wrapper, next_state);
                }
                }
            }
            return true;
            }

            if (!ini.get<cfg::context::disconnect_reason>().empty()) {
                acl->acl_serial.manager_disconnect_reason = ini.get<cfg::context::disconnect_reason>();
                ini.get_mutable_ref<cfg::context::disconnect_reason>().clear();
                ini.set_acl<cfg::context::disconnect_reason_ack>(true);
            }
            else if (!ini.get<cfg::context::auth_command>().empty()) {
                if (!::strcasecmp(this->ini.get<cfg::context::auth_command>().c_str(), "rail_exec")) {
                    const uint16_t flags                = ini.get<cfg::context::auth_command_rail_exec_flags>();
                    const char*    original_exe_or_file = ini.get<cfg::context::auth_command_rail_exec_original_exe_or_file>().c_str();
                    const char*    exe_or_file          = ini.get<cfg::context::auth_command_rail_exec_exe_or_file>().c_str();
                    const char*    working_dir          = ini.get<cfg::context::auth_command_rail_exec_working_dir>().c_str();
                    const char*    arguments            = ini.get<cfg::context::auth_command_rail_exec_arguments>().c_str();
                    const uint16_t exec_result          = ini.get<cfg::context::auth_command_rail_exec_exec_result>();
                    const char*    account              = ini.get<cfg::context::auth_command_rail_exec_account>().c_str();
                    const char*    password             = ini.get<cfg::context::auth_command_rail_exec_password>().c_str();

                    rdp_api* rdpapi = mod_wrapper.get_rdp_api();

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

                ini.get_mutable_ref<cfg::context::auth_command>().clear();
            }
        }

        if (mod_wrapper.is_connected()) {
            // AuthCHANNEL CHECK
            // if an answer has been received, send it to
            // rdp serveur via mod (should be rdp module)
            // TODO Check if this->mod is RDP MODULE
            if (ini.get<cfg::mod_rdp::auth_channel>()[0]) {
                // Get sesman answer to AUTHCHANNEL_TARGET
                if (!ini.get<cfg::context::auth_channel_answer>().empty()) {
                    // If set, transmit to auth_channel channel
                    mod_wrapper.get_mod()->send_auth_channel_data(ini.get<cfg::context::auth_channel_answer>().c_str());
                    // Erase the context variable
                    ini.get_mutable_ref<cfg::context::auth_channel_answer>().clear();
                }
            }

            // CheckoutCHANNEL CHECK
            // if an answer has been received, send it to
            // rdp serveur via mod (should be rdp module)
            // TODO Check if this->mod is RDP MODULE
            if (ini.get<cfg::mod_rdp::checkout_channel>()[0]) {
                // Get sesman answer to AUTHCHANNEL_TARGET
                if (!ini.get<cfg::context::pm_response>().empty()) {
                    // If set, transmit to auth_channel channel
                    mod_wrapper.get_mod()->send_checkout_channel_data(ini.get<cfg::context::pm_response>().c_str());
                    // Erase the context variable
                    ini.get_mutable_ref<cfg::context::pm_response>().clear();
                }
            }

            if (!ini.get<cfg::context::rd_shadow_type>().empty()) {
                mod_wrapper.get_mod()->create_shadow_session(ini.get<cfg::context::rd_shadow_userdata>().c_str(),
                    ini.get<cfg::context::rd_shadow_type>().c_str());
                ini.get_mutable_ref<cfg::context::rd_shadow_type>().clear();
            }
        }
        return true;

    }


    void show_ultimatum(std::string info, timeval ultimatum, timeval now) 
    {
        timeval timeoutastv = to_timeval(std::chrono::seconds(ultimatum.tv_sec) + std::chrono::microseconds(ultimatum.tv_usec)
                    - std::chrono::seconds(now.tv_sec) - std::chrono::microseconds(now.tv_usec));
        if (timeoutastv.tv_sec == 0 && timeoutastv.tv_usec == 0){
            LOG(LOG_INFO, "%s %ld.%ld s", info, timeoutastv.tv_sec, timeoutastv.tv_usec/100000);
        }
    }

    timeval prepare_timeout(timeval ultimatum, timeval now,
                const Front & front,
                TimerContainer & timer_events_,
                TopFdContainer & fd_events_,
                GraphicTimerContainer & graphic_timer_events_,
                GraphicFdContainer & graphic_fd_events_,
                const GraphicEventContainer & graphic_events_,
                bool front_pending,
                bool mod_pending)
    {
        this->show_ultimatum("", ultimatum, now);
        auto top_update_tv = [&](int /*fd*/, auto& top){
            if (top.timer_data.is_enabled) {
                if (top.timer_data.tv.tv_sec >= 0) {
                    this->show_ultimatum("top timer =", top.timer_data.tv, now);
                    ultimatum = std::min(ultimatum, top.timer_data.tv);
                }
            }
        };

        auto timer_update_tv = [&](auto& timer){
            if (timer.tv.tv_sec >= 0) {
                this->show_ultimatum("timer =", timer.tv, now);
                ultimatum = std::min(ultimatum, timer.tv);
            }
        };

        timer_events_.for_each(timer_update_tv);
        this->show_ultimatum("ultimatum (timers) =", ultimatum, now);

        fd_events_.for_each(top_update_tv);
        this->show_ultimatum("ultimatm (fd) =", ultimatum, now);

        if (front.state == Front::FRONT_UP_AND_RUNNING) {
            graphic_timer_events_.for_each(timer_update_tv);
            this->show_ultimatum("ultimatum (graphic timer) =", ultimatum, now);
            graphic_fd_events_.for_each(top_update_tv);
            this->show_ultimatum("ultimatum (graphic fd) =", ultimatum, now);
        }

        if (front.front_must_notify_resize) {
            ultimatum = now;
            this->show_ultimatum("ultimatum (front must notify resize) =", ultimatum, now);
        }

        if (mod_pending) {
            ultimatum = now;
            this->show_ultimatum("(mod tls pending)", ultimatum, now);
        }

        if ((front.state == Front::FRONT_UP_AND_RUNNING and !graphic_events_.is_empty())) {
            ultimatum = now;
            this->show_ultimatum("(mod graphic event)", ultimatum, now);
        }

        if (front_pending) {
            ultimatum = now;
            this->show_ultimatum("(front tls pending)", ultimatum, now);
        }
        return ultimatum;
    }


    void front_incoming_data(SocketTransport& front_trans, Front & front, ModWrapper & mod_wrapper, SesmanInterface & sesman)
    {
        if (front.front_must_notify_resize) {
            front.notify_resize(mod_wrapper.get_callback());
        }

        front.rbuf.load_data(front_trans);

        while (front.rbuf.next(TpduBuffer::PDU)) // or TdpuBuffer::CredSSP in NLA
        {
            bytes_view tpdu = front.rbuf.current_pdu_buffer();
            uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
            front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback(), sesman);
        }
    }

    void acl_incoming_data(Acl & acl, Inifile& ini, ModWrapper & mod_wrapper)
    {
        acl.acl_serial.receive();
        if (!ini.changed_field_size()) {
            mod_wrapper.acl_update();
        }
    }

public:
    Session(SocketTransport&& front_trans, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
    : ini(ini)
    {

        TRANSLATIONCONF.set_ini(&ini);
        std::string disconnection_message_error;

        Authentifier authentifier(ini, cctx, to_verbose_flags(ini.get<cfg::debug::auth>()));

        SessionReactor session_reactor;
        TopFdContainer fd_events_;
        GraphicFdContainer graphic_fd_events_;
        TimerContainer timer_events_;
        GraphicEventContainer graphic_events_;
        GraphicTimerContainer graphic_timer_events_;
        
        TimeSystem timeobj;

        session_reactor.set_current_time(tvtime());
        SesmanInterface sesman(ini);
        Front front(session_reactor, timer_events_, sesman, front_trans, rnd, ini, cctx, authentifier,
            ini.get<cfg::client::fast_path>()
        );
        std::unique_ptr<Acl> acl;

        try {
            Font glyphs = Font(app_path(AppPath::DefaultFontFile), ini.get<cfg::globals::spark_view_specific_glyph_width>());

            auto & theme_name = this->ini.get<cfg::internal_mod::theme>();
            LOG_IF(this->ini.get<cfg::debug::config>(), LOG_INFO, "LOAD_THEME: %s", theme_name);
            Theme theme;
            ::load_theme(theme, theme_name);

            ClientExecute rail_client_execute(session_reactor, timer_events_, front, front, front.client_info.window_list_caps, ini.get<cfg::debug::mod_internal>() & 1);

            windowing_api* winapi = nullptr;
            ModWrapper mod_wrapper(front, front.get_palette(), front, front.keymap, front.client_info, glyphs, rail_client_execute, winapi, this->ini);
            ModFactory mod_factory(mod_wrapper, session_reactor, sesman, graphic_fd_events_, timer_events_, graphic_events_, graphic_timer_events_, front.client_info, front, front, ini, glyphs, theme, rail_client_execute, authentifier);
            EndSessionWarning end_session_warning;

            ModuleManager mm(mod_factory, session_reactor, fd_events_, graphic_fd_events_, timer_events_, graphic_events_, sesman, front, front.keymap, front.client_info, rail_client_execute, glyphs, theme, this->ini, cctx, rnd, timeobj, authentifier, authentifier);

            if (ini.get<cfg::debug::session>()) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(nullptr);
            if (ini.get<cfg::debug::performance>() & 0x8000) {
                this->write_performance_log(start_time);
            }

            bool run_session = true;

            using namespace std::chrono_literals;

            // POSSIBLE STATES FOR SESSION:
            // ============================
            // SESSION_STATE_INCOMING:        FRONT CONNECTED, NO ACL, NO MODULE
            //      This state is the initial state of the session when RDP client just connected
            //      to the socket. The connection to the socket has not yet been opened.
            //      No Module.
            
            // SESSION_STATE_RUNNING:         FRONT CONNECTED, ACL, MODULE
            //        Front is still connected (either performaning initial negotiation or
            //        already connected and up and running). ACL is connected.
            //        For now from proxy point of view we don't make any difference 
            //        if ACL has performed primary authentication or not.
            //        A module is available through mod_wrapper (can be either null module
            //        some internal module of a module actually connected to a remote server)
            
            // SESSION_STATE_BACKEND_CLEANUP: FRONT DISCONNECTED: CLOSING BACKEND (ACL, MODULE)
            //        The front socket raised an error which means it has been disconnected
            //        but we may still have to prperly disconnect from target server,
            //        and to disconnect from ACL.
            
            // SESSION_STATE_CLOSE_BOX:       FRONT CONNECTED: BACKEND CLOSED (NO ACL, NO MODULE)
            //        The front socket is still connected, but the connection to ACL has been closed
            //        the close of ACL may have occured from sesman side or from proxy side
            //        and be caused either a deconnexion (or hang) of proxy, a failure to
            //        reply to keepalive, or as a consequence of disconnection from target server.

           
            SessionState session_state = SessionState::SESSION_STATE_INCOMING;

            while (run_session) {

                timeval now = tvtime();
                session_reactor.set_current_time(now);

                Select ioswitch(timeval{now.tv_sec + this->select_timeout_tv_sec, now.tv_usec});

                switch (session_state) {
                case SessionState::SESSION_STATE_INCOMING:
                {
                    // Pre assertion: connected front socket
                    // No ACL socket, No connected mode

                    bool front_has_waiting_data_to_write = front_trans.has_data_to_write();

                    // =============================================================
                    // This block takes care of outgoing data waiting in buffers because system write buffer is full
                    if (front_has_waiting_data_to_write){
                        if (front_has_waiting_data_to_write){
                            ioswitch.set_write_sck(front_trans.sck);
                        }
                        int num = ioswitch.select(now);
                        if (num < 0) {
                            if (errno != EINTR) {
                                // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                                // EBADF: means fd has been closed (by me) or as already returned an error on another call
                                // EINVAL: invalid value in timeout (my fault again)
                                // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                                LOG(LOG_ERR, "Proxy send wait raised error %d : %s", errno, strerror(errno));
                                run_session = false;
                                continue;
                            }
                        }

                        if (front_trans.sck != INVALID_SOCKET && ioswitch.is_set_for_writing(front_trans.sck)) {
                            front_trans.send_waiting_data();
                        }
                        if (num > 0) { continue; }
                        // if the select stopped on timeout or EINTR we will give a try to reading
                    }
                    
                    // =============================================================
                    // Now prepare select for listening on all read sockets
                    // timeout or immediate wakeups are managed using timeout
                    // =============================================================


                    // sockets for mod or front aren't managed using fd events
                    if (front_trans.sck != INVALID_SOCKET) {
    //                    LOG(LOG_INFO, "Wait for read event on front fd=%d", front_trans.sck);
                        ioswitch.set_read_sck(front_trans.sck);
                    }

                    // if event lists are waiting for incoming data 
                    fd_events_.for_each([&](int fd, auto& /*top*/){ 
                            if (fd != INVALID_SOCKET){ioswitch.set_read_sck(fd);}
                    });
                    
                    timeval ultimatum = prepare_timeout(ioswitch.get_timeout(), now,
                            front,  
                            timer_events_,
                            fd_events_,
                            graphic_timer_events_,
                            graphic_fd_events_,
                            graphic_events_,
                            front_trans.has_tls_pending_data(),
                            false
                            );

                    ioswitch.set_timeout(ultimatum);

                    int num = ioswitch.select(now);

                    if (num < 0) {
                        if (errno != EINTR) {
                            // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                            // EBADF: means fd has been closed (by me) or as already returned an error on another call
                            // EINVAL: invalid value in timeout (my fault again)
                            // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                            LOG(LOG_ERR, "Proxy data wait loop raised error %d : %s", errno, strerror(errno));
                            run_session = false;
                        }
                        continue;
                    }

                    now = tvtime();
                    session_reactor.set_current_time(now);
                    
                    if (ini.get<cfg::debug::performance>() & 0x8000) {
                        this->write_performance_log(now.tv_sec);
                    }

                    bool const front_is_set = front_trans.has_tls_pending_data() 
                    || (front_trans.sck != INVALID_SOCKET 
                    && ioswitch.is_set_for_reading(front_trans.sck));

                    try {
                        if (front_is_set){
                            this->front_incoming_data(front_trans, front, mod_wrapper, sesman);
                        }
                    } catch (Error const& e) {
                        // RemoteApp disconnection initiated by user
                        // ERR_DISCONNECT_BY_USER == e.id
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU)
                            // Can be caused by client disconnect.
                            && (e.id != ERR_MCS_APPID_IS_MCS_DPUM)
                            && (e.id != ERR_RDP_HANDSHAKE_TIMEOUT)
                            // Can be caused by wabwatchdog.
                            && (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        front_trans.sck = INVALID_SOCKET;
                        run_session = false;
                        continue;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                        continue;
                    }

                    switch (front.state) {
                    default:
                    {
                        fd_events_.exec_action([&ioswitch](int fd, auto& /*e*/){
                            return fd != INVALID_SOCKET && ioswitch.is_set_for_reading(fd);
                        });
                    }
                    break;
                    case Front::FRONT_UP_AND_RUNNING:
                    {
                        try {
                            this->start_acl_running(acl, cctx, rnd, now, ini, authentifier, fstat);
                            session_state = SessionState::SESSION_STATE_RUNNING;
                        }
                        catch (...) {
                            this->ini.set<cfg::context::auth_error_message>("No authentifier available");
                            mod_wrapper.last_disconnect();
                            session_state = SessionState::SESSION_STATE_CLOSE_BOX;
                            run_session = false;
                            LOG(LOG_INFO, "start acl failed");
                            if (ini.get<cfg::globals::enable_close_box>()) {
                                LOG(LOG_INFO, "New_mod: MODULE_INTERNAL_CLOSE (was %s)", get_module_name(mod_wrapper.old_target_module));
                                rail_client_execute.enable_remote_program(front.client_info.remote_program);
                                log_proxy::set_user("");
                                mod_wrapper.connected = false;

                                if (mod_wrapper.old_target_module != MODULE_INTERNAL_CLOSE) {
                                    front.must_be_stop_capture();
                                    switch (mod_wrapper.old_target_module){
                                    case MODULE_XUP: authentifier.delete_remote_mod(); break;
                                    case MODULE_RDP: authentifier.delete_remote_mod(); break;
                                    case MODULE_VNC: authentifier.delete_remote_mod(); break;
                                    default:;
                                    }
                                }
                                mod_wrapper.old_target_module = MODULE_INTERNAL_CLOSE;
                                mm.new_mod(mod_wrapper, MODULE_INTERNAL_CLOSE);
                                run_session = true;
                            }
                            continue;
                        }
                    }
                    break;
                    } // switch front_state
                }
                break;

                case SessionState::SESSION_STATE_RUNNING:
                {
                    // Pre assertion: connected front socket
                    // No ACL socket, No connected mode

                    bool front_has_waiting_data_to_write = front_trans.has_data_to_write();
                    bool mod_has_waiting_data_to_write   = mod_wrapper.has_mod()
                                                        && mod_wrapper.get_mod_transport()
                                                        && mod_wrapper.get_mod_transport()->has_data_to_write();

                    // =============================================================
                    // This block takes care of outgoing data waiting in buffers because system write buffer is full
                    if (front_has_waiting_data_to_write || mod_has_waiting_data_to_write){
                        if (front_has_waiting_data_to_write){
                            ioswitch.set_write_sck(front_trans.sck);
                        }
                        if (mod_has_waiting_data_to_write){
                            ioswitch.set_write_sck(mod_wrapper.get_mod_transport()->sck);
                        }

                        int num = ioswitch.select(now);
                        if (num < 0) {

                            if (errno != EINTR) {
                                // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                                // EBADF: means fd has been closed (by me) or as already returned an error on another call
                                // EINVAL: invalid value in timeout (my fault again)
                                // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                                LOG(LOG_ERR, "Proxy send wait raised error %d : %s", errno, strerror(errno));
                                run_session = false;
                                continue;
                            }
                        }
                        if (mod_wrapper.has_mod() 
                        && mod_wrapper.get_mod_transport() 
                        && ioswitch.is_set_for_writing(mod_wrapper.get_mod_transport()->sck)) {
                            mod_wrapper.get_mod_transport()->send_waiting_data();
                        }

                        if (front_trans.sck != INVALID_SOCKET
                        && ioswitch.is_set_for_writing(front_trans.sck)) {
                            front_trans.send_waiting_data();
                        }
                        if (num > 0) { continue; }
                        // if the select stopped on timeout or EINTR we will give a try to reading
                    }
                    
                    // =============================================================
                    // Now prepare select for listening on all read sockets
                    // timeout or immediate wakeups are managed using timeout
                    // =============================================================


                    // sockets for mod or front aren't managed using fd events
                    if (mod_wrapper.has_mod() && mod_wrapper.get_mod_transport()) {
                        int fd = mod_wrapper.get_mod_transport()->sck;
    //                    LOG(LOG_INFO, "Wait for read event on mod fd=%d", fd);
                        if (fd != INVALID_SOCKET) {
                            ioswitch.set_read_sck(fd);
                        }
                    }

                    if (front_trans.sck != INVALID_SOCKET) {
    //                    LOG(LOG_INFO, "Wait for read event on front fd=%d", front_trans.sck);
                        ioswitch.set_read_sck(front_trans.sck);
                    }

                    // if event lists are waiting for incoming data 
                    fd_events_.for_each(
                        [&](int fd, auto& /*top*/){ 
    //                        LOG(LOG_INFO, "Wait for read event on fd=%d", fd);
                            if (fd != INVALID_SOCKET){
                                ioswitch.set_read_sck(fd);
                            }
                    });
                    
                    if (mod_wrapper.has_mod() and front.state == Front::FRONT_UP_AND_RUNNING) {
                        graphic_fd_events_.for_each(
                        [&](int fd, auto& /*top*/){ 
    //                        LOG(LOG_INFO, "Wait for read event on graphic fd=%d", fd);
                            if (fd != INVALID_SOCKET){
                                ioswitch.set_read_sck(fd);
                            }
                    });
                    }

                    ioswitch.set_read_sck(acl->auth_trans.sck);

                    bool mod_data_pending = (mod_wrapper.has_mod()
                            && mod_wrapper.get_mod_transport()
                            && mod_wrapper.get_mod_transport()->has_tls_pending_data());

                    timeval ultimatum = prepare_timeout(ioswitch.get_timeout(), now,
                            front,  
                            timer_events_,
                            fd_events_,
                            graphic_timer_events_,
                            graphic_fd_events_,
                            graphic_events_,
                            front_trans.has_tls_pending_data(),
                            mod_data_pending
                            );

                    ioswitch.set_timeout(ultimatum);

                    int num = ioswitch.select(now);

                    if (num < 0) {
                        if (errno != EINTR) {
                            // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                            // EBADF: means fd has been closed (by me) or as already returned an error on another call
                            // EINVAL: invalid value in timeout (my fault again)
                            // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                            LOG(LOG_ERR, "Proxy data wait loop raised error %d : %s", errno, strerror(errno));
                            run_session = false;
                        }
                        continue;
                    }

                    now = tvtime();
                    session_reactor.set_current_time(now);
                    
                    if (ini.get<cfg::debug::performance>() & 0x8000) {
                        this->write_performance_log(now.tv_sec);
                    }

                    bool const front_is_set = front_trans.has_tls_pending_data() 
                    || (front_trans.sck != INVALID_SOCKET 
                    && ioswitch.is_set_for_reading(front_trans.sck));

                    bool acl_is_set = ioswitch.is_set_for_reading(acl->auth_trans.sck);

                    try {
                        if (front_is_set){
                            this->front_incoming_data(front_trans, front, mod_wrapper, sesman);
                        }
                    } catch (Error const& e) {
                        // RemoteApp disconnection initiated by user
                        // ERR_DISCONNECT_BY_USER == e.id
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU)
                            // Can be caused by client disconnect.
                            && (e.id != ERR_MCS_APPID_IS_MCS_DPUM)
                            && (e.id != ERR_RDP_HANDSHAKE_TIMEOUT)
                            // Can be caused by wabwatchdog.
                            && (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        front_trans.sck = INVALID_SOCKET;
                        run_session = false;
                        continue;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                        continue;
                    }

                    if (acl_is_set) {
                        this->acl_incoming_data(*acl.get(), ini, mod_wrapper);
                    }

                    switch (front.state) {
                    default:
                    {
                        fd_events_.exec_action([&ioswitch](int fd, auto& /*e*/){
                            return fd != INVALID_SOCKET && ioswitch.is_set_for_reading(fd);
                        });
                    }
                    break;
                    case Front::FRONT_UP_AND_RUNNING:
                    {
                        if (!sesman.auth_info_sent){
                            sesman.set_acl_screen_info();
                            sesman.set_acl_auth_info();
                            if (this->ini.changed_field_size()) {
                                acl->acl_serial.send_acl_data();
                                continue;
                            }
                        }

                        // Exception ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE
                        try {
                            // Close by end date reached
                            const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
                            if (enddate != 0 && (static_cast<uint32_t>(now.tv_sec) > enddate)) {
                                throw Error(ERR_SESSION_CLOSE_ENDDATE_REACHED);
                            }
                            // Close by rejeted message received
                            if (!this->ini.get<cfg::context::rejected>().empty()) {
                                throw Error(ERR_SESSION_CLOSE_REJECTED_BY_ACL_MESSAGE);
                            }
                            // Keep Alive
                            if (acl->keepalive.check(now.tv_sec, this->ini)) {
                                throw Error(ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED);
                            }
                            // Inactivity management
                            if (acl->inactivity.check_user_activity(now.tv_sec, front.has_user_activity)) {
                                throw Error(ERR_SESSION_CLOSE_USER_INACTIVITY);
                            }

                            auto const end_tv = session_reactor.get_current_time();
                            timer_events_.exec_timer(end_tv);
                            fd_events_.exec_timeout(end_tv);
                            graphic_timer_events_.exec_timer(end_tv, mod_wrapper.get_graphic_wrapper());
                            graphic_fd_events_.exec_timeout(end_tv, mod_wrapper.get_graphic_wrapper());
                            fd_events_.exec_action([&ioswitch](int fd, auto& /*e*/){
                                return fd != INVALID_SOCKET && ioswitch.is_set_for_reading(fd);});

                            // new value incoming from authentifier
                           this->rt_display(ini, mod_wrapper, front);

                           if (BACK_EVENT_NONE == mod_wrapper.get_mod()->get_mod_signal()) {
                                auto& gd = mod_wrapper.get_graphic_wrapper();
                                graphic_events_.exec_action(gd);
                                graphic_fd_events_.exec_action([&ioswitch](int fd, auto& /*e*/){
                                    return fd != INVALID_SOCKET 
                                        &&  ioswitch.is_set_for_reading(fd);
                                }, gd);
                            }

                            run_session = this->front_up_and_running(acl, now, start_time, ini, mm, mod_wrapper, end_session_warning, front, authentifier, rail_client_execute);

                        } catch (Error const& e) {
                            LOG(LOG_ERR, "Exception in sequencing = %s", e.errmsg());
                            run_session = false;
                            switch (end_session_exception(e, authentifier, ini)){
                            case 0: // End of session loop
                            break;
                            case 1: // Close Box
                            {
                                session_state = SessionState::SESSION_STATE_CLOSE_BOX;
                                mod_wrapper.last_disconnect();
                                authentifier.set_acl_serial(nullptr);
                                acl.reset();
                                if (ini.get<cfg::globals::enable_close_box>()) {
                                    LOG(LOG_INFO, "New_mod: MODULE_INTERNAL_CLOSE (was %s)", get_module_name(mod_wrapper.old_target_module));
                                    rail_client_execute.enable_remote_program(front.client_info.remote_program);
                                    log_proxy::set_user("");
                                    mod_wrapper.connected = false;

                                    if (mod_wrapper.old_target_module != MODULE_INTERNAL_CLOSE) {
                                        front.must_be_stop_capture();

                                        switch (mod_wrapper.old_target_module){
                                        case MODULE_XUP: authentifier.delete_remote_mod(); break;
                                        case MODULE_RDP: authentifier.delete_remote_mod(); break;
                                        case MODULE_VNC: authentifier.delete_remote_mod(); break;
                                        default:;
                                        }
                                    }
                                    mod_wrapper.old_target_module = MODULE_INTERNAL_CLOSE;
                                    mm.new_mod(mod_wrapper, MODULE_INTERNAL_CLOSE);
                                    run_session = true;
                                }
                            }
                            break;
                            case 2: // TODO: should we put some counter to avoid retrying indefinitely?
                                acl->acl_serial.remote_answer = false;
                                LOG(LOG_INFO, "Retrying current module");
                                mod_wrapper.remove_mod();

                                LOG(LOG_INFO, "New_mod: MODULE_RDP (was %s)", get_module_name(mod_wrapper.old_target_module));
                                rail_client_execute.enable_remote_program(front.client_info.remote_program);
                                log_proxy::set_user(this->ini.get<cfg::globals::auth_user>().c_str());
                                mod_wrapper.connected = false;
                                if (mod_wrapper.old_target_module != MODULE_RDP) {
                                    front.must_be_stop_capture();
                                    switch (mod_wrapper.old_target_module){
                                    case MODULE_XUP: authentifier.delete_remote_mod(); break;
                                    case MODULE_RDP: authentifier.delete_remote_mod(); break;
                                    case MODULE_VNC: authentifier.delete_remote_mod(); break;
                                    default:;
                                    }
                                    authentifier.new_remote_mod();
                                }
                                mod_wrapper.old_target_module = MODULE_RDP;
                                mm.new_mod(mod_wrapper, MODULE_RDP);
                                run_session = true;
                            break;
                            }
                        }
                    }
                    break;
                    } // switch
    //                LOG(LOG_INFO, "while loop run_session=%s", run_session?"true":"false");            
                }
                break;

                case SessionState::SESSION_STATE_BACKEND_CLEANUP:
                {
                }
                break;

                case SessionState::SESSION_STATE_CLOSE_BOX:
                {
                    // Pre assertion: front socket still connected
                    // ACL already disconnected
                    // current mode is close mod

                    bool front_has_waiting_data_to_write = front_trans.has_data_to_write();

                    // =============================================================
                    // This block takes care of outgoing data waiting in buffers because system write buffer is full
                    if (front_has_waiting_data_to_write){
                        if (front_has_waiting_data_to_write){
                            ioswitch.set_write_sck(front_trans.sck);
                        }

                        int num = ioswitch.select(now);
                        if (num < 0) {
                            if (errno != EINTR) {
                                // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                                // EBADF: means fd has been closed (by me) or as already returned an error on another call
                                // EINVAL: invalid value in timeout (my fault again)
                                // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                                LOG(LOG_ERR, "Proxy send wait raised error %d : %s", errno, strerror(errno));
                                run_session = false;
                                continue;
                            }
                        }

                        if (front_trans.sck != INVALID_SOCKET && ioswitch.is_set_for_writing(front_trans.sck)) {
                            front_trans.send_waiting_data();
                        }
                        if (num > 0) { continue; }
                        // if the select stopped on timeout or EINTR we will give a try to reading
                    }
                    
                    // =============================================================
                    // Now prepare select for listening on all read sockets
                    // timeout or immediate wakeups are managed using timeout
                    // =============================================================
                    
                    // sockets for mod or front aren't managed using fd events
                    if (front_trans.sck != INVALID_SOCKET) {
                        ioswitch.set_read_sck(front_trans.sck);
                    }

                    // if event lists are waiting for incoming data 
                    fd_events_.for_each([&](int fd, auto& /*top*/){ 
                            if (fd != INVALID_SOCKET){ioswitch.set_read_sck(fd);}});

                    if (mod_wrapper.has_mod() and front.state == Front::FRONT_UP_AND_RUNNING) {
                        graphic_fd_events_.for_each([&](int fd, auto& /*top*/){ 
                            if (fd != INVALID_SOCKET){ioswitch.set_read_sck(fd);}});
                    }

                    timeval ultimatum = prepare_timeout(ioswitch.get_timeout(), now,
                            front,  
                            timer_events_,
                            fd_events_,
                            graphic_timer_events_,
                            graphic_fd_events_,
                            graphic_events_,
                            front_trans.has_tls_pending_data(),
                            false
                            );

                    ioswitch.set_timeout(ultimatum);

                    int num = ioswitch.select(now);
                    if (num < 0) {
                        if (errno != EINTR) {
                            // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                            // EBADF: means fd has been closed (by me) or as already returned an error on another call
                            // EINVAL: invalid value in timeout (my fault again)
                            // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)
                            LOG(LOG_ERR, "Proxy data wait loop raised error %d : %s", errno, strerror(errno));
                            run_session = false;
                        }
                        continue;
                    }

                    now = tvtime();
                    session_reactor.set_current_time(now);
                    
                    if (ini.get<cfg::debug::performance>() & 0x8000) {
                        this->write_performance_log(now.tv_sec);
                    }

                    bool const front_is_set = front_trans.has_tls_pending_data() 
                    || (front_trans.sck != INVALID_SOCKET && ioswitch.is_set_for_reading(front_trans.sck));

                    try {
                        if (front_is_set){
                            this->front_incoming_data(front_trans, front, mod_wrapper, sesman);
                        }

                        auto const end_tv = session_reactor.get_current_time();
                        timer_events_.exec_timer(end_tv);
                        fd_events_.exec_timeout(end_tv);
                        if (EnableGraphics{true}) {
                            graphic_timer_events_.exec_timer(end_tv, mod_wrapper.get_graphic_wrapper());
                            graphic_fd_events_.exec_timeout(end_tv, mod_wrapper.get_graphic_wrapper());
                        }
                        fd_events_.exec_action([&ioswitch](int fd, auto& /*e*/)
                                                { return fd != INVALID_SOCKET 
                                                        &&  ioswitch.is_set_for_reading(fd);
                                                });
                        BackEvent_t signal = mod_wrapper.get_mod()->get_mod_signal();
                        if ((signal == BACK_EVENT_STOP)||(signal==BACK_EVENT_NEXT)){
                            run_session = false;
                        }
                    } catch (Error const& e) {

                        // RemoteApp disconnection initiated by user
                        // ERR_DISCONNECT_BY_USER == e.id
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU)
                            // Can be caused by client disconnect.
                            && (e.id != ERR_MCS_APPID_IS_MCS_DPUM)
                            && (e.id != ERR_RDP_HANDSHAKE_TIMEOUT)
                            // Can be caused by wabwatchdog.
                            && (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        front_trans.sck = INVALID_SOCKET;
                        run_session = false;
                        continue;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                        continue;
                    }
                }
                break;
                } // switch SESSION_STATE

            } // loop

            if (mod_wrapper.get_mod()) {
                mod_wrapper.get_mod()->disconnect();
            }
            front.disconnect();
        }
        catch (Error const& e) {
            disconnection_message_error = e.errmsg();
            LOG(LOG_INFO, "Session::Session Init exception = %s!", disconnection_message_error);
        }
        catch (const std::exception & e) {
            disconnection_message_error = e.what();
            LOG(LOG_ERR, "Session::Session exception (3) = %s!", disconnection_message_error);
        }
        catch(...) {
            disconnection_message_error = "Exception in Session::Session";
            LOG(LOG_ERR, "Session::Session other exception in Init");
        }
        // silent message for localhost for watchdog
        if (ini.get<cfg::globals::host>() != "127.0.0.1") {
            if (!ini.is_asked<cfg::globals::host>()) {
                LOG(LOG_INFO, "Session::Client Session Disconnected");
            }
            log_proxy::disconnection(disconnection_message_error.c_str());
        }
        front.must_be_stop_capture();
    }

    Session(Session const &) = delete;

    ~Session() {
        if (this->ini.template get<cfg::debug::performance>() & 0x8000) {
            this->write_performance_log(this->perf_last_info_collect_time + 3);
        }
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        auto const& session_id = this->ini.template get<cfg::context::session_id>();
        if (!session_id.empty()) {
            char new_session_file[256];
            snprintf( new_session_file, sizeof(new_session_file), "%s/session_%s.pid"
                    , app_path(AppPath::LockDir).c_str(), session_id.c_str());
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", app_path(AppPath::LockDir).c_str(), child_pid);
            unlink(old_session_file);
        }
    }

private:
    void write_performance_log(time_t now) {
        if (!this->perf_last_info_collect_time) {
            assert(!this->perf_file);

            this->perf_last_info_collect_time = now - this->select_timeout_tv_sec;

            struct tm tm_;

            localtime_r(&this->perf_last_info_collect_time, &tm_);

            char filename[2048];
            snprintf(filename, sizeof(filename), "%s/rdpproxy,%04d%02d%02d-%02d%02d%02d,%d.perf",
                this->ini.template get<cfg::video::record_tmp_path>().c_str(),
                tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec, this->perf_pid
                );

            this->perf_file = File(filename, "w");
            this->perf_file.write(cstr_array_view(
                "time_t;"
                "ru_utime.tv_sec;ru_utime.tv_usec;ru_stime.tv_sec;ru_stime.tv_usec;"
                "ru_maxrss;ru_ixrss;ru_idrss;ru_isrss;ru_minflt;ru_majflt;ru_nswap;"
                "ru_inblock;ru_oublock;ru_msgsnd;ru_msgrcv;ru_nsignals;ru_nvcsw;ru_nivcsw\n"));
        }
        else if (this->perf_last_info_collect_time + this->select_timeout_tv_sec > now) {
            return;
        }

        struct rusage resource_usage;

        getrusage(RUSAGE_SELF, &resource_usage);

        do {
            this->perf_last_info_collect_time += this->select_timeout_tv_sec;

            struct tm result;

            localtime_r(&this->perf_last_info_collect_time, &result);

            ::fprintf(
                  this->perf_file.get()
                , "%lu;"
                  "%lu;%lu;%lu;%lu;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld;%ld\n"
                , static_cast<unsigned long>(now)
                , static_cast<unsigned long>(resource_usage.ru_utime.tv_sec)  /* user CPU time used */
                , static_cast<unsigned long>(resource_usage.ru_utime.tv_usec)
                , static_cast<unsigned long>(resource_usage.ru_stime.tv_sec)  /* system CPU time used */
                , static_cast<unsigned long>(resource_usage.ru_stime.tv_usec)
                , resource_usage.ru_maxrss                                    /* maximum resident set size */
                , resource_usage.ru_ixrss                                     /* integral shared memory size */
                , resource_usage.ru_idrss                                     /* integral unshared data size */
                , resource_usage.ru_isrss                                     /* integral unshared stack size */
                , resource_usage.ru_minflt                                    /* page reclaims (soft page faults) */
                , resource_usage.ru_majflt                                    /* page faults (hard page faults)   */
                , resource_usage.ru_nswap                                     /* swaps */
                , resource_usage.ru_inblock                                   /* block input operations */
                , resource_usage.ru_oublock                                   /* block output operations */
                , resource_usage.ru_msgsnd                                    /* IPC messages sent */
                , resource_usage.ru_msgrcv                                    /* IPC messages received */
                , resource_usage.ru_nsignals                                  /* signals received */
                , resource_usage.ru_nvcsw                                     /* voluntary context switches */
                , resource_usage.ru_nivcsw                                    /* involuntary context switches */
            );
            this->perf_file.flush();
        }
        while (this->perf_last_info_collect_time + this->select_timeout_tv_sec <= now);
    }

    static unique_fd acl_connect(std::string const & authtarget, bool no_log)
    {
        unique_fd client_sck = addr_connect(authtarget.c_str(), no_log);
        if (!client_sck.is_open()) {
            LOG(LOG_ERR,
                "Failed to connect to authentifier (%s)",
                authtarget.c_str());
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        return client_sck;
    }
};

template<class SocketType, class... Args>
void session_start_sck(
    char const* name, unique_fd sck,
    Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat,
    Args&&... args)
{
    Session session(SocketType(
        name, std::move(sck), "", 0, ini.get<cfg::client::recv_timeout>(),
        static_cast<Args&&>(args)...,
        to_verbose_flags(ini.get<cfg::debug::front>() | (!strcmp(ini.get<cfg::globals::host>().c_str(), "127.0.0.1") ? uint64_t(SocketTransport::Verbose::watchdog) : 0))
    ), ini, cctx, rnd, fstat);
}

} // anonymous namespace

void session_start_tls(unique_fd sck, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
{
    session_start_sck<SocketTransport>("RDP Client", std::move(sck), ini, cctx, rnd, fstat);
}

void session_start_ws(unique_fd sck, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
{
    session_start_sck<WsTransport>("RDP Ws Client", std::move(sck), ini, cctx, rnd, fstat,
        WsTransport::UseTls(false), WsTransport::TlsOptions());
}

void session_start_wss(unique_fd sck, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
{
    session_start_sck<WsTransport>("RDP Wss Client", std::move(sck), ini, cctx, rnd, fstat,
        WsTransport::UseTls(true), WsTransport::TlsOptions{
            ini.get<cfg::globals::certificate_password>(),
            ini.get<cfg::client::ssl_cipher_list>(),
            ini.get<cfg::client::tls_min_level>(),
            ini.get<cfg::client::tls_max_level>(),
            ini.get<cfg::client::show_common_cipher_list>(),
        });
}

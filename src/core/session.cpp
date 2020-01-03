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
#include "capture/capture.hpp"
#include "configs/config.hpp"
#include "core/session_reactor.hpp"
#include "core/set_server_redirection_target.hpp"
#include "front/front.hpp"
#include "mod/mod_api.hpp"
#include "transport/socket_transport.hpp"
#include "transport/ws_transport.hpp"
#include "utils/bitmap.hpp"
#include "utils/colors.hpp"
#include "utils/file.hpp"
#include "utils/genfstat.hpp"
#include "utils/invalid_socket.hpp"
#include "utils/netutils.hpp"
#include "utils/rect.hpp"
#include "utils/select.hpp"
#include "utils/stream.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/log_siem.hpp"
#include "utils/load_theme.hpp"

#include <array>

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
            io_fd_zero(this->rfds);
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

            return ::select(
                this->max + 1, &this->rfds,
                this->want_write ? &this->wfds : nullptr,
                nullptr, &timeoutastv);
        }

        void set_timeout(timeval next_timeout)
        {
            this->timeout = next_timeout;
        }

        std::chrono::milliseconds get_timeout(timeval now)
        {
            const timeval & ultimatum = this->timeout;
            const timeval & starttime = now;
            if (ultimatum > starttime) {
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::seconds(ultimatum.tv_sec) + std::chrono::microseconds(ultimatum.tv_usec)
                        - std::chrono::seconds(starttime.tv_sec) - std::chrono::microseconds(starttime.tv_usec));
            }
            return 0ms;
        }

        bool is_set(int fd){
            return fd != INVALID_SOCKET && io_fd_isset(fd, this->wfds);
        };

        void set_read_sck(int sck)
        {
            this->max = prepare_fds(sck, this->max, this->rfds);
        }

        void set_write_sck(int sck)
        {
            if (!this->want_write) {
                this->want_write = true;
                io_fd_zero(this->wfds);
            }
            this->max = prepare_fds(sck, this->max, this->wfds);
        }

        void immediate_wakeup(timeval now)
        {
            this->timeout = now;
        }
    };

    struct [[nodiscard]] SckNoRead
    {
        int sck_front = INVALID_SOCKET;
        int sck_mod = INVALID_SOCKET;
        // int sck_acl = INVALID_SOCKET;

        [[nodiscard]] bool contains(int fd) const noexcept
        {
            return sck_front == fd
                || sck_mod == fd
                // || sck_acl == fd
            ;
        }
    };

    bool last_module{false};

    Inifile & ini;

    time_t      perf_last_info_collect_time = 0;
    const pid_t perf_pid = getpid();
    File        perf_file = nullptr;

    static const time_t select_timeout_tv_sec = 3;

    int check_exception(Error const& e, Inifile& ini)
    {
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
                SessionProbeOnLaunchFailure::retry_without_session_probe) {
                   ini.get_mutable_ref<cfg::mod_rdp::enable_session_probe>() = false;
                   return 1; // retry
            }
            return 2; // warn authentifier if possible before closing
        }
        else if (e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) {
            return 1; // retry
        }
        else if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
            ini.set<cfg::context::perform_automatic_reconnection>(true);
            return 1; // retry
        }
        else if (e.id == ERR_RAIL_NOT_ENABLED) {
            ini.get_mutable_ref<cfg::mod_rdp::use_native_remoteapp_capability>() = false;
            return 1; // retry
        }
        else if (e.id == ERR_RDP_SERVER_REDIR){
            return 3;
        }
        return 0;
    }

    void start_acl_activate(BackEvent_t session_reactor_signal, ModWrapper & mod_wrapper, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, ModuleManager & mm, SessionReactor & session_reactor, Authentifier & authentifier, ReportMessageApi & report_message, Fstat & fstat)
    {
        // authentifier never opened or closed by me (close box)
        try {
            // now is authentifier start time
            acl = std::make_unique<Acl>(
                ini, Session::acl_connect(ini.get<cfg::globals::authfile>(), (strcmp(ini.get<cfg::globals::host>().c_str(), "127.0.0.1") == 0)), now.tv_sec, cctx, rnd, fstat
            );
            const auto sck = acl->auth_trans.sck;
            fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
            authentifier.set_acl_serial(&acl->acl_serial);
            session_reactor_signal = BACK_EVENT_NEXT;
        }
        catch (...) {
            this->ini.set<cfg::context::auth_error_message>("No authentifier available");
            mod_wrapper.last_disconnect();
            this->last_module = true;
            session_reactor_signal = BACK_EVENT_STOP;
        }
    }

    void start_acl_running(BackEvent_t & session_reactor_signal, ModWrapper & mod_wrapper, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, ModuleManager & mm, SessionReactor & session_reactor, Authentifier & authentifier, ReportMessageApi & report_message, Fstat & fstat)
    {
        // authentifier never opened or closed by me (close box)
        try {
            // now is authentifier start time
            acl = std::make_unique<Acl>(
                ini, Session::acl_connect(ini.get<cfg::globals::authfile>(), (strcmp(ini.get<cfg::globals::host>().c_str(), "127.0.0.1") == 0)), now.tv_sec, cctx, rnd, fstat
            );
            const auto sck = acl->auth_trans.sck;
            fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
            authentifier.set_acl_serial(&acl->acl_serial);
            session_reactor_signal = BACK_EVENT_NEXT;
        }
        catch (...) {
            this->ini.set<cfg::context::auth_error_message>("No authentifier available");
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            this->last_module = true;
            session_reactor_signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
        }
    }

    BackEvent_t check_acl(ModuleManager & mm, Acl & acl,
        AuthApi & authentifier, ReportMessageApi & report_message, ModWrapper & mod_wrapper,
        time_t now, BackEvent_t signal, bool & has_user_activity)
    {
        const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            this->last_module = true;
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::session_out_time, language(this->ini)));
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            return ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
        }

        // Close by rejeted message received
        if (!this->ini.get<cfg::context::rejected>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
            LOG(LOG_INFO, "Close by Rejected message received : %s",
                this->ini.get<cfg::context::rejected>());
            this->ini.set_acl<cfg::context::rejected>("");
            this->last_module = true;
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            return ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
        }

        // Keep Alive
        if (acl.keepalive.check(now, this->ini)) {
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::miss_keepalive, language(this->ini)));
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            this->last_module = true;
            return ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
        }

        // Inactivity management

        if (acl.inactivity.check_user_activity(now, has_user_activity)) {
            this->last_module = true;
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::close_inactivity, language(this->ini)));
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            return ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
        }

        // Manage module (refresh or next)
        if (this->ini.changed_field_size()) {
            if (mm.connected) {
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
                    mm.new_mod(mod_wrapper, MODULE_INTERNAL_TRANSITION, authentifier, report_message);
                }
            }
            if (signal == BACK_EVENT_REFRESH) {
                signal = BACK_EVENT_NONE;
            }
        }
        else if (acl.acl_serial.remote_answer || signal == BACK_EVENT_RETRY_CURRENT) {
            acl.acl_serial.remote_answer = false;
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
            }
            else if ((signal == BACK_EVENT_NEXT)
                ||(signal == BACK_EVENT_RETRY_CURRENT)) {
                if (signal == BACK_EVENT_NEXT) {
                    LOG(LOG_INFO, "===========> MODULE_NEXT");
                }
                else {
                    assert(signal == BACK_EVENT_RETRY_CURRENT);
                    LOG(LOG_INFO, "===========> MODULE_RETRY_CURRENT");
                }

                ModuleIndex next_state = (signal == BACK_EVENT_NEXT)
                                       ? mm.next_module() : MODULE_RDP;

                if (next_state == MODULE_TRANSITORY) {
                    acl.acl_serial.remote_answer = false;
                    return signal;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    this->last_module = true;
                    mod_wrapper.last_disconnect();
                    if (ini.get<cfg::globals::enable_close_box>()) {
                        mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                    }
                    return ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    acl.keepalive.stop();
                }
                if (mod_wrapper.get_mod()) {
                    mod_wrapper.get_mod()->disconnect();
                }
                mod_wrapper.remove_mod();
                try {
                    mm.new_mod(mod_wrapper, next_state, authentifier, report_message);
                }
                catch (Error const& e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        // TODO : see STRMODULE_TRANSITORY
                        this->ini.set_acl<cfg::context::module>("transitory");

                        acl.acl_serial.remote_answer = false;
                        authentifier.disconnect_target();

                        acl.acl_serial.report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return BACK_EVENT_NEXT;
                    }

                    if ((e.id == ERR_RDP_SERVER_REDIR) 
                    && mm.ini.get<cfg::mod_rdp::server_redirection_support>()) {
                        acl.acl_serial.server_redirection_target();
                        acl.acl_serial.remote_answer = true;
                        return BACK_EVENT_NEXT;
                    }

                    throw;
                }
                if (!acl.keepalive.is_started() && mm.connected) {
                    acl.keepalive.start(now);
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

                        rdp_api* rdpapi = mm.get_rdp_api(mod_wrapper);

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

        if (mm.connected) {
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

        return signal;
    }


    bool front_up_and_running(BackEvent_t & session_reactor_signal, bool const front_is_set, Select& ioswitch, SessionReactor& session_reactor, BackEvent_t & signal, std::unique_ptr<Acl> & acl, timeval & now, const time_t start_time, Inifile& ini, ModuleManager & mm, ModWrapper & mod_wrapper, EndSessionWarning & end_session_warning, Front & front, Authentifier & authentifier, ReportMessageApi & report_message)
    {
        try {
            session_reactor.execute_timers(SessionReactor::EnableGraphics{true}, [&]() -> gdi::GraphicApi& {
                return mod_wrapper.get_graphic_wrapper();
            });
            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
        } catch (Error const& e) {
            switch (this->check_exception(e, ini)){
            case 3:
                if (ini.get<cfg::mod_rdp::server_redirection_support>()) {
                    set_server_redirection_target(ini, authentifier);
                    session_reactor_signal = BACK_EVENT_RETRY_CURRENT;
                }
                else {
                    LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                    signal = BackEvent_t(session_reactor_signal);
                    this->last_module = true;

                    if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                        this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                    }

                    mod_wrapper.last_disconnect();
                    if (ini.get<cfg::globals::enable_close_box>()) {
                        mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                    }
                    signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                    session_reactor_signal = signal;

                    if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                        return false;
                    }
                }
            break;
            case 2:
                if (acl){
                    ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));
                    authentifier.report("SESSION_PROBE_LAUNCH_FAILED", "");
                }
                else {
                    LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                    signal = BackEvent_t(session_reactor_signal);
                    this->last_module = true;
                    if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                        this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                    }
                    mod_wrapper.last_disconnect();
                    if (ini.get<cfg::globals::enable_close_box>()) {
                        mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                    }
                    signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                    session_reactor_signal = signal;
                    if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                        return false;
                    }
                }
            break;
            case 1:
                session_reactor_signal = BACK_EVENT_RETRY_CURRENT;
            break;
            default:
                LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                signal = BackEvent_t(session_reactor_signal);
                this->last_module = true;
                if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                    this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                }
                mod_wrapper.last_disconnect();
                if (ini.get<cfg::globals::enable_close_box>()) {
                    mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                }
                signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                session_reactor_signal = signal;
                if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                    return false;
                }
            break;
            }
        }

        session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
            return io_fd_isset(fd, ioswitch.rfds);
        });
        session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);

        // front event
        try {
            if (session_reactor.has_front_event()) {
                session_reactor.execute_callbacks(mod_wrapper.get_callback());
                session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
            }
            if (front_is_set) {
                front.rbuf.load_data(front.trans);
                while (front.rbuf.next(TpduBuffer::PDU))
                {
                    bytes_view tpdu = front.rbuf.current_pdu_buffer();
                    uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                    front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback());
                }
                session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
            }
        } catch (Error const& e) {
            // RemoteApp disconnection initiated by user
            // ERR_DISCONNECT_BY_USER == e.id
            if (// Can be caused by client disconnect.
                (e.id != ERR_X224_RECV_ID_IS_RD_TPDU) &&
                // Can be caused by client disconnect.
                (e.id != ERR_MCS_APPID_IS_MCS_DPUM) &&
                (e.id != ERR_RDP_HANDSHAKE_TIMEOUT) &&
                // Can be caused by wabwatchdog.
                (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
            }
            return false;
        } catch (...) {
            LOG(LOG_ERR, "Proxy data processing raised unknown error");
            return false;
        }

        // acl event
        try {
            if (front.state == Front::UP_AND_RUNNING) {
                // new value incoming from authentifier
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

                    if (this->ini.get<cfg::context::forcemodule>() && !mm.is_connected()) {
                        session_reactor_signal = BACK_EVENT_NEXT;
                        this->ini.set<cfg::context::forcemodule>(false);
                        // Do not send back the value to sesman.
                    }
                }

                try
                {
                    if (BACK_EVENT_NONE == session_reactor_signal) {
                        // Process incoming module trafic
                        auto& gd = mod_wrapper.get_graphic_wrapper();
                        session_reactor.execute_graphics([&ioswitch](int fd, auto& /*e*/){
                            return io_fd_isset(fd, ioswitch.rfds);
                        }, gd);
                        session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                    }
                }
                catch (Error const & e) {
                    switch (this->check_exception(e, ini)){
                    case 3:
                        if (ini.get<cfg::mod_rdp::server_redirection_support>()) {
                            set_server_redirection_target(ini, authentifier);
                            session_reactor_signal = BACK_EVENT_RETRY_CURRENT;
                        }
                        else {
                            LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                            signal = BackEvent_t(session_reactor_signal);
                            this->last_module = true;
                            if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                                this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                            }
                            mod_wrapper.last_disconnect();
                            if (ini.get<cfg::globals::enable_close_box>()) {
                                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                            }
                            signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                            session_reactor_signal = signal;

                            if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                                return false;
                            }
                        }
                    break;
                    case 2:
                        if (acl){
                            ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));
                            authentifier.report("SESSION_PROBE_LAUNCH_FAILED", "");
                        }
                        else {
                            LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                            signal = BackEvent_t(session_reactor_signal);
                            this->last_module = true;
                            if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                                this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                            }
                            mod_wrapper.last_disconnect();
                            if (ini.get<cfg::globals::enable_close_box>()) {
                                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                            }
                            signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                            session_reactor_signal = signal;
                            if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                                return false;
                            }
                        }
                    break;
                    case 1:
                        session_reactor_signal = BACK_EVENT_RETRY_CURRENT;
                    break;
                    default:
                        LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
                        signal = BackEvent_t(session_reactor_signal);
                        this->last_module = true;
                        if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                        }
                        mod_wrapper.last_disconnect();
                        if (ini.get<cfg::globals::enable_close_box>()) {
                            mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
                        }
                        signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
                        session_reactor_signal = signal;
                        if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                            return false;
                        }
                    break;
                    }
                }

                // Incoming data from ACL
                if (acl && (acl->auth_trans.has_pending_data() || io_fd_isset(acl->auth_trans.sck, ioswitch.rfds))) {
                    // authentifier received updated values
                    acl->acl_serial.receive();
                    if (!ini.changed_field_size()) {
                        session_reactor.execute_sesman(ini);
                        // TODO: check, should not change signal
                        session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                    }
                }

                const bool enable_osd = ini.get<cfg::globals::enable_osd>();
                if (enable_osd) {
                    const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
                    if (enddate && mod_wrapper.is_up_and_running()) {
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

                if (acl){
                    if (front.state == Front::UP_AND_RUNNING) {
                        signal = BackEvent_t(session_reactor_signal);
                        int i = 0;
                        do {
                            if (++i == 11) {
                                LOG(LOG_ERR, "loop event error");
                                break;
                            }
                            session_reactor_signal = BACK_EVENT_NONE;
                            if (signal == BACK_EVENT_STOP) {
                                session_reactor_signal = BACK_EVENT_STOP;
                                if (this->last_module) {
                                    authentifier.set_acl_serial(nullptr);
                                    acl.reset();
                                }
                                return false;
                            }
                            if (!this->last_module) {
                                signal = this->check_acl(mm, *acl,
                                    authentifier, authentifier, mod_wrapper,
                                    now.tv_sec, signal, front.has_user_activity
                                );
                            }
                            if (session_reactor_signal == BACK_EVENT_NONE) {
                                session_reactor_signal = signal;
                                break;
                            }
                            if (signal != BACK_EVENT_NONE) {
                                session_reactor_signal = signal;
                            }
                            else {
                                signal = BackEvent_t(session_reactor_signal);
                            }
                        } while (session_reactor_signal != BACK_EVENT_NONE);
                    }
                    if (this->last_module) {
                        authentifier.set_acl_serial(nullptr);
                        acl.reset();
                    }
                    return true;
                }
                else if ((!this->ini.is_asked<cfg::globals::nla_auth_user>())
                && this->ini.get<cfg::client::enable_nla>()) {
                    acl->acl_serial.send_acl_data();
                }
            }
            return true;
        } catch (Error const& e) {
            LOG(LOG_ERR, "Session::Session exception (2) = %s", e.errmsg());
            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier, report_message);
            }
            this->last_module = true;
            session_reactor_signal = ini.get<cfg::globals::enable_close_box>()?BACK_EVENT_NONE:BACK_EVENT_STOP;
            if (BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                return false;
            }
        }
        return true;
    }

public:
    Session(SocketTransport&& front_trans, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
    : ini(ini)
    {
        TRANSLATIONCONF.set_ini(&ini);
        std::string disconnection_message_error;

        const bool mem3blt_support = true;
        Authentifier authentifier(ini, cctx, to_verbose_flags(ini.get<cfg::debug::auth>()));

        SessionReactor session_reactor;
        BackEvent_t session_reactor_signal = BACK_EVENT_NONE;
//        void set_next_event(/*BackEvent_t*/int signal)
//        {
//            LOG(LOG_DEBUG, "SessionReactor::set_next_event %d", signal);
//            assert(!this->signal || this->signal == signal);
//            this->signal = signal;
//            // assert(is not already set)
//        }
                
        session_reactor.set_current_time(tvtime());
        Front front(
            session_reactor, front_trans, rnd, ini, cctx, authentifier,
            ini.get<cfg::client::fast_path>(), mem3blt_support
        );

        std::unique_ptr<Acl> acl;

        try {
            TimeSystem timeobj;
            
            // load font for internal pages
            Font glyphs = Font(app_path(AppPath::DefaultFontFile),
                ini.get<cfg::globals::spark_view_specific_glyph_width>());;

            // load theme for internal pages
            auto & theme_name = this->ini.get<cfg::internal_mod::theme>();
            LOG_IF(this->ini.get<cfg::debug::config>(), LOG_INFO, "LOAD_THEME: %s", theme_name);

            Theme theme;
            ::load_theme(theme, theme_name);

            ClientExecute rail_client_execute(session_reactor, front, front,
                                            front.client_info.window_list_caps,
                                            ini.get<cfg::debug::mod_internal>() & 1);


            windowing_api* winapi = nullptr;
            
            ModWrapper mod_wrapper(front, front.get_palette(), front, front.client_info, glyphs, theme, rail_client_execute, winapi, this->ini);

            ModFactory mod_factory(mod_wrapper, session_reactor, front.client_info, front, front, ini, glyphs, theme, rail_client_execute);
            EndSessionWarning end_session_warning;
            ModuleManager mm(end_session_warning, mod_factory, session_reactor, front, front.keymap, front.client_info, rail_client_execute, glyphs, theme, this->ini, cctx, rnd, timeobj);

            BackEvent_t signal       = BACK_EVENT_NONE;

            if (ini.get<cfg::debug::session>()) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(nullptr);
            if (ini.get<cfg::debug::performance>() & 0x8000) {
                this->write_performance_log(start_time);
            }

            bool run_session = true;

            using namespace std::chrono_literals;

            timeval now = tvtime();
            session_reactor.set_current_time(now);

            while (run_session) {
                timeval default_timeout = now;
                default_timeout.tv_sec += this->select_timeout_tv_sec;

                Select ioswitch(default_timeout);

                SckNoRead sck_no_read;

                if (front_trans.has_waiting_data()) {
                    LOG(LOG_INFO, "front_trans.has_waiting_data()");
                    ioswitch.set_write_sck(front_trans.sck);
                    sck_no_read.sck_front = front_trans.sck;
                }
                else {
                    ioswitch.set_read_sck(front_trans.sck);
                    if (mm.validator_fd > 0) {
                        ioswitch.set_read_sck(mm.validator_fd);
                    }
                }

                LOG(LOG_INFO, "mod_trans (1)");
                auto mod_trans = mod_wrapper.get_mod_transport();
                
                if (mod_trans && !mod_trans->has_pending_data()) {
                    LOG(LOG_INFO, "mod_trans don't have pending data (2)");
                    if (mod_trans->has_waiting_data()){
                        sck_no_read.sck_mod = mod_trans->sck;
                        ioswitch.set_write_sck(sck_no_read.sck_mod);
                    }
                    else if (sck_no_read.sck_front != INVALID_SOCKET) {
                        sck_no_read.sck_mod = mod_trans->sck;
                    }
                }

                if (acl) {
                    LOG(LOG_INFO, "read acl (1)");
                    ioswitch.set_read_sck(acl->auth_trans.sck);
                }

                // We should check we are able to write on acl socket
                if (front.state == Front::PRIMARY_AUTH_NLA) {
                    LOG(LOG_INFO, "primary auth nla (1) user=%s", this->ini.get<cfg::globals::nla_auth_user>());
                    if ((this->ini.is_asked<cfg::context::nla_password_hash>())
                        && this->ini.get<cfg::client::enable_nla>()) {
                        acl->acl_serial.send_acl_data();
                    }
                }

                if (front_trans.has_pending_data()
                || (mod_trans && mod_trans->has_pending_data())
                || (acl && acl->auth_trans.has_pending_data())){
                    ioswitch.immediate_wakeup(session_reactor.get_current_time());
                }


                session_reactor.for_each_fd(
                    SessionReactor::EnableGraphics{front.state == Front::UP_AND_RUNNING},
                    [&](int fd){
                        if (!sck_no_read.contains(fd)) {
                            ioswitch.set_read_sck(fd);
                        }
                    }
                );

                now = tvtime();
                session_reactor.set_current_time(now);
                ioswitch.set_timeout(
                        session_reactor.get_next_timeout(SessionReactor::EnableGraphics{front.state == Front::UP_AND_RUNNING}, ioswitch.get_timeout(now)));

                int num = ioswitch.select(now);

                if (num < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                    // EBADF: means fd has been closed (by me) or as already returned an error on another call
                    // EINVAL: invalid value in timeout (my fault again)
                    // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)

                    LOG(LOG_ERR, "Proxy data wait loop raised error %d : %s", errno, strerror(errno));
                    run_session = false;
                    continue;
                }

                {
                    if (ioswitch.is_set(sck_no_read.sck_mod)) {
                        mod_trans->send_waiting_data();
                    }

                    if (ioswitch.is_set(sck_no_read.sck_front)) {
                        front_trans.send_waiting_data();
                    }
                }

                now = tvtime();
                session_reactor.set_current_time(now);
                if (ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now.tv_sec);
                }

                switch (front.state) {
                case Front::UP_AND_RUNNING:
                {
                    if (!acl && !this->last_module) {
                        this->start_acl_running(session_reactor_signal, mod_wrapper, acl, cctx, rnd, now, ini, mm, session_reactor, authentifier, authentifier, fstat);
                    }
                    bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
                    run_session = this->front_up_and_running(session_reactor_signal, front_is_set, ioswitch, session_reactor, signal, acl, now, start_time, ini, mm, mod_wrapper, end_session_warning, front, authentifier, authentifier);
                    
                    if (!acl && BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                        run_session = false;
                    }
                }
                break;
                case Front::PRIMARY_AUTH_NLA:
                {
                    bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
                    if (!acl && !this->last_module) {
                        this->start_acl_activate(session_reactor_signal, mod_wrapper, acl, cctx, rnd, now, ini, mm, session_reactor, authentifier, authentifier, fstat);
                    }

                    session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                        return io_fd_isset(fd, ioswitch.rfds);
                    });
                    session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);

                    // Incoming data from ACL
                    if (acl && (acl->auth_trans.has_pending_data() || io_fd_isset(acl->auth_trans.sck, ioswitch.rfds))) {
                        // authentifier received updated values
                        acl->acl_serial.receive();
                        if (!ini.changed_field_size()) {
                            session_reactor.execute_sesman(ini);
                            // FIXME: check should not change signal
                            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                        }
                    }

                    // front event
                    try {
                        if (session_reactor.has_front_event()) {
                            session_reactor.execute_callbacks(mod_wrapper.get_callback());
                            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                        }

                        if (front_is_set) {
                            front.rbuf.load_data(front.trans);
                            while (front.rbuf.next(TpduBuffer::CREDSSP))
                            {
                                bytes_view tpdu = front.rbuf.current_pdu_buffer();
                                uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                                front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback());
                            }
                            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                        }
                    } catch (Error const& e) {
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU) &&
                            // Can be caused by client disconnect.
                            (e.id != ERR_MCS_APPID_IS_MCS_DPUM) &&
                            (e.id != ERR_RDP_HANDSHAKE_TIMEOUT) &&
                            // Can be caused by wabwatchdog.
                            (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        run_session = false;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                    }
                    if (!acl && BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                        run_session = false;
                    }
                }
                break;
                default:
                {
                    bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
                    session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                        return io_fd_isset(fd, ioswitch.rfds);
                    });
                    session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);

                    // front event
                    try {
                        if (session_reactor.has_front_event()) {
                            session_reactor.execute_callbacks(mod_wrapper.get_callback());
                            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                        }

                        if (front_is_set) {
                            front.rbuf.load_data(front.trans);
                            while (front.rbuf.next(TpduBuffer::PDU))
                            {
                                bytes_view tpdu = front.rbuf.current_pdu_buffer();
                                uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                                front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback());
                            }
                            session_reactor_signal = BackEvent_t(mod_wrapper.get_mod()->session_reactor_signal);
                        }
                    } catch (Error const& e) {
                        // RemoteApp disconnection initiated by user
                        // ERR_DISCONNECT_BY_USER == e.id
                        if (
                            // Can be caused by client disconnect.
                            (e.id != ERR_X224_RECV_ID_IS_RD_TPDU) &&
                            // Can be caused by client disconnect.
                            (e.id != ERR_MCS_APPID_IS_MCS_DPUM) &&
                            (e.id != ERR_RDP_HANDSHAKE_TIMEOUT) &&
                            // Can be caused by wabwatchdog.
                            (e.id != ERR_TRANSPORT_NO_MORE_DATA)) {
                            LOG(LOG_ERR, "Proxy data processing raised error %u : %s", e.id, e.errmsg(false));
                        }
                        run_session = false;
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                    }
                    if (!acl && BackEvent_t(session_reactor_signal) == BACK_EVENT_STOP) {
                        run_session = false;
                    }
                }
                break;
                }
            }
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
            LOG(LOG_INFO, "Session::Session other exception in Init");
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
            ini.get<cfg::client::ssl_cipher_list>().c_str(),
            ini.get<cfg::client::tls_min_level>(),
            ini.get<cfg::client::tls_max_level>(),
            ini.get<cfg::client::show_common_cipher_list>(),
        });
}

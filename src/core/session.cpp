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

        bool is_set_for_writing(int fd){
            return fd != INVALID_SOCKET && io_fd_isset(fd, this->wfds);
        };

        bool is_set_for_reading(int fd){
            return fd != INVALID_SOCKET && io_fd_isset(fd, this->rfds);
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

    bool last_module{false};

    Inifile & ini;

    time_t      perf_last_info_collect_time = 0;
    const pid_t perf_pid = getpid();
    File        perf_file = nullptr;

    static const time_t select_timeout_tv_sec = 3;

    void start_acl_activate(ModWrapper & mod_wrapper, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, Authentifier & authentifier, Fstat & fstat)
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
        }
        catch (...) {
            this->ini.set<cfg::context::auth_error_message>("No authentifier available");
            mod_wrapper.last_disconnect();
            this->last_module = true;
        }
    }

    void start_acl_running(ModWrapper & mod_wrapper, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, ModuleManager & mm, Authentifier & authentifier, Fstat & fstat)
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
        }
        catch (...) {
            this->ini.set<cfg::context::auth_error_message>("No authentifier available");
            mod_wrapper.last_disconnect();
            if (ini.get<cfg::globals::enable_close_box>()) {
                mm.new_mod_internal_close(mod_wrapper, authentifier);
            }
            this->last_module = true;
        }
    }

    bool close_box(ModuleManager & mm, std::unique_ptr<Acl> & acl, Authentifier & authentifier, ModWrapper & mod_wrapper, Inifile & ini)
    {
        this->last_module = true;
        mod_wrapper.last_disconnect();
        authentifier.set_acl_serial(nullptr);
        acl.reset();
        if (ini.get<cfg::globals::enable_close_box>()) {
            mm.new_mod_internal_close(mod_wrapper, authentifier);
            return true;
        }
        return false;
    }


    // Returns run_session value
    bool check_acl(ModuleManager & mm, std::unique_ptr<Acl> & acl,
        Authentifier & authentifier, ReportMessageApi & report_message, ModWrapper & mod_wrapper,
        time_t now, bool & has_user_activity)
    {
        LOG(LOG_INFO, "check_acl enddate"); 
        const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Close by enddate reached");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::session_out_time, language(this->ini)));
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
        }

        LOG(LOG_INFO, "check_acl rejected"); 
        // Close by rejeted message received
        if (!this->ini.get<cfg::context::rejected>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
            LOG(LOG_INFO, "Close by Rejected message received : %s", this->ini.get<cfg::context::rejected>());
            this->ini.set_acl<cfg::context::rejected>("");
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
        }

        LOG(LOG_INFO, "check_acl keepalive"); 
        // Keep Alive
        if (acl->keepalive.check(now, this->ini)) {
            LOG(LOG_INFO, "Close by Missed keepalive");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::miss_keepalive, language(this->ini)));
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
        }

        LOG(LOG_INFO, "check_acl inactivity"); 
        // Inactivity management
        if (acl->inactivity.check_user_activity(now, has_user_activity)) {
            LOG(LOG_INFO, "Close by user Inactivity");
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::close_inactivity, language(this->ini)));
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
        }

        BackEvent_t signal = mod_wrapper.get_mod()->get_mod_signal();

        auto signal_name = [](BackEvent_t sig) {
            return 
            sig == BACK_EVENT_NONE?"BACK_EVENT_NONE"
           :sig == BACK_EVENT_NEXT?"BACK_EVENT_NEXT"
           :sig == BACK_EVENT_REFRESH?"BACK_EVENT_REFRESH"
           :sig == BACK_EVENT_STOP?"BACK_EVENT_STOP"
           :sig == BACK_EVENT_RETRY_CURRENT?"BACK_EVENT_RETRY_CURRENT"
           :"BACK_EVENT_UNKNOWN";
        };
        LOG(LOG_INFO, "check_acl mod_signal=%s", signal_name(signal)); 

        if (!acl->keepalive.is_started() && mm.connected) {
            acl->keepalive.start(now);
        }

        // There are modified fields to send to sesman
        LOG(LOG_INFO, "check_acl data to send to sesman"); 
        if (this->ini.changed_field_size()) {
            if (mm.connected) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                acl->acl_serial.send_acl_data();
                return true;
            }
            switch (signal){
            case BACK_EVENT_RETRY_CURRENT:
            break;
            case BACK_EVENT_STOP:
            break;
            case BACK_EVENT_NONE:
            break;
            case BACK_EVENT_NEXT:
                acl->acl_serial.remote_answer = false;
                acl->acl_serial.send_acl_data();
                mod_wrapper.remove_mod();
                mm.new_mod(mod_wrapper, MODULE_INTERNAL_TRANSITION, authentifier, report_message);
            break;
            case BACK_EVENT_REFRESH:
                acl->acl_serial.remote_answer = false;
                acl->acl_serial.send_acl_data();
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
            break;
            }
            return true;
        }
        if (acl->acl_serial.remote_answer || signal == BACK_EVENT_RETRY_CURRENT) {
            LOG(LOG_INFO, "check_acl remote_answer signal=%s", signal_name(signal)); 
            acl->acl_serial.remote_answer = false;
            switch (signal){
            default:
            case BACK_EVENT_NONE:
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
            return true;
            case BACK_EVENT_REFRESH:
                LOG(LOG_INFO, "Remote Answer REFRESH");
                // should not happen
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
            return true;
            case BACK_EVENT_NEXT:
            {
                LOG(LOG_INFO, "Remote Answer, current module ask NEXT");
                ModuleIndex next_state = mm.next_module();
                if (next_state == MODULE_TRANSITORY) {
                    LOG(LOG_INFO, "check_acl TRANSITORY signal=%s", signal_name(signal)); 
                    acl->acl_serial.remote_answer = false;
                    mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NEXT);
                    return true;
                }
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    LOG(LOG_INFO, "check_acl is INTERNAL CLOSE signal=%s", signal_name(signal)); 
                    return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    LOG(LOG_INFO, "check_acl is INTERNAL CLOSE BACK signal=%s", signal_name(signal)); 
                    acl->keepalive.stop();
                }
                mm.new_mod(mod_wrapper, next_state, authentifier, report_message);
            }
            return true;
            case BACK_EVENT_RETRY_CURRENT:
            {
                LOG(LOG_INFO, "Remote Answer, current module ask RETRY");
                mod_wrapper.remove_mod();
                mm.new_mod(mod_wrapper, MODULE_RDP, authentifier, report_message);
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_NONE);
            }
            return true;
            }

            if (!this->ini.get<cfg::context::disconnect_reason>().empty()) {
                acl->acl_serial.manager_disconnect_reason = this->ini.get<cfg::context::disconnect_reason>();
                this->ini.get_mutable_ref<cfg::context::disconnect_reason>().clear();
                this->ini.set_acl<cfg::context::disconnect_reason_ack>(true);
            }
            else if (!this->ini.get<cfg::context::auth_command>().empty()) {
                if (!::strcasecmp(this->ini.get<cfg::context::auth_command>().c_str(), "rail_exec")) {
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
        return true;
    }

    bool end_session_exception(Error const& e, std::unique_ptr<Acl> & acl, ModuleManager & mm, ModWrapper & mod_wrapper, Authentifier & authentifier, Inifile & ini) {

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
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_RETRY_CURRENT);
                return true;
            }
            LOG(LOG_ERR, "Session::Session Exception (1) = %s", e.errmsg());
            if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
            }
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
        }
        else if (e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) {
            LOG(LOG_INFO, "====> Retry SESSION_PROBE_DISCONNECTION_RECONNECTION");
            mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_RETRY_CURRENT);
            return true;
        }
        else if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
            LOG(LOG_INFO, "====> Retry AUTOMATIC_RECONNECTION_REQUIRED");
            ini.set<cfg::context::perform_automatic_reconnection>(true);
            mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_RETRY_CURRENT);
            return true;
        }
        else if (e.id == ERR_RAIL_NOT_ENABLED) {
            LOG(LOG_INFO, "====> Retry witout Native remoteapp capability");
            ini.get_mutable_ref<cfg::mod_rdp::use_native_remoteapp_capability>() = false;
            mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_RETRY_CURRENT);
            return true;
        }
        else if (e.id == ERR_RDP_SERVER_REDIR){
            if (ini.get<cfg::mod_rdp::server_redirection_support>()) {
                LOG(LOG_INFO, "====> Retry Server redirection");
                set_server_redirection_target(ini, authentifier);
                mod_wrapper.get_mod()->set_mod_signal(BACK_EVENT_RETRY_CURRENT);
                return true;
            }
            else {
                LOG(LOG_ERR, "Start Session Failed on forbidden redirection = %s", e.errmsg());
                if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
                    this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
                }
                return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);  
            }
        }
        LOG(LOG_ERR, "Start Session Failed = %s", e.errmsg());
        if (ERR_RAIL_LOGON_FAILED_OR_WARNING != e.id) {
            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
        }
        return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
    }


    bool front_close_box(bool const front_is_set, Select& ioswitch, SessionReactor& session_reactor, GraphicFdContainer & graphic_fd_events_, TimerContainer& timer_events_, GraphicTimerContainer & graphic_timer_events_, CallbackEventContainer & front_events_, ModWrapper & mod_wrapper, Front & front, SesmanInterface & acl_cb)
    {
        bool run_session = true;
        try {
            session_reactor.execute_timers(graphic_fd_events_,
                                           timer_events_,
                                           graphic_timer_events_,
                                           EnableGraphics{true}, [&]() -> gdi::GraphicApi& {
                return mod_wrapper.get_graphic_wrapper();
            });
            session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                return ioswitch.is_set_for_reading(fd);
            });
            if (!front_events_.is_empty()) {
                front_events_.exec_action(mod_wrapper.get_callback());
            }
            if (front_is_set) {
                front.rbuf.load_data(front.trans);
                while (front.rbuf.next(TpduBuffer::PDU))
                {
                    bytes_view tpdu = front.rbuf.current_pdu_buffer();
                    uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                    front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback(), acl_cb);
                }
            }
            BackEvent_t signal = mod_wrapper.get_mod()->get_mod_signal();
            if ((signal == BACK_EVENT_STOP)||(signal==BACK_EVENT_NEXT)){
                run_session = false;
            }

        } catch (Error const& e) {
            run_session = false;
        }
        return run_session;
    }

    void rt_display(Inifile & ini, ModuleManager & mm, ModWrapper & mod_wrapper, Front & front)
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

            if (this->ini.get<cfg::context::forcemodule>() && !mm.is_connected()) {
                this->ini.set<cfg::context::forcemodule>(false);
                // Do not send back the value to sesman.
            }
        }
    }

    bool module_sequencing(ModuleManager & mm, std::unique_ptr<Acl> & acl,
        Authentifier & authentifier, ModWrapper & mod_wrapper,
        timeval now, Front & front)
    {
        LOG(LOG_INFO, "module_sequencing"); 
        int i = 0;
        do {
            if (++i == 11) {
                LOG(LOG_INFO, "module_sequencing: emergency exit"); 
                break;
            }
            if (!this->last_module) {
                LOG(LOG_INFO, "module_sequencing: not last module"); 
                bool run_session = this->check_acl(mm, acl,
                    authentifier, authentifier, mod_wrapper,
                    now.tv_sec, front.has_user_activity
                );
                if (!run_session) {
                    return false;
                }
            }
        } while (mod_wrapper.get_mod()->get_mod_signal() != BACK_EVENT_NONE);
        return true;
    }

    bool front_up_and_running(bool const front_is_set, Select& ioswitch, 
                              SessionReactor& session_reactor,
                              GraphicFdContainer & graphic_fd_events_,
                              TimerContainer& timer_events_,
                              GraphicEventContainer& graphic_events_,
                              GraphicTimerContainer graphic_timer_events_, CallbackEventContainer & front_events_, SesmanEventContainer & sesman_events_, std::unique_ptr<Acl> & acl, timeval & now, const time_t start_time, Inifile& ini, ModuleManager & mm, ModWrapper & mod_wrapper, EndSessionWarning & end_session_warning, Front & front, Authentifier & authentifier, SesmanInterface & acl_cb)
    {
        LOG(LOG_INFO, "front_up_and_running : execute_timers");
        try {
            session_reactor.execute_timers(graphic_fd_events_, timer_events_, graphic_timer_events_, EnableGraphics{true}, [&]() -> gdi::GraphicApi& {
                return mod_wrapper.get_graphic_wrapper();
            });

            LOG(LOG_INFO, "front_up_and_running : execute_events");
            session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                return ioswitch.is_set_for_reading(fd);
            });

        } catch (Error const& e) {
            if (false == end_session_exception(e, acl, mm, mod_wrapper, authentifier, ini)) {
                return false;
            }
        }


        // front event
        try {
            if (!front_events_.is_empty()) {
                LOG(LOG_INFO, "front_up_and_running:  has_front_events");
                front_events_.exec_action(mod_wrapper.get_callback());
            }
            if (front_is_set) {
                LOG(LOG_INFO, "front_up_and_running front is set");
                front.rbuf.load_data(front.trans);
                while (front.rbuf.next(TpduBuffer::PDU))
                {
                    bytes_view tpdu = front.rbuf.current_pdu_buffer();
                    uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                    front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback(), acl_cb);
                }
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
            try {

            // new value incoming from authentifier
            LOG(LOG_INFO, "--------------------- rt_display()");
            this->rt_display(ini, mm, mod_wrapper, front);

            if (BACK_EVENT_NONE == mod_wrapper.get_mod()->get_mod_signal()) {
                // Process incoming module trafic
                auto& gd = mod_wrapper.get_graphic_wrapper();
                session_reactor.execute_graphics(graphic_fd_events_, graphic_events_, [&ioswitch](int fd, auto& /*e*/){
                    return ioswitch.is_set_for_reading(fd);
                }, gd);
            }

                // Incoming data from ACL
                LOG(LOG_INFO, "-------------------------- check acl pending");
                if (acl && (acl->auth_trans.has_pending_data() || ioswitch.is_set_for_reading(acl->auth_trans.sck))) {
                    // authentifier received updated values
                    LOG(LOG_INFO, "acl pending");
                    acl->acl_serial.receive();
                    LOG(LOG_INFO, "data received from acl");
                    if (!ini.changed_field_size()) {
                        LOG(LOG_INFO, "sesman event");
                        sesman_events_.exec_action(ini);
                    }
                }

                LOG(LOG_INFO, "-------------------------- enable OSD");
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

                LOG(LOG_INFO, "-------------------------- module sequencing");
                return this->module_sequencing(mm, acl, authentifier, mod_wrapper, now, front);
            } catch (Error const& e) {
                LOG(LOG_ERR, "Exception in sequencing = %s", e.errmsg());
                if (false == end_session_exception(e, acl, mm, mod_wrapper, authentifier, ini)) {
                    return false;
                }
            }
        } catch (Error const& e) {
            LOG(LOG_ERR, "Session::Session exception (2) = %s", e.errmsg());
            this->ini.set<cfg::context::auth_error_message>(local_err_msg(e, language(ini)));
            return this->close_box(mm, acl, authentifier, mod_wrapper, this->ini);
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
        GraphicFdContainer graphic_fd_events_;
        TimerContainer timer_events_;
        GraphicEventContainer graphic_events_;
        CallbackEventContainer front_events_;
        SesmanEventContainer sesman_events_;
        GraphicTimerContainer graphic_timer_events_;

//        void set_next_event(/*BackEvent_t*/int signal)
//        {
//            LOG(LOG_DEBUG, "set_next_event %d", signal);
//            assert(!this->signal || this->signal == signal);
//            this->signal = signal;
//            // assert(is not already set)
//        }
        
        session_reactor.set_current_time(tvtime());
        Front front(
            session_reactor, timer_events_, front_events_, front_trans, rnd, ini, cctx, authentifier,
            ini.get<cfg::client::fast_path>(), mem3blt_support
        );
        SesmanInterface acl_cb(ini);

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

            ClientExecute rail_client_execute(session_reactor, timer_events_, front, front,
                                            front.client_info.window_list_caps,
                                            ini.get<cfg::debug::mod_internal>() & 1);


            windowing_api* winapi = nullptr;
            
            ModWrapper mod_wrapper(front, front.get_palette(), front, front.client_info, glyphs, theme, rail_client_execute, winapi, this->ini);

            ModFactory mod_factory(mod_wrapper, session_reactor, graphic_fd_events_, timer_events_, graphic_events_, graphic_timer_events_, sesman_events_, front.client_info, front, front, ini, glyphs, theme, rail_client_execute);
            EndSessionWarning end_session_warning;
            ModuleManager mm(end_session_warning, mod_factory, session_reactor, graphic_fd_events_, timer_events_, graphic_events_, sesman_events_, front, front.keymap, front.client_info, rail_client_execute, glyphs, theme, this->ini, cctx, rnd, timeobj);

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
                LOG(LOG_INFO, "while loop beginning of loop");

                timeval default_timeout = now;
                default_timeout.tv_sec += this->select_timeout_tv_sec;

                Select ioswitch(default_timeout);

                int sck_no_read_sck_front = INVALID_SOCKET;
                int sck_no_read_sck_mod = INVALID_SOCKET;

                LOG(LOG_INFO, "front_trans.has_waiting_data()");
                if (front_trans.has_waiting_data()) {
                    LOG(LOG_INFO, "front_trans.has_waiting_data()");
                    ioswitch.set_write_sck(front_trans.sck);
                    sck_no_read_sck_front = front_trans.sck;
                }
                else {
                    ioswitch.set_read_sck(front_trans.sck);
                    if (mm.validator_fd > 0) {
                        ioswitch.set_read_sck(mm.validator_fd);
                    }
                }

                LOG(LOG_INFO, "mod_trans (1)");
                auto mod_trans = mod_wrapper.get_mod_transport();

                LOG(LOG_INFO, "mod_trans->has_pending_data()");
                if (mod_trans && !mod_trans->has_pending_data()) {
                    LOG(LOG_INFO, "mod_trans don't have pending data (2)");
                    if (mod_trans->has_waiting_data()){
                        sck_no_read_sck_mod = mod_trans->sck;
                        ioswitch.set_write_sck(sck_no_read_sck_mod);
                    }
                    else if (sck_no_read_sck_front != INVALID_SOCKET) {
                        sck_no_read_sck_mod = mod_trans->sck;
                    }
                }

                LOG(LOG_INFO, "acl check");
                if (acl) {
                    LOG(LOG_INFO, "read acl (1)");
                    ioswitch.set_read_sck(acl->auth_trans.sck);
                }

//                // We should check we are able to write on acl socket
//                if (front.state == Front::PRIMARY_AUTH_NLA) {
//                    LOG(LOG_INFO, "primary auth nla (1) user=%s", this->ini.get<cfg::globals::nla_auth_user>());
//                    if ((this->ini.is_asked<cfg::context::nla_password_hash>())
//                        && this->ini.get<cfg::client::enable_nla>()) {
//                        acl->acl_serial.send_acl_data();
//                    }
//                }

                LOG(LOG_INFO, "front_trans check pending data");
                if (front_trans.has_pending_data()
                || (mod_trans && mod_trans->has_pending_data())
                || (acl && acl->auth_trans.has_pending_data())){
                    ioswitch.immediate_wakeup(session_reactor.get_current_time());
                }


                auto g = [sck_no_read_sck_front,sck_no_read_sck_mod,&ioswitch](int fd, auto& /*top*/){
                    assert(fd != -1);
                    if ((sck_no_read_sck_front != fd)
                    &&  (sck_no_read_sck_mod != fd)) {
                        ioswitch.set_read_sck(fd);
                    }
                };
                session_reactor.fd_events_.for_each(g);
                if (front.state == Front::FRONT_UP_AND_RUNNING) {
                    graphic_fd_events_.for_each(g);
                }

                now = tvtime();
                session_reactor.set_current_time(now);
                ioswitch.set_timeout(
                        session_reactor.get_next_timeout(
                            graphic_fd_events_,
                            timer_events_,
                            graphic_events_,
                            graphic_timer_events_,
                            front_events_,
                            EnableGraphics{front.state == Front::FRONT_UP_AND_RUNNING},
                            ioswitch.get_timeout(now)));

                LOG(LOG_INFO, "select");
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
                    LOG(LOG_INFO, "send mod data waiting to be sent");
                    if (ioswitch.is_set_for_writing(sck_no_read_sck_mod)) {
                        mod_trans->send_waiting_data();
                    }

                    LOG(LOG_INFO, "send front data waiting to be sent");
                    if (ioswitch.is_set_for_writing(sck_no_read_sck_front)) {
                        front_trans.send_waiting_data();
                    }
                }

                now = tvtime();
                session_reactor.set_current_time(now);
                if (ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now.tv_sec);
                }

                LOG(LOG_INFO, "front is set flag");
                bool const front_is_set = front_trans.has_pending_data() || ioswitch.is_set_for_reading(front_trans.sck);
                
                LOG(LOG_INFO, "acl is set flag");
                bool acl_is_set = false; //(acl) && ioswitch.is_set_for_reading(acl->auth_trans.sck);
                if (acl){
                    if (ioswitch.is_set_for_reading(acl->auth_trans.sck)){
                        acl_is_set = true;
                    }
                }

                LOG(LOG_INFO, "mod is set flag");
                bool mod_is_set = false;
                if (mod_wrapper.has_mod()) {
                    LOG(LOG_INFO, "mod is set flag ?");
                    if (ioswitch.is_set_for_reading(sck_no_read_sck_mod)){
                        mod_is_set = true;
                    }
                }

                LOG(LOG_INFO, "SELECT LOOP: %s front_data=%s %s acl_data=%s mod=%s [%s]",
                    front.state_name(),
                    front_is_set?"yes":"no",
                    acl?"ACL Connected":"ACL not connected",
                    acl_is_set?"yes":"no",
                    mod_is_set?"yes":"no",
                    mod_wrapper.module_name());

                switch (front.state) {
                default:
                // Design a State to ensure ACL Start
//                if (acl){
//                    ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));
//                    authentifier.report("SESSION_PROBE_LAUNCH_FAILED", "");
//                }
                {
                    bool const front_is_set = front_trans.has_pending_data() || ioswitch.is_set_for_reading(front_trans.sck);

                    session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                        return ioswitch.is_set_for_reading(fd);
                    });

                    // front event
                    try {
                        if (!front_events_.is_empty()) {
                            front_events_.exec_action(mod_wrapper.get_callback());
                        }

                        if (front_is_set) {
                            front.rbuf.load_data(front.trans);
                            while (front.rbuf.next(TpduBuffer::PDU))
                            {
                                bytes_view tpdu = front.rbuf.current_pdu_buffer();
                                uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                                front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback(), acl_cb);
                            }
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
                }
                break;
                case Front::FRONT_UP_AND_RUNNING:
                {
                    if (!acl && !this->last_module) {
                        LOG(LOG_INFO, "start_acl_running");
                        this->start_acl_running(mod_wrapper, acl, cctx, rnd, now, ini, mm, authentifier, fstat);
                        if (this->last_module && !ini.get<cfg::globals::enable_close_box>()) {
                            run_session = false;
                            continue;
                        }
                    }

                    if (!acl_cb.auth_info_sent){
                        LOG(LOG_INFO, "Sending ACL auth_info");
                        acl_cb.set_acl_screen_info();
                        acl_cb.set_acl_auth_info();
                        if (this->ini.changed_field_size()) {
                            LOG(LOG_INFO, "ACL Data to send");                            
                            acl->acl_serial.send_acl_data();
                            continue;
                        }
                    }

                    bool const front_is_set = front_trans.has_pending_data() || ioswitch.is_set_for_reading(front_trans.sck);

                    if (this->last_module){
                        run_session = this->front_close_box(front_is_set, ioswitch, session_reactor, graphic_fd_events_, timer_events_, graphic_timer_events_, front_events_, mod_wrapper, front, acl_cb);
                        continue;
                    }

                    run_session = this->front_up_and_running(front_is_set, ioswitch, session_reactor, graphic_fd_events_, timer_events_, graphic_events_, graphic_timer_events_, front_events_, sesman_events_, acl, now, start_time, ini, mm, mod_wrapper, end_session_warning, front, authentifier, acl_cb);
                }
                break;
                case Front::PRIMARY_AUTH_NLA:
                {
                    bool const front_is_set = front_trans.has_pending_data() || ioswitch.is_set_for_reading(front_trans.sck);
                    if (!acl && !this->last_module) {
                        this->start_acl_activate(mod_wrapper, acl, cctx, rnd, now, ini, authentifier, fstat);
                    }

                    session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                        return ioswitch.is_set_for_reading(fd);
                    });

                    // Incoming data from ACL
                    if (acl && (acl->auth_trans.has_pending_data() || ioswitch.is_set_for_reading(acl->auth_trans.sck))) {
                        LOG(LOG_INFO, "ACL pending");
                        // authentifier received updated values
                        acl->acl_serial.receive();
                        if (!ini.changed_field_size()) {
                            LOG(LOG_INFO, "sesman event");
                            sesman_events_.exec_action(ini);;
                        }
                    }

                    // front event
                    try {
                        if (!front_events_.is_empty()) {
                            front_events_.exec_action(mod_wrapper.get_callback());
                        }
                        if (front_is_set) {
                            front.rbuf.load_data(front.trans);
                            while (front.rbuf.next(TpduBuffer::CREDSSP))
                            {
                                bytes_view tpdu = front.rbuf.current_pdu_buffer();
                                uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                                front.incoming(tpdu, current_pdu_type, mod_wrapper.get_callback(), acl_cb);
                            }
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
                    if (!acl) {
                        run_session = false;
                    }
                }
                break;
                } // switch
                LOG(LOG_INFO, "while loop run_session=%s", run_session?"true":"false");
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

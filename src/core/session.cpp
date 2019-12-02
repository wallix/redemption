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


    Inifile & ini;

    time_t      perf_last_info_collect_time = 0;
    const pid_t perf_pid = getpid();
    File        perf_file = nullptr;

    static const time_t select_timeout_tv_sec = 3;


    void invoke_close_box(Error const& e, BackEvent_t & signal, Inifile& ini, ModuleManager & mm, SessionReactor & session_reactor, Authentifier & authentifier, Front & front, bool & run_session)
    {
        signal = BackEvent_t(session_reactor.signal);

        const char * auth_error_message = ((ERR_RAIL_LOGON_FAILED_OR_WARNING == e.id) ? nullptr : local_err_msg(e, language(ini)));

        auto const enable_close_box  = ini.get<cfg::globals::enable_close_box>();
        ini.set<cfg::globals::enable_close_box>(enable_close_box && front.state == Front::UP_AND_RUNNING);

        mm.invoke_close_box(
            auth_error_message,
            signal, authentifier, authentifier);
        session_reactor.signal = signal;

        ini.set<cfg::globals::enable_close_box>(enable_close_box);

        if (BackEvent_t(session_reactor.signal) == BACK_EVENT_STOP) {
            run_session = false;
        }
    }


    void check_exception(Error const& e, BackEvent_t & signal, Inifile& ini, ModuleManager & mm, SessionReactor & session_reactor, Authentifier & authentifier, Front & front, bool & run_session, const Acl * acl)
    {
        if ((e.id == ERR_SESSION_PROBE_LAUNCH) ||
            (e.id == ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE) ||
            (e.id == ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS) ||
            (e.id == ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON) ||
            (e.id == ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE) ||
            (e.id == ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE) ||
            (e.id == ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET) ||
            (e.id == ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS) ||
            (e.id == ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED) ||
            (e.id == ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG) ||
            (e.id == ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG)) {
            if (ini.get<cfg::mod_rdp::session_probe_on_launch_failure>() ==
                SessionProbeOnLaunchFailure::retry_without_session_probe) {
                   ini.get_ref<cfg::mod_rdp::enable_session_probe>() = false;

                session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
            }
            else if (acl) {
                ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(ini)));

                authentifier.report("SESSION_PROBE_LAUNCH_FAILED", "");
            }
            else {
                LOG(LOG_ERR, "Session::Session exception (1) = %s", e.errmsg());
                this->invoke_close_box(e, signal, ini, mm, session_reactor, authentifier, front, run_session);
            }
        }
        else if ((e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) ||
                 (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED)) {
            if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
                ini.set<cfg::context::perform_automatic_reconnection>(true);
            }

            session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
        }
        else if (e.id == ERR_RAIL_NOT_ENABLED) {
            ini.get_ref<cfg::mod_rdp::use_native_remoteapp_capability>() = false;

            session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
        }
        else if ((e.id == ERR_RDP_SERVER_REDIR) &&
                 ini.get<cfg::mod_rdp::server_redirection_support>()) {
            set_server_redirection_target(ini, authentifier);
            session_reactor.set_next_event(BACK_EVENT_RETRY_CURRENT);
        }
        else {
            LOG(LOG_ERR, "Session::Session exception (2) = %s", e.errmsg());
            this->invoke_close_box(e, signal, ini, mm, session_reactor, authentifier, front, run_session);
        }
    }

    void start_acl(std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, Inifile& ini, ModuleManager & mm, SessionReactor & session_reactor, Authentifier & authentifier, BackEvent_t signal, Fstat & fstat)
    {
        if (!acl) {
            if (!mm.last_module) {
                // authentifier never opened or closed by me (close box)
                try {
                    // now is authentifier start time
                    acl = std::make_unique<Acl>(
                        ini, Session::acl_connect(ini.get<cfg::globals::authfile>()), now.tv_sec, cctx, rnd, fstat
                    );
                    const auto sck = acl->auth_trans.sck;
                    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
                    authentifier.set_acl_serial(&acl->acl_serial);
                    session_reactor.set_next_event(BACK_EVENT_NEXT);
                }
                catch (...) {
                    signal = BackEvent_t(session_reactor.signal);
                    session_reactor.signal = 0;
                    mm.invoke_close_box("No authentifier available", signal, authentifier, authentifier);
                    if (!session_reactor.signal || signal) {
                        session_reactor.signal = signal;
                    }
                }
            }
        }
    }


    void front_starting(bool & run_session, SocketTransport& front_trans, Select& ioswitch, SessionReactor& session_reactor, BackEvent_t & signal, BackEvent_t & front_signal, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, const time_t start_time, Inifile& ini, ModuleManager & mm, Front & front, Authentifier & authentifier, Fstat & fstat)
    {
        SessionReactor::EnableGraphics enable_graphics{false};

        if (!acl) {
            this->start_acl(acl, cctx, rnd, now, ini, mm, session_reactor, authentifier, signal, fstat);
        }

        try {
            session_reactor.execute_timers(enable_graphics, [&]() -> gdi::GraphicApi& {
                return mm.get_graphic_wrapper();
            });
        } catch (Error const& e) {
            this->check_exception(e, signal, ini, mm, session_reactor, authentifier, front, run_session, acl.get());
        }

        session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
            return io_fd_isset(fd, ioswitch.rfds);
        });

        // front event
        try {
            if (session_reactor.has_front_event()) {
                session_reactor.execute_callbacks(mm.get_callback());
            }
            bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
            if (front_is_set) {
                front.rbuf.load_data(front.trans);
                while (front.rbuf.next(front.is_in_nla()?(TpduBuffer::CREDSSP):(TpduBuffer::PDU)))
                {
                    bytes_view tpdu = front.rbuf.current_pdu_buffer();
                    uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                    front.incoming(tpdu, current_pdu_type, mm.get_callback());
                }
            }
        } catch (Error const& e) {
            if (ERR_DISCONNECT_BY_USER == e.id) {
                front_signal = BACK_EVENT_NEXT;
            }
            else {
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
            }
        } catch (...) {
            LOG(LOG_ERR, "Proxy data processing raised unknown error");
            run_session = false;
        }

        if (run_session == false){
            return;
        }

        // acl event
        try {
            if (front.state == Front::UP_AND_RUNNING) {
                // new value incoming from authentifier
                if (ini.check_from_acl()) {
                    auto const rt_status = front.set_rt_display(ini.get<cfg::video::rt_display>());

                    if (ini.get<cfg::client::enable_osd_4_eyes>()) {
                        Translator tr(language(ini));
                        switch (rt_status) {
                            case Capture::RTDisplayResult::Enabled:
                                mm.osd_message(tr(trkeys::enable_rt_display).to_string(), true);
                                break;
                            case Capture::RTDisplayResult::Disabled:
                                mm.osd_message(tr(trkeys::disable_rt_display).to_string(), true);
                                break;
                            case Capture::RTDisplayResult::Unchanged:
                                break;
                        }
                    }

                    mm.check_module();
                }

                try
                {
                    if (BACK_EVENT_NONE == session_reactor.signal) {
                        // Process incoming module trafic
                        auto& gd = mm.get_graphic_wrapper();
                        session_reactor.execute_graphics([&ioswitch](int fd, auto& /*e*/){
                            return io_fd_isset(fd, ioswitch.rfds);
                        }, gd);
                    }
                }
                catch (Error const & e) {
                    this->check_exception(e, signal, ini, mm, session_reactor, authentifier, front, run_session, acl.get());
                }

                // Incoming data from ACL
                if (acl && (acl->auth_trans.has_pending_data() || io_fd_isset(acl->auth_trans.sck, ioswitch.rfds))) {
                    // authentifier received updated values
                    acl->acl_serial.receive();
                    if (!ini.changed_field_size()) {
                        session_reactor.execute_sesman(ini);
                    }
                }

                const bool enable_osd = ini.get<cfg::globals::enable_osd>();
                if (enable_osd) {
                    const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
                    if (enddate && mm.is_up_and_running()) {
                        mm.update_end_session_warning(start_time, static_cast<time_t>(enddate), now.tv_sec);
                    }
                }

                if (acl){
                    if (front.state == Front::UP_AND_RUNNING) {
                        signal = BackEvent_t(session_reactor.signal);
                        int i = 0;
                        do {
                            if (++i == 11) {
                                LOG(LOG_ERR, "loop event error");
                                break;
                            }
                            session_reactor.signal = 0;
                            run_session = acl->acl_serial.check(
                                authentifier, authentifier, mm,
                                now.tv_sec, signal, front_signal, front.has_user_activity
                            );
                            if (!session_reactor.signal) {
                                session_reactor.signal = signal;
                                break;
                            }

                            if (signal) {
                                session_reactor.signal = signal;
                            }
                            else {
                                signal = BackEvent_t(session_reactor.signal);
                            }
                        } while (session_reactor.signal);
                    }
                    else if (BackEvent_t(session_reactor.signal) == BACK_EVENT_STOP) {
                        run_session = false;
                    }
                    if (mm.last_module) {
                        authentifier.set_acl_serial(nullptr);
                        acl.reset();
                    }
                }
                else if ((!this->ini.is_asked<cfg::globals::nla_auth_user>())
                && this->ini.get<cfg::client::enable_nla>()) {
                    acl->acl_serial.send_acl_data();
                }
            }
        } catch (Error const& e) {
            LOG(LOG_ERR, "Session::Session exception (2) = %s", e.errmsg());
            signal = BackEvent_t(session_reactor.signal);
            mm.invoke_close_box(
                local_err_msg(e, language(ini)),
                signal, authentifier, authentifier);
            session_reactor.signal = signal;

            if (BackEvent_t(session_reactor.signal) == BACK_EVENT_STOP) {
                run_session = false;
            }
        }
    }

    void front_up_and_running(bool & run_session, SocketTransport& front_trans, Select& ioswitch, SessionReactor& session_reactor, BackEvent_t & signal, BackEvent_t & front_signal, std::unique_ptr<Acl> & acl, CryptoContext& cctx, Random& rnd, timeval & now, const time_t start_time, Inifile& ini, ModuleManager & mm, Front & front, Authentifier & authentifier, Fstat & fstat)
    {
        SessionReactor::EnableGraphics enable_graphics{true};
        try {
            session_reactor.execute_timers(enable_graphics, [&]() -> gdi::GraphicApi& {
                return mm.get_graphic_wrapper();
            });
        } catch (Error const& e) {
            this->check_exception(e, signal, ini, mm, session_reactor, authentifier, front, run_session, acl.get());
        }

        if (!acl) {
            this->start_acl(acl, cctx, rnd, now, ini, mm, session_reactor, authentifier, signal, fstat);
        }

        session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
            return io_fd_isset(fd, ioswitch.rfds);
        });

        // front event
        try {
            if (session_reactor.has_front_event()) {
                session_reactor.execute_callbacks(mm.get_callback());
            }
            bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
            if (front_is_set) {
                front.rbuf.load_data(front.trans);
                while (front.rbuf.next(front.is_in_nla()?(TpduBuffer::CREDSSP):(TpduBuffer::PDU)))
                {
                    bytes_view tpdu = front.rbuf.current_pdu_buffer();
                    uint8_t current_pdu_type = front.rbuf.current_pdu_get_type();
                    front.incoming(tpdu, current_pdu_type, mm.get_callback());
                }
            }
        } catch (Error const& e) {
            if (ERR_DISCONNECT_BY_USER == e.id) {
                front_signal = BACK_EVENT_NEXT;
            }
            else {
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
            }
        } catch (...) {
            LOG(LOG_ERR, "Proxy data processing raised unknown error");
            run_session = false;
        }

        if (run_session == false){
            return;
        }

        // acl event
        try {
            if (front.state == Front::UP_AND_RUNNING) {
                // new value incoming from authentifier
                if (ini.check_from_acl()) {
                    auto const rt_status = front.set_rt_display(ini.get<cfg::video::rt_display>());

                    if (ini.get<cfg::client::enable_osd_4_eyes>()) {
                        Translator tr(language(ini));
                        switch (rt_status) {
                            case Capture::RTDisplayResult::Enabled:
                                mm.osd_message(tr(trkeys::enable_rt_display).to_string(), true);
                                break;
                            case Capture::RTDisplayResult::Disabled:
                                mm.osd_message(tr(trkeys::disable_rt_display).to_string(), true);
                                break;
                            case Capture::RTDisplayResult::Unchanged:
                                break;
                        }
                    }

                    mm.check_module();
                }

                try
                {
                    if (BACK_EVENT_NONE == session_reactor.signal) {
                        // Process incoming module trafic
                        auto& gd = mm.get_graphic_wrapper();
                        session_reactor.execute_graphics([&ioswitch](int fd, auto& /*e*/){
                            return io_fd_isset(fd, ioswitch.rfds);
                        }, gd);
                    }
                }
                catch (Error const & e) {
                    this->check_exception(e, signal, ini, mm, session_reactor, authentifier, front, run_session, acl.get());
                }

                // Incoming data from ACL
                if (acl && (acl->auth_trans.has_pending_data() || io_fd_isset(acl->auth_trans.sck, ioswitch.rfds))) {
                    // authentifier received updated values
                    acl->acl_serial.receive();
                    if (!ini.changed_field_size()) {
                        session_reactor.execute_sesman(ini);
                    }
                }

                const bool enable_osd = ini.get<cfg::globals::enable_osd>();
                if (enable_osd) {
                    const uint32_t enddate = ini.get<cfg::context::end_date_cnx>();
                    if (enddate && mm.is_up_and_running()) {
                        mm.update_end_session_warning(start_time, static_cast<time_t>(enddate), now.tv_sec);
                    }
                }

                if (acl){
                    if (front.state == Front::UP_AND_RUNNING) {
                        signal = BackEvent_t(session_reactor.signal);
                        int i = 0;
                        do {
                            if (++i == 11) {
                                LOG(LOG_ERR, "loop event error");
                                break;
                            }
                            session_reactor.signal = 0;
                            run_session = acl->acl_serial.check(
                                authentifier, authentifier, mm,
                                now.tv_sec, signal, front_signal, front.has_user_activity
                            );
                            if (!session_reactor.signal) {
                                session_reactor.signal = signal;
                                break;
                            }

                            if (signal) {
                                session_reactor.signal = signal;
                            }
                            else {
                                signal = BackEvent_t(session_reactor.signal);
                            }
                        } while (session_reactor.signal);
                    }
                    else if (BackEvent_t(session_reactor.signal) == BACK_EVENT_STOP) {
                        run_session = false;
                    }
                    if (mm.last_module) {
                        authentifier.set_acl_serial(nullptr);
                        acl.reset();
                    }
                }
                else if ((!this->ini.is_asked<cfg::globals::nla_auth_user>())
                && this->ini.get<cfg::client::enable_nla>()) {
                    acl->acl_serial.send_acl_data();
                }
            }
        } catch (Error const& e) {
            LOG(LOG_ERR, "Session::Session exception (2) = %s", e.errmsg());
            signal = BackEvent_t(session_reactor.signal);
            mm.invoke_close_box(
                local_err_msg(e, language(ini)),
                signal, authentifier, authentifier);
            session_reactor.signal = signal;

            if (BackEvent_t(session_reactor.signal) == BACK_EVENT_STOP) {
                run_session = false;
            }
        }
    }

public:
    Session(SocketTransport&& front_trans, Inifile& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat)
    : ini(ini)
    {
        TRANSLATIONCONF.set_ini(&ini);

        SessionReactor session_reactor;

        const bool mem3blt_support = true;

        Authentifier authentifier(ini, cctx, to_verbose_flags(ini.get<cfg::debug::auth>()));
        session_reactor.set_current_time(tvtime());

        Front front(
            session_reactor, front_trans, rnd, ini, cctx, authentifier,
            ini.get<cfg::client::fast_path>(), mem3blt_support
        );

        std::unique_ptr<Acl> acl;

        std::string disconnection_message_error;

        try {
            TimeSystem timeobj;
            ModuleManager mm(session_reactor, front, this->ini, rnd, timeobj);

            BackEvent_t signal       = BACK_EVENT_NONE;
            BackEvent_t front_signal = BACK_EVENT_NONE;

            if (ini.get<cfg::debug::session>()) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(nullptr);
            if (ini.get<cfg::debug::performance>() & 0x8000) {
                this->write_performance_log(start_time);
            }

            bool run_session = true;

            // TODO: we should define some select object to wrap rfds, wfds and timeouts
            // and hide events inside modules managing sockets (or timers)
            // this should help in the future to generalise architecture
            // to multiple simultaneous fronts and mods. It should also simplify
            // module manager. Complexity of module transition should be hidden behind module
            // managers

            // fd_set wfds;
            // io_fd_zero(wfds);

            using namespace std::chrono_literals;

            timeval now = tvtime();
            session_reactor.set_current_time(now);

            while (run_session) {
                timeval default_timeout = now;
                default_timeout.tv_sec += this->select_timeout_tv_sec;

                Select ioswitch(default_timeout);

                SessionReactor::EnableGraphics enable_graphics{front.state == Front::UP_AND_RUNNING};
                // LOG(LOG_DEBUG, "front.up_and_running = %d", front.up_and_running);

                auto const sck_no_read = this->set_fds(
                    ioswitch, session_reactor, enable_graphics,
                    front_trans, mm, acl);

                // LOG(LOG_DEBUG, "timeout = %ld %ld", timeout.tv_sec, timeout.tv_usec);
                now = tvtime();
                session_reactor.set_current_time(now);
                ioswitch.set_timeout(
                        session_reactor.get_next_timeout(enable_graphics, ioswitch.get_timeout(now)));

                // 0 if tv < tv_now : returns immediately
        //                ioswitch.timeoutastv = to_timeval(
        //                                        session_reactor.get_next_timeout(enable_graphics, timeout)
        //                                      - now);
                // LOG(LOG_DEBUG, "tv_now: %ld %ld", tv_now.tv_sec, tv_now.tv_usec);
                // session_reactor.timer_events_.info(tv_now);

                int num = ioswitch.select(now);

                // for (unsigned i = 0; i <= max; ++i) {
                //     LOG(LOG_DEBUG, "fd %u is set %d", i, io_fd_isset(i, rfds));
                // }

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

                this->send_waiting_data(ioswitch, sck_no_read, front_trans, mm, acl);

                now = tvtime();
                session_reactor.set_current_time(now);
                if (ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now.tv_sec);
                }

                switch (front.state) {
                case Front::UP_AND_RUNNING:
                {
                    this->front_up_and_running(run_session, front_trans, ioswitch, session_reactor, signal, front_signal, acl, cctx, rnd, now, start_time, ini, mm, front, authentifier, fstat);
                    if (run_session == false){
                        break;
                    }
                }
                break;
                default:
                {
                    this->front_starting(run_session, front_trans, ioswitch, session_reactor, signal, front_signal, acl, cctx, rnd, now, start_time, ini, mm, front, authentifier, fstat);
                    if (run_session == false){
                        break;
                    }
                }
                }
            }
            if (mm.get_mod()) {
                mm.get_mod()->disconnect();
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

    static unique_fd acl_connect(std::string const & authtarget)
    {
        unique_fd client_sck = addr_connect(authtarget.c_str());
        if (!client_sck.is_open()) {
            LOG(LOG_ERR,
                "Failed to connect to authentifier (%s)",
                authtarget.c_str());
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        return client_sck;
    }

    static void send_waiting_data(
        Select& ioswitch,
        SckNoRead const& sck_no_read,
        SocketTransport& front_trans,
        ModuleManager& mm,
        std::unique_ptr<Acl>& /*acl*/)
    {
        auto is_set = [&ioswitch](int fd){
            return fd != INVALID_SOCKET && io_fd_isset(fd, ioswitch.wfds);
        };

        if (is_set(sck_no_read.sck_mod)) {
            mm.get_socket()->send_waiting_data();
        }

        if (is_set(sck_no_read.sck_front)) {
            front_trans.send_waiting_data();
        }

//         if (is_set(sck_no_read.sck_acl)) {
//             acl->auth_trans.send_waiting_data();
//         }
    }

    static SckNoRead set_fds(
        Select& ioswitch,
        SessionReactor& session_reactor, SessionReactor::EnableGraphics enable_graphics,
        SocketTransport const& front_trans,
        ModuleManager const& mm,
        std::unique_ptr<Acl> const& acl)
    {
        SckNoRead sck_no_read;

        if (front_trans.has_waiting_data()) {
            ioswitch.set_write_sck(front_trans.sck);
            sck_no_read.sck_front = front_trans.sck;
        }
        else if (mm.get_mod()->is_up_and_running() || !bool(enable_graphics)) {
            ioswitch.set_read_sck(front_trans.sck);

            if (mm.validator_fd > 0) {
                ioswitch.set_read_sck(mm.validator_fd);
            }
        }

        if (mm.get_socket() && !mm.has_pending_data()) {
            if (mm.get_socket()->has_waiting_data()) {
                sck_no_read.sck_mod = mm.get_socket()->sck;
                ioswitch.set_write_sck(sck_no_read.sck_mod);
            }
            else if (sck_no_read.sck_front != INVALID_SOCKET) {
                sck_no_read.sck_mod = mm.get_socket()->sck;
            }
        }

        if (acl) {
            ioswitch.set_read_sck(acl->auth_trans.sck);
        }

        if (front_trans.has_pending_data()
        || mm.has_pending_data()
        || (acl && acl->auth_trans.has_pending_data())){
            ioswitch.immediate_wakeup(session_reactor.get_current_time());
        }

        session_reactor.for_each_fd(
            enable_graphics,
            [&](int fd){
                if (!sck_no_read.contains(fd)) {
                    ioswitch.set_read_sck(fd);
                }
            }
        );

        return sck_no_read;
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

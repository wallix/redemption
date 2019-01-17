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

#pragma once

#include "acl/authentifier.hpp"
#include "acl/module_manager.hpp"
#include "capture/capture.hpp"
#include "configs/config.hpp"
#include "core/server.hpp"
#include "core/session_reactor.hpp"
#include "core/set_server_redirection_target.hpp"
#include "front/front.hpp"
#include "mod/mod_api.hpp"
#include "transport/socket_transport.hpp"
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


class Session
{
    Inifile & ini;

    time_t      perf_last_info_collect_time = 0;
    const pid_t perf_pid = getpid();
    File        perf_file = nullptr;

    static const time_t select_timeout_tv_sec = 3;

public:
    Session(unique_fd sck, Inifile & ini, CryptoContext & cctx, Random & rnd, Fstat & fstat)
        : ini(ini)
    {
        TRANSLATIONCONF.set_ini(&ini);

        SessionReactor session_reactor;

        SocketTransport front_trans(
            "RDP Client", std::move(sck), "", 0, std::chrono::milliseconds(ini.get<cfg::client::recv_timeout>()),
            to_verbose_flags(this->ini.get<cfg::debug::front>())
        );

        const bool mem3blt_support = true;

        Authentifier authentifier(ini, cctx, to_verbose_flags(ini.get<cfg::debug::auth>()));
        session_reactor.set_current_time(tvtime());
        time_t now = session_reactor.get_current_time().tv_sec;

        Front front(
            session_reactor, front_trans, rnd, this->ini, cctx, authentifier,
            this->ini.get<cfg::client::fast_path>(), mem3blt_support, now
        );

        std::unique_ptr<Acl> acl;

        std::string disconnection_message_error;

        try {
            TimeSystem timeobj;
            ModuleManager mm(session_reactor, front, this->ini, rnd, timeobj);

            BackEvent_t signal       = BACK_EVENT_NONE;
            BackEvent_t front_signal = BACK_EVENT_NONE;

            if (this->ini.get<cfg::debug::session>()) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(nullptr);
            if (this->ini.get<cfg::debug::performance>() & 0x8000) {
                this->write_performance_log(start_time);
            }

            bool run_session = true;

            constexpr std::array<unsigned, 4> timers{{ 30*60, 10*60, 5*60, 1*60, }};
            const unsigned OSD_STATE_INVALID = timers.size();
            const unsigned OSD_STATE_NOT_YET_COMPUTED = OSD_STATE_INVALID + 1;
            unsigned osd_state = OSD_STATE_NOT_YET_COMPUTED;
            const bool enable_osd = this->ini.get<cfg::globals::enable_osd>();


            // TODO: we should define some select object to wrap rfds, wfds and timeouts
            // and hide events inside modules managing sockets (or timers)
            // this should help in the future to generalise architecture
            // to multiple simultaneous fronts and mods. It should also simplify
            // module manager. Complexity of module transition should be hidden behind module
            // managers

            // fd_set wfds;
            // io_fd_zero(wfds);

            using namespace std::chrono_literals;

            session_reactor.set_current_time(tvtime());

            while (run_session) {

                timeval default_timeout = session_reactor.get_current_time();
                default_timeout.tv_sec += this->select_timeout_tv_sec;

                Select ioswitch(default_timeout);

                SessionReactor::EnableGraphics enable_graphics{front.up_and_running};
                // LOG(LOG_DEBUG, "front.up_and_running = %d", front.up_and_running);

                auto const sck_no_read = this->set_fds(
                    ioswitch, session_reactor, enable_graphics,
                    front_trans, mm, acl);

                // LOG(LOG_DEBUG, "timeout = %ld %ld", timeout.tv_sec, timeout.tv_usec);
                session_reactor.set_current_time(tvtime());
                ioswitch.set_timeout(
                    session_reactor.get_next_timeout(
                        enable_graphics, ioswitch.get_timeout(session_reactor.get_current_time())));

                // 0 if tv < tv_now : returns immediately
//                ioswitch.timeoutastv = to_timeval(
//                                        session_reactor.get_next_timeout(enable_graphics, timeout)
//                                      - session_reactor.get_current_time());
                // LOG(LOG_DEBUG, "tv_now: %ld %ld", tv_now.tv_sec, tv_now.tv_usec);
                // session_reactor.timer_events_.info(tv_now);

                int num = ioswitch.select(session_reactor.get_current_time());

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

                session_reactor.set_current_time(tvtime());
                now = session_reactor.get_current_time().tv_sec;
                if (this->ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now);
                }

                auto check_exception = [this, &session_reactor, &authentifier, &acl, &signal, &mm](Error const& e) {
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
                        if (this->ini.get<cfg::mod_rdp::session_probe_on_launch_failure>() ==
                            SessionProbeOnLaunchFailure::retry_without_session_probe) {
                            this->ini.get_ref<cfg::mod_rdp::enable_session_probe>() = false;

                            session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
                        }
                        else if (acl) {
                            this->ini.set_acl<cfg::context::session_probe_launch_error_message>(local_err_msg(e, language(this->ini)));

                            authentifier.report("SESSION_PROBE_LAUNCH_FAILED", "");
                        }
                        else {
                            LOG(LOG_ERR, "Session::Session exception (1) = %s\n", e.errmsg());
                            time_t now = time(nullptr);
                            signal = BackEvent_t(session_reactor.signal);

                            const char * auth_error_message = ((ERR_RAIL_LOGON_FAILED_OR_WARNING == e.id) ? nullptr : local_err_msg(e, language(this->ini)));

                            mm.invoke_close_box(
                                auth_error_message,
                                signal, now, authentifier, authentifier);
                            session_reactor.signal = signal;
                        }
                    }
                    else if ((e.id == ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION) ||
                             (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED)) {
                        if (e.id == ERR_AUTOMATIC_RECONNECTION_REQUIRED) {
                            this->ini.set<cfg::context::perform_automatic_reconnection>(true);
                        }

                        session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
                    }
                    else if (e.id == ERR_RAIL_NOT_ENABLED) {
                        this->ini.get_ref<cfg::mod_rdp::use_native_remoteapp_capability>() = false;

                        session_reactor.set_event_next(BACK_EVENT_RETRY_CURRENT);
                    }
                    else if ((e.id == ERR_RDP_SERVER_REDIR) &&
                             this->ini.get<cfg::mod_rdp::server_redirection_support>()) {
                        set_server_redirection_target(this->ini, authentifier);
                        session_reactor.set_next_event(BACK_EVENT_RETRY_CURRENT);
                    }
                    else {
                        LOG(LOG_ERR, "Session::Session exception (2) = %s\n", e.errmsg());
                        time_t now = time(nullptr);
                        signal = BackEvent_t(session_reactor.signal);

                        const char * auth_error_message = ((ERR_RAIL_LOGON_FAILED_OR_WARNING == e.id) ? nullptr : local_err_msg(e, language(this->ini)));

                        mm.invoke_close_box(
                            auth_error_message,
                            signal, now, authentifier, authentifier);
                        session_reactor.signal = signal;
                    }
                };

                try {
                    session_reactor.execute_timers(enable_graphics, [&]() -> gdi::GraphicApi& {
                        return mm.get_graphic_wrapper();
                    });
                } catch (Error const& e) {
                    check_exception(e);
                }

                session_reactor.execute_events([&ioswitch](int fd, auto& /*e*/){
                    return io_fd_isset(fd, ioswitch.rfds);
                });

                bool const front_is_set = front_trans.has_pending_data() || io_fd_isset(front_trans.sck, ioswitch.rfds);
                if (session_reactor.has_front_event() || front_is_set) {
                    try {
                        if (session_reactor.has_front_event()) {
                            session_reactor.execute_callbacks(mm.get_callback());
                        }
                        if (front_is_set) {
                            front.incoming(mm.get_callback(), now);
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
                            continue;
                        }
                    } catch (...) {
                        LOG(LOG_ERR, "Proxy data processing raised unknown error");
                        run_session = false;
                        continue;
                    };
                }

                try {
                    if (front.up_and_running) {
                        // new value incoming from authentifier
                        if (this->ini.check_from_acl()) {
                            auto const rt_status
                              = front.set_rt_display(ini.get<cfg::video::rt_display>());

                            if (this->ini.get<cfg::client::enable_osd_4_eyes>()) {
                                Translator tr(language(this->ini));
                                switch (rt_status) {
                                    case Capture::RTDisplayResult::Enabled:
                                        mm.osd_message(tr(trkeys::enable_rt_display), true);
                                        break;
                                    case Capture::RTDisplayResult::Disabled:
                                        mm.osd_message(tr(trkeys::disable_rt_display), true);
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
                            check_exception(e);
                        }

                        // Incoming data from ACL, or opening authentifier
                        if (!acl) {
                            if (!mm.last_module) {
                                // authentifier never opened or closed by me (close box)
                                try {
                                    // now is authentifier start time
                                    acl = std::make_unique<Acl>(
                                        ini, Session::acl_connect(this->ini.get<cfg::globals::authfile>()), now, cctx, rnd, fstat
                                    );
                                    const auto sck = acl->auth_trans.sck;
                                    fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) & ~O_NONBLOCK);
                                    authentifier.set_acl_serial(&acl->acl_serial);
                                    session_reactor.set_next_event(BACK_EVENT_NEXT);
                                }
                                catch (...) {
                                    signal = BackEvent_t(session_reactor.signal);
                                    session_reactor.signal = 0;
                                    mm.invoke_close_box("No authentifier available", signal, now, authentifier, authentifier);
                                    if (!session_reactor.signal || signal) {
                                        session_reactor.signal = signal;
                                    }
                                }
                            }
                        }
                        else if (acl->auth_trans.has_pending_data() || io_fd_isset(acl->auth_trans.sck, ioswitch.rfds)) {
                            // authentifier received updated values
                            acl->acl_serial.receive();
                            if (!ini.changed_field_size()) {
                                session_reactor.execute_sesman(ini);
                            }
                        }

                        if (enable_osd) {
                            const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
                            if (enddate && mm.is_up_and_running()) {
                                if (osd_state == OSD_STATE_NOT_YET_COMPUTED) {
                                    osd_state = (enddate <= static_cast<uint32_t>(now))
                                        ? OSD_STATE_INVALID
                                        : timers.rbegin()
                                            - std::lower_bound(
                                                timers.rbegin(),
                                                timers.rend(),
                                                enddate - start_time);
                                }
                                else if (osd_state < OSD_STATE_INVALID
                                     && enddate - now <= timers[osd_state]) {
                                    const unsigned minutes = (enddate - now + 30) / 60;
                                    std::string mes = str_concat(
                                        std::to_string(minutes),
                                        ' ',
                                        TR(trkeys::minute, language(this->ini)),
                                        (minutes > 1) ? "s " : " ",
                                        TR(trkeys::before_closing, language(this->ini))
                                    );
                                    mm.osd_message(std::move(mes), true);
                                    ++osd_state;
                                }
                            }
                        }

                        if (acl) {
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
                                    now, signal, front_signal, front.has_user_activity
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
                } catch (Error const& e) {
                    LOG(LOG_ERR, "Session::Session exception (2) = %s\n", e.errmsg());
                    time_t now = time(nullptr);
                    signal = BackEvent_t(session_reactor.signal);
                    mm.invoke_close_box(
                        local_err_msg(e, language(this->ini)),
                        signal, now, authentifier, authentifier);
                    session_reactor.signal = signal;
                };

            }
            if (mm.get_mod()) {
                mm.get_mod()->disconnect(time(nullptr));
            }
            front.disconnect();
        }
        catch (Error const& e) {
            disconnection_message_error = e.errmsg();
            LOG(LOG_INFO, "Session::Session Init exception = %s!\n", disconnection_message_error);
        }
        catch (const std::exception & e) {
            disconnection_message_error = e.what();
            LOG(LOG_ERR, "Session::Session exception (3) = %s!\n", disconnection_message_error);
        }
        catch(...) {
            disconnection_message_error = "Exception in Session::Session";
            LOG(LOG_INFO, "Session::Session other exception in Init\n");
        }
        // silent message for localhost for watchdog
        if (!this->ini.is_asked<cfg::globals::host>()
        && (this->ini.get<cfg::globals::host>() != "127.0.0.1")) {
            LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        }
        detail::log_proxy_disconnection(disconnection_message_error.c_str());
        front.must_be_stop_capture();
    }

    Session(Session const &) = delete;

    ~Session() {
        if (this->ini.get<cfg::debug::performance>() & 0x8000) {
            this->write_performance_log(this->perf_last_info_collect_time + 3);
        }
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        if (!this->ini.get<cfg::context::session_id>().empty()) {
            char new_session_file[256];
            snprintf( new_session_file, sizeof(new_session_file), "%s/session_%s.pid"
                    , app_path(AppPath::LockDir), this->ini.get<cfg::context::session_id>().c_str());
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", app_path(AppPath::LockDir), child_pid);
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
                this->ini.get<cfg::video::record_tmp_path>().c_str(),
                tm_.tm_year + 1900, tm_.tm_mon, tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec, this->perf_pid
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
        size_t const pos = authtarget.find(':');
        unique_fd client_sck = (pos == std::string::npos)
            ? local_connect(authtarget.c_str(), 30, 1000)
            : [&](){
                // TODO: add some explicit error checking
                char* end;
                std::string ip = authtarget.substr(0, pos);
                long port = std::strtol(authtarget.c_str() + pos + 1, &end, 10);
                if (port > std::numeric_limits<int>::max()) {
                    return unique_fd{-1};
                }
                return ip_connect(ip.c_str(), int(port), 30, 1000);
            }();

        if (!client_sck.is_open()) {
            LOG(LOG_ERR,
                "Failed to connect to authentifier (%s)",
                authtarget.c_str());
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        return client_sck;
    }


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

        bool contains(int fd) const noexcept
        {
            return sck_front == fd
                || sck_mod == fd
                // || sck_acl == fd
            ;
        }
    };

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

        // if (is_set(sck_no_read.sck_acl)) {
        //     acl->auth_trans.send_waiting_data();
        // }
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
        //     if (acl->auth_trans.has_waiting_data()) {
        //         sck_no_read.sck_acl = acl->auth_trans.sck;
        //         ioswitch.set_write_sck(sck_no_read.sck_acl);
        //     }
        //     else {
                ioswitch.set_read_sck(acl->auth_trans.sck);
        //     }
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

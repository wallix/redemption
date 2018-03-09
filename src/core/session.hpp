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

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#include <array>

#include "utils/invalid_socket.hpp"
#include "utils/verbose_flags.hpp"

#include "acl/authentifier.hpp"
#include "core/server.hpp"
#include "core/wait_obj.hpp"
#include "core/session_reactor.hpp"
#include "front/front.hpp"
#include "mod/mod_api.hpp"
#include "system/ssl_calls.hpp"
#include "transport/transport.hpp"
#include "utils/colors.hpp"
#include "utils/bitmap.hpp"
#include "utils/netutils.hpp"
#include "utils/rect.hpp"
#include "utils/stream.hpp"
#include "utils/genfstat.hpp"

#include "capture/capture.hpp"

#include "configs/config.hpp"


class Session
{
    struct Acl
    {
        SocketTransport auth_trans;
        AclSerializer   acl_serial;

        Acl(Inifile & ini, unique_fd client_sck, time_t now,
            CryptoContext & cctx, Random & rnd, Fstat & fstat)
        : auth_trans(
            "Authentifier", std::move(client_sck),
            ini.get<cfg::globals::authfile>().c_str(), 0,
            std::chrono::seconds(1),
            to_verbose_flags(ini.get<cfg::debug::auth>()))
        , acl_serial(
            ini, now, this->auth_trans, cctx, rnd, fstat,
            to_verbose_flags(ini.get<cfg::debug::auth>()))
        {}
    };

    Inifile & ini;

          time_t   perf_last_info_collect_time;
    const pid_t    perf_pid;
          FILE   * perf_file;

    static const time_t select_timeout_tv_sec = 3;

    static void wait_on_sck(SocketTransport& sck, fd_set& rfds, unsigned& max)
    {
        if (sck.sck > INVALID_SOCKET) {
            io_fd_set(sck.sck, rfds);
            max = std::max(static_cast<unsigned>(sck.sck), max);
        }
    }

    static bool sck_is_set(SocketTransport& sck, fd_set& rfds)
    {
        return sck.has_pending_data() || io_fd_isset(sck.sck, rfds);
    }

public:
    Session(unique_fd sck, Inifile & ini, CryptoContext & cctx, Random & rnd, Fstat & fstat)
        : ini(ini)
        , perf_last_info_collect_time(0)
        , perf_pid(getpid())
        , perf_file(nullptr)
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

            const timeval time_mark = { this->select_timeout_tv_sec, 0 };

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

            while (run_session) {
                unsigned max = 0;
                fd_set rfds;

                io_fd_zero(rfds);
                timeval timeout = time_mark;

                if (mm.mod->is_up_and_running() || !front.up_and_running) {
                    wait_on_sck(front_trans, rfds, max);
                    if (front.capture) {
                        front.capture->get_capture_event().wait_on_timeout(timeout);
                    }
                }

                if (acl) {
                    wait_on_sck(acl->auth_trans, rfds, max);
                }

                if (front_trans.has_pending_data()
                 || mm.has_pending_data()
                 || (acl && acl->auth_trans.has_pending_data())) {
                    timeout = {0, 0};
                }

                session_reactor.clear();

                auto tv = session_reactor.timer_events_.get_next_timeout();
                if (front.up_and_running) {
                    auto tv2 = session_reactor.graphic_timer_events_.get_next_timeout();
                    if (tv.tv_sec >= 0) {
                        if (tv2.tv_sec >= 0) {
                            tv = std::min(tv, tv2);
                        }
                    }
                    else {
                        tv = tv2;
                    }
                }
                auto tv_now = tvtime();
                // LOG(LOG_DEBUG, "%ld %ld - %ld %ld", tv.tv_sec, tv.tv_usec, tv_now.tv_sec, tv_now.tv_usec);
                if (tv.tv_sec >= 0 && tv < timeout + tv_now) {
                    if (tv < tv_now) {
                        timeout = {0, 0};
                    }
                    else {
                        timeout = tv - tv_now;
                    }
                }
                // LOG(LOG_DEBUG, "tv_now: %ld %ld", tv_now.tv_sec, tv_now.tv_usec);
                // session_reactor.timer_events_.info(tv_now);

                for (auto& top_fd : session_reactor.fd_events_.elements) {
                    // LOG(LOG_DEBUG, "set fd: %d", top_fd->fd);
                    if (top_fd->alive()) {
                        io_fd_set(top_fd->value.fd, rfds);
                    }
                    max = std::max(max, unsigned(top_fd->value.fd));
                }
                for (auto& top_fd : session_reactor.graphic_fd_events_.elements) {
                    // LOG(LOG_DEBUG, "set fd: %d", top_fd->fd);
                    if (top_fd->alive()) {
                        io_fd_set(top_fd->value.fd, rfds);
                    }
                    max = std::max(max, unsigned(top_fd->value.fd));
                }

                int num = select(max + 1, &rfds, nullptr/*&wfds*/, nullptr, &timeout);

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

                session_reactor.set_current_time(tvtime());
                now = session_reactor.get_current_time().tv_sec;
                if (this->ini.get<cfg::debug::performance>() & 0x8000) {
                    this->write_performance_log(now);
                }

                auto const end_tv = session_reactor.get_current_time();
                if (num == 0) {
                    session_reactor.timer_events_.exec(end_tv);
                    if (front.up_and_running) {
                        session_reactor.graphic_timer_events_.exec(end_tv, mm.get_graphic_wrapper(front));
                    }
                }
                // session_reactor.timer_events_.info(end_tv);

                session_reactor.fd_events_.exec(rfds);
                if (session_reactor.front_events().size() || sck_is_set(front_trans, rfds)) {
                    try {
                        session_reactor.front_events_.exec(mm.get_callback());
                        if (sck_is_set(front_trans, rfds)) {
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
                            front.update_config(ini.get<cfg::video::rt_display>());
                            mm.check_module();
                        }

                        try
                        {
                            if (BACK_EVENT_NONE == session_reactor.signal) {
                                // Process incoming module trafic
                                session_reactor.graphic_events_.exec(mm.get_graphic_wrapper(front));
                                session_reactor.graphic_fd_events_.exec(rfds, mm.get_graphic_wrapper(front));
                            }
                        }
                        catch (Error const & e) {
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
                                    throw;
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
                                // SET new target in ini
                                const char * host = char_ptr_cast(
                                    this->ini.get<cfg::mod_rdp::redir_info>().host);
                                const char * password = char_ptr_cast(
                                    this->ini.get<cfg::mod_rdp::redir_info>().password);
                                const char * username = char_ptr_cast(
                                    this->ini.get<cfg::mod_rdp::redir_info>().username);
                                const char * change_user = "";
                                if (this->ini.get<cfg::mod_rdp::redir_info>().dont_store_username &&
                                    (username[0] != 0)) {
                                    LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                                    this->ini.set_acl<cfg::globals::target_user>(username);
                                    change_user = username;
                                }
                                if (password[0] != 0) {
                                    LOG(LOG_INFO, "SrvRedir: Change target password");
                                    this->ini.set_acl<cfg::context::target_password>(password);
                                }
                                LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
                                this->ini.set_acl<cfg::context::target_host>(host);
                                char message[768] = {};
                                sprintf(message, "%s@%s", change_user, host);
                                authentifier.report("SERVER_REDIRECTION", message);

                                session_reactor.set_next_event(BACK_EVENT_RETRY_CURRENT);
                            }
                            else {
                                throw;
                            }
                        }

                        if (front.capture && front.capture->get_capture_event().is_set(INVALID_SOCKET, rfds)) {
                            front.periodic_snapshot();
                        }

                        // Incoming data from ACL, or opening authentifier
                        if (!acl) {
                            if (!mm.last_module) {
                                // authentifier never opened or closed by me (close box)
                                try {
                                    std::string const & authtarget = this->ini.get<cfg::globals::authfile>();
                                    size_t pos = authtarget.find(':');
                                    unique_fd client_sck = (pos == std::string::npos)
                                        ? local_connect(authtarget.c_str(), 30, 1000)
                                        : [&](){
                                            // TODO: add some explicit error checking
                                            std::string ip = authtarget.substr(0, pos);
                                            int port = std::atoi(authtarget.c_str() + pos+1);
                                            return ip_connect(ip.c_str(), port, 30, 1000);
                                        }();
                                    if (!client_sck.is_open()) {
                                        LOG(LOG_ERR,
                                            "Failed to connect to authentifier (%s)",
                                            this->ini.get<cfg::globals::authfile>().c_str());
                                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                                    }

                                    // now is authentifier start time
                                    acl.reset(new Acl(
                                        ini, std::move(client_sck), now, cctx, rnd, fstat
                                    ));
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
                        else if (sck_is_set(acl->auth_trans, rfds)) {
                            // authentifier received updated values
                            acl->acl_serial.receive();
                            if (!ini.changed_field_size()) {
                                session_reactor.sesman_events_.exec(ini);
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
                                    std::string mes;
                                    mes.reserve(128);
                                    const unsigned minutes = (enddate - now + 30) / 60;
                                    mes += std::to_string(minutes);
                                    mes += ' ';
                                    mes += TR(trkeys::minute, language(this->ini));
                                    mes += (minutes > 1) ? "s " : " ";
                                    mes += TR(trkeys::before_closing, language(this->ini));
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
                                else if (signal) {
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
                    LOG(LOG_ERR, "Session::Session exception = %s\n", e.errmsg());
                    time_t now = time(nullptr);
                    signal = BackEvent_t(session_reactor.signal);
                    mm.invoke_close_box(
                        local_err_msg(e, language(this->ini)),
                        signal, now, authentifier, authentifier);
                    session_reactor.signal = signal;
                }
            }
            if (mm.mod) {
                mm.mod->disconnect(time(nullptr));
            }
            front.disconnect();
        }
        catch (Error const& e) {
            LOG(LOG_INFO, "Session::Session Init exception = %s!\n", e.errmsg());
        }
        catch (const std::exception & e) {
            LOG(LOG_ERR, "Session::Session exception = %s!\n", e.what());
        }
        catch(...) {
            LOG(LOG_INFO, "Session::Session other exception in Init\n");
        }
        // silent message for localhost for watchdog
        if (!this->ini.is_asked<cfg::globals::host>()
        && (this->ini.get<cfg::globals::host>() != "127.0.0.1")) {
            LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        }
        front.must_be_stop_capture();
    }

    Session(Session const &) = delete;

    ~Session() {
        if (this->ini.get<cfg::debug::performance>() & 0x8000) {
            this->write_performance_log(this->perf_last_info_collect_time + 3);
        }
        if (this->perf_file) {
            ::fclose(this->perf_file);
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

            this->perf_file = ::fopen(filename, "w");

            ::fprintf(this->perf_file,
                "time_t;"
                "ru_utime.tv_sec;ru_utime.tv_usec;ru_stime.tv_sec;ru_stime.tv_usec;"
                "ru_maxrss;ru_ixrss;ru_idrss;ru_isrss;ru_minflt;ru_majflt;ru_nswap;"
                "ru_inblock;ru_oublock;ru_msgsnd;ru_msgrcv;ru_nsignals;ru_nvcsw;ru_nivcsw\n");

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
                  this->perf_file
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
            ::fflush(this->perf_file);
        }
        while (this->perf_last_info_collect_time + this->select_timeout_tv_sec <= now);
    }
};

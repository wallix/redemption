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

#ifndef _REDEMPTION_CORE_SESSION_HPP_
#define _REDEMPTION_CORE_SESSION_HPP_

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#include <array>

#include "server.hpp"
#include "colors.hpp"
#include "stream.hpp"
#include "front.hpp"
#include "ssl_calls.hpp"
#include "rect.hpp"
#include "client_info.hpp"
#include "netutils.hpp"

#include "config.hpp"
#include "wait_obj.hpp"
#include "transport.hpp"
#include "bitmap.hpp"

#include "authentifier.hpp"

using namespace std;

enum {
    // before anything else : exchange of credentials
//    SESSION_STATE_RSA_KEY_HANDSHAKE,
    // initial state no module loaded, init not done
    SESSION_STATE_ENTRY,
    // no module loaded
    // init_done
    // login window destoyed if necessary
    // user clicked on OK to run module  or provided connection info on cmd line
    // but did not received credentials yet
    SESSION_STATE_WAITING_FOR_NEXT_MODULE,
    // a module is loaded and active but required some action
    // involving requesting remote context
    SESSION_STATE_WAITING_FOR_CONTEXT,
    // init_done, module loaded and running
    SESSION_STATE_RUNNING,
    // display dialog when connection is closed
    SESSION_STATE_CLOSE_CONNECTION,
    // disconnect session
    SESSION_STATE_STOP
};

struct Session {
    Inifile  * ini;
    uint32_t & verbose;

    int internal_state;
    long id;                     // not used

    Front * front;

    SessionManager * acl;

    UdevRandom gen;

    SocketTransport * ptr_auth_trans;
    wait_obj        * ptr_auth_event;

          time_t   perf_last_info_collect_time;
    const pid_t    perf_pid;
          FILE   * perf_file;

    const time_t select_timeout_tv_sec = 3;

    Session(int sck, Inifile * ini)
            : ini(ini)
            , verbose(this->ini->debug.session)
            , acl(NULL)
            , ptr_auth_trans(NULL)
            , ptr_auth_event(NULL)
            , perf_last_info_collect_time(0)
            , perf_pid(getpid())
            , perf_file(NULL) {
        try {
            SocketTransport front_trans("RDP Client", sck, "", 0, this->ini->debug.front);
            wait_obj front_event(&front_trans);
            // Contruct auth_trans (SocketTransport) and auth_event (wait_obj)
            //  here instead of inside Sessionmanager

            this->ptr_auth_trans = NULL;
            this->ptr_auth_event = NULL;
            this->acl            = NULL;

            this->internal_state = SESSION_STATE_ENTRY;

            const bool enable_fastpath = true;
            const bool mem3blt_support = true;

            this->front = new Front( &front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, &this->gen
                                   , ini, enable_fastpath, mem3blt_support);

            ModuleManager mm(*this->front, *this->ini);
            BackEvent_t signal = BACK_EVENT_NONE;

            // Under conditions (if this->ini->video.inactivity_pause == true)
            PauseRecord pause_record(this->ini->video.inactivity_timeout);

            if (this->verbose) {
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const time_t start_time = time(NULL);
            if (this->ini->debug.performance & 0x8000) {
                this->write_performance_log(start_time);
            }

            const timeval time_mark = { this->select_timeout_tv_sec, 0 };

            bool run_session = true;

            constexpr std::array<unsigned, 4> timers{{ 30*60, 10*60, 5*60, 1*60, }};
            unsigned osd_state = timers.size();
            const bool enable_osd = this->ini->globals.enable_osd;

            while (run_session) {
                unsigned max = 0;
                fd_set rfds;
                fd_set wfds;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                timeval timeout = time_mark;

                front_event.add_to_fd_set(rfds, max, timeout);
                if (this->front->capture) {
                    this->front->capture->capture_event.add_to_fd_set(rfds, max, timeout);
                }
                TODO("Looks like acl and mod can be unified into a common class, where events can happen");
                TODO("move ptr_auth_event to acl");
                if (this->acl) {
                    this->ptr_auth_event->add_to_fd_set(rfds, max, timeout);
                }
                mm.mod->get_event().add_to_fd_set(rfds, max, timeout);

                const bool has_pending_data =
                    (front_event.st->tls && SSL_pending(front_event.st->allocated_ssl));
                if (has_pending_data)
                    memset(&timeout, 0, sizeof(timeout));

                int num = select(max + 1, &rfds, &wfds, 0, &timeout);

                if (num < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                    // EBADF: means fd has been closed (by me) or as already returned an error on another call
                    // EINVAL: invalid value in timeout (my fault again)
                    // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)

                    LOG(LOG_ERR, "Proxy data wait loop raised error %u : %s", errno, strerror(errno));
                    run_session = false;
                    continue;
                }

                time_t now = time(NULL);
                if (this->ini->debug.performance & 0x8000) {
                    this->write_performance_log(now);
                }

                if (front_event.is_set(rfds) ||
                    (front_event.st->tls && SSL_pending(front_event.st->allocated_ssl))) {
                    try {
                        this->front->incoming(*mm.mod);
                    } catch (...) {
                        run_session = false;
                        continue;
                    };
                }

                try {
                    if (this->front->up_and_running) {
                        if (this->ini->video.inactivity_pause
                            && mm.connected
                            && this->front->capture) {
                            pause_record.check(now, *this->front);
                        }

                        // Process incoming module trafic
                        if (mm.mod->get_event().is_set(rfds)) {
                            mm.mod->draw_event(now);

                            if (mm.mod->get_event().signal != BACK_EVENT_NONE) {
                                signal = mm.mod->get_event().signal;
                                mm.mod->get_event().reset();
                            }
                        }
                        // TODO: update only when new value incoming
                        if (this->ini->check_from_acl()) {
                            this->front->update_config(*this->ini);
                        }
                        if (this->front->capture
                            && this->front->capture->capture_event.is_set(rfds)) {
                            this->front->periodic_snapshot();
                        }
                        // Incoming data from ACL, or opening acl
                        if (!this->acl) {
                            if (!mm.last_module) { // acl never opened or closed by me (close box)
                                try {
                                    int client_sck = ip_connect(this->ini->globals.authip,
                                                                this->ini->globals.authport,
                                                                30,
                                                                1000,
                                                                this->ini->debug.auth);

                                    if (client_sck == -1) {
                                        LOG(LOG_ERR, "Failed to connect to authentifier");
                                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                                    }

                                    this->ptr_auth_trans = new SocketTransport( "Authentifier"
                                                                                , client_sck
                                                                                , this->ini->globals.authip
                                                                                , this->ini->globals.authport
                                                                                , this->ini->debug.auth
                                                                                );
                                    this->ptr_auth_event = new wait_obj(this->ptr_auth_trans);
                                    this->acl = new SessionManager( *this->ini
                                                                  , *front
                                                                  , *this->ptr_auth_trans
                                                                  , start_time // proxy start time
                                                                  , now        // acl start time
                                                                  );

                                    osd_state = [&](uint32_t enddata) -> unsigned {
                                        if (!enddata || enddata <= now) {
                                            return timers.size();
                                        }
                                        unsigned i = timers.rend() - std::lower_bound(
                                            timers.rbegin(), timers.rend(), enddata - start_time
                                        );
                                        return i ? i-1 : 0;
                                    }(this->ini->context.end_date_cnx.get());
                                    signal = BACK_EVENT_NEXT;
                                }
                                catch (...) {
                                    mm.invoke_close_box("No authentifier available",signal, now);
                                }
                            }
                        }
                        else {
                            if (this->ptr_auth_event->is_set(rfds)) {
                                // acl received updated values
                                this->acl->receive();
                            }
                        }

                        if (enable_osd) {
                            const uint32_t enddate = this->ini->context.end_date_cnx.get();
                            if (enddate
                            && osd_state < timers.size()
                            && enddate - now <= timers[osd_state]
                            && mm.is_up_and_running()) {
                                std::string mes;
                                mes.reserve(128);
                                const unsigned minutes = (enddate - now + 30) / 60;
                                mes += std::to_string(minutes);
                                mes += ' ';
                                mes += TR("minute", *this->ini);
                                if (minutes > 1) {
                                    mes += "s ";
                                } else {
                                    mes += ' ';
                                }
                                mes += TR("before_closing", *this->ini);
                                mm.osd_message(std::move(mes));
                                ++osd_state;
                            }
                        }

                        if (this->acl) {
                            run_session = this->acl->check(mm, now, front_trans, signal);
                        }
                        else if (signal == BACK_EVENT_STOP) {
                            mm.mod->get_event().reset();
                            run_session = false;
                        }
                        if (mm.last_module) {
                            if (this->acl) {
                                delete this->acl;
                                this->acl = NULL;
                            }
                        }
                    }
                } catch (Error & e) {
                    LOG(LOG_INFO, "Session::Session exception = %d!\n", e.id);
                    time_t now = time(NULL);
                    mm.invoke_close_box(e.errmsg(), signal, now);
                };
            }
            if (mm.mod) {
                mm.mod->disconnect();
            }
            this->front->disconnect();
        }
        catch (const Error & e) {
            LOG(LOG_INFO, "Session::Session Init exception = %d!\n", e.id);
        }
        catch(...) {
            LOG(LOG_INFO, "Session::Session other exception in Init\n");
        }
        LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        this->front->stop_capture();
    }

    ~Session() {
        if (this->ini->debug.performance & 0x8000) {
            this->write_performance_log(this->perf_last_info_collect_time + 3);
        }
        if (this->perf_file) {
            ::fclose(this->perf_file);
        }
        delete this->front;
        if (this->acl) { delete this->acl; }
        if (this->ptr_auth_event) { delete this->ptr_auth_event; }
        if (this->ptr_auth_trans) { delete this->ptr_auth_trans; }
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        if (!this->ini->context.session_id.get().is_empty()) {
            char new_session_file[256];
            snprintf( new_session_file, sizeof(new_session_file), "%s/session_%s.pid"
                    , PID_PATH , this->ini->context.session_id.get_cstr());
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", PID_PATH, child_pid);
            unlink(old_session_file);
        }
    }

private:
    void write_performance_log(time_t now) {
        if (!this->perf_last_info_collect_time) {
            REDASSERT(!this->perf_file);

            this->perf_last_info_collect_time = now - this->select_timeout_tv_sec;

            struct tm tm_;

            localtime_r(&this->perf_last_info_collect_time, &tm_);

            char filename[2048];
            snprintf(filename, sizeof(filename), "%s/rdpproxy,%04d%02d%02d-%02d%02d%02d,%d.perf",
                this->ini->video.record_tmp_path,
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
                  "%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu\n"
                , now
                , resource_usage.ru_utime.tv_sec, resource_usage.ru_utime.tv_usec   /* user CPU time used               */
                , resource_usage.ru_stime.tv_sec, resource_usage.ru_stime.tv_usec   /* system CPU time used             */
                , resource_usage.ru_maxrss                                          /* maximum resident set size        */
                , resource_usage.ru_ixrss                                           /* integral shared memory size      */
                , resource_usage.ru_idrss                                           /* integral unshared data size      */
                , resource_usage.ru_isrss                                           /* integral unshared stack size     */
                , resource_usage.ru_minflt                                          /* page reclaims (soft page faults) */
                , resource_usage.ru_majflt                                          /* page faults (hard page faults)   */
                , resource_usage.ru_nswap                                           /* swaps                            */
                , resource_usage.ru_inblock                                         /* block input operations           */
                , resource_usage.ru_oublock                                         /* block output operations          */
                , resource_usage.ru_msgsnd                                          /* IPC messages sent                */
                , resource_usage.ru_msgrcv                                          /* IPC messages received            */
                , resource_usage.ru_nsignals                                        /* signals received                 */
                , resource_usage.ru_nvcsw                                           /* voluntary context switches       */
                , resource_usage.ru_nivcsw                                          /* involuntary context switches     */
            );
            ::fflush(this->perf_file);
        }
        while (this->perf_last_info_collect_time + this->select_timeout_tv_sec <= now);
    }
};

#endif

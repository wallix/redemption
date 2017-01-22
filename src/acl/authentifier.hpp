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
             Raphael Zhou, Meng Tan, Jennifer Inthavong

  Session related with ACL
  find out the next module to run from context reading
*/


#pragma once

#include "utils/log.hpp"
#include "configs/config.hpp"
#include "acl_serializer.hpp"
#include "module_manager.hpp"
#include "front/front.hpp"

#include "utils/verbose_flags.hpp"


#include <fstream>
#include <iostream>
#include <string>
#include <time.h>

#define LOG_SESSION(normal_log, session_log, session_type, type, session_id,    \
        ip_client, ip_target, user, device, service, account, priority, format, \
        ...                                                                     \
    )                                                                           \
    LOGCHECK__REDEMPTION__INTERNAL((                                            \
        LOG_REDEMPTION_FORMAT_CHECK(format, __VA_ARGS__),                       \
        LOGSYSLOG_REDEMPTION_SESSION_INTERNAL(                                  \
            normal_log,                                                         \
            session_log,                                                        \
            session_type, type, session_id, ip_client, ip_target,               \
            user, device, service, account, priority,                           \
            "%s (%d/%d) -- type='%s'%s" format,                                 \
            "[%s Session] "                                                     \
                "type='%s' "                                                    \
                "session_id='%s' "                                              \
                "client_ip='%s' "                                               \
                "target_ip='%s' "                                               \
                "user='%s' "                                                    \
                "device='%s' "                                                  \
                "service='%s' "                                                 \
                "account='%s'%s"                                                \
                format,                                                         \
            ((*format) ? " " : ""),                                             \
            __VA_ARGS__                                                         \
        ), 1)                                                                   \
    )

namespace {
    template<class... Ts>
    void LOGSYSLOG_REDEMPTION_SESSION_INTERNAL(
        bool normal_log,
        bool session_log,

        const char * session_type,
        const char * type,
        const char * session_id,
        const char * ip_client,
        const char * ip_target,
        const char * user,
        const char * device,
        const char * service,
        const char * account,

        int priority,
        const char *format_with_pid,
        const char *format2,
        Ts const & ... args
    ) {
#if defined(LOGNULL)
        (void)normal_log;
        (void)session_log;
        (void)session_type;
        (void)type;
        (void)session_id;
        (void)ip_client;
        (void)ip_target;
        (void)user;
        (void)device;
        (void)service;
        (void)account;
        (void)priority;
        (void)format_with_pid;
        (void)format2;
        (void)std::initializer_list<int>{(void(args), 1)...};
#else
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wformat-nonliteral")

        # if defined(LOGPRINT) || defined(IN_IDE_PARSER)
        (void)priority;
        #  define LOGSYSLOG_REDEMPTION_SESSION_INTERNAL_PRINTF printf
        # else
        #  define LOGSYSLOG_REDEMPTION_SESSION_INTERNAL_PRINTF(...) syslog(priority, __VA_ARGS__)
        # endif
        if (normal_log) {
            LOGSYSLOG_REDEMPTION_SESSION_INTERNAL_PRINTF(
                format_with_pid,
                prioritynames[priority], getpid(), getpid(),
                type, args...
            );
        }
        if (session_log) {
            LOGSYSLOG_REDEMPTION_SESSION_INTERNAL_PRINTF(
                format2,
                session_type,
                type,
                session_id,
                ip_client,
                ip_target,
                user,
                device,
                service,
                account,
                args...
             );
        }
        # undef LOGSYSLOG_REDEMPTION_SESSION_INTERNAL_PRINTF

        REDEMPTION_DIAGNOSTIC_POP
#endif
    }
}

class Authentifier : public auth_api {

    AclSerializer * acl_serial;

    mutable std::string session_type;


public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        state = 0x10,
    };

    Authentifier(Verbose verbose)
        : acl_serial(nullptr)
        , verbose(verbose)
    {
        if (this->verbose & Verbose::state) {
            LOG(LOG_INFO, "auth::Authentifier");
        }
    }
    
    void set_acl_serial(AclSerializer * acl_serial)
    {
        this->acl_serial = acl_serial;
    }

    ~Authentifier() override {
        if (this->verbose & Verbose::state) {
            LOG(LOG_INFO, "auth::~Authentifier");
        }
    }

public:
    bool check(MMApi & mm, time_t now, BackEvent_t & signal, BackEvent_t & front_signal, bool & has_user_activity) {
        //LOG(LOG_INFO, "================> ACL check: now=%u, signal=%u",
        //    (unsigned)now, static_cast<unsigned>(signal));
        if (signal == BACK_EVENT_STOP) {
            // here, mm.last_module should be false only when we are in login box
            mm.mod->get_event().reset();
            return false;
        }

        if (mm.last_module) {
            // at a close box (mm.last_module is true),
            // we are only waiting for a stop signal
            // and Authentifier should not exist anymore.
            return true;
        }

        const uint32_t enddate = this->acl_serial->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            const char * message = TR("session_out_time", language(this->acl_serial->ini));
            mm.invoke_close_box(message, signal, now);

            return true;
        }

        // Close by rejeted message received
        if (!this->acl_serial->ini.get<cfg::context::rejected>().empty()) {
            this->acl_serial->ini.set<cfg::context::auth_error_message>(this->acl_serial->ini.get<cfg::context::rejected>());
//            const char * error_message = this->acl_serial->ini.get<cfg::context::rejected>();
//            this->set_auth_error_message(error_message);
            LOG(LOG_INFO, "Close by Rejected message received : %s",
                this->acl_serial->ini.get<cfg::context::rejected>());
            this->acl_serial->ini.set_acl<cfg::context::rejected>("");
            mm.invoke_close_box(nullptr, signal, now);
            return true;
        }

        // Keep Alive
        if (this->acl_serial->keepalive.check(now, this->acl_serial->ini)) {
            mm.invoke_close_box(TR("miss_keepalive", language(this->acl_serial->ini)), signal, now);
            return true;
        }

        // Inactivity management
        if (this->acl_serial->inactivity.check_user_activity(now, has_user_activity)) {
            mm.invoke_close_box(TR("close_inactivity", language(this->acl_serial->ini)), signal, now);
            return true;
        }

        // Manage module (refresh or next)
        if (this->acl_serial->ini.changed_field_size()) {
            if (mm.connected) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                this->acl_serial->send_acl_data();
            }
            else if (signal == BACK_EVENT_REFRESH || signal == BACK_EVENT_NEXT) {
                this->acl_serial->remote_answer = false;
                this->acl_serial->send_acl_data();
            }
        }
        else if (this->acl_serial->remote_answer 
        || (signal == BACK_EVENT_RETRY_CURRENT) 
        || (front_signal == BACK_EVENT_NEXT)) {
            this->acl_serial->remote_answer = false;
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
                // TODO signal management (refresh/next) should go to ModuleManager, it's basically the same behavior. It could be implemented by closing module then opening another one of the same kind
                mm.mod->refresh_context(this->acl_serial->ini);
                mm.mod->get_event().signal = BACK_EVENT_NONE;
                mm.mod->get_event().set();
            }
            else if ((signal == BACK_EVENT_NEXT) 
                    || (signal == BACK_EVENT_RETRY_CURRENT) 
                    || (front_signal == BACK_EVENT_NEXT)) {
                if ((signal == BACK_EVENT_NEXT) 
                   || (front_signal == BACK_EVENT_NEXT)) {
                    LOG(LOG_INFO, "===========> MODULE_NEXT");
                }
                else {
                    REDASSERT(signal == BACK_EVENT_RETRY_CURRENT);

                    LOG(LOG_INFO, "===========> MODULE_RETRY_CURRENT");
                }

                int next_state = (((signal == BACK_EVENT_NEXT) 
                                  || (front_signal == BACK_EVENT_NEXT)) ? mm.next_module() : MODULE_RDP);

                front_signal = BACK_EVENT_NONE;

                if (next_state == MODULE_TRANSITORY) {
                    this->acl_serial->remote_answer = false;

                    return true;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    mm.invoke_close_box(nullptr, signal, now);
                    return true;
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    this->acl_serial->keepalive.stop();
                }
                mm.remove_mod();
                try {
                    mm.new_mod(next_state, now, this);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        this->acl_serial->ini.set_acl<cfg::context::module>(STRMODULE_TRANSITORY);

                        signal = BACK_EVENT_NEXT;

                        this->acl_serial->remote_answer = false;

                        this->report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return true;
                    }
                    else if (e.id == ERR_RDP_SERVER_REDIR) {
                        // SET new target in ini
                        const char * host = char_ptr_cast(
                            this->acl_serial->ini.get<cfg::mod_rdp::redir_info>().host);
                        const char * password = char_ptr_cast(
                            this->acl_serial->ini.get<cfg::mod_rdp::redir_info>().password);
                        const char * username = char_ptr_cast(
                            this->acl_serial->ini.get<cfg::mod_rdp::redir_info>().username);
                        const char * change_user = "";
                        if (this->acl_serial->ini.get<cfg::mod_rdp::redir_info>().dont_store_username &&
                            (username[0] != 0)) {
                            LOG(LOG_INFO, "SrvRedir: Change target username to '%s'", username);
                            this->acl_serial->ini.set_acl<cfg::globals::target_user>(username);
                            change_user = username;
                        }
                        if (password[0] != 0) {
                            LOG(LOG_INFO, "SrvRedir: Change target password");
                            this->acl_serial->ini.set_acl<cfg::context::target_password>(password);
                        }
                        LOG(LOG_INFO, "SrvRedir: Change target host to '%s'", host);
                        this->acl_serial->ini.set_acl<cfg::context::target_host>(host);
                        char message[768] = {};
                        sprintf(message, "%s@%s", change_user, host);
                        this->report("SERVER_REDIRECTION", message);
                        this->acl_serial->remote_answer = true;
                        signal = BACK_EVENT_NEXT;
                        return true;
                    }
                    else {
                        throw;
                    }
                }
                if (!this->acl_serial->keepalive.is_started() && mm.connected) {
                    this->acl_serial->keepalive.start(now);
                }
            }
            else
            {
                if (!this->acl_serial->ini.get<cfg::context::disconnect_reason>().empty()) {
                    this->acl_serial->ini.set<cfg::context::manager_disconnect_reason>(
                        this->acl_serial->ini.get<cfg::context::disconnect_reason>());
                    this->acl_serial->ini.get_ref<cfg::context::disconnect_reason>().clear();

                    this->acl_serial->ini.set_acl<cfg::context::disconnect_reason_ack>(true);
                }
            }
        }

        // LOG(LOG_INFO, "connect=%s acl_serial->check=%s", this->connected?"Y":"N", acl_serial->check()?"Y":"N");

        // AuthCHANNEL CHECK
        // if an answer has been received, send it to
        // rdp serveur via mod (should be rdp module)
        // TODO Check if this->mod is RDP MODULE
        if (mm.connected && this->acl_serial->ini.get<cfg::mod_rdp::auth_channel>()[0]) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            if (!this->acl_serial->ini.get<cfg::context::auth_channel_answer>().empty()) {
                // If set, transmit to auth_channel channel
                mm.mod->send_auth_channel_data(this->acl_serial->ini.get<cfg::context::auth_channel_answer>().c_str());
                // Erase the context variable
                this->acl_serial->ini.get_ref<cfg::context::auth_channel_answer>().clear();
            }
        }
        return true;
    }

    void receive() {
        if (this->verbose & Verbose::state) {
            LOG(LOG_INFO, "+++++++++++> ACL receive <++++++++++++++++");
        }
        // TODO: replace code below by new function call below in acl_serial
        // or maybe even inline at caller level
//        this->acl_serial->acl_receive(this->session_type, this->acl_serial->remote_answer);
        try {
            this->acl_serial->incoming();

            if (!this->acl_serial->ini.get<cfg::context::module>().compare("RDP") ||
                !this->acl_serial->ini.get<cfg::context::module>().compare("VNC")) {
                this->session_type = this->acl_serial->ini.get<cfg::context::module>();
            }

            this->acl_serial->remote_answer = true;
        } catch (...) {
            // acl connection lost
            this->acl_serial->ini.set_acl<cfg::context::authenticated>(false);

            if (this->acl_serial->ini.get<cfg::context::manager_disconnect_reason>().empty()) {
                this->acl_serial->ini.set_acl<cfg::context::rejected>(
                    TR("manager_close_cnx", language(this->acl_serial->ini)));
            }
            else {
                this->acl_serial->ini.set_acl<cfg::context::rejected>(
                    this->acl_serial->ini.get<cfg::context::manager_disconnect_reason>());
                this->acl_serial->ini.get_ref<cfg::context::manager_disconnect_reason>().clear();
            }
        }
    }

    void set_auth_channel_target(const char * target) override {
        this->acl_serial->ini.set_acl<cfg::context::auth_channel_target>(target);
    }

    void set_auth_error_message(const char * error_message) override {
        this->acl_serial->set_auth_error_message(error_message);
    }

    void report(const char * reason, const char * message) override {
        this->acl_serial->report(reason, message);
    }

    void disconnect_target() override {
        this->acl_serial->ini.set_acl<cfg::context::module>(STRMODULE_CLOSE);
    }

    // TODO: provide explicit init ? or even move that to ACLSerializer ?
    void log4(bool duplicate_with_pid, const char * type,
            const char * extra = nullptr) const override {
        const bool session_log =
            this->acl_serial->ini.get<cfg::session_log::enable_session_log>();
        if (!duplicate_with_pid && !session_log) return;

        /* Log to file */
        const bool log_redir = this->acl_serial->ini.get<cfg::session_log::session_log_redirection>();

        if (log_redir) {
            std::string const & filename = this->acl_serial->ini.get<cfg::session_log::log_path>();
            std::ofstream log_file(filename, std::ios::app);

            if(!log_file.is_open()) {
                LOG(LOG_INFO, "auth::bad SIEM log file creation");
            }
            else {
                time_t seconds = time(nullptr);
                struct tm * timeinfo = localtime(&seconds);
                log_file << (1900+timeinfo->tm_year) << "-";
                log_file << (timeinfo->tm_mon+1) << "-" << timeinfo->tm_mday << " " << timeinfo->tm_hour << ":" <<timeinfo->tm_min << ":" <<timeinfo->tm_sec << " ";
                log_file << " [" << (this->session_type.empty() ? "Neutral" : this->session_type.c_str()) << " Session] " << " " ;
                log_file << "type=" << type << " " ;
                log_file << "session_id=" << this->acl_serial->ini.get<cfg::context::session_id>() << " " ;
                log_file << "client_ip=" << this->acl_serial->ini.get<cfg::globals::host>() << " " ;
                log_file << "target_ip=" << (isdigit(this->acl_serial->ini.get<cfg::context::target_host>()[0]) ?
                                             this->acl_serial->ini.get<cfg::context::target_host>() :
                                             this->acl_serial->ini.get<cfg::context::ip_target>()) << " " ;
                log_file << "user=" << this->acl_serial->ini.get<cfg::globals::auth_user>() << " " ;
                log_file << "device=" << this->acl_serial->ini.get<cfg::globals::target_device>() << " " ;
                log_file << "service=" << this->acl_serial->ini.get<cfg::context::target_service>() << " " ;
                log_file << "account=" << this->acl_serial->ini.get<cfg::globals::target_user>() << " " ;
                log_file << (extra ? extra : "") << std::endl;
                log_file.close();
            }
        }

        /* Log to syslog */
        LOG_SESSION( duplicate_with_pid
                   , session_log

                   , (this->session_type.empty() ? "Neutral" : this->session_type.c_str())
                   , type
                   , this->acl_serial->ini.get<cfg::context::session_id>().c_str()
                   , this->acl_serial->ini.get<cfg::globals::host>().c_str()
                   , (isdigit(this->acl_serial->ini.get<cfg::context::target_host>()[0]) ?
                      this->acl_serial->ini.get<cfg::context::target_host>().c_str() :
                      this->acl_serial->ini.get<cfg::context::ip_target>().c_str())
                   , this->acl_serial->ini.get<cfg::globals::auth_user>().c_str()
                   , this->acl_serial->ini.get<cfg::globals::target_device>().c_str()
                   , this->acl_serial->ini.get<cfg::context::target_service>().c_str()
                   , this->acl_serial->ini.get<cfg::globals::target_user>().c_str()

                   , LOG_INFO
                   , "%s"
                   , (extra ? extra : "")
                   );
    }
};


class PauseRecord {
    // Stop record on inactivity Variables
    bool stop_record_inactivity;
    time_t stop_record_time;
    time_t last_record_activity_time;
    uint64_t last_total_received;
    uint64_t last_total_sent;
    Front & front;
    MMApi & mm;
    Inifile & ini;

public:
    explicit PauseRecord(
        std::chrono::seconds timeout,
        Front & front, MMApi & mm, Inifile & ini)
    : stop_record_inactivity(false)
    , stop_record_time(std::max<time_t>(timeout.count(), 30))
    , last_record_activity_time(0)
    , last_total_received(0)
    , last_total_sent(0)
    , front(front)
    , mm(mm)
    , ini(ini)
    {
    }

    void check(time_t now, Front & front) {
        // Procedure which stops the recording on inactivity
        if (this->last_record_activity_time == 0) this->last_record_activity_time = now;
        if ((front.get_total_received() == this->last_total_received)
            && (front.get_total_sent() == this->last_total_sent)) {
            if (!this->stop_record_inactivity &&
                (now > this->last_record_activity_time + this->stop_record_time)) {
                this->stop_record_inactivity = true;
//                front.can_be_pause_capture();
            }
        }
        else {
            this->last_record_activity_time = now;
            this->last_total_received = front.get_total_received();
            this->last_total_sent = front.get_total_sent();
            // front.trans->reset_quantum_sent();
            // Here we only reset the quantum sent
            // because Check() will already reset the
            // quantum received when checking for inactivity
            if (this->stop_record_inactivity) {
                this->stop_record_inactivity = false;
//                if (front.can_be_resume_capture()) {
//                    if (this->acl_serial->ini.get<cfg::globals::bogus_refresh_rect>() &&
//                        this->acl_serial->ini.get<cfg::globals::allow_using_multiple_monitors>() &&
//                        (this->front.client_info.cs_monitor.monitorCount > 1)) {
//                        this->mm.mod->rdp_suppress_display_updates();
//                        this->mm.mod->rdp_allow_display_updates(0, 0,
//                            this->front.client_info.width, this->front.client_info.height);
//                    }
//                    this->mm.mod->rdp_input_invalidate(Rect( 0, 0, this->front.client_info.width, this->front.client_info.height));
//                }
            }
        }
    }
};


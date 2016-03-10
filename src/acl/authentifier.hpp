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

  Session related with ACL
  find out the next module to run from context reading
*/

#ifndef _REDEMPTION_ACL_AUTHENTIFIER_HPP_
#define _REDEMPTION_ACL_AUTHENTIFIER_HPP_

#include "log.hpp"
#include "config.hpp"
#include "activity_checker.hpp"
#include "acl_serializer.hpp"
#include "module_manager.hpp"
#include "front.hpp"

class KeepAlive {
    // Keep alive Variables
    int  grace_delay;
    long timeout;
    long renew_time;
    bool wait_answer;     // true when we are waiting for a positive response
                          // false when positive response has been received and
                          // timers have been set to new timers.
    uint32_t verbose;
    bool connected;

public:
    KeepAlive(int _grace_delay, uint32_t verbose)
        : grace_delay(_grace_delay)
        , timeout(0)
        , renew_time(0)
        , wait_answer(false)
        , verbose(verbose)
        , connected(false)
    {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "KEEP ALIVE CONSTRUCTOR");
        }
    }

    ~KeepAlive() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "KEEP ALIVE DESTRUCTOR");
        }
    }

    bool is_started() {
        return this->connected;
    }

    void start(time_t now) {
        this->connected = true;
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::start_keep_alive");
        }
        this->timeout    = now + 2 * this->grace_delay;
        this->renew_time = now + this->grace_delay;
    }

    void stop() {
        this->connected = false;
    }

    bool check(time_t now, Inifile & ini) {
        if (this->connected) {
            // LOG(LOG_INFO, "now=%u timeout=%u  renew_time=%u wait_answer=%s grace_delay=%u", now, this->timeout, this->renew_time, this->wait_answer?"Y":"N", this->grace_delay);
            // Keep alive timeout
            if (now > this->timeout) {
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                // mm.invoke_close_box("Missed keepalive from ACL", signal, now);
                return true;
            }

            // LOG(LOG_INFO, "keepalive state ask=%s bool=%s\n",
            //     ini.is_asked<cfg::context::keepalive>()?"Y":"N",
            //     ini.get<cfg::context::keepalive>()?"Y":"N");

            // Keepalive received positive response
            if (this->wait_answer
                && !ini.is_asked<cfg::context::keepalive>()
                && ini.get<cfg::context::keepalive>()) {
                if (this->verbose & 0x10) {
                    LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
                }
                this->timeout    = now + 2*this->grace_delay;
                this->renew_time = now + this->grace_delay;
                this->wait_answer = false;
            }

            // Keep alive asking for an answer from ACL
            if (!this->wait_answer
                && (now > this->renew_time)) {

                this->wait_answer = true;

                ini.ask<cfg::context::keepalive>();
            }
        }
        return false;
    }
};

class Inactivity {
    // Inactivity management
    // let t be the timeout of the blocking select in session loop,
    // the effective inactivity timeout detection will be between
    // inactivity_timeout and inactivity_timeout + t.
    // hence we should have t << inactivity_timeout.
    time_t inactivity_timeout;
    time_t last_activity_time;

    ActivityChecker & checker;

    uint32_t verbose;

public:
    Inactivity(ActivityChecker & checker, uint32_t timeout, time_t start, uint32_t verbose)
    : inactivity_timeout((timeout>30)?timeout:30)
    , last_activity_time(start)
    , checker(checker)
    , verbose(verbose)
    {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "INACTIVITY CONSTRUCTOR");
        }
    }

    ~Inactivity() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "INACTIVITY DESTRUCTOR");
        }
    }

    bool check(time_t now) {
        if (!this->checker.check_and_reset_activity()) {
            if (now > this->last_activity_time + this->inactivity_timeout) {
                LOG(LOG_INFO, "Session User inactivity : closing");
                // mm.invoke_close_box("Connection closed on inactivity", signal, now);
                return true;
            }
        }
        else {
            this->last_activity_time = now;
        }
        return false;
    }
};

class SessionManager : public auth_api {
    Inifile & ini;

    AclSerializer acl_serial;

    bool remote_answer;       // false initialy, set to true once response is
                              // received from acl and asked_remote_answer is
                              // set to false

    uint32_t verbose;

    KeepAlive keepalive;
    Inactivity inactivity;

    bool wait_for_capture;

    mutable std::string session_type;

public:
    SessionManager(Inifile & ini, ActivityChecker & activity_checker, Transport & _auth_trans, time_t start_time, time_t acl_start_time)
        : ini(ini)
        , acl_serial(ini, _auth_trans, ini.get<cfg::debug::auth>())
        , remote_answer(false)
        //, start_time(start_time)
        //, acl_start_time(acl_start_time)
        , verbose(ini.get<cfg::debug::auth>())
        , keepalive(ini.get<cfg::globals::keepalive_grace_delay>(), ini.get<cfg::debug::auth>())
        , inactivity(activity_checker, ini.get<cfg::globals::session_timeout>(),
                     acl_start_time, ini.get<cfg::debug::auth>())
        , wait_for_capture(true)
    {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::SessionManager");
        }
    }

    ~SessionManager() override {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::~SessionManager");
        }
    }

public:
    bool check(MMApi & mm, time_t now, BackEvent_t & signal) {
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

        const uint32_t enddate = this->ini.get<cfg::context::end_date_cnx>();
        if (enddate != 0 && (static_cast<uint32_t>(now) > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            const char * message = TR("session_out_time", language(this->ini));
            mm.invoke_close_box(message, signal, now);

            return true;
        }

        // Close by rejeted message received
        if (!this->ini.get<cfg::context::rejected>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(this->ini.get<cfg::context::rejected>());
            LOG(LOG_INFO, "Close by Rejected message received : %s", this->ini.get<cfg::context::rejected>().c_str());
            this->ini.set_acl<cfg::context::rejected>("");
            mm.invoke_close_box(nullptr, signal, now);
            return true;
        }

        // Keep Alive
        if (this->keepalive.check(now, this->ini)) {
            mm.invoke_close_box(TR("miss_keepalive", language(this->ini)), signal, now);
            return true;
        }

        // Inactivity management
        if (this->inactivity.check(now)) {
            mm.invoke_close_box(TR("close_inactivity", language(this->ini)), signal, now);
            return true;
        }

        // Manage module (refresh or next)
        if (this->ini.changed_field_size()) {
            if (mm.connected) {
                // send message to acl with changed values when connected to
                // a module (rdp, vnc, xup ...) and something changed.
                // used for authchannel and keepalive.
                this->ask_acl();
            }
            else if (signal == BACK_EVENT_REFRESH || signal == BACK_EVENT_NEXT) {
                this->remote_answer = false;
                this->ask_acl();
            }
        }
        else if (this->remote_answer || (signal == BACK_EVENT_RETRY_CURRENT)) {
            this->remote_answer = false;
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
                TODO("signal management (refresh/next) should go to ModuleManager, "
                     "it's basically the same behavior. It could be implemented by "
                     "closing module then opening another one of the same kind");
                mm.mod->refresh_context(this->ini);
                mm.mod->get_event().signal = BACK_EVENT_NONE;
                mm.mod->get_event().set();
            }
            else if ((signal == BACK_EVENT_NEXT) || (signal == BACK_EVENT_RETRY_CURRENT)) {
                if (signal == BACK_EVENT_NEXT) {
                    LOG(LOG_INFO, "===========> MODULE_NEXT");
                }
                else {
                    REDASSERT(signal == BACK_EVENT_RETRY_CURRENT);

                    LOG(LOG_INFO, "===========> MODULE_RETRY_CURRENT");
                }

                int next_state = ((signal == BACK_EVENT_NEXT) ? mm.next_module() : MODULE_RDP);

                if (next_state == MODULE_TRANSITORY) {
                    this->remote_answer = false;

                    return true;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    mm.invoke_close_box(nullptr, signal, now);
                    return true;
                }
                if (next_state == MODULE_INTERNAL_CLOSE_BACK) {
                    mm.stop_record();
                    this->keepalive.stop();
                }
                mm.remove_mod();
                try {
                    mm.new_mod(next_state, now, this);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        this->ini.set_acl<cfg::context::module>(STRMODULE_TRANSITORY);

                        signal = BACK_EVENT_NEXT;

                        this->remote_answer = false;

                        this->report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return true;
                    }
                    else if (e.id == ERR_RDP_SERVER_REDIR) {
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
                        this->report("SERVER_REDIRECTION", message);
                        this->remote_answer = true;
                        signal = BACK_EVENT_NEXT;
                        return true;
                    }
                    else {
                        throw;
                    }
                }
                if (!this->keepalive.is_started() && mm.connected) {
                    this->keepalive.start(now);
                }
            }
        }
        if (this->wait_for_capture && mm.is_up_and_running()) {
            this->ini.check_record_config();
            mm.record(this);
            this->wait_for_capture = false;
        }

        // LOG(LOG_INFO, "connect=%s ini.check=%s", this->connected?"Y":"N", this->ini.check()?"Y":"N");

        // AuthCHANNEL CHECK
        // if an answer has been received, send it to
        // rdp serveur via mod (should be rdp module)
        TODO("Check if this->mod is RDP MODULE");
        if (mm.connected && this->ini.get<cfg::mod_rdp::auth_channel>()[0]) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            if (!this->ini.get<cfg::context::auth_channel_answer>().empty()) {
                // If set, transmit to auth_channel channel
                mm.mod->send_auth_channel_data(this->ini.get<cfg::context::auth_channel_answer>().c_str());
                // Erase the context variable
                this->ini.get_ref<cfg::context::auth_channel_answer>().clear();
            }
        }
        return true;
    }

    void receive() {
        LOG(LOG_INFO, "+++++++++++> ACL receive <++++++++++++++++");
        try {
            this->acl_serial.incoming();

            if (!this->ini.get<cfg::context::module>().compare("RDP") ||
                !this->ini.get<cfg::context::module>().compare("VNC")) {
                this->session_type = this->ini.get<cfg::context::module>().c_str();
            }

            this->remote_answer = true;
        } catch (...) {
            // acl connection lost
            this->ini.set_acl<cfg::context::authenticated>(false);
            this->ini.set_acl<cfg::context::rejected>(TR("manager_close_cnx", language(this->ini)));
        }
    }

    void ask_acl() {
        LOG(LOG_INFO, "Ask acl\n");
        this->acl_serial.send_acl_data();
    }

    void set_auth_channel_target(const char * target) override {
        this->ini.set_acl<cfg::context::auth_channel_target>(target);
    }

    //void set_auth_channel_result(const char * result) override
    //{
    //    this->ini.get<cfg::context::auth_channel_result>().set_from_cstr(result);
    //}

    void set_auth_error_message(const char * error_message) override {
        this->ini.set<cfg::context::auth_error_message>(error_message);
    }

    void report(const char * reason, const char * message) override {
        this->ini.ask<cfg::context::keepalive>();

        char report[1024];
        snprintf(report, sizeof(report), "%s:%s:%s", reason,
            this->ini.get<cfg::globals::target_device>().c_str(), message);
        this->ini.set_acl<cfg::context::reporting>(report);

        this->ask_acl();
    }

    void disconnect_target() override {
        this->ini.set_acl<cfg::context::module>(STRMODULE_CLOSE);
    }

    void log4(bool duplicate_with_pid, const char * type,
            const char * extra = nullptr) const override {
        const bool session_log =
            this->ini.get<cfg::session_log::enable_session_log>();
        if (!duplicate_with_pid && !session_log) return;

        LOG_SESSION( duplicate_with_pid
                   , session_log

                   , (this->session_type.empty() ? "Neutral" : this->session_type.c_str())
                   , type
                   , this->ini.get<cfg::context::session_id>().c_str()
                   , this->ini.get<cfg::globals::auth_user>().c_str()
                   , this->ini.get<cfg::globals::target_device>().c_str()
                   , this->ini.get<cfg::context::target_service>().c_str()
                   , this->ini.get<cfg::globals::target_user>().c_str()

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

public:
    explicit PauseRecord(time_t timeout)
        : stop_record_inactivity(false)
        , stop_record_time((timeout > 30)?timeout:30)
        , last_record_activity_time(0)
        , last_total_received(0)
        , last_total_sent(0)
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
                front.pause_capture();
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
                front.resume_capture();
                // resume capture
            }
        }
    }
};

#endif

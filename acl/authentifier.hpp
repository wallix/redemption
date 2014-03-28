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

#include <unistd.h>
#include <fcntl.h>

#include "stream.hpp"
#include "config.hpp"
#include "netutils.hpp"
#include "acl_serializer.hpp"
#include "module_manager.hpp"
#include "translation.hpp"

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

    bool check(time_t now, Inifile * ini) {
        if (this->connected) {
            // LOG(LOG_INFO, "now=%u timeout=%u  renew_time=%u wait_answer=%s grace_delay=%u", now, this->timeout, this->renew_time, this->wait_answer?"Y":"N", this->grace_delay);
            // Keep alive timeout
            if (now > this->timeout) {
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                // mm.invoke_close_box("Missed keepalive from ACL", signal, now);
                return true;
            }

            // LOG(LOG_INFO, "keepalive state ask=%s bool=%s\n",
            //     ini->context_is_asked(AUTHID_KEEPALIVE)?"Y":"N",
            //     ini->context_get_bool(AUTHID_KEEPALIVE)?"Y":"N");

            // Keepalive received positive response
            if (this->wait_answer
                && !ini->context_is_asked(AUTHID_KEEPALIVE)
                && ini->context_get_bool(AUTHID_KEEPALIVE)) {
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

                ini->context_ask(AUTHID_KEEPALIVE);
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
    uint64_t last_total_received;

    uint32_t verbose;

public:
    Inactivity(uint32_t max_tick, time_t start, uint32_t verbose)
        : inactivity_timeout(max_tick?30*max_tick:10)
        , last_activity_time(start)
        , last_total_received(0)
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

    bool check(time_t now, Transport & trans) {
        if (trans.total_received == this->last_total_received) {
            if (now > this->last_activity_time + this->inactivity_timeout) {
                LOG(LOG_INFO, "Session User inactivity : closing");
                // mm.invoke_close_box("Connection closed on inactivity", signal, now);
                return true;
            }
        }
        else {
            this->last_activity_time = now;
            this->last_total_received = trans.total_received;
        }
        return false;
    }
};

class SessionManager : public auth_api {
    Inifile * ini;

public:
    AclSerializer acl_serial;

    bool remote_answer;       // false initialy, set to true once response is received from acl
                              // and asked_remote_answer is set to false
    time_t start_time;
    time_t acl_start_time;

    uint32_t verbose;

private:
    KeepAlive keepalive;
    Inactivity inactivity;

    bool wait_for_capture;

public:
    SessionManager(Inifile * ini, Transport & _auth_trans, time_t start_time, time_t acl_start_time)
        : ini(ini)
        , acl_serial(AclSerializer(ini, _auth_trans, ini->debug.auth))
        , remote_answer(false)
        , start_time(start_time)
        , acl_start_time(acl_start_time)
        , verbose(ini->debug.auth)
        , keepalive(KeepAlive(ini->globals.keepalive_grace_delay, ini->debug.auth))
        , inactivity(Inactivity(ini->globals.max_tick, acl_start_time, ini->debug.auth))
        , wait_for_capture(true)
    {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::SessionManager");
        }
        this->ini->to_send_set.insert(AUTHID_KEEPALIVE);
    }

    virtual ~SessionManager() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::~SessionManager");
        }
    }

public:
    bool check(MMApi & mm, time_t now, Transport & trans, BackEvent_t & signal) {
       // LOG(LOG_INFO, "================> ACL check: now=%u, signal=%u",
       //     (unsigned)now, static_cast<unsigned>(signal));
        if (signal == BACK_EVENT_STOP) {
            // here, mm.last_module should be false only when we are in login box
            mm.mod->event.reset();
            return false;
        }

        if (mm.last_module) {
            // at a close box (mm.last_module is true),
            // we are only waiting for a stop signal
            // and Authentifier should not exist anymore.
            return true;
        }

        long enddate = this->ini->context.end_date_cnx.get();
        if (enddate != 0 && (now > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            const char * message = "Session is out of allowed timeframe";
            if (this->ini) {
                message = TR("session_out_time", *(this->ini));
            }
            mm.invoke_close_box(message, signal, now);

            return true;
        }

        // Close by rejeted message received
        if (!this->ini->context.rejected.is_empty()) {
            this->ini->context.auth_error_message.copy_str(this->ini->context.rejected.get());
            LOG(LOG_INFO, "Close by Rejected message received : %s", this->ini->context.rejected.get_cstr());
            this->ini->context.rejected.set_empty();
            mm.invoke_close_box(NULL, signal, now);
            return true;
        }

        // Keep Alive
        if (this->keepalive.check(now, this->ini)) {
            if (this->ini) {
                mm.invoke_close_box(TR("miss_keepalive", *(this->ini)), signal, now);
            }
            else {
                mm.invoke_close_box("Missed keepalive from ACL", signal, now);
            }
            return true;
        }

        // Inactivity management
        if (this->inactivity.check(now, trans)) {
            if (this->ini) {
                mm.invoke_close_box(TR("close_inactivity", *(this->ini)), signal, now);
            }
            else {
                mm.invoke_close_box("Connection closed on inactivity", signal, now);
            }
            return true;
        }

        // Manage module (refresh or next)
        if (this->ini->check()) {
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
        else if (this->remote_answer) {
            if (signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
                TODO("signal management (refresh/next) should go to ModuleManager, "
                     "it's basically the same behavior. It could be implemented by "
                     "closing module then opening another one of the same kind");
                mm.mod->refresh_context(*this->ini);
                mm.mod->event.signal = BACK_EVENT_NONE;
                mm.mod->event.set();
            }
            else if (signal == BACK_EVENT_NEXT) {
                LOG(LOG_INFO, "===========> MODULE_NEXT");
                int next_state = mm.next_module();

                if (next_state == MODULE_TRANSITORY) {
                    this->remote_answer = false;

                    return true;
                }

                signal = BACK_EVENT_NONE;
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    mm.invoke_close_box(NULL, signal, now);
                    return true;
                }
                mm.remove_mod();
                try {
                    mm.new_mod(next_state, now, this);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        this->ini->context.module.set_from_cstr("transitory");

                        signal = BACK_EVENT_NEXT;

                        this->remote_answer = false;
                        this->report("CONNECTION_FAILED",
                            "Failed to connect to remote TCP host.");

                        return true;
                    }
                    else {
                        throw e;
                    }
                }
                if (!this->keepalive.is_started() && mm.connected) {
                    this->keepalive.start(now);
                }
            }
        }
        if (this->wait_for_capture &&
            mm.is_up_and_running()) {
            mm.record(this);
            this->wait_for_capture = false;
        }

        // LOG(LOG_INFO, "connect=%s ini->check=%s", this->connected?"Y":"N", this->ini->check()?"Y":"N");

        // AuthCHANNEL CHECK
        // if an answer has been received, send it to
        // rdp serveur via mod (should be rdp module)
        TODO("Check if this->mod is RDP MODULE");
        if (mm.connected && this->ini->globals.auth_channel[0]) {
            // Get sesman answer to AUTHCHANNEL_TARGET
            if (!this->ini->context.authchannel_answer.get().is_empty()) {
                // If set, transmit to auth_channel channel
                mm.mod->send_auth_channel_data(this->ini->context.authchannel_answer.get_cstr());
                this->ini->context.authchannel_answer.use();
                // Erase the context variable
                this->ini->context.authchannel_answer.set_empty();
            }
        }
        return true;
    }

    void receive() {
        LOG(LOG_INFO, "+++++++++++> ACL receive <++++++++++++++++");
        try {
            this->acl_serial.incoming();
            this->remote_answer = true;
        } catch (...) {
            // acl connection lost
            this->ini->context.authenticated.set(false);
            if (this->ini) {
                this->ini->context.rejected.set_from_cstr(TR("manager_close_cnx", *(this->ini)));
            }
            else {
                this->ini->context.rejected.set_from_cstr("Connection closed by manager");
            }
        }
    }

    void ask_acl() {
        LOG(LOG_INFO, "Ask acl\n");
        this->acl_serial.send_acl_data();
    }

    virtual void set_auth_channel_target(const char * target)
    {
        this->ini->context.authchannel_target.set_from_cstr(target);
    }

    virtual void set_auth_channel_result(const char * result)
    {
        this->ini->context.authchannel_result.set_from_cstr(result);
    }

    virtual void report(const char * reason, const char * message)
    {
        this->ini->context_ask(AUTHID_KEEPALIVE);

        char report[1024];
        snprintf(report, sizeof(report), "%s:%s:%s", reason,
            this->ini->globals.target_device.get_cstr(), message);
        this->ini->context.reporting.set_from_cstr(report);

        this->ask_acl();
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
    PauseRecord(time_t timeout)
        : stop_record_inactivity(false)
        , stop_record_time((timeout > 30)?timeout:30)
        , last_record_activity_time(0)
        , last_total_received(0)
        , last_total_sent(0)
    {
    }

    ~PauseRecord() {}

    void check(time_t now, Front & front) {
        // Procedure which stops the recording on inactivity
        if (this->last_record_activity_time == 0) this->last_record_activity_time = now;
        if ((front.trans->total_received == this->last_total_received)
            && (front.trans->total_sent == this->last_total_sent)) {
            if (!this->stop_record_inactivity &&
                (now > this->last_record_activity_time + this->stop_record_time)) {
                this->stop_record_inactivity = true;
                front.pause_capture();
            }
        }
        else {
            this->last_record_activity_time = now;
            this->last_total_received = front.trans->total_received;
            this->last_total_sent = front.trans->total_sent;
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

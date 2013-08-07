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
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Raphael Zhou, Meng Tan

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

class SessionManager {
    Inifile * ini;

    // Keep alive Variables
    int  keepalive_grace_delay;
    long keepalive_time;
    long keepalive_renew_time;
    bool check_keepalive;     // true when we are waiting for a positive response
                              // false when positive response has been received and
                              // timers have been set to new timers.
    // Inactivity detection variables
    bool check_inactivity;
    long prev_remain;
    time_t inactivity_timeout;
    time_t last_activity_time;

public:
    AclSerializer acl_serial;

    bool lost_acl;            // false initialy, true when connection with acl is lost
    bool asked_remote_answer; // false initialy, set to true when a msg is sent to acl
    bool remote_answer;       // false initialy, set to true once response is received from acl
                              // and asked_remote_answer is set to false
    time_t start_time;
    time_t acl_start_time;

    uint32_t verbose;

    SessionManager(Inifile * ini, Transport & _auth_trans, time_t start_time, time_t acl_start_time)
        : ini(ini)
        , keepalive_grace_delay(ini->globals.keepalive_grace_delay)
        , keepalive_time(0)
        , keepalive_renew_time(0)
        , check_keepalive(false)
        , check_inactivity(true)
        , inactivity_timeout(ini->globals.max_tick?30*ini->globals.max_tick:10)
        , last_activity_time(acl_start_time)
        , acl_serial(AclSerializer(ini, _auth_trans, ini->debug.auth))
        , lost_acl(false)
        , asked_remote_answer(false)
        , remote_answer(false)
        , start_time(start_time)
        , acl_start_time(acl_start_time)
        , verbose(ini->debug.auth)
    {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::SessionManager");
        }
        this->prev_remain = 0;
        this->ini->to_send_set.insert(AUTHID_KEEPALIVE);
    }

    ~SessionManager() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::~SessionManager");
        }
    }

    void start_keepalive(time_t now) {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::start_keep_alive");
        }
        this->keepalive_time       = now + 2*this->keepalive_grace_delay;
        this->keepalive_renew_time = now + this->keepalive_grace_delay;
    }



public:

    bool check(MMApi & mm, time_t now, Transport & trans, BackEvent_t & signal) {
        // LOG(LOG_INFO, "================> ACL check: now=%u, signal=%u", (unsigned)now, (unsigned)signal);

        if (signal == BACK_EVENT_STOP) {
            // here, mm.last_module should be false only when we are in login box
            mm.mod->event.reset();
            return false;
        }
        if (mm.last_module) {
            // at a close box (mm.last_module is true),
            // we are only waiting for a stop signal
            return true;
        }

        long enddate = this->ini->context.end_date_cnx.get();
        if (enddate != 0 && (now > enddate)) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            mm.invoke_close_box("Session is out of allowed timeframe", signal, now);
            return true;
        }


        // Check if acl connection is lost.
        if (this->lost_acl) {
            LOG(LOG_INFO, "Connection with ACL is lost");
            mm.invoke_close_box("Connection closed by manager (ACL closed)", signal, now);
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

        // Keep alive
        if (this->keepalive_time) {

            // LOG(LOG_INFO, "now=%u keepalive_time=%u  keepalive_renew_time=%u check_keepalive=%s", now, this->keepalive_time, this->keepalive_renew_time, this->check_keepalive?"Y":"N");
            // Keep alive timeout
            if (now > this->keepalive_time) {
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                mm.invoke_close_box("Missed keepalive from ACL", signal, now);
                return true;
            }

            // LOG(LOG_INFO, "keepalive state ask=%s bool=%s\n",
            //     this->ini->context_is_asked(AUTHID_KEEPALIVE)?"Y":"N",
            //     this->ini->context_get_bool(AUTHID_KEEPALIVE)?"Y":"N");

            //Keepalive received positive response
            if (this->check_keepalive
                && !this->ini->context_is_asked(AUTHID_KEEPALIVE)
                && this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
                if (this->verbose & 0x10) {
                    LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
                }
                this->keepalive_time       = now + 2*this->keepalive_grace_delay;
                this->keepalive_renew_time = now + this->keepalive_grace_delay;
                this->check_keepalive = false;
            }

            // Keep alive asking for an answer from ACL
            if (!this->check_keepalive
                && (now > this->keepalive_renew_time)) {

                this->check_keepalive = true;

                this->ini->context_ask(AUTHID_KEEPALIVE);
                if (this->verbose & 0x10) {
                    LOG(LOG_INFO, "asked_remote_answer=%s", this->asked_remote_answer?"Y":"N");
                }
            }
        }   // if (this->keepalive_time)


        //Inactivity management
        // let t be the timeout of the blocking select in session loop,
        // the effective inactivity timeout detection will be between
        // inactivity_timeout and inactivity_timeout + t.
        // hence we should have t << inactivity_timeout.
        // for now, check_inactivity is not necessary but it
        // indicates that this part of code is about inactivity management
        if (this->check_inactivity) {
            if (trans.last_quantum_received == 0) {
                if (now > this->last_activity_time + this->inactivity_timeout) {
                    LOG(LOG_INFO, "Session User inactivity : closing");
                    mm.invoke_close_box("Connection closed on inactivity", signal, now);
                    return true;
                }
                if (this->verbose & 0x10) {
                    long remain = this->last_activity_time + this->inactivity_timeout - now;
                    if ((remain / 10) != this->prev_remain
                        && (remain != this->inactivity_timeout)) {
                        this->prev_remain = remain / 10;
                        LOG(LOG_INFO, "Session User inactivity : %d secs remaining before closing", remain);
                    }
                }
            }
            else {
                this->last_activity_time = now;
                trans.tick();
                if (this->verbose & 0x10) {
                    if (this->prev_remain != 0) {
                        LOG(LOG_INFO, "Session User inactivity : Timer reset");
                        this->prev_remain = 0;
                    }
                }
            }
        }


        // Manage module (refresh or next)
        TODO("Check the needs and reference of this->asked_remote_answer. "
             "Maybe could be replaced by ini->check() alone ?");
        if (!this->asked_remote_answer && this->ini->check()) {
            if (signal == BACK_EVENT_REFRESH || signal == BACK_EVENT_NEXT) {
                this->asked_remote_answer = true;
                this->remote_answer       = false;
                this->ask_acl();
            }
        }
        else {
            this->asked_remote_answer = false;

            if (this->remote_answer && signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                signal = BACK_EVENT_NONE;
                TODO("signal management (refresh/next) should go to ModuleManager, "
                     "it's basically the same behavior. It could be implemented by "
                     "closing module then opening another one of the same kind");
                mm.mod->refresh_context(*this->ini);
                mm.mod->event.signal = BACK_EVENT_NONE;
                mm.mod->event.set();
            }
            else if (this->remote_answer && signal == BACK_EVENT_NEXT) {

                LOG(LOG_INFO, "===========> MODULE_NEXT");
                signal = BACK_EVENT_NONE;
                int next_state = mm.next_module();
                if (next_state == MODULE_INTERNAL_CLOSE) {
                    mm.invoke_close_box(NULL, signal, now);
                    return true;
                }
                mm.remove_mod();
                try {
                    mm.new_mod(next_state,now);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        mm.invoke_close_box("Failed to connect to remote TCP host", signal, now);
                        return true;
                    }
                    else {
                        throw e;
                    }
                }
                if ((this->keepalive_time == 0) && mm.connected) {
                    this->start_keepalive(now);

                    mm.record();
                }
            }
        }

        // send message to acl with changed values when connected to
        // a module (rdp, vnc, xup ...) and something changed
        // used for authchannel and keepalive.

        // LOG(LOG_INFO, "connect=%s ini->check=%s", this->connected?"Y":"N", this->ini->check()?"Y":"N");

        if (mm.connected && this->ini->check()) {
            this->ask_acl();
        }

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
            if (!this->lost_acl) {
                this->acl_serial.incoming();
                this->remote_answer = true;
            }
        } catch (...) {
            // acl connection lost
            this->ini->context.authenticated.set(false);
            this->ini->context.rejected.set_from_cstr("Authentifier service failed");
            this->lost_acl = true;
        }
    }

    void ask_acl() {
        LOG(LOG_INFO, "Ask next module remote\n");
        this->acl_serial.ask_next_module_remote();
    }

};

class PauseRecordFunctor {
    // Stop record on inactivity Variables
    bool stop_record_inactivity;
    time_t stop_record_time;
    time_t last_record_activity_time;

public:

    PauseRecordFunctor()
        : stop_record_inactivity(false)
        , stop_record_time(30)               // stop recording on 30 seconds of inactivity
        , last_record_activity_time(0)
    {
    }
    ~PauseRecordFunctor() {}

    bool operator()(time_t now, Front & front) {
        // Procedure which stops the recording on inactivity
        if (this->last_record_activity_time == 0) this->last_record_activity_time = now;
        if ((front.trans->last_quantum_received == 0)
            && (front.trans->last_quantum_sent == 0)) {
            if (!this->stop_record_inactivity &&
                (now > this->last_record_activity_time + this->stop_record_time)) {
                this->stop_record_inactivity = true;
                front.pause_capture();
            }
        }
        else {
            this->last_record_activity_time = now;
            front.trans->reset_quantum_sent();
            // Here we only reset the quantum sent
            // because Check will already reset the
            // quantum received when checking for inactivity
            if (this->stop_record_inactivity) {
                this->stop_record_inactivity = false;
                front.resume_capture();
                // resume capture
            }
        }
        return true;
    }
};

#endif

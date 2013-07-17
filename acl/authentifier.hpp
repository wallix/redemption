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

    int  tick_count;
    int  max_tick;
    int  keepalive_grace_delay;
    long keepalive_time;
    long keepalive_renew_time;

public:
    BackEvent_t signal;
    AclSerializer acl_serial;


    bool lost_acl;            // false initialy, true when connection with acl is lost
    bool internal_domain;
    bool connected;
    bool last_module;         // indicating a last module (close modules)
    bool asked_remote_answer; // false initialy, set to true when a msg is sent to acl
    bool remote_answer;       // false initialy, set to true once response is received from acl
                              // and asked_remote_answer is set to false
    time_t start_time;        // never used ?
    time_t acl_start_time;    // never used ?
    uint32_t verbose;
    bool read_auth;

    SessionManager( Inifile * _ini, Transport & _auth_trans, time_t start_time
                    , time_t acl_start_time, int _keepalive_grace_delay, int _max_tick
                    , bool _internal_domain, uint32_t _verbose)
        : ini(_ini)
        , tick_count(0)
        , max_tick(_max_tick)
        , keepalive_grace_delay(_keepalive_grace_delay)
        , keepalive_time(0)
        , keepalive_renew_time(0)
        , signal(BACK_EVENT_NONE)
        , acl_serial(AclSerializer(_ini,_auth_trans,_verbose))
        , lost_acl(false)
        , internal_domain(_internal_domain)
        , connected(false)
        , last_module(false)
        , asked_remote_answer(false)
        , remote_answer(false)
        , start_time(start_time)
        , acl_start_time(acl_start_time)
        , verbose(_verbose)
        , read_auth(false) {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::SessionManager");
        }
    }

    ~SessionManager() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::~SessionManager");
        }
    }

    //    void start_keep_alive(long & keepalive_time)
    //    {
    //        if (this->verbose & 0x10) {
    //            LOG(LOG_INFO, "auth::start_keep_alive");
    //        }
    //        this->tick_count = 1;

    //        this->ini->context_ask(AUTHID_KEEPALIVE);
    //        this->acl_serial.send(AUTHID_KEEPALIVE);
    //        keepalive_time = ::time(NULL) + 30;
    //    }
    void start_keepalive() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::start_keep_alive");
        }

        this->tick_count           = 1;
        this->keepalive_time       =
        this->keepalive_renew_time = ::time(NULL) + this->keepalive_grace_delay;

        this->ini->to_send_set.insert(AUTHID_KEEPALIVE);
        // this->ini->context.keep_alive.set_to_send_field();

        this->ini->context_ask(AUTHID_KEEPALIVE);

        // this->signal = BACK_EVENT_REFRESH;
        //this->acl_serial.send(AUTHID_KEEPALIVE);
    }

    // Check movie start/stop/pause

    int get_mod_from_protocol() {
        if (this->verbose & 0x10) {
            LOG(LOG_INFO, "auth::get_mod_from_protocol");
        }
        // Initialy, it no protocol known and get_value should provide "ASK".
        const char * protocol = this->ini->context.target_protocol.get_value();
        //const char * protocol = this->ini->context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0);
        if (this->internal_domain) {
            const char * target = this->ini->globals.target_device.get_cstr();
            //const char * target = this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0);
            if (0 == strncmp(target, "autotest", 8)) {
                protocol = "INTERNAL";
            }
        }
        TODO("connected information is known by module_manager, it should be module manager that change this state when opening some modules")
        int res = MODULE_EXIT;
        if (!this->connected && 0 == strncasecmp(protocol, "RDP", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol RDP");
            }
            res = MODULE_RDP;
            this->connected = true;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "APPLICATION", 12)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol APPLICATION");
            }
            res = MODULE_RDP;
            this->connected = true;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "VNC", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol VNC");
            }
            res = MODULE_VNC;
            this->connected = true;
        }
        else if (!this->connected && 0 == strncasecmp(protocol, "XUP", 4)) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol XUP");
            }
            res = MODULE_XUP;
            this->connected = true;
        }
        else if (strncasecmp(protocol, "INTERNAL", 8) == 0) {
            const char * target = this->ini->globals.target_device.get_cstr();
            // const char * target = this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0);
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL");
            }
            if (0 == strcmp(target, "bouncer2")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL bouncer2");
                }
                res = MODULE_INTERNAL_BOUNCER2;
            }
            else if (0 == strncmp(target, "autotest", 8)) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL test");
                }
                const char * user = this->ini->globals.target_user.get_cstr();
                // const char * user = this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0);
                size_t len_user = strlen(user);
                strncpy(this->ini->context.movie, user, sizeof(this->ini->context.movie));
                this->ini->context.movie[sizeof(this->ini->context.movie) - 1] = 0;
                if (0 != strcmp(".mwrm", user + len_user - 5)) {
                    strcpy(this->ini->context.movie + len_user, ".mwrm");
                }
                res = MODULE_INTERNAL_TEST;
            }
            else if (0 == strcmp(target, "selector")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL selector");
                }
                res = MODULE_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (0 == strcmp(target, "login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "rwl_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else if (0 == strcmp(target, "rwl")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "close")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL close");
                }
                res = MODULE_INTERNAL_CLOSE;
                // this->last_module = true;
            }
            else if (0 == strcmp(target, "widget2_close")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_close");
                }
                res = MODULE_INTERNAL_WIDGET2_CLOSE;
                // this->last_module = true;
            }
            else if (0 == strcmp(target, "widget2_dialog")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_dialog");
                }
                res = MODULE_INTERNAL_WIDGET2_DIALOG;
            }
            else if (0 == strcmp(target, "widget2_message")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MODULE_INTERNAL_WIDGET2_MESSAGE;
            }
            else if (0 == strcmp(target, "widget2_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "widget2_rwl")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "widget2_rwl_login")) {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else {
                if (this->verbose & 0x4) {
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL card");
                }
                res = MODULE_INTERNAL_CARD;
            }
            // TODO("it looks strange we have to reset connect to false. Once connected is true it should stay so until the end of the session")
            // this->connected = false;
        }
        else if (this->connected) {
            if (this->verbose & 0x4) {
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_close");
            }
            res = MODULE_INTERNAL_WIDGET2_CLOSE;
            // this->last_module = true;
            // TODO("it looks strange we have to reset connect to false. Once connected is true it should stay so until the end of the session")
            // this->connected   = false;
        }
        else {
            LOG(LOG_WARNING, "Unsupported target protocol %c%c%c%c",
                protocol[0], protocol[1], protocol[2], protocol[3]);
            res = MODULE_EXIT;
        }
        return res;
    }

protected:
    bool invoke_mod_close(MMApi & mm, const char * auth_error_message) {
        LOG(LOG_INFO, "invoke_mod_close");
        if (this->last_module) {
            return false;
        }
        this->ini->context.auth_error_message.copy_c_str(auth_error_message);
        this->asked_remote_answer = false;
        this->last_module         = true;
        this->keepalive_time      = 0;
        mm.remove_mod();
        // TODO("it looks strange we have to reset connect to false. Once connected is true it should stay so until the end of the session")
        // this->connected           = false;
        mm.new_mod(MODULE_INTERNAL_WIDGET2_CLOSE);
        this->signal = BACK_EVENT_NONE;
        return true;
    }

public:
    bool check(MMApi & mm, time_t now, Transport & trans) {
        TODO("Should only check enddate if it has been changed by acl");
        long enddate = this->ini->context.end_date_cnx.get();
        // LOG(LOG_INFO, "keep_alive(%lu, %lu, %lu)", keepalive_time, now, enddate));
        if (enddate != 0 && (now > enddate) && !this->last_module) {
            LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
            return invoke_mod_close(mm, "Session is out of allowed timeframe");
        }

        if (this->signal == BACK_EVENT_STOP) {
            mm.mod->event.reset();
            return false;
        }

        // Check if acl connection is lost.
        if (this->lost_acl && !this->last_module) {
            return invoke_mod_close(mm, "Connection closed by manager (ACL closed)");
        }

        // Keep alive
        if (this->keepalive_time) {
            if (now > (this->keepalive_time + this->keepalive_grace_delay)) {
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                return invoke_mod_close(mm, "Missed keepalive from ACL");
            }

            if (this->read_auth) {
                if (this->verbose & 0x10) {
                    LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
                }

                this->read_auth = false;

                if (this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
                    // this->ini->context_ask(AUTHID_KEEPALIVE);
                    this->keepalive_time       =
                    this->keepalive_renew_time = now + this->keepalive_grace_delay;
                }
            }

            if (now > this->keepalive_renew_time) {
                this->keepalive_renew_time = now + this->keepalive_grace_delay;

                if (this->verbose & 8) {
                    LOG( LOG_INFO, "%llu bytes received in last quantum, total: %llu tick:%d"
                         , trans.last_quantum_received, trans.total_received, this->tick_count);
                }

                if (trans.last_quantum_received == 0) {
                    this->tick_count++;
                    // 15 minutes before closing on inactivity
                    if (this->tick_count > this->max_tick) {
                        LOG(LOG_INFO, "Session ACL inactivity : closing");
                        return invoke_mod_close(mm, "Connection closed on inactivity");
                    }
                }
                else {
                    this->tick_count = 0;
                }
                LOG(LOG_INFO, "Session ACL inactivity : tick count: %u, disconnect on %uth tick", this->tick_count,this->max_tick);
                trans.tick();

                // ===================== check if keepalive ======================
                // try {
                this->ini->context_ask(AUTHID_KEEPALIVE);
                //     // this->signal = BACK_EVENT_REFRESH;
                //     //this->acl_serial.send(AUTHID_KEEPALIVE);
                // }
                // catch (...) {
                //     return invoke_mod_close(mm, "Connection closed by manager (ACL closed).");
                // }
            }
        }   // if (this->keepalive_time)

/*
        if (!this->last_module && (now - this->acl_start_time) > 10) {
        this->asked_remote_answer = false;
        this->last_module = true;
        mm.remove_mod();
        mm.new_mod(MODULE_INTERNAL_WIDGET2_CLOSE);
        return true;
        }
*/

        // Manage module (refresh or next)
        TODO("Check the needs and reference of this->asked_remote_answer. "
             "Maybe could be replaced by ini->check() alone ?");
        if (!this->asked_remote_answer && this->ini->check()) {
            if (this->signal == BACK_EVENT_REFRESH || this->signal == BACK_EVENT_NEXT) {
                this->asked_remote_answer = true;
                this->remote_answer       = false;
                this->ask_next_module_remote();
            }
        }
        else {
            this->asked_remote_answer = false;

            if (this->remote_answer && this->signal == BACK_EVENT_REFRESH) {
                LOG(LOG_INFO, "===========> MODULE_REFRESH");
                this->signal = BACK_EVENT_NONE;
                TODO("signal management (refresh/next) should go to ModuleManager, it's basically the same behavior. It could be implemented by closing module then opening anothe one of the same kind")
                mm.mod->refresh_context(*this->ini);
                mm.mod->event.signal = BACK_EVENT_NONE;
                mm.mod->event.set();
            }
            else if (this->remote_answer && this->signal == BACK_EVENT_NEXT) {
                LOG(LOG_INFO, "===========> MODULE_NEXT");
                this->signal = BACK_EVENT_NONE;
                int next_state = this->next_module();
                mm.remove_mod();
                try {
                    mm.new_mod(next_state);
                }
                catch (Error & e) {
                    if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                        return invoke_mod_close(mm, "Failed to connect to remote TCP host");
                    }
                    else if (e.id == ERR_SESSION_UNKNOWN_BACKEND) {
                        return invoke_mod_close(mm, "Unknown BackEnd.");
                    }
                    else {
                        throw e;
                    }
                }
                this->keepalive_time = 0;
                if (this->connected) {
                    this->start_keepalive();

                    mm.record();
                }
            }
        }

        // send message to acl with changed values when connected to
        // a module (rdp, vnc, xup ...) and something changed
        // used for authchannel and keepalive.

        if (this->connected && this->ini->check()) {
            this->ask_next_module_remote();
        }

        return true;
    }

    void receive() {
        try {
            if (!this->lost_acl) {
                this->acl_serial.incoming();
                this->read_auth     = true;
                this->remote_answer = true;
            }
        } catch (...) {
            // acl connection lost
            this->ini->context.authenticated.set(false);
            this->ini->context.rejected.set_from_cstr("Authentifier service failed");
            this->lost_acl = true;
        }
    }

    int next_module() {
        if (this->ini->context_is_asked(AUTHID_AUTH_USER)
            ||  this->ini->context_is_asked(AUTHID_PASSWORD)) {
            LOG(LOG_INFO, "===========> MODULE_LOGIN");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // Selector Target device and Target user contains list or possible targets
        else if (!this->ini->context_is_asked(AUTHID_SELECTOR)
                 &&   this->ini->context_get_bool(AUTHID_SELECTOR)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_USER)) {
            LOG(LOG_INFO, "===============> MODULE_SELECTOR");
            return MODULE_INTERNAL_WIDGET2_SELECTOR;
        }
        // Target User or Device asked and not in selector : back to login
        else if (this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 ||  this->ini->context_is_asked(AUTHID_TARGET_USER)) {
            LOG(LOG_INFO, "===============> MODULE_LOGIN (2)");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show message
        else if (this->ini->context_is_asked(AUTHID_DISPLAY_MESSAGE)) {
            LOG(LOG_INFO, "==================> MODULE_DISPLAY");
            return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show confirmation message
        else if (this->ini->context_is_asked(AUTHID_ACCEPT_MESSAGE)) {
            LOG(LOG_INFO, "=================> MODULE_ACCEPT");
            return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
        }
        // Authenticated = true, means we have : AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER, TARGET_PASSWORD
        // proceed with connection.
        else if (this->ini->context.authenticated.get()) {
            //                record_video = this->ini->globals.movie.get();
            //                keep_alive = true;
            if (this->ini->context.auth_error_message.is_empty()) {
                this->ini->context.auth_error_message.copy_c_str("End of connection");
            }
            LOG(LOG_INFO, "=================> MODULE_FROM_PROTOCOL");
            return this->get_mod_from_protocol();
        }
        // User authentication rejected : close message
        else {
            if (!this->ini->context.rejected.get().is_empty()) {
                this->ini->context.auth_error_message.copy_str(this->ini->context.rejected.get());
            }
            if (this->ini->context.auth_error_message.is_empty()) {
                this->ini->context.auth_error_message.copy_c_str("Authentifier service failed");
            }
            LOG(LOG_INFO, "MODULE_INTERNAL_CLOSE");
            return MODULE_INTERNAL_CLOSE;
        }
    }

    TODO("May be we should rename this method since it does not only ask for next module "
         "but it is also used for keep alive and auth channel messages acl");
    void ask_next_module_remote() {
        this->acl_serial.ask_next_module_remote();
    }


    /*
    // Set AUTHCHANNEL_TARGET dict value and transmit request to sesman (then wabenginge)
    void ask_auth_channel_target(const char * target) {
        if (this->verbose) {
            LOG(LOG_INFO, "SessionManager::ask_auth_channel_target(%s)", target);
        }

        this->ini->context_set_value(AUTHID_AUTHCHANNEL_TARGET, target);
        this->acl_serial.send(AUTHID_AUTHCHANNEL_TARGET);
    }

    // Set AUTHCHANNEL_RESULT dict value and transmit request to sesman (then wabenginge)
    void set_auth_channel_result(const char * result) {
        if (this->verbose) {
            LOG(LOG_INFO, "SessionManager::set_auth_channel_result(%s)", result);
        }

        this->ini->context_set_value(AUTHID_AUTHCHANNEL_RESULT, result);
        this->acl_serial.send(AUTHID_AUTHCHANNEL_RESULT);
    }

    */

    //    bool close_on_timestamp(long & timestamp)
    //    {
    //        bool res = false;
    //        if (MOD_STATE_DONE_CONNECTED == this->mod_state) {
    //            long enddate = this->ini->context.end_date_cnx;
    //            if (enddate != 0 && (timestamp > enddate)) {
    //                if (this->verbose & 0x10) {
    //                    LOG(LOG_INFO, "auth::close_on_timestamp");
    //                }
    //                LOG(LOG_INFO, "Session is out of allowed timeframe : stopping");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                res = true;
    //            }
    //        }
    //        return res;
    //    }

    //    bool keep_alive_checking(long & keepalive_time, long & now, Transport & trans)
    //    {

    //        //        LOG(LOG_INFO, "keep_alive(%lu, %lu)", keepalive_time, now);
    //        if (MOD_STATE_DONE_CONNECTED == this->mod_state) {
    //            long enddate = this->ini->context.end_date_cnx;
    //            //            LOG(LOG_INFO, "keep_alive(%lu, %lu, %lu)", keepalive_time, now, enddate));
    //            if (enddate != 0 && (now > enddate)) {
    //                LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                return false;
    //            }
    //        }

    //        if (keepalive_time == 0) {
    //            //            LOG(LOG_INFO, "keep_alive disabled");
    //            return true;
    //        }

    //        TODO("we should manage a mode to disconnect on inactivity when we are on login box or on selector")
    //            if (now > (keepalive_time + this->keepalive_grace_delay)) {
    //                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
    //                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (timeout)");
    //                return false;
    //            }


    //        if (now > keepalive_time) {
    //            // ===================== check if no traffic =====================
    //            if (this->verbose & 8) {
    //                LOG(LOG_INFO, "%llu bytes received in last quantum, total: %llu tick:%d",
    //                    trans.last_quantum_received, trans.total_received,
    //                    this->tick_count);
    //            }
    //            if (trans.last_quantum_received == 0) {
    //                this->tick_count++;
    //                if (this->tick_count > this->max_tick) { // 15 minutes before closing on inactivity
    //                    this->ini->context.auth_error_message.copy_c_str("Connection closed on inactivity");
    //                    LOG(LOG_INFO, "Session ACL inactivity : closing");
    //                    this->mod_state = MOD_STATE_DONE_CLOSE;
    //                    return false;
    //                }

    //                keepalive_time = now + this->keepalive_grace_delay;
    //            }
    //            else {
    //                this->tick_count = 0;
    //            }
    //            trans.tick();

    //            // ===================== check if keepalive ======================
    //            try {
    //                this->acl_serial.send(AUTHID_KEEPALIVE);
    //            }
    //            catch (...) {
    //                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed).");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                return false;
    //            }
    //        }
    //        return true;
    //    }

    //    bool keep_alive_response(long & keepalive_time, long & now)
    //    {
    //        if (this->verbose & 0x10) {
    //            LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
    //        }
    //        try {
    //            this->acl_serial.incoming();
    //            if (this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
    //                keepalive_time = now + this->keepalive_grace_delay;
    //                this->ini->context_ask(AUTHID_KEEPALIVE);
    //            }
    //        }
    //        catch (...) {
    //            this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed)");
    //            this->mod_state = MOD_STATE_DONE_CLOSE;
    //            return false;
    //        }
    //        return true;
    //    }


    //    bool keep_alive(long & keepalive_time, long & now, Transport * trans, bool read_auth)
    //    {
    ////        LOG(LOG_INFO, "keep_alive(%lu, %lu)", keepalive_time, now);
    //        if (MOD_STATE_DONE_CONNECTED == this->mod_state) {
    //            long enddate = this->ini->context.end_date_cnx;
    ////            LOG(LOG_INFO, "keep_alive(%lu, %lu, %lu)", keepalive_time, now, enddate));
    //            if (enddate != 0 && (now > enddate)) {
    //                LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                return false;
    //            }
    //        }

    //        if (keepalive_time == 0) {
    ////            LOG(LOG_INFO, "keep_alive disabled");
    //            return true;
    //        }

    //        TODO("we should manage a mode to disconnect on inactivity when we are on login box or on selector")
    //        if (now > (keepalive_time + this->keepalive_grace_delay)) {
    //            LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
    //            this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (timeout)");
    //            return false;
    //        }


    //        if (now > keepalive_time) {
    //            // ===================== check if no traffic =====================
    //            if (this->verbose & 8) {
    //                LOG(LOG_INFO, "%llu bytes received in last quantum, total: %llu tick:%d",
    //                          trans->last_quantum_received, trans->total_received,
    //                          this->tick_count);
    //            }
    //            if (trans->last_quantum_received == 0) {
    //                this->tick_count++;
    //                if (this->tick_count > this->max_tick) { // 15 minutes before closing on inactivity
    //                    this->ini->context.auth_error_message.copy_c_str("Connection closed on inactivity");
    //                    LOG(LOG_INFO, "Session ACL inactivity : closing");
    //                    this->mod_state = MOD_STATE_DONE_CLOSE;
    //                    return false;
    //                }

    //                keepalive_time = now + this->keepalive_grace_delay;
    //            }
    //            else {
    //                this->tick_count = 0;
    //            }
    //            trans->tick();

    //            // ===================== check if keepalive ======================
    //            try {
    //                this->acl_serial.send(AUTHID_KEEPALIVE);
    //            }
    //            catch (...) {
    //                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed).");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                return false;
    //            }
    //        }

    //        if (read_auth) {
    //            if (this->verbose & 0x10) {
    //                LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
    //            }
    //            try {
    //                this->acl_serial.incoming();
    //                if (this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
    //                    keepalive_time = now + this->keepalive_grace_delay;
    //                    this->ini->context_ask(AUTHID_KEEPALIVE);
    //                }
    //            }
    //            catch (...) {
    //                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed)");
    //                this->mod_state = MOD_STATE_DONE_CLOSE;
    //                return false;
    //            }
    //        }

    //        return true;
    //    }


    //        if (strcmp(this->ini.context.mode_console.c_str(), "force") == 0) {
    //            this->front.set_console_session(true);
    //            LOG(LOG_INFO, "ModuleManager::mode console : force");
    //        }
    //        else if (strcmp(this->ini.context.mode_console.c_str(), "forbid") == 0) {
    //            this->front.set_console_session(false);
    //            LOG(LOG_INFO, "ModuleManager::mode console : forbid");
    //        }
    //        else {
    //            // default is "allow", do nothing special
    //        }

    // Check keep alive not reached
    //                if (!this->acl->check_keep_alive(this->keep_alive_time, timestamp)) {
    //                }

    // Check inactivity not reached
    //                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)) {
    //                }

    // Check movie start/stop/pause
    //                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)) {
    //                }

    //                        // Check if acl received an answer to auth_channel_target
    //                        if (this->ini.globals.auth_channel[0]) {
    //                            // Get acl answer to AUTHCHANNEL_TARGET
    //                            if (!this->ini.context.authchannel_answer.is_empty()) {
    //                                // If set, transmit to auth_channel channel
    //                                this->mod->send_auth_channel_data(
    //                                    this->ini.context.authchannel_answer.c_str());
    //                                // Erase the context variable
    //                                this->ini.context.authchannel_answer.empty();
    //                            }
    //                        }

    // data incoming from server module
    //                        if (this->mod->event.is_set(rfds)) {
    //                            this->mod->event.reset();
    //                            if (this->verbose & 8) {
    //                                LOG(LOG_INFO, "Session::back_event fired");
    //                            }
    //                            BackEvent_t signal = this->mod->draw_event();
    //                            switch (signal) {
    //                            case BACK_EVENT_NONE:
    //                                // continue with same module
    //                            break;
    //                            case BACK_EVENT_STOP:
    //                                // current module finished for some serious reason implying immediate exit
    //                                // without going to close box.
    //                                // the typical case (and only one used for now) is... we are coming from CLOSE_BOX
    //                                this->internal_state = SESSION_STATE_STOP;
    //                                break;
    //                            case BACK_EVENT_REFRESH:
    //                            {
    //                                if (this->verbose & 8) {
    //                                    LOG(LOG_INFO, "Session::back event refresh");
    //                                }
    //                                bool record_video = false;
    //                                bool keep_alive = false;
    //                                int next_state = this->sesman->ask_next_module(
    //                                                                    this->keep_alive_time,
    //                                                                    record_video, keep_alive, this->nextmod);
    //                                if (next_state != MODULE_WAITING) {
    //                                    this->internal_state = SESSION_STATE_RUNNING;
    //                                }
    //                                else {
    //                                    this->internal_state = SESSION_STATE_WAITING_FOR_CONTEXT;
    //                                }
    //                            }
    //                            break;
    //                            case BACK_EVENT_NEXT:
    //                            default:
    //                            {
    //                                if (this->verbose & 8) {
    //                                   LOG(LOG_INFO, "Session::back event end module");
    //                                }
    //                               // end the current module and switch to new one
    //                                this->remove_mod();
    //                                bool record_video = false;
    //                                bool keep_alive = false;

    //                                int next_state = this->sesman->ask_next_module(
    //                                                                    this->keep_alive_time,
    //                                                                    record_video, keep_alive,
    //                                                                    this->nextmod);
    //                                if (this->verbose & 8) {
    //                                    LOG(LOG_INFO, "session::next_state %u", next_state);
    //                                }

    //                                if (next_state != MODULE_WAITING) {
    //                                    this->internal_state = SESSION_STATE_STOP;
    //                                    try {
    //                                        this->session_setup_mod(next_state);
    //                                        if (record_video) {
    //                                            this->this->front.start_capture(
    //                                                this->this->front.client_info.width,
    //                                                this->this->front.client_info.height,
    //                                                this->ini
    //                                                );
    //                                        }
    //                                        else {
    //                                            this->this->front.stop_capture();
    //                                        }
    //                                        if (this->sesman && keep_alive) {
    //                                            this->sesman->start_keep_alive(keep_alive_time);
    //                                        }
    //                                        this->internal_state = SESSION_STATE_RUNNING;
    //                                    }
    //                                    catch (const Error & e) {
    //                                        LOG(LOG_INFO, "Session::connect failed Error=%u", e.id);
    //                                        this->nextmod = INTERNAL_CLOSE;
    //                                        this->session_setup_mod(MODULE_INTERNAL);
    //                                        this->keep_alive_time = 0;
    //                                        delete sesman;
    //                                        this->sesman = NULL;
    //                                        this->internal_state = SESSION_STATE_RUNNING;
    //                                        this->this->front.stop_capture();
    //                                        LOG(LOG_INFO, "Session::capture stopped, authentifier stopped");
    //                                    }
    //                                }
    //                            }
    //                            break;
    //                            }
    //                        }
    //                    }
    //                    break;
    //                    case SESSION_STATE_CLOSE_CONNECTION:
    //                    {
    //                        if (this->mod->event.is_set(rfds)) {
    //                            this->internal_state = SESSION_STATE_STOP;
    //                        }
    //                    }
    //                    break;
    //                }
    //                if (this->internal_state == SESSION_STATE_STOP) {
    //                    break;
    //                }


    //    if (!this->acl) {
    //        LOG(LOG_INFO, "Session::no authentifier available, closing");
    //        this->ini.context.auth_error_message.copy_c_str("No authentifier available");
    //        this->internal_state  = SESSION_STATE_CLOSE_CONNECTION;
    //        this->nextmod         = INTERNAL_CLOSE;
    //        this->session_setup_mod(MODULE_INTERNAL);
    //        this->keep_alive_time = 0;
    //        this->internal_state  = SESSION_STATE_RUNNING;
    //        this->this->front.stop_capture();
    //    }


    //    if (strcmp(this->ini.context.mode_console.c_str(), "force") == 0) {
    //        this->this->front.set_console_session(true);
    //        LOG(LOG_INFO, "Session::mode console : force");
    //    }
    //    else if (strcmp(this->ini.context.mode_console.c_str(), "forbid") == 0) {
    //        this->this->front.set_console_session(false);
    //        LOG(LOG_INFO, "Session::mode console : forbid");
    //    }
    //    else {
    //        // default is "allow", do nothing special
    //    }

};



#endif

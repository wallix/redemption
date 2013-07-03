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
    enum {
        MOD_STATE_INIT,
        MOD_STATE_DONE_RECEIVED_CREDENTIALS,
        MOD_STATE_DONE_DISPLAY_MESSAGE,
        MOD_STATE_DONE_VALID_MESSAGE,
        MOD_STATE_DONE_LOGIN,
        MOD_STATE_DONE_SELECTOR,
        MOD_STATE_DONE_PASSWORD,
        MOD_STATE_DONE_CONNECTED,
        MOD_STATE_DONE_CLOSE,
        MOD_STATE_DONE_EXIT,
    } mod_state;

    // running module
    // change module
    // refresh module

    Inifile * ini;

    int tick_count;

    public:
    BackEvent_t signal;
    AclSerializer acl_serial;
    int keepalive_grace_delay;
    int max_tick;
    bool internal_domain;
    uint32_t verbose;

    SessionManager(Inifile * _ini, Transport & _auth_trans, int _keepalive_grace_delay,
                   int _max_tick, bool _internal_domain, uint32_t _verbose)
        : mod_state(MOD_STATE_INIT)
        , ini(_ini)
        , tick_count(0)
        , signal(BACK_EVENT_NONE)
        , acl_serial(AclSerializer(_ini,_auth_trans,_verbose))
        , keepalive_grace_delay(_keepalive_grace_delay)
        , max_tick(_max_tick)
        , internal_domain(_internal_domain)
        , verbose(_verbose)

    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::SessionManager");
        }
    }

    ~SessionManager()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::~SessionManager");
        }
    }

    void start_keep_alive(long & keepalive_time)
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::start_keep_alive");
        }
        this->tick_count = 1;

        this->ini->context_ask(AUTHID_KEEPALIVE);
        this->acl_serial.send(AUTHID_KEEPALIVE);
        keepalive_time = ::time(NULL) + 30;
    }

    // Set AUTHCHANNEL_TARGET dict value and transmit request to sesman (then wabenginge)
    void ask_auth_channel_target(const char * target)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "SessionManager::ask_auth_channel_target(%s)", target);
        }

        this->ini->context_set_value(AUTHID_AUTHCHANNEL_TARGET, target);
        this->acl_serial.send(AUTHID_AUTHCHANNEL_TARGET);
    }

    // Set AUTHCHANNEL_RESULT dict value and transmit request to sesman (then wabenginge)
    void set_auth_channel_result(const char * result)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "SessionManager::set_auth_channel_result(%s)", result);
        }

        this->ini->context_set_value(AUTHID_AUTHCHANNEL_RESULT, result);
        this->acl_serial.send(AUTHID_AUTHCHANNEL_RESULT);
    }

    bool close_on_timestamp(long & timestamp)
    {
        bool res = false;
        if (MOD_STATE_DONE_CONNECTED == this->mod_state){
            long enddate = this->ini->context.end_date_cnx;
            if (enddate != 0 && (timestamp > enddate)) {
                if (this->verbose & 0x10){
                    LOG(LOG_INFO, "auth::close_on_timestamp");
                }
                LOG(LOG_INFO, "Session is out of allowed timeframe : stopping");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                res = true;
            }
        }
        return res;
    }

    bool keep_alive_checking(long & keepalive_time, long & now, Transport & trans)
    {

        //        LOG(LOG_INFO, "keep_alive(%lu, %lu)", keepalive_time, now);
        if (MOD_STATE_DONE_CONNECTED == this->mod_state){
            long enddate = this->ini->context.end_date_cnx;
            //            LOG(LOG_INFO, "keep_alive(%lu, %lu, %lu)", keepalive_time, now, enddate));
            if (enddate != 0 && (now > enddate)) {
                LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                return false;
            }
        }

        if (keepalive_time == 0){
            //            LOG(LOG_INFO, "keep_alive disabled");
            return true;
        }

        TODO("we should manage a mode to disconnect on inactivity when we are on login box or on selector")
            if (now > (keepalive_time + this->keepalive_grace_delay)){
                LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (timeout)");
                return false;
            }


        if (now > keepalive_time){
            // ===================== check if no traffic =====================
            if (this->verbose & 8){
                LOG(LOG_INFO, "%llu bytes received in last quantum, total: %llu tick:%d",
                    trans.last_quantum_received, trans.total_received,
                    this->tick_count);
            }
            if (trans.last_quantum_received == 0){
                this->tick_count++;
                if (this->tick_count > this->max_tick){ // 15 minutes before closing on inactivity
                    this->ini->context.auth_error_message.copy_c_str("Connection closed on inactivity");
                    LOG(LOG_INFO, "Session ACL inactivity : closing");
                    this->mod_state = MOD_STATE_DONE_CLOSE;
                    return false;
                }

                keepalive_time = now + this->keepalive_grace_delay;
            }
            else {
                this->tick_count = 0;
            }
            trans.tick();

            // ===================== check if keepalive ======================
            try {
                this->acl_serial.send(AUTHID_KEEPALIVE);
            }
            catch (...){
                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed).");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                return false;
            }
        }
        return true;
    }
    bool keep_alive_response(long & keepalive_time, long & now)
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
        }
        try {
            this->acl_serial.incoming();
            if (this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
                keepalive_time = now + this->keepalive_grace_delay;
                this->ini->context_ask(AUTHID_KEEPALIVE);
            }
        }
        catch (...){
            this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed)");
            this->mod_state = MOD_STATE_DONE_CLOSE;
            return false;
        }
        return true;
    }
    
    
    bool keep_alive(long & keepalive_time, long & now, Transport * trans, bool read_auth)
    {
//        LOG(LOG_INFO, "keep_alive(%lu, %lu)", keepalive_time, now);
        if (MOD_STATE_DONE_CONNECTED == this->mod_state){
            long enddate = this->ini->context.end_date_cnx;
//            LOG(LOG_INFO, "keep_alive(%lu, %lu, %lu)", keepalive_time, now, enddate));
            if (enddate != 0 && (now > enddate)) {
                LOG(LOG_INFO, "Session is out of allowed timeframe : closing");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                return false;
            }
        }

        if (keepalive_time == 0){
//            LOG(LOG_INFO, "keep_alive disabled");
            return true;
        }

        TODO("we should manage a mode to disconnect on inactivity when we are on login box or on selector")
        if (now > (keepalive_time + this->keepalive_grace_delay)){
            LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
            this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (timeout)");
            return false;
        }


        if (now > keepalive_time){
            // ===================== check if no traffic =====================
            if (this->verbose & 8){
                LOG(LOG_INFO, "%llu bytes received in last quantum, total: %llu tick:%d",
                          trans->last_quantum_received, trans->total_received,
                          this->tick_count);
            }
            if (trans->last_quantum_received == 0){
                this->tick_count++;
                if (this->tick_count > this->max_tick){ // 15 minutes before closing on inactivity
                    this->ini->context.auth_error_message.copy_c_str("Connection closed on inactivity");
                    LOG(LOG_INFO, "Session ACL inactivity : closing");
                    this->mod_state = MOD_STATE_DONE_CLOSE;
                    return false;
                }

                keepalive_time = now + this->keepalive_grace_delay;
            }
            else {
                this->tick_count = 0;
            }
            trans->tick();

            // ===================== check if keepalive ======================
            try {
                this->acl_serial.send(AUTHID_KEEPALIVE);
            }
            catch (...){
                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed).");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                return false;
            }
        }

        if (read_auth) {
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::keep_alive ACL incoming event");
            }
            try {
                this->acl_serial.incoming();
                if (this->ini->context_get_bool(AUTHID_KEEPALIVE)) {
                    keepalive_time = now + this->keepalive_grace_delay;
                    this->ini->context_ask(AUTHID_KEEPALIVE);
                }
            }
            catch (...){
                this->ini->context.auth_error_message.copy_c_str("Connection closed by manager (ACL closed)");
                this->mod_state = MOD_STATE_DONE_CLOSE;
                return false;
            }
        }

        return true;
    }

    int get_mod_from_protocol()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::get_mod_from_protocol");
        }
        const char * protocol = this->ini->context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0);
        if (this->internal_domain){
            const char * target = this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0);
            if (0 == strncmp(target, "autotest", 8)){
                protocol = "INTERNAL";
            }
        }
        int res = MODULE_EXIT;
        if (strncasecmp(protocol, "RDP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol RDP");
            }
            res = MODULE_RDP;
        }
        else if (strncasecmp(protocol, "VNC", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol VNC");
            }
            res = MODULE_VNC;
        }
        else if (strncasecmp(protocol, "XUP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol XUP");
            }
            res = MODULE_XUP;
        }
        else if (strncasecmp(protocol, "INTERNAL", 8) == 0){
            const char * target = this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0);
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL");
            }
            if (0 == strcmp(target, "bouncer2")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL bouncer2");
                }
                res = MODULE_INTERNAL_BOUNCER2;
            }
            else if (0 == strncmp(target, "autotest", 8)){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL test");
                }
                const char * user = this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0);
                size_t len_user = strlen(user);
                strncpy(this->ini->context.movie, user, sizeof(this->ini->context.movie));
                this->ini->context.movie[sizeof(this->ini->context.movie) - 1] = 0;
                if (0 != strcmp(".mwrm", user + len_user - 5)){
                    strcpy(this->ini->context.movie + len_user, ".mwrm");
                }
                res = MODULE_INTERNAL_TEST;
            }
            else if (0 == strcmp(target, "selector")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL selector");
                }
                res = MODULE_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (0 == strcmp(target, "login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "rwl_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else if (0 == strcmp(target, "rwl")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "close")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL close");
                }
                res = MODULE_INTERNAL_CLOSE;
            }
            else if (0 == strcmp(target, "widget2_close")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_close");
                }
                res = MODULE_INTERNAL_WIDGET2_CLOSE;
            }
            else if (0 == strcmp(target, "widget2_dialog")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_dialog");
                }
                res = MODULE_INTERNAL_WIDGET2_DIALOG;
            }
            else if (0 == strcmp(target, "widget2_message")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MODULE_INTERNAL_WIDGET2_MESSAGE;
            }
            else if (0 == strcmp(target, "widget2_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_login");
                }
                res = MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "widget2_rwl")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "widget2_rwl_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_rwl_login");
                }
                res = MODULE_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else {
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL card");
                }
                res = MODULE_INTERNAL_CARD;
            }
        }
        else {
            LOG(LOG_WARNING, "Unsupported target protocol %c%c%c%c",
                protocol[0], protocol[1], protocol[2], protocol[3]);
            res = MODULE_EXIT;
//            assert(false);
        }
        return res;
    }

    void receive(){
        LOG(LOG_INFO, "ACL Receive()");
        this->receive_next_module();
        
    }

    void check(BackEvent_t & last_mod_draw_event, ModuleManager & mm){
    }

    int next_module()
    {
        if (this->signal == BACK_EVENT_NONE) {
            LOG(LOG_INFO, "========> MODULE_RUNNING");
            return MODULE_RUNNING;
        }

        this->signal = BACK_EVENT_NONE;

        if (this->signal == BACK_EVENT_REFRESH) {
            LOG(LOG_INFO, "===========> MODULE_REFRESH");
            return MODULE_REFRESH;
        }
    
        if (this->ini->context_is_asked(AUTHID_AUTH_USER) 
        ||  this->ini->context_is_asked(AUTHID_PASSWORD)){
            LOG(LOG_INFO, "===========> MODULE_LOGIN");
            return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // Selector Target device and Target user contains list or possible targets
        else if (!this->ini->context_is_asked(AUTHID_SELECTOR)
             &&   this->ini->context_get_bool(AUTHID_SELECTOR)
             &&  !this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
             &&  !this->ini->context_is_asked(AUTHID_TARGET_USER)){
            LOG(LOG_INFO, "===============> MODULE_SELECTOR");
            return MODULE_INTERNAL_WIDGET2_SELECTOR;
        }
        // Target User or Device asked and not in selector : back to login
        else if (this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
             ||  this->ini->context_is_asked(AUTHID_TARGET_USER)){
                LOG(LOG_INFO, "===============> MODULE_LOGIN (2)");
                return MODULE_INTERNAL_WIDGET2_LOGIN;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show message
        else if (this->ini->context_is_asked(AUTHID_DISPLAY_MESSAGE)){
            LOG(LOG_INFO, "==================> MODULE_DISPLAY");
            return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
        }
        // AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER known, but acl asked to show confirmation message
        else if (this->ini->context_is_asked(AUTHID_ACCEPT_MESSAGE)){
                     LOG(LOG_INFO, "=================> MODULE_ACCEPT");
           return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
        }
        // Authenticated = true, means we have : AUTH_USER, AUTH_PASSWORD, TARGET_DEVICE, TARGET_USER, TARGET_PASSWORD
        // proceed with connection.
        else if (this->ini->context.authenticated){
//                record_video = this->ini->globals.movie;
//                keep_alive = true;
            if (this->ini->context.auth_error_message.is_empty()) {
                this->ini->context.auth_error_message.copy_c_str("End of connection");
            }
             LOG(LOG_INFO, "=================> MODULE_FROM_PROTOCOL");
            return this->get_mod_from_protocol();
        }
        // User authentication rejected : close message
        else {
            if (!this->ini->context.rejected.is_empty()) {
                this->ini->context.auth_error_message.copy_str(this->ini->context.rejected);
            }
            if (this->ini->context.auth_error_message.is_empty()) {
                this->ini->context.auth_error_message.copy_c_str("Authentifier service failed");
            }
             LOG(LOG_INFO, "MODULE_INTERNAL_CLOSE");
            return MODULE_INTERNAL_CLOSE;
        }
    }

    void ask_next_module_remote()
    {
        this->acl_serial.ask_next_module_remote();
    }

    int ask_next_module()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::ask_next_module");
        }
        switch (this->mod_state){
        default:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module default state");
            }
            return MODULE_WAITING;
        break;
        case MOD_STATE_DONE_SELECTOR:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_SELECTOR state");
            }
            this->acl_serial.ask_next_module_remote();
            return MODULE_WAITING;
        break;
        case MOD_STATE_DONE_LOGIN:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_LOGIN state");
            }
            this->acl_serial.ask_next_module_remote();
            return MODULE_WAITING;
        break;
        case MOD_STATE_DONE_PASSWORD:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_PASSWORD state");
            }
            this->acl_serial.ask_next_module_remote();
            return MODULE_WAITING;
        break;
        case MOD_STATE_DONE_RECEIVED_CREDENTIALS:
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS state");
        }
        {
            if (this->verbose & 0x20){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_AUTH_USER=%s",
                    (this->ini->context_is_asked(AUTHID_AUTH_USER) ? "True": "False"));
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD=%s",
                    (this->ini->context_is_asked(AUTHID_PASSWORD) ? "True": "False"));
            }

            if (this->ini->context_is_asked(AUTHID_AUTH_USER)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_AUTH_USER is asked");
                }
                this->mod_state = MOD_STATE_DONE_LOGIN;
                return MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (this->ini->context_is_asked(AUTHID_PASSWORD)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD is asked");
                }
                this->mod_state = MOD_STATE_DONE_LOGIN;
                return MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (!this->ini->context_is_asked(AUTHID_SELECTOR)
                 &&   this->ini->context_get_bool(AUTHID_SELECTOR)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_USER)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_SELECTOR is asked");
                }
                this->mod_state = MOD_STATE_DONE_SELECTOR;
                return MODULE_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 ||  this->ini->context_is_asked(AUTHID_TARGET_USER)){
                    if (this->verbose & 0x20){
                        LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_TARGET_DEVICE is asked");
                    }
                    this->mod_state = MOD_STATE_DONE_LOGIN;
                    return MODULE_INTERNAL_WIDGET2_LOGIN;
            }
            else if (this->ini->context_is_asked(AUTHID_DISPLAY_MESSAGE)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_DISPLAY_MESSAGE is asked");
                }
                this->mod_state = MOD_STATE_DONE_DISPLAY_MESSAGE;
                return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
            }
            else if (this->ini->context_is_asked(AUTHID_ACCEPT_MESSAGE)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_ACCEPT_MESSAGE is asked");
                }
                this->mod_state = MOD_STATE_DONE_VALID_MESSAGE;
                return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
            }
            else if (this->ini->context.authenticated){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS authenticated is True");
                }
//                record_video = this->ini->globals.movie;
//                keep_alive = true;
                if (this->ini->context.auth_error_message.is_empty()) {
                    this->ini->context.auth_error_message.copy_c_str("End of connection");
                }
                this->mod_state = MOD_STATE_DONE_CONNECTED;
                return this->get_mod_from_protocol();
            }
            else {
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS else");
                }

                if (!this->ini->context.rejected.is_empty()) {
                    this->ini->context.auth_error_message.copy_str(this->ini->context.rejected);
                }
                if (this->ini->context.auth_error_message.is_empty()) {
                    this->ini->context.auth_error_message.copy_c_str("Authentifier service failed");
                }
                this->mod_state = MOD_STATE_DONE_CONNECTED;
                return MODULE_INTERNAL_CLOSE;
            }
        }
        break;
        case MOD_STATE_DONE_CONNECTED:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->mod_state = MOD_STATE_DONE_CLOSE;
            return MODULE_INTERNAL_CLOSE;
        break;
        case MOD_STATE_DONE_CLOSE:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->mod_state = MOD_STATE_DONE_EXIT;
            return MODULE_EXIT_INTERNAL_CLOSE;
        break;
        case MOD_STATE_DONE_EXIT:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_EXIT state");
            }
            // we should never goes here, the main loop should have stopped before
            LOG(LOG_WARNING, "unexpected forced exit");
            return MODULE_EXIT_INTERNAL_CLOSE;
        break;
        }
    }

    void receive_next_module()
    {
        LOG(LOG_INFO, "received next module");
        try {
            this->acl_serial.incoming();
        } catch (...) {
            this->ini->context.authenticated = false;
            this->ini->context.rejected.copy_c_str("Authentifier service failed");
        }
        this->mod_state = MOD_STATE_DONE_RECEIVED_CREDENTIALS;
    }
};

#endif

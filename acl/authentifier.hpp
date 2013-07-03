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
#include "sockettransport.hpp"
#include "acl_serializer.hpp"
#include "mod_api.hpp"
#include "front.hpp"
#include "null/null.hpp"
#include "rdp/rdp.hpp"
#include "vnc/vnc.hpp"
#include "xup/xup.hpp"
#include "transitory/transitory.hpp"
#include "cli/cli_mod.hpp"

#include "internal/widget2/bouncer2.hpp"
#include "internal/widget2/test_card_mod.hpp"
#include "internal/widget2/replay_mod.hpp"
#include "internal/widget2/selector_mod.hpp"
#include "internal/widget2/wab_close_mod.hpp"
#include "internal/widget2/dialog_mod.hpp"
#include "internal/widget2/login_mod.hpp"
#include "internal/widget2/rwl_mod.hpp"
#include "internal/widget2/rwl_login_mod.hpp"

typedef enum {
    INTERNAL_NONE,
    INTERNAL_DIALOG_DISPLAY_MESSAGE,
    INTERNAL_DIALOG_VALID_MESSAGE,
    INTERNAL_CLOSE,
    INTERNAL_BOUNCER2,
    INTERNAL_TEST,
    INTERNAL_CARD,
    INTERNAL_WIDGET2_SELECTOR,
    INTERNAL_WIDGET2_CLOSE,
    INTERNAL_WIDGET2_DIALOG,
    INTERNAL_WIDGET2_MESSAGE,
    INTERNAL_WIDGET2_LOGIN,
    INTERNAL_WIDGET2_RWL,
    INTERNAL_WIDGET2_RWL_LOGIN,
} submodule_t;

enum {
    MCTX_STATUS_EXIT,
    MCTX_STATUS_WAITING,
    MCTX_STATUS_VNC,
    MCTX_STATUS_RDP,
    MCTX_STATUS_XUP,
    MCTX_STATUS_INTERNAL,
    MCTX_STATUS_INTERNAL_INTERNAL_CLOSE,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_CLOSE,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_DIALOG,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_MESSAGE,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL_LOGIN,
    MCTX_STATUS_INTERNAL_INTERNAL_CARD,
    MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_VALID_MESSAGE,
    MCTX_STATUS_INTERNAL_INTERNAL_BOUNCER2,
    MCTX_STATUS_INTERNAL_INTERNAL_TEST,
    MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_SELECTOR,
    MCTX_STATUS_EXIT_INTERNAL_CLOSE,
    MCTX_STATUS_TRANSITORY,
    MCTX_STATUS_AUTH,
    MCTX_STATUS_CLI,
};

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

    Inifile * ini;

    int tick_count;

    public:
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
        int res = MCTX_STATUS_EXIT;
        if (strncasecmp(protocol, "RDP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol RDP");
            }
            res = MCTX_STATUS_RDP;
        }
        else if (strncasecmp(protocol, "VNC", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol VNC");
            }
            res = MCTX_STATUS_VNC;
        }
        else if (strncasecmp(protocol, "XUP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol XUP");
            }
            res = MCTX_STATUS_XUP;
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
                res = MCTX_STATUS_INTERNAL_INTERNAL_BOUNCER2;
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
                res = MCTX_STATUS_INTERNAL_INTERNAL_TEST;
            }
            else if (0 == strcmp(target, "selector")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL selector");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (0 == strcmp(target, "login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "rwl_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else if (0 == strcmp(target, "rwl")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "close")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL close");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_CLOSE;
            }
            else if (0 == strcmp(target, "widget2_close")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_close");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_CLOSE;
            }
            else if (0 == strcmp(target, "widget2_dialog")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_dialog");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_DIALOG;
            }
            else if (0 == strcmp(target, "widget2_message")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_message");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_MESSAGE;
            }
            else if (0 == strcmp(target, "widget2_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN;
            }
            else if (0 == strcmp(target, "widget2_rwl")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL rwl_login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL;
            }
            else if (0 == strcmp(target, "widget2_rwl_login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL widget2_rwl_login");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL_LOGIN;
            }
            else {
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL card");
                }
                res = MCTX_STATUS_INTERNAL_INTERNAL_CARD;
            }
        }
        else {
            LOG(LOG_WARNING, "Unsupported target protocol %c%c%c%c",
                protocol[0], protocol[1], protocol[2], protocol[3]);
            res = MCTX_STATUS_EXIT;
//            assert(false);
        }
        return res;
    }


    void receive(){
        LOG(LOG_INFO, "ACL Receive()");
        this->receive_next_module();
        
    }

    void remove_mod(mod_api * & mod, mod_api * no_mod, Transport * & mod_transport)
    {
        if (mod != no_mod){
            delete mod;
            if (mod_transport) {
                delete mod_transport;
                mod_transport = NULL;
            }
            mod = no_mod;
        }
    }

    void setup_mod(int target_module, mod_api * & mod, mod_api * no_mod, Transport * & mod_transport, Front * front)
    {
        LOG(LOG_INFO, "Authentifier::setup_mod(target_module=%u)", target_module);

//        if (strcmp(this->ini->context.mode_console.c_str(), "force") == 0){
//            front->set_console_session(true);
//            LOG(LOG_INFO, "Authentifier::mode console : force");
//        }
//        else if (strcmp(this->ini->context.mode_console.c_str(), "forbid") == 0){
//            front->set_console_session(false);
//            LOG(LOG_INFO, "Authentifier::mode console : forbid");
//        }
//        else {
//            // default is "allow", do nothing special
//        }

        this->remove_mod(mod, no_mod, mod_transport);

        switch (target_module)
        {
            case MCTX_STATUS_INTERNAL_INTERNAL_CLOSE:
            {
                LOG(LOG_INFO, "Authentifier::Creation of new mod 'INTERNAL::Close'");
                if (this->ini->context.auth_error_message.is_empty()) {
                    this->ini->context.auth_error_message.copy_c_str("Connection to server ended");
                }
                mod = new WabCloseMod(*this->ini,
                                      *front,
                                       front->client_info.width,
                                       front->client_info.height);
                front->init_pointers();
            }
            LOG(LOG_INFO, "Authentifier::internal module Close ready");
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_BOUNCER2:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'bouncer2'");
                mod = new Bouncer2Mod(*front,
                                       front->client_info.width,
                                       front->client_info.height
                                     );
                if (this->verbose){
                    LOG(LOG_INFO, "Authentifier::internal module 'bouncer2' ready");
                }
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_TEST:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'test'");
                mod = new ReplayMod(
                      *front
                    , this->ini->video.replay_path
                    , this->ini->context.movie
                    , front->client_info.width
                    , front->client_info.height
                    , this->ini->context.auth_error_message
                    );
                if (this->verbose){
                    LOG(LOG_INFO, "Authentifier::internal module 'test' ready");
                }
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_CARD:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'test_card'");
                mod = new TestCardMod(*front,
                                       front->client_info.width,
                                       front->client_info.height
                                            );
                LOG(LOG_INFO, "Authentifier::internal module 'test_card' ready");
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_SELECTOR:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'selector'");
                mod = new SelectorMod(*this->ini,
                                      *front,
                                       front->client_info.width,
                                       front->client_info.height
                                       );
                if (this->verbose){
                    LOG(LOG_INFO, "Authentifier::internal module 'selector' ready");
                }
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_CLOSE:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'CloseMod'");
                mod = new WabCloseMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height
                );
                LOG(LOG_INFO, "Authentifier::internal module 'CloseMod' ready");
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_VALID_MESSAGE:
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_DIALOG:
            {
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'Dialog Accept Message'");
                const char * message = this->ini->context.message.c_str();
                const char * button = this->ini->translation.button_refused.get().c_str();
                const char * caption = "Information";
                mod = new DialogMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height,
                    caption,
                    message,
                    button
                );
                LOG(LOG_INFO, "Authentifier::internal module 'Dialog Accept Message' ready");
            }
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_DISPLAY_MESSAGE:
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_MESSAGE:
            {
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'Dialog Display Message'");
                const char * message = this->ini->context.message.c_str();
                const char * button = NULL;
                const char * caption = "Information";
                mod = new DialogMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height,
                    caption,
                    message,
                    button
                );
                LOG(LOG_INFO, "Authentifier::internal module 'Dialog Display Message' ready");
            }
            break;
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'Login'");
                mod = new LoginMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height);
                LOG(LOG_INFO, "Authentifier::internal module Login ready");
                break;
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'Login'");
                mod = new RwlMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height);
                LOG(LOG_INFO, "Authentifier::internal module Login ready");
                break;
            case MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_RWL_LOGIN:
                LOG(LOG_INFO, "Authentifier::Creation of internal module 'Login'");
                mod = new RwlLoginMod(
                    *this->ini,
                    *front,
                    front->client_info.width,
                    front->client_info.height);
                    LOG(LOG_INFO, "Authentifier::internal module Login ready");
                break;

            case MCTX_STATUS_XUP:
            {
                const char * name = "XUP Target";
                if (this->verbose){
                    LOG(LOG_INFO, "Authentifier::Creation of new mod 'XUP'\n");
                }

                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port.get(),
                                            4, 1000,
                                            this->ini->debug.mod_xup);

                if (client_sck == -1){
                    this->ini->context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                SocketTransport * t = new SocketTransport(
                      name
                    , client_sck
                    , this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini->context.target_port.get()
                    , this->ini->debug.mod_xup);
                mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote X host");
                mod = new xup_mod( t
                                   , *front
                                   , front->client_info.width
                                   , front->client_info.height
                                   , this->ini->context.opt_width.get()
                                   , this->ini->context.opt_height.get()
                                   , this->ini->context.opt_bpp.get()
                                   );
                mod->event.obj = client_sck;
                mod->draw_event();
//                this->mod->rdp_input_invalidate(Rect(0, 0, front->get_client_info().width, front->get_client_info().height));
                this->ini->context.auth_error_message.empty();
                LOG(LOG_INFO, "Authentifier::Creation of new mod 'XUP' suceeded\n");
            }
            break;

            case MCTX_STATUS_RDP:
            {
                LOG(LOG_INFO, "Authentifier::Creation of new mod 'RDP'");
                REDOC("hostname is the name of the RDP host ('windows' hostname) it is **not** used to get an ip address.")
                char hostname[255];
                hostname[0] = 0;
                if (front->client_info.hostname[0]){
                    memcpy(hostname, front->client_info.hostname, 31);
                    hostname[31] = 0;
                }
                static const char * name = "RDP Target";

                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port.get(),
                                            3, 1000,
                                            this->ini->debug.mod_rdp);

                if (client_sck == -1){
                    this->ini->context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                TODO("RZ: We need find a better way to give access of STRAUTHID_AUTH_ERROR_MESSAGE to SocketTransport")
                SocketTransport * t = new SocketTransport(
                      name
                    , client_sck
                    , this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini->context.target_port.get()
                    , this->ini->debug.mod_rdp
                    , &this->ini->context.auth_error_message
                    );
                mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote RDP host");
                UdevRandom gen;
                mod = new mod_rdp( t
                                       , this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0)
                                       , this->ini->context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                                       , "0.0.0.0"  // client ip is silenced
                                       , *front
                                       , hostname
                                       , true
                                       , front->client_info
                                       , &gen
                                       , front->keymap.key_flags
//                                       , this->acl   // we give mod_rdp a direct access to sesman for auth_channel channel
                                       , this->ini->globals.auth_channel
                                       , this->ini->globals.alternate_shell
                                       , this->ini->globals.shell_working_directory
                                       , this->ini->client.clipboard
                                       , true   // support fast-path
                                       , true   // support mem3blt
                                       , this->ini->globals.enable_bitmap_update
                                       , this->ini->debug.mod_rdp
                                       , true   // support new pointer
                                       );
                mod->event.obj = client_sck;

                mod->rdp_input_invalidate(Rect(0, 0, front->client_info.width, front->client_info.height));
                LOG(LOG_INFO, "Authentifier::Creation of new mod 'RDP' suceeded\n");
                this->ini->context.auth_error_message.empty();
            }
            break;

            case MCTX_STATUS_VNC:
            {
                LOG(LOG_INFO, "Authentifier::Creation of new mod 'VNC'\n");
                static const char * name = "VNC Target";


                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port.get(),
                                            3, 1000,
                                            this->ini->debug.mod_vnc);

                if (client_sck == -1){
                    this->ini->context.auth_error_message.copy_c_str("failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }

                SocketTransport * t = new SocketTransport(
                      name
                    , client_sck
                    , this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
                    , this->ini->context.target_port.get()
                    , this->ini->debug.mod_vnc);
                mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote VNC host");

                mod = new mod_vnc(
                      t
                    , this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0)
                    , this->ini->context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                    , *front
                    , front->client_info.width
                    , front->client_info.height
                    , front->client_info.keylayout
                    , front->keymap.key_flags
                    , this->ini->client.clipboard
                    , true /* RRE encoding */
                    , this->ini->debug.mod_vnc);
                mod->event.obj = client_sck;
                mod->draw_event();

                LOG(LOG_INFO, "Authentifier::Creation of new mod 'VNC' suceeded\n");
                this->ini->context.auth_error_message.empty();
            }
            break;

            default:
            {
                LOG(LOG_INFO, "Authentifier::Unknown backend exception\n");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

    void check(BackEvent_t & last_mod_draw_event, mod_api * & mod, mod_api * no_mod, Transport * & mod_transport, Front * front){
        LOG(LOG_INFO, "ACL Check()");
        switch (last_mod_draw_event) {
        case BACK_EVENT_NONE:
        LOG(LOG_INFO, "Back event none");
        // continue with same module
        break;
        case BACK_EVENT_STOP:
        LOG(LOG_INFO, "Back event stop");
        // current module finished for some serious reason implying immediate exit
        // without going to close box.
        // the typical case (and only one used for now) is... we are coming from CLOSE_BOX
        break;
        case BACK_EVENT_REFRESH:
            LOG(LOG_INFO, "Back event refresh");
        break;
        case BACK_EVENT_NEXT:
            LOG(LOG_INFO, "Back event next");
            
            int next_state = this->ask_next_module();

            if (next_state != MCTX_STATUS_WAITING){
                this->setup_mod(next_state, mod, no_mod, mod_transport, front);
            }
        break;
        }
        
        
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
            this->acl_serial.ask_next_module_remote();
            return MCTX_STATUS_WAITING;
        break;
        case MOD_STATE_DONE_SELECTOR:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_SELECTOR state");
            }
            this->acl_serial.ask_next_module_remote();
            return MCTX_STATUS_WAITING;
        break;
        case MOD_STATE_DONE_LOGIN:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_LOGIN state");
            }
            this->acl_serial.ask_next_module_remote();
            return MCTX_STATUS_WAITING;
        break;
        case MOD_STATE_DONE_PASSWORD:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_PASSWORD state");
            }
            this->acl_serial.ask_next_module_remote();
            return MCTX_STATUS_WAITING;
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
                return MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN;
            }
            else if (this->ini->context_is_asked(AUTHID_PASSWORD)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_PASSWORD is asked");
                }
                this->mod_state = MOD_STATE_DONE_LOGIN;
                return MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN;
            }
            else if (!this->ini->context_is_asked(AUTHID_SELECTOR)
                 &&   this->ini->context_get_bool(AUTHID_SELECTOR)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 &&  !this->ini->context_is_asked(AUTHID_TARGET_USER)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_SELECTOR is asked");
                }
                this->mod_state = MOD_STATE_DONE_SELECTOR;
                return MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_SELECTOR;
            }
            else if (this->ini->context_is_asked(AUTHID_TARGET_DEVICE)
                 ||  this->ini->context_is_asked(AUTHID_TARGET_USER)){
                    if (this->verbose & 0x20){
                        LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_TARGET_DEVICE is asked");
                    }
                    this->mod_state = MOD_STATE_DONE_LOGIN;
                    return MCTX_STATUS_INTERNAL_INTERNAL_WIDGET2_LOGIN;
            }
            else if (this->ini->context_is_asked(AUTHID_DISPLAY_MESSAGE)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_DISPLAY_MESSAGE is asked");
                }
                this->mod_state = MOD_STATE_DONE_DISPLAY_MESSAGE;
                return MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_DISPLAY_MESSAGE;
            }
            else if (this->ini->context_is_asked(AUTHID_ACCEPT_MESSAGE)){
                if (this->verbose & 0x20){
                    LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS AUTHID_ACCEPT_MESSAGE is asked");
                }
                this->mod_state = MOD_STATE_DONE_VALID_MESSAGE;
                return MCTX_STATUS_INTERNAL_INTERNAL_DIALOG_VALID_MESSAGE;
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
                return MCTX_STATUS_INTERNAL_INTERNAL_CLOSE;
            }
        }
        break;
        case MOD_STATE_DONE_CONNECTED:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->mod_state = MOD_STATE_DONE_CLOSE;
            return MCTX_STATUS_INTERNAL_INTERNAL_CLOSE;
        break;
        case MOD_STATE_DONE_CLOSE:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->mod_state = MOD_STATE_DONE_EXIT;
            return MCTX_STATUS_EXIT_INTERNAL_CLOSE;
        break;
        case MOD_STATE_DONE_EXIT:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_EXIT state");
            }
            // we should never goes here, the main loop should have stopped before
            LOG(LOG_WARNING, "unexpected forced exit");
            return MCTX_STATUS_EXIT_INTERNAL_CLOSE;
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

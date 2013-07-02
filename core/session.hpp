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
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#include "server.hpp"
#include "colors.hpp"
#include "stream.hpp"
#include "constants.hpp"
#include "ssl_calls.hpp"
#include "rect.hpp"
#include "client_info.hpp"
#include "netutils.hpp"

#include "config.hpp"
#include "wait_obj.hpp"
#include "transport.hpp"
#include "bitmap.hpp"

#include "authentifier.hpp"
//#include "front.hpp"
//#include "null/null.hpp"
//#include "rdp/rdp.hpp"
//#include "vnc/vnc.hpp"
//#include "xup/xup.hpp"
//#include "transitory/transitory.hpp"
//#include "cli/cli_mod.hpp"

//#include "internal/widget2/bouncer2.hpp"
//#include "internal/widget2/test_card_mod.hpp"
//#include "internal/widget2/replay_mod.hpp"
//#include "internal/widget2/selector_mod.hpp"
//#include "internal/widget2/wab_close_mod.hpp"
//#include "internal/widget2/dialog_mod.hpp"
//#include "internal/widget2/login_mod.hpp"
//#include "internal/widget2/rwl_mod.hpp"
//#include "internal/widget2/rwl_login_mod.hpp"

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
    SESSION_STATE_STOP,
};

struct Session {
    wait_obj & front_event;

    Inifile  * ini;
    uint32_t & verbose;

    submodule_t nextmod;
    int internal_state;
    long id;                     // not used
    time_t keep_alive_time;

    struct mod_api * mod; /* module interface */
    struct mod_api * no_mod;

    Transport * mod_transport;

    struct Front * front;

    SessionManager * acl;
    UdevRandom gen;

    SocketTransport * ptr_auth_trans;
    wait_obj        * ptr_auth_event;

    Session(wait_obj & front_event, int sck, Inifile * ini)
        : front_event(front_event)
        , ini(ini)
        , verbose(this->ini->debug.session)
        , nextmod(INTERNAL_NONE)
        , mod_transport(NULL)
        , acl(NULL)
        , ptr_auth_trans(NULL)
        , ptr_auth_event(NULL)
    {
        try {
            SocketTransport front_trans("RDP Client", sck, "", 0, this->ini->debug.front);
            // Contruct auth_trans (SocketTransport) and auth_event (wait_obj)
            //  here instead of inside Sessionmanager

            this->ptr_auth_trans = NULL;
            this->ptr_auth_event = NULL;
            this->acl = NULL;

            this->mod = 0;
            this->internal_state = SESSION_STATE_ENTRY;
            const bool enable_fastpath = true;
            const bool tls_support     = this->ini->globals.enable_tls;
            const bool mem3blt_support = true;
            this->front = new Front(&front_trans, SHARE_PATH "/" DEFAULT_FONT_NAME, &this->gen,
                ini, enable_fastpath, tls_support, mem3blt_support);
            this->no_mod = new null_mod(*(this->front));
            this->mod = this->no_mod;

            /* module interface */
            this->keep_alive_time = 0;

            if (this->verbose){
                LOG(LOG_INFO, "Session::session_main_loop() starting");
            }

            const char * state_names[] =
            { "Initializing client session"                         // SESSION_STATE_ENTRY
            , "Waiting for authentifier"                            // SESSION_STATE_WAITING_FOR_NEXT_MODULE
            , "Waiting for authentifier (context refresh required)" // SESSION_STATE_WAITING_FOR_CONTEXT
            , "Running"                                             // SESSION_STATE_RUNNING
            , "Close connection"                                    // SESSION_STATE_CLOSE_CONNECTION
            , "Stop required"                                       // SESSION_STATE_STOP
            };

            int previous_state = SESSION_STATE_STOP;
            struct timeval time_mark = { 0, 0 };
            // We want to start a module
            BackEvent_t last_mod_draw_event = BACK_EVENT_NEXT;
            while (1) {

                if (time_mark.tv_sec == 0 && time_mark.tv_usec < 500){
                    time_mark.tv_sec = 0;
                    time_mark.tv_usec = 50000;
                }

                if (this->internal_state != previous_state)
                    LOG(LOG_INFO, "Session::-------------- %s -------------------", state_names[this->internal_state]);
                previous_state = this->internal_state;

                unsigned max = 0;
                fd_set rfds;
                fd_set wfds;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                struct timeval timeout = time_mark;

                this->front_event.add_to_fd_set(rfds, max);
                if (this->acl){
                    this->ptr_auth_event->add_to_fd_set(rfds, max);
                }
                if (this->mod != no_mod){
                    this->mod->event.add_to_fd_set(rfds, max);
                }

                if (this->mod->event.is_set(rfds)) {
                    timeout.tv_sec  = 0;
                    timeout.tv_usec = 0;
                }

                int num = select(max + 1, &rfds, &wfds, 0, &timeout);

                if (num < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Cope with EBADF, EINVAL, ENOMEM : none of these should ever happen
                    // EBADF: means fd has been closed (my me) or as already returned an error on another call
                    // EINVAL: invalid value in timeout (my fault again)
                    // ENOMEM: no enough memory in kernel (unlikely fort 3 sockets)

                    LOG(LOG_ERR, "Proxy data wait loop raised error %u : %s", errno, strerror(errno));
                    throw Error(ERR_SOCKET_ERROR);
                }

                time_t timestamp = time(NULL);
                if (this->front_event.is_set(rfds)) {
                    switch (this->front->incoming(*this->mod)){
                    default:
                    case FRONT_DISCONNECTED:
                        throw Error(ERR_SOCKET_ERROR);
                    break;
                    case FRONT_CONNECTING:
                        continue;
                    break;
                    case FRONT_RUNNING:
                    break;
                    }
                }

                TODO("We should have a first loop to wait for front to get up and running, then we will proceed with the standard loop")
                if (this->front->up_and_running){                    
                
//                      this->front->periodic_snapshot(this->mod->get_pointer_displayed());

                    // Process incoming module trafic
                    if (this->mod != this->no_mod){
                        if (this->mod->event.is_set(rfds)){
                            this->mod->event.reset();
                            last_mod_draw_event = this->mod->draw_event();
                        }
                    }
                    
                    // Incoming data from ACL, or opening acl
                    if (!this->acl){
                        this->connect_authentifier(last_mod_draw_event);
                    }
                    else {
                        if (this->ptr_auth_event->is_set(rfds)){
                            // acl received updated values
                            this->acl->receive();
                        }
                        // take current state into account to:
                        // - continue with same current module
                        // - change current module
                        // - refresh module that asked for it
                        // - check keep alive or inactivity (may stop connection)
                        this->acl->check(last_mod_draw_event, this->mod, this->no_mod, this->mod_transport, this->front);
                    }
                }

            }
            this->front->disconnect();
        }
        catch (const Error & e) {
            LOG(LOG_INFO, "Session::Session exception = %d!\n", e.id);
        }
        catch(...){
            LOG(LOG_INFO, "Session::Session other exception\n");
        }
        LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        this->front->stop_capture();
    }

                    // Mod refresh context
//                        this->mod->refresh_context(*this->ini);
//                        this->mod->event.set();
                    // or Mod next module

                // Check keep alive not reached
//                if (!this->acl->check_keep_alive(this->keep_alive_time, timestamp)){
//                }

                // Check inactivity not reached
//                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)){
//                }

                // Check movie start/stop/pause
//                if (!this->acl->check_inactivity(this->keep_alive_time, timestamp)){
//                }

//                        // Check if acl received an answer to auth_channel_target
//                        if (this->ini->globals.auth_channel[0]) {
//                            // Get acl answer to AUTHCHANNEL_TARGET
//                            if (!this->ini->context.authchannel_answer.is_empty()) {
//                                // If set, transmit to auth_channel channel
//                                this->mod->send_auth_channel_data(
//                                    this->ini->context.authchannel_answer.c_str());
//                                // Erase the context variable
//                                this->ini->context.authchannel_answer.empty();
//                            }
//                        }

                        // data incoming from server module
//                        if (this->mod->event.is_set(rfds)){
//                            this->mod->event.reset();
//                            if (this->verbose & 8){
//                                LOG(LOG_INFO, "Session::back_event fired");
//                            }
//                            BackEvent_t signal = this->mod->draw_event();
//                            switch (signal){
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
//                                if (this->verbose & 8){
//                                    LOG(LOG_INFO, "Session::back event refresh");
//                                }
//                                bool record_video = false;
//                                bool keep_alive = false;
//                                int next_state = this->sesman->ask_next_module(
//                                                                    this->keep_alive_time,
//                                                                    record_video, keep_alive, this->nextmod);
//                                if (next_state != MCTX_STATUS_WAITING){
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
//                                if (this->verbose & 8){
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
//                                if (this->verbose & 8){
//                                    LOG(LOG_INFO, "session::next_state %u", next_state);
//                                }

//                                if (next_state != MCTX_STATUS_WAITING){
//                                    this->internal_state = SESSION_STATE_STOP;
//                                    try {
//                                        this->session_setup_mod(next_state);
//                                        if (record_video) {
//                                            this->front->start_capture(
//                                                this->front->client_info.width,
//                                                this->front->client_info.height,
//                                                *this->ini
//                                                );
//                                        }
//                                        else {
//                                            this->front->stop_capture();
//                                        }
//                                        if (this->sesman && keep_alive){
//                                            this->sesman->start_keep_alive(keep_alive_time);
//                                        }
//                                        this->internal_state = SESSION_STATE_RUNNING;
//                                    }
//                                    catch (const Error & e) {
//                                        LOG(LOG_INFO, "Session::connect failed Error=%u", e.id);
//                                        this->nextmod = INTERNAL_CLOSE;
//                                        this->session_setup_mod(MCTX_STATUS_INTERNAL);
//                                        this->keep_alive_time = 0;
//                                        delete sesman;
//                                        this->sesman = NULL;
//                                        this->internal_state = SESSION_STATE_RUNNING;
//                                        this->front->stop_capture();
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
//                if (this->internal_state == SESSION_STATE_STOP){
//                    break;
//                }


//    if (!this->acl) {
//        LOG(LOG_INFO, "Session::no authentifier available, closing");
//        this->ini->context.auth_error_message.copy_c_str("No authentifier available");
//        this->internal_state  = SESSION_STATE_CLOSE_CONNECTION;
//        this->nextmod         = INTERNAL_CLOSE;
//        this->session_setup_mod(MCTX_STATUS_INTERNAL);
//        this->keep_alive_time = 0;
//        this->internal_state  = SESSION_STATE_RUNNING;
//        this->front->stop_capture();
//    }


//    if (strcmp(this->ini->context.mode_console.c_str(), "force") == 0){
//        this->front->set_console_session(true);
//        LOG(LOG_INFO, "Session::mode console : force");
//    }
//    else if (strcmp(this->ini->context.mode_console.c_str(), "forbid") == 0){
//        this->front->set_console_session(false);
//        LOG(LOG_INFO, "Session::mode console : forbid");
//    }
//    else {
//        // default is "allow", do nothing special
//    }

// Check movie start/stop/pause
//    if (this->ini->globals.movie) {
//        if (this->front->capture_state == Front::CAPTURE_STATE_UNKNOWN) {
//            this->front->start_capture( this->front->client_info.width
//                                      , this->front->client_info.height
//                                      , *this->ini
//                                      );
//            this->mod->rdp_input_invalidate(
//                Rect( 0, 0, this->front->client_info.width
//                    , this->front->client_info.height));
//        }
//        else if (this->front->capture_state == Front::CAPTURE_STATE_PAUSED) {
//            this->front->resume_capture();
//            this->mod->rdp_input_invalidate(
//                Rect( 0, 0, this->front->client_info.width
//                    , this->front->client_info.height));
//        }
//    }
//    else if (this->front->capture_state == Front::CAPTURE_STATE_STARTED) {
//        this->front->pause_capture();
//    }



    ~Session()
    {
        delete this->front;
        this->remove_mod();
        delete this->no_mod;
        if (this->acl) { delete this->acl; }
        if (this->ptr_auth_event) { delete this->ptr_auth_event; }
        if (this->ptr_auth_trans) { delete this->ptr_auth_trans; }
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        if (!this->ini->context.session_id.is_empty()) {
            char new_session_file[256];
            sprintf(new_session_file, "%s/session_%s.pid", PID_PATH,
                this->ini->context.session_id.c_str());
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", PID_PATH, child_pid);
            unlink(old_session_file);
        }
    }


    void connect_authentifier(BackEvent_t & last_mod_draw_event)
    {
        int client_sck = ip_connect(this->ini->globals.authip,
                                    this->ini->globals.authport,
                                    30,
                                    1000,
                                    this->ini->debug.auth);

        if (client_sck == -1) {
            LOG(LOG_ERR, "Failed to connect to authentifieur");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }
        
        this->ptr_auth_trans = new SocketTransport( "Authentifier"
                                                  , client_sck
                                                  , this->ini->globals.authip
                                                  , this->ini->globals.authport
                                                  , this->ini->debug.auth
                                                  );
        this->ptr_auth_event = new wait_obj(this->ptr_auth_trans->sck);
        this->acl = new SessionManager( this->ini
                                         , *this->ptr_auth_trans
                                         , this->ini->globals.keepalive_grace_delay
                                         , this->ini->globals.max_tick
                                         , this->ini->globals.internal_domain
                                         , this->ini->debug.auth);
    }

    void remove_mod()
    {
        if (this->mod != this->no_mod){
            delete this->mod;
            if (this->mod_transport) {
                delete this->mod_transport;
                this->mod_transport = NULL;
            }
            this->mod = this->no_mod;
        }
    }

};

#endif

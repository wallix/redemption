/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Javier Caverni

*/

#if !defined(__CORE_SESSION_HPP__)
#define __CORE_SESSION_HPP__

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

#include "config.hpp"
#include "wait_obj.hpp"
#include "transport.hpp"
#include "bitmap.hpp"
#include "modcontext.hpp"

#include "sesman.hpp"
#include "front.hpp"
#include "null/null.hpp"
#include "internal/login.hpp"
#include "internal/bouncer2.hpp"
#include "internal/close.hpp"
#include "internal/dialog.hpp"
#include "internal/test_card.hpp"
#include "internal/test_internal.hpp"
#include "internal/selector.hpp"
#include "rdp/rdp.hpp"
#include "vnc/vnc.hpp"
#include "xup/xup.hpp"
#include "transitory/transitory.hpp"
#include "cli/cli_mod.hpp"

using namespace std;

enum {
//    // before anything else : exchange of credentials
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

    int * refreshconf;

    wait_obj & front_event;
    SocketTransport & front_trans;

    Inifile * ini;
    uint32_t & verbose;

    ModContext * context;
    int internal_state;
    long id;
    time_t keep_alive_time;


    struct client_mod * mod; /* module interface */
    struct client_mod * no_mod;

    struct Front* front;

    SessionManager * sesman;
    UdevRandom gen;

    Session(wait_obj & front_event, SocketTransport & front_trans, const char * ip_source, int * refreshconf, Inifile * ini)
        : refreshconf(refreshconf)
        , front_event(front_event)
        , front_trans(front_trans)
        , ini(ini)
        , verbose(this->ini->globals.debug.session)
    {
        try {
            this->context = new ModContext();
            this->context->cpy(STRAUTHID_HOST, ip_source);
            this->sesman = new SessionManager(*this->context
                                             , this->ini->globals.keepalive_grace_delay
                                             , this->ini->globals.max_tick
                                             , this->ini->globals.internal_domain
                                             , this->ini->globals.debug.auth);
            this->mod = 0;
            this->internal_state = SESSION_STATE_ENTRY;
            this->front = new Front(&this->front_trans, &this->gen, ini);
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
            while (1) {
                if (*this->refreshconf){
                    LOG(LOG_INFO, "refresh conf: reading directory %s", ini->globals.dynamic_conf_path);
                    if (*this->refreshconf & 1){
                        *this->refreshconf ^= 1;
                        DIR * d = opendir(ini->globals.dynamic_conf_path);
                        if (d){

                            size_t path_len = strlen(ini->globals.dynamic_conf_path);
                            size_t file_len = pathconf(ini->globals.dynamic_conf_path, _PC_NAME_MAX) + 1;
                            char * buffer = (char*)malloc(file_len + path_len);
                            strcpy(buffer, ini->globals.dynamic_conf_path);
                            size_t len = offsetof(struct dirent, d_name) + file_len;
                            struct dirent * entryp = (struct dirent *)malloc(len);
                            struct dirent * result;
                            for (readdir_r(d, entryp, &result) ; result ; readdir_r(d, entryp, &result)) {
                                if ((0 == strcmp(entryp->d_name, ".")) || (0 == strcmp(entryp->d_name, ".."))){
                                    continue;
                                }
                                strcpy(buffer + path_len, entryp->d_name);
                                struct stat st;
                                if (stat(buffer, &st) < 0){
                                    LOG(LOG_INFO, "Failed to read dynamic configuration file %s [%u: %s]",
                                        buffer, errno, strerror(errno));
                                    continue;
                                }
                                try {
                                    ini->cparse(buffer);
                                    this->front->update_config(*ini);
                                }
                                catch(...){
                                    LOG(LOG_INFO, "Error reading conf file %s", buffer);
                                    continue;
                                }
                                LOG(LOG_INFO, "reading conf file %s", buffer);
                                if (unlink(buffer) < 0){
                                    LOG(LOG_INFO, "Failed to remove dynamic configuration file %s after parsing [%u: %s]",
                                        buffer, errno, strerror(errno));
                                }
                            }
                            closedir(d);
                            free(entryp);
                            free(buffer);
                        }
                        else {
                            LOG(LOG_INFO, "Failed to open dynamic configuration directory %s [%u: %s]",
                                ini->globals.dynamic_conf_path, errno, strerror(errno));
                        }
                    }
                }

                if (time_mark.tv_sec == 0 && time_mark.tv_usec < 500){
                    time_mark.tv_sec = 0;
                    time_mark.tv_usec = 50000;
                }

                if (this->internal_state != previous_state)
                    LOG(LOG_DEBUG, "Session::-------------- %s\n", state_names[this->internal_state]);
                previous_state = this->internal_state;

                unsigned max = 0;
                fd_set rfds;
                fd_set wfds;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                struct timeval timeout = time_mark;

                this->front_event.add_to_fd_set(rfds, max);
                if (this->front->up_and_running){
                    this->sesman->add_to_fd_set(rfds, max);
                    this->mod->event.add_to_fd_set(rfds, max);
                }
                int num = select(max + 1, &rfds, &wfds, 0, &timeout);
                if (num < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    LOG(LOG_ERR, "Proxy data wait loop raised error %u : %s", errno, strerror(errno));
                    this->internal_state = previous_state = SESSION_STATE_STOP;
                }
                else {
                    if (this->front_event.is_set(rfds)) {
                        try {
                            this->front->incoming(*this->mod);
                        }
                        catch(...){
                            this->internal_state = previous_state = SESSION_STATE_STOP;
                        };
                    }
                }
                switch (previous_state)
                {
                    case SESSION_STATE_ENTRY:
                    {
                        if (this->front->up_and_running){
                            this->session_setup_mod(MCTX_STATUS_CLI, this->context);
                            this->mod->event.set();
                            this->internal_state = SESSION_STATE_RUNNING;
                        }
                    }
                    break;
                    case SESSION_STATE_WAITING_FOR_NEXT_MODULE:
                    {
                        if (this->sesman->event(rfds)){
                            this->sesman->receive_next_module();

                            if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"force")==0){
                                this->front->set_console_session(true);
                                LOG(LOG_INFO, "Session::mode console : force");
                            }
                            else if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"forbid")==0){
                                this->front->set_console_session(false);
                                LOG(LOG_INFO, "Session::mode console : forbid");
                            }
                            else {
                                // default is "allow", do nothing special
                            }

                            if (this->mod != this->no_mod) {
                                delete this->mod;
                                this->mod = this->no_mod;
                            }
                            this->mod = new transitory_mod(*(this->front),
                                                           this->front->client_info.width,
                                                           this->front->client_info.height);

                            this->mod->event.set();
                            this->internal_state = SESSION_STATE_RUNNING;
                        }
                    }
                    break;
                    case SESSION_STATE_WAITING_FOR_CONTEXT:
                    {
                        if (this->sesman->event(rfds)){
                            this->sesman->receive_next_module();

                            if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"force")==0){
                                this->front->set_console_session(true);
                                LOG(LOG_INFO, "Session::mode console : force");
                            }
                            else if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"forbid")==0){
                                this->front->set_console_session(false);
                                LOG(LOG_INFO, "Session::mode console : forbid");
                            }
                            else {
                                // default is "allow", do nothing special
                            }

                            this->mod->refresh_context(*this->context);

                            this->mod->event.set();
                            this->internal_state = SESSION_STATE_RUNNING;
                        }
                    }
                    break;
                    case SESSION_STATE_RUNNING:
                    {
                        time_t timestamp = time(NULL);
                        this->front->periodic_snapshot(this->mod->get_pointer_displayed());

                        TODO(" this should use the WAIT_FOR_CONTEXT state or some race conditon may cause mayhem")
                        if (this->sesman->close_on_timestamp(timestamp)
                        || !this->sesman->keep_alive_or_inactivity(rfds, this->keep_alive_time, timestamp, &this->front_trans)){
                            this->internal_state = SESSION_STATE_STOP;
                            this->context->nextmod = ModContext::INTERNAL_CLOSE;
                            this->session_setup_mod(MCTX_STATUS_INTERNAL, this->context);
                            this->keep_alive_time = 0;
                            TODO(" move that to sesman ? (to hide implementation details)")

                            delete this->sesman->auth_event;
                            this->sesman->auth_event = 0;

                            this->internal_state = SESSION_STATE_RUNNING;
                            this->front->stop_capture();
                        }

                        // data incoming from server module
                        if (this->front->up_and_running
                        &&  this->mod->event.is_set(rfds)){
                            this->mod->event.reset();
                            if (this->verbose){
                                LOG(LOG_INFO, "Session::back_event fired");
                            }
                            BackEvent_t signal = this->mod->draw_event();
                            switch (signal){
                            case BACK_EVENT_NONE:
                                // continue with same module
                            break;
                            case BACK_EVENT_STOP:
                                // current module finished for some serious reason implying immediate exit
                                // without going to close box.
                                // the typical case (and only one used for now) is... we are coming from CLOSE_BOX
                                this->internal_state = SESSION_STATE_STOP;
                                return;
                            case BACK_EVENT_REFRESH:
                            if (this->verbose){
                                LOG(LOG_INFO, "Session::back event refresh");
                            }
                            {
                                bool record_video = false;
                                bool keep_alive = false;
                                int next_state = this->sesman->ask_next_module(
                                                                    this->keep_alive_time,
                                                                    this->ini->globals.authip,
                                                                    this->ini->globals.authport,
                                                                    record_video, keep_alive);
                                if (next_state != MCTX_STATUS_WAITING){
                                    this->internal_state = SESSION_STATE_STOP;
                                    delete this->mod;
                                    this->mod = this->no_mod;
                                    this->session_setup_mod(next_state, this->context);
                                    this->internal_state = SESSION_STATE_RUNNING;
                                }
                                else {
                                    this->internal_state = SESSION_STATE_WAITING_FOR_CONTEXT;
                                }
                            }
                            break;
                            case BACK_EVENT_1:
                            case BACK_EVENT_2:
                            case BACK_EVENT_3:
                            case BACK_EVENT_5:
                            default:
                            {
                                if (this->verbose){
                                   LOG(LOG_INFO, "Session::back event end module");
                                }
                               // end the current module and switch to new one
                                if (this->mod != this->no_mod){
                                    delete this->mod;
                                    this->mod = this->no_mod;
                                }
                                this->context->cpy(STRAUTHID_OPT_WIDTH, this->front->client_info.width);
                                this->context->cpy(STRAUTHID_OPT_HEIGHT, this->front->client_info.height);
                                this->context->cpy(STRAUTHID_OPT_BPP, this->front->client_info.bpp);
                                bool record_video = false;
                                bool keep_alive = false;
                                if (this->verbose){
                                    LOG(LOG_INFO, "Session::asking next module");
                                }
                                int next_state = this->sesman->ask_next_module(
                                                                    this->keep_alive_time,
                                                                    this->ini->globals.authip,
                                                                    this->ini->globals.authport,
                                                                    record_video, keep_alive);
                                if (next_state != MCTX_STATUS_WAITING){
                                    this->internal_state = SESSION_STATE_STOP;
                                    try {
                                        this->session_setup_mod(next_state, this->context);
                                        if (record_video) {
                                            this->front->start_capture(
                                                this->front->client_info.width,
                                                this->front->client_info.height,
                                                *this->ini,
                                                *this->context
                                                );
                                        }
                                        else {
                                            this->front->stop_capture();
                                        }
                                        if (keep_alive){
                                            this->sesman->start_keep_alive(keep_alive_time);
                                        }
                                        this->internal_state = SESSION_STATE_RUNNING;
                                    }
                                    catch (const Error & e) {
                                         LOG(LOG_INFO, "Session::connect failed Error=%u", e.id);
                                         if (e.id == ERR_SOCKET_CONNECT_FAILED) {
                                            this->internal_state = SESSION_STATE_CLOSE_CONNECTION;
                                            this->context->nextmod = ModContext::INTERNAL_CLOSE;
                                            this->session_setup_mod(MCTX_STATUS_INTERNAL, this->context);
                                            this->keep_alive_time = 0;
                                            TODO(" move that to sesman (to hide implementation details)")

                                            delete this->sesman->auth_event;
                                            this->sesman->auth_event = 0;

                                            this->internal_state = SESSION_STATE_RUNNING;
                                            this->front->stop_capture();
                                        }
                                        else {
                                            throw;
                                        }
                                    }
                                }
                                else {
                                    this->internal_state = SESSION_STATE_WAITING_FOR_NEXT_MODULE;
                                }
                            }
                            break;
                            }
                        }
                    }
                    break;
                    case SESSION_STATE_CLOSE_CONNECTION:
                    {
                        if (this->mod->event.is_set(rfds)) {
                            this->internal_state = SESSION_STATE_STOP;
                        }
                    }
                    break;
                }
                if (this->internal_state == SESSION_STATE_STOP){
                    break;
                }
            }
            this->front->disconnect();
        }
        catch(...){
        };
        LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        this->front->stop_capture();
    }


    ~Session()
    {
        delete this->front;
        if (this->mod != this->no_mod){
            delete this->mod;
            this->mod = this->no_mod;
        }
        delete this->no_mod;
        delete this->sesman;
        // Suppress Session file from disk (original name with PID or renamed with session_id)
        if ( this->context->get(STRAUTHID_SESSION_ID)[0] != 0 ) {
            char new_session_file[256];
            sprintf(new_session_file, "%s/session_%s.pid", PID_PATH, this->context->get(STRAUTHID_SESSION_ID));
            unlink(new_session_file);
        }
        else {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/session_%d.pid", PID_PATH, child_pid);
            unlink(old_session_file);
        }
        delete this->context;
    }

    void session_setup_mod(int target_module, const ModContext * context)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::session_setup_mod(target_module=%u)", target_module);
        }


        if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"force")==0){
            this->front->set_console_session(true);
            LOG(LOG_INFO, "Session::mode console : force");
        }
        else if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"forbid")==0){
            this->front->set_console_session(false);
            LOG(LOG_INFO, "Session::mode console : forbid");
        }
        else {
            // default is "allow", do nothing special
        }

        switch (target_module)
        {
            case MCTX_STATUS_CLI:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'CLI parse'");
                }
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                this->mod = new cli_mod(*this->context, *(this->front),
                                        this->front->client_info,
                                        this->front->client_info.width,
                                        this->front->client_info.height);
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'CLI parse' suceeded");
                }
            }
            break;

            case MCTX_STATUS_INTERNAL:
            {
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                switch (this->context->nextmod){
                    case ModContext::INTERNAL_CLOSE:
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of new mod 'INTERNAL::Close'");
                        }
                        if (this->context->get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                            this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection to server ended");
                        }
                        this->mod = new close_mod(*this->context,
                                                  *this->front,
                                                  this->front->client_info.width,
                                                  this->front->client_info.height);
                        this->front->init_pointers();
                    }
                    if (this->verbose){
                        LOG(LOG_INFO, "Session::internal module Close ready");
                    }
                    break;
                    case ModContext::INTERNAL_DIALOG_VALID_MESSAGE:
                    {
                        const char * message = NULL;
                        const char * button = NULL;
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of new mod 'INTERNAL::Dialog Accept Message'");
                        }
                        message = this->context->get(STRAUTHID_MESSAGE);
                        button = this->context->get(STRAUTHID_TRANS_BUTTON_REFUSED);
                        this->mod = new dialog_mod(*this->context,
                                        *this->front,
                                        this->front->client_info.width,
                                        this->front->client_info.height,
                                        message,
                                        button,
                                        this->ini);
                    }
                    if (this->verbose){
                        LOG(LOG_INFO, "Session::internal module 'Dialog Accept Message' ready");
                    }
                    break;

                    case ModContext::INTERNAL_DIALOG_DISPLAY_MESSAGE:
                    {
                        const char * message = NULL;
                        const char * button = NULL;
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of new mod 'INTERNAL::Dialog Display Message'");
                        }
                        message = this->context->get(STRAUTHID_MESSAGE);
                        button = NULL;
                        this->mod = new dialog_mod(
                                        *this->context,
                                        *this->front,
                                        this->front->client_info.width,
                                        this->front->client_info.height,
                                        message,
                                        button,
                                        this->ini);
                    }
                    if (this->verbose){
                        LOG(LOG_INFO, "Session::internal module 'Dialog Display Message' ready");
                    }
                    break;
                    case ModContext::INTERNAL_LOGIN:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Login'");
                        }
                        this->mod = new login_mod(
                                         *this->context,
                                         *this->front,
                                         this->front->client_info.width,
                                         this->front->client_info.height,
                                         this->ini);
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module Login ready");
                        }
                    break;
                    case ModContext::INTERNAL_BOUNCER2:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'bouncer2'");
                        }
                        this->mod = new bouncer2_mod(*this->front,
                                                     this->front->client_info.width,
                                                     this->front->client_info.height
                                                     );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'bouncer2' ready");
                        }
                    break;
                    case ModContext::INTERNAL_TEST:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'test'");
                        }
                        this->mod = new test_internal_mod(
                                        *this->context,
                                        *this->front,
                                        this->ini->globals.replay_path,
                                        this->context->movie,
                                        this->front->client_info.width,
                                        this->front->client_info.height
                                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'test' ready");
                        }
                    break;
                    case ModContext::INTERNAL_CARD:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'test_card'");
                        }
                        this->mod = new test_card_mod(
                                        *this->front,
                                        this->front->client_info.width,
                                        this->front->client_info.height
                                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'test_card' ready");
                        }
                    break;
                    case ModContext::INTERNAL_SELECTOR:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'selector'");
                        }
                        this->mod = new selector_mod(
                                        *this->context,
                                        *this->front,
                                        this->front->client_info.width,
                                        this->front->client_info.height
                                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'selector' ready");
                        }
                    break;
                    default:
                    break;
                }
            }
            break;

            case MCTX_STATUS_XUP:
            {
                const char * name = "XUP Target";
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'XUP'\n");
                }
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                ClientSocketTransport * t = new ClientSocketTransport(name,
                                this->context->get(STRAUTHID_TARGET_DEVICE),
                                atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                4, 1000,
                                this->ini->globals.debug.mod_xup);
                if (!t->connect()){
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                else {
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed authentification on remote X host");
                }
                this->mod = new xup_mod(t, *this->context, *(this->front),
                                        this->front->client_info.width,
                                        this->front->client_info.height);
                this->mod->event.obj = t->sck;
                this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "");
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'XUP' suceeded\n");
                }
            }
            break;

            case MCTX_STATUS_RDP:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'RDP'");
                }
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                // hostname is the name of the RDP host ("windows" hostname)
                // it is **not** used to get an ip address.
                char hostname[255];
                hostname[0] = 0;
                if (this->front->client_info.hostname[0]){
                    memcpy(hostname, this->front->client_info.hostname, 31);
                    hostname[31] = 0;
                }
                static const char * name = "RDP Target";
                ClientSocketTransport * t = new ClientSocketTransport(
                                        name,
                                        this->context->get(STRAUTHID_TARGET_DEVICE),
                                        atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                        3, 1000,
                                        this->ini->globals.debug.mod_rdp);
                if (!t->connect()){
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                else {
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed authentification on remote RDP host");
                }
                TODO("Wait obj should work with transport object, not directly with socket")
                // enable or disable clipboard
                // this->context->get_bool(STRAUTHID_OPT_CLIPBOARD)
                // enable or disable device redirection
                // this->context->get_bool(STRAUTHID_OPT_DEVICEREDIRECTION)
                const ClientInfo & info = this->front->client_info;

                this->mod = new mod_rdp(t,
                                    this->context->get(STRAUTHID_TARGET_USER),
                                    this->context->get(STRAUTHID_TARGET_PASSWORD),
                                    "0.0.0.0", // client ip is silenced
                                    *this->front,
                                    hostname,
                                    true,
                                    info,
                                    &this->gen,
                                    this->front->keymap.key_flags,
                                    this->ini->globals.debug.mod_rdp);
                this->mod->event.obj = t->sck;

                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->client_info.width, this->front->client_info.height));
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'RDP' suceeded\n");
                }
                this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "");
            }
            break;

            case MCTX_STATUS_VNC:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC'\n");
                }
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                static const char * name = "VNC Target";
                ClientSocketTransport *t = new ClientSocketTransport(
                                                name,
                                                this->context->get(STRAUTHID_TARGET_DEVICE),
                                                atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                                3, 1000,
                                                this->ini->globals.debug.mod_vnc);
                if (!t->connect()){
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed to connect to remote TCP host");
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                else {
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "failed authentification on remote VNC host");
                }
                this->mod = new mod_vnc(t,
                    this->context->get(STRAUTHID_TARGET_USER),
                    this->context->get(STRAUTHID_TARGET_PASSWORD),
                    *this->front,
                    this->front->client_info.width,
                    this->front->client_info.height,
                    this->front->client_info.keylayout,
                    this->front->keymap.key_flags,
                    this->ini->globals.debug.mod_vnc);
                this->mod->event.obj = t->sck;
                this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC' suceeded\n");
                }
                this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "");
            }
            break;

            default:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Unknown backend exception\n");
                }
                if (this->mod != this->no_mod) {
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

};

#endif

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
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou
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

    Inifile * ini;
    uint32_t & verbose;

    submodule_t nextmod;
    int internal_state;
    long id;
    time_t keep_alive_time;

    struct mod_api * mod; /* module interface */
    struct mod_api * no_mod;

    Transport * mod_transport;

    struct Front * front;

    SessionManager * sesman;
    UdevRandom gen;

    Session(wait_obj & front_event, int sck, int * refreshconf, Inifile * ini)
        : refreshconf(refreshconf)
        , front_event(front_event)
        , ini(ini)
        , verbose(this->ini->debug.session)
        , nextmod(INTERNAL_NONE)
        , mod_transport(NULL)
    {
        SocketTransport front_trans("RDP Client", sck, "", 0, this->ini->debug.front);

        try {
            this->sesman = new SessionManager( this->ini
                                             , this->ini->globals.keepalive_grace_delay
                                             , this->ini->globals.max_tick
                                             , this->ini->globals.internal_domain
                                             , this->ini->debug.auth);
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
                    LOG(LOG_INFO, "Session::-------------- %s -------------------", state_names[this->internal_state]);
                previous_state = this->internal_state;

                unsigned max = 0;
                fd_set rfds;
                fd_set wfds;

                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                struct timeval timeout = time_mark;

                this->front_event.add_to_fd_set(rfds, max);
                if (this->front->up_and_running && this->sesman){
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
                            this->session_setup_mod(MCTX_STATUS_CLI, this->nextmod);
                            this->mod->event.set();
                            this->internal_state = SESSION_STATE_RUNNING;
                        }
                    }
                    break;
                    case SESSION_STATE_WAITING_FOR_NEXT_MODULE:
                    {
                        if (this->sesman->event(rfds)){
                            this->sesman->receive_next_module();

                            if (strcmp(this->ini->context.mode_console.c_str(), "force") == 0){
                                this->front->set_console_session(true);
                                LOG(LOG_INFO, "Session::mode console : force");
                            }
                            else if (strcmp(this->ini->context.mode_console.c_str(), "forbid") == 0){
                                this->front->set_console_session(false);
                                LOG(LOG_INFO, "Session::mode console : forbid");
                            }
                            else {
                                // default is "allow", do nothing special
                            }

                            this->remove_mod();
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

                            if (strcmp(this->ini->context.mode_console.c_str(), "force") == 0){
                                this->front->set_console_session(true);
                                LOG(LOG_INFO, "Session::mode console : force");
                            }
                            else if (strcmp(this->ini->context.mode_console.c_str(), "forbid") == 0){
                                this->front->set_console_session(false);
                                LOG(LOG_INFO, "Session::mode console : forbid");
                            }
                            else {
                                // default is "allow", do nothing special
                            }

                            this->mod->refresh_context(*this->ini);

                            this->mod->event.set();
                            this->internal_state = SESSION_STATE_RUNNING;
                        }
                    }
                    break;
                    case SESSION_STATE_RUNNING:
                    {
                        time_t timestamp = time(NULL);
                        this->front->periodic_snapshot(this->mod->get_pointer_displayed());

                        if (this->sesman && !this->sesman->keep_alive(rfds, this->keep_alive_time, timestamp, &front_trans)){
                            this->nextmod = INTERNAL_CLOSE;
                            this->session_setup_mod(MCTX_STATUS_INTERNAL, this->nextmod);
                            this->keep_alive_time = 0;
                            this->internal_state = SESSION_STATE_RUNNING;
                            this->front->stop_capture();
                            delete this->sesman;
                            this->sesman = NULL;
                        }
                        // Check if sesman received an answer to auth_channel_target
                        if (this->ini->globals.auth_channel[0]) {
                            // Get sesman answer to AUTHCHANNEL_TARGET
                            if (!this->ini->context.authchannel_answer.is_empty()) {
                                // If set, transmit to auth_channel channel
                                this->mod->send_auth_channel_data(
                                    this->ini->context.authchannel_answer.c_str());
                                // Erase the context variable
                                this->ini->context.authchannel_answer.empty();
                            }
                        }

                        // data incoming from server module
                        if (this->front->up_and_running
                        &&  this->mod->event.is_set(rfds)){
                            this->mod->event.reset();
                            if (this->verbose & 8){
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
                            {
                                if (this->verbose & 8){
                                    LOG(LOG_INFO, "Session::back event refresh");
                                }
                                bool record_video = false;
                                bool keep_alive = false;
                                int next_state = this->sesman->ask_next_module(
                                                                    this->keep_alive_time,
                                                                    this->ini->globals.authip,
                                                                    this->ini->globals.authport,
                                                                    record_video, keep_alive, this->nextmod);
                                if (next_state != MCTX_STATUS_WAITING){
                                    this->internal_state = SESSION_STATE_RUNNING;
                                }
                                else {
                                    this->internal_state = SESSION_STATE_WAITING_FOR_CONTEXT;
                                }
                            }
                            break;
                            case BACK_EVENT_NEXT:
                            default:
                            {
                                if (this->verbose & 8){
                                   LOG(LOG_INFO, "Session::back event end module");
                                }
                               // end the current module and switch to new one
                                this->remove_mod();
                                this->ini->context.opt_width  = this->front->client_info.width;
                                this->ini->context.opt_height = this->front->client_info.height;
                                this->ini->context.opt_bpp    = this->front->client_info.bpp;
                                bool record_video = false;
                                bool keep_alive = false;
                                if (!this->sesman){
                                    LOG(LOG_INFO, "Session::no authentifier available, closing");
                                    this->internal_state = SESSION_STATE_CLOSE_CONNECTION;
                                    this->nextmod = INTERNAL_CLOSE;
                                    this->session_setup_mod(MCTX_STATUS_INTERNAL, this->nextmod);
                                    this->keep_alive_time = 0;
                                    this->internal_state = SESSION_STATE_RUNNING;
                                    this->front->stop_capture();
                                }
                                else {
                                    if (this->verbose & 8){
                                        LOG(LOG_INFO, "Session::starting next module");
                                    }

                                    int next_state = this->sesman->ask_next_module(
                                                                        this->keep_alive_time,
                                                                        this->ini->globals.authip,
                                                                        this->ini->globals.authport,
                                                                        record_video, keep_alive,
                                                                        this->nextmod);
                                    if (this->verbose & 8){
                                        LOG(LOG_INFO, "session::next_state %u", next_state);
                                    }

                                    if (next_state != MCTX_STATUS_WAITING){
                                        this->internal_state = SESSION_STATE_STOP;
                                        try {
                                            this->session_setup_mod(next_state, this->nextmod);
                                            if (record_video) {
                                                this->front->start_capture(
                                                    this->front->client_info.width,
                                                    this->front->client_info.height,
                                                    *this->ini
                                                    );
                                            }
                                            else {
                                                this->front->stop_capture();
                                            }
                                            if (this->sesman && keep_alive){
                                                this->sesman->start_keep_alive(keep_alive_time);
                                            }
                                            this->internal_state = SESSION_STATE_RUNNING;
                                        }
                                        catch (const Error & e) {
                                            LOG(LOG_INFO, "Session::connect failed Error=%u", e.id);
                                            this->nextmod = INTERNAL_CLOSE;
                                            this->session_setup_mod(MCTX_STATUS_INTERNAL, this->nextmod);
                                            this->keep_alive_time = 0;
                                            delete sesman;
                                            this->sesman = NULL;
                                            this->internal_state = SESSION_STATE_RUNNING;
                                            this->front->stop_capture();
                                            LOG(LOG_INFO, "Session::capture stopped, authentifier stopped");
                                        }
                                    }
                                    else {
                                        this->internal_state = SESSION_STATE_WAITING_FOR_NEXT_MODULE;
                                    }
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
        catch (const Error & e) {
            LOG(LOG_INFO, "Session::Session exception = %d!\n", e.id);
        }
        catch(...){
            LOG(LOG_INFO, "Session::Session other exception\n");
        }
        LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        this->front->stop_capture();
    }

    ~Session()
    {
        delete this->front;
        this->remove_mod();
        delete this->no_mod;
        delete this->sesman;
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

    TODO("We shoudl be able to flatten MCTX_STATUS and submodule, combining the two we get the desired target")
    void session_setup_mod(int target_module, submodule_t submodule)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::session_setup_mod(target_module=%u, submodule=%u)", target_module, (unsigned)submodule);
        }

        if (strcmp(this->ini->context.mode_console.c_str(), "force") == 0){
            this->front->set_console_session(true);
            LOG(LOG_INFO, "Session::mode console : force");
        }
        else if (strcmp(this->ini->context.mode_console.c_str(), "forbid") == 0){
            this->front->set_console_session(false);
            LOG(LOG_INFO, "Session::mode console : forbid");
        }
        else {
            // default is "allow", do nothing special
        }

        this->remove_mod();

        switch (target_module)
        {
            case MCTX_STATUS_CLI:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'CLI parse'");
                }
                this->mod = new cli_mod(*(this->ini), *(this->front),
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
                switch (submodule){
                    case INTERNAL_CLOSE:
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of new mod 'INTERNAL::Close'");
                        }
                        if (this->ini->context.auth_error_message.is_empty()) {
                            this->ini->context.auth_error_message.copy_c_str("Connection to server ended");
                        }
                        this->mod = new WabCloseMod(*this->ini,
                                                    *this->front,
                                                    this->front->client_info.width,
                                                    this->front->client_info.height);
                        this->front->init_pointers();
                    }
                    if (this->verbose){
                        LOG(LOG_INFO, "Session::internal module Close ready");
                    }
                    break;
                    case INTERNAL_BOUNCER2:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'bouncer2'");
                        }
                        this->mod = new Bouncer2Mod(*this->front,
                                                    this->front->client_info.width,
                                                    this->front->client_info.height
                                                    );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'bouncer2' ready");
                        }
                    break;
                    case INTERNAL_TEST:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'test'");
                        }
                        this->mod = new ReplayMod(
                              *this->front
                            , this->ini->video.replay_path
                            , this->ini->context.movie
                            , this->front->client_info.width
                            , this->front->client_info.height
                            , this->ini->context.auth_error_message
                            );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'test' ready");
                        }
                    break;
                    case INTERNAL_CARD:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'test_card'");
                        }
                        this->mod = new TestCardMod(*this->front,
                                                    this->front->client_info.width,
                                                    this->front->client_info.height
                                                    );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'test_card' ready");
                        }
                    break;
                    case INTERNAL_WIDGET2_SELECTOR:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'selector'");
                        }
                        this->mod = new SelectorMod(*this->ini,
                                                    *this->front,
                                                    this->front->client_info.width,
                                                    this->front->client_info.height
                                                    );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'selector' ready");
                        }
                    break;
                    case INTERNAL_WIDGET2_CLOSE:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'CloseMod'");
                        }
                        this->mod = new WabCloseMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height
                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'CloseMod' ready");
                        }
                    break;
                    case INTERNAL_DIALOG_VALID_MESSAGE:
                    case INTERNAL_WIDGET2_DIALOG:
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Dialog Accept Message'");
                        }

                        const char * message = this->ini->context.message.c_str();
                        const char * button = this->ini->translation.button_refused.c_str();
                        const char * caption = "Information";
                        this->mod = new DialogMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height,
                            caption,
                            message,
                            button
                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'Dialog Accept Message' ready");
                        }
                    }
                    break;
                    case INTERNAL_DIALOG_DISPLAY_MESSAGE:
                    case INTERNAL_WIDGET2_MESSAGE:
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Dialog Display Message'");
                        }

                        const char * message = this->ini->context.message.c_str();
                        const char * button = NULL;
                        const char * caption = "Information";
                        this->mod = new DialogMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height,
                            caption,
                            message,
                            button
                        );
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module 'Dialog Display Message' ready");
                        }
                    }
                    break;
                    case INTERNAL_WIDGET2_LOGIN:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Login'");
                        }
                        this->mod = new LoginMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height);
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module Login ready");
                        }
                        break;
                    case INTERNAL_WIDGET2_RWL:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Login'");
                        }
                        this->mod = new RwlMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height);
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module Login ready");
                        }
                        break;
                    case INTERNAL_WIDGET2_RWL_LOGIN:
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of internal module 'Login'");
                        }
                        this->mod = new RwlLoginMod(
                            *this->ini,
                            *this->front,
                            this->front->client_info.width,
                            this->front->client_info.height);
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::internal module Login ready");
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

                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port,
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
                    , this->ini->context.target_port
                    , this->ini->debug.mod_xup);
                this->mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote X host");
                this->mod = new xup_mod( t
                                       , *this->front
                                       , this->front->client_info.width
                                       , this->front->client_info.height
                                       , this->ini->context.opt_width
                                       , this->ini->context.opt_height
                                       , this->ini->context.opt_bpp
                                       );
                this->mod->event.obj = client_sck;
                this->mod->draw_event();
//                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                this->ini->context.auth_error_message.empty();
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
                REDOC("hostname is the name of the RDP host ('windows' hostname) it is **not** used to get an ip address.")
                char hostname[255];
                hostname[0] = 0;
                if (this->front->client_info.hostname[0]){
                    memcpy(hostname, this->front->client_info.hostname, 31);
                    hostname[31] = 0;
                }
                static const char * name = "RDP Target";

                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port,
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
                    , this->ini->context.target_port
                    , this->ini->debug.mod_rdp
                    , &this->ini->context.auth_error_message
                    );
                this->mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote RDP host");
                this->mod = new mod_rdp( t
                                       , this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0)
                                       , this->ini->context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                                       , "0.0.0.0"  // client ip is silenced
                                       , *this->front
                                       , hostname
                                       , true
                                       , this->front->client_info
                                       , &this->gen
                                       , this->front->keymap.key_flags
                                       , this->sesman   // we give mod_rdp a direct access to sesman for auth_channel channel
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
                this->mod->event.obj = client_sck;

                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->client_info.width, this->front->client_info.height));
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'RDP' suceeded\n");
                }
                this->ini->context.auth_error_message.empty();
            }
            break;

            case MCTX_STATUS_VNC:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC'\n");
                }
                static const char * name = "VNC Target";


                int client_sck = ip_connect(this->ini->context_get_value(AUTHID_TARGET_DEVICE, NULL, 0),
                                            this->ini->context.target_port,
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
                    , this->ini->context.target_port
                    , this->ini->debug.mod_vnc);
                this->mod_transport = t;

                this->ini->context.auth_error_message.copy_c_str("failed authentification on remote VNC host");

                this->mod = new mod_vnc(
                      t
                    , this->ini->context_get_value(AUTHID_TARGET_USER, NULL, 0)
                    , this->ini->context_get_value(AUTHID_TARGET_PASSWORD, NULL, 0)
                    , *this->front
                    , this->front->client_info.width
                    , this->front->client_info.height
                    , this->front->client_info.keylayout
                    , this->front->keymap.key_flags
                    , this->ini->client.clipboard
                    , true /* RRE encoding */
                    , this->ini->debug.mod_vnc);
                this->mod->event.obj = client_sck;
                this->mod->draw_event();

                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC' suceeded\n");
                }
                this->ini->context.auth_error_message.empty();
            }
            break;

            default:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Unknown backend exception\n");
                }
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }
};

#endif

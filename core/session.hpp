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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#if !defined(__CORE_SESSION_HPP__)
#define __CORE_SESSION_HPP__

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

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

static ProtocolKeyword KeywordsDefinitions[] = {
    {STRAUTHID_OPT_MOVIE_PATH, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_ERROR_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_HOST, TYPE_TEXT, "!"},
    {STRAUTHID_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_DEVICE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PORT, TYPE_INTEGER, "!3389"},
    {STRAUTHID_TARGET_PROTOCOL, TYPE_TEXT, "!RDP"},
    {STRAUTHID_END_TIME, TYPE_TEXT, "!-"},
    {STRAUTHID_SELECTOR_GROUP_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_DEVICE_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_LINES_PER_PAGE, TYPE_TEXT, "!20"},
    {STRAUTHID_SELECTOR_NUMBER_OF_PAGES, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_CURRENT_PAGE, TYPE_TEXT, "!1"},
    {STRAUTHID_OPT_MOVIE, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_OPT_CLIPBOARD, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_OPT_DEVICEREDIRECTION, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_DISPLAY_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_ACCEPT_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_OPT_WIDTH, TYPE_INTEGER, "!800"},
    {STRAUTHID_OPT_HEIGHT, TYPE_INTEGER, "!600"},
    {STRAUTHID_OPT_BPP, TYPE_INTEGER, "!24"},
    {STRAUTHID_PROXY_TYPE, TYPE_TEXT, "!RDP"},
    {STRAUTHID_AUTHENTICATED, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_SELECTOR, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_KEEPALIVE, TYPE_BOOLEAN, "ASK"},
    {STRAUTHID_END_DATE_CNX, TYPE_INTEGER, "!0"},
    {STRAUTHID_OPT_BITRATE, TYPE_INTEGER, "!40000"},
    {STRAUTHID_OPT_FRAMERATE, TYPE_INTEGER, "!5"},
    {STRAUTHID_OPT_QSCALE, TYPE_INTEGER, "!15"},
    {STRAUTHID_OPT_CODEC_ID, TYPE_TEXT, "!flv"},
    {STRAUTHID_REJECTED, TYPE_TEXT, "!Connection refused by authentifier."},
    // password or AuthenticationInteractive
    {"authentication_challenge", TYPE_TEXT, "!password"},
    // Translation
    {STRAUTHID_TRANS_BUTTON_OK, TYPE_TEXT, "!OK"},
    {STRAUTHID_TRANS_BUTTON_CANCEL, TYPE_TEXT, "!Cancel"},
    {STRAUTHID_TRANS_BUTTON_HELP, TYPE_TEXT, "!Help"},
    {STRAUTHID_TRANS_BUTTON_CLOSE, TYPE_TEXT, "!Close"},
    {STRAUTHID_TRANS_BUTTON_REFUSED, TYPE_TEXT, "!Refused"},
    {STRAUTHID_TRANS_LOGIN, TYPE_TEXT, "!login"},
    {STRAUTHID_TRANS_USERNAME, TYPE_TEXT, "!username"},
    {STRAUTHID_TRANS_PASSWORD, TYPE_TEXT, "!password"},
    {STRAUTHID_TRANS_TARGET, TYPE_TEXT, "!target"},
    {STRAUTHID_TRANS_DIAGNOSTIC, TYPE_TEXT, "!diagnostic"},
    {STRAUTHID_TRANS_CONNECTION_CLOSED, TYPE_TEXT, "!Connection closed"},
    {STRAUTHID_TRANS_HELP_MESSAGE, TYPE_TEXT, "!Help message"},
    {STRAUTHID_MODE_CONSOLE, TYPE_TEXT, "!allow"},
    {STRAUTHID_VIDEO_QUALITY, TYPE_TEXT, "!medium"},
    {STRAUTHID_TIMEZONE, TYPE_INTEGER, "!-3600"},
};

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

    int sck;
    Inifile * ini;
    uint32_t & verbose;

    ModContext * context;
    int internal_state;
    long id;
    struct SocketTransport * trans;
    time_t keep_alive_time;

    wait_obj * front_event;
    wait_obj * back_event;


    struct client_mod * mod; /* module interface */
    struct client_mod * no_mod;

    struct Front* front;
    int mouse_x;
    int mouse_y;

    SessionManager * sesman;
    UdevRandom gen;

    Session(int sck, const char * ip_source, Inifile * ini)
        : sck(sck), ini(ini), verbose(this->ini->globals.debug.session)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::new Session(%u)", sck);
        }
        this->context = new ModContext(
                KeywordsDefinitions,
                sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword));
        this->context->cpy(STRAUTHID_HOST, ip_source);

        this->sesman = new SessionManager(*this->context, this->ini->globals.debug.auth);
        this->sesman->auth_trans_t = 0;

        this->mod = 0;

        this->internal_state = SESSION_STATE_ENTRY;
        this->front_event = new wait_obj(sck);

        /* create these when up and running */
        this->trans = new SocketTransport("RDP Client", sck, this->ini->globals.debug.front);

        /* set non blocking */
        int rv = 0;
    //    rv = fcntl(this->sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);
    //    if (rv < 0){
    //        /* 1 session_main_loop fnctl socket error */
    //        throw 1;
    //    }
        int nodelay = 1;

        /* SOL_TCP IPPROTO_TCP */
        rv = setsockopt(this->sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
        if (rv < 0){
            /* 2 session_main_loop fnctl socket error */
            throw 2;
        }

        this->front = new Front(this->trans, &this->gen, ini);
        this->no_mod = new null_mod(*(this->front));
        this->front->init_mod();
        this->mod = this->no_mod;

        /* module interface */
        this->back_event = 0;
        this->keep_alive_time = 0;
    }


    ~Session()
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::end of Session(%u)", sck);
        }
        delete this->front;
        delete this->front_event;
        delete this->trans;
        if (this->back_event){
            delete this->back_event;
        }
        if (this->mod != this->no_mod){
            delete this->mod;
            this->mod = this->no_mod;
        }
        delete this->no_mod;
        delete this->sesman;
        delete this->context;
    }

    int session_main_loop()
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::session_main_loop()");
        }
        int rv = 0;
        try {
            int previous_state = SESSION_STATE_STOP;
            while (1) {
                int timeout = 50;
                static struct timeval time_mark = { 0, 0 };
                if (time_mark.tv_sec == 0 && time_mark.tv_usec < 500){
                    time_mark.tv_sec = timeout / 1000;
                    time_mark.tv_usec = (timeout % 1000) * 1000;
                }
                switch (this->internal_state)
                {
                    case SESSION_STATE_ENTRY:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "Session::-------------- Initializing client session\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_ENTRY(time_mark);
                    break;
                    case SESSION_STATE_WAITING_FOR_NEXT_MODULE:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "Session::-------------- Waiting for authentifier\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_WAITING_FOR_NEXT_MODULE(time_mark);
                    break;
                    case SESSION_STATE_WAITING_FOR_CONTEXT:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "Session::-------------- Waiting for authentifier (context refresh required)\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_WAITING_FOR_CONTEXT(time_mark);
                    break;
                    case SESSION_STATE_RUNNING:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "Session::-------------- Running\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_RUNNING(time_mark);
                    break;
                    case SESSION_STATE_CLOSE_CONNECTION:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "Session::-------------- Close connection");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_CLOSE_CONNECTION(time_mark);
                    break;
                }
                if (this->internal_state == SESSION_STATE_STOP){
                    if (this->verbose){
                        LOG(LOG_INFO, "Session::Session::session_main_loop::stop required()");
                    }
                    break;
                }
            }
            this->front->disconnect();
        }
        catch(...){
            rv = 1;
        };
        LOG(LOG_INFO, "Session::Client Session Disconnected\n");
        this->front->stop_capture();
        if (this->sck){
            shutdown(this->sck, 2);
            close(this->sck);
        }
       return rv;
    }

    int step_STATE_ENTRY(const struct timeval & time_mark)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::step_STATE_ENTRY(%u.%0.6u)", time_mark.tv_sec, time_mark.tv_usec);
        }
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        select(max + 1, &rfds, &wfds, 0, &timeout);
        if (this->verbose){
            LOG(LOG_INFO, "Session::step_STATE_ENTRY::timeout=%u.%0.6u", timeout.tv_sec, timeout.tv_usec);
        }
        if (this->front_event->is_set()) {
            try {
                this->front->incoming(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };

            if (this->front->up_and_running){
                this->session_setup_mod(MCTX_STATUS_CLI, this->context);
                this->internal_state = SESSION_STATE_RUNNING;
            }
        }

        return this->internal_state;
    }



    int step_STATE_WAITING_FOR_NEXT_MODULE(const struct timeval & time_mark)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::step_STATE_WAITING_FOR_NEXT_MODULE(%u.%0.6u)", time_mark.tv_sec, time_mark.tv_usec);
        }
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        this->sesman->add_to_fd_set(rfds, max);
        select(max + 1, &rfds, &wfds, 0, &timeout);
        if (this->front_event->is_set()) { /* incoming client data */
            try {
                this->front->incoming(*this->mod);
            }
            catch(...){
                LOG(LOG_INFO, "Session::Forced stop from client side");
                return SESSION_STATE_STOP;
            };
        }

        if (this->sesman->event()){
            this->sesman->receive_next_module();
            this->session_setup_mod(MCTX_STATUS_TRANSITORY, this->context);
            this->internal_state = SESSION_STATE_RUNNING;
        }
        return this->internal_state;
    }

    int step_STATE_WAITING_FOR_CONTEXT(const struct timeval & time_mark)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::step_STATE_WAITING_FOR_CONTEXT(%u.%0.6u)", time_mark.tv_sec, time_mark.tv_usec);
        }
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        TODO(" we should manage some **real** timeout here  if context didn't answered in time  then we should close session.")
        struct timeval timeout = { 1, 0 };

        this->front_event->add_to_fd_set(rfds, max);
        this->sesman->add_to_fd_set(rfds, max);
        select(max + 1, &rfds, &wfds, 0, &timeout);

        if (this->front_event->is_set()) { /* incoming client data */
            try {
                this->front->incoming(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        if (this->sesman->event()){
            if (this->verbose){
                LOG(LOG_INFO, "Session::Auth Event");
            }
            this->sesman->receive_next_module();
            this->mod->refresh_context(*this->context);
            this->back_event->set();
            this->internal_state = SESSION_STATE_RUNNING;
        }
        return this->internal_state;
    }

    int step_STATE_RUNNING(const struct timeval & time_mark)
    {
        if (this->verbose > 1000){
            LOG(LOG_INFO, "Session::step_STATE_RUNNING(%u.%0.6u)", time_mark.tv_sec, time_mark.tv_usec);
        }
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        if (this->front->up_and_running){
            this->back_event->add_to_fd_set(rfds, max);
        }
        this->sesman->add_to_fd_set(rfds, max);

        select(max + 1, &rfds, &wfds, 0, &timeout);

        time_t timestamp = time(NULL);
        this->front->periodic_snapshot(this->mod->get_pointer_displayed());

        if (this->front_event->is_set()) { /* incoming client data */
            try {
                this->front->incoming(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        // incoming data from context
        TODO(" this should use the WAIT_FOR_CONTEXT state or some race conditon may cause mayhem")
        if (this->sesman->close_on_timestamp(timestamp)
        || !this->sesman->keep_alive_or_inactivity(this->keep_alive_time, timestamp, this->trans)){
            this->internal_state = SESSION_STATE_STOP;
            this->context->nextmod = ModContext::INTERNAL_CLOSE;
            this->session_setup_mod(MCTX_STATUS_INTERNAL, this->context);
            this->keep_alive_time = 0;
            TODO(" move that to sesman (to hide implementation details)")
            if (this->sesman->auth_event){
                delete this->sesman->auth_event;
                this->sesman->auth_event = 0;
            }
            this->internal_state = SESSION_STATE_RUNNING;
            this->front->stop_capture();
        }

        if (this->front->up_and_running
        && this->back_event->is_set()){ // data incoming from server module
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
                return SESSION_STATE_STOP;
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
                                this->context->get_bool(STRAUTHID_OPT_MOVIE),
                                this->context->get(STRAUTHID_OPT_MOVIE_PATH),
                                this->context->get(STRAUTHID_OPT_CODEC_ID),
                                this->context->get(STRAUTHID_VIDEO_QUALITY));
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
                            if (this->sesman->auth_event){
                                delete this->sesman->auth_event;
                                this->sesman->auth_event = 0;
                            }
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
        return this->internal_state;
    }


    int step_STATE_CLOSE_CONNECTION(const struct timeval & time_mark)
    {
        if (this->verbose){
            LOG(LOG_INFO, "Session::step_STATE_CLOSE_CONNECTION(%u.%0.6u)", time_mark.tv_sec, time_mark.tv_usec);
        }

        struct timeval timeout = time_mark;

        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        this->front_event->add_to_fd_set(rfds, max);
        this->back_event->add_to_fd_set(rfds, max);

        select(max + 1, &rfds, &wfds, 0, &timeout);

        if (this->back_event->is_set()) {
            return SESSION_STATE_STOP;
        }
        if (this->front_event->is_set()) {
            try {
                this->front->incoming(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        return this->internal_state;
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

        TODO(" wait_obj should become implementation details of modules  sesman and front end")
        if (this->back_event) {
            delete this->back_event;
            this->back_event = 0;
        }
        if (this->mod != this->no_mod) {
            delete this->mod;
            this->mod = this->no_mod;
        }

        switch (target_module)
        {
            case MCTX_STATUS_CLI:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'CLI parse'");
                }
                this->back_event = new wait_obj(-1);
                this->front->init_mod();
                this->mod = new cli_mod(*this->context, *(this->front),
                                        this->front->client_info,
                                        this->front->client_info.width,
                                        this->front->client_info.height);
                this->back_event->set();
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'CLI parse' suceeded");
                }
            }
            break;

            case MCTX_STATUS_TRANSITORY:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'TRANSITORY'");
                }
                this->back_event = new wait_obj(-1);
                this->front->init_mod();
                this->mod = new transitory_mod(*(this->front),
                                               this->front->client_info.width,
                                               this->front->client_info.height);
                // Transitory finish immediately
                this->back_event->set();
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'TRANSITORY' suceeded");
                }
            }
            break;

            case MCTX_STATUS_INTERNAL:
            {
                this->back_event = new wait_obj(-1);
                switch (this->context->nextmod){
                    case ModContext::INTERNAL_CLOSE:
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Session::Creation of new mod 'INTERNAL::Close'");
                        }
                        if (this->context->get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                            this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection to server failed");
                        }
                        this->front->init_mod();
                        this->mod = new close_mod(this->back_event, *this->context,
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
                        this->front->init_mod();
                        this->mod = new dialog_mod(
                                        this->back_event,
                                        *this->context,
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
                        this->front->init_mod();
                        this->mod = new dialog_mod(
                                        this->back_event,
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
                        this->front->init_mod();
                        this->mod = new login_mod(
                                        this->back_event,
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
                        this->front->init_mod();
                        this->mod = new bouncer2_mod(this->back_event,
                                                     *this->front,
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
                        this->front->init_mod();
                        this->mod = new test_internal_mod(
                                        this->back_event,
                                        *this->context,
                                        *this->front,
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
                        this->front->init_mod();
                        this->mod = new test_card_mod(
                                        this->back_event,
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
                        this->front->init_mod();
                        this->mod = new selector_mod(
                                        this->back_event,
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
                ClientSocketTransport * t = new ClientSocketTransport(name, 
                                this->context->get(STRAUTHID_TARGET_DEVICE),
                                atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                4, 1000, 
                                this->ini->globals.debug.mod_xup);
                t->connect();
                this->back_event = new wait_obj(t->sck);
                this->front->init_mod();
                this->mod = new xup_mod(t, *this->context, *(this->front),
                                        this->front->client_info.width,
                                        this->front->client_info.height);
                this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
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
                t->connect();
                TODO("Wait obj should work with transport object, not directly with socket")
                this->back_event = new wait_obj(t->sck);
                // enable or disable clipboard
                // this->context->get_bool(STRAUTHID_OPT_CLIPBOARD)
                // enable or disable device redirection
                // this->context->get_bool(STRAUTHID_OPT_DEVICEREDIRECTION)
                this->front->init_mod();

                const ClientInfo & info = this->front->client_info;

                this->mod = new mod_rdp(t,
                                    this->context->get(STRAUTHID_TARGET_USER),
                                    this->context->get(STRAUTHID_TARGET_PASSWORD),
                                    *this->front,
                                    hostname,
                                    true,
                                    info,
                                    &this->gen,
                                    this->ini->globals.debug.mod_rdp);
//                    this->back_event->set();

                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->client_info.width, this->front->client_info.height));
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'RDP' suceeded\n");
                }
            }
            break;

            case MCTX_STATUS_VNC:
            {
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC'\n");
                }
                static const char * name = "VNC Target";
                ClientSocketTransport *t = new ClientSocketTransport(
                                                name, 
                                                this->context->get(STRAUTHID_TARGET_DEVICE),
                                                atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                                3, 1000,
                                                this->ini->globals.debug.mod_vnc);
                t->connect();
                this->back_event = new wait_obj(t->sck);
                this->front->init_mod();
                this->mod = new mod_vnc(t,
                    this->context->get(STRAUTHID_TARGET_USER),
                    this->context->get(STRAUTHID_TARGET_PASSWORD),
                    *this->front,
                    this->front->client_info.width,
                    this->front->client_info.height,
                    this->ini->globals.debug.mod_vnc);
                this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                if (this->verbose){
                    LOG(LOG_INFO, "Session::Creation of new mod 'VNC' suceeded\n");
                }
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

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

#if !defined(__SESSION_HPP__)
#define __SESSION_HPP__

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
#include "file_loc.hpp"
#include "rect.hpp"
#include "client_info.hpp"

#include "config.hpp"
#include "wait_obj.hpp"
#include "transport.hpp"
#include "bitmap.hpp"
#include "modcontext.hpp"

#include "sesman.hpp"
#include "front.hpp"
#include "../mod/null/null.hpp"
#include "../mod/internal/login.hpp"
#include "../mod/internal/bouncer2.hpp"
#include "../mod/internal/close.hpp"
#include "../mod/internal/dialog.hpp"
#include "../mod/internal/test_card.hpp"
#include "../mod/internal/test_internal.hpp"
#include "../mod/internal/selector.hpp"
#include "../mod/rdp/rdp.hpp"
#include "../mod/vnc/vnc.hpp"
#include "../mod/xup/xup.hpp"
#include "../mod/transitory/transitory.hpp"
#include "../mod/cli/cli_mod.hpp"


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
    // before anything else : exchange of credentials
    SESSION_STATE_RSA_KEY_HANDSHAKE,
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

static inline int get_pixel(uint8_t* data, int x, int y, int width, int bpp)
{
    int pixels_per_byte = 8/bpp;
    int real_width = (width + pixels_per_byte-1) / pixels_per_byte;
    int start = y * real_width + x / pixels_per_byte;
    int shift = x & (pixels_per_byte-1);

    #warning this need some cleanup, but we should define unit tests before correcting it, because mistaking is easy in these kind of things.
    if (bpp == 1) {
        return (data[start] & (0x80 >> shift)) != 0;
    } else if (bpp == 4) {
        if (shift == 0) {
            return (data[start] & 0xf0) >> 4;
        } else {
            return data[start] & 0x0f;
        }
    }
    return 0;
}


static inline int load_pointer(const char* file_name, uint8_t* data, uint8_t* mask, int* x, int* y)
{
    int rv = 0;

    try {
        if (access(file_name, F_OK)){
            LOG(LOG_WARNING, "pointer file [%s] does not exist\n", file_name);
            throw 1;
        }
        Stream stream(8192);
        int fd = open(file_name, O_RDONLY);
        if (fd < 1) {
            LOG(LOG_WARNING, "loading pointer from file [%s] failed\n", file_name);
            throw 1;
        }
        int lg = read(fd, stream.data, 8192);
        if (!lg){
            throw 1;
        }
        close(fd);

        #warning : the ways we do it now we have some risk of reading out of buffer (data that are not from file)

        stream.skip_uint8(6);
        int w = stream.in_uint8();
        int h = stream.in_uint8();
        stream.skip_uint8(2);
        *x = stream.in_uint16_le();
        *y = stream.in_uint16_le();
        stream.skip_uint8(22);
        int bpp = stream.in_uint8();
        stream.skip_uint8(25);

        BGRPalette palette;
        if (w == 32 && h == 32) {
            if (bpp == 1) {
                memcpy(palette, stream.in_uint8p(8), 8);
                // read next 32x32 bytes
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.skip_uint8(128);
            } else if (bpp == 4) {
                memcpy(palette, stream.in_uint8p(64), 64);
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
            #warning probably bogus, we are in case bpp = 4 and we call get_pixel with 1 as bpp
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.skip_uint8(512);
            }
            memcpy(mask, stream.p, 128); /* mask */
        }
    }
    catch(...){
        rv = 1;
    }
    return rv;
}

struct Session {

    ModContext * context;
    int internal_state;
    long id;
    struct SocketTransport * trans;
    time_t keep_alive_time;

    int sck;
    wait_obj * front_event;
    wait_obj * back_event;

    Inifile * ini;

    struct client_mod * mod; /* module interface */
    struct client_mod * no_mod;

    struct Front* front;
    int mouse_x;
    int mouse_y;

    SessionManager * sesman;

    Session(int sck, const char * ip_source, Inifile * ini)
    {
        this->context = new ModContext(
                KeywordsDefinitions,
                sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword));
        this->sesman = new SessionManager(*this->context);
        this->sesman->auth_trans_t = 0;

        this->mod = 0;

        this->internal_state = SESSION_STATE_RSA_KEY_HANDSHAKE;
        this->ini = ini;
        this->sck = sck;
        this->front_event = new wait_obj(sck);

        /* create these when up and running */
        this->trans = new SocketTransport(sck);

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

        this->front = new Front(this->trans, ini);
        this->no_mod = new null_mod(*this->context, *(this->front));
        this->mod = this->no_mod;

        this->context->cpy(STRAUTHID_HOST, ip_source);

        /* module interface */
        this->back_event = 0;
        this->keep_alive_time = 0;
    }


    ~Session()
    {
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
                    case SESSION_STATE_RSA_KEY_HANDSHAKE:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- RSA Key Handshake\n");
                        this->internal_state = this->step_STATE_KEY_HANDSHAKE(time_mark);
                    break;
                    case SESSION_STATE_ENTRY:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- Initializing client session\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_ENTRY(time_mark);
                    break;
                    case SESSION_STATE_WAITING_FOR_NEXT_MODULE:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- Waiting for authentifier\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_WAITING_FOR_NEXT_MODULE(time_mark);
                    break;
                    case SESSION_STATE_WAITING_FOR_CONTEXT:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- Waiting for authentifier (context refresh required)\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_WAITING_FOR_CONTEXT();
                    break;
                    case SESSION_STATE_RUNNING:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- Running\n");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_RUNNING(time_mark);
                    break;
                    case SESSION_STATE_CLOSE_CONNECTION:
                        if (this->internal_state != previous_state)
                            LOG(LOG_DEBUG, "-------------- Close connection");
                        previous_state = this->internal_state;
                        this->internal_state = this->step_STATE_CLOSE_CONNECTION();
                    break;
                }
                if (this->internal_state == SESSION_STATE_STOP){
                    break;
                }
            }
            this->front->disconnect();
        }
        catch(...){
            rv = 1;
        };
        LOG(LOG_INFO, "Client Session Disconnected\n");
        this->mod->stop_capture();
        if (this->sck){
            shutdown(this->sck, 2);
            close(this->sck);
        }
       return rv;
    }

    int step_STATE_KEY_HANDSHAKE(const struct timeval & time)
    {
        this->front->incoming();
        return SESSION_STATE_ENTRY;
    }

    int step_STATE_ENTRY(const struct timeval & time_mark)
    {
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        select(max + 1, &rfds, &wfds, 0, &timeout);
        if (this->front_event->is_set()) {
            try {
                this->front->activate_and_process_data(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };

            if (this->front->up_and_running){
                // if we reach this point we are up_and_running,
                // hence width and height and colors and keymap are availables
                /* resize the main window */
                this->mod->front_resize();
                this->mod->gd.server_reset_clip();
                this->front->reset();
                this->front->set_keyboard_layout();

                BGRPalette palette;
                init_palette332(palette);

                this->mod->gd.color_cache(palette, 0);

                struct pointer_item pointer_item;

                memset(&pointer_item, 0, sizeof(pointer_item));
                load_pointer(SHARE_PATH "/" CURSOR0,
                    pointer_item.data,
                    pointer_item.mask,
                    &pointer_item.x,
                    &pointer_item.y);

                this->front->cache.add_pointer_static(&pointer_item, 0);
                this->front->send_pointer(0,
                                 pointer_item.data,
                                 pointer_item.mask,
                                 pointer_item.x,
                                 pointer_item.y);

                memset(&pointer_item, 0, sizeof(pointer_item));
                load_pointer(SHARE_PATH "/" CURSOR1,
                    pointer_item.data,
                    pointer_item.mask,
                    &pointer_item.x,
                    &pointer_item.y);
                this->front->cache.add_pointer_static(&pointer_item, 1);

                this->front->send_pointer(1,
                                 pointer_item.data,
                                 pointer_item.mask,
                                 pointer_item.x,
                                 pointer_item.y);

                if (this->front->get_client_info().username[0]){
                    this->context->parse_username(this->front->get_client_info().username);
                }

                if (this->front->get_client_info().password[0]){
                    this->context->cpy(STRAUTHID_PASSWORD, this->front->get_client_info().password);
                }

                this->internal_state = SESSION_STATE_RUNNING;
                this->session_setup_mod(MCTX_STATUS_CLI, this->context);
            }
        }

        return this->internal_state;
    }



    int step_STATE_WAITING_FOR_NEXT_MODULE(const struct timeval & time_mark)
    {
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
                this->front->activate_and_process_data(*this->mod);
            }
            catch(...){
                LOG(LOG_INFO, "Forced stop from client side");
                return SESSION_STATE_STOP;
            };
        }

        if (this->sesman->event()){
            this->sesman->receive_next_module();
            if (this->session_setup_mod(MCTX_STATUS_TRANSITORY, this->context)){
                    this->internal_state = SESSION_STATE_RUNNING;
            }
        }
        return this->internal_state;
    }

    int step_STATE_WAITING_FOR_CONTEXT()
    {
        struct timeval time_mark = { 1, 0 };
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        #warning we should manage some **real** timeout here, if context didn't answered in time, then we should close session.
        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        this->sesman->add_to_fd_set(rfds, max);
        select(max + 1, &rfds, &wfds, 0, &timeout);

        if (this->front_event->is_set()) { /* incoming client data */
            try {
                this->front->activate_and_process_data(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        if (this->sesman->event()){
            LOG(LOG_INFO, "Auth Event");
            this->sesman->receive_next_module();
            this->mod->refresh_context(*this->context);
            this->back_event->set();
            this->internal_state = SESSION_STATE_RUNNING;
        }
        return this->internal_state;
    }

    int step_STATE_RUNNING(const struct timeval & time_mark)
    {
        unsigned max = 0;
        fd_set rfds;
        fd_set wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        struct timeval timeout = time_mark;

        this->front_event->add_to_fd_set(rfds, max);
        this->back_event->add_to_fd_set(rfds, max);
        this->sesman->add_to_fd_set(rfds, max);

        select(max + 1, &rfds, &wfds, 0, &timeout);

        time_t timestamp = time(NULL);
        this->mod->periodic_snapshot(this->mod->get_pointer_displayed());

        if (this->front_event->is_set()) { /* incoming client data */
            try {
                this->front->activate_and_process_data(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        // incoming data from context
        #warning this should use the WAIT_FOR_CONTEXT state or some race conditon may cause mayhem
        if (this->sesman->close_on_timestamp(timestamp)
        || !this->sesman->keep_alive_or_inactivity(this->keep_alive_time, timestamp, this->trans)){
            this->internal_state = SESSION_STATE_STOP;
            this->context->nextmod = ModContext::INTERNAL_CLOSE;
            if (this->session_setup_mod(MCTX_STATUS_INTERNAL, this->context)){
                this->keep_alive_time = 0;
                #warning move that to sesman (to hide implementation details)
                if (this->sesman->auth_event){
                    delete this->sesman->auth_event;
                    this->sesman->auth_event = 0;
                }
                this->internal_state = SESSION_STATE_RUNNING;
            }
            this->mod->stop_capture();
        }

        if (this->back_event->is_set()){ // data incoming from server module
    //        LOG(LOG_INFO, "back_event fired");
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
            LOG(LOG_INFO, "back event refresh");
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
                    if (this->session_setup_mod(next_state, this->context)){
                        this->internal_state = SESSION_STATE_RUNNING;
                    }
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
                // end the current module and switch to new one
                if (this->mod != this->no_mod){
                    delete this->mod;
                    this->mod = this->no_mod;
                }
                this->context->cpy(STRAUTHID_OPT_WIDTH, this->front->get_client_info().width);
                this->context->cpy(STRAUTHID_OPT_HEIGHT, this->front->get_client_info().height);
                this->context->cpy(STRAUTHID_OPT_BPP, this->front->get_client_info().bpp);
                bool record_video = false;
                bool keep_alive = false;
                LOG(LOG_INFO, "asking next module");
                int next_state = this->sesman->ask_next_module(
                                                    this->keep_alive_time,
                                                    this->ini->globals.authip,
                                                    this->ini->globals.authport,
                                                    record_video, keep_alive);
                if (next_state != MCTX_STATUS_WAITING){
                    this->internal_state = SESSION_STATE_STOP;
                    if (this->session_setup_mod(next_state, this->context)){
                        if (record_video) {
                            this->mod->start_capture(
                                this->mod->gd.get_front_width(),
                                this->mod->gd.get_front_height(),
                                this->context->get_bool(STRAUTHID_OPT_MOVIE),
                                this->context->get(STRAUTHID_OPT_MOVIE_PATH),
                                this->context->get(STRAUTHID_OPT_CODEC_ID),
                                this->context->get(STRAUTHID_VIDEO_QUALITY),
                                atoi(this->context->get(STRAUTHID_TIMEZONE)));
                        }
                        else {
                            this->mod->stop_capture();
                        }
                        if (keep_alive){
                            this->sesman->start_keep_alive(keep_alive_time);
                        }
                        this->internal_state = SESSION_STATE_RUNNING;
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

    int step_STATE_CLOSE_CONNECTION()
    {
        unsigned max = 0;
        fd_set rfds;

        FD_ZERO(&rfds);

        this->front_event->add_to_fd_set(rfds, max);
        this->back_event->add_to_fd_set(rfds, max);

        if (this->back_event->is_set()) {
            return SESSION_STATE_STOP;
        }
        if (this->front_event->is_set()) {
            try {
                this->front->activate_and_process_data(*this->mod);
            }
            catch(...){
                return SESSION_STATE_STOP;
            };
        }

        return this->internal_state;
    }


    bool session_setup_mod(int status, const ModContext * context)
    {
        try {
            if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"force")==0){
                this->front->set_console_session(true);
                LOG(LOG_INFO, "mode console : force");
            }
            else if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"forbid")==0){
                this->front->set_console_session(false);
                LOG(LOG_INFO, "mode console : forbid");
            }
            else {
                // default is "allow", do nothing special
            }

            #warning wait_obj should become implementation details of modules, sesman and front end
            if (this->back_event) {
                delete this->back_event;
                this->back_event = 0;
            }
            if (this->mod != this->no_mod) {
                delete this->mod;
                this->mod = this->no_mod;
            }

            switch (status)
            {
                case MCTX_STATUS_CLI:
                {
                    this->back_event = new wait_obj(-1);
                    this->mod = new cli_mod(*this->context, *(this->front));
                    this->back_event->set();
                    LOG(LOG_INFO, "Creation of new mod 'CLI parse' suceeded\n");
                }
                break;

                case MCTX_STATUS_TRANSITORY:
                {
                    this->back_event = new wait_obj(-1);
                    this->mod = new transitory_mod(*this->context, *(this->front));
                    // Transitory finish immediately
                    this->back_event->set();
                    LOG(LOG_INFO, "Creation of new mod 'TRANSITORY' suceeded\n");
                }
                break;

                case MCTX_STATUS_INTERNAL:
                {
                    this->back_event = new wait_obj(-1);
                    switch (this->context->nextmod){
                        case ModContext::INTERNAL_CLOSE:
                        {
                            if (this->context->get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                                this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection to server failed");
                            }
                            this->mod = new close_mod(this->back_event, *this->context, *this->front, this->ini);

                            #warning we should probably send mouse pointers before any internal module connection
                            struct pointer_item pointer_item;

                            memset(&pointer_item, 0, sizeof(pointer_item));
                            load_pointer(SHARE_PATH "/" CURSOR0,
                                    pointer_item.data,
                                    pointer_item.mask,
                                    &pointer_item.x,
                                    &pointer_item.y);

                            this->front->cache.add_pointer_static(&pointer_item, 0);
                            this->front->send_pointer(0,
                                    pointer_item.data,
                                    pointer_item.mask,
                                    pointer_item.x,
                                    pointer_item.y);

                            memset(&pointer_item, 0, sizeof(pointer_item));
                            load_pointer(SHARE_PATH "/" CURSOR1,
                                    pointer_item.data,
                                    pointer_item.mask,
                                    &pointer_item.x,
                                    &pointer_item.y);
                            this->front->cache.add_pointer_static(&pointer_item, 1);

                            this->front->send_pointer(1,
                                    pointer_item.data,
                                    pointer_item.mask,
                                    pointer_item.x,
                                    pointer_item.y);
                        }
                        LOG(LOG_INFO, "internal module Close ready");
                        break;
                        case ModContext::INTERNAL_DIALOG_VALID_MESSAGE:
                        {
                            const char * message = NULL;
                            const char * button = NULL;
                            LOG(LOG_INFO, "Creation of internal module 'Dialog Accept Message'");
                            message = this->context->get(STRAUTHID_MESSAGE);
                            button = this->context->get(STRAUTHID_TRANS_BUTTON_REFUSED);
                            this->mod = new dialog_mod(
                                            this->back_event,
                                            *this->context,
                                            *this->front,
                                            message,
                                            button,
                                            this->ini);
                        }
                        LOG(LOG_INFO, "internal module 'Dialog Accept Message' ready");
                        break;

                        case ModContext::INTERNAL_DIALOG_DISPLAY_MESSAGE:
                        {
                            const char * message = NULL;
                            const char * button = NULL;
                            LOG(LOG_INFO, "Creation of internal module 'Dialog Display Message'");
                            message = this->context->get(STRAUTHID_MESSAGE);
                            button = NULL;
                            this->mod = new dialog_mod(
                                            this->back_event,
                                            *this->context,
                                            *this->front,
                                            message,
                                            button,
                                            this->ini);
                        }
                        LOG(LOG_INFO, "internal module 'Dialog Display Message' ready");
                        break;
                        case ModContext::INTERNAL_LOGIN:
                            LOG(LOG_INFO, "Creation of internal module 'Login'");
                            this->mod = new login_mod(
                                            this->back_event,
                                             *this->context,
                                             *this->front,
                                             this->ini);
                            LOG(LOG_INFO, "internal module Login ready");
                        break;
                        case ModContext::INTERNAL_BOUNCER2:
                            LOG(LOG_INFO, "Creation of internal module 'bouncer2'");
                            this->mod = new bouncer2_mod(this->back_event, *this->front);
                            LOG(LOG_INFO, "internal module 'bouncer2' ready");
                        break;
                        case ModContext::INTERNAL_TEST:
                            LOG(LOG_INFO, "Creation of internal module 'test'");
                            this->mod = new test_internal_mod(
                                            this->back_event,
                                            *this->context,
                                            *this->front);
                            LOG(LOG_INFO, "internal module 'test' ready");
                        break;
                        case ModContext::INTERNAL_CARD:
                            LOG(LOG_INFO, "Creation of internal module 'test_card'");
                            this->mod = new test_card_mod(
                                            this->back_event,
                                            *this->context,
                                            *this->front);
                            LOG(LOG_INFO, "internal module 'test_card' ready");
                        break;
                        case ModContext::INTERNAL_SELECTOR:
                            LOG(LOG_INFO, "Creation of internal module 'selector'");
                            this->mod = new selector_mod(
                                            this->back_event,
                                            *this->context,
                                            *this->front);
                            LOG(LOG_INFO, "internal module 'selector' ready");
                        break;
                        default:
                        break;
                    }
                }
                break;

                case MCTX_STATUS_XUP:
                {
                    SocketTransport * t = new SocketTransport(
                                                connect(this->context->get(STRAUTHID_TARGET_DEVICE),
                                                atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                                4, 2500000));
                    this->back_event = new wait_obj(t->sck);
                    this->mod = new xup_mod(t, *this->context, *(this->front));
                    this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                    LOG(LOG_INFO, "Creation of new mod 'XUP' suceeded\n");
                }
                break;

                case MCTX_STATUS_RDP:
                {
                    // hostname is the name of the RDP host ("windows" hostname)
                    // it is **not** used to get an ip address.
                    char hostname[255];
                    hostname[0] = 0;
                    if (this->front->get_client_info().hostname[0]){
                        memcpy(hostname, this->front->get_client_info().hostname, 31);
                        hostname[31] = 0;
                    }
                    SocketTransport * t = new SocketTransport(
                                            connect(this->context->get(STRAUTHID_TARGET_DEVICE),
                                                atoi(this->context->get(STRAUTHID_TARGET_PORT))));
                    this->back_event = new wait_obj(t->sck);
                    this->mod = new mod_rdp(t,
                                        *this->back_event,
                                        *this->context,
                                        *(this->front),
                                        hostname,
                                        this->front->get_client_info().keylayout,
                                        this->context->get_bool(STRAUTHID_OPT_CLIPBOARD),
                                        this->context->get_bool(STRAUTHID_OPT_DEVICEREDIRECTION));
//                    this->back_event->set();
                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                    LOG(LOG_INFO, "Creation of new mod 'RDP' suceeded\n");
                }
                break;

                case MCTX_STATUS_VNC:
                {
                    SocketTransport *t = new SocketTransport(
                        connect(this->context->get(STRAUTHID_TARGET_DEVICE), atoi(this->context->get(STRAUTHID_TARGET_PORT))));
                    this->back_event = new wait_obj(t->sck);
                    this->mod = new mod_vnc(t, *this->context, *(this->front), this->front->get_client_info().keylayout);
                    this->mod->draw_event();
//                    this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
                    LOG(LOG_INFO, "Creation of new mod 'VNC' suceeded\n");
                }
                break;

                default:
                {
                    throw Error(ERR_SESSION_UNKNOWN_BACKEND);
                }
            }
        }
        catch (...) {
            return false;
        };

        return true;
    }

};

#endif

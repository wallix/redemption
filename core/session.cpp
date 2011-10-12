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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Session object, in charge of session

*/
#include "graphic_device.hpp"

#include "colors.hpp"
#include "rect.hpp"
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

#include "session.hpp"

#include "region.hpp"
#include "log.hpp"
#include "client_mod.hpp"
#include "transport.hpp"
#include "config.hpp"

#include "error.hpp"
#include "wait_obj.hpp"
#include "constants.hpp"









int Session::step_STATE_RUNNING(const struct timeval & time_mark)
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
        int signal = this->mod->draw_event();
        if (signal){ // signal is the return status from module
                     // (used only for internal modules)
            if (signal == 4){
                return SESSION_STATE_STOP;
            }
            if (this->mod != this->no_mod){
                delete this->mod;
                this->mod = this->no_mod;
            }
            snprintf(this->context->get(STRAUTHID_OPT_WIDTH), 10, "%d", this->front->get_client_info().width);
            snprintf(this->context->get(STRAUTHID_OPT_HEIGHT), 10, "%d", this->front->get_client_info().height);
            snprintf(this->context->get(STRAUTHID_OPT_BPP), 10, "%d", this->front->get_client_info().bpp);
            bool record_video = false;
            bool keep_alive = false;
            LOG(LOG_INFO, "ask next module");
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
    }
    return this->internal_state;
}

int Session::session_main_loop()
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

bool Session::session_setup_mod(int status, const ModContext * context)
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
                    LOG(LOG_INFO, "internal module Dialog Valid Message ready");
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
                    LOG(LOG_INFO, "internal module Dialog Display Message ready");
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
                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
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
                this->back_event->set();
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
                this->mod->rdp_input_invalidate(Rect(0, 0, this->front->get_client_info().width, this->front->get_client_info().height));
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

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

*/

TODO(" rename sesman to authentifier")
#if !defined(__SESMAN3_HPP__)
#define __SESMAN3_HPP__

#include "stream.hpp"
#include "config.hpp"
#include "sesman.hpp"
#include "modcontext.hpp"

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


    ModContext & context;
    int tick_count;
    public:

    struct SocketTransport * auth_trans_t;
    wait_obj * auth_event;
    uint32_t verbose;

    SessionManager(ModContext & context, uint32_t verbose)
        : mod_state(MOD_STATE_INIT), context(context), tick_count(0), verbose(verbose)
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::SessionManager");
        }
        this->auth_trans_t = NULL;
        this->auth_event = 0;
    }

    ~SessionManager()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::~SessionManager");
        }
        if (this->auth_trans_t) {
            delete this->auth_trans_t;
            this->auth_trans_t = 0;
        }
        if (this->auth_event){
            delete this->auth_event;
            this->auth_event = 0;
        }
    }

    bool event(){
        if (this->verbose & 0x40){
            LOG(LOG_INFO, "auth::event?");
        }
        return this->auth_event?this->auth_event->is_set():false;
    }

    void start_keep_alive(long & keepalive_time)
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::start_keep_alive");
        }
        this->tick_count = 1;
        if (this->auth_trans_t){
            Stream stream(8192);

            stream.out_uint32_be(0); // skip length
            this->context.ask(STRAUTHID_KEEPALIVE);
            this->out_item(stream, STRAUTHID_KEEPALIVE);
            // now set length
            int total_length = stream.get_offset(0);
            stream.p = stream.data;
            stream.out_uint32_be(total_length);
            // and send
            this->auth_trans_t->send(stream.data, total_length);
            keepalive_time = ::time(NULL) + 30;
        }
    }

    void in_items(Stream & stream)
    {
        if (this->verbose & 0x40){
            LOG(LOG_INFO, "auth::in_items");
        }
        for (; stream.p < stream.end ; this->in_item(stream)){
                    ;
        }

    }

    void in_item(Stream & stream)
    {
        enum { STATE_KEYWORD, STATE_VALUE } state = STATE_KEYWORD;
        uint8_t * value = stream.p;
        uint8_t * keyword = stream.p;
        const uint8_t * start = stream.p;
        for ( ; stream.p < stream.end ; stream.p++){
            switch (state){
            case STATE_KEYWORD:
                if (*stream.p == '\n'){
                    *stream.p = 0;
                    value = stream.p+1;
                    state = STATE_VALUE;
                }
                break;
            case STATE_VALUE:
                if (*stream.p == '\n'){
                    *stream.p = 0;
                    if (!this->context.get((char*)keyword)) {
                        LOG(LOG_INFO, "keyword %s unknown. Skip it.", keyword);
                    }
                    else
                    {
                        if ((0==strncasecmp((char*)value, "ask", 3))){
                            this->context.ask((char*)keyword);
                            LOG(LOG_INFO, "receiving %s '%s'\n", value, keyword);
                        }
                        else {
                            this->context.cpy((char*)keyword, (char*)value+(value[0]=='!'?1:0));

                            if ((strncasecmp((char*)value, "ask", 3) != 0)
                            && ((strncasecmp("password", (char*)keyword, 8) == 0)
                            || (strncasecmp("target_password", (char*)keyword, 15) == 0))){
                               LOG(LOG_INFO, "receiving '%s'=<hidden>\n", (char*)keyword);
                            }
                            else{
                                LOG(LOG_INFO, "receiving '%s'=%s\n", keyword, this->context.get((char*)keyword));
                            }
                        }
                    }
                    stream.p = stream.p+1;
                    return;
                }
                break;
            }
        }
        LOG(LOG_WARNING, "Unexpected exit while parsing ACL message");
        hexdump((char *)start, stream.p-start);
        throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
    }

    bool close_on_timestamp(long & timestamp)
    {
        bool res = false;
        if (MOD_STATE_DONE_CONNECTED == this->mod_state){
            long enddate = atol(this->context.get(STRAUTHID_END_DATE_CNX));
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

    bool keep_alive_or_inactivity(long & keepalive_time, long & now, Transport * trans)
    {
        // Keepalive Data exchange with sesman
        if (this->auth_trans_t){
            if (this->auth_event?this->auth_event->is_set():false) {
                if (this->verbose & 0x10){
                    LOG(LOG_INFO, "auth::keep_alive_or_inactivity");
                }
                try {
                    this->incoming();
                    keepalive_time = now + 30;
                }
                catch (...){
                    if (this->verbose & 0x10){
                        LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager");
                    }
                    this->context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection closed by manager (socket closed)");
                    return false;
                }
            }
            if (keepalive_time && (now > keepalive_time + 30)){
                if (this->verbose & 0x10){
                    LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
                }
                this->context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection closed by manager");
                return false;
            }
            else if (keepalive_time && (now > keepalive_time)){
                if (this->verbose & 0x8){
                    LOG(LOG_INFO, "%llu bytes sent in last quantum,"
                                  " total: %llu tick:%d",
                                  trans->last_quantum_sent, trans->total_sent,
                                  this->tick_count);
                }
                if (trans->last_quantum_sent == 0){
                    this->tick_count++;
                    if (this->tick_count > 30){ // 10 minutes before closing on inactivity
                        this->context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection closed on inactivity");
                        return false;
                    }
                }
                else {
                    this->tick_count = 0;
                }
                trans->tick();

                keepalive_time = now + 30;
                Stream stream(8192);
                stream.out_uint32_be(0); // skip length
                // set data
                this->context.ask(STRAUTHID_KEEPALIVE);
                this->out_item(stream, STRAUTHID_KEEPALIVE);
                // now set length in header
                int total_length = stream.get_offset(0);
                stream.p = stream.data;
                stream.out_uint32_be(total_length); /* size */
                // and send
                this->auth_trans_t->send(stream.data, total_length);
            }
        }
        return true;
    }

    int get_mod_from_protocol()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::get_mod_from_protocol");
        }
        const char * protocol = this->context.get(STRAUTHID_TARGET_PROTOCOL);
        int res = MCTX_STATUS_EXIT;
        if (strncasecmp(protocol, "RDP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol RDP");
            }
            res = MCTX_STATUS_RDP;
            this->mod_state = MOD_STATE_DONE_CONNECTED;
        }
        else if (strncasecmp(protocol, "VNC", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol VNC");
            }
            res = MCTX_STATUS_VNC;
            this->mod_state = MOD_STATE_DONE_CONNECTED;
        }
        else if (strncasecmp(protocol, "XUP", 4) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol XUP");
            }
            res = MCTX_STATUS_XUP;
            this->mod_state = MOD_STATE_DONE_CONNECTED;
        }
        else if (strncasecmp(protocol, "INTERNAL", 8) == 0){
            if (this->verbose & 0x4){
                LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL");
            }
            res = MCTX_STATUS_INTERNAL;
            char * target = this->context.get(STRAUTHID_TARGET_DEVICE);
            if (0 == strcmp(target, "bouncer2")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL bouncer2");
                }
                this->context.nextmod = ModContext::INTERNAL_BOUNCER2;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
            else if (0 == strcmp(target, "test")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL test");
                }
                this->context.nextmod = ModContext::INTERNAL_TEST;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
            else if (0 == strcmp(target, "selector")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL selector");
                }
                this->context.nextmod = ModContext::INTERNAL_SELECTOR;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
            else if (0 == strcmp(target, "login")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL login");
                }
                this->context.nextmod = ModContext::INTERNAL_LOGIN;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
            else if (0 == strcmp(target, "close")){
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL close");
                }
                this->context.nextmod = ModContext::INTERNAL_CLOSE;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
            else {
                if (this->verbose & 0x4){
                    LOG(LOG_INFO, "auth::get_mod_from_protocol INTERNAL card");
                }
                this->context.nextmod = ModContext::INTERNAL_CARD;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
        }
        else {
            LOG(LOG_WARNING, "Unsupported target protocol %c%c%c%c",
                protocol[0], protocol[1], protocol[2], protocol[3]);
            this->context.nextmod = ModContext::INTERNAL_CARD;
            this->mod_state = MOD_STATE_DONE_CONNECTED;
            assert(false);
        }
        return res;
    }


    int ask_next_module(long & keepalive_time, const char * auth_host, int auth_port, bool & record_video, bool & keep_alive)
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::ask_next_module");
        }
        int next_state = MCTX_STATUS_EXIT;
        switch (this->mod_state){
        default:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module default state");
            }
            next_state = this->ask_next_module_remote(auth_host, auth_port);
        break;
        case MOD_STATE_DONE_SELECTOR:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_SELECTOR state");
            }
            next_state = this->ask_next_module_remote(auth_host, auth_port);
        break;
        case MOD_STATE_DONE_LOGIN:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_LOGIN state");
            }
            next_state = this->ask_next_module_remote(auth_host, auth_port);
        break;
        case MOD_STATE_DONE_PASSWORD:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_PASSWORD state");
            }
            next_state = this->ask_next_module_remote(auth_host, auth_port);
        break;
        case MOD_STATE_DONE_RECEIVED_CREDENTIALS:
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_RECEIVED_CREDENTIALS state");
        }
        {
            if (this->context.is_asked(STRAUTHID_AUTH_USER)){
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_LOGIN;
                this->mod_state = MOD_STATE_DONE_LOGIN;
            }
            else if (this->context.is_asked(STRAUTHID_PASSWORD)){
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_LOGIN;
                this->mod_state = MOD_STATE_DONE_LOGIN;
            }
            else if (!this->context.is_asked(STRAUTHID_SELECTOR)
                 &&   this->context.get_bool(STRAUTHID_SELECTOR)
                 &&  !this->context.is_asked(STRAUTHID_TARGET_DEVICE)
                 &&  !this->context.is_asked(STRAUTHID_TARGET_USER)){
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_SELECTOR;
                this->mod_state = MOD_STATE_DONE_SELECTOR;
            }
            else if (this->context.is_asked(STRAUTHID_TARGET_DEVICE)
                 ||  this->context.is_asked(STRAUTHID_TARGET_USER)){
                    next_state = MCTX_STATUS_INTERNAL;
                    this->context.nextmod = ModContext::INTERNAL_LOGIN;
                    this->mod_state = MOD_STATE_DONE_LOGIN;
            }
            else if (this->context.is_asked(STRAUTHID_DISPLAY_MESSAGE)){
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_DIALOG_DISPLAY_MESSAGE;
                this->mod_state = MOD_STATE_DONE_DISPLAY_MESSAGE;
            }
            else if (this->context.is_asked(STRAUTHID_ACCEPT_MESSAGE)){
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_DIALOG_VALID_MESSAGE;
                this->mod_state = MOD_STATE_DONE_VALID_MESSAGE;
            }
            else if (this->context.get_bool(STRAUTHID_AUTHENTICATED)){
                next_state = this->get_mod_from_protocol();
                record_video = this->context.get_bool(STRAUTHID_OPT_MOVIE);
                keep_alive = true;
                if (context.get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                    context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "End of connection");
                }
            }
            else {
                if (context.get(STRAUTHID_REJECTED)[0] != 0){
                    context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, context.get(STRAUTHID_REJECTED));
                }
                if (context.get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                    context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Authentifier service failed");
                }
                TODO(" check life cycle of auth_trans_t")
                if (this->auth_trans_t){
                    delete this->auth_trans_t;
                    this->auth_trans_t = 0;
                }
                next_state = MCTX_STATUS_INTERNAL;
                this->context.nextmod = ModContext::INTERNAL_CLOSE;
                this->mod_state = MOD_STATE_DONE_CONNECTED;
            }
        }
        break;
        case MOD_STATE_DONE_CONNECTED:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->context.nextmod = ModContext::INTERNAL_CLOSE;
            next_state = MCTX_STATUS_INTERNAL;
            this->mod_state = MOD_STATE_DONE_CLOSE;
        break;
        case MOD_STATE_DONE_CLOSE:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_CONNECTED state");
            }
            this->mod_state = MOD_STATE_DONE_EXIT;
            next_state = MCTX_STATUS_EXIT;
        break;
        case MOD_STATE_DONE_EXIT:
            if (this->verbose & 0x10){
                LOG(LOG_INFO, "auth::ask_next_module MOD_STATE_DONE_EXIT state");
            }
            // we should never goes here, the main loop should have stopped before
            LOG(LOG_WARNING, "unexpected forced exit");
        break;
        }
        return next_state;
    }

    TODO(""move that function to ModContext  create specialized stream object ModContextStream"")

    void out_item(Stream & stream, const char * key)
    {
        if (this->context.is_asked(key)){
            LOG(LOG_INFO, "sending %s=ASK\n", key);
            stream.out_copy_bytes(key, strlen(key));
            stream.out_copy_bytes("\nASK\n",5);
        }
        else {
            const char * tmp = this->context.get(key);
            // do not send empty values
            if (tmp[0] != 0){
                if ((strncasecmp("password", (char*)key, 8) == 0)
                ||(strncasecmp("target_password", (char*)key, 15) == 0)){
                    LOG(LOG_INFO, "sending %s=<hidden>\n", key);
                }
                else {
                    LOG(LOG_INFO, "sending %s=%s\n", key, tmp);
                }
                stream.out_copy_bytes(key, strlen(key));
                stream.out_uint8('\n');
                stream.out_uint8('!');
                stream.out_copy_bytes(tmp, strlen(tmp));
                stream.out_uint8('\n');
            }
        }
    }

    void add_to_fd_set(fd_set & rfds, unsigned & max)
    {
        TODO(" look at concept behind wait_obj")
        if (this->auth_event){
            this->auth_event->add_to_fd_set(rfds, max);
        }
    }


    int ask_next_module_remote(const char * auth_host, int authport)
    {
        // if anything happen, like authentification socked closing, stop current connection
        try {
            Stream stream(8192);

            stream.out_uint32_be(0);
            TODO(" is there a way to make auth_event RAII ? (initialized in sesman constructor)")
            if (!this->auth_trans_t){
                static const char * name = "Authentifier";
                int sck = connect(auth_host, authport, name, 30, 1000);
                this->auth_trans_t = new SocketTransport(name, sck, this->verbose);
                TODO(" create a realloc method")
                if (this->auth_event){
                    delete this->auth_event;
                }
                this->auth_event = new wait_obj(this->auth_trans_t->sck);
            }
            this->out_item(stream, STRAUTHID_PROXY_TYPE);
            this->out_item(stream, STRAUTHID_DISPLAY_MESSAGE);
            this->out_item(stream, STRAUTHID_ACCEPT_MESSAGE);
            this->out_item(stream, STRAUTHID_HOST);
            this->out_item(stream, STRAUTHID_AUTH_USER);
            this->out_item(stream, STRAUTHID_PASSWORD);
            this->out_item(stream, STRAUTHID_TARGET_USER);
            this->out_item(stream, STRAUTHID_TARGET_DEVICE);
            this->out_item(stream, STRAUTHID_SELECTOR);
            this->out_item(stream, STRAUTHID_SELECTOR_GROUP_FILTER);
            this->out_item(stream, STRAUTHID_SELECTOR_DEVICE_FILTER);
            this->out_item(stream, STRAUTHID_SELECTOR_LINES_PER_PAGE);
            this->out_item(stream, STRAUTHID_SELECTOR_CURRENT_PAGE);
            this->out_item(stream, STRAUTHID_TARGET_PASSWORD);
            this->out_item(stream, STRAUTHID_OPT_WIDTH);
            this->out_item(stream, STRAUTHID_OPT_HEIGHT);
            this->out_item(stream, STRAUTHID_OPT_BPP);
            /* translation message */
            this->out_item(stream, STRAUTHID_TRANS_BUTTON_OK);
            this->out_item(stream, STRAUTHID_TRANS_BUTTON_CANCEL);
            this->out_item(stream, STRAUTHID_TRANS_BUTTON_HELP);
            this->out_item(stream, STRAUTHID_TRANS_BUTTON_CLOSE);
            this->out_item(stream, STRAUTHID_TRANS_BUTTON_REFUSED);
            this->out_item(stream, STRAUTHID_TRANS_LOGIN);
            this->out_item(stream, STRAUTHID_TRANS_USERNAME);
            this->out_item(stream, STRAUTHID_TRANS_PASSWORD);
            this->out_item(stream, STRAUTHID_TRANS_TARGET);
            this->out_item(stream, STRAUTHID_TRANS_DIAGNOSTIC);
            this->out_item(stream, STRAUTHID_TRANS_CONNECTION_CLOSED);

            int total_length = stream.get_offset(0);
            stream.p = stream.data;
            stream.out_uint32_be(total_length); /* size */
            this->auth_trans_t->send(stream.data, total_length);

        } catch (Error e) {
            this->context.cpy(STRAUTHID_AUTHENTICATED, false);
            this->context.cpy(STRAUTHID_REJECTED, "Authentifier service failed");
            delete this->auth_trans_t;
            this->auth_trans_t = NULL;
        }
        return MCTX_STATUS_WAITING;
    }

    void incoming()
    {
        Stream stream(4);
        this->auth_trans_t->recv((char**)&(stream.end), 4);
        size_t size = stream.in_uint32_be();
        if (size > 65536){
            LOG(LOG_WARNING, "Error: ACL message too big (got %u max 64 K)", size);
            throw Error(ERR_ACL_MESSAGE_TOO_BIG);
        }
        if (size > stream.capacity){
            stream.init(size);
        }
        this->auth_trans_t->recv((char**)&(stream.end), size - 4);
        this->in_items(stream);
    }

    int receive_next_module()
    {
        try {
            this->incoming();
        } catch (...) {
            this->context.cpy(STRAUTHID_AUTHENTICATED, false);
            this->context.cpy(STRAUTHID_REJECTED, "Authentifier service failed");
            delete this->auth_trans_t;
            this->auth_trans_t = NULL;
        }
        this->mod_state = MOD_STATE_DONE_RECEIVED_CREDENTIALS;
        return MCTX_STATUS_TRANSITORY;
    }

};

#endif

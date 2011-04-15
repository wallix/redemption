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

#warning rename sesman to authentifier
#if !defined(__SESMAN3_HPP__)
#define __SESMAN3_HPP__

#include "stream.hpp"
#include "config.hpp"
#include "sesman.hpp"
#include "modcontext.hpp"

class SessionManager {

    ModContext & context;
    public:

    struct SocketTransport * auth_trans_t;
    wait_obj * auth_event;

    SessionManager(ModContext & context) : context(context) {
        this->auth_trans_t = NULL;
        this->auth_event = 0;
    }

    ~SessionManager()
    {
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
        return this->auth_event?this->auth_event->is_set():false;
    }

    void start_keep_alive(long & keepalive_time)
    {
        if (this->auth_trans_t){
            Stream stream(8192);

            stream.skip_uint8(4); // skip length
            this->out_ask_item(stream, STRAUTHID_KEEPALIVE);
            stream.mark_end();
            // now set length
            stream.p = stream.data;
            int total_length = stream.end - stream.data;
            stream.out_uint32_be(total_length);
            // and send
            this->auth_trans_t->send((char*)stream.data, total_length);
            keepalive_time = ::time(NULL) + 30;
        }
    }

    void in_items(Stream & stream)
    {
        for (stream.p = stream.data + 4
            ; stream.p < stream.end
            ; this->in_item(stream)){
                    ;
        }

    }

    void in_item(Stream & stream)
    {
        enum { STATE_KEYWORD, STATE_VALUE } state = STATE_KEYWORD;
        uint8_t * value = stream.p;
        uint8_t * keyword = stream.p;
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
                            if ((0==strcasecmp((char*)keyword, STRAUTHID_ACCEPT_MESSAGE))
                            || (0==strcasecmp((char*)keyword, STRAUTHID_DISPLAY_MESSAGE))
                            || (0==strcasecmp((char*)keyword, STRAUTHID_TRANS_HELP_MESSAGE))) {
                                this->context.cpy_long((char*)keyword, (char*)(!(0[value]=='!')?0:1)[value]);
                            }
                            else {
                                this->context.cpy((char*)keyword, (char*)value+(value[0]=='!'?1:0));
                            }
                            if ((strncasecmp((char*)value, "ask", 3) != 0)
                            && ((strncasecmp("password", (char*)keyword, 8) == 0)
                            || (strncasecmp("target_password", (char*)keyword, 15) == 0))){
                               LOG(LOG_INFO, "receiving '%s'=<hidden>\n", keyword);
                            }
                            else{
                                LOG(LOG_INFO, "receiving '%s'=%s\n", keyword, value+(!(0[value]=='!')?0:1));
                            }
                        }
                    }
                    stream.p = stream.p+1;
                    return;
                }
                break;
            }
        }
        #warning we should have stopped after fully getting the value, hence this case is an error, see how this exception should be qualified.
        throw 0;
    }

    bool close_on_timestamp(long & timestamp)
    {
        if (MOD_STATE_CONNECTED_RDP == this->context.mod_state){
            long enddate = atol(this->context.get(STRAUTHID_END_DATE_CNX));
            if (enddate != 0 && (timestamp > enddate)) {
                LOG(LOG_INFO, "Session is out of allowed timeframe : stopping");
                this->context.mod_state = MOD_STATE_MESSAGE_CONNEXION_CLOSE_AT_LIMIT;
                return true;
            }
        }
        return false;
    }

    bool keep_alive(long & keepalive_time, long & now)
    {
        // Keepalive Data exchange with sesman
        if (this->auth_trans_t){
            if (this->auth_event?this->auth_event->is_set():false) {
                try {
                    Stream stream(8192);

                    this->auth_trans_t->recv((char**)&(stream.end), 4);
    #warning check if size is < 8192
                    int size = stream.in_uint32_be();
                    this->auth_trans_t->recv((char**)&(stream.end), size-4);
                    this->in_items(stream);
                    keepalive_time = now + 30;
                }
                catch (...){
                    this->context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection closed by manager");
                    return false;
                }
            }
            if (keepalive_time && (now > keepalive_time + 30)){
                this->context.cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection closed by manager");
                return false;
            }
            else if (keepalive_time && (now > keepalive_time)){
                keepalive_time = now + 30;
                Stream stream(8192);

                // skip length
                stream.skip_uint8(4);
                // set data
                this->out_ask_item(stream, STRAUTHID_KEEPALIVE);
                stream.mark_end();
                // now set length in header
                stream.p = stream.data;
                int total_length = stream.end - stream.data;
                stream.out_uint32_be(total_length); /* size */
                // and send
                this->auth_trans_t->send((char*)stream.data, total_length);
            }
        }
        return true;
    }

    int get_mod_from_protocol()
    {
        const char * protocol = this->context.get(STRAUTHID_TARGET_PROTOCOL);
        int res = MCTX_STATUS_EXIT;
        if (strncasecmp(protocol, "RDP", 4) == 0){
            res = MCTX_STATUS_RDP;
        }
        else if (strncasecmp(protocol, "VNC", 4) == 0){
            res = MCTX_STATUS_VNC;
        }
        else if (strncasecmp(protocol, "XUP", 4) == 0){
            res = MCTX_STATUS_XUP;
        }
        else {
            assert(false);
        }
        return res;
    }


    int ask_next_module(long & keepalive_time, const char * auth_host, int authport, bool & record_video, bool & keep_alive)
    {
        int next_state = MCTX_STATUS_EXIT;
        switch (this->context.mod_state){
        default:
            LOG(LOG_INFO, "DEFAULT MOD STATE\n");
            next_state = this->ask_next_module_remote(auth_host, 3350);
            if (next_state == MCTX_STATUS_CLOSE){
                next_state = MCTX_STATUS_TRANSITORY;
                this->context.mod_state = MOD_STATE_RECEIVED_CREDENTIALS;
            }
        break;
        case MOD_STATE_RECEIVED_CREDENTIALS:
        LOG(LOG_INFO, "RECEIVED CREDENTIALS\n");
        {
            if (this->context.is_asked(STRAUTHID_AUTH_USER)){
                this->context.wab_auth = 0;
                next_state = MCTX_STATUS_LOGIN;
                this->context.mod_state = MOD_STATE_INIT;
            }

            else if (this->context.is_asked(STRAUTHID_TARGET_DEVICE)){
                this->context.wab_auth = 0;
                next_state = MCTX_STATUS_LOGIN;
                this->context.mod_state = MOD_STATE_INIT;
            }

            else if (this->context.is_asked(STRAUTHID_TARGET_USER)){
                this->context.wab_auth = 0;
                next_state = MCTX_STATUS_LOGIN;
                this->context.mod_state = MOD_STATE_INIT;
            }

            else if (this->context.is_asked(STRAUTHID_PASSWORD)){
                this->context.wab_auth = 1;
                next_state = MCTX_STATUS_LOGIN;
                this->context.mod_state = MOD_STATE_INIT;
            }

            else if (this->context.is_asked(STRAUTHID_DISPLAY_MESSAGE)){
                this->context.mod_state = MOD_STATE_DISPLAY_MESSAGE;
                next_state = MCTX_STATUS_DIALOG;
            }

            else if (this->context.is_asked(STRAUTHID_ACCEPT_MESSAGE)){
                this->context.mod_state = MOD_STATE_VALID_MESSAGE;
                next_state = MCTX_STATUS_DIALOG;
            }

            else if (this->context.get_bool(STRAUTHID_AUTHENTICATED)){

                LOG(LOG_INFO, "NOW WE SHOULD START AN RDP CONNEXION\n");
                next_state = this->get_mod_from_protocol();
                this->context.mod_state = MOD_STATE_CONNECTED_RDP;
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
                this->context.mod_state = MOD_STATE_CONNECTED_RDP;
                #warning check life cycle of auth_trans_t
                if (this->auth_trans_t){
                    delete this->auth_trans_t;
                    this->auth_trans_t = 0;
                }
                next_state = MCTX_STATUS_CLOSE;
            }
        }
        break;
        case MOD_STATE_MESSAGE_CONNEXION_CLOSE_AT_LIMIT:
        LOG(LOG_INFO, "MESSAGE CONNEXION CLOSE AT LIMIT\n");
        {
        }
        break;
        case MOD_STATE_CONNECTED_RDP:
        LOG(LOG_INFO, "CONNECTED RDP\n");
        this->context.mod_state = MOD_STATE_CLOSE;
        next_state = MCTX_STATUS_CLOSE;
        break;
        case MOD_STATE_CLOSE:
        LOG(LOG_INFO, "CLOSE\n");
        {
            this->context.mod_state = MOD_STATE_CLOSE;
            next_state = MCTX_STATUS_EXIT;
        }
        break;
        }
        return next_state;
    }

#warning move that function to ModContext, create specialized stream object ModContextStream
    void out_item(Stream & stream, const char * key)
    {
        const char * tmp = this->context.get(key);
        #warning why shouldn't I send empty values ?
        // do not send empty values
        if (tmp[0] != 0){
            if (strncasecmp(tmp, "ask", 3) != 0
            &&((strncasecmp("password", (char*)key, 8) == 0)
            ||(strncasecmp("target_password", (char*)key, 15) == 0))){
                LOG(LOG_INFO, "sending %s=<hidden>\n", key);
            }
            else {
                LOG(LOG_INFO, "sending %s=%s\n", key, tmp);
            }
            stream.out_copy_bytes(key, strlen(key));
            stream.out_uint8('\n');
            if ('!' == tmp[0]
            || (0 == strncasecmp(tmp, "ask", 3))){
                stream.out_uint8('!');
            }
            stream.out_copy_bytes(tmp, strlen(tmp));
            stream.out_uint8('\n');
        }
    }

    void out_ask_item(Stream & stream, const char * key)
    {
        this->context.ask(key);
        LOG(LOG_INFO, "sending %s=ASK\n", key);
        stream.out_copy_bytes(key, strlen(key));
        stream.out_copy_bytes("\nASK\n",5);
    }

    void add_to_fd_set(fd_set & rfds, unsigned & max)
    {
        #warning look at concept behind wait_obj
        if (this->auth_event){
            this->auth_event->add_to_fd_set(rfds, max);
        }
    }


    int ask_next_module_remote(const char * auth_host, int authport)
    {
        // if anything happen, like authentification socked closing, stop current connection
        try {
            Stream stream(8192);

            stream.skip_uint8(4);
            #warning is there a way to make auth_event RAII ? (initialized in sesman constructor)
            if (!this->auth_trans_t){
                this->auth_trans_t = new SocketTransport(auth_host, authport, 0, 4, 1000000);
                #warning create a realloc method
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
            this->out_ask_item(stream, STRAUTHID_TARGET_PASSWORD);
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
            stream.mark_end();

            stream.p = stream.data;
            int total_length = stream.end - stream.data;
            stream.out_uint32_be(total_length); /* size */
            this->auth_trans_t->send((char*)stream.data, total_length);

        } catch (Error e) {
            this->context.cpy(STRAUTHID_AUTHENTICATED, false);
            this->context.cpy(STRAUTHID_REJECTED, "Authentifier service failed");
            delete this->auth_trans_t;
            this->auth_trans_t = NULL;
        }
        return MCTX_STATUS_WAITING;
    }

    int receive_next_module()
    {
        Stream stream(8192);

        try {
            this->auth_trans_t->recv((char**)&(stream.end), 4);
            int size = stream.in_uint32_be();
            this->auth_trans_t->recv((char**)&(stream.end), size-4);
            this->in_items(stream);
        } catch (...) {
            this->context.cpy(STRAUTHID_AUTHENTICATED, false);
            this->context.cpy(STRAUTHID_REJECTED, "Authentifier service failed");
            delete this->auth_trans_t;
            this->auth_trans_t = NULL;
        }
        return MCTX_STATUS_TRANSITORY;
    }

};

#endif

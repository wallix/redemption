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
   Author(s): Christophe Grosjean, Meng Tan

   Protocol layer for communication with ACL
*/

#ifndef _REDEMPTION_ACL_SERIALIZER_HPP_
#define _REDEMPTION_ACL_SERIALIZER_HPP_
#include <unistd.h>
#include <fcntl.h>
#include <list>


#include "stream.hpp"
#include "config.hpp"
#include "netutils.hpp"
#include "sockettransport.hpp"
#include "wait_obj.hpp"


class AclSerializer{
    enum {
        HEADER_SIZE = 4,
    };
    Inifile * ini;
    Transport & auth_trans;
    uint32_t verbose;

public:
    AclSerializer(Inifile * ini, Transport & auth_trans, uint32_t verbose)
        : ini(ini)
        , auth_trans(auth_trans)
        , verbose(verbose)
    {
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

                    if ((0 == strncasecmp((char*)value, "ask", 3))) {
                        this->ini->context_ask_by_string((char *)keyword);
                        LOG(LOG_INFO, "receiving %s '%s'\n", value, keyword);
                    }
                    else {
                        this->ini->context_set_value_by_string((char *)keyword,
                            (char *)value + (value[0] == '!' ? 1 : 0));

                        if (  (strncasecmp("password",        (char *)keyword, 9 ) == 0)
                           || (strncasecmp("target_password", (char *)keyword, 16) == 0)
                           ){
                            LOG(LOG_INFO, "receiving '%s'=<hidden>\n", (char *)keyword);
                        }
                        else{
                            char buffer[128];
                            LOG(LOG_INFO, "receiving '%s'='%s'\n", keyword,
                                this->ini->context_get_value_by_string((char *)keyword, buffer, sizeof(buffer)));
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
    void incoming()
    {
        BStream stream(HEADER_SIZE);
        this->auth_trans.recv(&stream.end, HEADER_SIZE);
        size_t size = stream.in_uint32_be();
        if (size > 65536){
            LOG(LOG_WARNING, "Error: ACL message too big (got %u max 64 K)", size);
            throw Error(ERR_ACL_MESSAGE_TOO_BIG);
        }
        if (size > stream.capacity){
            stream.init(size);
        }
        this->auth_trans.recv(&stream.end, size);
        LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (receive) = %u", size);
        bool flag = this->ini->context.session_id.is_empty();
        this->in_items(stream);
        if (flag && !this->ini->context.session_id.is_empty()) {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/redemption/session_%d.pid", PID_PATH, child_pid);
            char new_session_file[256];
            sprintf(new_session_file, "%s/redemption/session_%s.pid", PID_PATH,
                this->ini->context.session_id.c_str());
            rename(old_session_file, new_session_file);
        }

        LOG(LOG_INFO, "SESSION_ID = %s", this->ini->context.session_id.c_str());
    }


    TODO("move that function to Inifile create specialized stream object InifileStream")
    TODO("out_item should get an ID not a string key")
    void out_item_by_string(Stream & stream, const char * key)
    {
        if (this->ini->context_is_asked_by_string(key)){
            LOG(LOG_INFO, "sending %s=ASK\n", key);
            stream.out_copy_bytes(key, strlen(key));
            stream.out_copy_bytes("\nASK\n",5);
        }
        else {
            char temp_buffer[256];

            const char * tmp = this->ini->context_get_value_by_string(key, temp_buffer, sizeof(temp_buffer));

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
    
    TODO("maybe out_item should be in config , not here")
    void out_item(Stream & stream, authid_t authid)
    {
        const char * key = string_from_authid(authid);
        if (this->ini->context_is_asked(authid)){
            LOG(LOG_INFO, "sending (from authid) %s=ASK\n", key);
            stream.out_copy_bytes(key, strlen(key));
            stream.out_copy_bytes("\nASK\n",5);
        }
        else {
            char temp_buffer[256];

            const char * tmp = this->ini->context_get_value(authid, temp_buffer, sizeof(temp_buffer));

            if ((strncasecmp("password", (char*)key, 8) == 0)
            ||(strncasecmp("target_password", (char*)key, 15) == 0)){
                LOG(LOG_INFO, "sending (from authid) %s=<hidden>\n", key);
            }
            else {
                LOG(LOG_INFO, "sending (from authid) %s=%s\n", key, tmp);
            }
            stream.out_copy_bytes(key, strlen(key));
            stream.out_uint8('\n');
            stream.out_uint8('!');
            stream.out_copy_bytes(tmp, strlen(tmp));
            stream.out_uint8('\n');
        }
    }
    
    TODO("We should not have any way to send only one value. Change the way it is done by calling code")
    void send(const authid_t authid)
    {
        try {
            BStream stream(8192);
            stream.out_uint32_be(0);
            this->out_item(stream, authid);
            stream.mark_end();
            int total_length = stream.get_offset();
            LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (send) %u", total_length - HEADER_SIZE);
            stream.set_out_uint32_be(total_length - HEADER_SIZE, 0); /* size in header */
            this->auth_trans.send(stream.get_data(), total_length);
        } catch (Error e) {
            this->ini->context.authenticated = false;
            this->ini->context.rejected.copy_c_str("Authentifier service failed");
        }
    }

    typedef struct {
        bool tosend;
        authid_t field_id;
    } authid_to_send_t;

    void send(authid_to_send_t * list, size_t len)
    {
        try {
            BStream stream(8192);
            stream.out_uint32_be(0);

            for (unsigned i = 0; i < len; ++i) {
                if (list[i].tosend){
                    this->out_item(stream, list[i].field_id);
                }
            }
            stream.mark_end();
            int total_length = stream.get_offset();
            LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (send) %u", total_length - HEADER_SIZE);
            stream.set_out_uint32_be(total_length - HEADER_SIZE, 0); /* size in header */
            this->auth_trans.send(stream.get_data(), total_length);
        } catch (Error e) {
            this->ini->context.authenticated = false;
            this->ini->context.rejected.copy_c_str("Authentifier service failed");
        }
    }

    
    void ask_next_module_remote()
    {
        authid_to_send_t tosend[] = {
            {true, AUTHID_PROXY_TYPE},
            {true, AUTHID_DISPLAY_MESSAGE},
            {true, AUTHID_ACCEPT_MESSAGE},
            {true, AUTHID_HOST},
            {true, AUTHID_TARGET},
            {true, AUTHID_AUTH_USER},
            {true, AUTHID_PASSWORD},
            {true, AUTHID_TARGET_USER},
            {true, AUTHID_TARGET_DEVICE},
            {true, AUTHID_TARGET_PROTOCOL},
            {true, AUTHID_SELECTOR},
            {true, AUTHID_SELECTOR_GROUP_FILTER},
            {true, AUTHID_SELECTOR_DEVICE_FILTER},
            {true, AUTHID_SELECTOR_LINES_PER_PAGE},
            {true, AUTHID_SELECTOR_CURRENT_PAGE},
            {true, AUTHID_TARGET_PASSWORD},
            {true, AUTHID_OPT_WIDTH},
            {true, AUTHID_OPT_HEIGHT},
            {true, AUTHID_OPT_BPP},
            {true, AUTHID_REAL_TARGET_DEVICE},
            {this->ini->context_get_value(AUTHID_TRACE_SEAL, NULL, 0)[0], AUTHID_TRACE_SEAL}
        };
        this->send(tosend, sizeof(tosend)/sizeof(tosend[0]));
    }

};

#endif

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
   Updating context dictionnary from incoming acl traffic
*/

#ifndef _REDEMPTION_ACL_SERIALIZER_HPP_
#define _REDEMPTION_ACL_SERIALIZER_HPP_
#include <unistd.h>
#include <fcntl.h>


#include "stream.hpp"
#include "config.hpp"
#include "netutils.hpp"
#include "sockettransport.hpp"

class AclSerializer{

    typedef struct {
        bool tosend;
        authid_t field_id;
    } authid_to_send_t;

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
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::AclSerializer");
        }
    }

    ~AclSerializer()
    {
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::~AclSerializer");
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
        if (size > stream.get_capacity()){
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
            {this->ini->context_has_changed(AUTHID_PROXY_TYPE),                  AUTHID_PROXY_TYPE},
            {this->ini->context_has_changed(AUTHID_DISPLAY_MESSAGE),             AUTHID_DISPLAY_MESSAGE},
            {this->ini->context_has_changed(AUTHID_ACCEPT_MESSAGE),              AUTHID_ACCEPT_MESSAGE},
            {this->ini->context_has_changed(AUTHID_HOST),                        AUTHID_HOST},
            {this->ini->context_has_changed(AUTHID_TARGET),                      AUTHID_TARGET},
            {this->ini->context_has_changed(AUTHID_AUTH_USER),                   AUTHID_AUTH_USER},
            {this->ini->context_has_changed(AUTHID_PASSWORD),                    AUTHID_PASSWORD},
            {this->ini->context_has_changed(AUTHID_TARGET_USER),                 AUTHID_TARGET_USER},
            {this->ini->context_has_changed(AUTHID_TARGET_DEVICE),               AUTHID_TARGET_DEVICE},
            {this->ini->context_has_changed(AUTHID_TARGET_PROTOCOL),             AUTHID_TARGET_PROTOCOL},
            {this->ini->context_has_changed(AUTHID_SELECTOR),                    AUTHID_SELECTOR},
            {this->ini->context_has_changed(AUTHID_SELECTOR_GROUP_FILTER),       AUTHID_SELECTOR_GROUP_FILTER},
            {this->ini->context_has_changed(AUTHID_SELECTOR_DEVICE_FILTER),      AUTHID_SELECTOR_DEVICE_FILTER},
            {this->ini->context_has_changed(AUTHID_SELECTOR_LINES_PER_PAGE),     AUTHID_SELECTOR_LINES_PER_PAGE},
            {this->ini->context_has_changed(AUTHID_SELECTOR_CURRENT_PAGE),       AUTHID_SELECTOR_CURRENT_PAGE},
            {this->ini->context_has_changed(AUTHID_TARGET_PASSWORD),             AUTHID_TARGET_PASSWORD},
            {this->ini->context_has_changed(AUTHID_OPT_WIDTH),                   AUTHID_OPT_WIDTH},
            {this->ini->context_has_changed(AUTHID_OPT_HEIGHT),                  AUTHID_OPT_HEIGHT},
            {this->ini->context_has_changed(AUTHID_OPT_BPP),                     AUTHID_OPT_BPP},
            {this->ini->context_has_changed(AUTHID_REAL_TARGET_DEVICE),          AUTHID_REAL_TARGET_DEVICE},
            {0 == (this->ini->context_get_value(AUTHID_TRACE_SEAL, NULL, 0)[0] != 0), AUTHID_TRACE_SEAL}
        };

        this->send(tosend, sizeof(tosend) / sizeof(tosend[0]));
    }
};

#endif

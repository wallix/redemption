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

#include <forward_list>

#include "stream.hpp"
#include "config.hpp"
#include "authid.hpp"
#include "transport.hpp"
#include "translation.hpp"
#include "get_printable_password.hpp"

class AclSerializer{
    enum {
        HEADER_SIZE = 4
    };

    Inifile & ini;
    Transport & auth_trans;
    uint32_t verbose;

public:
    AclSerializer(Inifile & ini, Transport & auth_trans, uint32_t verbose)
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
        this->auth_trans.disconnect();
        if (this->verbose & 0x10){
            LOG(LOG_INFO, "auth::~AclSerializer");
        }
    }

    struct ArrayItemsView {
        uint8_t * p;
        uint8_t * end;
    };

    void in_items(ArrayItemsView & view)
    {
        if (this->verbose & 0x40){
            LOG(LOG_INFO, "auth::in_items");
        }
        for (; view.p < view.end ; this->in_item(view)){
            ;
        }
    }

    void in_item(ArrayItemsView & view)
    {
        const char * keyword = reinterpret_cast<const char*>(view.p);
        const uint8_t * start = view.p;
        for ( ; view.p < view.end; ++view.p){
            if (*view.p == '\n') {
                *view.p = 0;
                ++view.p;
                break;
            }
        }
        const char * value = reinterpret_cast<const char*>(view.p);
        for ( ; view.p < view.end; ++view.p){
            if (*view.p == '\n') {
                *view.p = 0;

                if (auto field = this->ini.get_acl_field(authid_from_string(keyword))) {
                    if ((0 == strncasecmp(value, "ask", 3))) {
                        field.ask();
                        LOG(LOG_INFO, "receiving ASK '%s'", keyword);
                    }
                    else {
                        // BASE64 TRY
                        // unsigned char output[32000];
                        // if (value[0] == '!') value++;
                        // size_t value_len = strlen((const char*)value);
                        // size_t out_len = this->ini.b64.decode(output, sizeof(output), (const unsigned char *)value, value_len);
                        // output[out_len] = 0;
                        // this->ini.set_from_acl((char *)keyword,
                        //                         (char *)output);
                        field.set(value + (value[0] == '!' ? 1 : 0));
                        const char * val         = field.c_str();
                        const char * display_val = val;
                        if ((strncasecmp("password", keyword, 9 ) == 0) ||
                            (strncasecmp("target_application_password", keyword, 27) == 0) ||
                            (strncasecmp("target_password", keyword, 16) == 0) ||
                            ((strncasecmp("auth_channel_answer", keyword, 19) == 0) && (strcasestr(val, "password") != nullptr))) {
                            display_val = ::get_printable_password(val, this->ini.get<cfg::debug::password>());
                        }
                        LOG(LOG_INFO, "receiving '%s'='%s'", keyword, display_val);
                    }
                }
                else {
                    LOG(LOG_WARNING, "AclSerializer::in_item(stream): unknown strauthid=\"%s\"", keyword);
                }

                view.p = view.p+1;
                return;
            }
        }
        LOG(LOG_WARNING, "Unexpected exit while parsing ACL message");
        hexdump(start, view.p-start);
        throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
    }

    void incoming()
    {
        constexpr std::size_t buf_capacity = HEADER_SIZE < 65536 ? 65536 : HEADER_SIZE;
        uint8_t static_buf[buf_capacity];
        std::unique_ptr<uint8_t[]> dyn_buf;
        uint8_t * buf = static_buf;
        auto end = buf;
        this->auth_trans.recv(&end, HEADER_SIZE);

        size_t size = Parse(buf).in_uint32_be();

        if (size > 1024 * 1024) {
            LOG(LOG_WARNING, "Error: ACL message too big (got %u max 1M)", size);
            throw Error(ERR_ACL_MESSAGE_TOO_BIG);
        }

        if (size > buf_capacity) {
            dyn_buf.reset(new uint8_t[buf_capacity]);
            buf = dyn_buf.get();
        }

        end = buf;
        this->auth_trans.recv(&end, size);

        if (this->verbose & 0x40){
            LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (receive) = %u", size);
        }
        bool flag = this->ini.get<cfg::context::session_id>().empty();
        ArrayItemsView view{buf, buf+size};
        this->in_items(view);
        if (flag && !this->ini.get<cfg::context::session_id>().empty()) {
            int child_pid = getpid();
            char old_session_file[256];
            sprintf(old_session_file, "%s/redemption/session_%d.pid", PID_PATH, child_pid);
            char new_session_file[256];
            sprintf(new_session_file, "%s/redemption/session_%s.pid", PID_PATH,
                    this->ini.get<cfg::context::session_id>().c_str());
            rename(old_session_file, new_session_file);
        }
        if (this->verbose & 0x40){
            LOG(LOG_INFO, "SESSION_ID = %s", this->ini.get<cfg::context::session_id>().c_str());
        }
    }

private:
    struct Buffers {
        static constexpr size_t buf_len = 65535;
        struct Buffer {
            std::array<char, buf_len> data;
            size_t sz = 0;
        };
        std::forward_list<Buffer> buffer_list;
        std::forward_list<Buffer>::iterator before_it = buffer_list.before_begin();
        Buffer buf;
        Buffer * current_buff = &buf;

        Buffers() {
            this->buf.sz = 4u;
        }

        size_t size() const {
            size_t total_length = this->buf.sz;
            for (auto & x : buffer_list) {
                total_length += x.sz;
            }
            return total_length;
        }

        template<class Fn>
        void for_each(Fn fn) const {
            fn(this->buf.data.data(), this->buf.sz);
            for (auto & buf : this->buffer_list) {
                fn(buf.data.data(), buf.sz);
            }
        }

        void push(char c) {
            if (this->current_buff->sz == buf_len) {
                this->extent_buffer();
            }
            this->current_buff->data[this->current_buff->sz++] = c;
        }

        void push(char const * s) {
            while (*s) {
                while (this->current_buff->sz != buf_len && *s) {
                    this->current_buff->data[this->current_buff->sz++] = *s;
                    ++s;
                }
                if (*s) {
                    this->extent_buffer();
                }
            }
        }

        void extent_buffer() {
            this->before_it = this->buffer_list.emplace_after(this->before_it);
            this->current_buff = &*this->before_it;
        }
    };

public:
    void send_acl_data() {
        if (this->verbose & 0x01){
            LOG(LOG_INFO, "Begin Sending data to ACL: numbers of changed fields = %u", this->ini.changed_field_size());
        }
        if (this->ini.changed_field_size()) {
            Buffers buffers;

            auto const password_printing_mode = this->ini.get<cfg::debug::password>();

            this->ini.for_each_changed_field([&](Inifile::FieldReference bfield, authid_t authid){
                char const * key = string_from_authid(authid);
                buffers.push(key);
                buffers.push('\n');
                if (bfield.is_asked()) {
                    buffers.push("ASK\n");
                    LOG(LOG_INFO, "sending %s=ASK", key);
                }
                else {
                    char const * val = bfield.c_str();
                    buffers.push('!');
                    buffers.push(val);
                    buffers.push('\n');
                    const char * display_val = val;
                    if ((strncasecmp("password", key, 8) == 0)
                     || (strncasecmp("target_password", key, 15) == 0)) {
                        display_val = get_printable_password(val, password_printing_mode);
                    }
                    LOG(LOG_INFO, "sending %s=%s", key, display_val);
                }
            });

            size_t const total_length = buffers.size();
            if (this->verbose & 0x40){
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (send) %u", total_length - HEADER_SIZE);
            }

            OutStream(buffers.buf.data.data(), HEADER_SIZE)
              .out_uint32_be(total_length - HEADER_SIZE); /* size in header */

            try {
                buffers.for_each([&](char const * p, size_t len) {
                    this->auth_trans.send(p, len);
                });
            }
            catch (Error const &) {
                this->ini.set_acl<cfg::context::authenticated>(false);
                this->ini.set_acl<cfg::context::rejected>(TR("acl_fail", language(this->ini)));
                // this->ini.context.rejected.set_from_cstr("Authentifier service failed");
            }

            this->ini.clear_send_index();
        }
    }
};

#endif

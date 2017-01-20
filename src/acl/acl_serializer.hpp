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


#pragma once
#include <unistd.h>
#include <fcntl.h>

#include <cinttypes>

#include "utils/sugar/exchange.hpp"
#include "utils/stream.hpp"
#include "configs/config.hpp"
#include "core/authid.hpp"
#include "transport/transport.hpp"
#include "utils/translation.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/verbose_flags.hpp"

class AclSerializer{
    enum {
        HEADER_SIZE = 4
    };

    Inifile & ini;
    Transport & auth_trans;
    char session_id[256];

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        variable = 0x2,
        buffer   = 0x40,
        state    = 0x10,
    };

public:
    AclSerializer(Inifile & ini, Transport & auth_trans, Verbose verbose)
        : ini(ini)
        , auth_trans(auth_trans)
        , session_id{}
        , verbose(verbose)
    {
        std::snprintf(this->session_id, sizeof(this->session_id), "%d", getpid());
        if (this->verbose & Verbose::state){
            LOG(LOG_INFO, "auth::AclSerializer");
        }
    }

    ~AclSerializer()
    {
        this->auth_trans.disconnect();
        if (this->verbose & Verbose::state){
            LOG(LOG_INFO, "auth::~AclSerializer");
        }
        char session_file[256];
        std::snprintf(session_file, sizeof(session_file),
                      "%s/redemption/session_%s.pid", PID_PATH, this->session_id);
        unlink(session_file);
    }

private:
    class Reader
    {
        static constexpr size_t buf_len = 65535;
        char buf[buf_len];
        char key_name_buf[64];
        bool has_next_buffer = true;
        std::string data_multipacket;
        char * p;
        char * e;

        Transport & trans;
        const implicit_bool_flags<Verbose> verbose;

    public:
        Reader(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {
            this->safe_read_packet();
        }

        char const * key(bool always_internal_copy) {
            auto m = std::find(this->p, this->e, '\n');
            if (m == e) {
                size_t key_buf_len = this->e - this->p;
                if (key_buf_len) {
                    if (key_buf_len > sizeof(this->key_name_buf)) {
                        LOG(LOG_ERR, "Error: ACL key length too big (got %zu max 64o)", key_buf_len);
                        throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                    }
                    memcpy(this->key_name_buf, this->p, key_buf_len);
                }
                else if (!this->has_next_buffer) {
                    if (key_buf_len) {
                        throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
                    }
                    return nullptr;
                }
                this->safe_read_packet();
                m = std::find(this->p, this->e, '\n');
                if (key_buf_len) {
                    if (size_t(m - this->p) > sizeof(this->key_name_buf)) {
                        LOG(LOG_ERR, "Error: ACL key length too big (got %" PRIdPTR " max 64o)", m - this->p);
                        throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                    }
                    *m = 0;
                    ++m;
                    memcpy(this->key_name_buf, this->p, m - this->p);
                    this->p = m;
                    return reinterpret_cast<char const *>(this->key_name_buf);
                }
            }
            if (always_internal_copy) {
                *m = 0;
                ++m;
                memcpy(this->key_name_buf, this->p, m - this->p);
                this->p = m;
                return this->key_name_buf;
            }
            *m = 0;
            return exchange(this->p, m+1);
        }

        bool is_set_value() {
            if (this->p == this->e) {
                this->read_packet();
            }
            return *this->p == '!';
        }

        bool consume_ask() {
            char c = this->getc();
            if (!('a' == c || 'A' == c)) {
                return false;
            }
            c = this->getc();
            if (!('s' == c || 'S' == c)) {
                return false;
            }
            c = this->getc();
            if (!('k' == c || 'K' == c)) {
                return false;
            }
            return this->getc() == '\n';
        }

        array_view_const_char get_val() {
            if (this->p == this->e) {
                this->read_packet();
            }
            else if (*this->p == '!') {
                ++this->p;
            }
            auto m = std::find(this->p, this->e, '\n');
            if (m != this->e) {
                *m = 0;
                std::size_t const sz = m - this->p;
                return {exchange(this->p, m+1), sz};
            }
            data_multipacket.clear();
            do {
                data_multipacket.insert(data_multipacket.end(), this->p, this->e);
                if (data_multipacket.size() > 1024*1024) {
                    LOG(LOG_ERR, "Error: ACL data too big (got %zu max 1M)", data_multipacket.size());
                    throw Error(ERR_ACL_MESSAGE_TOO_BIG);
                }
                this->read_packet();
                m = std::find(this->p, this->e, '\n');
            } while (m == e);
            data_multipacket.insert(data_multipacket.end(), this->p, m);
            this->p = m + 1;
            return {data_multipacket.data(), data_multipacket.size()};
        }

        void hexdump() const {
            ::hexdump(this->buf, this->e - this->buf);
        }

    private:
        char getc() {
            if (this->p == this->e) {
                this->read_packet();
            }
            char c = *this->p;
            ++this->p;
            return c;
        }

        void read_packet() {
            if (!this->has_next_buffer) {
                throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
            }
            this->safe_read_packet();
        }

        void safe_read_packet() {
            uint16_t buf_sz = 0;
            do {
                auto end = this->buf;
                this->trans.recv(&end, HEADER_SIZE);

                InStream in_stream(this->buf, 4);
                this->has_next_buffer = in_stream.in_uint16_be();
                buf_sz = in_stream.in_uint16_be();
            } while (buf_sz == 0 && this->has_next_buffer);

            this->p = this->buf;
            this->e = this->buf;
            this->trans.recv(&e, buf_sz);

            if (this->verbose & Verbose::buffer){
                if (this->has_next_buffer){
                    LOG(LOG_INFO, "ACL SERIALIZER : multi buffer (receive)");
                }
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (receive) = %" PRIdPTR, this->e - this->p);
            }
        }
    };

public:
    void in_items()
    {
        Reader reader(this->auth_trans, this->verbose);

        while (auto key = reader.key(this->verbose & Verbose::variable)) {
            auto authid = authid_from_string(key);
            if (auto field = this->ini.get_acl_field(authid)) {
                if (reader.is_set_value()) {
                    if (field.set(reader.get_val()) && (this->verbose & Verbose::variable)) {
                        const char * val         = field.c_str();
                        const char * display_val = val;
                        if (cfg::crypto::key0::index() == authid ||
                            cfg::crypto::key1::index() == authid ||
                            cfg::context::password::index() == authid ||
                            cfg::context::target_password::index() == authid ||
                            cfg::globals::target_application_password::index() == authid ||
                            (cfg::context::auth_channel_answer::index() == authid &&
                             strcasestr(val, "password") != nullptr)
                        ) {
                            display_val = ::get_printable_password(val, this->ini.get<cfg::debug::password>());
                        }
                        LOG(LOG_INFO, "receiving '%s'='%s'", key, display_val);
                    }
                }
                else if (reader.consume_ask()) {
                    field.ask();
                    if (this->verbose & Verbose::variable) {
                        LOG(LOG_INFO, "receiving ASK '%s'", key);
                    }
                }
                else {
                    reader.hexdump();
                    throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
                }
            }
            else {
                LOG(LOG_WARNING, "Unexpected receving '%s'", key);
            }
        }
    }

    void incoming()
    {
        bool flag = this->ini.get<cfg::context::session_id>().empty();
        this->in_items();
        if (flag && !this->ini.get<cfg::context::session_id>().empty()) {
            char old_session_file[256];
            std::snprintf(old_session_file, sizeof(old_session_file),
                          "%s/redemption/session_%s.pid", PID_PATH, this->session_id);
            char new_session_file[256];
            std::snprintf(new_session_file, sizeof(new_session_file),
                         "%s/redemption/session_%s.pid", PID_PATH,
                    this->ini.get<cfg::context::session_id>().c_str());
            std::rename(old_session_file, new_session_file);
            std::snprintf(this->session_id, sizeof(this->session_id), "%s",
                          this->ini.get<cfg::context::session_id>().c_str());
        }
        if (this->verbose & Verbose::buffer){
            LOG(LOG_INFO, "SESSION_ID = %s", this->ini.get<cfg::context::session_id>());
        }
    }

private:
    class Buffers
    {
        static constexpr uint16_t buf_len = 65535;

        struct Buffer
        {
            char data[buf_len];
            uint16_t flags = 0;
            uint16_t sz = HEADER_SIZE; // packet size
        };

        Buffer buf;
        Transport & trans;
        const implicit_bool_flags<Verbose> verbose;

    public:
        Buffers(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {}

        void push(char c) {
            if (this->buf.sz == buf_len) {
                this->new_buffer();
            }
            this->buf.data[this->buf.sz++] = c;
        }

        void push(char const * s) {
            while (*s) {
                while (this->buf.sz != buf_len && *s) {
                    this->buf.data[this->buf.sz++] = *s;
                    ++s;
                }
                if (*s) {
                    this->new_buffer();
                }
            }
        }

        void send_buffer() {
            if (this->verbose & Verbose::buffer){
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (send) %d", this->buf.sz - HEADER_SIZE);
            }
            OutStream stream(this->buf.data, HEADER_SIZE);
            stream.out_uint16_be(this->buf.flags);
            stream.out_uint16_be(this->buf.sz - HEADER_SIZE);
            this->trans.send(this->buf.data, this->buf.sz);
            this->buf.flags = 0u;
            this->buf.sz = HEADER_SIZE;
        }

    private:
        enum { MULTIBUF = 1 };
        void new_buffer() {
            if (this->verbose & Verbose::buffer){
                LOG(LOG_INFO, "ACL SERIALIZER : multi buffer (send)");
            }
            this->buf.flags |= MULTIBUF;
            this->send_buffer();
        }
    };

public:
    void send_acl_data() {
        if (this->verbose & Verbose::variable){
            LOG(LOG_INFO, "Begin Sending data to ACL: numbers of changed fields = %zu", this->ini.changed_field_size());
        }
        if (this->ini.changed_field_size()) {
            auto const password_printing_mode = this->ini.get<cfg::debug::password>();

            try {
                Buffers buffers(this->auth_trans, this->verbose);

                for (auto && field : this->ini.get_fields_changed()) {
                    char const * key = string_from_authid(field.authid());
                    buffers.push(key);
                    buffers.push('\n');
                    if (field.is_asked()) {
                        buffers.push("ASK\n");
                        if (this->verbose & Verbose::variable) {
                            LOG(LOG_INFO, "sending %s=ASK", key);
                        }
                    }
                    else {
                        char const * val = field.c_str();
                        buffers.push('!');
                        buffers.push(val);
                        buffers.push('\n');
                        const char * display_val = val;
                        if ((strncasecmp("password", key, 8) == 0)
                         || (strncasecmp("target_password", key, 15) == 0)) {
                            display_val = get_printable_password(val, password_printing_mode);
                        }
                        if (this->verbose & Verbose::variable) {
                            LOG(LOG_INFO, "sending %s=%s", key, display_val);
                        }
                    }
                }

                buffers.send_buffer();
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

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
  Author(s): Christophe Grosjean, Meng Tauth_rail_exec_an, Jennifer Inthavong

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/

#include "acl/acl_serializer.hpp"
#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/log_id.hpp"
#include "main/version.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "utils/fileutils.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/log.hpp"
#include "utils/log_siem.hpp"
#include "utils/stream.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/hexdump.hpp"

#include <string>
#include <algorithm>

#include <ctime>
#include <cstdio>
#include <cassert>

#include "acl/session_logfile.hpp"


namespace
{
    enum {
        ACL_SERIALIZER_HEADER_SIZE = 4
    };
}


AclSerializer::AclSerializer(Inifile & ini)
: ini(ini)
, auth_trans(nullptr)
, session_id{}
, remote_answer(false)
, verbose(safe_cast<Verbose>(ini.get<cfg::debug::auth>()))
{
    std::snprintf(this->session_id, sizeof(this->session_id), "%d", getpid());
}

AclSerializer::~AclSerializer() = default;

namespace
{
    class Reader
    {
        using Verbose = AclSerializer::Verbose;

        static constexpr size_t buf_len = 65535;
        char buf[buf_len];
        bool has_next_buffer = true;
        std::string data_multipacket;
        char * p;
        char * e;

        Transport & trans;
        const Verbose verbose;

    public:
        Reader(Transport & trans, Verbose verbose)
        : p(nullptr)
        , e(nullptr)
        , trans(trans)
        , verbose(verbose)
        {
            this->safe_read_packet();
        }

        zstring_view key()
        {
            auto m = std::find(this->p, this->e, '\n');
            if (m == this->e) {
                if (this->e - this->p) {
                    LOG(LOG_ERR, "ACL SERIALIZER: key is truncated");
                    throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
                }
                if (!this->has_next_buffer) {
                    return nullptr;
                }
                this->safe_read_packet();
                m = std::find(this->p, this->e, '\n');
            }

            *m = 0;
            std::size_t const len = m - this->p;
            auto* start_s = std::exchange(this->p, m+1);
            return zstring_view(zstring_view::is_zero_terminated(), start_s, len);
        }

        bool is_set_value()
        {
            if (this->p == this->e) {
                this->read_packet();
            }
            return *this->p == '!';
        }

        bool consume_ask()
        {
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

        zstring_view get_val()
        {
            if (this->p == this->e) {
                this->read_packet();
            }
            else if (*this->p == '!') {
                ++this->p;
            }
            auto m = std::find(this->p, this->e, '\n');
            if (m != this->e) {
                *m = 0;
                std::size_t const len = m - this->p;
                auto* start_s = std::exchange(this->p, m+1);
                return zstring_view(zstring_view::is_zero_terminated(), start_s, len);
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
            return data_multipacket;
        }

        void hexdump() const
        {
            ::hexdump(this->buf, this->e - this->buf);
        }

    private:
        char getc()
        {
            if (this->p == this->e) {
                this->read_packet();
            }
            char c = *this->p;
            ++this->p;
            return c;
        }

        void read_packet()
        {
            if (!this->has_next_buffer) {
                LOG(LOG_ERR, "Error: ERR_ACL_UNEXPECTED_IN_ITEM_OUT (2)");
                throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
            }
            this->safe_read_packet();
        }

        void safe_read_packet()
        {
            uint16_t buf_sz = 0;
            do {
                InStream in_stream(this->trans.recv_boom(this->buf, ACL_SERIALIZER_HEADER_SIZE));
                this->has_next_buffer = in_stream.in_uint16_be();
                buf_sz = in_stream.in_uint16_be();
            } while (buf_sz == 0 && this->has_next_buffer);

            this->p = this->buf;
            this->e = this->buf;
            this->trans.recv_boom(e, buf_sz);
            e += buf_sz;

            if (bool(this->verbose & Verbose::buffer)) {
                LOG_IF(this->has_next_buffer,
                    LOG_INFO, "ACL SERIALIZER : multi buffer (receive)");
                LOG(LOG_INFO, "ACL SERIALIZER : Data size (receive) = %" PRIdPTR "(without header)", this->e - this->p);
            }
        }
    };

    chars_view get_loggable_value(
        zstring_view value,
        Inifile::LoggableCategory loggable_category,
        uint32_t printing_mode)
    {
        switch (loggable_category) {
            case Inifile::LoggableCategory::Loggable:
                return value;

            case Inifile::LoggableCategory::LoggableButWithPassword:
                if (nullptr == strcasestr(value.c_str(), "password")) {
                    return value;
                }
                [[fallthrough]];

            case Inifile::LoggableCategory::Unloggable:
                return ::get_printable_password(value, printing_mode);
        }

        REDEMPTION_UNREACHABLE();
    }

} // anonymous namespace

void AclSerializer::in_items()
{
    Reader reader(*this->auth_trans, this->verbose);
    zstring_view key;

    while (!(key = reader.key()).empty()) {
        if (auto field = this->ini.get_acl_field_by_name(key)) {
            if (reader.is_set_value()) {
                if (field.set(reader.get_val()) && bool(this->verbose & Verbose::variable)) {
                    Inifile::ZStringBuffer zstring_buffer;
                    zstring_view val = field.to_zstring_view(zstring_buffer);

                    chars_view display_val = get_loggable_value(
                        val, field.loggable_category(), this->ini.get<cfg::debug::password>());

                    LOG_IF(bool(this->verbose & Verbose::variable), LOG_INFO,
                        "receiving '%.*s'='%.*s'",
                        int(key.size()), key.data(),
                        int(display_val.size()), display_val.data());

                    // TODO: big hack, generalize this to a set of callbacks to call
                    if (field.authid() == cfg::globals::inactivity_timeout::index){
                        this->on_inactivity_timeout();
                    }
                }
            }
            else if (reader.consume_ask()) {
                field.ask();
                LOG_IF(bool(this->verbose & Verbose::variable), LOG_INFO,
                    "receiving ASK '%*s'", int(key.size()), key.data());
                // callback if the key is listened to for asks
            }
            else {
                reader.hexdump();
                LOG(LOG_ERR, "Error: ERR_ACL_UNEXPECTED_IN_ITEM_OUT (3)");
                throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
            }
        }
        else {
            char sauthid[128];
            std::size_t const min = std::min(std::size(sauthid)-1, key.size());
            memcpy(sauthid, key.data(), min);
            sauthid[min] = 0;
            // this invalidate key value
            auto val = reader.get_val();
            LOG(LOG_WARNING, "Unexpected receive '%s' - '%.*s'",
                sauthid, int(val.size()), val.data());
        }
    }
}

void AclSerializer::incoming()
{
    bool flag = this->ini.get<cfg::context::session_id>().empty();
    this->in_items();
    if (flag && !this->ini.get<cfg::context::session_id>().empty()) {
        char old_session_file[2048];
        std::snprintf(old_session_file, sizeof(old_session_file), "%s/session_%s.pid",
                      app_path(AppPath::LockDir).c_str(), this->session_id);
        char new_session_file[2048];
        std::snprintf(new_session_file, sizeof(new_session_file), "%s/session_%s.pid",
                      app_path(AppPath::LockDir).c_str(),
                      this->ini.get<cfg::context::session_id>().c_str());
        std::rename(old_session_file, new_session_file);
        std::snprintf(this->session_id, sizeof(this->session_id), "%s",
                      this->ini.get<cfg::context::session_id>().c_str());
    }
    LOG_IF(bool(this->verbose & Verbose::buffer),
        LOG_INFO, "SESSION_ID = %s", this->ini.get<cfg::context::session_id>());
}

namespace
{
    class Buffers
    {
        using Verbose = AclSerializer::Verbose;

        static constexpr uint16_t buf_len = 65535;

        struct Buffer
        {
            char data[buf_len];
            uint16_t flags = 0;
            uint16_t sz = ACL_SERIALIZER_HEADER_SIZE; // packet size
        };

        Buffer buf;
        Transport & trans;
        const Verbose verbose;

    public:
        Buffers(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {}

        void push(char c)
        {
            if (this->buf.sz == buf_len) {
                this->new_buffer();
            }
            this->buf.data[this->buf.sz++] = c;
        }

        void push(chars_view av)
        {
            do {
                auto n = std::min<std::size_t>(av.size(), this->buf_len - this->buf.sz);
                memcpy(this->buf.data + this->buf.sz, av.data(), n);
                this->buf.sz += n;
                av = av.from_offset(n);
                if (!av.empty()) {
                    this->new_buffer();
                }
            } while(!av.empty());
        }

        void send_buffer()
        {
            LOG_IF(bool(this->verbose & Verbose::buffer),
                LOG_INFO, "ACL SERIALIZER : Data size without header (send) %d",
                this->buf.sz - ACL_SERIALIZER_HEADER_SIZE);
            OutStream stream({this->buf.data, ACL_SERIALIZER_HEADER_SIZE});
            stream.out_uint16_be(this->buf.flags);
            stream.out_uint16_be(this->buf.sz - ACL_SERIALIZER_HEADER_SIZE);
            this->trans.send(this->buf.data, this->buf.sz);
            this->buf.flags = 0u;
            this->buf.sz = ACL_SERIALIZER_HEADER_SIZE;
        }

    private:
        void new_buffer()
        {
            enum { MULTIBUF = 1 };
            LOG_IF(bool(this->verbose & Verbose::buffer),
                LOG_INFO, "ACL SERIALIZER : multi buffer (send)");
            this->buf.flags |= MULTIBUF;
            this->send_buffer();
        }
    };
} // anonymous namespace

void AclSerializer::send_acl_data()
{
    LOG_IF(bool(this->verbose & Verbose::variable),
        LOG_INFO, "Begin Sending data to ACL: numbers of changed fields = %zu",
        this->ini.changed_field_size());

    if (this->ini.changed_field_size()) {
        auto const password_printing_mode = this->ini.get<cfg::debug::password>();

        try {
            Buffers buffers(*this->auth_trans, this->verbose);

            for (auto field : this->ini.get_fields_changed()) {
                zstring_view key = field.get_acl_name();
                buffers.push(key);
                buffers.push('\n');
                if (field.is_asked()) {
                    buffers.push("ASK\n"_av);
                    LOG_IF(bool(this->verbose & Verbose::variable),
                        LOG_INFO, "sending %s=ASK", key);
                }
                else {
                    Inifile::ZStringBuffer zstring_buffer;
                    auto val = field.to_zstring_view(zstring_buffer);
                    buffers.push('!');
                    buffers.push(val);
                    buffers.push('\n');

                    chars_view display_val = get_loggable_value(
                        val, field.loggable_category(), password_printing_mode);
                    LOG_IF(bool(this->verbose & Verbose::variable),
                        LOG_INFO, "sending %s=%.*s", key,
                        int(display_val.size()), display_val.data());
                }
            }

            buffers.send_buffer();
        }
        catch (Error const & e) {
            LOG(LOG_ERR, "ACL SERIALIZER : %s", e.errmsg());
            this->ini.set_acl<cfg::context::authenticated>(false);
            this->ini.set_acl<cfg::context::rejected>(TR(trkeys::acl_fail, language(this->ini)));
        }

        this->ini.clear_send_index();
    }
}


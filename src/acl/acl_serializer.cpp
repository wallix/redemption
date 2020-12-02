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
#include "core/error.hpp"
#include "transport/transport.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/hexdump.hpp"

#include <memory>

#include <ctime>
#include <cstdio>
#include <cassert>


namespace
{
    enum {
        ACL_SERIALIZER_HEADER_SIZE = 4
    };
}


AclSerializer::AclSerializer(Inifile & ini, Transport & trans)
: verbose(safe_cast<Verbose>(ini.get<cfg::debug::auth>()))
, ini(ini)
, auth_trans(trans)
, session_id{}
{
    std::snprintf(this->session_id, sizeof(this->session_id), "%d", getpid());
}

AclSerializer::~AclSerializer() = default;

namespace
{
    class Reader
    {
        using Verbose = AclSerializer::Verbose;

        static constexpr std::size_t buf_len = 65536;

        Transport & trans;
        const Verbose verbose;

        int32_t nfield;
        uint8_t type;
        InStream in_stream{{buf, 0}};
        std::unique_ptr<uint8_t[]> dynamicbuf;
        uint8_t buf[buf_len];

    public:
        Reader(Transport & trans, Verbose verbose)
        : trans(trans)
        , verbose(verbose)
        {
        }

        void read_nfield()
        {
            this->read(4);
            this->nfield = safe_int{this->in_stream.in_uint16_be()};
        }

        bool has_field() const
        {
            return this->nfield;
        }

        bool has_data() const
        {
            return this->in_stream.in_remain();
        }

        chars_view read_key()
        {
            --this->nfield;

            this->read(2);
            this->type = this->in_stream.in_uint8();
            auto const name_len = this->in_stream.in_uint8();

            this->read(name_len);
            return this->in_stream.in_skip_bytes(name_len).as_chars();
        }

        bool is_ask() const
        {
            assert(this->type == '?' || this->type == '!');
            return this->type == '?';
        }

        bytes_view read_value(zstring_view key)
        {
            this->read(4);
            auto const value_len = this->in_stream.in_uint32_be();
            if (this->in_stream.in_check_rem(value_len)) {
                return this->in_stream.in_skip_bytes(value_len);
            }
            else if (value_len < 256u) {
                this->read(value_len);
                return this->in_stream.in_skip_bytes(value_len);
            }
            else if (value_len > 1024u*1024u) {
                LOG(LOG_ERR, "Error: ACL data too big (got %u max 1MiB) with %s", value_len, key);
                throw Error(ERR_ACL_MESSAGE_TOO_BIG);
            }
            else {
                this->dynamicbuf.reset(new uint8_t[value_len]); /* NOLINT C++20 */
                auto* p = this->dynamicbuf.get();
                auto remaining = this->in_stream.in_remain();
                this->in_stream.in_copy_bytes(p, remaining);
                p += remaining;
                remaining = value_len - remaining;

                do {
                    const std::size_t n = this->read_to({p, remaining});
                    p += n;
                    remaining -= n;
                } while (remaining);

                return {this->dynamicbuf.get(), p};
            }
        }

    private:
        void read(std::size_t minimal_requierment)
        {
            std::size_t remaining = this->in_stream.in_remain();
            if (REDEMPTION_UNLIKELY(remaining < minimal_requierment)) {
                std::memmove(buf, this->in_stream.get_current(), remaining);
                do {
                    auto av = writable_buffer_view{buf};
                    remaining += this->read_to(av.from_offset(remaining));
                } while (REDEMPTION_UNLIKELY(remaining < minimal_requierment));

                this->in_stream = InStream({buf, remaining});
            }
        }

        std::size_t read_to(writable_bytes_view av)
        {
            const std::size_t n = this->trans.partial_read(av);
            if (!n) {
                LOG(LOG_ERR, "Error: ERR_ACL_UNEXPECTED_IN_ITEM_OUT (2)");
                throw Error(ERR_ACL_UNEXPECTED_IN_ITEM_OUT);
            }
            else if (bool(this->verbose)) {
                LOG_IF(bool(this->verbose & Verbose::buffer), LOG_INFO,
                    "ACL SERIALIZER : Data size (receive) = %zu", n);
                if (bool(this->verbose & Verbose::dump)) {
                    hexdump(av.first(n));
                }
            }
            return n;
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

    using Verbose = AclSerializer::Verbose;

    AclFieldMask read_acl_fields(Inifile& ini, Transport& auth_trans, Verbose verbose)
    {
        Reader reader(auth_trans, verbose);

        AclFieldMask flags {};

        do {
            reader.read_nfield();
            while (reader.has_field()) {
                auto key = reader.read_key();
                if (auto field = ini.get_acl_field_by_name(key)) {
                    if (reader.is_ask()) {
                        field.ask();
                        LOG_IF(bool(verbose & Verbose::variable), LOG_INFO,
                            "receiving ASK '%*s'", int(key.size()), key.data());
                        // callback if the key is listened to for asks
                    }
                    else {
                        auto zkey = field.get_acl_name();
                        if (bool is_ok = field.set(reader.read_value(zkey))) {
                            flags.set(field.authid());
                            if (bool(verbose & Verbose::variable)) {
                                Inifile::ZStringBuffer zstring_buffer;
                                zstring_view val = field.to_zstring_view(zstring_buffer);

                                chars_view display_val = get_loggable_value(
                                    val, field.loggable_category(), ini.get<cfg::debug::password>());

                                LOG(LOG_INFO, "receiving '%s'='%.*s'",
                                    zkey, int(display_val.size()), display_val.data());
                            }
                        }
                    }
                }
                else {
                    char sauthid[256];
                    std::size_t const min = std::min(std::size(sauthid)-1, key.size());
                    memcpy(sauthid, key.data(), min);
                    sauthid[min] = 0;
                    // this invalidate key value
                    auto val = reader.read_value("<unknown>"_zv);
                    LOG(LOG_WARNING, "Unexpected receive '%s' - '%.*s'",
                        sauthid, int(val.size()), val.data());
                }
            }
        } while (reader.has_data());

        return flags;
    }

} // anonymous namespace

AclFieldMask AclSerializer::incoming()
{
    return read_acl_fields(this->ini, this->auth_trans, this->verbose);
}

namespace
{
    class Writer
    {
        using Verbose = AclSerializer::Verbose;

        static constexpr std::size_t buf_len = 65536;

        Transport & trans;
        const Verbose verbose;
        StaticOutStream<buf_len> out_stream;

    public:
        Writer(Transport & trans, Verbose verbose, uint32_t n)
        : trans(trans)
        , verbose(verbose)
        {
            out_stream.out_uint16_be(n);
        }

        void send_buffer()
        {
            this->send_data(this->out_stream.get_produced_bytes());
        }

        void push_ask(zstring_view name)
        {
            assert(name.size() <= 255);

            if (!this->out_stream.has_room(2u + name.size())) {
                this->send_multi_buffer();
            }
            this->out_stream.out_uint8('?');
            this->out_stream.out_uint8(checked_int{name.size()});
            this->out_stream.out_copy_bytes(name);
        }

        void push_value(zstring_view name, bytes_view value)
        {
            assert(name.size() <= 255);

            if (!this->out_stream.has_room(2u + name.size() + 4u + value.size())) {
                this->send_multi_buffer();
            }
            this->out_stream.out_uint8('!');
            this->out_stream.out_uint8(checked_int{name.size()});
            this->out_stream.out_copy_bytes(name);
            this->out_stream.out_uint32_be(checked_int{value.size()});

            if (this->out_stream.has_room(value.size())) {
                this->out_stream.out_copy_bytes(value);
            }
            else {
                const auto min = std::min(this->out_stream.tailroom(), value.size());
                this->out_stream.out_copy_bytes(value.first(min));
                this->send_multi_buffer();
                this->send_data(value.from_offset(min));
            }
        }

    private:
        void send_multi_buffer()
        {
            LOG_IF(bool(this->verbose & Verbose::buffer),
                LOG_INFO, "ACL SERIALIZER : multi buffer (send)");
            this->send_buffer();
            this->out_stream.rewind();
        }

        void send_data(bytes_view data)
        {
            LOG_IF(bool(this->verbose & Verbose::buffer),
                LOG_INFO, "ACL SERIALIZER : Data size (send) %zu", data.size());
            if (bool(this->verbose & Verbose::dump)) {
                hexdump(data);
            }
            this->trans.send(data);
        }
    };
} // anonymous namespace

std::size_t AclSerializer::send_acl_data()
{
    const auto fields = this->ini.get_acl_fields_changed();
    const auto nfield = fields.size();

    if (REDEMPTION_UNLIKELY(nfield)) {
        const bool enable_verbose = bool(this->verbose & Verbose::variable);

        LOG_IF(enable_verbose,
            LOG_INFO, "Sending data to ACL: numbers of changed fields = %zu", nfield);

        const auto password_printing_mode = this->ini.get<cfg::debug::password>();

        Writer writer(this->auth_trans, this->verbose, checked_int{nfield});

        for (auto field : fields) {
            const zstring_view key = field.get_acl_name();
            if (field.is_asked()) {
                writer.push_ask(key);
                LOG_IF(enable_verbose, LOG_INFO, "sending %s=ASK", key);
            }
            else {
                Inifile::ZStringBuffer zstring_buffer;
                const auto val = field.to_zstring_view(zstring_buffer);
                writer.push_value(key, val);

                if (enable_verbose) {
                    const chars_view display_val = get_loggable_value(
                        val, field.loggable_category(), password_printing_mode);
                    LOG(LOG_INFO, "sending %s=%.*s", key,
                        int(display_val.size()), display_val.data());
                }
            }
        }

        writer.send_buffer();

        this->ini.clear_acl_fields_changed();
    }

    return nfield;
}

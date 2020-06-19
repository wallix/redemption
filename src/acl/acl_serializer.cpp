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
#include "core/report_message_api.hpp"
#include "main/version.hpp"
#include "mod/rdp/rdp_api.hpp"
#include "std17/charconv.hpp"
#include "utils/fileutils.hpp"
#include "utils/get_printable_password.hpp"
#include "utils/log.hpp"
#include "utils/log_siem.hpp"
#include "utils/stream.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "core/set_server_redirection_target.hpp"

#include <string>
#include <algorithm>

#include <ctime>
#include <cstdio>
#include <cassert>


KeepAlive::KeepAlive(std::chrono::seconds grace_delay_, Verbose verbose)
: grace_delay(grace_delay_.count())
, timeout(0)
, renew_time(0)
, wait_answer(false)
, connected(false)
, verbose(verbose)
{
    LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "KEEP ALIVE CONSTRUCTOR");
}

KeepAlive::~KeepAlive()
{
    LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "KEEP ALIVE DESTRUCTOR");
}

bool KeepAlive::is_started()
{
    return this->connected;
}

void KeepAlive::start(time_t now)
{
    this->connected = true;
    LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "auth::start_keep_alive");
    this->timeout    = now + 2 * this->grace_delay;
    this->renew_time = now + this->grace_delay;
}

void KeepAlive::stop()
{
    this->connected = false;
}

bool KeepAlive::check(time_t now, Inifile & ini)
{
    if (this->connected) {
        // LOG(LOG_INFO, "now=%u timeout=%u  renew_time=%u wait_answer=%s grace_delay=%u", now, this->timeout, this->renew_time, this->wait_answer?"Y":"N", this->grace_delay);
        // Keep alive timeout
        if (now > this->timeout) {
            LOG(LOG_INFO, "auth::keep_alive_or_inactivity Connection closed by manager (timeout)");
            return true;
        }

        // LOG(LOG_INFO, "keepalive state ask=%s bool=%s",
        //     ini.is_asked<cfg::context::keepalive>()?"Y":"N",
        //     ini.get<cfg::context::keepalive>()?"Y":"N");

        // Keepalive received positive response
        if (this->wait_answer
            && !ini.is_asked<cfg::context::keepalive>()
            && ini.get<cfg::context::keepalive>()) {
            LOG_IF(bool(this->verbose & Verbose::state),
                LOG_INFO, "auth::keep_alive ACL incoming event");
            this->timeout    = now + 2*this->grace_delay;
            this->renew_time = now + this->grace_delay;
            this->wait_answer = false;
        }

        // Keep alive asking for an answer from ACL
        if (!this->wait_answer
            && (now > this->renew_time)) {

            this->wait_answer = true;

            ini.ask<cfg::context::keepalive>();
        }
    }
    return false;
}


Inactivity::Inactivity(std::chrono::seconds timeout, time_t start, Verbose verbose)
: inactivity_timeout(std::max<time_t>(timeout.count(), 30))
, last_activity_time(start)
, verbose(verbose)
{
    LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "INACTIVITY CONSTRUCTOR");
}

Inactivity::~Inactivity()
{
//    this->disconnect();
    LOG_IF(bool(this->verbose & Verbose::state), LOG_INFO, "INACTIVITY DESTRUCTOR");
}

bool Inactivity::check_user_activity(time_t now, bool & has_user_activity)
{
    if (!has_user_activity) {
        if (now > this->last_activity_time + this->inactivity_timeout) {
            this->last_activity_time = now;
            LOG(LOG_INFO, "Session User inactivity : closing");
            return true;
        }
    }
    else {
        has_user_activity = false;
    }
    return false;
}

void Inactivity::update_inactivity_timeout(time_t inactivity_timeout)
{
    this->inactivity_timeout = inactivity_timeout;
}

time_t Inactivity::get_inactivity_timeout()
{
    return this->inactivity_timeout;
}


SessionLogFile::SessionLogFile(CryptoContext & cctx, Random & rnd, Fstat & fstat, ReportError report_error)
: ct(cctx, rnd, fstat, std::move(report_error))
{}

SessionLogFile::~SessionLogFile()
{
    try {
        this->close();
    }
    catch (Error const& e) {
        LOG(LOG_ERR, "~SessionLogFile: %s", e.errmsg());
    }
}

void SessionLogFile::open(
    std::string const& log_path, std::string const& hash_path,
    int groupid, bytes_view derivator)
{
    assert(!this->ct.is_open());
    this->ct.open(log_path.c_str(), hash_path.c_str(), groupid, -1, derivator);
    // force to create the file
    this->ct.send("", 0);
}

void SessionLogFile::close()
{
    if (this->ct.is_open()) {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];
        this->ct.close(qhash, fhash);
    }
}

void SessionLogFile::write_line(std::time_t time, chars_view av)
{
    assert(this->ct.is_open());

    char mbstr[100];
    auto const len = std::strftime(mbstr, sizeof(mbstr), "%F %T ", std::localtime(&time));
    if (len) {
        this->ct.send(mbstr, len);
    }

    this->ct.send(av.data(), av.size());
    this->ct.send("\n", 1);
}

namespace
{
    enum {
        ACL_SERIALIZER_HEADER_SIZE = 4
    };
}


AclSerializer::AclSerializer(Inifile & ini, TimeBase & timebase)
: ini(ini)
, timebase(timebase)
, session_id{}
, remote_answer(false)
, verbose(to_verbose_flags(ini.get<cfg::debug::auth>()))
{
    std::snprintf(this->session_id, sizeof(this->session_id), "%d", getpid());
}

AclSerializer::~AclSerializer()
{
}

void AclSerializer::report(const char * reason, const char * message)
{
    this->ini.ask<cfg::context::keepalive>();
    char report[1024];
    snprintf(report, sizeof(report), "%s:%s:%s", reason,
        this->ini.get<cfg::globals::target_device>().c_str(), message);
    this->ini.set_acl<cfg::context::reporting>(report);
    this->send_acl_data();
}

void AclSerializer::server_redirection_target() {
    set_server_redirection_target(this->ini, *this);
}

namespace
{
    template<std::size_t N>
    struct StringBuf
    {
        [[nodiscard]] std::string_view sv() const noexcept
        {
            return {buf, len};
        }

        std::size_t capacity() noexcept
        {
            return N;
        }

        char* data() noexcept
        {
            return buf;
        }

        void setsize(std::size_t n) noexcept
        {
            len = n;
        }

    private:
        char buf[N];
        std::size_t len;
    };

    StringBuf<64> from_gmtime(std::time_t time) noexcept
    {
        StringBuf<64> buf;

        struct tm t;
        gmtime_r(&time, &t);

        constexpr char const* months[]{
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
        };

        // MMM(text) dd yyyy hh:mm:ss
        int len = snprintf(buf.data(), buf.capacity(), "%s %02d %04d %02d:%02d:%02d",
            months[t.tm_mon], t.tm_mday, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
        assert(len > 0);
        buf.setsize(len);

        return buf;
    }

    constexpr inline chars_view ints_s[]{
        "0"_av, "1"_av, "2"_av, "3"_av, "4"_av, "5"_av, "6"_av, "7"_av, "8"_av, "9"_av,
        "10"_av, "11"_av, "12"_av, "13"_av, "14"_av, "15"_av, "16"_av, "17"_av, "18"_av,
        "19"_av, "20"_av, "21"_av, "22"_av, "23"_av, "24"_av, "25"_av, "26"_av, "27"_av,
        "28"_av, "29"_av, "30"_av, "31"_av, "32"_av, "33"_av, "34"_av, "35"_av, "36"_av,
        "37"_av, "38"_av, "39"_av, "40"_av, "41"_av, "42"_av, "43"_av, "44"_av, "45"_av,
        "46"_av, "47"_av, "48"_av, "49"_av, "50"_av, "51"_av, "52"_av, "53"_av, "54"_av,
        "55"_av, "56"_av, "57"_av, "58"_av, "59"_av, "60"_av, "61"_av, "62"_av, "63"_av,
        "64"_av, "65"_av, "66"_av, "67"_av, "68"_av, "69"_av, "70"_av, "71"_av, "72"_av,
        "73"_av, "74"_av, "75"_av, "76"_av, "77"_av, "78"_av, "79"_av, "80"_av, "81"_av,
        "82"_av, "83"_av, "84"_av, "85"_av, "86"_av, "87"_av, "88"_av, "89"_av, "90"_av,
        "91"_av, "92"_av, "93"_av, "94"_av, "95"_av, "96"_av, "97"_av, "98"_av, "99"_av,
        "100"_av, "101"_av, "102"_av, "103"_av, "104"_av, "105"_av, "106"_av, "107"_av,
        "108"_av, "109"_av, "110"_av, "111"_av, "112"_av, "113"_av, "114"_av, "115"_av,
        "116"_av, "117"_av, "118"_av, "119"_av, "120"_av,
    };

    namespace table_formats
    {
        constexpr auto arcsight()
        {
            std::array<char, 256> t{};
            t[int('=')] = '=';
            t[int('\\')] = '\\';
            t[int('\n')] = 'n';
            t[int('\r')] = 'r';
            return t;
        }

        constexpr inline auto arcsight_table = arcsight();
        constexpr inline auto& siem_table = qvalue_table_formats::log_table;
    }

    inline void log_format_set_siem(
        std::string& buffer,
        chars_view session_type,
        chars_view user,
        chars_view account,
        chars_view session_id,
        chars_view host,
        chars_view target_ip,
        chars_view device,
        chars_view service)
    {
        buffer.clear();
        auto append = [&](auto* key, chars_view value){
            buffer += key;
            escaped_qvalue(buffer, value, table_formats::siem_table);
            buffer += "\" ";
        };

        if (session_type.empty()) {
            buffer += "[Neutral Session] ";
        }
        else {
            buffer += '[';
            buffer.append(session_type.data(), session_type.size());
            buffer += " Session] ";
        }
        append("session_id=\"", session_id);
        append("client_ip=\"",  host);
        append("target_ip=\"",  target_ip);
        append("user=\"",       user);
        append("device=\"",     device);
        append("service=\"",    service);
        append("account=\"",    account);
    }

    inline void log_format_set_arcsight(
        std::string& buffer,
        LogId id,
        std::time_t time,
        chars_view session_type,
        chars_view user,
        chars_view account,
        chars_view session_id,
        chars_view host,
        chars_view target_ip,
        chars_view device,
        chars_view service,
        KVList kv_list)
    {
        static_assert(std::size(ints_s) >= std::size(detail::log_id_string_map));
        buffer.clear();
        str_append(buffer,
            from_gmtime(time).sv(),
            " host message CEF:1|Wallix|Bastion|" VERSION "|",
            ints_s[unsigned(id)], '|',
            detail::log_id_string_map[unsigned(id)], "|"
            "5" /*TODO severity*/
            "|WallixBastionSessionType=", session_type.empty() ? "Neutral"_av : session_type,
            " WallixBastionSessionId=", session_id,
            " WallixBastionHost=", host,
            " WallixBastionTargetIP=", target_ip,
            " WallixBastionUser=", user,
            " WallixBastionDevice=", device,
            " WallixBastionService=", service,
            " WallixBastionAccount=", account
        );
        kv_list_to_string(buffer, kv_list, '=', "", table_formats::arcsight_table);
    }
}

void AclSerializer::log6(LogId id, KVList kv_list)
{
    const timeval time = this->timebase.get_current_time();
    std::string buffer_info;
    buffer_info.reserve(kv_list.size() * 50 + 30);

    time_t const time_now = time.tv_sec;
    this->log_file->write_line(time_now, log_format_set_info(buffer_info, id, kv_list));

    auto target_ip = [this]{
        char c = this->ini.get<cfg::context::target_host>()[0];
        using av = chars_view;
        return ('0' <= c && '9' <= c)
            ? av(this->ini.get<cfg::context::target_host>())
            : av(this->ini.get<cfg::context::ip_target>());
    };

    /* Log to SIEM (redirected syslog) */
    if (this->ini.get<cfg::session_log::enable_session_log>()) {
        std::string buffer;
        log_format_set_siem(
            buffer,
            this->session_type,
            this->ini.get<cfg::globals::auth_user>(),
            this->ini.get<cfg::globals::target_user>(),
            this->ini.get<cfg::context::session_id>(),
            this->ini.get<cfg::globals::host>(),
            target_ip(),
            this->ini.get<cfg::globals::target_device>(),
            this->ini.get<cfg::context::target_service>());

        LOG_SIEM("%s%s", buffer.c_str(), buffer_info.c_str());
    }

    if (this->ini.get<cfg::session_log::enable_arcsight_log>()) {
        log_format_set_arcsight(
            buffer_info, id, time_now,
            this->session_type,
            this->ini.get<cfg::globals::auth_user>(),
            this->ini.get<cfg::globals::target_user>(),
            this->ini.get<cfg::context::session_id>(),
            this->ini.get<cfg::globals::host>(),
            target_ip(),
            this->ini.get<cfg::globals::target_device>(),
            this->ini.get<cfg::context::target_service>(),
            kv_list);

        LOG_SIEM("%s", buffer_info);
    }
}

void AclSerializer::start_session_log()
{
    const int groupid = ini.get<cfg::video::capture_groupid>();
    auto const& subdir = this->ini.get<cfg::capture::record_subdirectory>();
    auto const& record_dir = this->ini.get<cfg::video::record_path>();
    auto const& hash_dir = this->ini.get<cfg::video::hash_path>();
    auto const& filebase = this->ini.get<cfg::capture::record_filebase>();

    std::string record_path = str_concat(record_dir.as_string(), subdir, '/');
    std::string hash_path = str_concat(hash_dir.as_string(), subdir, '/');

    for (auto* s : {&record_path, &hash_path}) {
        if (recursive_create_directory(s->c_str(), S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
            LOG(LOG_ERR,
                "AclSerializer::start_session_log: Failed to create directory: \"%s\"", *s);
        }
    }

    std::string basename = str_concat(filebase, ".log");
    record_path += basename;
    hash_path += basename;
    this->log_file->open(record_path, hash_path, groupid, /*derivator=*/basename);
}

void AclSerializer::close_session_log()
{
    this->log_file->close();
}

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
                LOG(LOG_INFO, "ACL SERIALIZER : Data size without header (receive) = %" PRIdPTR, this->e - this->p);
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

                    LOG(LOG_INFO, "receiving '%.*s'='%.*s'",
                        int(key.size()), key.data(),
                        int(display_val.size()), display_val.data());
                }
            }
            else if (reader.consume_ask()) {
                field.ask();
                LOG_IF(bool(this->verbose & Verbose::variable), LOG_INFO,
                    "receiving ASK '%*s'", int(key.size()), key.data());
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
            LOG(LOG_WARNING, "Unexpected receiving '%s' - '%.*s'",
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


Acl::Acl(Inifile & ini, TimeBase & time_base)
: ini(ini)
, acl_serial(nullptr)
, keepalive(ini.get<cfg::globals::keepalive_grace_delay>(), to_verbose_flags(ini.get<cfg::debug::auth>()))
, inactivity(ini.get<cfg::globals::session_timeout>(),
             time_base.get_current_time().tv_sec,
             to_verbose_flags(ini.get<cfg::debug::auth>()))
{}

time_t Acl::get_inactivity_timeout()
{
    return this->inactivity.get_inactivity_timeout();
}

void Acl::receive()
{
    try {
        this->acl_serial->incoming();

        if (this->ini.get<cfg::context::module>() == "RDP"
        ||  this->ini.get<cfg::context::module>() == "VNC") {
            this->acl_serial->session_type = this->ini.get<cfg::context::module>();
        }
        this->acl_serial->remote_answer = true;
    } catch (...) {
        LOG(LOG_INFO, "Acl::receive() Session lost");
        // acl connection lost
        this-> status = state_disconnected_by_authentifier;
        this->ini.set_acl<cfg::context::authenticated>(false);

        if (this->manager_disconnect_reason.empty()) {
            this->ini.set_acl<cfg::context::rejected>(
                TR(trkeys::manager_close_cnx, language(this->ini)));
        }
        else {
            this->ini.set_acl<cfg::context::rejected>(this->manager_disconnect_reason);
            this->manager_disconnect_reason.clear();
        }
    }
}


void Acl::update_inactivity_timeout()
{
    time_t conn_opts_inactivity_timeout = this->ini.get<cfg::globals::inactivity_timeout>().count();
    if (conn_opts_inactivity_timeout > 0) {
        if (this->inactivity.get_inactivity_timeout()!= conn_opts_inactivity_timeout) {
            this->inactivity.update_inactivity_timeout(conn_opts_inactivity_timeout);
        }
    }
}


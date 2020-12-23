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
  Copyright (C) Wallix 2010-2020
  Author(s): Wallix Team

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/

#include "acl/session_logfile.hpp"

#include "configs/config.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/fileutils.hpp"
#include "utils/log_siem.hpp"
#include "main/version.hpp"


SessionLogFile::SessionLogFile(
    Inifile const& ini, CryptoContext& cctx, Random& rnd, Fstat& fstat,
    std::function<void (const Error &)> notify_error)
: ini(ini)
, cctx(cctx)
, ct(cctx, rnd, fstat, std::move(notify_error))
{
    this->log6_buffer.reserve(512);
}

SessionLogFile::~SessionLogFile()
{
    try {
        this->close_session_log();
    }
    catch (Error const& e) {
        LOG(LOG_ERR, "~SessionLogFile: %s", e.errmsg());
    }
}

void SessionLogFile::log6(time_t time_now, LogId id, KVLogList kv_list)
{
    assert(this->ct.is_open());

    chars_view av = log_format_set_info(this->log6_buffer, id, kv_list);

    char mbstr[100];
    auto const len = std::strftime(mbstr, sizeof(mbstr), "%F %T ", std::localtime(&time_now));
    if (len) {
        this->ct.send(mbstr, len);
    }

    this->ct.send(av.data(), av.size());
    this->ct.send("\n", 1);
}

void SessionLogFile::open_session_log()
{
    assert(!this->ct.is_open());

    this->cctx.set_master_key(this->ini.get<cfg::crypto::encryption_key>());
    this->cctx.set_hmac_key(this->ini.get<cfg::crypto::sign_key>());
    this->cctx.set_trace_type(this->ini.get<cfg::globals::trace_type>());

    const int groupid = this->ini.get<cfg::video::capture_groupid>();
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

    this->ct.open(
        record_path.c_str(), hash_path.c_str(), groupid,
        this->ini.get<cfg::video::file_permissions>(), /*derivator=*/basename);
    // force to create the file
    this->ct.send("", 0);
}

void SessionLogFile::close_session_log()
{
    if (this->ct.is_open()) {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];
        this->ct.close(qhash, fhash);
    }
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

    inline StringBuf<64> from_gmtime(std::time_t time) noexcept
    {
        StringBuf<64> buf;

        struct tm t;
        gmtime_r(&time, &t);

        static char const* months[]{
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
        };

        // MMM(text) dd yyyy hh:mm:ss
        int len = snprintf(buf.data(), buf.capacity(), "%s %02d %04d %02d:%02d:%02d",
            months[t.tm_mon], t.tm_mday, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
        assert(len > 0);
        buf.setsize(unsigned(len));

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
            str_append(buffer, '[', session_type, " Session] "_av);
        }
        append("session_id=\"", session_id);
        append("client_ip=\"",  host);
        append("target_ip=\"",  target_ip);
        append("user=\"",       user);
        append("device=\"",     device);
        append("service=\"",    service);
        append("account=\"",    account);
    }

    inline chars_view get_target_ip(const Inifile & ini)
    {
        char c = ini.get<cfg::context::target_host>()[0];
        using av = chars_view;
        return ('0' <= c && c <= '9')
            ? av(ini.get<cfg::context::target_host>())
            : av(ini.get<cfg::context::ip_target>());
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
        KVLogList kv_list)
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

SiemLogger::SiemLogger()
{
    buffer_.reserve(512);
}

void SiemLogger::log_syslog_format(
    LogId id, KVLogList kv_list, const Inifile & ini, chars_view session_type)
{
    if (ini.get<cfg::session_log::enable_session_log>()) {
        log_format_set_siem(
            this->buffer_,
            session_type,
            ini.get<cfg::globals::auth_user>(),
            ini.get<cfg::globals::target_user>(),
            ini.get<cfg::context::session_id>(),
            ini.get<cfg::globals::host>(),
            get_target_ip(ini),
            ini.get<cfg::globals::target_device>(),
            ini.get<cfg::context::target_service>());

        log_format_append_info(this->buffer_, id, kv_list);

        LOG_SIEM("%s", this->buffer_.c_str());
    }
}

void SiemLogger::log_arcsight_format(
    std::time_t time_now,
    LogId id, KVLogList kv_list, const Inifile& ini, chars_view session_type)
{
    if (ini.get<cfg::session_log::enable_arcsight_log>()) {
        log_format_set_arcsight(
            this->buffer_,
            id,
            time_now,
            session_type,
            ini.get<cfg::globals::auth_user>(),
            ini.get<cfg::globals::target_user>(),
            ini.get<cfg::context::session_id>(),
            ini.get<cfg::globals::host>(),
            get_target_ip(ini),
            ini.get<cfg::globals::target_device>(),
            ini.get<cfg::context::target_service>(),
            kv_list);

        LOG_SIEM("%s", this->buffer_);
    }
}

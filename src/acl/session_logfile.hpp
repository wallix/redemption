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


#pragma once

#include "core/back_event_t.hpp"
#include "acl/auth_api.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/timebase.hpp"
#include "configs/config.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/fileutils.hpp"
#include "main/version.hpp"
#include "utils/log_siem.hpp"
#include <string>
#include <chrono>

#include <ctime>

class SessionLogFile
{
    Inifile & ini;
    TimeBase & time_base;
    CryptoContext & cctx;
    OutCryptoTransport ct;
    bool logfile_is_open = false;

public:
    SessionLogFile(Inifile&ini, TimeBase & time_base, CryptoContext & cctx, Random & rnd, Fstat & fstat, std::function<void(const Error & error)> notify_error)
    : ini(ini), time_base(time_base), cctx(cctx), ct(cctx, rnd, fstat, notify_error)
    {}

    ~SessionLogFile()
    {
        try {
            this->close_session_log();
        }
        catch (Error const& e) {
            LOG(LOG_ERR, "~SessionLogFile: %s", e.errmsg());
        }
    }

    bool is_open()
    {
        return this->logfile_is_open;
    }

    void open(std::string const& log_path, std::string const& hash_path, int groupid, bytes_view derivator)
    {
        assert(!this->ct.is_open());
        this->ct.open(log_path.c_str(), hash_path.c_str(), groupid, -1, derivator);
        // force to create the file
        this->ct.send("", 0);
        this->logfile_is_open = true;
    }

    void log6(LogId id, KVList kv_list)
    {
        const timeval time = this->time_base.get_current_time();
        std::string buffer_info;
        buffer_info.reserve(kv_list.size() * 50 + 30);

        time_t const time_now = time.tv_sec;
        this->write_line(time_now, log_format_set_info(buffer_info, id, kv_list));
    }

    void write_line(std::time_t time, chars_view av)
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

    void start_session_log()
    {
        this->cctx.set_master_key(this->ini.get<cfg::crypto::key0>());
        this->cctx.set_hmac_key(this->ini.get<cfg::crypto::key1>());
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
        this->open(record_path, hash_path, groupid, /*derivator=*/basename);
    }

    void close_session_log()
    {
        if (this->ct.is_open()) {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH];
            uint8_t fhash[MD_HASH::DIGEST_LENGTH];
            this->ct.close(qhash, fhash);
            this->logfile_is_open = false;
        }
    }
};

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

   
    inline void log_siem_syslog(LogId id, KVList kv_list, const Inifile & ini, const std::string & session_type)
    {
        if (ini.get<cfg::session_log::enable_session_log>()) {
        
            auto target_ip = [&ini]{
                char c = ini.get<cfg::context::target_host>()[0];
                using av = chars_view;
                return ('0' <= c && '9' <= c)
                    ? av(ini.get<cfg::context::target_host>())
                    : av(ini.get<cfg::context::ip_target>());
            };

            std::string buffer_info;
            buffer_info.reserve(kv_list.size() * 50 + 30);
            log_format_set_info(buffer_info, id, kv_list);

            std::string buffer;
            log_format_set_siem(
                buffer,
                session_type,
                ini.get<cfg::globals::auth_user>(),
                ini.get<cfg::globals::target_user>(),
                ini.get<cfg::context::session_id>(),
                ini.get<cfg::globals::host>(),
                target_ip(),
                ini.get<cfg::globals::target_device>(),
                ini.get<cfg::context::target_service>());

            LOG_SIEM("%s%s", buffer.c_str(), buffer_info.c_str());
        }
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

    inline void log_siem_arcsight(std::time_t time_now, LogId id, KVList kv_list, const Inifile & ini, const std::string & session_type)
    {
        if (ini.get<cfg::session_log::enable_arcsight_log>()) {

            auto target_ip = [&ini]{
                char c = ini.get<cfg::context::target_host>()[0];
                using av = chars_view;
                return ('0' <= c && '9' <= c)
                    ? av(ini.get<cfg::context::target_host>())
                    : av(ini.get<cfg::context::ip_target>());
            };

            std::string buffer;
            log_format_set_arcsight(
                buffer, id, time_now,
                session_type,
                ini.get<cfg::globals::auth_user>(),
                ini.get<cfg::globals::target_user>(),
                ini.get<cfg::context::session_id>(),
                ini.get<cfg::globals::host>(),
                target_ip(),
                ini.get<cfg::globals::target_device>(),
                ini.get<cfg::context::target_service>(),
                kv_list);

            LOG_SIEM("%s", buffer);
        }
    }
    
}


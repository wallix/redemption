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
#include "utils/key_qvalue_pairs.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "utils/log_siem.hpp"
#include "utils/tm_to_chars.hpp"
#include "main/version.hpp"


namespace
{
    struct GmTimeBuffer
    {
        using data_format = dateformats::MMM_dd_YYYY_HH_MM_SS;

        GmTimeBuffer(std::time_t time) noexcept
        {
            struct tm t;
            gmtime_r(&time, &t);
            data_format::to_chars(data, t);
        }

        std::string_view sv() const noexcept
        {
            return {data, data_format::output_length};
        }

        char data[data_format::output_length];
    };

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
    };
    static_assert(std::size(ints_s) >= detail::log_id_max);

    namespace table_formats
    {
        constexpr inline auto arcsight_escaped_table = []{
            std::array<char, 256> t{};
            t['='] = '=';
            t['\\'] = '\\';
            t['\n'] = 'n';
            t['\r'] = 'r';
            return t;
        }();

        using qvalue_table_formats::siem_escaped_table;
    } // namespace table_formats
} // anonymous namespace


SessionLogFile::SessionLogFile(
    CryptoContext& cctx, Random& rnd,
    Siem siem, Syslog syslog, Arcsight arcsight,
    std::function<void(const Error &)> notify_error)
: enable_siem(safe_int(siem))
, enable_syslog(safe_int(syslog))
, enable_arcsight(safe_int(arcsight))
, ct(cctx, rnd, std::move(notify_error))
{
    buffer.grow_without_copy(512);
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

void SessionLogFile::set_control_owner_ctx(chars_view name)
{
    if (name.empty()) {
        control_owner_extra_log_len = 0;
    }
    else {
        auto prefix = " control_owner=\""_av;
        auto buffer_size = prefix.size() + safe_size_for_escaped_qvalue(name) + 1;
        char* data = control_owner_extra_log.grow_without_copy(buffer_size).as_charp();
        char* p = data;
        p = qvalue_table_formats::append(p, prefix);
        p = escaped_qvalue(p, name, table_formats::siem_escaped_table);
        *p++ = '"';
        auto final_len = static_cast<std::size_t>(p - data);
        control_owner_extra_log_len = final_len;
    }
}

std::string const& SessionLogFile::get_target_ip(Inifile const& ini)
{
    char c = ini.get<cfg::context::target_host>()[0];
    return ('0' <= c && c <= '9')
        ? ini.get<cfg::context::target_host>()
        : ini.get<cfg::context::ip_target>();
}

std::string const& SessionLogFile::get_account(Inifile const& ini)
{
    return ini.get<cfg::globals::target_user>();
}

void SessionLogFile::log(
    std::time_t time_now, Inifile& ini,
    chars_view session_type, LogId id, KVLogList kv_list)
{
    assert(this->ct.is_open());

    std::size_t len = safe_size_for_log_format_append_info(id, kv_list)
                    + control_owner_extra_log_len;

    char* p;

    constexpr std::size_t meta_prefix_len = 20 /* "YYYY-MM-DD HH:MM:SS " */;
    constexpr std::size_t meta_suffix_len = 1 /* \n */;

    if (enable_siem) {
        struct Data {
            chars_view prefix;
            chars_view value;
        };

        Data pairs[] {
            {"session_id=\""_av, ini.get<cfg::context::session_id>()},
            {"client_ip=\""_av,  ini.get<cfg::globals::host>()},
            {"target_ip=\""_av,  get_target_ip(ini)},
            {"user=\""_av,       ini.get<cfg::globals::auth_user>()},
            {"device=\""_av,     ini.get<cfg::globals::target_device>()},
            {"service=\""_av,    ini.get<cfg::context::target_service>()},
            {"account=\""_av,    get_account(ini)},
        };

        auto suffix_type = " Session"_av;
        auto prefix_type = session_type.empty() ? "Neutral"_av : session_type;

        len += suffix_type.size() + prefix_type.size() + 3 /* '[' + ']' + ' ' */;
        for (Data d : pairs) {
            len += d.prefix.size() + safe_size_for_escaped_qvalue(d.value) + 2;
        }

        p = buffer.grow_without_copy(len).as_charp();

        *p++ = '[';
        p = qvalue_table_formats::append(p, prefix_type);
        p = qvalue_table_formats::append(p, suffix_type);
        *p++ = ']';
        for (Data d : pairs) {
            *p++ = ' ';
            p = qvalue_table_formats::append(p, d.prefix);
            p = escaped_qvalue(p, d.value, table_formats::siem_escaped_table);
            *p++ = '"';
        }
        *p++ = ' ';
    }
    else {
        len += meta_prefix_len + meta_suffix_len;
        p = buffer.grow_without_copy(len).as_charp() + meta_prefix_len;
    }

    char* start_meta_format = p;

    // log for SIEM and meta
    p = log_format_append_info(p, id, kv_list);
    p = qvalue_table_formats::append(p, {control_owner_extra_log.buffer().as_charp(), control_owner_extra_log_len});

    if (enable_siem) {
        char const* data = buffer.buffer().as_charp();
        LOG_REDEMPTION_INTERNAL_IMPL(LOG_INFO, "%.*s", int(p - data), data);
    }

    if (REDEMPTION_UNLIKELY(enable_syslog)) {
        LOG(LOG_INFO, "<%.*s> %.*s",
            int(session_type.size()), session_type.data(),
            int(p - start_meta_format), start_meta_format);
    }

    *p++ = '\n';
    char* meta = start_meta_format - meta_prefix_len;
    struct tm tm;
    localtime_r(&time_now, &tm);
    *dateformats::YYYY_mm_dd_HH_MM_SS::to_chars(meta, tm) = ' ';
    this->ct.send({meta, p});

    if (enable_arcsight) {
        GmTimeBuffer str_time{time_now};
        chars_view contexts[] {
            str_time.sv(),
            " host message CEF:1|Wallix|Bastion|" VERSION "|"_av,
            ints_s[unsigned(id)], "|"_av,
            detail::log_id_string_map[unsigned(id)], "|"
            "5"_av /*TODO severity*/
            "|WallixBastionSessionType="_av, session_type.empty() ? "Neutral"_av : session_type,
            " WallixBastionSessionId="_av, ini.get<cfg::context::session_id>(),
            " WallixBastionHost="_av, ini.get<cfg::globals::host>(),
            " WallixBastionTargetIP="_av, get_target_ip(ini),
            " WallixBastionUser="_av, ini.get<cfg::globals::auth_user>(),
            " WallixBastionDevice="_av, ini.get<cfg::globals::target_device>(),
            " WallixBastionService="_av, ini.get<cfg::context::target_service>(),
            " WallixBastionAccount="_av, ini.get<cfg::globals::target_user>()
        };
        constexpr auto prefix = "="_av;
        constexpr auto suffix = ""_av;
        std::size_t len = safe_size_for_kv_list_to_string(prefix, suffix, kv_list);
        for (auto av : contexts) {
            len += av.size();
        }
        char* msg = buffer.grow_without_copy(len).as_charp();
        p = msg;
        for (auto av : contexts) {
            p = qvalue_table_formats::append(p, av);
        }
        p = kv_list_to_string(p, kv_list, prefix, suffix, table_formats::arcsight_escaped_table);

        LOG_REDEMPTION_INTERNAL_IMPL(LOG_INFO, "%.*s", int(p - msg), msg);
    }
}

void SessionLogFile::open_session_log(
    const char * const record_path, const char * const hash_path,
    FilePermissions file_permissions, bytes_view derivator)
{
    assert(!this->ct.is_open());

    this->ct.open(record_path, hash_path, file_permissions, derivator);
    // force to create the file
    this->ct.send("", 0);
}

void SessionLogFile::close_session_log()
{
    this->control_owner_extra_log_len = 0;
    if (this->ct.is_open()) {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];
        this->ct.close(qhash, fhash);
    }
}

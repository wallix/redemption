/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "core/log_id.hpp"
#include "acl/auth_api.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include <array>
#include <vector>
#include <algorithm>

#include <cstring>


namespace qvalue_table_formats
{
    constexpr inline auto siem_escaped_table = []{
        std::array<char, 256> t{};
        t['"'] = '"';
        t['\\'] = '\\';
        t['\n'] = 'n';
        t['\r'] = 'r';
        return t;
    }();

    static char* append(char* buffer, chars_view av)
    {
        memcpy(buffer, av.data(), av.size());
        return buffer + av.size();
    }

    constexpr inline chars_view log_id_as_type_strings[]{
        #define f(x, cat, attr) "type=\"" #x "\""_av,
        X_LOG_ID(f)
        #undef f
    };

    constexpr inline auto siem_value_prefix = "=\""_av;
    constexpr inline auto siem_value_suffix = "\""_av;
} // namespace qvalue_table_formats

inline std::size_t safe_size_for_escaped_qvalue(chars_view data) noexcept
{
    return data.size() * 2;
}

[[nodiscard]]
inline char * escaped_qvalue(
    char * buffer,
    chars_view data,
    std::array<char, 256> const& escaped_table)
{
    auto escaped = [&](char c){
        // char -> uchar because char(128) must be negative
        using uchar = unsigned char;
        return escaped_table[unsigned(uchar(c))];
    };

    auto first = data.begin();
    auto last = data.end();

    char const* p;
    while ((p = std::find_if(first, last, escaped)) != last) {
        buffer = qvalue_table_formats::append(buffer, {first, p});
        *buffer++ = '\\';
        *buffer++ = escaped(*p);
        first = p + 1;
    }

    return qvalue_table_formats::append(buffer, {first, last});
}

inline std::size_t safe_size_for_kv_list_to_string(
    chars_view prefix,
    chars_view suffix,
    KVLogList kv_list) noexcept
{
    std::size_t len = (prefix.size() + suffix.size() + 1) * kv_list.size();

    for (auto& kv : kv_list) {
        len += kv.key.size();
        len += safe_size_for_escaped_qvalue(kv.value);
    }

    return len;
}

[[nodiscard]]
inline char * kv_list_to_string(
    char * buffer, KVLogList kv_list,
    chars_view prefix, chars_view suffix,
    std::array<char, 256> const& escaped_table)
{
    for (auto& kv : kv_list) {
        *buffer++ = ' ';
        buffer = qvalue_table_formats::append(buffer, kv.key);
        buffer = qvalue_table_formats::append(buffer, prefix);
        buffer = escaped_qvalue(buffer, kv.value, escaped_table);
        buffer = qvalue_table_formats::append(buffer, suffix);
    }

    return buffer;
}

inline void kv_list_to_string(
    std::vector<char>& buffer, KVLogList kv_list,
    chars_view prefix, chars_view suffix,
    std::array<char, 256> const& escaped_table)
{
    auto old_size = buffer.size();
    buffer.resize(old_size + safe_size_for_kv_list_to_string(prefix, suffix, kv_list));

    char* p = buffer.data() + old_size;
    p = kv_list_to_string(p, kv_list, prefix, suffix, escaped_table);

    auto final_len = static_cast<std::size_t>(p - buffer.data());
    buffer.resize(final_len);
}

inline std::size_t safe_size_for_log_format_append_info(LogId id, KVLogList kv_list)
{
    auto type = qvalue_table_formats::log_id_as_type_strings[int(id)];

    return type.size()
      + safe_size_for_kv_list_to_string(qvalue_table_formats::siem_value_prefix,
                                        qvalue_table_formats::siem_value_suffix,
                                        kv_list);
}

inline char * log_format_append_info(char * buffer, LogId id, KVLogList kv_list)
{
    auto type = qvalue_table_formats::log_id_as_type_strings[int(id)];

    buffer = qvalue_table_formats::append(buffer, type);
    buffer = kv_list_to_string(buffer, kv_list,
                               qvalue_table_formats::siem_value_prefix,
                               qvalue_table_formats::siem_value_suffix,
                               qvalue_table_formats::siem_escaped_table);

    return buffer;
}

inline void log_format_set_info(std::vector<char>& buffer, LogId id, KVLogList kv_list)
{
    buffer.clear();
    buffer.resize(safe_size_for_log_format_append_info(id, kv_list));
    char* p = log_format_append_info(buffer.data(), id, kv_list);

    auto final_len = static_cast<std::size_t>(p - buffer.data());
    buffer.resize(final_len);
}

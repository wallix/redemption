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
#include "utils/sugar/algostring.hpp"

#include <string>
#include <algorithm>


namespace qvalue_table_formats
{
    constexpr auto log()
    {
        std::array<char, 256> t{};
        t[int('"')] = '"';
        t[int('\\')] = '\\';
        t[int('\n')] = 'n';
        t[int('\r')] = 'r';
        return t;
    }

    constexpr inline auto log_table = log();
} // namespace qvalue_table_formats

inline void escaped_qvalue(
    std::string& escaped_subject,
    chars_view subject,
    std::array<char, 256> const& escaped_table)
{
    auto escaped = [&](char c){
        // char -> uchar because char(128) must be negative
        using uchar = unsigned char;
        return escaped_table[unsigned(uchar(c))];
    };

    auto first = subject.begin();
    auto last = subject.end();

    decltype(first) p;
    while ((p = std::find_if(first, last, escaped)) != last) {
        str_append(escaped_subject, chars_view{first, p}, '\\', escaped(*p));
        first = p + 1;
    }

    escaped_subject.append(first, last);
}

template<class Prefix, class Suffix>
inline std::string& kv_list_to_string(
    std::string& buffer, KVLogList kv_list,
    Prefix prefix, Suffix suffix,
    std::array<char, 256> const& escaped_table)
{
    for (auto& kv : kv_list) {
        str_append(buffer, ' ', kv.key, prefix);
        escaped_qvalue(buffer, kv.value, escaped_table);
        buffer += suffix;
    }

    return buffer;
}

constexpr inline chars_view log_id_string_type_map[]{
    #define f(x, cat) "type=\"" #x "\""_av,
    X_LOG_ID(f)
    #undef f
};

inline chars_view log_format_set_info(std::string& buffer, LogId id, KVLogList kv_list)
{
    auto type = log_id_string_type_map[int(id)];
    buffer.assign(type.begin(), type.end());
    kv_list_to_string(buffer, kv_list, "=\"", '"', qvalue_table_formats::log_table);
    return buffer;
}

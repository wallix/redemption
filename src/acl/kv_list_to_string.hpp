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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#define NOT_UNDEF_X_LOG_ID 1

#include "core/log_id.hpp"
#include "core/report_message_api.hpp"
#include "utils/key_qvalue_pairs.hpp"

template<class Prefix, class Suffix>
inline std::string& kv_list_to_string(
    std::string& buffer, KVList kv_list,
    Prefix prefix, Suffix suffix,
    std::array<char, 256> const& escaped_table)
{
    for (auto& kv : kv_list) {
        buffer += ' ';
        buffer.append(kv.key.data(), kv.key.size());
        buffer += prefix;
        escaped_qvalue(buffer, kv.value, escaped_table);
        buffer += suffix;
    }

    return buffer;
}

constexpr inline array_view_const_char log_id_string_type_map[]{
    #define f(x, cat) "type=\"" #x "\""_av,
    X_LOG_ID(f)
    #undef f
};

inline array_view_const_char log_format_set_info(std::string& buffer, LogId id, KVList kv_list)
{
    auto type = log_id_string_type_map[int(id)];
    buffer.assign(type.begin(), type.end());
    kv_list_to_string(buffer, kv_list, "=\"", '"', qvalue_table_formats::log_table);
    return buffer;
}

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
Copyright (C) Wallix 2022
Author(s): Proxies Team
*/

#include "test_only/session_log_test.hpp"

#include "utils/strutils.hpp"
#include "core/log_id.hpp"


void SessionLogTest::report(const char * reason, const char * message)
{
    str_append(messages, reason, ": ", message, '\n');
}

void SessionLogTest::log6(LogId id, KVLogList kv_list)
{
    messages += detail::log_id_string_map[int(id)].to_sv();
    for (auto& kv : kv_list) {
        str_append(messages, ' ', kv.key, "=\"", kv.value, '"');
    }
    messages += extra_msg;
    messages += '\n';
}

void SessionLogTest::set_owner_control_ctx(chars_view name)
{
    if (name.empty()) {
        extra_msg.clear();
    }
    else {
        str_assign(extra_msg, " control_owner=\"", name, '"');
    }
}

std::string SessionLogTest::events()
{
    auto res = messages;
    messages.clear();
    return res;
}

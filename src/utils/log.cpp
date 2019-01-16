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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include "log.hpp"
#include "utils/strutils.hpp"

#include <cstring>


namespace
{
    static char log_proxy_psid[32] = "42";
    static char log_proxy_username[256] = "";
}

namespace detail
{
    void log_proxy_init(char const* psid, char const* source_ip, int source_port) noexcept
    {
        utils::strlcpy(log_proxy_psid, psid);
        if (0 != strcmp(source_ip, "127.0.0.1")) {
            LOG__REDEMPTION__INTERNAL__IMPL(
                LOG_INFO,
                R"([RDP Proxy] psid="%s" type="INCOMING_CONNECTION" src_ip="%s" src_port="%d")",
                log_proxy_psid, source_ip, source_port
            );
        }
    }

    void log_proxy_set_user(char const* username) noexcept
    {
        utils::strlcpy(log_proxy_username, username);
    }

    void log_proxy_target_disconnection(char const* reason) noexcept
    {
        if (reason && *reason) {
            LOG_PROXY_SIEM("TARGET_DISCONNECTION", R"(reason="%s")", reason);
        }
        else {
            LOG_PROXY_SIEM("TARGET_DISCONNECTION", "");
        }
    }

    void log_proxy_logout(char const* reason) noexcept
    {
        if (log_proxy_username[0]){
            if (reason && *reason) {
                LOG_PROXY_SIEM("LOGOUT", R"(reason="%s")", reason);
            }
            else {
                LOG_PROXY_SIEM("LOGOUT", "");
            }
            detail::log_proxy_set_user("");
        }
    }

    void log_proxy_disconnection(char const* reason) noexcept
    {
        if (reason && *reason) {
            if (log_proxy_username[0]) {
                LOG__REDEMPTION__INTERNAL__IMPL(
                    LOG_INFO,
                    R"([RDP Proxy] psid="%s" user="%s" type="DISCONNECT" reason="%s")",
                    log_proxy_psid, log_proxy_username, reason
                );
            }
            else {
                LOG__REDEMPTION__INTERNAL__IMPL(
                    LOG_INFO,
                    R"([RDP Proxy] psid="%s" type="DISCONNECT" reason="%s")",
                    log_proxy_psid, reason
                );
            }
        }
        else {
            if (log_proxy_username[0]) {
                LOG__REDEMPTION__INTERNAL__IMPL(
                    LOG_INFO,
                    R"([RDP Proxy] psid="%s" user="%s" type="DISCONNECT")",
                    log_proxy_psid, log_proxy_username
                );
            }
            else {
                LOG__REDEMPTION__INTERNAL__IMPL(
                    LOG_INFO,
                    R"([RDP Proxy] psid="%s" type="DISCONNECT")",
                    log_proxy_psid
                );
            }
        }
    }

    char const* log_proxy_get_psid() noexcept
    {
        return log_proxy_psid;
    }

    char const* log_proxy_get_user() noexcept
    {
        return log_proxy_username;
    }
}

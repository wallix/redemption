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

#include "log_siem.hpp"
#include "utils/log.hpp"
#include "utils/strutils.hpp"


# define LOG_SIEM(format, ...) do {                            \
    LOG_REDEMPTION_INTERNAL_IMPL(                              \
        /* check format and insert LOG_INFO */                 \
        (void(sizeof(printf(format, __VA_ARGS__))), LOG_INFO), \
        format, __VA_ARGS__);                                  \
} while (0)


namespace log_siem
{
    namespace
    {
        char g_psid[32] = "42"; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */
        char g_username[256] = ""; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */
    }

    void set_psid(std::string_view psid) noexcept
    {
        utils::strlcpy(g_psid, psid);
    }

    void incoming_connection(std::string_view source_ip, int source_port) noexcept
    {
        LOG_SIEM(
            R"([rdpproxy] psid="%s" type="INCOMING_CONNECTION" src_ip="%.*s" src_port="%d")",
            g_psid, int(source_ip.size()), source_ip.data(), source_port
        );
    }

    void set_user(std::string_view username) noexcept
    {
        utils::strlcpy(g_username, username);
    }

    void target_connection(char const* target_user, char const* session_id,
                           char const* target_host, unsigned target_port) noexcept
    {
        LOG_SIEM(
            R"([rdpproxy] psid="%s" user="%s" type="TARGET_CONNECTION" target="%s" session_id="%s" host="%s" port="%u")",
            g_psid, g_username, target_user, session_id, target_host, target_port
        );
    }

    void target_connection_failed(char const* target_user, char const* session_id,
                                  char const* target_host, unsigned target_port,
                                  char const* reason) noexcept
    {
        LOG_SIEM(
            R"([rdpproxy] psid="%s" user="%s" type="TARGET_CONNECTION_FAILED" target="%s" session_id="%s" host="%s" port="%u" reason="%s")",
            g_psid, g_username, target_user, session_id, target_host, target_port, reason
        );
    }

    void target_disconnection(char const* reason, char const* session_id) noexcept
    {
        if (reason && *reason) {
            LOG_SIEM(
                R"([rdpproxy] psid="%s" user="%s" type="TARGET_DISCONNECTION" session_id="%s" reason="%s")",
                g_psid, g_username, session_id, reason
            );
        }
        else {
            LOG_SIEM(
                R"([rdpproxy] psid="%s" user="%s" type="TARGET_DISCONNECTION" session_id="%s")",
                g_psid, g_username, session_id
            );
        }
    }

    void disconnection(char const* reason) noexcept
    {
        if (reason && *reason) {
            if (g_username[0]) {
                LOG_SIEM(
                    R"([rdpproxy] psid="%s" user="%s" type="DISCONNECT" reason="%s")",
                    g_psid, g_username, reason
                );
            }
            else {
                LOG_SIEM(
                    R"([rdpproxy] psid="%s" type="DISCONNECT" reason="%s")",
                    g_psid, reason
                );
            }
        }
        else {
            if (g_username[0]) {
                LOG_SIEM(
                    R"([rdpproxy] psid="%s" user="%s" type="DISCONNECT")",
                    g_psid, g_username
                );
            }
            else {
                LOG_SIEM(
                    R"([rdpproxy] psid="%s" type="DISCONNECT")",
                    g_psid
                );
            }
        }
    }
} // namespace log_siem

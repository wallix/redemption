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
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Connect to target Underlying socket for module needs
*/

#pragma once

#include "utils/log.hpp"
#include "configs/config.hpp"
#include "utils/timebase.hpp"
#include "acl/auth_api.hpp"
#include "utils/translation.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/log_siem.hpp"
#include "core/log_id.hpp"
#include "utils/netutils.hpp"
#include "utils/load_theme.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/update_lock.hpp"
#include "utils/log_siem.hpp"
#include "utils/fileutils.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "acl/module_manager/enums.hpp"
#include "core/back_event_t.hpp"

static inline unique_fd connect_to_target_host(Inifile & ini, AuthApi& sesman, trkeys::TrKey const& authentification_fail, bool enable_ipv6 = false)
{
    auto throw_error = [&ini, &sesman](char const* error_message, int id) {
        LOG_PROXY_SIEM("TARGET_CONNECTION_FAILED",
            R"(target="%s" host="%s" port="%u" reason="%s")",
            ini.get<cfg::globals::target_user>(),
            ini.get<cfg::context::target_host>(),
            ini.get<cfg::context::target_port>(),
            error_message);

        sesman.log6(LogId::CONNECTION_FAILED, {});

       ini.set<cfg::context::auth_error_message>(TR(trkeys::target_fail, language(ini)));

        LOG(LOG_ERR, "%s", (id == 1)
            ? "Failed to connect to remote TCP host (1)"
            : "Failed to connect to remote TCP host (2)");
        throw Error(ERR_SOCKET_CONNECT_FAILED);
    };

    LOG_PROXY_SIEM("TARGET_CONNECTION",
        R"(target="%s" host="%s" port="%u")",
        ini.get<cfg::globals::target_user>(),
        ini.get<cfg::context::target_host>(),
        ini.get<cfg::context::target_port>());

    const char *ip = ini.get<cfg::context::target_host>().c_str();
    int port = ini.get<cfg::context::target_port>();
    const char *error_message = nullptr;
    char resolved_ip_addr[NI_MAXHOST] { };
    unique_fd client_sck { -1 };

    // Handle ipv4 or both ipv4 and ipv6
    if (!enable_ipv6)
    {
        in_addr s4_sin_addr;

        if ((error_message = resolve_ipv4_address(ip, s4_sin_addr)))
        {
            // TODO: actually this is DNS Failure or invalid address
            throw_error(error_message, 1);
        }
        snprintf(resolved_ip_addr,
                 sizeof(resolved_ip_addr),
                 "%s",
                 inet_ntoa(s4_sin_addr));
        client_sck = ip_connect(ip, port, &error_message);
    }
    else
    {
        AddrInfoPtrWithDel_t addr_info_ptr =
            resolve_both_ipv4_and_ipv6_address(ip, port, &error_message);

        if (error_message)
        {
            throw_error(error_message, 1);
        }
        if (int res = ::getnameinfo(addr_info_ptr->ai_addr,
                                    addr_info_ptr->ai_addrlen,
                                    resolved_ip_addr,
                                    sizeof(resolved_ip_addr),
                                    nullptr,
                                    0,
                                    NI_NUMERICHOST))
        {
            error_message = (res == EAI_SYSTEM) ?
                ::strerror(errno) : ::gai_strerror(res);
            throw_error(error_message, 1);
        }
        client_sck = ip_connect_both_ipv4_and_ipv6(ip, port, &error_message);
    }
    if (!client_sck.is_open())
    {
        throw_error(error_message, 2);
    }
    ini.set<cfg::context::auth_error_message>(TR(authentification_fail,
                                                 language(ini)));
    ini.set<cfg::context::ip_target>(resolved_ip_addr);
    return client_sck;
}


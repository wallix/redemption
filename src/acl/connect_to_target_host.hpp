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
#include "core/session_reactor.hpp"
#include "core/report_message_api.hpp"
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
#include "acl/module_manager/enums.hpp"
#include "core/back_event_t.hpp"

static inline unique_fd connect_to_target_host(Inifile & ini, const TimeBase & time_base, ReportMessageApi& report_message, trkeys::TrKey const& authentification_fail, char *clientAddress = nullptr)
{
    auto throw_error = [&ini, time_base, &report_message](char const* error_message, int id) {
        LOG_PROXY_SIEM("TARGET_CONNECTION_FAILED",
            R"(target="%s" host="%s" port="%u" reason="%s")",
            ini.get<cfg::globals::target_user>(),
            ini.get<cfg::context::target_host>(),
            ini.get<cfg::context::target_port>(),
            error_message);

        report_message.log6(LogId::CONNECTION_FAILED, time_base.get_current_time(), {});

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

    const char * ip = ini.get<cfg::context::target_host>().c_str();
    char ip_addr[256] {};
    in_addr s4_sin_addr;
    if (auto error_message = resolve_ipv4_address(ip, s4_sin_addr)) {
        // TODO: actually this is DNS Failure or invalid address
        throw_error(error_message, 1);
    }

    snprintf(ip_addr, sizeof(ip_addr), "%s", inet_ntoa(s4_sin_addr));

    char const* error_message = nullptr;
    unique_fd client_sck = ip_connect(ip, ini.get<cfg::context::target_port>(), clientAddress, &error_message);

    if (!client_sck.is_open()) {
        throw_error(error_message, 2);
    }

    ini.set<cfg::context::auth_error_message>(TR(authentification_fail, language(ini)));
    ini.set<cfg::context::ip_target>(ip_addr);

    return client_sck;
}


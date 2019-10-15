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
   Copyright (C) Wallix 2018
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/

#pragma once

#include "utils/difftimeval.hpp"
#include "utils/fixed_random.hpp"
#include "core/RDP/nego.hpp"
#include "proxy_recorder/nla_tee_transport.hpp"
#include "proxy_recorder/extract_user_domain.hpp"

class NegoClient
{
    FixedRandom random;
    std::string extra_message;
    Transport & trans;
    RdpNego nego;

public:
    NegoClient(
        bool is_nla, bool is_admin_mode, Transport& trans, TimeObj & timeobj,
        char const* host, char const* target_user, char const* password,
        bool enable_kerberos, const TLSClientParams & tls_client_params, uint64_t verbosity
    )
    : trans(trans)
    , nego(true, target_user, is_nla, is_admin_mode, host, enable_kerberos,
        this->random, timeobj, this->extra_message, Translation::EN,
        tls_client_params,
        to_verbose_flags(verbosity))
    {
        auto [username, domain] = extract_user_domain(target_user);
        nego.set_identity(username, domain, password, "ProxyRecorder");
        // static char ln_info[] = "tsv://MS Terminal Services Plugin.1.Sessions\x0D\x0A";
        // nego.set_lb_info(byte_ptr_cast(ln_info), sizeof(ln_info)-1);
    }

    void send_negotiation_request()
    {
        this->nego.send_negotiation_request(this->trans);
    }

    bool recv_next_data(TpduBuffer& tpdu_buffer, ServerNotifier& notifier)
    {
        return this->nego.recv_next_data(tpdu_buffer, this->trans, notifier);
    }
};


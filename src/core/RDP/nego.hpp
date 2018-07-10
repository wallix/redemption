/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Meng Tan

   Early Transport Protocol Security Negotiation stage

*/

#pragma once

#include "configs/autogen/enums.hpp"
#include "transport/transport.hpp"
#include "utils/verbose_flags.hpp"

#include <memory>

class rdpCredsspClient;
class ServerNotifier;
class Random;
class TimeObj;
class InStream;
class TpduBuffer;

struct RdpNego
{
public:
    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED = 0x01
    };

    const bool tls;
    const bool nla;

private:
    const bool krb;
    const bool restricted_admin_mode;

    bool nla_tried = false;

public:
    uint32_t selected_protocol;

private:
    uint32_t enabled_protocols;
    char username[128];

    uint8_t hostname[16];
    uint8_t user[128];
    uint8_t password[2048];
    uint8_t domain[256];
    const char * target_host;

    uint8_t * current_password;
    Random & rand;
    TimeObj & timeobj;
    char * lb_info;

    std::unique_ptr<rdpCredsspClient> credssp;

    std::string& extra_message;
    Translation::language_t lang;

    enum class /*REDEMPTION_CXX_NODISCARD*/ State
    {
        Negociate,
        SslHybrid,
        Tls,
        Credssp,
        Final,
    };

    State state = State::Negociate;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        credssp     = 0x400,
        negotiation = 128,
    };

    bool enhanced_rdp_security_is_in_effect() const;
    void set_lb_info(uint8_t * lb_info, size_t lb_info_length);

    RdpNego(
        const bool tls, const char * username, bool nla, bool admin_mode,
        const char * target_host, const char krb, Random & rand, TimeObj & timeobj,
        std::string& extra_message, Translation::language_t lang,
        const Verbose verbose = {});

    ~RdpNego();

    void set_identity(char const * user, char const * domain, char const * pass, char const * hostname);

    void send_negotiation_request(OutTransport trans);

    struct ServerCert
    {
        const bool            store;
        const ServerCertCheck check;
        const char *          path;
        ServerNotifier&       notifier;
    };

    const char * get_target_host() const {
        return this->target_host;
    }

    const char * get_user_name() const {
        return this->username;
    }

    /// \return false if terminal state
    REDEMPTION_CXX_NODISCARD
    bool recv_next_data(TpduBuffer& buf, Transport& trans, ServerCert const& cert);

private:
    State fallback_to_tls(OutTransport trans);

    State recv_connection_confirm(OutTransport trans, InStream x224_stream, ServerCert const& cert);

    State activate_ssl_tls(OutTransport trans, ServerCert const& cert);

    State activate_ssl_hybrid(OutTransport trans, ServerCert const& cert);

    State recv_credssp(OutTransport trans, InStream stream);
};

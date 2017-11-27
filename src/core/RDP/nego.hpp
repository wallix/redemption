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

class RdpNego
{
public:
    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED = 0x01
    };

    const bool tls;
    const bool nla;

private:
    bool krb;
    bool restricted_admin_mode;

    bool nla_tried = false;

public:
    enum
    {
        NEGO_STATE_INITIAL,
        NEGO_STATE_NEGOCIATE,
        // NEGO_STATE_FAIL, // Negotiation failure */
        NEGO_STATE_FINAL,
        NEGO_STATE_CREDSSP
    } state;

    uint32_t selected_protocol;

private:
    uint32_t enabled_protocols;
    char username[128];

public:
    OutTransport trans;

private:
    uint8_t hostname[16];
    uint8_t user[128];
public: // for test_nego
    uint8_t password[2048];
private:
    uint8_t domain[256];
    const char * target_host;

    uint8_t * current_password;
    Random & rand;
    TimeObj & timeobj;
    char * lb_info;

    std::unique_ptr<rdpCredsspClient> credssp;

    std::string& extra_message;

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        credssp     = 0x400,
        negotiation = 128,
    };

public:
    RdpNego(const bool tls, Transport & socket_trans, const char * username, bool nla,
            const char * target_host, const char krb, Random & rand, TimeObj & timeobj,
            std::string& extra_message,
            const Verbose verbose = {});
    ~RdpNego();

    void set_identity(char const * user, char const * domain, char const * pass, char const * hostname);
    void set_lb_info(uint8_t * lb_info, size_t lb_info_length);

    // 2.2.1.2 Server X.224 Connection Confirm PDU
    // 2.2.1.2.1 RDP Negotiation Response (RDP_NEG_RSP)
    // 2.2.1.2.2 RDP Negotiation Failure (RDP_NEG_FAILURE)
    void recv_credssp(InStream & stream);

    bool recv_connection_confirm(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path,
        InStream & stream);

    // 2.2.1.1 Client X.224 Connection Request PDU
    // ===========================================
    void send_negotiation_request();
    void fallback_to_tls();

    bool enhanced_rdp_security_is_in_effect() const;
};

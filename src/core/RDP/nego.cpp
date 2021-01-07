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

#include "core/RDP/nego.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/nla/nla_client_kerberos.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/x224.hpp"

#include "utils/sugar/multisz.hpp"
#include "utils/sugar/algostring.hpp"


// Protocol Security Negotiation Protocols
// NLA : Network Level Authentication (TLS implicit)
// TLS : TLS Encryption without NLA
// RDP: Standard Legacy RDP Encryption without TLS nor NLA

struct RdpNegoProtocols
{
    enum {
        Rdp = 0x00000001,
        Tls = 0x00000002,
        Nla = 0x00000004
    };
};

RdpNego::RdpNego(
    const bool tls, const std::string & username, bool nla, bool admin_mode,
    const char * target_host, const bool krb, Random & rand, const TimeBase & time_base,
    std::string& extra_message, Language lang,
    const TLSClientParams & tls_client_params,
    const Verbose verbose)
: tls(nla || tls)
, nla(nla)
, krb(nla && krb)
, restricted_admin_mode(admin_mode)
, selected_protocol(RdpNegoProtocols::Rdp)
, enabled_protocols(RdpNegoProtocols::Rdp
    | (this->tls ? RdpNegoProtocols::Tls : 0)
    | (this->nla ? RdpNegoProtocols::Nla : 0))
, target_host(target_host)
, current_password(nullptr)
, rand(rand)
, time_base(time_base)
, lb_info(nullptr)
, extra_message(extra_message)
, lang(lang)
, tls_client_params(tls_client_params)
, verbose(verbose)
{
    LOG(LOG_INFO, "RdpNego: TLS=%s NLA=%s adminMode=%s",
        ((this->enabled_protocols & RdpNegoProtocols::Tls) ? "Enabled" : "Disabled"),
        ((this->enabled_protocols & RdpNegoProtocols::Nla) ? "Enabled" : "Disabled"),
        (this->restricted_admin_mode ? "Enabled" : "Disabled")
        );

    if (this->restricted_admin_mode
        && !(this->enabled_protocols & RdpNegoProtocols::Nla)) {
        LOG(LOG_ERR, "NLA disabled. Restricted admin mode requires NLA.");
        throw Error(ERR_NEGO_NLA_REQUIRED_BY_RESTRICTED_ADMIN_MODE);
    }

    this->username = username;
}

RdpNego::~RdpNego() = default;

void RdpNego::set_identity(bytes_view user, bytes_view domain, char const * pass, const std::string & hostname)
{
    if (this->nla) {
        this->user.assign(user.data(), user.data()+user.size());
        this->domain = std::vector<uint8_t>{} << domain;

        // Password is a multi-sz!
        // TODO sould be array_view<z?string_view> or vector<z?string_view>
        MultiSZCopy(char_ptr_cast(this->password), sizeof(this->password), pass);
        this->current_password = this->password;

        this->hostname = hostname;
    }
}

void RdpNego::set_lb_info(uint8_t * lb_info, size_t lb_info_length)
{
    if (lb_info_length > 0) {
        this->lb_info = char_ptr_cast(lb_info);
    }
}



// 2.2.1.2 Server X.224 Connection Confirm PDU
// ===========================================

// The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
// server to client during the Connection Initiation phase (see section
// 1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
// (section 2.2.1.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in
// [X224] section 13.4.

// rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1)
// structure or an optional RDP Negotiation Failure (section 2.2.1.2.2)
// structure. The length of the negotiation structure is include " in the X.224
// Connection Confirm Length Indicator field.

// 2.2.1.2.1 RDP Negotiation Response (RDP_NEG_RSP)
// ================================================

// The RDP Negotiation Response structure is used by a server to inform the
// client of the security protocol which it has selected to use for the
// connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
// field MUST be set to 0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is
// a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +-------------------------------------+-------------------------------------+
// | 0x01 EXTENDED_CLIENT_DATA_SUPPORTED | The server supports Extended Client |
// |                                     | Data Blocks in the GCC Conference   |
// |                                     | Create Request user data (section   |
// |                                     | 2.2.1.3).                           |
// +-------------------------------------+-------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the selected security protocol.

// +----------------------------+----------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Standard RDP Security (section 5.3)          |
// +----------------------------+----------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +----------------------------+----------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +----------------------------+----------------------------------------------+


// 2.2.1.2.2 RDP Negotiation Failure (RDP_NEG_FAILURE)
// ===================================================

// The RDP Negotiation Failure structure is used by a server to inform the
// client of a failure that has occurred while preparing security for the
// connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
// field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the packet
// is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
// are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
// field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
// failure code.

// +--------------------------------------+------------------------------------+
// | 0x00000001 SSL_REQUIRED_BY_SERVER    | The server requires that the       |
// |                                      | client support Enhanced RDP        |
// |                                      | Security (section 5.4) with either |
// |                                      | TLS 1.0 (section 5.4.5.1) or       |
// |                                      | CredSSP (section 5.4.5.2). If only |
// |                                      | CredSSP was requested then the     |
// |                                      | server only supports TLS.          |
// +--------------------------------------+------------------------------------+
// | 0x00000002 SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only   |
// |                                      | use Standard RDP Security          |
// |                                      | mechanisms (section 5.3) and does  |
// |                                      | not support any External Security  |
// |                                      | Protocols (section 5.4.5).         |
// +--------------------------------------+------------------------------------+
// | 0x00000003 SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid|
// |                                      | authentication certificate and     |
// |                                      | cannot initialize the External     |
// |                                      | Security Protocol Provider         |
// |                                      | (section 5.4.5).                   |
// +--------------------------------------+------------------------------------+
// | 0x00000004 INCONSISTENT_FLAGS        | The list of requested security     |
// |                                      | protocols is not consistent with   |
// |                                      | the current security protocol in   |
// |                                      | effect. This error is only possible|
// |                                      | when the Direct Approach (see      |
// |                                      | sections 5.4.2.2 and 1.3.1.2) is   |
// |                                      | used and an External Security      |
// |                                      | Protocol (section 5.4.5) is already|
// |                                      | being used.                        |
// +--------------------------------------+------------------------------------+
// | 0x00000005 HYBRID_REQUIRED_BY_SERVER | The server requires that the client|
// |                                      | support Enhanced RDP Security      |
// |                                      | (section 5.4) with CredSSP (section|
// |                                      | 5.4.5.2).                          |
// +--------------------------------------+------------------------------------+

bool RdpNego::recv_next_data(TpduBuffer& tpdubuf, Transport& trans, ServerNotifier& notifier)
{
    switch (this->state) {
        case State::Negociate:
            LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::recv_next_data::Negociate");
            tpdubuf.load_data(trans);
            if (!tpdubuf.next(TpduBuffer::PDU)) {
                return true;
            }
            do {
                LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "nego->state=RdpNego::NEGO_STATE_NEGOCIATE");
                LOG(LOG_INFO, "RdpNego::NEGO_STATE_%s",
                                    (this->nla) ? "NLA" :
                                    (this->tls) ? "TLS" :
                                                  "RDP");
                this->state = this->recv_connection_confirm(trans, InStream(tpdubuf.current_pdu_buffer()), notifier);
            } while (this->state == State::Negociate && tpdubuf.next(TpduBuffer::PDU));
            return (this->state != State::Final);

        case State::SslHybrid:
            LOG(LOG_INFO, "RdpNego::recv_next_data::SslHybrid");
            this->state = this->activate_ssl_hybrid(trans, notifier);
            return (this->state != State::Final);

        case State::Tls:
            LOG(LOG_INFO, "RdpNego::recv_next_data::TLS");
            this->state = this->activate_ssl_tls(trans, notifier);
            return (this->state != State::Final);

        case State::Credssp:
            LOG(LOG_INFO, "RdpNego::recv_next_data::Credssp");
            try {
                tpdubuf.load_data(trans);
            }
            catch (Error const &) {
                this->state = this->fallback_to_tls(trans);
                return true;
            }

            while (this->state == State::Credssp && tpdubuf.next(TpduBuffer::CREDSSP)) {
                this->state = this->recv_credssp(trans, tpdubuf.current_pdu_buffer());
            }

            while (this->state == State::Negociate && tpdubuf.next(TpduBuffer::PDU)) {
                this->state = this->recv_connection_confirm(trans, InStream(tpdubuf.current_pdu_buffer()), notifier);
            }
            return (this->state != State::Final);

        case State::Final:
            return false;
    }

    REDEMPTION_UNREACHABLE();
}

RdpNego::State RdpNego::recv_connection_confirm(OutTransport trans, InStream x224_stream, ServerNotifier& notifier)
{
    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::recv_connection_confirm");

    X224::CC_TPDU_Recv x224(x224_stream);

    if (x224.rdp_neg_type == X224::RDP_NEG_NONE) {
        this->enabled_protocols = RdpNegoProtocols::Rdp;
        LOG(LOG_INFO, "RdpNego::recv_connection_confirm done (legacy, no TLS)");
        return State::Final;
    }

    this->selected_protocol = x224.rdp_neg_code;

    if (x224.rdp_neg_type == X224::RDP_NEG_RSP)
    {
        if (x224.rdp_neg_code == X224::PROTOCOL_HYBRID)
        {
            LOG(LOG_INFO, "activating TLS (HYBRID)");
            return this->activate_ssl_hybrid(trans, notifier);
        }

        if (x224.rdp_neg_code == X224::PROTOCOL_TLS)
        {
            LOG(LOG_INFO, "activating TLS");
            return this->activate_ssl_tls(trans, notifier);
        }

        if (x224.rdp_neg_code == X224::PROTOCOL_RDP)
        {
            LOG(LOG_INFO, "activating RDP Legacy");
            return State::Final;
        }
    }
    else if (x224.rdp_neg_type == X224::RDP_NEG_FAILURE)
    {
        if (x224.rdp_neg_code == X224::HYBRID_REQUIRED_BY_SERVER)
        {
            LOG(LOG_INFO, "Enable NLA is probably required");

            if (!this->nla_tried) {
                str_append(this->extra_message, " ", TR(trkeys::err_nla_required, this->lang));
            }
            trans.disconnect();

            throw Error(this->nla_tried
                ? ERR_NLA_AUTHENTICATION_FAILED
                : ERR_NEGO_HYBRID_REQUIRED_BY_SERVER);
        }

        if (x224.rdp_neg_code == X224::SSL_REQUIRED_BY_SERVER) {
            LOG(LOG_INFO, "Enable TLS is probably required");

            if (!this->tls) {
                str_append(this->extra_message, " ", TR(trkeys::err_tls_required, this->lang));
            }
            trans.disconnect();

            throw Error(ERR_NEGO_SSL_REQUIRED_BY_SERVER);
        }

        if (x224.rdp_neg_code == X224::SSL_NOT_ALLOWED_BY_SERVER
            || x224.rdp_neg_code == X224::SSL_CERT_NOT_ON_SERVER) {
            LOG(LOG_INFO, "Can't activate SSL, falling back to RDP legacy encryption");

            trans.disconnect();
            if (!trans.connect()){
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
            this->enabled_protocols = RdpNegoProtocols::Rdp;
            this->send_negotiation_request(trans);
            return State::Negociate;
        }
    }

    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::recv_connection_confirm done");
    return State::Final;
}

inline bool enable_client_tls(OutTransport trans, ServerNotifier& notifier, const TLSClientParams & tls_client_params)
{
    switch (trans.enable_client_tls(notifier, tls_client_params))
    {
        case Transport::TlsResult::WaitExternalEvent: return false;
        case Transport::TlsResult::Want: return false;
        case Transport::TlsResult::Fail:
            LOG(LOG_ERR, "enable_client_tls fail");
            REDEMPTION_CXX_FALLTHROUGH;
        case Transport::TlsResult::Ok: break;
    }
    return true;
}

RdpNego::State RdpNego::activate_ssl_tls(OutTransport trans, ServerNotifier& notifier) const
{
    if (!enable_client_tls(trans, notifier, this->tls_client_params)) {
        return State::Tls;
    }
    return State::Final;
}

RdpNego::State RdpNego::activate_ssl_hybrid(OutTransport trans, ServerNotifier& notifier)
{
    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::activate_ssl_hybrid");

    // if (x224.rdp_neg_flags & X224::RESTRICTED_ADMIN_MODE_SUPPORTED) {
    //     LOG(LOG_INFO, "Restricted Admin Mode Supported");
    //     this->restricted_admin_mode = true;
    // }
    if (!enable_client_tls(trans, notifier, this->tls_client_params)) {
        return State::SslHybrid;
    }

    this->nla_tried = true;

    LOG(LOG_INFO, "activating CREDSSP");
    if (this->krb) {
        #ifndef __EMSCRIPTEN__
        try {
            this->credsspKerberos = std::make_unique<rdpCredsspClientKerberos>(
                trans, this->user, this->domain, this->current_password,
                this->hostname, this->target_host,
                this->restricted_admin_mode,
                this->rand, this->extra_message, this->lang,
                bool(this->verbose & Verbose::credssp),
                bool(this->verbose & Verbose::negotiation)
            );
        }
        catch (const Error &) {
            LOG(LOG_INFO, "CREDSSP Kerberos Authentication Failed, fallback to NTLM");
            this->krb = false;
        }
        #else
        this->krb = false;
        LOG(LOG_ERR, "Unsupported kerberos: fallback to NTLM");
        #endif
    }

    if (!this->krb) {
        try {
            this->NTLM = std::make_unique<rdpClientNTLM>(
                this->user, this->domain,
                this->current_password,
                this->hostname.c_str(),
                trans.get_transport().get_public_key(),
                this->restricted_admin_mode,
                this->rand, this->time_base,
                bool(this->verbose & Verbose::credssp),
                bool(this->verbose & Verbose::negotiation)
            );
            trans.send(this->NTLM->authenticate_start());
        }
        catch (const Error &){
            LOG(LOG_INFO, "NLA/CREDSSP NTLM Authentication Failed (1)");
            this->fallback_to_tls(trans);
            return State::Negociate;
        }
    }
    // TODO: success should not be default result
    return State::Credssp;
}

RdpNego::State RdpNego::recv_credssp(OutTransport trans, bytes_view data)
{
    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::recv_credssp");

    if (this->krb) {
        #ifndef __EMSCRIPTEN__
        switch (this->credsspKerberos->authenticate_next(data))
        {
            case credssp::State::Cont:
                break;
            case credssp::State::Err:
                LOG(LOG_INFO, "NLA/CREDSSP Authentication Failed (2)");
                return this->fallback_to_tls(trans);
            case credssp::State::Finish:
                this->credsspKerberos.reset();
                return State::Final;
        }
        #else
            LOG(LOG_ERR, "Unsupported kerberos");
            assert(!this->krb);
        #endif
    }
    else {
        auto v = this->NTLM->authenticate_next(data);
        switch (this->NTLM->state)
        {
            case credssp::State::Cont:
                trans.send(v);
                break;
            case credssp::State::Err:
                LOG(LOG_INFO, "NLA/CREDSSP Authentication Failed (2)");
                return this->fallback_to_tls(trans);
            case credssp::State::Finish:
                trans.send(v);
                this->NTLM.reset();
                return State::Final;
        }
    }
    return State::Credssp;
}

RdpNego::State RdpNego::fallback_to_tls(OutTransport trans)
{
    LOG(LOG_INFO, "RdpNego::fallback_to_tls");
    trans.disconnect();

    if (!trans.connect()){
        LOG(LOG_ERR, "Failed to disconnect transport");
        throw Error(ERR_SOCKET_CONNECT_FAILED);
    }
    if (this->restricted_admin_mode) {
        LOG(LOG_ERR, "NLA failed. Restricted admin mode requires NLA.");
        throw Error(ERR_NEGO_NLA_REQUIRED_BY_RESTRICTED_ADMIN_MODE);
    }

    this->current_password += (strlen(char_ptr_cast(this->current_password)) + 1);

    if (*this->current_password) {
        LOG(LOG_INFO, "try next password");
        this->send_negotiation_request(trans);
    }
    else {
        LOG(LOG_INFO, "Can't activate NLA");
        LOG(LOG_INFO, "falling back to SSL only");
        this->enabled_protocols = RdpNegoProtocols::Tls | RdpNegoProtocols::Rdp;
        this->send_negotiation_request(trans);
        return State::Negociate;
    }
    return State::Credssp;
}


// 2.2.1.1 Client X.224 Connection Request PDU
// ===========================================

// The X.224 Connection Request PDU is an RDP Connection Sequence PDU sent from
// client to server during the Connection Initiation phase (see section 1.3.1.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Crq (7 bytes): An X.224 Class 0 Connection Request transport protocol
// data unit (TPDU), as specified in [X224] section 13.3.

// routingToken (variable): An optional and variable-length routing token
// (used for load balancing) terminated by a carriage-return (CR) and line-feed
// (LF) ANSI sequence. For more information about Terminal Server load balancing
// and the routing token format, see [MSFT-SDLBTS]. The length of the routing
// token and CR+LF sequence is include " in the X.224 Connection Request Length
// Indicator field. If this field is present, then the cookie field MUST NOT be
//  present.

//cookie (variable): An optional and variable-length ANSI text string terminated
// by a carriage-return (CR) and line-feed (LF) ANSI sequence. This text string
// MUST be "Cookie: mstshash=IDENTIFIER", where IDENTIFIER is an ANSI string
//(an example cookie string is shown in section 4.1.1). The length of the entire
// cookie string and CR+LF sequence is include " in the X.224 Connection Request
// Length Indicator field. This field MUST NOT be present if the routingToken
// field is present.

// rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
// structure. The length of this negotiation structure is include " in the X.224
// Connection Request Length Indicator field.

void RdpNego::send_negotiation_request(OutTransport trans)
{
    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::send_x224_connection_request_pdu: ...");
    char cookie[256];
    snprintf(cookie, 256, "Cookie: mstshash=%s\x0D\x0A", this->username.c_str());
    char * cookie_or_token = this->lb_info ? this->lb_info : cookie;
    if (bool(this->verbose & Verbose::negotiation)) {
        LOG(LOG_INFO, "Send %s:", this->lb_info ? "load_balance_info" : "cookie");
        hexdump_c(cookie_or_token, strlen(cookie_or_token));
    }
    uint32_t rdp_neg_requestedProtocols = X224::PROTOCOL_RDP
        | ((this->enabled_protocols & RdpNegoProtocols::Nla) ?
           X224::PROTOCOL_HYBRID : 0)
        | ((this->enabled_protocols & RdpNegoProtocols::Tls) ?
           X224::PROTOCOL_TLS : 0);

    StaticOutStream<65536> stream;
    X224::CR_TPDU_Send(stream, cookie_or_token,
                       (this->tls || this->nla) ? (X224::RDP_NEG_REQ) : (X224::RDP_NEG_NONE),
                       [](bool restricted_admin_mode) -> uint8_t {
                            if (restricted_admin_mode) {
                                LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu: Requiring Restricted Administration mode");
                                return X224::RESTRICTED_ADMIN_MODE_REQUIRED;
                            }

                            return 0;
                       }(this->restricted_admin_mode),
                       rdp_neg_requestedProtocols);
    trans.send(stream.get_produced_bytes());
    LOG_IF(bool(this->verbose & Verbose::negotiation), LOG_INFO, "RdpNego::send_x224_connection_request_pdu: done.");
}

bool RdpNego::enhanced_rdp_security_is_in_effect() const
{
    return (this->enabled_protocols != RdpNegoProtocols::Rdp);
}

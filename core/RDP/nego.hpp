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

#ifndef _REDEMPTION_CORE_RDP_NEGO_HPP_
#define _REDEMPTION_CORE_RDP_NEGO_HPP_
#include "RDP/nla/nla.hpp"
#include "RDP/x224.hpp"
#include "client_info.hpp"
struct RdpNego
{
    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED = 0x01,
    };

    // Protocol Security Negotiation Protocols
    enum RDP_NEG_PROTOCOLS
    {
        PROTOCOL_RDP = 0x00000001,
        PROTOCOL_TLS = 0x00000002,
        PROTOCOL_NLA = 0x00000004
    };

//    int port;
    uint32_t flags;
    bool tls;
    bool nla;
//    char* hostname;
//    char* cookie;

    enum
    {
        NEGO_STATE_INITIAL,
        NEGO_STATE_NLA,  // Network Level Authentication (TLS implicit)
        NEGO_STATE_TLS,  // TLS Encryption without NLA
        NEGO_STATE_RDP,  // Standard Legacy RDP Encryption
        NEGO_STATE_FAIL, // Negotiation failure */
        NEGO_STATE_FINAL
    } state;

//    int tcp_connected;
//    struct rdp_blob
//    {
//        void* data;
//        int length;
//    } * routing_token;

    uint32_t selected_protocol;
    uint32_t requested_protocol;
    uint32_t enabled_protocols;
    char username[128];
    Transport * trans;

    uint8_t hostname[16];
    uint8_t user[128];
    uint8_t password[256];
    uint8_t domain[256];

    TODO("Should not have such variable, but for input/output tests timestamp (and generated nonce) should be static");
    bool test;

    RdpNego(const bool tls, Transport * socket_trans, const char * username, bool nla)
    : flags(0)
    , tls(nla || tls)
    , nla(nla)
    , state(NEGO_STATE_INITIAL)
    , selected_protocol(PROTOCOL_RDP)
    , requested_protocol(PROTOCOL_RDP)
    , trans(socket_trans)
    , test(false)
    {
        if (this->tls){
            this->enabled_protocols = RdpNego::PROTOCOL_RDP
                                    | RdpNego::PROTOCOL_TLS;
            if (this->nla) {
                this->enabled_protocols |= RdpNego::PROTOCOL_NLA;
            }
        }
        else {
            this->enabled_protocols = RdpNego::PROTOCOL_RDP;
        }
        strncpy(this->username, username, 127);
        this->username[127] = 0;

        memset(this->hostname, 0, sizeof(this->hostname));
        memset(this->user, 0, sizeof(this->user));
        memset(this->password, 0, sizeof(this->password));
        memset(this->domain, 0, sizeof(this->domain));
    }

    virtual ~RdpNego() {
    }

    void set_identity(uint8_t * user, uint8_t * domain, uint8_t * pass, uint8_t * hostname) {
        if (this->nla) {
            memcpy(this->user, user, sizeof(this->user) - 1);
            this->user[sizeof(this->user) - 1] = 0;
            memcpy(this->domain, domain, sizeof(this->domain) - 1);
            this->domain[sizeof(this->domain) - 1] = 0;
            memcpy(this->password, pass, sizeof(this->password) - 1);
            this->password[sizeof(this->password) - 1] = 0;
            memcpy(this->hostname, hostname, sizeof(this->hostname) - 1);
            this->hostname[sizeof(this->hostname) - 1] = 0;
        }
    }

    void server_event(bool ignore_certificate_change)
    {
        switch (this->state){
        case NEGO_STATE_INITIAL:
            LOG(LOG_INFO, "RdpNego::NEGO_STATE_INITIAL");
            this->send_negotiation_request();
            if (this->nla) {
                this->state = NEGO_STATE_NLA;
            }
            else if (this->tls){
                this->state = NEGO_STATE_TLS;
            }
            else {
                this->state = NEGO_STATE_RDP;
            }
        break;
        default:
        case NEGO_STATE_NLA:
            LOG(LOG_INFO, "RdpNego::NEGO_STATE_NLA");
            this->recv_connection_confirm(ignore_certificate_change);
        break;
        case NEGO_STATE_TLS:
            LOG(LOG_INFO, "RdpNego::NEGO_STATE_TLS");
            this->recv_connection_confirm(ignore_certificate_change);
        break;
        case NEGO_STATE_RDP:
            LOG(LOG_INFO, "RdpNego::NEGO_STATE_RDP");
            this->recv_connection_confirm(ignore_certificate_change);
        break;
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

    void recv_connection_confirm(bool ignore_certificate_change)
    {
        LOG(LOG_INFO, "RdpNego::recv_connection_confirm");
        BStream stream(65536);
        X224::RecvFactory f(*this->trans, stream);
        X224::CC_TPDU_Recv x224(*this->trans, stream);

        if (x224.rdp_neg_type == 0){
            this->tls = false;
            this->state = NEGO_STATE_FINAL;
            return;
        }
        this->selected_protocol = x224.rdp_neg_code;

        if (this->nla) {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_HYBRID){
                LOG(LOG_INFO, "activating SSL");
                this->trans->enable_client_tls(ignore_certificate_change);
                LOG(LOG_INFO, "activating CREDSSP");
                rdpCredssp credssp(*this->trans,
                                   this->user, this->domain, this->password, this->hostname);
                if (this->test) {
                    credssp.hardcodedtests = true;
                }
                int res = credssp.credssp_client_authenticate();
                if (res != 1) {
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                this->state = NEGO_STATE_FINAL;
            }
            else {
                LOG(LOG_INFO, "Can't activate NLA, falling back to SSL only");
                this->nla = false;
                this->trans->disconnect();
                if (!this->trans->connect()){
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                this->send_negotiation_request();
                this->state = NEGO_STATE_TLS;
                this->enabled_protocols = RdpNego::PROTOCOL_TLS | RdpNego::PROTOCOL_RDP;
            }
        }
        else if (this->tls) {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_TLS){
                LOG(LOG_INFO, "activating SSL");
                this->trans->enable_client_tls(ignore_certificate_change);
                this->state = NEGO_STATE_FINAL;
            }
            else if (x224.rdp_neg_type == X224::RDP_NEG_FAILURE
            && (x224.rdp_neg_code == X224::SSL_NOT_ALLOWED_BY_SERVER
            || x224.rdp_neg_code == X224::SSL_CERT_NOT_ON_SERVER)){
                LOG(LOG_INFO, "Can't activate SSL, falling back to RDP legacy encryption");
                this->tls = false;
                this->trans->disconnect();
                if (!this->trans->connect()){
                    throw Error(ERR_SOCKET_CONNECT_FAILED);
                }
                this->send_negotiation_request();
                this->state = NEGO_STATE_RDP;
                this->enabled_protocols = RdpNego::PROTOCOL_RDP;
            }
            else {
                LOG(LOG_INFO, "Enable NLA is probably required");
                this->trans->disconnect();
                throw Error(ERR_SOCKET_CONNECT_FAILED);
            }
            TODO("Other cases are errors, set an appropriate error message");
        }
        else {
            if (x224.rdp_neg_type == X224::RDP_NEG_RSP
            && x224.rdp_neg_code == X224::PROTOCOL_RDP){
                this->state = NEGO_STATE_FINAL;
            }
            TODO("Check tpdu has no embedded negotiation code");
            this->state = NEGO_STATE_FINAL;
            TODO("Other cases are errors, set an appropriate error message");
        }
        LOG(LOG_INFO, "RdpNego::recv_connection_confirm done");
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

    void send_negotiation_request()
    {
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu");
        BStream stream;
        char cookie[256];
        snprintf(cookie, 256, "Cookie: mstshash=%s\x0D\x0A", this->username);

        uint32_t rdp_neg_requestedProtocols = X224::PROTOCOL_RDP;
        if (this->tls) {
            rdp_neg_requestedProtocols |= X224::PROTOCOL_TLS;
        }
        if (this->nla) {
            rdp_neg_requestedProtocols |= X224::PROTOCOL_HYBRID;
        }

        X224::CR_TPDU_Send(stream, cookie,
            this->tls?(X224::RDP_NEG_REQ):(X224::RDP_NEG_NONE),
            0,
            rdp_neg_requestedProtocols);
        this->trans->send(stream);
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu done");
    }


    void send_negotiation_response()
    {
//        STREAM* s;
//        rdpSettings* settings;
//        int length;
//        uint8 *bm, *em;
//        boolean ret;

//        ret = true;
//        settings = nego->transport->settings;

//        s = transport_send_stream_init(nego->transport, 256);
//        length = TPDU_CONNECTION_CONFIRM_LENGTH;
//        stream_get_mark(s, bm);
//        stream_seek(s, length);

//        if (nego->selected_protocol > PROTOCOL_RDP)
//        {
//            /* RDP_NEG_DATA must be present for TLS and NLA */
//            stream_write_uint8(s, TYPE_RDP_NEG_RSP);
//            stream_write_uint8(s, EXTENDED_CLIENT_DATA_SUPPORTED); /* flags */
//            stream_write_uint16(s, 8); /* RDP_NEG_DATA length (8) */
//            stream_write_uint32(s, nego->selected_protocol); /* selectedProtocol */
//            length += 8;
//        }
//        else if (!settings->rdp_security)
//        {
//            stream_write_uint8(s, TYPE_RDP_NEG_FAILURE);
//            stream_write_uint8(s, 0); /* flags */
//            stream_write_uint16(s, 8); /* RDP_NEG_DATA length (8) */
//            /*
//            * TODO: Check for other possibilities,
//            *       like SSL_NOT_ALLOWED_BY_SERVER.
//            */
//            printf("this->send_negotiation_response: client supports only Standard RDP Security\n");
//            stream_write_uint32(s, SSL_REQUIRED_BY_SERVER);
//            length += 8;
//            ret = false;
//        }

//        stream_get_mark(s, em);
//        stream_set_mark(s, bm);
//        tpkt_write_header(s, length);
//        tpdu_write_connection_confirm(s, length - 5);
//        stream_set_mark(s, em);

//        if (transport_write(nego->transport, s) < 0){
//            return false;
//        }

//        if (ret)
//        {
//            /* update settings with negotiated protocol security */
//            settings->requested_protocols = nego->requested_protocols;
//            settings->selected_protocol = nego->selected_protocol;

//            if (settings->selected_protocol == PROTOCOL_RDP)
//            {
//                settings->tls_security = false;
//                settings->nla_security = false;
//                settings->rdp_security = true;
//                settings->encryption = true;
//                settings->encryption_method = ENCRYPTION_METHOD_40BIT | ENCRYPTION_METHOD_128BIT | ENCRYPTION_METHOD_FIPS;
//                settings->encryption_level = ENCRYPTION_LEVEL_CLIENT_COMPATIBLE;
//            }
//            else if (settings->selected_protocol == PROTOCOL_TLS)
//            {
//                settings->tls_security = true;
//                settings->nla_security = false;
//                settings->rdp_security = false;
//                settings->encryption = false;
//                settings->encryption_method = ENCRYPTION_METHOD_NONE;
//                settings->encryption_level = ENCRYPTION_LEVEL_NONE;
//            }
//            else if (settings->selected_protocol == PROTOCOL_NLA)
//            {
//                settings->tls_security = true;
//                settings->nla_security = true;
//                settings->rdp_security = false;
//                settings->encryption = false;
//                settings->encryption_method = ENCRYPTION_METHOD_NONE;
//                settings->encryption_level = ENCRYPTION_LEVEL_NONE;
//            }
//        }

//        return ret;
    }


    void recv_resquest(const char * certificate_password, ClientInfo & client_info,
                       bool tls_support, bool tls_fallback_legacy) {
        // Receive Request
        {
            BStream stream(65536);
            X224::RecvFactory fac_x224(*this->trans, stream);
            X224::CR_TPDU_Recv x224(*this->trans, stream, false);
            if (x224._header_size != (size_t)(stream.size())){
                LOG(LOG_ERR, "Front::incoming::connection request : all data should have been consumed,"
                    " %d bytes remains", stream.size() - x224._header_size);
            }
            this->requested_protocol = x224.rdp_neg_requestedProtocols;

            if (// Proxy doesnt supports TLS or RDP client doesn't support TLS
                (!tls_support || 0 == (this->requested_protocol & X224::PROTOCOL_TLS))
                // Fallback to legacy security protocol (RDP) is allowed.
                && tls_fallback_legacy) {
                LOG(LOG_INFO, "Fallback to legacy security protocol");
                this->tls = false;
            }
        }

        // Send Response
        {
            BStream stream(256);
            uint8_t rdp_neg_type = 0;
            uint8_t rdp_neg_flags = 0;
            uint32_t rdp_neg_code = 0;
            if (this->nla) {
                LOG(LOG_INFO, "-----------------> Front::NLA Support Enabled");
                if (this->requested_protocol & X224::PROTOCOL_HYBRID) {
                    rdp_neg_type = X224::RDP_NEG_RSP;
                    rdp_neg_code = X224::PROTOCOL_HYBRID;
                    client_info.encryptionLevel = 0;
                }
                else {
                    rdp_neg_type = X224::RDP_NEG_FAILURE;
                    rdp_neg_code = X224::HYBRID_REQUIRED_BY_SERVER;
                    // Fallback to tls ?
                }
            }
            else if (this->tls) {
                LOG(LOG_INFO, "-----------------> Front::TLS Support Enabled");
                if (this->requested_protocol & X224::PROTOCOL_TLS) {
                    rdp_neg_type = X224::RDP_NEG_RSP;
                    rdp_neg_code = X224::PROTOCOL_TLS;
                    client_info.encryptionLevel = 0;
                }
                else {
                    rdp_neg_type = X224::RDP_NEG_FAILURE;
                    rdp_neg_code = X224::SSL_REQUIRED_BY_SERVER;
                }
            }
            else {
                LOG(LOG_INFO, "-----------------> Front::TLS Support not Enabled");
            }

            X224::CC_TPDU_Send x224(stream, rdp_neg_type, rdp_neg_flags, rdp_neg_code);
            this->trans->send(stream);

            if (this->tls){
                this->trans->enable_server_tls(certificate_password);
            }
        }
    }

};


#endif

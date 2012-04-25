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
   Author(s): Christophe Grosjean

   Early Transport Protocol Security Negotiation stage

*/

#ifndef __CORE_RDP_NEGO_HPP__
#define __CORE_RDP_NEGO_HPP__

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

    RdpNego(uint32_t enabled_protocols, Transport * trans, const char * username)
    : flags(0)
    , state(NEGO_STATE_INITIAL)
    , selected_protocol(PROTOCOL_RDP)
    , requested_protocol(PROTOCOL_RDP)
    , enabled_protocols(enabled_protocols)
    , trans(trans)
    {
        strncpy(this->username, username, 127);
        this->username[127] = 0;
    }

    void server_event()
    {
        switch (this->state){
        case NEGO_STATE_INITIAL:
            this->send_negotiation_request();
            this->state = NEGO_STATE_RDP;
        break;
        default:
        case NEGO_STATE_RDP:
            this->recv_connection_confirm();
            this->state = NEGO_STATE_FINAL;
        break;
        }
    }

    void connect()
    {
//        if (nego->state == this->STATE_INITIAL)
//        {
//            if (nego->enabled_protocols[PROTOCOL_NLA] > 0)
//                nego->state = this->STATE_NLA;
//            else if (nego->enabled_protocols[PROTOCOL_TLS] > 0)
//                nego->state = this->STATE_TLS;
//            else if (nego->enabled_protocols[PROTOCOL_RDP] > 0)
//                nego->state = this->STATE_RDP;
//            else
//                nego->state = this->STATE_FAIL;
//        }

//        do
//        {
//            DEBUG_NEGO("state: %s", this->STATE_STRINGS[nego->state]);

//            if (nego->state == this->STATE_NLA){
//                nego->requested_protocols = PROTOCOL_NLA | PROTOCOL_TLS;
//                DEBUG_NEGO("Attempting NLA security");
//            }
//            else if (nego->state == this->STATE_TLS){
//                nego->requested_protocols = PROTOCOL_TLS;
//                DEBUG_NEGO("Attempting TLS security");
//            }
//            else if (nego->state == this->STATE_RDP){
//                nego->requested_protocols = PROTOCOL_RDP;
//                DEBUG_NEGO("Attempting RDP security");
//            }
//            else{
//                DEBUG_NEGO("invalid negotiation state for sending");
//                nego->state = this->STATE_FINAL;
//                return false;
//            }

//            if (nego->state == this->STATE_NLA){
//                nego->requested_protocols = PROTOCOL_NLA | PROTOCOL_TLS;
//                DEBUG_NEGO("Attempting NLA security");
//                DEBUG_NEGO("state: %s", this->STATE_STRINGS[nego->state]);
//                if (nego->state != this->STATE_FINAL)
//                {
//                    this->tcp_disconnect(nego);
//                    if (nego->enabled_protocols[PROTOCOL_TLS] > 0)
//                        nego->state = this->STATE_TLS;
//                    else if (nego->enabled_protocols[PROTOCOL_RDP] > 0)
//                        nego->state = this->STATE_RDP;
//                    else
//                        nego->state = this->STATE_FAIL;
//                }
//            }
//            else if (nego->state == this->STATE_TLS){
//                nego->requested_protocols = PROTOCOL_TLS;
//                DEBUG_NEGO("Attempting TLS security");
//                if (nego->state != this->STATE_FINAL)
//                {
//                    this->tcp_disconnect(nego);

//                    if (nego->enabled_protocols[PROTOCOL_RDP] > 0)
//                        nego->state = this->STATE_RDP;
//                    else
//                        nego->state = this->STATE_FAIL;
//                }
//            }
//            else if (nego->state == this->STATE_RDP){
//                nego->requested_protocols = PROTOCOL_RDP;
//                DEBUG_NEGO("Attempting RDP security");
//            }
//            else{
//                DEBUG_NEGO("invalid negotiation state for sending");
//                nego->state = this->STATE_FINAL;
//                return false;
//            }
//            if (this->tcp_connect(nego))
//            {
//                if (this->send_negotiation_request(nego))
//                {
//                    STREAM* s = transport_recv_stream_init(nego->transport, 1024);
//                    if (transport_read(nego->transport, s) < 0)
//                        nego->state = this->STATE_FAIL;
//                }
//                else
//                {
//                    nego->state = this->STATE_FAIL;
//                }
//            }
//            else
//            {
//                nego->state = this->STATE_FAIL;
//            }



//            if (nego->state == this->STATE_FAIL)
//            {
//                DEBUG_NEGO("Protocol Security Negotiation Failure");
//                nego->state = this->STATE_FINAL;
//                return false;
//            }
//        }
//        while (nego->state != this->STATE_FINAL);

//        DEBUG_NEGO("Negotiated %s security", PROTOCOL_SECURITY_STRINGS[nego->selected_protocol]);

//        /* update settings with negotiated protocol security */
//        nego->transport->settings->requested_protocols = nego->requested_protocols;
//        nego->transport->settings->selected_protocol = nego->selected_protocol;
//        nego->transport->settings->negotiationFlags = nego->flags;

//        if(nego->selected_protocol == PROTOCOL_RDP)
//        {
//            nego->transport->settings->encryption = true;
//            nego->transport->settings->encryption_method = ENCRYPTION_METHOD_40BIT | ENCRYPTION_METHOD_128BIT | ENCRYPTION_METHOD_FIPS;
//            nego->transport->settings->encryption_level = ENCRYPTION_LEVEL_CLIENT_COMPATIBLE;
//        }

//        return true;
    }


    void tcp_connect()
    {
//        if (nego->tcp_connected == 0)
//        {
//            if (transport_connect(nego->transport, nego->hostname, nego->port) == false)
//            {
//                nego->tcp_connected = 0;
//                return false;
//            }
//            else
//            {
//                nego->tcp_connected = 1;
//                return true;
//            }
//        }

//        return true;
    }


    void tcp_disconnect()
    {
//        if (nego->tcp_connected)
//            transport_disconnect(nego->transport);

//        nego->tcp_connected = 0;
//        return 1;
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
// structure. The length of the negotiation structure is included in the X.224
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

    void recv_connection_confirm()
    {
        Stream stream(8192);
        X224In cctpdu(trans, stream);
        if (cctpdu.tpkt.version != 3){
            throw Error(ERR_T123_EXPECTED_TPKT_VERSION_3);
        }
        if (cctpdu.tpdu_hdr.code != X224Packet::CC_TPDU){
            throw Error(ERR_X224_EXPECTED_CONNECTION_CONFIRM);
        }

//        uint8 li;
//        uint8 type;
//        rdpNego* nego = (rdpNego*) extra;

//        if (tpkt_read_header(s) == 0)
//            return false;

//        li = tpdu_read_connection_confirm(s);

//        if (li > 6)
//        {
//            /* rdpNegData (optional) */

//            stream_read_uint8(s, type); /* Type */

//            switch (type)
//            {
//                case TYPE_RDP_NEG_RSP:
//                    this->process_negotiation_response(nego, s);

//                    DEBUG_NEGO("selected_protocol: %d", nego->selected_protocol);

//                    /* enhanced security selected ? */
//                    if (nego->selected_protocol) {
//                        if (nego->selected_protocol == PROTOCOL_NLA &&
//                            !nego->enabled_protocols[PROTOCOL_NLA])
//                            nego->state = this->STATE_FAIL;
//                        if (nego->selected_protocol == PROTOCOL_TLS &&
//                            !nego->enabled_protocols[PROTOCOL_TLS])
//                            nego->state = this->STATE_FAIL;
//                    } else if (!nego->enabled_protocols[PROTOCOL_RDP])
//                        nego->state = this->STATE_FAIL;
//                break;

//                case TYPE_RDP_NEG_FAILURE:
//                    this->process_negotiation_failure(nego, s);
//                break;
//            }
//        }
//        else
//        {
//            DEBUG_NEGO("no rdpNegData");
//            if (!nego->enabled_protocols[PROTOCOL_RDP])
//                nego->state = this->STATE_FAIL;
//            else
//                nego->state = this->STATE_FINAL;
//        }

//        return true;
    }

    void read_request(Stream & stream)
    {
//        uint8 li;
//        uint8 c;
//        uint8 type;

//        tpkt_read_header(s);
//        li = tpdu_read_connection_request(s);

//        if (li != stream_get_left(s) + 6)
//        {
//            printf("Incorrect TPDU length indicator.\n");
//            return false;
//        }

//        if (stream_get_left(s) > 8)
//        {
//            /* Optional routingToken or cookie, ending with CR+LF */
//            while (stream_get_left(s) > 0)
//            {
//                stream_read_uint8(s, c);
//                if (c != '\x0D')
//                    continue;
//                stream_peek_uint8(s, c);
//                if (c != '\x0A')
//                    continue;

//                stream_seek_uint8(s);
//                break;
//            }
//        }

//        if (stream_get_left(s) >= 8)
//        {
//            /* rdpNegData (optional) */

//            stream_read_uint8(s, type); /* Type */
//            if (type != TYPE_RDP_NEG_REQ)
//            {
//                printf("Incorrect negotiation request type %d\n", type);
//                return false;
//            }

//            this->process_negotiation_request(nego, s);
//        }

//        return true;
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
    // token and CR+LF sequence is included in the X.224 Connection Request Length
    // Indicator field. If this field is present, then the cookie field MUST NOT be
    //  present.

    //cookie (variable): An optional and variable-length ANSI text string terminated
    // by a carriage-return (CR) and line-feed (LF) ANSI sequence. This text string
    // MUST be "Cookie: mstshash=IDENTIFIER", where IDENTIFIER is an ANSI string
    //(an example cookie string is shown in section 4.1.1). The length of the entire
    // cookie string and CR+LF sequence is included in the X.224 Connection Request
    // Length Indicator field. This field MUST NOT be present if the routingToken
    // field is present.

    // rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
    // structure. The length of this negotiation structure is included in the X.224
    // Connection Request Length Indicator field.

    void send_negotiation_request()
    {
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu");
        Stream out;
        X224Out crtpdu(X224Packet::CR_TPDU, out);
        crtpdu.stream.out_concat("Cookie: mstshash=");
        crtpdu.stream.out_concat(this->username);
        crtpdu.stream.out_concat("\r\n");
//        crtpdu.stream.out_uint8(0x01);
//        crtpdu.stream.out_uint8(0x00);
//        crtpdu.stream.out_uint32_le(0x00);
        crtpdu.extend_tpdu_hdr();
        crtpdu.end();
        crtpdu.send(this->trans);
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu done");

//        STREAM* s;
//        int length;
//        uint8 *bm, *em;

//        s = transport_send_stream_init(nego->transport, 256);
//        length = TPDU_CONNECTION_REQUEST_LENGTH;
//        stream_get_mark(s, bm);
//        stream_seek(s, length);

//        if (nego->routing_token != NULL)
//        {
//            stream_write(s, nego->routing_token->data, nego->routing_token->length);
//            length += nego->routing_token->length;
//        }
//        else if (nego->cookie != NULL)
//        {
//            int cookie_length = strlen(nego->cookie);
//            stream_write(s, "Cookie: mstshash=", 17);
//            stream_write(s, (uint8*) nego->cookie, cookie_length);
//            stream_write_uint8(s, 0x0D); /* CR */
//            stream_write_uint8(s, 0x0A); /* LF */
//            length += cookie_length + 19;
//        }

//        DEBUG_NEGO("requested_protocols: %d", nego->requested_protocols);
//        if (nego->requested_protocols > PROTOCOL_RDP)
//        {
//            /* RDP_NEG_DATA must be present for TLS and NLA */
//            stream_write_uint8(s, TYPE_RDP_NEG_REQ);
//            stream_write_uint8(s, 0); /* flags, must be set to zero */
//            stream_write_uint16(s, 8); /* RDP_NEG_DATA length (8) */
//            stream_write_uint32(s, nego->requested_protocols); /* requestedProtocols */
//            length += 8;
//        }

//        stream_get_mark(s, em);
//        stream_set_mark(s, bm);
//        tpkt_write_header(s, length);
//        tpdu_write_connection_request(s, length - 5);
//        stream_set_mark(s, em);

//        if (transport_write(nego->transport, s) < 0)
//            return false;

//        return true;
    }


    void process_negotiation_request(Stream & stream)
    {
//        uint8 flags;
//        uint16 length;

//        DEBUG_NEGO("RDP_NEG_REQ");

//        stream_read_uint8(s, flags);
//        stream_read_uint16(s, length);
//        stream_read_uint32(s, nego->requested_protocols);

//        nego->state = this->STATE_FINAL;
    }


    void process_negotiation_response(Stream & stream)
    {
//        uint16 length;

//        DEBUG_NEGO("RDP_NEG_RSP");

//        stream_read_uint8(s, nego->flags);
//        stream_read_uint16(s, length);
//        stream_read_uint32(s, nego->selected_protocol);

//        nego->state = this->STATE_FINAL;
    }

    void process_negotiation_failure(Stream & stream)
    {
//        uint8 flags;
//        uint16 length;
//        uint32 failureCode;

//        DEBUG_NEGO("RDP_NEG_FAILURE");

//        stream_read_uint8(s, flags);
//        stream_read_uint16(s, length);
//        stream_read_uint32(s, failureCode);

//        switch (failureCode)
//        {
//        case SSL_REQUIRED_BY_SERVER:
//            DEBUG_NEGO("Error: SSL_REQUIRED_BY_SERVER");
//        break;
//        case SSL_NOT_ALLOWED_BY_SERVER:
//            DEBUG_NEGO("Error: SSL_NOT_ALLOWED_BY_SERVER");
//        break;
//        case SSL_CERT_NOT_ON_SERVER:
//            DEBUG_NEGO("Error: SSL_CERT_NOT_ON_SERVER");
//        break;
//        case INCONSISTENT_FLAGS:
//            DEBUG_NEGO("Error: INCONSISTENT_FLAGS");
//        break;
//        case HYBRID_REQUIRED_BY_SERVER:
//            DEBUG_NEGO("Error: HYBRID_REQUIRED_BY_SERVER");
//        break;
//        default:
//            DEBUG_NEGO("Error: Unknown protocol security error %d", failureCode);
//        break;
//        }

//        nego->state = this->STATE_FAIL;
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


};


#endif

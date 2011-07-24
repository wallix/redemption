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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   mcs layer at rdp module

*/
#if !defined(__RDP_MCS_HPP__)
#define __RDP_MCS_HPP__

#include "x224.hpp"

#include <iostream>
#include <vector>
using namespace std;


/* mcs */
struct rdp_mcs {
    Transport * trans;
    int userid;
    vector<struct mcs_channel_item *> channel_list;

    rdp_mcs(Transport * trans) : trans(trans), userid(1)
    {
    }

    ~rdp_mcs()
    {
    }

//2.2.1.1    Client X.224 Connection Request PDU
//==============================================

// The X.224 Connection Request PDU is an RDP Connection Sequence PDU sent from
// client to server during the Connection Initiation phase (see section
// 1.3.1.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Crq (7 bytes): An X.224 Class 0 Connection Request transport protocol
//   data unit (TPDU), as specified in [X224] section 13.3.

// routingToken (variable): Optional and variable-length routing token bytes
//   used for load balancing terminated by a carriage-return (CR) and line-feed
//   (LF) ANSI sequence. For more information, see [MSFT-SDLBTS]. The length of
//   the routing token and CR+LF sequence is included in the X.224 Connection
//   Request Length Indicator field.

// rdpNegData (8 bytes): An optional RDP Negotiation Request (section 2.2.1.1.1)
//   structure. The length of this negotiation structure is included in the
//   X.224 Connection Request Length Indicator field.

// 2.2.1.1.1   RDP Negotiation Request (RDP_NEG_REQ)
// =================================================
//  The RDP Negotiation Request structure is used by a client to advertise the
//  security protocols which it supports.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x01 (TYPE_RDP_NEG_REQ) to indicate that the packet
//   is a Negotiation Request.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size.
//   This field MUST be set to 0x0008 (8 bytes).

// requestedProtocols (4 bytes): A 32-bit, unsigned integer. Flags indicating
//   the supported security protocols.

// +---------------------------------+-----------------------------------------+
// | 0x00000000 PROTOCOL_RDP_FLAG    |  Legacy RDP encryption.                 |
// +---------------------------------+-----------------------------------------+
// | 0x00000001 PROTOCOL_SSL_FLAG    | TLS 1.0 (section 5.4.5.1).              |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID_FLAG | Credential Security Support Provider    |
// |                                 | protocol (CredSSP) (section 5.4.5.2).   |
// |                                 | If this flag is set, then the           |
// |                                 | PROTOCOL_SSL_FLAG (0x00000001) SHOULD   |
// |                                 | also be set because Transport Layer     |
// |                                 | Security (TLS) is a subset of CredSSP.  |
// +---------------------------------+-----------------------------------------+

// 2.2.1.2   Server X.224 Connection Confirm PDU
// =============================================

//  The X.224 Connection Confirm PDU is an RDP Connection Sequence PDU sent from
//  server to client during the Connection Initiation phase (see section
//  1.3.1.1). It is sent as a response to the X.224 Connection Request PDU
//  (section 2.2.1.1).

//tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//x224Ccf (7 bytes): An X.224 Class 0 Connection Confirm TPDU, as specified in [X224] section
//   13.4.

//rdpNegData (8 bytes): Optional RDP Negotiation Response (section 2.2.1.2.1) structure or an
//   optional RDP Negotiation Failure (section 2.2.1.2.2) structure. The length of the negotiation
//   structure is included in the X.224 Connection Confirm Length Indicator field.

// 2.2.1.2.1   RDP Negotiation Response (RDP_NEG_RSP)
// ==================================================

//  The RDP Negotiation Response structure is used by a server to inform the
//  client of the security protocol which it has selected to use for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This field MUST be set to
//   0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +--------------------------------+------------------------------------------+
// | EXTENDED_CLIENT_DATA_SUPPORTED | The server supports extended client data |
// | 0x00000001                     | blocks in the GCC Conference Create      |
// |                                | Request user data (section 2.2.1.3).     |
// +--------------------------------+------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the
//   selected security protocol.

// +---------------------------------------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Legacy RDP encryption                        |
// +---------------------------------------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +---------------------------------------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +---------------------------------------------------------------------------+

// 2.2.1.2.2   RDP Negotiation Failure (RDP_NEG_FAILURE)
// =====================================================

//  The RDP Negotiation Failure structure is used by a server to inform the
//  client of a failure that has occurred while preparing security for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the
//   packet is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
//   failure code.

// +---------------------------+-----------------------------------------------+
// | SSL_REQUIRED_BY_SERVER    | The server requires that the client support   |
// | 0x00000001                | Enhanced RDP Security (section 5.4) with      |
// |                           | either TLS 1.0 (section 5.4.5.1) or CredSSP   |
// |                           | (section 5.4.5.2). If only CredSSP was        |
// |                           | requested then the server only supports TLS.  |
// +---------------------------+-----------------------------------------------+
// | SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only use Standard |
// | 0x00000002                | RDP Security mechanisms (section 5.3) and     |
// |                           | does not support any External                 |
// |                           | Security Protocols (section 5.4.5).           |
// +---------------------------+-----------------------------------------------+
// | SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid server    |
// | 0x00000003                | authentication certificate and cannot         |
// |                           | initialize the External Security Protocol     |
// |                           | Provider (section 5.4.5).                     |
// +---------------------------+-----------------------------------------------+
// | INCONSISTENT_FLAGS        | The list of requested security protocols is   |
// | 0x00000004                | not consistent with the current security      |
// |                           | protocol in effect. This error is only        |
// |                           | possible when the Direct Approach (see        |
// |                           | sections 5.4.2.2 and 1.3.1.2) is used and an  |
// |                           | External Security Protocol (section 5.4.5) is |
// |                           | already being used.                           |
// +---------------------------+-----------------------------------------------+
// | HYBRID_REQUIRED_BY_SERVER | The server requires that the client support   |
// | 0x00000005                | Enhanced RDP Security (section 5.4) with      |
// |                           | CredSSP (section 5.4.5.2).                    |
// +---------------------------+-----------------------------------------------+



    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_recv(Stream & stream, int& chan) throw(Error)
    {
        stream.init(65535);
        X224In(this->trans, stream);
        int opcode = stream.in_uint8();
        int appid = opcode >> 2;
        if (appid != MCS_SDIN) {
            throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
        }
        stream.skip_uint8(2);
        chan = stream.in_uint16_be();
        stream.skip_uint8(1);
        int len = stream.in_uint8();
        if (len & 0x80) {
            stream.skip_uint8(1);
        }
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_ber_out_header(Stream & stream, int tag_val, int len)
    {
        if (tag_val > 0xff) {
            stream.out_uint16_be( tag_val);
        } else {
            stream.out_uint8( tag_val);
        }
        if (len >= 0x80) {
            stream.out_uint8( 0x82);
            stream.out_uint16_be( len);
        } else {
            stream.out_uint8( len);
        }
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_ber_out_int16(Stream & stream, int value)
    {
        this->rdp_mcs_ber_out_header(stream, BER_TAG_INTEGER, 2);
        stream.out_uint8( (value >> 8));
        stream.out_uint8( value);
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_out_domain_params(Stream & stream, int max_channels,
                          int max_users, int max_tokens, int max_pdu_size)
    {
        this->rdp_mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 32);
        this->rdp_mcs_ber_out_int16(stream, max_channels);
        this->rdp_mcs_ber_out_int16(stream, max_users);
        this->rdp_mcs_ber_out_int16(stream, max_tokens);
        this->rdp_mcs_ber_out_int16(stream, 1);
        this->rdp_mcs_ber_out_int16(stream, 0);
        this->rdp_mcs_ber_out_int16(stream, 1);
        this->rdp_mcs_ber_out_int16(stream, max_pdu_size);
        this->rdp_mcs_ber_out_int16(stream, 2);
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_send_connection_initial(Stream & client_mcs_data) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = client_mcs_data.end - client_mcs_data.data;
        int len = 7 + 3 * 34 + 4 + data_len;
        this->rdp_mcs_ber_out_header(stream, MCS_CONNECT_INITIAL, len);
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* calling domain */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* called domain */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_BOOLEAN, 1);
        stream.out_uint8(0xff); /* upward flag */
        this->rdp_mcs_out_domain_params(stream, 34, 2, 0, 0xffff); /* target params */
        this->rdp_mcs_out_domain_params(stream, 1, 1, 1, 0x420); /* min params */
        this->rdp_mcs_out_domain_params(stream, 0xffff, 0xfc17, 0xffff, 0xffff); /* max params */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        stream.out_copy_bytes(client_mcs_data.data, data_len);

        tpdu.end();
        tpdu.send(this->trans);
    }

    int ber_parse_header(Stream & stream, int tag_val) throw(Error)
    {

        #warning this should be some kind of check val stream primitive
        int tag = 0;
        if (tag_val > 0xff) {
            tag = stream.in_uint16_be();
        }
        else {
            tag = stream.in_uint8();
        }
        if (tag != tag_val) {
            throw Error(ERR_MCS_BER_PARSE_HEADER_VAL_NOT_MATCH);
        }
        #warning seems to be some kind of multi bytes read. Use explicit primitive in stream.
        int l = stream.in_uint8();
        int len = l;
        if (l & 0x80) {
            len = 0;
            for (l = l & ~0x80; l > 0 ; l--) {
                len = (len << 8) | stream.in_uint8();
            }
        }
        #warning we should change check behavior here and check before accessing data, not after, use check_rem
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_PARSE_HEADER_ERROR_CHECKING_STREAM);
        }
        return len;
    }

    void rdp_mcs_send_edrq() throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8( (MCS_EDRQ << 2));
        stream.out_uint16_be(0x100); /* height */
        stream.out_uint16_be(0x100); /* interval */

        tpdu.end();
        tpdu.send(this->trans);
    }


    void rdp_mcs_send_aurq() throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_AURQ << 2));
        stream.mark_end();

        tpdu.end();
        tpdu.send(this->trans);
    }

    void rdp_mcs_recv_aucf() throw(Error)
    {
        Stream stream(8192);
        X224In(this->trans, stream);
        int opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_AUCF) {
            throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
        }
        int res = stream.in_uint8();
        if (res != 0) {
            throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
        }
        if (opcode & 2) {
            this->userid = stream.in_uint16_be();
        }
        if (!(stream.check_end())) {
            throw Error(ERR_MCS_RECV_AUCF_ERROR_CHECKING_STREAM);
        }
    }


    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_send_cjrq(int chanid) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_CJRQ << 2));
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chanid);

        tpdu.end();
        tpdu.send(this->trans);
    }


    /*****************************************************************************/
    /* returns error : channel join confirm */
    void rdp_mcs_recv_cjcf() throw(Error)
    {
        int opcode;
        Stream stream(8192);
        X224In(this->trans, stream);
        opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_CJCF) {
            throw Error(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF);
        }
        if (0 != stream.in_uint8()) {
            throw Error(ERR_MCS_RECV_CJCF_EMPTY);
        }
        stream.skip_uint8(4); /* mcs_userid, req_chanid */
        if (opcode & 2) {
            stream.skip_uint8(2); /* join_chanid */
        }
        if (!stream.check_end()) {
            throw Error(ERR_MCS_RECV_CJCF_ERROR_CHECKING_STREAM);
        }
    }

    /* Send an MCS transport data packet to a specific channel */
    void rdp_mcs_send_to_channel(Stream & stream, int chan_id)
    {
        stream.p = stream.mcs_hdr;
        int len = ((stream.end - stream.p) - 8) | 0x8000;
        stream.out_uint8(MCS_SDRQ << 2);
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chan_id);
        stream.out_uint8(0x70);
        stream.out_uint16_be(len);
    }
};

#endif

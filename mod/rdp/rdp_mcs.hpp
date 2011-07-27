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

#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"

#include <iostream>
#include <vector>
using namespace std;


/* mcs */
struct rdp_mcs : public Mcs {

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

    void rdp_mcs_recv(Transport * trans, Stream & stream, int& chan) throw(Error)
    {
        stream.init(65535);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In(trans, stream);

        int opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_SDIN) {
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

    void rdp_mcs_send_edrq(Transport * trans) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8( (MCS_EDRQ << 2));
        stream.out_uint16_be(0x100); /* height */
        stream.out_uint16_be(0x100); /* interval */

        tpdu.end();
        tpdu.send(trans);
    }


    void rdp_mcs_send_aurq(Transport * trans) throw (Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_AURQ << 2));
        stream.mark_end();

        tpdu.end();
        tpdu.send(trans);
    }

    void rdp_mcs_recv_aucf(Transport * trans) throw(Error)
    {
        Stream stream(8192);
        X224In(trans, stream);
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
    void rdp_mcs_send_cjrq(Transport * trans, int chanid) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_CJRQ << 2));
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chanid);

        tpdu.end();
        tpdu.send(trans);
    }

    /* returns error : channel join confirm */
    void rdp_mcs_recv_cjcf(Transport * trans) throw(Error)
    {
        Stream stream(8192);
        X224In(trans, stream);
        int opcode = stream.in_uint8();
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
};

#endif

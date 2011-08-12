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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   mcs layer at core module

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

/* used in mcs */
struct mcs_channel_item {
    char name[16];
    int flags;
    int chanid;
    mcs_channel_item(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

struct Mcs {

    int userid;
    vector<struct mcs_channel_item *> channel_list;

    Mcs() : userid(1)
    {
    }

    ~Mcs()
    {
        int count = (int) this->channel_list.size();
        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                delete channel_item;
            }
        }
    }

// 2.2.1.3 Client MCS Connect Initial PDU with GCC Conference Create Request
// =========================================================================

// The MCS Connect Initial PDU is an RDP Connection Sequence PDU sent from
// client to server during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent after receiving the X.224 Connection Confirm PDU
// (section 2.2.1.2). The MCS Connect Initial PDU encapsulates a GCC Conference
// Create Request, which encapsulates concatenated blocks of settings data. A
// basic high-level overview of the nested structure for the Client MCS Connect
// Initial PDU is illustrated in section 1.3.1.1, in the figure specifying MCS
// Connect Initial PDU. Note that the order of the settings data blocks is
// allowed to vary from that shown in the previously mentioned figure and the
// message syntax layout that follows. This is possible because each data block
// is identified by a User Data Header structure (section 2.2.1.3.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//   section 13.7.

// mcsCi (variable): Variable-length BER-encoded MCS Connect Initial structure
//   (using definite-length encoding) as described in [T125] (the ASN.1
//   structure definition is detailed in [T125] section 7, part 2). The userData
//   field of the MCS Connect Initial encapsulates the GCC Conference Create
//   Request data (contained in the gccCCrq and subsequent fields). The maximum
//   allowed size of this user data is 1024 bytes, which implies that the
//   combined size of the gccCCrq and subsequent fields MUST be less than 1024
//   bytes.

// gccCCrq (variable): Variable-length Packed Encoding Rule encoded
//   (PER-encoded) GCC Connect Data structure, which encapsulates a Connect GCC
//   PDU that contains a GCC Conference Create Request structure as described in
//   [T124] (the ASN.1 structure definitions are detailed in [T124] section 8.7)
//   appended as user data to the MCS Connect Initial (using the format
//   described in [T124] sections 9.5 and 9.6). The userData field of the GCC
//   Conference Create Request contains one user data set consisting of
//   concatenated client data blocks.

// clientCoreData (216 bytes): Client Core Data structure (section 2.2.1.3.2).

// clientSecurityData (12 bytes): Client Security Data structure (section
//   2.2.1.3.3).

// clientNetworkData (variable): Optional and variable-length Client Network
//   Data structure (section 2.2.1.3.4).

// clientClusterData (12 bytes): Optional Client Cluster Data structure (section
//   2.2.1.3.5).

// clientMonitorData (variable): Optional Client Monitor Data structure (section
//   2.2.1.3.6). This field MUST NOT be included if the server does not
//   advertise support for extended client data blocks by using the
//   EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section
//   2.2.1.2.1.

    void mcs_recv_connection_initial(Stream & data, Transport * trans)
    {
        Stream stream(8192);
        X224In(trans, stream);

        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();
        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);
        if (stream.in_uint8() != BER_TAG_BOOLEAN) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        /* make a copy of client mcs data */
        data.init(len);
        data.out_copy_bytes(stream.p, len);
        data.mark_end();
        stream.skip_uint8(len);
    }

    void mcs_send_connection_initial(Stream & client_mcs_data, Transport * trans) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = client_mcs_data.end - client_mcs_data.data;
        int len = 7 + 3 * 34 + 4 + data_len;
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_INITIAL);
        stream.out_ber_len(len);
        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(0); /* calling domain */
        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(0); /* called domain */
        stream.out_uint8(BER_TAG_BOOLEAN);
        stream.out_ber_len(1);
        stream.out_uint8(0xff); /* upward flag */

        // target params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(34);     // max_channels
        stream.out_ber_int16(2);      // max_users
        stream.out_ber_int16(0);      // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0xffff); // max_pdu_size
        stream.out_ber_int16(2);

        // min params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(1);     // max_channels
        stream.out_ber_int16(1);     // max_users
        stream.out_ber_int16(1);     // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0x420); // max_pdu_size
        stream.out_ber_int16(2);

        // max params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(0xffff); // max_channels
        stream.out_ber_int16(0xfc17); // max_users
        stream.out_ber_int16(0xffff); // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0xffff); // max_pdu_size
        stream.out_ber_int16(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(data_len);
        stream.out_copy_bytes(client_mcs_data.data, data_len);

        tpdu.end();
        tpdu.send(trans);
    }

    void mcs_recv_connect_response(Stream & stream, Transport * trans) throw(Error)
    {
        X224In(trans, stream);
        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_RESPONSE) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();

        if (stream.in_uint8() != BER_TAG_RESULT) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        int res = stream.in_uint8();

        if (res != 0) {
            throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
        }
        if (stream.in_uint8() != BER_TAG_INTEGER) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len); /* connect id */

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
    }

    void mcs_send_connect_response(Stream & data, Transport * trans) throw(Error)
    {
//        LOG(LOG_INFO, .mcs_send_connect_response");
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = data.end - data.data;
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        stream.out_ber_len(data_len + 38);

        stream.out_uint8(BER_TAG_RESULT);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_uint8(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_uint8(26);
        stream.out_ber_int8(22); // max_channels
        stream.out_ber_int8(3); // max_users
        stream.out_ber_int8(0); // max_tokens
        stream.out_ber_int8(1);
        stream.out_ber_int8(0);
        stream.out_ber_int8(1);
        stream.out_ber_int24(0xfff8); // max_pdu_size
        stream.out_ber_int8(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(data_len);
        /* mcs data */
        stream.out_copy_bytes(data.data, data_len);

        tpdu.end();
        tpdu.send(trans);
    }

    //   2.2.1.5 Client MCS Erect Domain Request PDU
    //   -------------------------------------------
    //   The MCS Erect Domain Request PDU is an RDP Connection Sequence PDU sent
    //   from client to server during the Channel Connection phase (see section
    //   1.3.1.1). It is sent after receiving the MCS Connect Response PDU (section
    //   2.2.1.4).

    //   tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    //   x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
    //      section 13.7.

    // See description of tpktHeader and x224 Data TPDU in cheat sheet

    //   mcsEDrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an MCS
    //      Erect Domain Request structure, as specified in [T125] (the ASN.1
    //      structure definitions are given in [T125] section 7, parts 3 and 10).

    void mcs_recv_edrq(Transport * trans)
    {
        Stream stream(8192);
        X224In(trans, stream);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_EDRQ) {
            throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
        }
        stream.skip_uint8(2);
        stream.skip_uint8(2);
        if (opcode & 2) {
            this->userid = stream.in_uint16_be();
        }
        if (!stream.check_end()) {
            throw Error(ERR_MCS_RECV_EDQR_TRUNCATED);
        }
    }

    // 2.2.1.6 Client MCS Attach User Request PDU
    // ------------------------------------------
    // The MCS Attach User Request PDU is an RDP Connection Sequence PDU
    // sent from client to server during the Channel Connection phase (see
    // section 1.3.1.1) to request a user channel ID. It is sent after
    // transmitting the MCS Erect Domain Request PDU (section 2.2.1.5).

    // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

    // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
    //   [X224] section 13.7.

    // See description of tpktHeader and x224 Data TPDU in cheat sheet

    // mcsAUrq (1 byte): PER-encoded MCS Domain PDU which encapsulates an
    //  MCS Attach User Request structure, as specified in [T125] (the ASN.1
    //  structure definitions are given in [T125] section 7, parts 5 and 10).

    void mcs_recv_aurq(Transport * trans)
    {
        Stream stream(8192);
        X224In(trans, stream);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_AURQ) {
            throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
        }
        if (opcode & 2) {
            this->userid = stream.in_uint16_be();
        }
        if (!stream.check_end()) {
            throw Error(ERR_MCS_RECV_AURQ_TRUNCATED);
        }
    }

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

    void mcs_recv(Transport * trans, Stream & stream, int& chan) throw(Error)
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

    void mcs_recv_aucf(Transport * trans) throw(Error)
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


    /* returns error : channel join confirm */
    void mcs_recv_cjcf(Transport * trans) throw(Error)
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

    void mcs_recv_cjrq(Transport * trans) throw(Error)
    {
        Stream stream(8192);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In(trans, stream);

        int opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_CJRQ) {
            throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
        }
        stream.skip_uint8(4);
        if (opcode & 2) {
            stream.skip_uint8(2);
        }
    }

    void mcs_send_cjcf(Transport * trans, int userid, int chanid) throw(Error)
    {
//        LOG(LOG_INFO, .mcs_send_cjcf");

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_CJCF << 2) | 2);
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.out_uint16_be(chanid);
        stream.out_uint16_be(chanid);

        tpdu.end();
        tpdu.send(trans);
    }


    void mcs_disconnect(Transport * trans) throw (Error)
    {
//        LOG(LOG_INFO, "mcs_disconnect");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(trans);
    }

    /* returns a zero based index of the channel,
      -1 if error or if it dosen't exist */
    int mcs_get_channel_id(const char * name)
    {
//        LOG(LOG_INFO, "mcs_get_channel_id");

        int rv = -1;

        int count = (int) this->channel_list.size();

        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                if (0 == strcasecmp(name, channel_item->name)) {
                    rv = index;
                    break;
                }
            }
        }
        return rv;
    }

};


struct McsOut
{
    Stream & stream;
    X224Out tpdu;

    McsOut(uint8_t pdutype, Stream & stream) 
        : stream(stream), tpdu(X224Packet::DT_TPDU, stream)
    {
        switch(pdutype){
        case MCS_EDRQ:
            stream.out_uint8((MCS_EDRQ << 2));
            stream.out_uint16_be(0x100); /* height */
            stream.out_uint16_be(0x100); /* interval */
        break;

        // 2.2.1.7 Server MCS Attach User Confirm PDU
        // ------------------------------------------
        // The MCS Attach User Confirm PDU is an RDP Connection Sequence
        // PDU sent from server to client during the Channel Connection
        // phase (see section 1.3.1.1). It is sent as a response to the MCS
        // Attach User Request PDU (section 2.2.1.6).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
        //   section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //   section [X224] 13.7.

        // mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
        //   an MCS Attach User Confirm structure, as specified in [T125]
        //   (the ASN.1 structure definitions are given in [T125] section 7,
        // parts 5 and 10).
        case MCS_AUCF:
            stream.out_uint8(((MCS_AUCF << 2) | 2));
        break;


        case MCS_AURQ:
            stream.out_uint8((MCS_AURQ << 2));
        break;

        // 2.2.1.8 Client MCS Channel Join Request PDU
        // -------------------------------------------
        // The MCS Channel Join Request PDU is an RDP Connection Sequence PDU sent
        // from client to server during the Channel Connection phase (see section
        // 1.3.1.1). It is sent after receiving the MCS Attach User Confirm PDU
        // (section 2.2.1.7). The client uses the MCS Channel Join Request PDU to
        // join the user channel obtained from the Attach User Confirm PDU, the
        // I/O channel and all of the static virtual channels obtained from the
        // Server Network Data structure (section 2.2.1.4.4).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
        //                     section 13.7.

        // mcsCJrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an
        //                    MCS Channel Join Request structure as specified in
        //                    [T125] sections 10.19 and I.3 (the ASN.1 structure
        //                    definitions are given in [T125] section 7, parts 6
        //                    and 10).

        // ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
        // {
        //     initiator UserId
        //     channelId ChannelId
        //               -- may be zero
        // }
        case MCS_CJRQ:
            stream.out_uint8((MCS_CJRQ << 2));
        break;

        // 2.2.1.9 Server MCS Channel Join Confirm PDU
        // -------------------------------------------
        // The MCS Channel Join Confirm PDU is an RDP Connection Sequence
        // PDU sent from server to client during the Channel Connection
        // phase (see section 1.3.1.1). It is sent as a response to the MCS
        // Channel Join Request PDU (section 2.2.1.8).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
        //   section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //  [X224] section 13.7.

        // mcsCJcf (8 bytes): PER-encoded MCS Domain PDU which encapsulates
        //  an MCS Channel Join Confirm PDU structure, as specified in
        //  [T125] (the ASN.1 structure definitions are given in [T125]
        //  section 7, parts 6 and 10).
        case MCS_CJCF:
            stream.out_uint8((MCS_CJCF << 2) | 2);
        break;

        default:
        break;
        }
    }

    void end()
    {
        this->tpdu.end();
    }

    void send(Transport * trans)
    {
        this->tpdu.send(trans);
    }

};

#endif

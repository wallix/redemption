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

   MCS Channel Management, complies with T.125

*/

#if !defined(__CORE_RDP_MCS_HPP__)
#define __CORE_RDP_MCS_HPP__

#include "client_info.hpp"
#include "RDP/x224.hpp"
#include "RDP/gcc.hpp"
#include "RDP/gcc_conference_user_data/cs_core.hpp"
#include "RDP/gcc_conference_user_data/cs_cluster.hpp"
#include "RDP/gcc_conference_user_data/cs_sec.hpp"
#include "RDP/gcc_conference_user_data/cs_net.hpp"
#include "RDP/gcc_conference_user_data/cs_monitor.hpp"
#include "RDP/gcc_conference_user_data/sc_core.hpp"
#include "RDP/gcc_conference_user_data/sc_sec1.hpp"
#include "RDP/gcc_conference_user_data/sc_net.hpp"
#include "channel_list.hpp"
#include "genrandom.hpp"

enum {
    MCS_EDRQ =  1, /* Erect Domain Request */
    MCS_DPUM =  8, /* Disconnect Provider Ultimatum */
    MCS_AURQ = 10, /* Attach User Request */
    MCS_AUCF = 11, /* Attach User Confirm */
    MCS_CJRQ = 14, /* Channel Join Request */
    MCS_CJCF = 15, /* Channel Join Confirm */
    MCS_SDRQ = 25, /* Send Data Request */
    MCS_SDIN = 26, /* Send Data Indication */
};

class McsOut
{
    Stream & stream;
    uint8_t offlen;
    public:
    McsOut(Stream & stream, uint8_t command, uint8_t user_id, uint16_t chan_id)
        : stream(stream), offlen(stream.get_offset(-6))
    {
        stream.out_uint8(command << 2);
        stream.out_uint16_be(user_id);
        stream.out_uint16_be(chan_id);
        stream.out_uint8(0x70);
        stream.out_uint16_be(0); // skip len
    }

    void end(){
        int len = stream.get_offset(offlen + 2);
        stream.set_out_uint16_be(0x8000|len, this->offlen);
    }
};


class McsIn
{
    Stream & stream;
    public:
    uint8_t opcode;
    uint16_t user_id;
    uint16_t chan_id;
    uint8_t magic_0x70; // some ber header ?
    uint16_t len;

    McsIn(Stream & stream)
        : stream(stream), opcode(0), user_id(0), chan_id(0), len(0)
    {
        this->opcode = stream.in_uint8();
        this->user_id = stream.in_uint16_be();
        this->chan_id = stream.in_uint16_be();
        this->magic_0x70 = stream.in_uint8();
        this->len = stream.in_uint8();
        if (this->len & 0x80){
            this->len = ((this->len & 0x7F) << 8) + stream.in_uint8();
        }
    }

    void end(){
        if (this->stream.p != this->stream.end){
            LOG(LOG_ERR, "all data should have been consumed : remains %d", stream.end - stream.p);
        }
    }

};


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


static inline void mcs_send_connect_initial(
            Transport * trans,
            const ChannelList & channel_list,
            const uint16_t front_width,
            const uint16_t front_height,
            const uint8_t front_bpp,
            int keylayout,
            char * hostname,
            const int use_rdp5,
            const bool console_session){

//    Stream data(8192);

//    int data_len = data.end - data.data;
//    int len = 7 + 3 * 34 + 4 + data_len;

    Stream stream(32768);
    X224Out ci_tpdu(X224Packet::DT_TPDU, stream);

    stream.out_uint16_be(BER_TAG_MCS_CONNECT_INITIAL);
    uint32_t offset_data_len_connect_initial = stream.get_offset(0);
    stream.out_ber_len_uint16(0); // filled later, 3 bytes

    stream.out_uint8(BER_TAG_OCTET_STRING);
    stream.out_ber_len(1); /* calling domain */
    stream.out_uint8(1);
    stream.out_uint8(BER_TAG_OCTET_STRING);
    stream.out_ber_len(1); /* called domain */
    stream.out_uint8(1);
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
    uint32_t offset_data_len = stream.get_offset(0);
    stream.out_ber_len_uint16(0); // filled later, 3 bytes

    /* Generic Conference Control (T.124) ConferenceCreateRequest */

    size_t offset_gcc_conference_create_request_header_length = 0;
    gcc_write_conference_create_request_header(stream, offset_gcc_conference_create_request_header_length);    

    size_t offset_user_data_length = stream.get_offset(0);
    stream.out_per_length(256); // remaining length, reserve 16 bits

    // Client User Data
    // ================
    // 158 bytes
    mod_rdp_out_cs_core(stream, use_rdp5, front_width, front_height, front_bpp, keylayout, hostname);
    // 76 bytes
    mod_rdp_out_cs_cluster(stream, console_session);
    // 12 bytes
    mod_rdp_out_cs_sec(stream);
    // 12 * nbchan + 8 bytes
    mod_rdp_out_cs_net(stream, channel_list);

    stream.set_out_per_length(stream.get_offset(offset_user_data_length + 2), offset_user_data_length); // user data length
    stream.set_out_per_length(stream.get_offset(offset_gcc_conference_create_request_header_length + 2), offset_gcc_conference_create_request_header_length); // length including header

    // set mcs_data len, BER_TAG_OCTET_STRING (some kind of BLOB)
    stream.set_out_ber_len_uint16(stream.get_offset(offset_data_len + 3), offset_data_len);

    // set mcs_data len for BER_TAG_MCS_CONNECT_INITIAL
    stream.set_out_ber_len_uint16(stream.get_offset(offset_data_len_connect_initial + 3), offset_data_len_connect_initial);

    ci_tpdu.end();
    ci_tpdu.send(trans);
}

// 2.2.1.4  Server MCS Connect Response PDU with GCC Conference Create Response
// ----------------------------------------------------------------------------

// From [MSRDPCGR]

// The MCS Connect Response PDU is an RDP Connection Sequence PDU sent from
// server to client during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent as a response to the MCS Connect Initial PDU (section
// 2.2.1.3). The MCS Connect Response PDU encapsulates a GCC Conference Create
// Response, which encapsulates concatenated blocks of settings data.

// A basic high-level overview of the nested structure for the Server MCS
// Connect Response PDU is illustrated in section 1.3.1.1, in the figure
// specifying MCS Connect Response PDU. Note that the order of the settings
// data blocks is allowed to vary from that shown in the previously mentioned
// figure and the message syntax layout that follows. This is possible because
// each data block is identified by a User Data Header structure (section
// 2.2.1.4.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
// section 13.7.

// mcsCrsp (variable): Variable-length BER-encoded MCS Connect Response
//   structure (using definite-length encoding) as described in [T125]
//   (the ASN.1 structure definition is detailed in [T125] section 7, part 2).
//   The userData field of the MCS Connect Response encapsulates the GCC
//   Conference Create Response data (contained in the gccCCrsp and subsequent
//   fields).

// gccCCrsp (variable): Variable-length PER-encoded GCC Connect Data structure
//   which encapsulates a Connect GCC PDU that contains a GCC Conference Create
//   Response structure as described in [T124] (the ASN.1 structure definitions
//   are specified in [T124] section 8.7) appended as user data to the MCS
//   Connect Response (using the format specified in [T124] sections 9.5 and
//   9.6). The userData field of the GCC Conference Create Response contains
//   one user data set consisting of concatenated server data blocks.

// serverCoreData (12 bytes): Server Core Data structure (section 2.2.1.4.2).

// serverSecurityData (variable): Variable-length Server Security Data structure
//   (section 2.2.1.4.3).

// serverNetworkData (variable): Variable-length Server Network Data structure
//   (section 2.2.1.4.4).

static inline void mcs_recv_connect_response(
                        Transport * trans,
                        ChannelList & mod_channel_list,
                        const ChannelList & front_channel_list,
                        CryptContext & encrypt, CryptContext & decrypt,
                        uint32_t & server_public_key_len,
                        uint8_t (& client_crypt_random)[512],
                        int & crypt_level,
                        int & use_rdp5,
                        Random * gen)
{
    Stream cr_stream(32768);
    X224In(trans, cr_stream);
    if (cr_stream.in_uint16_be() != BER_TAG_MCS_CONNECT_RESPONSE) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = cr_stream.in_ber_len();

    if (cr_stream.in_uint8() != BER_TAG_RESULT) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();

    int res = cr_stream.in_uint8();

    if (res != 0) {
        throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
    }
    if (cr_stream.in_uint8() != BER_TAG_INTEGER) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.in_skip_bytes(len); /* connect id */

    if (cr_stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.in_skip_bytes(len);

    if (cr_stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();

    cr_stream.in_skip_bytes(21); /* header (T.124 ConferenceCreateResponse) */
    len = cr_stream.in_uint8();

    if (len & 0x80) {
        len = cr_stream.in_uint8();
    }
    while (cr_stream.p < cr_stream.end) {
        uint16_t tag = cr_stream.in_uint16_le();
        uint16_t length = cr_stream.in_uint16_le();
        if (length <= 4) {
            throw Error(ERR_MCS_DATA_SHORT_HEADER);
        }
        uint8_t *next_tag = (cr_stream.p + length) - 4;
        switch (tag) {
        case SC_CORE:
            parse_mcs_data_sc_core(cr_stream, use_rdp5);
        break;
        case SC_SECURITY:
            parse_mcs_data_sc_security(cr_stream, encrypt, decrypt,
                                       server_public_key_len, client_crypt_random,
                                       crypt_level,
                                       gen);
        break;
        case SC_NET:
            parse_mcs_data_sc_net(cr_stream, front_channel_list, mod_channel_list);
            break;
        default:
            LOG(LOG_WARNING, "response tag 0x%x", tag);
            break;
        }
        cr_stream.p = next_tag;
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

static inline void mcs_recv_connect_initial(
                Transport * trans,
                ClientInfo * client_info,
                ChannelList & channel_list)
{
    Stream stream(32768);
    X224In(trans, stream);

    if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = stream.in_ber_len();
    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);
    if (stream.in_uint8() != BER_TAG_BOOLEAN) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();

    stream.in_skip_bytes(23);

// 2.2.1.3.1 User Data Header (TS_UD_HEADER)
// =========================================

// type (2 bytes): A 16-bit, unsigned integer. The type of the data
//                 block that this header precedes.

// +-------------------+-------------------------------------------------------+
// | CS_CORE 0xC001    | The data block that follows contains Client Core      |
// |                   | Data (section 2.2.1.3.2).                             |
// +-------------------+-------------------------------------------------------+
// | CS_SECURITY 0xC002| The data block that follows contains Client           |
// |                   | Security Data (section 2.2.1.3.3).                    |
// +-------------------+-------------------------------------------------------+
// | CS_NET 0xC003     | The data block that follows contains Client Network   |
// |                   | Data (section 2.2.1.3.4).                             |
// +-------------------+-------------------------------------------------------+
// | CS_CLUSTER 0xC004 | The data block that follows contains Client Cluster   |
// |                   | Data (section 2.2.1.3.5).                             |
// +-------------------+-------------------------------------------------------+
// | CS_MONITOR 0xC005 | The data block that follows contains Client           |
// |                   | Monitor Data (section 2.2.1.3.6).                     |
// +-------------------+-------------------------------------------------------+
// | SC_CORE 0x0C01    | The data block that follows contains Server Core      |
// |                   | Data (section 2.2.1.4.2)                              |
// +-------------------+-------------------------------------------------------+
// | SC_SECURITY 0x0C02| The data block that follows contains Server           |
// |                   | Security Data (section 2.2.1.4.3).                    |
// +-------------------+-------------------------------------------------------+
// | SC_NET 0x0C03     | The data block that follows contains Server Network   |
// |                   | Data (section 2.2.1.4.4)                              |
// +-------------------+-------------------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. The size in bytes of the data
//   block, including this header.

    while (stream.check_rem(4)) {
        uint8_t * current_header = stream.p;
        uint16_t tag = stream.in_uint16_le();
        uint16_t length = stream.in_uint16_le();
        if (length < 4 || !stream.check_rem(length - 4)) {
            LOG(LOG_ERR,
                "error reading block tag %d size %d\n",
                tag, length);
            break;
        }

        switch (tag){
            case CS_CORE:
                TODO(" we should check length to call the two variants of core_data (or begin by reading the common part then the extended part)")
                parse_mcs_data_cs_core(stream, client_info);
            break;
            case CS_SECURITY:
                parse_mcs_data_cs_security(stream);
            break;
            case CS_NET:
                parse_mcs_data_cs_net(stream, client_info, channel_list);
            break;
            case CS_CLUSTER:
                parse_mcs_data_cs_cluster(stream, client_info);
            break;
            case CS_MONITOR:
                parse_mcs_data_cs_monitor(stream);
            break;
            case SC_SECURITY:
//                parse_mcs_data_sc_security(stream);
            break;
            default:
                LOG(LOG_INFO, "Unexpected data block tag %x\n", tag);
            break;
        }
        stream.p = current_header + length;
    }
}


// 2.2.1.4  Server MCS Connect Response PDU with GCC Conference Create Response
// ----------------------------------------------------------------------------

// From [MSRDPCGR]

// The MCS Connect Response PDU is an RDP Connection Sequence PDU sent from
// server to client during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent as a response to the MCS Connect Initial PDU (section
// 2.2.1.3). The MCS Connect Response PDU encapsulates a GCC Conference Create
// Response, which encapsulates concatenated blocks of settings data.

// A basic high-level overview of the nested structure for the Server MCS
// Connect Response PDU is illustrated in section 1.3.1.1, in the figure
// specifying MCS Connect Response PDU. Note that the order of the settings
// data blocks is allowed to vary from that shown in the previously mentioned
// figure and the message syntax layout that follows. This is possible because
// each data block is identified by a User Data Header structure (section
// 2.2.1.4.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
// section 13.7.

// mcsCrsp (variable): Variable-length BER-encoded MCS Connect Response
//   structure (using definite-length encoding) as described in [T125]
//   (the ASN.1 structure definition is detailed in [T125] section 7, part 2).
//   The userData field of the MCS Connect Response encapsulates the GCC
//   Conference Create Response data (contained in the gccCCrsp and subsequent
//   fields).

// gccCCrsp (variable): Variable-length PER-encoded GCC Connect Data structure
//   which encapsulates a Connect GCC PDU that contains a GCC Conference Create
//   Response structure as described in [T124] (the ASN.1 structure definitions
//   are specified in [T124] section 8.7) appended as user data to the MCS
//   Connect Response (using the format specified in [T124] sections 9.5 and
//   9.6). The userData field of the GCC Conference Create Response contains
//   one user data set consisting of concatenated server data blocks.

// serverCoreData (12 bytes): Server Core Data structure (section 2.2.1.4.2).

// serverSecurityData (variable): Variable-length Server Security Data structure
//   (section 2.2.1.4.3).

// serverNetworkData (variable): Variable-length Server Network Data structure
//   (section 2.2.1.4.4).


// 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
// ------------------------------------------

//The TS_UD_CS_CORE data block contains core client connection-related
// information.

//header (4 bytes): GCC user data block header, as specified in section
//                  2.2.1.3.1. The User Data Header type field MUST be set to
//                  CS_CORE (0xC001).

// version (4 bytes): A 32-bit, unsigned integer. Client version number for the
//                    RDP. The major version number is stored in the high 2
//                    bytes, while the minor version number is stored in the
//                    low 2 bytes.
// +------------+------------------------------------+
// |   Value    |    Meaning                         |
// +------------+------------------------------------+
// | 0x00080001 | RDP 4.0 clients                    |
// +------------+------------------------------------+
// | 0x00080004 | RDP 5.0, 5.1, 5.2, and 6.0 clients |
// +------------+------------------------------------+

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                         width in pixels (up to a maximum value of 4096
//                         pixels).

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                          height in pixels (up to a maximum value of 2048
//                          pixels).

// colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color depth.
//                       Values in this field MUST be ignored if the
//                       postBeta2ColorDepth field is present.
// +--------------------------+-------------------------+
// |     Value                |        Meaning          |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+

// SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access sequence.
//                        This field SHOULD be set to RNS_UD_SAS_DEL (0xAA03).

// keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout (active
//                           input locale identifier). For a list of possible
//                           input locales, see [MSDN-MUI].

// clientBuild (4 bytes): A 32-bit, unsigned integer. The build number of the
//                        client.

// clientName (32 bytes): Name of the client computer. This field contains up to
//                        15 Unicode characters plus a null terminator.

// keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
// +-------+--------------------------------------------+
// | Value |              Meaning                       |
// +-------+--------------------------------------------+
// |   1   | IBM PC/XT or compatible (83-key) keyboard  |
// +-------+--------------------------------------------+
// |   2   | Olivetti "ICO" (102-key) keyboard          |
// +-------+--------------------------------------------+
// |   3   | IBM PC/AT (84-key) and similar keyboards   |
// +-------+--------------------------------------------+
// |   4   | IBM enhanced (101- or 102-key) keyboard    |
// +-------+--------------------------------------------+
// |   5   | Nokia 1050 and similar keyboards           |
// +-------+--------------------------------------------+
// |   6   | Nokia 9140 and similar keyboards           |
// +-------+--------------------------------------------+
// |   7   | Japanese keyboard                          |
// +-------+--------------------------------------------+

// keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard subtype
//                            (an original equipment manufacturer-dependent
//                            value).

// keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number of
//                                function keys on the keyboard.

// imeFileName (64 bytes): A 64-byte field. The Input Method Editor (IME) file
//                         name associated with the input locale. This field
//                         contains up to 31 Unicode characters plus a null
//                         terminator.

// postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
//                                color depth. Values in this field MUST be
//                                ignored if the highColorDepth field is
//                                present.
// +--------------------------+-------------------------+
// |      Value               |         Meaning         |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientProductId (2 bytes): A 16-bit, unsigned integer. The client product ID.
//                            This field SHOULD be initialized to 1. If this
//                            field is present, then all of the preceding fields
//                            MUST also be present. If this field is not
//                            present, then none of the subsequent fields MUST
//                            be present.

// serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number. This field
//                         SHOULD be initialized to 0. If this field is present,
//                         then all of the preceding fields MUST also be
//                         present. If this field is not present, then none of
//                         the subsequent fields MUST be present.

// highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
//                           depth.
// +-------+-------------------------------------------------------------------+
// | Value |                      Meaning                                      |
// +-------+-------------------------------------------------------------------+
// |     4 |   4 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |     8 |   8 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |    15 |  15-bit 555 RGB mask                                              |
// |       |  (5 bits for red, 5 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    16 |  16-bit 565 RGB mask                                              |
// |       |  (5 bits for red, 6 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    24 |  24-bit RGB mask                                                  |
// |       |  (8 bits for red, 8 bits for green, and 8 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies the
//                                 high color depths that the client is capable
//                                 of supporting.
// +-----------------------------+---------------------------------------------+
// |          Flag               |                Meaning                      |
// +-----------------------------+---------------------------------------------+
// | 0x0001 RNS_UD_24BPP_SUPPORT | 24-bit RGB mask                             |
// |                             | (8 bits for red, 8 bits for green,          |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0002 RNS_UD_16BPP_SUPPORT | 16-bit 565 RGB mask                         |
// |                             | (5 bits for red, 6 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0004 RNS_UD_15BPP_SUPPORT | 15-bit 555 RGB mask                         |
// |                             | (5 bits for red, 5 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0008 RNS_UD_32BPP_SUPPORT | 32-bit RGB mask                             |
// |                             | (8 bits for the alpha channel,              |
// |                             | 8 bits for red, 8 bits for green,           |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// earlyCapabilityFlags (2 bytes): A 16-bit, unsigned integer. It specifies
// capabilities early in the connection sequence.
// +---------------------------------------------+-----------------------------|
// |                Flag                         |              Meaning        |
// +---------------------------------------------+-----------------------------|
// | 0x0001 RNS_UD_CS_SUPPORT_ERRINFO_PDU        | Indicates that the client   |
// |                                             | supports the Set Error Info |
// |                                             | PDU (section 2.2.5.1).      |
// +---------------------------------------------+-----------------------------|
// | 0x0002 RNS_UD_CS_WANT_32BPP_SESSION         | Indicates that the client is|
// |                                             | requesting a session color  |
// |                                             | depth of 32 bpp. This flag  |
// |                                             | is necessary because the    |
// |                                             | highColorDepth field does   |
// |                                             | not support a value of 32.  |
// |                                             | If this flag is set, the    |
// |                                             | highColorDepth field SHOULD |
// |                                             | be set to 24 to provide an  |
// |                                             | acceptable fallback for the |
// |                                             | scenario where the server   |
// |                                             | does not support 32 bpp     |
// |                                             | color.                      |
// +---------------------------------------------+-----------------------------|
// | 0x0004 RNS_UD_CS_SUPPORT_STATUSINFO_PDU     | Indicates that the client   |
// |                                             | supports the Server Status  |
// |                                             | Info PDU (section 2.2.5.2). |
// +---------------------------------------------+-----------------------------|
// | 0x0008 RNS_UD_CS_STRONG_ASYMMETRIC_KEYS     | Indicates that the client   |
// |                                             | supports asymmetric keys    |
// |                                             | larger than 512 bits for use|
// |                                             | with the Server Certificate |
// |                                             | (section 2.2.1.4.3.1) sent  |
// |                                             | in the Server Security Data |
// |                                             | block (section 2.2.1.4.3).  |
// +---------------------------------------------+-----------------------------|
// | 0x0020 RNS_UD_CS_RESERVED1                  | Reserved for future use.    |
// |                                             | This flag is ignored by the |
// |                                             | server.                     |
// +---------------------------------------------+-----------------------------+
// | 0x0040 RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU | Indicates that the client   |
// |                                             | supports the Monitor Layout |
// |                                             | PDU (section 2.2.12.1).     |
// +---------------------------------------------+-----------------------------|
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientDigProductId (64 bytes): Contains a value that uniquely identifies the
//                                client. If this field is present, then all of
//                                the preceding fields MUST also be present. If
//                                this field is not present, then none of the
//                                subsequent fields MUST be present.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding to align the
//   serverSelectedProtocol field on the correct byte boundary.
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer. It contains the value returned
//   by the server in the selectedProtocol field of the RDP Negotiation Response structure
//   (section 2.2.1.2.1). In the event that an RDP Negotiation Response structure was not sent,
//   this field MUST be initialized to PROTOCOL_RDP (0). If this field is present, then all of the
//   preceding fields MUST also be present.

static inline void mcs_send_connect_response(
                        Transport * trans,
                        ClientInfo * client_info,
                        const ChannelList & channel_list,
                        uint8_t (&server_random)[32],
                        int rc4_key_size,
                        uint8_t (&pub_mod)[512],
                        uint8_t (&pri_exp)[512],
                        Random * gen
                    ) throw(Error)
{
    Stream stream(32768);

    // TPKT Header (length = 337 bytes)
    // X.224 Data TPDU
    X224Out tpdu(X224Packet::DT_TPDU, stream);

    // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
    stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
    uint32_t offset_len_mcs_connect_response = stream.get_offset(0);
    // BER: Type Length
    stream.out_ber_len_uint16(0); // filled later, 3 bytes

    // Connect-Response::result = rt-successful (0)
    // The first byte (0x0a) is the ASN.1 BER encoded Enumerated type. The
    // length of the value is given by the second byte (1 byte), and the
    // actual value is 0 (rt-successful).
    stream.out_uint8(BER_TAG_RESULT);
    stream.out_ber_len_uint7(1);
    stream.out_uint8(0);

    // Connect-Response::calledConnectId = 0
    stream.out_uint8(BER_TAG_INTEGER);
    stream.out_ber_len_uint7(1);
    stream.out_uint8(0);

    // Connect-Response::domainParameters (26 bytes)
    stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
    stream.out_ber_len_uint7(26);
    // DomainParameters::maxChannelIds = 34
    stream.out_ber_int8(22);
    // DomainParameters::maxUserIds = 3
    stream.out_ber_int8(3);
    // DomainParameters::maximumTokenIds = 0
    stream.out_ber_int8(0);
    // DomainParameters::numPriorities = 1
    stream.out_ber_int8(1);
    // DomainParameters::minThroughput = 0
    stream.out_ber_int8(0);
    // DomainParameters::maxHeight = 1
    stream.out_ber_int8(1);
    // DomainParameters::maxMCSPDUsize = 65528
    stream.out_ber_int24(0xfff8);
    // DomainParameters::protocolVersion = 2
    stream.out_ber_int8(2);

    // Connect-Response::userData (287 bytes)
    stream.out_uint8(BER_TAG_OCTET_STRING);
    uint32_t offset_len_mcs_data = stream.get_offset(0);
    stream.out_ber_len_uint16(0); // filled later, 3 bytes


    // GCC Conference Create Response
    // ------------------------------

    // ConferenceCreateResponse Parameters
    // -----------------------------------

    // Generic definitions used in parameter descriptions:

    // simpleTextFirstCharacter UniversalString ::= {0, 0, 0, 0}

    // simpleTextLastCharacter UniversalString ::= {0, 0, 0, 255}

    // SimpleTextString ::=  BMPString (SIZE (0..255)) (FROM (simpleTextFirstCharacter..simpleTextLastCharacter))

    // TextString ::= BMPString (SIZE (0..255)) -- Basic Multilingual Plane of ISO/IEC 10646-1 (Unicode)

    // SimpleNumericString ::= NumericString (SIZE (1..255)) (FROM ("0123456789"))

    // DynamicChannelID ::= INTEGER (1001..65535) -- Those created and deleted by MCS

    // UserID ::= DynamicChannelID

    // H221NonStandardIdentifier ::= OCTET STRING (SIZE (4..255))
    //      -- First four octets shall be country code and
    //      -- Manufacturer code, assigned as specified in
    //      -- Annex A/H.221 for NS-cap and NS-comm

    // Key ::= CHOICE   -- Identifier of a standard or non-standard object
    // {
    //      object              OBJECT IDENTIFIER,
    //      h221NonStandard     H221NonStandardIdentifier
    // }

    // UserData ::= SET OF SEQUENCE
    // {
    //      key     Key,
    //      value   OCTET STRING OPTIONAL
    // }

    // ConferenceCreateResponse ::= SEQUENCE
    // {    -- MCS-Connect-Provider response user data
    //      nodeID              UserID, -- Node ID of the sending node
    //      tag                 INTEGER,
    //      result              ENUMERATED
    //      {
    //          success                         (0),
    //          userRejected                    (1),
    //          resourcesNotAvailable           (2),
    //          rejectedForSymmetryBreaking     (3),
    //          lockedConferenceNotSupported    (4),
    //          ...
    //      },
    //      userData            UserData OPTIONAL,
    //      ...
    //}


    // User Data                 : Optional
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // User Data: Optional user data which may be used for functions outside
    // the scope of this Recommendation such as authentication, billing,
    // etc.

    // Result                    : Mandatory
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // An indication of whether the request was accepted or rejected, and if
    // rejected, the reason why. It contains one of a list of possible
    // results: successful, user rejected, resources not available, rejected
    // for symmetry-breaking, locked conference not supported, Conference
    // Name and Conference Name Modifier already exist, domain parameters
    // unacceptable, domain not hierarchical, lower-layer initiated
    // disconnect, unspecified failure to connect. A negative result in the
    // GCC-Conference-Create confirm does not imply that the physical
    // connection to the node to which the connection was being attempted
    // is disconnected.

    // The ConferenceCreateResponse PDU is shown in Table 8-4. The Node ID
    // parameter, which is the User ID assigned by MCS in response to the
    // MCS-Attach-User request issued by the GCC Provider, shall be supplied
    // by the GCC Provider sourcing this PDU. The Tag parameter is assigned
    // by the source GCC Provider to be locally unique. It is used to
    // identify the returned UserIDIndication PDU. The Result parameter
    // includes GCC-specific failure information sourced directly from
    // the Result parameter in the GCC-Conference-Create response primitive.
    // If the Result parameter is anything except successful, the Result
    // parameter in the MCS-Connect-Provider response is set to
    // user-rejected.

    //            Table 8-4 – ConferenceCreateResponse GCCPDU
    // +------------------+------------------+--------------------------+
    // | Content          |     Source       |         Sink             |
    // +==================+==================+==========================+
    // | Node ID          | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Tag              | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Result           | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+
    // | User Data (opt.) | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+

    //PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
    // 00 05 00
    // 14 7c 00 01
    // 2a
    // 14 76 0a 01 01 00 01 c0 00 4d 63 44 6e
    // 81 08


    // 00 05 -> Key::object length = 5 bytes
    // 00 14 7c 00 01 -> Key::object = { 0 0 20 124 0 1 }
    stream.out_uint16_be(5);
    stream.out_copy_bytes("\x00\x14\x7c\x00\x01", 5);


    // 2a -> ConnectData::connectPDU length = 42 bytes
    // This length MUST be ignored by the client.
    stream.out_uint8(0x2a);

    // PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Response
    // PDU:
    // 14 76 0a 01 01 00 01 c0 00 00 4d 63 44 6e 81 08

    // 0x14:
    // 0 - extension bit (ConnectGCCPDU)
    // 0 - --\ ...
    // 0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateResponse (1)
    // 1 - --/ of type ConferenceCreateResponse
    // 0 - extension bit (ConferenceCreateResponse)
    // 1 - ConferenceCreateResponse::userData present
    // 0 - padding
    // 0 - padding
    stream.out_uint8(0x10 | 4);

    // ConferenceCreateResponse::nodeID
    //  = 0x760a + 1001 = 30218 + 1001 = 31219
    //  (minimum for UserID is 1001)
    stream.out_uint16_le(0x760a);

    // ConferenceCreateResponse::tag length = 1 byte
    stream.out_uint8(1);

    // ConferenceCreateResponse::tag = 1
    stream.out_uint8(1);

    // 0x00:
    // 0 - extension bit (Result)
    // 0 - --\ ...
    // 0 -   | ConferenceCreateResponse::result = success (0)
    // 0 - --/
    // 0 - padding
    // 0 - padding
    // 0 - padding
    // 0 - padding
    stream.out_uint8(0);

    // number of UserData sets = 1
    stream.out_uint8(1);

    // 0xc0:
    // 1 - UserData::value present
    // 1 - CHOICE: From Key select h221NonStandard (1)
    //               of type H221NonStandardIdentifier
    // 0 - padding
    // 0 - padding
    // 0 - padding
    // 0 - padding
    // 0 - padding
    // 0 - padding
    stream.out_uint8(0xc0);

    // h221NonStandard length = 0 + 4 = 4 octets
    //   (minimum for H221NonStandardIdentifier is 4)
    stream.out_uint8(0);

    // h221NonStandard (server-to-client H.221 key) = "McDn"
    stream.out_copy_bytes("McDn", 4);

//        uint16_t padding = channel_list.size() & 1;
//        uint16_t srv_channel_size = 8 + (channel_list.size() + padding) * 2;
//        stream.out_2BUE(8 + srv_channel_size + 236 + 4); // len


    uint32_t offset_user_data_len = stream.get_offset(0);
    stream.out_uint16_be(0);

    bool use_rdp5 = 1;
    out_mcs_data_sc_core(stream, use_rdp5);
    out_mcs_data_sc_net(stream, channel_list);
    front_out_gcc_conference_user_data_sc_sec1(stream, client_info->crypt_level, server_random, rc4_key_size, pub_mod, pri_exp, gen);

    TODO(" create a function in stream that sets differed ber_len_offsets (or other len_offset)")

    // set user_data_len (TWO_BYTE_UNSIGNED_ENCODING)
    stream.set_out_uint16_be(0x8000 | (stream.get_offset(offset_user_data_len + 2)), offset_user_data_len);
    // set mcs_data len, BER_TAG_OCTET_STRING (some kind of BLOB)
    stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_data + 3), offset_len_mcs_data);
    // set BER_TAG_MCS_CONNECT_RESPONSE len
    stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_connect_response + 3), offset_len_mcs_connect_response);

    tpdu.end();
    tpdu.send(trans);
}

#endif

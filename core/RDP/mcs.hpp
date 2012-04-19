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
#include "RDP/gcc_conference_user_data/cs_core.hpp"
#include "RDP/gcc_conference_user_data/cs_cluster.hpp"
#include "RDP/gcc_conference_user_data/cs_sec.hpp"
#include "RDP/gcc_conference_user_data/cs_net.hpp"
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
    stream.out_uint16_be(5);
    stream.out_uint16_be(0x14);
    stream.out_uint8(0x7c);
    stream.out_uint16_be(1);

    int length = 158 + 76 + 12 + 4;

    TODO(" another option could be to emit channel list even if number of channel is zero. It looks more logical to me than not passing any channel information (what happens in this case ?)")
    if (channel_list.size() > 0){
        length += channel_list.size() * 12 + 8;
    }

    stream.out_uint16_be((length | 0x8000)); /* remaining length */

    stream.out_uint16_be(8); /* length? */
    stream.out_uint16_be(16);
    stream.out_uint8(0);
    stream.out_uint16_le(0xc001);
    stream.out_uint8(0);

    stream.out_copy_bytes("Duca", 4); /* OEM ID: "Duca", as in Ducati. */
    stream.out_uint16_be(((length - 14) | 0x8000)); /* remaining length */

    /* Client User Data */
    mod_rdp_out_cs_core(stream, use_rdp5, front_width, front_height, front_bpp, keylayout, hostname);
    mod_rdp_out_cs_cluster(stream, console_session);
    mod_rdp_out_cs_sec(stream);
    mod_rdp_out_cs_net(stream, channel_list);

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



#endif

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

   X224 class 0 packets management, complies with
   RFC 1006, ISO transport services on top of the TCP

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

#include "RDP/x224.hpp"


class McsOut
{
    Stream & stream;
    uint8_t offlen;
    public:
    McsOut(Stream & stream, uint8_t command, uint8_t user_id, uint16_t chan_id)
        : stream(stream), offlen(stream.p - stream.data + 6)
    {
        stream.out_uint8(command << 2);
        stream.out_uint16_be(user_id);
        stream.out_uint16_be(chan_id);
        stream.out_uint8(0x70);
        stream.skip_uint8(2); //len
    }

    void end(){
        int len = stream.p - stream.data - offlen - 2;
        stream.set_out_uint16_be(0x8000|len, this->offlen);
    }
};


class McsIn
{
    public:
    uint8_t opcode;
    uint16_t user_id;
    uint16_t chan_id;
    uint8_t magic_0x70; // some ber header ?
    uint16_t len;

    McsIn(Stream & stream)
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
        #warning put some assertion here to ensure all data has been consumed
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

inline static void recv_connection_initial(Transport * trans, Stream & data)
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



#endif

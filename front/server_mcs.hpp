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

   mcs layer at core module

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

#include "stream.hpp"
#include "constants.hpp"
#include "file_loc.hpp"
#include "log.hpp"
#include "RDP/x224.hpp"

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;


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

/* mcs */
struct server_mcs {
    struct Transport *trans;
    int userid;
    vector<struct mcs_channel_item *> channel_list;
    server_mcs(struct Transport *trans)
        : trans(trans), userid(1)
    {
    }

    ~server_mcs(){
        int count = (int) this->channel_list.size();
        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                delete channel_item;
            }
        }
    }

    private:

    static void mcs_ber_out_header(Stream & stream, int tag_val, int len)
    {
        if (tag_val > 0xff) {
            stream.out_uint16_be(tag_val);
        } else {
            stream.out_uint8(tag_val);
        }
        if (len >= 0x80) {
            stream.out_uint8(0x82);
            stream.out_uint16_be(len);
        } else {
            stream.out_uint8(len);
        }
    }

    void mcs_ber_out_int8(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(value);
    }

    void mcs_ber_out_int16(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 2);
        stream.out_uint8((value >> 8));
        stream.out_uint8(value);
    }

    void mcs_ber_out_int24(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 3);
        stream.out_uint8(value >> 16);
        stream.out_uint8(value >> 8);
        stream.out_uint8(value);
    }

    int ber_parse_header(Stream & stream, int tag_val) throw (Error)
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
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
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
            throw Error(ERR_MCS_BER_HEADER_TRUNCATED);
        }
        return len;
    }

    void mcs_out_domain_params(Stream & stream, int max_channels,
                               int max_users, int max_tokens, int max_pdu_size)
    {
        this->mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 26);
        this->mcs_ber_out_int8(stream, max_channels);
        this->mcs_ber_out_int8(stream, max_users);
        this->mcs_ber_out_int8(stream, max_tokens);
        this->mcs_ber_out_int8(stream, 1);
        this->mcs_ber_out_int8(stream, 0);
        this->mcs_ber_out_int8(stream, 1);
        this->mcs_ber_out_int24(stream, max_pdu_size);
        this->mcs_ber_out_int8(stream, 2);
    }

//    void mcs_out_domain_params(Stream & stream, int max_channels,
//                          int max_users, int max_tokens, int max_pdu_size)
//    {
//        this->mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 32);
//        this->mcs_ber_out_int16(stream, max_channels);
//        this->mcs_ber_out_int16(stream, max_users);
//        this->mcs_ber_out_int16(stream, max_tokens);
//        this->mcs_ber_out_int16(stream, 1);
//        this->mcs_ber_out_int16(stream, 0);
//        this->mcs_ber_out_int16(stream, 1);
//        this->mcs_ber_out_int16(stream, max_pdu_size);
//        this->mcs_ber_out_int16(stream, 2);
//    }

    public:

    void mcs_send_connection_initial(Stream & client_mcs_data) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = client_mcs_data.end - client_mcs_data.data;
        int len = 7 + 3 * 34 + 4 + data_len;
        this->mcs_ber_out_header(stream, MCS_CONNECT_INITIAL, len);
        this->mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* calling domain */
        this->mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* called domain */
        this->mcs_ber_out_header(stream, BER_TAG_BOOLEAN, 1);
        stream.out_uint8(0xff); /* upward flag */
        this->mcs_out_domain_params(stream, 34, 2, 0, 0xffff); /* target params */
        this->mcs_out_domain_params(stream, 1, 1, 1, 0x420); /* min params */
        this->mcs_out_domain_params(stream, 0xffff, 0xfc17, 0xffff, 0xffff); /* max params */
        this->mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        stream.out_copy_bytes(client_mcs_data.data, data_len);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void mcs_recv_connection_initial(Stream & data)
    {
            Stream stream(8192);
            X224In(this->trans, stream);

            int len = this->ber_parse_header(stream, MCS_CONNECT_INITIAL);
            len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, BER_TAG_BOOLEAN);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
            stream.skip_uint8(len);
            len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);

            /* make a copy of client mcs data */
            data.init(len);
            data.out_copy_bytes(stream.p, len);
            data.mark_end();
            stream.skip_uint8(len);

    }

    void mcs_send_connect_response(Stream & data) throw(Error)
    {
//        LOG(LOG_INFO, "server_mcs_send_connect_response");
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = data.end - data.data;
        this->mcs_ber_out_header(stream, MCS_CONNECT_RESPONSE, data_len + 38);
        this->mcs_ber_out_header(stream, BER_TAG_RESULT, 1);
        stream.out_uint8(0);
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(0);
        this->mcs_out_domain_params(stream, 22, 3, 0, 0xfff8);
        this->mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        /* mcs data */
        stream.out_copy_bytes(data.data, data_len);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void mcs_recv_connect_response(Stream & stream) throw(Error)
    {
        int len = 0;
        X224In(this->trans, stream);
        len = this->ber_parse_header(stream, MCS_CONNECT_RESPONSE);
        len = this->ber_parse_header(stream, BER_TAG_RESULT);

        int res = stream.in_uint8();

        if (res != 0) {
            throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
        }
        len = this->ber_parse_header(stream, BER_TAG_INTEGER);
        stream.skip_uint8(len); /* connect id */

        len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
        stream.skip_uint8(len);

        len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);
    }

    void server_mcs_send_channel_join_confirm_PDU(int userid, int chanid) throw(Error)
    {
//        LOG(LOG_INFO, "server_mcs_send_channel_join_confirm_PDU");

        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_CJCF << 2) | 2);
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.out_uint16_be(chanid);
        stream.out_uint16_be(chanid);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void server_mcs_send_attach_user_confirm_PDU(int userid) throw(Error)
    {
//        LOG(LOG_INFO, "server_mcs_send_attach_user_confirm_PDU");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8(((MCS_AUCF << 2) | 2));
        stream.out_uint8(0);
        stream.out_uint16_be(userid);

        tpdu.end();
        tpdu.send(this->trans);
    }

    void server_mcs_send(Stream & stream, int chan) throw (Error)
    {
//        LOG(LOG_INFO, "server_mcs_send data=%p p=%p end=%p", stream.data, stream.p, stream.end);
        uint8_t * oldp = stream.p;
        stream.p = stream.mcs_hdr;
        int len = (stream.end - stream.p) - 8;
        if (len > 8192 * 2) {
            LOG(LOG_ERR,
                "error in server_mcs_send, size too long, its %d (buffer=%d)\n",
                len, stream.capacity);
        }
        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chan);
        stream.out_uint8(0x70);
        if (len >= 128) {
            len = len | 0x8000;
            stream.out_uint16_be(len);
        }
        else {
            stream.out_uint8(len);
            #warning this is ugly isn't there a way to avoid moving the whole buffer
            /* move everything up one byte */
            uint8_t *lp = stream.p;
            while (lp < stream.end) {
                lp[0] = lp[1];
                lp++;
            }
            stream.end--;
        }
        stream.p = oldp;
    }

    void server_mcs_disconnect() throw (Error)
    {
//        LOG(LOG_INFO, "server_mcs_disconnect");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(this->trans);
    }

    /* returns a zero based index of the channel,
      -1 if error or if it dosen't exist */
    int server_mcs_get_channel_id(const char * name)
    {
//        LOG(LOG_INFO, "server_mcs_get_channel_id");

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

    void server_mcs_parse_domain_params(Stream & stream)
    {
        int len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
        stream.skip_uint8(len);
        #warning we should change check behavior here and check before accessing data, not after, use check_rem
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_HEADER_TRUNCATED);
        }
    }

    private:


public:
    void join_channel(uint16_t channel_id)
    {
        Stream stream(8192);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In(this->trans, stream);

        int opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_CJRQ) {
            throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
        }
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
        stream.skip_uint8(4);
        if (opcode & 2) {
            stream.skip_uint8(2);
        }

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

        this->server_mcs_send_channel_join_confirm_PDU(this->userid, channel_id);
    }


};


#endif

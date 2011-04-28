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
#include "iso_layer.hpp"
#include "callback.hpp"

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
    Callback & cb;
    struct IsoLayer iso_layer;
    int userid;
    int chanid;
    Stream data;
    vector<struct mcs_channel_item *> channel_list;
    server_mcs(Callback & cb, struct Transport *trans)
        : cb(cb), iso_layer(trans), userid(1), chanid(1001)
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

    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
        CHANNEL_FLAG_FIRST = 0x01,
        CHANNEL_FLAG_LAST = 0x02,
        CHANNEL_FLAG_SHOW_PROTOCOL = 0x10,
    };

    public:

    void server_channel_init(Stream* s) throw (Error)
    {
        s->channel_hdr = s->p;
        s->p += 8;
    }

    void server_channel_send(Stream & stream, int channel_id, int total_data_len, int flags) throw (Error)
    {
        int chanid = (channel_id - MCS_GLOBAL_CHANNEL) - 1;

        struct mcs_channel_item* channel = this->get_channel(chanid);
        stream.p = stream.channel_hdr;
        stream.out_uint32_le(total_data_len);
        if (channel->flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        assert(channel->chanid == channel_id);
    }

    /* this will inform the callback that some channel data is ready */
    void server_channel_call_callback(Stream & stream, int channel_id, int total_data_len, int flags) throw (Error)
    {
        int size = (int)(stream.end - stream.p);
        #warning check the long parameter is OK for p here. At start it is a pointer, converting to long is dangerous. See why this should be necessary in callback.
        int rv = this->cb.callback(0x5555,
                               ((flags & 0xffff) << 16) | (channel_id & 0xffff),
                               size, (long)(stream.p), total_data_len);
        if (rv != 0){
            throw Error(ERR_CHANNEL_SESSION_CALLBACK_FAILED);
        }
    }

    /*****************************************************************************/
    /* This is called from the secure layer to process an incoming non global
       channel packet.
       'chanid' passed in here is the mcs channel id so it MCS_GLOBAL_CHANNEL
       plus something. */
    void server_channel_process(Stream & stream, int chanid) throw (Error)
    {
        /* this assumes that the channels are in order of chanid(mcs channel id)
           but they should be, see server_sec_process_mcs_data_channels
           the first channel should be MCS_GLOBAL_CHANNEL + 1, second
           one should be MCS_GLOBAL_CHANNEL + 2, and so on */
        int channel_id = (chanid - MCS_GLOBAL_CHANNEL) - 1;

        struct mcs_channel_item* channel = this->channel_list[channel_id];

        if (channel == 0) {
            throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
        }
        int length = stream.in_uint32_le();
        int flags = stream.in_uint32_le();
        this->server_channel_call_callback(stream, channel_id, length, flags);
    }


    int channel_count(){
        int rv = (int) this->channel_list.size();
        return rv;
    }

    mcs_channel_item * get_channel(int index) throw (Error)
    {
        int count = (int) this->channel_list.size();
        if (index < 0 || index >= count) {
            throw Error(ERR_MCS_CHANNEL_NOT_FOUND);
        }
        return this->channel_list[index];
    }


    void server_mcs_recv(Stream & stream, int* chan) throw (Error)
    {
        this->iso_layer.iso_recv(stream);
        int appid = stream.in_uint8() >> 2;
        /* Channel Join ReQuest datagram */
        while(appid == MCS_CJRQ) {
            /* this is channels getting added from the client */
            int userid = stream.in_uint16_be();
            int chanid = stream.in_uint16_be();
            this->server_mcs_send_channel_join_confirm_PDU(userid, chanid);
            this->iso_layer.iso_recv(stream);
            appid = stream.in_uint8() >> 2;
        }
        /* Disconnect Provider Ultimatum datagram */
        if (appid == MCS_DPUM) {
            throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
        }
        /* SenD ReQuest datagram */
        if (appid != MCS_SDRQ) {
            throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
        }
        stream.skip_uint8(2);
        *chan = stream.in_uint16_be();
        stream.skip_uint8(1);
        int len = stream.in_uint8();
        if (len & 0x80) {
            stream.skip_uint8(1);
        }
    }

    void server_mcs_recv_connect_initial(Stream & client_mcs_data) throw (Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_recv(stream);

        #warning ber_parse should probably be some kind of stream primitive
        int len = this->ber_parse_header(stream, MCS_CONNECT_INITIAL);
        len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);
        stream.skip_uint8(len);
        len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);
        stream.skip_uint8(len);
        len = this->ber_parse_header(stream, BER_TAG_BOOLEAN);
        stream.skip_uint8(len);

        this->server_mcs_parse_domain_params(stream);
        this->server_mcs_parse_domain_params(stream);
        this->server_mcs_parse_domain_params(stream);
        len = this->ber_parse_header(stream, BER_TAG_OCTET_STRING);

        /* make a copy of client mcs data */
        client_mcs_data.init(len);
        client_mcs_data.out_copy_bytes(stream.p, len);
        client_mcs_data.mark_end();

        stream.skip_uint8(len);
        if (!stream.check_end()) {
            throw Error(ERR_MCS_RECV_CONNECT_INITIAL_TRUNCATED);
        }
    }

    void server_mcs_recv_edrq() throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_recv(stream);
        int opcode = stream.in_uint8();
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

    void server_mcs_recv_aurq() throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_recv(stream);
        int opcode = stream.in_uint8();
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


    void server_mcs_recv_channel_join_request_PDU() throw(Error)
    {
            Stream stream(8192);
            // read tpktHeader (4 bytes = 3 0 len)
            // TPDU class 0    (3 bytes = LI F0 PDU_DT)
            this->iso_layer.iso_recv(stream);

            int opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_CJRQ) {
                throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
            }
            stream.skip_uint8(4);
            if (opcode & 2) {
                stream.skip_uint8(2);
            }
            // test if we went further than the end, this should be changed...
            if (!stream.check_end()) {
                throw Error(ERR_MCS_RECV_CJRQ_TRUNCATED);
            }
    }

    void server_mcs_send_channel_join_confirm_PDU(int userid, int chanid) throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_CJCF << 2) | 2);
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.out_uint16_be(chanid);
        stream.out_uint16_be(chanid);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send_attach_user_confirm_PDU(int userid) throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8(((MCS_AUCF << 2) | 2));
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send_connect_response() throw(Error)
    {
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        int data_len = this->data.end - this->data.data;
        this->iso_layer.iso_init(stream);
        this->server_mcs_ber_out_header(stream, MCS_CONNECT_RESPONSE, data_len + 38);
        this->server_mcs_ber_out_header(stream, BER_TAG_RESULT, 1);
        stream.out_uint8(0);
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(0);
        this->server_mcs_out_domain_params(stream, 22, 3, 0, 0xfff8);
        this->server_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        /* mcs data */
        stream.out_copy_bytes(this->data.data, data_len);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send(Stream & stream, int chan) throw (Error)
    {
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
            /* move everything up one byte */
            uint8_t *lp = stream.p;
            while (lp < stream.end) {
                lp[0] = lp[1];
                lp++;
            }
            stream.end--;
        }
        this->iso_layer.iso_send(stream);
        #warning do we need to call this for every mcs packet? maybe every 5 or so
        if (chan == MCS_GLOBAL_CHANNEL) {
            this->server_mcs_call_callback();
        }
    }

    void server_mcs_disconnect() throw (Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_init(Stream & stream)
    {
        this->iso_layer.iso_init(stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
    }

    /* returns a zero based index of the channel,
      -1 if error or if it dosen't exist */
    int server_mcs_get_channel_id(const char * name)
    {
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

    private:

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

    void server_mcs_parse_domain_params(Stream & stream)
    {
        int len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
        stream.skip_uint8(len);
        #warning we should change check behavior here and check before accessing data, not after, use check_rem
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_HEADER_TRUNCATED);
        }
    }


    static void server_mcs_ber_out_header(Stream & stream, int tag_val, int len)
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

    void server_mcs_ber_out_int8(Stream & stream, int value)
    {
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(value);
    }

    void server_mcs_ber_out_int24(Stream & stream, int value)
    {
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 3);
        stream.out_uint8(value >> 16);
        stream.out_uint8(value >> 8);
        stream.out_uint8(value);
    }

    void server_mcs_out_domain_params(Stream & stream,
                               int max_channels,
                               int max_users, int max_tokens,
                               int max_pdu_size)
    {
        this->server_mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 26);
        this->server_mcs_ber_out_int8(stream, max_channels);
        this->server_mcs_ber_out_int8(stream, max_users);
        this->server_mcs_ber_out_int8(stream, max_tokens);
        this->server_mcs_ber_out_int8(stream, 1);
        this->server_mcs_ber_out_int8(stream, 0);
        this->server_mcs_ber_out_int8(stream, 1);
        this->server_mcs_ber_out_int24(stream, max_pdu_size);
        this->server_mcs_ber_out_int8(stream, 2);
    }

    /* Inform the callback that an mcs packet has been sent.  This is needed so
       the module can send any high priority mcs packets like audio. */
    int server_mcs_call_callback()
    {
        return this->cb.callback(0x5556, 0, 0, 0, 0);
    }


};


#endif

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

#if !defined(__SERVER_MCS_HPP__)
#define __SERVER_MCS_HPP__

#include "stream.hpp"
#include "constants.hpp"
#include "file_loc.hpp"
#include "log.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;

struct server_mcs : public Mcs {
    int userid;
    vector<struct mcs_channel_item *> channel_list;
    server_mcs() : userid(1)
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

    public:


    void server_mcs_send_channel_join_confirm_PDU(Transport * trans, int userid, int chanid) throw(Error)
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
        tpdu.send(trans);
    }

    void server_mcs_send_attach_user_confirm_PDU(Transport * trans, int userid) throw(Error)
    {
//        LOG(LOG_INFO, "server_mcs_send_attach_user_confirm_PDU");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8(((MCS_AUCF << 2) | 2));
        stream.out_uint8(0);
        stream.out_uint16_be(userid);

        tpdu.end();
        tpdu.send(trans);
    }

    void server_mcs_send(Stream & stream, int chan) throw (Error)
    {
        LOG(LOG_INFO, "server_mcs_send data=%p p=%p end=%p", stream.data, stream.p, stream.end);
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
            stream.p = oldp;
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
            stream.p = oldp-1;
        }
    }

    void server_mcs_disconnect(Transport * trans) throw (Error)
    {
//        LOG(LOG_INFO, "server_mcs_disconnect");
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(trans);
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

    private:


public:
    void join_channel(Transport * trans, uint16_t channel_id)
    {
        Stream stream(8192);
        // read tpktHeader (4 bytes = 3 0 len)
        // TPDU class 0    (3 bytes = LI F0 PDU_DT)
        X224In(trans, stream);

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

        this->server_mcs_send_channel_join_confirm_PDU(trans, this->userid, channel_id);
    }


};


#endif

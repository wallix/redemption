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
#include "channel_list.hpp"

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



#endif

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

   MCS Connect Initial PDU with GCC Conference User Data


*/

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_NET_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_NET_HPP__

#include "channel_list.hpp"

// 2.2.1.4.4 Server Network Data (TS_UD_SC_NET)
// ============================================
// The TS_UD_SC_NET data block is a reply to the static virtual channel list
// presented in the Client Network Data structure (section 2.2.1.3.4).

// header (4 bytes): A GCC user data block header, as specified in section User
//  Data Header (section 2.2.1.3.1). The User Data Header type field MUST be set
//  to SC_NET (0x0C03).

// MCSChannelId (2 bytes): A 16-bit, unsigned integer. The MCS channel
//  identifier which the client MUST join to receive display data and send
//  client input (I/O channel).

// channelCount (2 bytes): A 16-bit, unsigned integer. The number of 16-bit,
//  unsigned integer MCS channel IDs in the channelIdArray field.

// channelIdArray (variable): A variable-length array of MCS channel IDs (each
//  channel ID is a 16-bit, unsigned integer) which have been allocated (the
//  number is given by the channelCount field). Each MCS channel ID corresponds
//  in position to the channels requested in the Client Network Data structure.
//  A channel value of 0 indicates that the channel was not allocated.

// Pad (2 bytes): A 16-bit, unsigned integer. Optional padding. Values in this
//  field MUST be ignored. The size in bytes of the Server Network Data
//  structure MUST be a multiple of 4. If the channelCount field contains an odd
//  value, then the size of the channelIdArray (and by implication the entire
//  Server Network Data structure) will not be a multiple of 4. In this
//  scenario, the Pad field MUST be present and it is used to add an additional
//  2 bytes to the size of the Server Network Data structure. If the
//  channelCount field contains an even value, then the Pad field is not
//  required and MUST NOT be present.

struct SCNetGccUserData {
    uint16_t userDataType;
    uint16_t length;

    SCNetGccUserData()
    : userDataType(SC_NET)
    , length(12) // default: everything except serverSelectedProtocol
    {
    }


    void emit(Stream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
    }

    void recv(Stream & stream, uint16_t length)
    {
        this->length = length;
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data SC_NET (%u bytes)", msg, this->length);
    }
};


static inline void parse_mcs_data_sc_net(Stream & stream, const ChannelList & front_channel_list, ChannelList & mod_channel_list)
{
    LOG(LOG_INFO, "SC_NET");

    stream.in_uint16_le(); /* base_channel */
    size_t num_channels = stream.in_uint16_le();

    /* We assume that the channel_id array is confirmed in the same order
    that it has been sent. If there are any channels not confirmed, they're
    going to be the last channels on the array sent in MCS Connect Initial */
    for (size_t index = 0; index < num_channels; index++){
        mod_channel_list.push_back(front_channel_list[index]);
    }
}

//03 0c 10 00 -> TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes

//eb 03 -> TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
//03 00 -> TS_UD_SC_NET::channelCount = 3
//ec 03 -> channel0 = 0x3ec = 1004 (rdpdr)
//ed 03 -> channel1 = 0x3ed = 1005 (cliprdr)
//ee 03 -> channel2 = 0x3ee = 1006 (rdpsnd)
//00 00 -> padding


static inline void out_mcs_data_sc_net(Stream & stream, const ChannelList & channel_list)
{
    uint16_t num_channels = channel_list.size();
    uint16_t padchan = num_channels & 1;

    stream.out_uint16_le(SC_NET);
    // length, including tag and length fields
    stream.out_uint16_le(8 + (num_channels + padchan) * 2);
    stream.out_uint16_le(MCS_GLOBAL_CHANNEL);
    stream.out_uint16_le(num_channels); /* number of other channels */

    for (int index = 0; index < num_channels; index++) {
            stream.out_uint16_le(MCS_GLOBAL_CHANNEL + (index + 1));
    }
    if (padchan){
        stream.out_uint16_le(0);
    }
}

#endif

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

static inline void parse_mcs_data_sc_net(Stream & stream, const ChannelList & front_channel_list, ChannelList & mod_channel_list)
{
    LOG(LOG_INFO, "SC_NET\n");

    stream.in_uint16_le(); /* base_channel */
    size_t num_channels = stream.in_uint16_le();

    /* We assume that the channel_id array is confirmed in the same order
    that it has been sent. If there are any channels not confirmed, they're
    going to be the last channels on the array sent in MCS Connect Initial */
    for (size_t index = 0; index < num_channels; index++){
        mod_channel_list.push_back(front_channel_list[index]);
    }
}

#endif

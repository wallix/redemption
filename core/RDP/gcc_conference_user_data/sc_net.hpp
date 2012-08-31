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

//03 0c 10 00 -> TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes

//eb 03 -> TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
//03 00 -> TS_UD_SC_NET::channelCount = 3
//ec 03 -> channel0 = 0x3ec = 1004 (rdpdr)
//ed 03 -> channel1 = 0x3ed = 1005 (cliprdr)
//ee 03 -> channel2 = 0x3ee = 1006 (rdpsnd)
//00 00 -> padding


static inline void out_mcs_data_sc_net(Stream & stream, const ChannelDefArray & channel_list)
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

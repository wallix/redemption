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

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_NET_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_NET_HPP__

#include "channel_list.hpp"
#include "client_info.hpp"

// 2.2.1.3.4 Client Network Data (TS_UD_CS_NET)
// --------------------------------------------
// The TS_UD_CS_NET packet contains a list of requested virtual channels.

// header (4 bytes): A 32-bit, unsigned integer. GCC user data block header,
//                   as specified in User Data Header (section 2.2.1.3.1).
//                   The User Data Header type field MUST be set to CS_NET
//                   (0xC003).

// channelCount (4 bytes): A 32-bit, unsigned integer. The number of
//                         requested static virtual channels (the maximum
//                         allowed is 31).

// channelDefArray (variable): A variable-length array containing the
//                             information for requested static virtual
//                             channels encapsulated in CHANNEL_DEF
//                             structures (section 2.2.1.3.4.1). The number
//                             of CHANNEL_DEF structures which follows is
//                             given by the channelCount field.

// 2.2.1.3.4.1 Channel Definition Structure (CHANNEL_DEF)
// ------------------------------------------------------
// The CHANNEL_DEF packet contains information for a particular static
// virtual channel.

// name (8 bytes): An 8-byte array containing a null-terminated collection
//                 of seven ANSI characters that uniquely identify the
//                 channel.

// options (4 bytes): A 32-bit, unsigned integer. Channel option flags.
//
//           Flag                             Meaning
// -------------------------------------------------------------------------
// CHANNEL_OPTION_INITIALIZED   Absence of this flag indicates that this
//        0x80000000            channel is a placeholder and that the
//                              server MUST NOT set it up.
// ------------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_RDP   This flag is unused and its value MUST be
//        0x40000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_SC    This flag is unused and its value MUST be
//        0x20000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_ENCRYPT_CS    This flag is unused and its value MUST be
//        0x10000000            ignored by the server.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_HIGH      Channel data MUST be sent with high MCS
//        0x08000000            priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_MED       Channel data MUST be sent with medium
//        0x04000000            MCS priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_PRI_LOW       Channel data MUST be sent with low MCS
//        0x02000000            priority.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_COMPRESS_RDP  Virtual channel data MUST be compressed
//        0x00800000            if RDP data is being compressed.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_COMPRESS      Virtual channel data MUST be compressed,
//        0x00400000            regardless of RDP compression settings.
// -------------------------------------------------------------------------
// CHANNEL_OPTION_SHOW_PROTOCOL The value of this flag MUST be ignored by
//        0x00200000            the server. The visibility of the Channel
//                              PDU Header (section 2.2.6.1.1) is
//                              determined by the CHANNEL_FLAG_SHOW_PROTOCOL
//                              (0x00000010) flag as defined in the flags
//                              field (section 2.2.6.1.1).
// -------------------------------------------------------------------------
//REMOTE_CONTROL_PERSISTENT     Channel MUST be persistent across remote
//       0x00100000             control transactions.

struct CSNetGccUserData {
    uint16_t userDataType;
    uint16_t length;

    CSNetGccUserData()
    : userDataType(CS_NET)
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
        LOG(LOG_INFO, "%s GCC User Data CS_NET (%u bytes)", msg, this->length);
    }
};

// this adds the mcs channels in the list of channels to be used when
// creating the server mcs data
static inline void parse_mcs_data_cs_net(Stream & stream, const ClientInfo * client_info, ChannelDefArray & channel_list)
{
    LOG(LOG_INFO, "CS_NET");
//    // this is an option set in rdpproxy.ini
//    // to disable all channels (no clipboard, no device redirection, etc)
//    if (client_info->channel_code != 1) { /* are channels on? */
//        return;
//    }
    uint32_t channelCount = stream.in_uint32_le();
    LOG(LOG_INFO, "cs_net:channel_count %u [%u]", channelCount, channel_list.size());

    for (uint32_t index = 0; index < channelCount; index++) {
        ChannelDef channel_item;
        memcpy(channel_item.name, stream.in_uint8p(8), 8);
        channel_item.flags = stream.in_uint32_be();
        channel_item.chanid = MCS_GLOBAL_CHANNEL + (index + 1);
        TODO(" push_back is not the best choice here as we have static space already available in channel_list we could even let ChannelDefArray manage parsing")
        channel_list.push_back(channel_item);
    }
}

static inline void mod_rdp_out_cs_net(Stream & stream, const ChannelDefArray & channel_list)
{
    /* Here we need to put channel information in order to redirect channel data
    from client to server passing through the "proxy" */
    size_t num_channels = channel_list.size();

    if (num_channels > 0) {
        LOG(LOG_INFO, "cs_net");
        LOG(LOG_INFO, "Sending Channels Settings to remote server [num channels=%u]", num_channels);
        stream.out_uint16_le(CS_NET);
        LOG(LOG_INFO, "cs_net::len=%u", num_channels * 12 + 8);
        stream.out_uint16_le(num_channels * 12 + 8); /* length */
        LOG(LOG_INFO, "cs_net::nb_chan=%u", num_channels);
        stream.out_uint32_le(num_channels); /* number of virtual channels */
        for (size_t index = 0; index < num_channels; index++){
            const ChannelDef & channel_item = channel_list[index];
            stream.out_copy_bytes(channel_item.name, 8);
            stream.out_uint32_be(channel_item.flags);
        }
    }
}

#endif

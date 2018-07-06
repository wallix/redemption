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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"
#include "core/RDP/gcc/userdata/mcs_channels.hpp"

#include <inttypes.h>

namespace GCC { namespace UserData {

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

//03 0c 10 00 -> TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes

//eb 03 -> TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
//03 00 -> TS_UD_SC_NET::channelCount = 3
//ec 03 -> channel0 = 0x3ec = 1004 (rdpdr)
//ed 03 -> channel1 = 0x3ed = 1005 (cliprdr)
//ee 03 -> channel2 = 0x3ee = 1006 (rdpsnd)
//00 00 -> padding

struct SCNet {
    uint16_t userDataType{SC_NET};
    uint16_t MCSChannelId{GCC::MCS_GLOBAL_CHANNEL};
    uint16_t channelCount{0};
    struct {
        uint16_t id;
    } channelDefArray[32];

    SCNet()

    = default;

    void emit(OutStream & stream) const
    {
        uint16_t const length = 8 + 4 * ((this->channelCount+1) >> 1);
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(length);
        stream.out_uint16_le(this->MCSChannelId);
        stream.out_uint16_le(this->channelCount);
        for (size_t i = 0; i < this->channelCount ; i++){
            stream.out_uint16_le(this->channelDefArray[i].id);
        }
        if (this->channelCount & 1){
            stream.out_uint16_le(0);
        }
    }

    void recv(InStream & stream, bool bogus_sc_net_size)
    {
        if (!stream.in_check_rem(8)){
            LOG(LOG_ERR, "SCNet::recv short header");
            throw Error(ERR_GCC);
        }

        this->userDataType = stream.in_uint16_le();
        uint16_t const length = stream.in_uint16_le();

        if (length < 8 || !stream.in_check_rem(length - 4)){
            LOG(LOG_ERR, "SCNet::recv bad header length=%" PRIu16 " size=%zu", length, stream.get_capacity());
            throw Error(ERR_GCC);
        }

        this->MCSChannelId = stream.in_uint16_le();
        this->channelCount = stream.in_uint16_le();

        if (!this->channelCount && (length == 10) && bogus_sc_net_size) {
            LOG(LOG_WARNING, "SCNet::recv accepts VirtualBox bogus TS_UD_SC_NET data block.");
        }
        else if (length != (((this->channelCount + (this->channelCount & 1)) << 1) + 8)) {
            LOG(LOG_ERR, "SCNet::recv bad header length=%d", length);
            throw Error(ERR_GCC);
        }

        if (this->channelCount >= 32) {
            LOG(LOG_ERR, "SCNet::recv channel count out of range (%u)", this->channelCount);
            throw Error(ERR_CHANNEL_OUT_OF_RANGE);
        }
        for (size_t i = 0; i < this->channelCount ; i++){
            this->channelDefArray[i].id = stream.in_uint16_le();
        }
        if (this->channelCount & 1){
            stream.in_skip_bytes(2);
        }
    }

    void log(const char * msg) const
    {
        // --------------------- Base Fields ---------------------------------------
        uint16_t const length = 8 + 4 * ((this->channelCount+1) >> 1);
        LOG(LOG_INFO, "%s GCC User Data SC_NET (%u bytes)", msg, length);
        LOG(LOG_INFO, "sc_net::MCSChannelId   = %u", this->MCSChannelId);
        LOG(LOG_INFO, "sc_net::channelCount   = %u", this->channelCount);

        for (size_t i = 0; i < this->channelCount ; i++){
            LOG(LOG_INFO, "sc_net::channel[%zu]::id = %" PRIu16
                , GCC::MCS_GLOBAL_CHANNEL + i + 1
                , this->channelDefArray[i].id
                );
        }
        if (this->channelCount & 1){
            LOG(LOG_INFO, "sc_net::padding 2 bytes 0000");
        }
    }
};

} // namespace UserData
} // namespace GCC

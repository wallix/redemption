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
#include "utils/stream.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {


// 2.2.1.3.7 Client Message Channel Data (TS_UD_CS_MCS_MSGCHANNEL)
// ===============================================================

// The TS_UD_CS_MCS_MSGCHANNEL packet indicates support for the message channel which
// is used to transport the Initiate Multitransport Request PDU (section 2.2.15.1).
// This packet is an Extended Client Data Block and MUST NOT be sent to a server which
// does not advertise support for Extended Client Data Blocks by using the
// EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section 2.2.1.2.1.

//    header (4 bytes): A GCC user data block header, as specified in User Data Header
// (section 2.2.1.3.1). The User Data Header type field MUST be set to CS_MCS_MSGCHANNEL
// (0xC006).

//    flags (4 bytes): A 32-bit, unsigned integer. This field is unused and reserved for
// future use. It MUST be set to zero.

struct CSMCSMsgChannel {
    uint16_t userDataType;
    uint16_t length;

    uint32_t flags;

    CSMCSMsgChannel()
    : userDataType(CS_MCS_MSGCHANNEL)
    , length(8)
    , flags(0)
    {
    }

    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->flags);
    }

    void recv(InStream & stream)
    {
        //LOG(LOG_INFO, "CSMCSMsgChannel");
        //hexdump_c(stream.get_current(), 8);
        if (!stream.in_check_rem(8)){
            LOG(LOG_ERR, "CS_MCS_MSGCHANNEL short header");
            throw Error(ERR_GCC);
        }
        this->userDataType         = stream.in_uint16_le();
        this->length               = stream.in_uint16_le();

        if (this->length != 8){
            LOG(LOG_ERR, "CS_MCS_MSGCHANNEL bad header length=%d", this->length);
            throw Error(ERR_GCC);
        }

        this->flags = stream.in_uint32_le();
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data CS_MCS_MSGCHANNEL (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "CSMCSMsgChannel::flags %u", this->flags);
    }
};

}} // namespaces

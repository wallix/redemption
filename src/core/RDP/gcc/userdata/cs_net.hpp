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
#include "utils/log.hpp"
#include "core/error.hpp"
#include "core/RDP/gcc/userdata/mcs_channels.hpp"

namespace GCC { namespace UserData {


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

struct CSNet {
    uint16_t userDataType = CS_NET;
    uint16_t length       = 12;
    uint32_t channelCount = 0;

    enum {
        CHANNEL_OPTION_INITIALIZED   = 0x80000000,
        CHANNEL_OPTION_ENCRYPT_RDP   = 0x40000000,
        CHANNEL_OPTION_ENCRYPT_SC    = 0x20000000,
        CHANNEL_OPTION_ENCRYPT_CS    = 0x10000000,
        CHANNEL_OPTION_PRI_HIGH      = 0x08000000,
        CHANNEL_OPTION_PRI_MED       = 0x04000000,
        CHANNEL_OPTION_PRI_LOW       = 0x02000000,
        CHANNEL_OPTION_COMPRESS_RDP  = 0x00800000,
        CHANNEL_OPTION_COMPRESS      = 0x00400000,
        CHANNEL_OPTION_SHOW_PROTOCOL = 0x00200000,
        REMOTE_CONTROL_PERSISTENT    = 0x00100000
    };

    struct {
        char name[8];
        uint32_t options;
    } channelDefArray[32];

    bool permissive = false;

    void emit(OutStream & stream) /* TODO const*/
    {
        stream.out_uint16_le(this->userDataType);
        this->length = this->channelCount * 12 + 8;
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->channelCount);
        for (size_t i = 0; i < this->channelCount ; i++){
            stream.out_copy_bytes(this->channelDefArray[i].name, 8);
            stream.out_uint32_le(this->channelDefArray[i].options);
        }
    }

    void recv(InStream & stream)
    {

        if (!stream.in_check_rem(8)){
            LOG(LOG_ERR, "CSNet::recv short header");
            throw Error(ERR_GCC);
        }

        this->userDataType = stream.in_uint16_le();
        this->length = stream.in_uint16_le();

        if (this->length < 8 || !stream.in_check_rem(this->length - 4)){
            LOG(LOG_ERR, "CSNet::recv bad header length=%u", this->length);
            throw Error(ERR_GCC);
        }

        this->channelCount = stream.in_uint32_le();
        if (this->channelCount >= 31) {
            LOG(LOG_ERR, "cs_net::recv channel count out of range (%u)", this->channelCount);
            this->channelCount = 0;
            if (this->permissive) {
                stream.in_skip_bytes(this->length - 8);
                return;
            }
            throw Error(ERR_CHANNEL_OUT_OF_RANGE);
        }
        if ((12 * this->channelCount + 8) != (this->length)) {
            LOG(LOG_ERR, "CSNet::recv length=%u and 12 * channelcount=%u not matching, ",
                this->length - 8u, this->channelCount);
            if (this->permissive) {
                stream.in_skip_bytes(this->length - 8);
                return;
            }
            throw Error(ERR_GCC);
        }

        for (size_t i = 0; i < this->channelCount ; i++){
            stream.in_copy_bytes(this->channelDefArray[i].name, 7);
            this->channelDefArray[i].name[7] = '\0';
            stream.in_skip_bytes(1);
            const size_t name_length = ::strlen(this->channelDefArray[i].name);
            ::memset(this->channelDefArray[i].name + name_length, 0,
                sizeof(this->channelDefArray[i].name) - name_length);
            this->channelDefArray[i].options = stream.in_uint32_le();
        }
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s GCC User Data CS_NET (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "cs_net::channelCount   = %u", this->channelCount);

        for (size_t i = 0; i < this->channelCount ; i++){
            uint32_t options = channelDefArray[i].options;
            LOG(LOG_INFO, "cs_net::channel '%.*s' [%zu]%s%s%s%s%s%s%s%s"
                , 8
                , channelDefArray[i].name, GCC::MCS_GLOBAL_CHANNEL + i + 1u
                , (options & CHANNEL_OPTION_INITIALIZED)?" INITIALIZED":""
                , (options & CHANNEL_OPTION_PRI_HIGH)?" PRI_HIGH":""
                , (options & CHANNEL_OPTION_PRI_MED)?" PRI_MED":""
                , (options & CHANNEL_OPTION_PRI_LOW)?" PRI_LOW":""
                , (options & CHANNEL_OPTION_COMPRESS_RDP)?" COMPRESS_RDP":""
                , (options & CHANNEL_OPTION_COMPRESS)?" COMPRESS":""
                , (options & REMOTE_CONTROL_PERSISTENT)?" PERSISTENT":""
                , (options & CHANNEL_OPTION_SHOW_PROTOCOL)?" SHOW_PROTOCOL":""
                );
        }
    }
};

} // namespace UserData
} // namespace GCC

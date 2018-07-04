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

namespace GCC { namespace UserData {

// 2.2.1.3.8 Client Multitransport Channel Data (TS_UD_CS_MULTITRANSPORT)
// ======================================================================

// The TS_UD_CS_MULTITRANSPORT packet is used to indicate support for the RDP Multitransport
// Layer ([MS-RDPEMT] section 1.3) and to specify multitransport characteristics. This packet
// is an Extended Client Data Block and MUST NOT be sent to a server which does not advertise
// support for Extended Client Data Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag
// (0x00000001) as described in section 2.2.1.2.1.

//    header (4 bytes): A GCC user data block header, as specified in User Data Header
// (section 2.2.1.3.1). The User Data Header type field MUST be set to CS_MULTITRANSPORT
// (0xC00A).

//    flags (4 bytes): A 32-bit, unsigned integer that specifies protocols supported by the
// client-side multitransport layer.

// +----------------------------+------------------------------------------------------------+
// | TRANSPORTTYPE_UDPFECR 0x01 | RDP-UDP Forward Error Correction (FEC) reliable transport  |
// |                            | ([MS-RDPEUDP] sections 1 to 3).                            |
// +----------------------------+------------------------------------------------------------+
// | TRANSPORTTYPE_UDPFECL 0x04 | RDP-UDP FEC lossy transport ([MS-RDPEUDP] sections 1 to 3).|
// +----------------------------+------------------------------------------------------------+
// |TRANSPORTTYPE_UDP_PREFERRED | Indicates that tunneling of static virtual channel traffic |
// | 0x100                      | over UDP is supported.                                     |
// +----------------------------+------------------------------------------------------------+

struct CSMultiTransport {
    enum {
          TRANSPORTTYPE_UDPFECR       = 0x01
        , TRANSPORTTYPE_UDPFECL       = 0x04
        , TRANSPORTTYPE_UDP_PREFERRED = 0x100
        , SOFTSYNC_TCP_TO_UDP         = 0x200
    };

    uint16_t userDataType{CS_MULTITRANSPORT};
    uint16_t length{8};

    uint32_t flags{0};

    CSMultiTransport()

    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->flags);
    }

    void recv(InStream & stream)
    {
        //LOG(LOG_INFO, "CSMultiTransport");
        //hexdump_c(stream.get_current(), 8);
        if (!stream.in_check_rem(8)){
            LOG(LOG_ERR, "CS_MULTITRANSPORT short header");
            throw Error(ERR_GCC);
        }
        this->userDataType         = stream.in_uint16_le();
        this->length               = stream.in_uint16_le();

        if (this->length != 8){
            LOG(LOG_ERR, "CS_MULTITRANSPORT bad header length=%d", this->length);
            throw Error(ERR_GCC);
        }

        this->flags = stream.in_uint32_le();
    }

    void log(const char * msg) const
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data CS_MULTITRANSPORT (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "CSMultiTransport::flags %u", this->flags);
    }
};

} // namespace UserData
} // namespace GCC

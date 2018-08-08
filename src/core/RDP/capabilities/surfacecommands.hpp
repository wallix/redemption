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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Surface Commands Capability Set ([MS-RDPBCGR] section 2.2.7.2.9)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.9 Surface Commands Capability Set (TS_SURFCMDS_CAPABILITYSET)
//======================================================================

// The TS_SURFCMDS_CAPABILITYSET structure advertises support for Surface Commands (section
// 2.2.9.2). This capability is sent by both the client and the server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of capability set. This field
//    MUST be set to 0x001C (CAPSETTYPE_SURFACE_COMMANDS).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data.

// cmdFlags (4 bytes): A 32-bit, unsigned integer. Flags indicating which Surface Commands are
//    supported.
//    +----------------------------+------------------------------------------------------------+
//    | SURFCMDS_SETSURFACEBITS    | The Set Surface Bits Command (section 2.2.9.2.1) is        |
//    | 0x00000002                 | supported.                                                 |
//    +----------------------------+------------------------------------------------------------+
//    | SURFCMDS_FRAMEMARKER       | The Frame Marker Command (section 2.2.9.2.3) is supported. |
//    | 0x00000010                 |                                                            |
//    +----------------------------+------------------------------------------------------------+
//    | SURFCMDS_STREAMSURFACEBITS | The Stream Surface Bits Command (section 2.2.9.2.2) is     |
//    | 0x00000040                 | supported.                                                 |
//    +----------------------------+------------------------------------------------------------+
//    If the client advertises support for surface commands, it MUST also indicate support for fast-
//    path output by setting the FASTPATH_OUTPUT_SUPPORTED (0x0001) flag in the extraFlags
//    field of the General Capability Set (section 2.2.7.1.1).

// reserved (4 bytes): This field is reserved for future use and has no effect on the RDP wire
//    traffic.

enum {
    CAPLEN_SURFACE_COMMANDS = 12
};

enum {
       SURFCMDS_SETSURFACEBITS = 0x02
     , SURFCMDS_FRAMEMARKER =  0x10
     , SURFCMDS_STREAMSURFACEBITS =  0x40
};

struct SurfaceCommandsCaps : public Capability {
    uint32_t cmdFlags{0}; // from a specific list of values (see enum); default 0x00
    uint32_t reserved{0}; // not Used

    SurfaceCommandsCaps()
    : Capability(CAPSETTYPE_SURFACE_COMMANDS, CAPLEN_SURFACE_COMMANDS)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);

        stream.out_uint32_le(this->cmdFlags);
        stream.out_uint32_le(this->reserved);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->cmdFlags = stream.in_uint32_le();
        this->reserved = stream.in_uint32_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s SurfaceCommands caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     SurfaceCommands caps::cmdFlags %u", this->cmdFlags);
        LOG(LOG_INFO, "     SurfaceCommands caps::reserved %u", this->reserved);
    }
};

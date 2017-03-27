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

   RDP Capabilities : Frame Acknowledge Capability Set ([MS-RDPRFX] section 2.2.1.3)

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.1.3 TS_FRAME_ACKNOWLEDGE_CAPABILITYSET
//===========================================

// The TS_FRAME_ACKNOWLEDGE_CAPABILITYSET structure advertises support for frame
// acknowledgment using the TS_FRAME_ACKNOWLEDGE_PDU (section 2.2.3.1) structure. This
// capability is sent by both the client and the server and is encapsulated in a server-to-client Demand
// Active PDU ([MS-RDPBCGR] section 2.2.1.13.1) or a client-to-server Confirm Active PDU ([MS-
// RDPBCGR] section 2.2.1.13.2).

// capabilitySetType (2 bytes): A 16-bit unsigned integer. The type of capability set. This field
//    MUST be set to CAPSETTYPE_FRAME_ACKNOWLEDGE (0x001E).

// lengthCapability (2 bytes): A 16-bit unsigned integer. The length, in bytes, of the capability
//    data.

// maxUnacknowledgedFrameCount (4 bytes): A 32-bit unsigned integer. When sent by the
//    server, it MAY be set to 0, in which case it simply advertises the server's ability to receive and
//    process the TS_FRAME_ACKNOWLEDGE_PDU (section 2.2.3.1). If it is not set to 0, it can be
//    used by the client as a hint that indicates the number of in-flight
//    TS_FRAME_ACKNOWLEDGE_PDUs that the server is prepared to accept. When sent by the
//    client, it provides a hint to the server as to how many in-flight frames the client can buffer.
//    Note that if the server chooses to have more in-flight frames than this number specifies, it is
//    possible that the client could be overloaded with frame data. The client MAY set this field to 0,
//    but this behavior SHOULD be avoided because it provides very little information to the server
//    other than that the client acknowledges frames.

enum {
    CAPLEN_FRAME_ACKNOWLEDGE = 8
};

struct FrameAcknowledgeCaps : public Capability {
    uint32_t maxUnacknowledgedFrameCount;

    FrameAcknowledgeCaps()
    : Capability(CAPSETTYPE_FRAME_ACKNOWLEDGE, CAPLEN_FRAME_ACKNOWLEDGE)
    , maxUnacknowledgedFrameCount(0) // SC: MAY be 0
                                     // CS: MAY be 0 but SHOULD be avoided because it provides too few information
    {
    }

    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->maxUnacknowledgedFrameCount);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->maxUnacknowledgedFrameCount = stream.in_uint32_le();
    }

    void log(const char * msg)
    {
        LOG(LOG_INFO, "%s FrameAcknowledge caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "FrameAcknowledge caps::maxUnacknowledgedFrameCount %u", this->maxUnacknowledgedFrameCount);
    }
};


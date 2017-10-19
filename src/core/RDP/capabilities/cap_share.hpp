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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   RDP Capabilities :

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.4 Share Capability Set (TS_SHARE_CAPABILITYSET)
// ====================================================
// The TS_SHARE_CAPABILITYSET structure is used to advertise the channel ID of
// the sender and is fully specified in [T128] section 8.2.12. This capability
// is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//    capability set. This field MUST be set to CAPSTYPE_SHARE (9).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//    of the capability data, including the size of the capabilitySetType and
//    lengthCapability fields.

// nodeId (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to 0
//    by the client and to the server channel ID by the server (0x03EA).

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//    field MUST be ignored.

enum {
    CAPLEN_SHARE = 8
};


struct ShareCaps : public Capability {
    uint16_t nodeId;
    uint16_t pad2octets;
    ShareCaps()
    : Capability(CAPSTYPE_SHARE, CAPLEN_SHARE)
    , nodeId(0)     // CS : SHOULD be set to 0
                    // SC : SHOULD be set to the server channel ID
    , pad2octets(0) // MUST be ignored
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->nodeId);
        stream.out_uint16_le(this->pad2octets);

    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->nodeId = stream.in_uint16_le();
        this->pad2octets = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s Share caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     Share caps::nodeId %u", this->nodeId);
        LOG(LOG_INFO, "     Share caps::pad2octets %u", this->pad2octets);
    }
};

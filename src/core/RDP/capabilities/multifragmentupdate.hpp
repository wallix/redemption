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

   RDP Capabilities : Multifragment Update Capability Set ([MS-RDPBCGR] section 2.2.7.2.6)

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.6 Multifragment Update Capability Set (TS_MULTIFRAGMENTUPDATE_CAPABILITYSET)
// ====================================================================================
// The TS_MULTIFRAGMENTUPDATE_CAPABILITYSET structure is used to specify capabilities related to
// the fragmentation and reassembly of Fast-Path Updates (see section 2.2.9.1.2.1). This capability is
// sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. Type of the capability set. This field
//    MUST be set to CAPSETTYPE_MULTIFRAGMENTUPDATE (26).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.

//  MaxRequestSize (4 bytes): A 32-bit, unsigned integer. The size of the buffer used to
//    reassemble the fragments of a Fast-Path Update (see section 2.2.9.1.2.1). The size of this
//    buffer places a cap on the size of the largest Fast-Path Update that can be fragmented (there
//    MUST always be enough buffer space to hold all of the related Fast-Path Update fragments for
//    reassembly).

enum {
    CAPLEN_MULTIFRAGMENTUPDATE = 8
};

struct MultiFragmentUpdateCaps : public Capability {
    uint32_t MaxRequestSize;

    MultiFragmentUpdateCaps()
    : Capability(CAPSETTYPE_MULTIFRAGMENTUPDATE, CAPLEN_MULTIFRAGMENTUPDATE)
    , MaxRequestSize(0) // 0 by default
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->MaxRequestSize);
    }

    void recv(InStream & stream, uint16_t len)override {
        this->len = len;
        this->MaxRequestSize = stream.in_uint32_le();
    }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s MultifragmentUpdate caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "MultifragmentUpdate caps::MaxRequestSize %u", this->MaxRequestSize);
    }
};

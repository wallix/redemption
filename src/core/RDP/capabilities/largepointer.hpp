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

   RDP Capabilities : Large Pointer Capability Set ([MS-RDPBCGR] section 2.2.7.2.7)

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.7 Large Pointer Capability Set (TS_LARGE_POINTER_CAPABILITYSET)
// =======================================================================

// The TS_LARGE_POINTER_CAPABILITYSET structure is used to specify capabilities related to large
// mouse pointer shape support. This capability is sent by both client and server.
// To support large pointer shapes, the client and server MUST support multifragment updates and
// indicate this support by exchanging the Multifragment Update Capability Set (section 2.2.7.2.6). The
// MaxRequestSize field of the Multifragment Update Capability Set MUST be set to at least 38,055
// bytes (so that a 96 x 96 pixel 32bpp pointer can be transported).

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. Type of the capability set. This field
//    MUST be set to CAPSETTYPE_LARGE_POINTER (27).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.

// largePointerSupportFlags (2 bytes): Support for large pointer shapes.
//    +--------------------------+----------------------------------------------------------+
//    | LARGE_POINTER_FLAG_96x96 | 96-pixel by 96-pixel mouse pointer shapes are supported. |
//    | 0x00000001               |                                                          |
//    +--------------------------+----------------------------------------------------------+
//    Mouse pointer shapes are used by the following pointer updates:
//    * Color Pointer Update (see section 2.2.9.1.1.4.4)
//    * New Pointer Update (see section 2.2.9.1.1.4.5)
//    * Fast-Path Color Pointer Update (see section 2.2.9.1.2.1.7)
//    * Fast-Path New Pointer Update (see section 2.2.9.1.2.1.8)
//    The pointer shape data is contained within the Color Pointer Update structure (see section
//    2.2.9.1.1.4.4) encapsulated by each of these updates.

enum  {
        LARGE_POINTER_FLAG_96x96 = 0x00000001
      };

enum {
    CAPLEN_LARGE_POINTER = 6
};

struct LargePointerCaps : public Capability {
    uint16_t largePointerSupportFlags;

    LargePointerCaps()
    : Capability(CAPSETTYPE_LARGE_POINTER, CAPLEN_LARGE_POINTER)
    , largePointerSupportFlags(0) // from a specific list of values (see enum)
    {
    }

    void emit(OutStream & stream) override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->largePointerSupportFlags);
    }

    void recv(InStream & stream, uint16_t len) override {
        this->len = len;
        this->largePointerSupportFlags = stream.in_uint16_le();
    }

    void log(const char * msg) override {
        LOG(LOG_INFO, "%s LargePointer caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "LargePointer caps::largePointerSupportFlags %u", this->largePointerSupportFlags);
    }
};


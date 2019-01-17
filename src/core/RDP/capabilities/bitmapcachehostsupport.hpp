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

   RDP Capabilities : OffScreen Bitmap Cache Host Support Capability Set ([MS-RDPBCGR] section 2.2.7.2.1)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

// 2.2.7.2.1 Bitmap Cache Host Support Capability Set (TS_BITMAPCACHE_HOSTSUPPORT_CAPABILITYSET)
// =============================================================================================

// The TS_BITMAPCACHE_HOSTSUPPORT_CAPABILITYSET structure is used to advertise
// support for persistent bitmap caching (see [MS-RDPEGDI] section 3.1.1.1.1).
// This capability set is only sent from server to client.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
// capability set. This field MUST be set to CAPSTYPE_BITMAPCACHE_HOSTSUPPORT
// (18).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
// of the capability data, including the size of the capabilitySetType and
// lengthCapability fields.

// cacheVersion (1 byte): An 8-bit, unsigned integer. Cache version. This field
// MUST be set to TS_BITMAPCACHE_REV2 (0x01), which indicates support for the
// Revision 2 bitmap caches (see [MS-RDPEGDI] section 3.1.1.1.1).

// pad1 (1 byte): An 8-bit, unsigned integer. Padding. Values in this field MUST be ignored.

// pad2 (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.


enum {
    CAPLEN_BITMAPCACHE_HOSTSUPPORT = 8
};

struct BitmapCacheHostSupportCaps : public Capability {
    uint8_t cacheVersion{1};
    uint8_t pad1{0};
    uint16_t pad2{0};

    BitmapCacheHostSupportCaps()
    : Capability(CAPSTYPE_BITMAPCACHE_HOSTSUPPORT, CAPLEN_BITMAPCACHE_HOSTSUPPORT)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint8(this->cacheVersion);
        stream.out_uint8(this->pad1);
        stream.out_uint16_le(this->pad2);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->cacheVersion = stream.in_uint8();
        this->pad1 = stream.in_uint8();
        this->pad2 = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s BitmapCacheHostSupport caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     BitmapCacheHostSupportCaps::%u", this->cacheVersion);
    }
};

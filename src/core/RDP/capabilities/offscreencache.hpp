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

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.7.1.9 Offscreen Bitmap Cache Capability Set (TS_OFFSCREEN_CAPABILITYSET)
// ============================================================================

// The TS_OFFSCREEN_CAPABILITYSET structure is used to advertise support for offscreen bitmap
// caching (see [MS-RDPEGDI] section 3.1.1.1.5). This capability is only sent from client to server.

// offscreenSupportLevel (4 bytes): A 32-bit, unsigned integer.
//    Offscreen bitmap cache support level.
//    +------------+------------------------------------------+
//    | FALSE      | Offscreen bitmap cache is not supported. |
//    | 0x00000000 |                                          |
//    +------------+------------------------------------------+
//    | TRUE       | Offscreen bitmap cache is supported.     |
//    | 0x00000001 |                                          |
//    +------------+------------------------------------------+

// offscreenCacheSize (2 bytes): A 16-bit, unsigned integer. The maximum size in kilobytes of
//    the offscreen bitmap cache (largest allowed value is 7680 KB).

// offscreenCacheEntries (2 bytes): A 16-bit, unsigned integer. The maximum number of cache
//    entries (largest allowed value is 500 entries).

enum {
    CAPLEN_OFFSCREENCACHE = 12
};


struct OffScreenCacheCaps : public Capability {
    uint32_t offscreenSupportLevel;
    uint16_t offscreenCacheSize;
    uint16_t offscreenCacheEntries;

    OffScreenCacheCaps()
    : Capability(CAPSTYPE_OFFSCREENCACHE, CAPLEN_OFFSCREENCACHE)
    , offscreenSupportLevel(false) // true/false
    , offscreenCacheSize(0)        // 0 by default (max = 7680 KB)
    , offscreenCacheEntries(0)     // 0 by default (max = 500 entries)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->offscreenSupportLevel);
        stream.out_uint16_le(this->offscreenCacheSize);
        stream.out_uint16_le(this->offscreenCacheEntries);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->offscreenSupportLevel = stream.in_uint32_le();
        this->offscreenCacheSize = stream.in_uint16_le();
        this->offscreenCacheEntries = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s OffScreenCache caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "OffScreenCache caps::offscreenSupportLevel %u", this->offscreenSupportLevel);
        LOG(LOG_INFO, "OffScreenCache caps::offscreenCacheSize %u", this->offscreenCacheSize);
        LOG(LOG_INFO, "OffScreenCache caps::offscreenCacheEntries %u", this->offscreenCacheEntries);
    }
};

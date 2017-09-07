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

   RDP Capabilities : DrawNineGrid Cache Capability Set ([MS-RDPEGDI] section 2.2.1.2)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/stream.hpp"

// 2.2.1.2 DrawNineGrid Cache Capability Set (TS_DRAW_NINEGRID_CAPABILITYSET)
// ==========================================================================
// The TS_DRAW_NINEGRID_CAPABILITYSET structure is used to advertise support for NineGrid
// bitmap caching and rendering (see sections 2.2.2.2.1.1.2.21, 2.2.2.2.1.1.2.22, and 2.2.2.2.1.3.4).
// This capability set is sent only from client to server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_DRAWNINEGRIDCACHE (0x0015).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length, in bytes, of the capability
//      data, including the size of the capabilitySetType and lengthCapability fields.

// drawNineGridSupportLevel (4 bytes): A 32-bit, unsigned integer. The level of support for
//    NineGrid drawing. This field MUST be set to one of the following values.
//    +------------------------------+---------------------------------------------------------+
//    | DRAW_NINEGRID_NO_SUPPORT     | NineGrid bitmap caching and rendering is not supported. |
//    | 0x00000000                   |                                                         |
//    +------------------------------+---------------------------------------------------------+
//    | DRAW_NINEGRID_SUPPORTED      | Revision 1 NineGrid bitmap caching and rendering is     |
//    | 0x00000001                   | supported. The Revision 1 versions of the stream bitmap |
//    |                              | alternate secondary orders (see section 2.2.2.2.1.3.5)  |
//    |                              | MUST be used to send the NineGrid bitmap from server to |
//    |                              | client.                                                 |
//    +------------------------------+---------------------------------------------------------+
//    | DRAW_NINEGRID_SUPPORTED_REV2 | Revision 2 NineGrid bitmap caching and rendering is     |
//    | 0x00000002                   | supported. The Revision 2 versions of the stream bitmap |
//    |                              | alternate secondary orders (see section 2.2.2.2.1.3.5)  |
//    |                              | MUST be used to send the NineGrid bitmap from server to |
//    |                              | client.                                                 |
//    +------------------------------+---------------------------------------------------------+

// drawNineGridCacheSize (2 bytes): A 16-bit, unsigned integer. The maximum size of the
//    NineGrid Bitmap Cache. The largest size allowed by current RDP servers is 2,560 kilobytes.

// drawNineGridCacheEntries (2 bytes): A 16-bit, unsigned integer. The maximum number of
//    entries allowed in the NineGrid Bitmap Cache. The maximum number of entries allowed by
//    current RDP servers is 256.

enum {
       DRAW_NINEGRID_NO_SUPPORT
     , DRAW_NINEGRID_SUPPORTED
     , DRAW_NINEGRID_SUPPORTED_REV2
};

enum {
    CAPLEN_DRAWNINEGRIDCACHE = 12
};

struct DrawNineGridCacheCaps : public Capability {
    uint32_t drawNineGridSupportLevel;
    uint16_t drawNineGridCacheSize;
    uint16_t drawNineGridCacheEntries;

    DrawNineGridCacheCaps()
    : Capability(CAPSTYPE_DRAWNINEGRIDCACHE, CAPLEN_DRAWNINEGRIDCACHE)
    , drawNineGridSupportLevel(DRAW_NINEGRID_NO_SUPPORT) // from a specific list of value (see enum)
    , drawNineGridCacheSize(0)    // 0 by default (max = 2560 Kilobytes)
    , drawNineGridCacheEntries(0) // 0 by default (max = 256 entries)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->drawNineGridSupportLevel);
        stream.out_uint16_le(this->drawNineGridCacheSize);
        stream.out_uint16_le(this->drawNineGridCacheEntries);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->drawNineGridSupportLevel = stream.in_uint32_le();
        this->drawNineGridCacheSize = stream.in_uint16_le();
        this->drawNineGridCacheEntries = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s DrawNineGridCache caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "DrawNineGridCache caps::drawNineGridSupportLevel %u", this->drawNineGridSupportLevel);
        LOG(LOG_INFO, "DrawNineGridCache caps::drawNineGridCacheSize %u", this->drawNineGridCacheSize);
        LOG(LOG_INFO, "DrawNineGridCache caps::drawNineGridCacheEntries %u", this->drawNineGridCacheEntries);
    }
};

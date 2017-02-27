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

   RDP Capabilities : Color Table Cache Capability Set (see [MS-RDPEGDI] section 2.2.1.1)
*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.1.1  Color Table Cache Capability Set (TS_COLORTABLE_CAPABILITYSET)

//    The TS_COLORTABLE_CAPABILITYSET structure is an unused capability set that advertises the size
//    of the color table cache used in conjunction with the Cache Color Table Secondary Drawing Order
//    (see section 2.2.2.2.1.2.4) and is based on the capability set in [T128] section 8.2.8. This capability
//    is sent by both client and server.

//    Instead of being specified by the Color Table Cache Capability Set, the existence of color table
//    caching is tied to support for the MemBlt (section 2.2.2.2.1.1.2.9) and Mem3Blt (section
//    2.2.2.2.1.1.2.10) Primary Drawing orders. If support for these orders is advertised in the Order
//    Capability Set (see [MS-RDPBCGR] section 2.2.7.1.3), the existence of a color table cache with
//    entries for six palettes is implied when palettized color is being used.

//     capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//       field MUST be set to CAPSTYPE_COLORCACHE (0x000A).

//     lengthCapability (2 bytes): A 16-bit, unsigned integer. The length, in bytes, of the capability
//       data, including the size of the capabilitySetType and lengthCapability fields.

//     colorTableCacheSize (2 bytes): A 16-bit, unsigned integer. The number of entries in the color
//       table cache (each entry stores a color table). This value MUST be ignored during capability
//       exchange and is assumed to be 0x0006.

//     pad2octets (2 bytes): A 16-bit, unsigned integer used as padding. Values in this field are
//       arbitrary and MUST be ignored.

enum {
    CAPLEN_COLORCACHE = 8
};


struct ColorCacheCaps : public Capability {
    uint16_t colorTableCacheSize;
    uint16_t pad2octets;
    ColorCacheCaps()
    : Capability(CAPSTYPE_COLORCACHE, CAPLEN_COLORCACHE)
    , colorTableCacheSize(0x0006)
    , pad2octets(0)
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->colorTableCacheSize);
        stream.out_uint16_le(this->pad2octets);
   }

    void recv(InStream & stream, uint16_t len)override {
        this->len = len;
        this->colorTableCacheSize = stream.in_uint16_le();
        this->pad2octets = stream.in_uint16_le();
    }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s ColorCache caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "ColorCache caps::colorTableCacheSize %u", this->colorTableCacheSize);
        LOG(LOG_INFO, "ColorCache caps::pad2octets %u", this->pad2octets);
    }
};


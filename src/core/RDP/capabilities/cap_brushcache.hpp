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

   RDP Capabilities : Brush Capability Set ([MS-RDPBCGR] section 2.2.7.1.7)

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

// 2.2.7.1.7 Brush Capability Set (TS_BRUSH_CAPABILITYSET)
// ======================================================
// The TS_BRUSH_CAPABILITYSET advertises client brush support. This capability is only sent from
// client to server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_BRUSH (15).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.

// brushSupportLevel (4 bytes): A 32-bit, unsigned integer. The maximum brush level
//    supported by the client.
//    +-------------------+--------------------------------------------------------------------------------+
//    | BRUSH_DEFAULT     | Support for solid-color and monochrome pattern brushes with no caching.        |
//    | 0x00000000        | This is an RDP 4.0 implementation.                                             |
//    +-------------------+--------------------------------------------------------------------------------+
//    | BRUSH_COLOR_8x8   | Ability to handle color brushes (4-bit or 8-bit in RDP 5.0; RDP 5.1, 5.2, 6.0, |
//    | 0x00000001        | 6.1, 7.0, 7.1, and 8.0 also support 16-bit and 24-bit) and caching. Brushes    |
//    |                   | are limited to 8-by-8 pixels.                                                  |
//    +-------------------+--------------------------------------------------------------------------------+
//    | BRUSH_COLOR_FULL  | Ability to handle color brushes (4-bit or 8-bit in RDP 5.0; RDP 5.1, 5.2, 6.0, |
//    | 0x00000002        | 6.1, 7.0, 7.1, and 8.0 also support 16-bit and 24-bit) and caching. Brushes    |
//    |                   | can have arbitrary dimensions.                                                 |
//    +-------------------+--------------------------------------------------------------------------------+


enum {
      BRUSH_DEFAULT
    , BRUSH_COLOR_8X8
    , BRUSH_COLOR_FULL
};

enum {
    CAPLEN_BRUSH = 8
};

struct BrushCacheCaps : public Capability {
    uint32_t brushSupportLevel;

    BrushCacheCaps()
    : Capability(CAPSTYPE_BRUSH, CAPLEN_BRUSH)
    , brushSupportLevel(BRUSH_DEFAULT) // By default, minimal
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->brushSupportLevel);
    }

    void recv(InStream & stream, uint16_t len)override {
        this->len = len;

        if (!stream.in_check_rem(4)){
            LOG(LOG_ERR, "Truncated BrushCacheCaps, need=4 remains=%zu",
                stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->brushSupportLevel = stream.in_uint32_le();
    }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s BrushCache caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "BrushCacheCaps caps::brushSupportLevel %u", this->brushSupportLevel);
    }
};


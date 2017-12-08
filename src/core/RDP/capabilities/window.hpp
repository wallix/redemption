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

   RDP Capabilities : Window List Capability Set ([MS-RDPERP] section 2.2.1.1.2)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "core/RDP/capabilities/virchan.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"

// [MS-RDPERP] - 2.2.1.1.2 Window List Capability Set
// ==================================================

// The Window List Capability Set is sent by the server in the Demand Active
//  PDU and by the client in the Confirm Active PDU, as specified in
//  [MS-RDPBCGR] section 2.2.1.13. It indicates that the client and server
//  are capable of communicating Windowing Alternate Secondary Drawing Orders
//  as extensions to the core RDP protocol drawing orders (see section
//  2.2.1.3).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       CapabilitySetType       |        LengthCapability       |
// +-------------------------------+-------------------------------+
// |                        WndSupportLevel                        |
// +---------------+-------------------------------+---------------+
// | NumIconCaches |      NumIconCacheEntries      |
// +---------------+-------------------------------+

// CapabilitySetType (2 bytes): An unsigned 16-bit integer. The type of
//  capability set. This field MUST be set to 0x0018 (CAPSTYPE_WINDOW).

// LengthCapability (2 bytes): An unsigned 16-bit integer. Specifies the
//  combined length of the CapabilitySetType, LengthCapability,
//  WndSupportLevel, NumIconCaches, and NumIconCacheEntries fields, in bytes.

// WndSupportLevel (4 bytes): An unsigned 32-bit integer. The windowing
//  support level. This field MUST be set to one of the following values.<2>

//  +-------------------------------+----------------------------------------+
//  | Value                         | Meaning                                |
//  +-------------------------------+----------------------------------------+
//  | TS_WINDOW_LEVEL_NOT_SUPPORTED | The client or server is not capable of |
//  | 0x00000000                    | supporting Windowing Alternate         |
//  |                               | Secondary Drawing Orders.              |
//  +-------------------------------+----------------------------------------+
//  | TS_WINDOW_LEVEL_SUPPORTED     | The client or server is capable of     |
//  | 0x00000001                    | supporting Windowing Alternate         |
//  |                               | Secondary Drawing Orders.              |
//  +-------------------------------+----------------------------------------+
//  | TS_WINDOW_LEVEL_SUPPORTED_EX  | The client or server is capable of     |
//  | 0x00000002                    | supporting Windowing Alternate         |
//  |                               | Secondary Drawing Orders and the       |
//  |                               | following flags:                       |
//  |                               |                                        |
//  |                               | * WINDOW_ORDER_FIELD_CLIENTAREASIZE    |
//  |                               |                                        |
//  |                               | * WINDOW_ORDER_FIELD_RPCONTENT         |
//  |                               |                                        |
//  |                               | * WINDOW_ORDER_FIELD_ROOTPARENT        |
//  +-------------------------------+----------------------------------------+

// NumIconCaches (1 byte): An unsigned 8-bit integer. The number of icon
//  caches requested by the server (Demand Active PDU) or supported by the
//  client (Confirm Active PDU).

//  The server maintains an icon cache and refers to it to avoid sending
//  duplicate icon information (see section 2.2.1.3.1.2.3). The client also
//  maintains an icon cache and refers to it when the server sends across a
//  Cached Icon Window Information Order.

// NumIconCacheEntries (2 bytes): An unsigned 16-bit integer. The number of
//  entries within each icon cache requested by the server (Demand Active
//  PDU) or supported by the client (Confirm Active PDU).

//  The server maintains an icon cache and refers to it to avoid sending
//  duplicate icon information (see section 2.2.1.3.1.2.3). The client also
//  maintains an icon cache and refers to it when the server sends across a
//  Cached Icon Window Information Order.

enum  {
        TS_WINDOW_LEVEL_NOT_SUPPORTED
      , TS_WINDOW_LEVEL_SUPPORTED
      , TS_WINDOW_LEVEL_SUPPORTED_EX
      };

enum {
    CAPLEN_WINDOW = 11
};

struct WindowListCaps : public Capability {
    uint32_t WndSupportLevel;
    uint8_t NumIconCaches;
    uint16_t NumIconCacheEntries;

    WindowListCaps()
    : Capability(CAPSTYPE_WINDOW, CAPLEN_WINDOW)
    , WndSupportLevel(TS_WINDOW_LEVEL_NOT_SUPPORTED) // from a specific list of values (see enum)
    , NumIconCaches(0)
    , NumIconCacheEntries(0)
    {
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->WndSupportLevel);
        stream.out_uint8(this->NumIconCaches);
        stream.out_uint16_le(this->NumIconCacheEntries);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->WndSupportLevel = stream.in_uint32_le();
        this->NumIconCaches = stream.in_uint8();
        this->NumIconCacheEntries = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s WindowsList caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     WindowsList caps::WndSupportLevel 0x%X", this->WndSupportLevel);
        LOG(LOG_INFO, "     WindowsList caps::NumIconCaches %u", this->NumIconCaches);
        LOG(LOG_INFO, "     WindowsList caps::NumIconCacheEntries %u", this->NumIconCacheEntries);
    }
};

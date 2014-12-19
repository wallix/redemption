/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_CORE_RDP_CHANNELS_RDPDR_HPP
#define REDEMPTION_CORE_RDP_CHANNELS_RDPDR_HPP

#include <stream.hpp>

namespace rdpdr {

// +----------------------------------------+-------------------------------------------------------+
// | Value                                  | Meaning                                               |
// +----------------------------------------+-------------------------------------------------------+
// | 0x496E PAKID_CORE_SERVER_ANNOUNCE      | Server Announce Request                               |
// +----------------------------------------+-------------------------------------------------------+
// | 0x4343  PAKID_CORE_CLIENTID_CONFIRM    | Client Announce Reply and Server Client ID Confirm    |
// +----------------------------------------+-------------------------------------------------------+
// | 0x434E  PAKID_CORE_CLIENT_NAME         | Client Name Request                                   |
// +----------------------------------------+-------------------------------------------------------+
// | 0x4441 PAKID_CORE_DEVICELIST_ANNOUNCE  | Client Device List Announce Request                   |
// +----------------------------------------+-------------------------------------------------------+
// | 0x6472 PAKID_CORE_DEVICE_REPLY         | Server Device Announce Response                       |
// +----------------------------------------+-------------------------------------------------------+
// | 0x4952 PAKID_CORE_DEVICE_IOREQUEST     | Device I/O Request                                    |
// +----------------------------------------+-------------------------------------------------------+
// | 0x4943 PAKID_CORE_DEVICE_IOCOMPLETION  | Device I/O Response                                   |
// +----------------------------------------+-------------------------------------------------------+
// | 0x5350 PAKID_CORE_SERVER_CAPABILITY    | Server Core Capability Request                        |
// +----------------------------------------+-------------------------------------------------------+
// | 0x4350 PAKID_CORE_CLIENT_CAPABILITY    | Client Core Capability Response                       |
// +----------------------------------------+-------------------------------------------------------+
// | 0x444D PAKID_CORE_DEVICELIST_REMOVE    | Client Drive Device List Remove                       |
// +----------------------------------------+-------------------------------------------------------+
// | 0x5043 PAKID_PRN_CACHE_DATA            | Add Printer Cachedata                                 |
// +----------------------------------------+-------------------------------------------------------+
// | 0x554C PAKID_CORE_USER_LOGGEDON        | Server User Logged On                                 |
// +----------------------------------------+-------------------------------------------------------+
// | 0x5543 PAKID_PRN_USING_XPS             | Server Printer Set XPS Mode                           |
// +----------------------------------------+-------------------------------------------------------+

enum class PacketId : uint16_t {
    PAKID_CORE_SERVER_ANNOUNCE     = 0x496e,
    PAKID_CORE_CLIENTID_CONFIRM    = 0x4343,
    PAKID_CORE_CLIENT_NAME         = 0x434e,
    PAKID_CORE_DEVICELIST_ANNOUNCE = 0x4441,
    PAKID_CORE_DEVICE_REPLY        = 0x6472,
    PAKID_CORE_DEVICE_IOREQUEST    = 0x4952,
    PAKID_CORE_DEVICE_IOCOMPLETION = 0x4943,
    PAKID_CORE_SERVER_CAPABILITY   = 0x5350,
    PAKID_CORE_CLIENT_CAPABILITY   = 0x4350,
    PAKID_CORE_DEVICELIST_REMOVE   = 0x444d,
    PAKID_PRN_CACHE_DATA           = 0x5043,
    PAKID_CORE_USER_LOGGEDON       = 0x554c,
    PAKID_PRN_USING_XPS            = 0x5543
};


// +----------------------------------------+-----------------------------------------------------------+
// Value                    |  Meaning                                                                  |
// +------------------------+---------------------------------------------------------------------------+
// 0x4472 RDPDR_CTYP_CORE   |  Device redirector core component; most of the packets                    |
//                             in this protocol are sent under this component ID.                       |
// +------------------------+---------------------------------------------------------------------------+
// 0x5052 RDPDR_CTYP_PRN    |  Printing component. The packets that use this ID are                     |
//                             typically about printer cache management and identifying XPS printers.   |
// +------------------------+---------------------------------------------------------------------------+

enum class Component : uint16_t {
    RDPDR_CTYP_CORE = 0x4472,
    RDPDR_CTYP_PRT  = 0x5052
};

struct SharedHeader {
    Component component;
    PacketId packet_id;

    static PacketId read_packet_id(Stream & stream) {
        stream.p += sizeof(component);
        return static_cast<PacketId>(stream.in_uint16_le());
    }
};

inline uint16_t read_num_capability(Stream & stream) {
    uint16_t num_capabilities = stream.in_uint16_le();
    stream.p += 2;
    return num_capabilities;
}

enum class CapabilityType : uint16_t {
    general     = 1,
    printer     = 2,
    port        = 3,
    drive       = 4,
    smartcard   = 5
};

struct CapabilityHeader {
    CapabilityType type;
    uint16_t length;
    uint32_t version;
};

}

#endif

/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    RDP Capabilities : Remote Programs Capability Set ([MS-RDPERP] section 2.2.1.1.1)
*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// [MS-RDPERP] - 2.2.1.1.1 Remote Programs Capability Set
// ======================================================

// The Remote Programs Capability Set is sent by the server in the Demand
//  Active PDU and by the client in the Confirm Active PDU, as specified in
//  [MS-RDPBCGR] section 2.2.1.13. It indicates that the client and server
//  are capable of communicating RAIL PDUs over the RAIL static virtual
//  channel.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       CapabilitySetType       |        LengthCapability       |
// +-------------------------------+-------------------------------+
// |                        RailSupportLevel                       |
// +---------------------------------------------------------------+

// CapabilitySetType (2 bytes): An unsigned 16-bit integer. The type of the
//  capability set. This field MUST be set to 0x0017 (CAPSTYPE_RAIL).

// LengthCapability (2 bytes): An unsigned 16-bit integer. The combined
//  length of the CapabilitySetType, LengthCapability, and RailSupportLevel
//  fields, in bytes.

// RailSupportLevel (4 bytes): A 4 byte bitfield specifying support for
//  Remote Programs and the Docked Language Bar for Remote Programs. <1>

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |S|L|H|I|C|M|K|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//  Where the bits are defined as:

//   +---------------------------------------------------+-----------------------------+
//   | Value                                             | Description                 |
//   +---------------------------------------------------+-----------------------------+
//   | S                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_SUPPORTED                           | client/server is capable of |
//   |                                                   | supporting Remote Programs; |
//   |                                                   | set to 0 otherwise.         |
//   +---------------------------------------------------+-----------------------------+
//   | L                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED            | client/server is capable of |
//   |                                                   | supporting Docked Language  |
//   |                                                   | Bar for Remote Programs;    |
//   |                                                   | set to 0 otherwise. This    |
//   |                                                   | flag MUST be set to 0 if    |
//   |                                                   | TS_RAIL_LEVEL_SUPPORTED is  |
//   |                                                   | 0.                          |
//   +---------------------------------------------------+-----------------------------+
//   | H                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_SHELL_INTEGRATION_SUPPORTED         | client/server is capable of |
//   |                                                   | supporting extended shell   |
//   |                                                   | integration like tabbed     |
//   |                                                   | windows and overlay icons   |
//   |                                                   | for Remote Programs; set to |
//   |                                                   | 0 otherwise. This flag MUST |
//   |                                                   | be set to 0 if              |
//   |                                                   | TS_RAIL_LEVEL_SUPPORTED is  |
//   |                                                   | 0.                          |
//   +---------------------------------------------------+-----------------------------+
//   | I                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED         | client/server is capable of |
//   |                                                   | supporting syncing          |
//   |                                                   | language/IME changes for    |
//   |                                                   | Remote Programs; set to 0   |
//   |                                                   | otherwise.                  |
//   +---------------------------------------------------+-----------------------------+
//   | C                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_SERVER_TO_CLIENT_IME_SYNC_SUPPORTED | client/server is capable of |
//   |                                                   | supporting syncing IME      |
//   |                                                   | changes originating at the  |
//   |                                                   | server for Remote Programs; |
//   |                                                   | set to 0 otherwise. This    |
//   |                                                   | flag MUST be set to 0 if    |
//   |                                                   | TS_RAIL_LEVEL_SUPPORTED is  |
//   |                                                   | 0.                          |
//   +---------------------------------------------------+-----------------------------+
//   | M                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_HIDE_MINIMIZED_APPS_SUPPORTED       | client/server supports      |
//   |                                                   | hiding minimized windows of |
//   |                                                   | Remote Programs on the      |
//   |                                                   | server; set to 0 otherwise. |
//   |                                                   | This flag MUST be set to 0  |
//   |                                                   | if TS_RAIL_LEVEL_SUPPORTED  |
//   |                                                   | is 0.                       |
//   +---------------------------------------------------+-----------------------------+
//   | K                                                 | Set to 1 if the             |
//   | TS_RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED           | client/server supports      |
//   |                                                   | syncing per-window cloak    |
//   |                                                   | state changes originating   |
//   |                                                   | on the client for Remote    |
//   |                                                   | Programs; set to 0          |
//   |                                                   | otherwise. This flag MUST   |
//   |                                                   | be set to 0 if              |
//   |                                                   | TS_RAIL_LEVEL_SUPPORTED is  |
//   |                                                   | 0.                          |
//   +---------------------------------------------------+-----------------------------+

enum {
      TS_RAIL_LEVEL_SUPPORTED                           = 0x01
    , TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED            = 0x02
    , TS_RAIL_LEVEL_SHELL_INTEGRATION_SUPPORTED         = 0x04
    , TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED         = 0x08
    , TS_RAIL_LEVEL_SERVER_TO_CLIENT_IME_SYNC_SUPPORTED = 0x10
    , TS_RAIL_LEVEL_HIDE_MINIMIZED_APPS_SUPPORTED       = 0x20
    , TS_RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED           = 0x40
};

enum {
    CAPLEN_RAIL = 8
};

struct RailCaps : public Capability {
    uint32_t RailSupportLevel;

    RailCaps()
    : Capability(CAPSTYPE_RAIL, CAPLEN_RAIL)
    , RailSupportLevel(0) // Only the two less significant bit are used (as flags.)
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->RailSupportLevel);
    }

    void recv(InStream & stream, uint16_t len)override {
        this->len = len;
        this->RailSupportLevel = stream.in_uint32_le();
    }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s Rail caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "Rail caps::RailSupportLevel 0x%X", this->RailSupportLevel);
    }
};

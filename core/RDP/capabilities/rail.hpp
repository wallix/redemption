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

   RDP Capabilities : Remote Programs Capability Set ([MS-RDPERP] section 2.2.1.1.1)

*/

#if !defined(__RDP_CAPABILITIES_RAIL_HPP__)
#define __RDP_CAPABILITIES_RAIL_HPP__

// 2.2.1.1.1 Remote Programs Capability Set
// ========================================
//  The Remote Programs Capability Set is sent by the server in the Demand Active PDU and by the
//  client in the Confirm Active PDU, as specified in [MS-RDPBCGR] section 2.2.1.13. It indicates that
//  the client and server are capable of communicating RAIL PDUs over the RAIL static virtual channel.

// RailSupportLevel (4 bytes): A 4 byte bitfield specifying support for Remote Programs and the
//    Docked Language Bar for Remote Programs. <1>
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |S|L|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    Where the bits are defined as:
//    +----------------------------------------+----------------------------------------------+
//    | S                                      | Set to 1 if the client/server is capable of  |
//    | TS_RAIL_LEVEL_SUPPORTED                | supporting Remote Programs; set to 0         |
//    |                                        | otherwise.                                   |
//    +----------------------------------------+----------------------------------------------+
//    | L                                      | Set to 1 if the client/server is capable of  |
//    | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED | supporting Docked Language Bar for Remote    |
//    |                                        | Programs; set to 0 otherwise. This flag MUST |
//    |                                        | be set to 0 if TS_RAIL_LEVEL_SUPPORTED is    |
//    |                                        | 0.                                           |
//    +----------------------------------------+----------------------------------------------+

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

    void emit(Stream & stream){
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->RailSupportLevel);
    }

    void recv(Stream & stream, uint16_t len){
        this->len = len;
        this->RailSupportLevel = stream.in_uint32_le();
    }

    void log(const char * msg){
        LOG(LOG_INFO, "%s Rail caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "Rail caps::RailSupportLevel %u", this->RailSupportLevel);
    }
};

#endif

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

   GCC Conference User Data : Server to Client Core (SC_CORE)

*/

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_CORE_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_SC_CORE_HPP__


// 2.2.1.4.2 Server Core Data (TS_UD_SC_CORE)
// ==========================================

// The TS_UD_SC_CORE data block contains core server connection-related
// information.

// header (4 bytes): GCC user data block header, as specified in User Data
//  Header (section 2.2.1.3.1). The User Data Header type field MUST be set to
//  SC_CORE (0x0C01).

// version (4 bytes): A 32-bit, unsigned integer. The server version number for
//  the RDP. The major version number is stored in the high two bytes, while the
//  minor version number is stored in the low two bytes.

// 0x00080001 RDP 4.0 servers
// 0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 servers

// If the server advertises a version number greater than or equal to 0x00080004,
// it MUST support a maximum length of 512 bytes for the UserName field in the
// Info Packet (section 2.2.1.11.1.1).

// clientRequestedProtocols (4 bytes): A 32-bit, unsigned integer that contains
//  the flags sent by the client in the requestedProtocols field of the RDP
//  Negotiation Request (section 2.2.1.1.1). In the event that an RDP
//  Negotiation Request was not received from the client, this field MUST be
//  initialized to PROTOCOL_RDP (0).

// Exemple:
//01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12 bytes
//04 00 08 00 -> TS_UD_SC_CORE::version = 0x0080004
//00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

struct SCCoreGccUserData {
    uint16_t userDataType;
    uint16_t length;
    uint32_t version;
    uint32_t clientRequestedProtocol;

    SCCoreGccUserData()
    : userDataType(SC_CORE)
    , length(12) // default: everything except serverSelectedProtocol
    , version(0x00080001)  // RDP version. 1 == RDP4, 4 == RDP5.
    , clientRequestedProtocol(0)
    {
    }


    void emit(Stream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->version);
        stream.out_uint32_le(this->clientRequestedProtocol);
    }

    void recv(Stream & stream, uint16_t length)
    {
        this->length = length;
        this->version = stream.in_uint32_le();
        this->clientRequestedProtocol = stream.in_uint32_le();
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data SC_CORE (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "cs_core::header::version [%04x] %s", this->version,
              (this->version==0x00080001) ? "RDP 4 client"
             :(this->version==0x00080004) ? "RDP 5.0, 5.1, 5.2, and 6.0 clients)"
                                          : "Unknown client");
        LOG(LOG_INFO, "cs_core::clientRequestedProtocol  = %u", this->clientRequestedProtocol);
    }
};

#endif

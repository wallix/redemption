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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   MCS Connect Initial PDU with GCC Conference User Data


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

TODO("Create SCCoreIn and SCCoreOut classes (on the model of SecIn/SecOut for Sec layer), or an SCCore object with emit() and receive() following the model of RDPOrders primitives")

static inline void parse_mcs_data_sc_core(Stream & stream, int & use_rdp5)
{
    LOG(LOG_INFO, "SC_CORE\n");
    uint32_t rdp_version = stream.in_uint32_le();
    LOG(LOG_DEBUG, "Remote RDP server supports version %s (was %s)",
                ((rdp_version==0x0080001)?"RDP4":"RDP5"),
                ((use_rdp5)?"RDP5":"RDP4"));
    LOG(LOG_INFO, "So WTF?\n");
    if (0x0080001 == rdp_version){ // can't use rdp5
        use_rdp5 = 0;
        TODO(" why caring of server_depth here ? Quite strange")
        //        this->server_depth = 8;
    }
}

//01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12
//bytes

//04 00 08 00 -> TS_UD_SC_CORE::version = 0x0008004
//00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

static inline void out_mcs_data_sc_core(Stream & stream, const bool use_rdp5)
{
    LOG(LOG_INFO, "SC_CORE\n");
    // length, including tag and length fields
    stream.out_uint16_le(SC_CORE);
    stream.out_uint16_le(12); /* len */
    const uint32_t rdp_version = use_rdp5?0x0080004:0x0080001;
    LOG(LOG_DEBUG, "RDP proxy server supports version %s (was %s)\n",
            (rdp_version==0x0080001)?"RDP4":"RDP5",
            ((use_rdp5)?"RDP5":"RDP4"));
    stream.out_uint32_le(rdp_version);
    const uint32_t clientRequestedProtocols = 0;
    stream.out_uint32_le(clientRequestedProtocols);
}

#endif

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

   RDP Capabilities : Virtual Channel Capability Set ([MS-RDPBCGR] section 2.2.7.1.10)

*/


#pragma once

#include "common.hpp"
#include "utils/stream.hpp"

// 2.2.7.1.10 Virtual Channel Capability Set (TS_VIRTUALCHANNEL_CAPABILITYSET)
// ===========================================================================

// The TS_VIRTUALCHANNEL_CAPABILITYSET structure is used to advertise virtual channel support
// characteristics. This capability is sent by both client and server.

// flags (4 bytes): A 32-bit, unsigned integer. Virtual channel compression flags.
//    +--------------------+----------------------------------------------------------------------------- +
//    | VCCAPS_NO_COMPR    |  Virtual channel compression is not supported.                               |
//    | 0x00000000         |                                                                              |
//    +--------------------+----------------------------------------------------------------------------- +
//    | VCCAPS_COMPR_SC    | Indicates to the server that virtual channel compression is supported by     |
//    | 0x00000001         | the client for server-to-client traffic. The highest compression level       |
//    |                    | supported by the client is advertised in the Client Info PDU (section        |
//    |                    | 2.2.1.11).                                                                   |
//    +--------------------+----------------------------------------------------------------------------- +
//    | VCCAPS_COMPR_CS_8K | Indicates to the client that virtual channel compression is supported by     |
//    | 0x00000002         | the server for client-to-server traffic (the compression level is limited to |
//    |                    | RDP 4.0 bulk compression).                                                   |
//    +--------------------+----------------------------------------------------------------------------- +

// VCChunkSize (4 bytes): A 32-bit unsigned integer. When sent from server to client, this field
//    contains the maximum allowed size of a virtual channel chunk. When sent from client to
//    server, the value in this field is ignored by the server; the server determines the maximum
//    virtual channel chunk size. This value MUST be greater than or equal to
//    CHANNEL_CHUNK_LENGTH and less than or equal to 16256.

enum  {
        VCCAPS_NO_COMPR
      , VCCAPS_COMPR_SC
      , VCCAPS_COMPR_CS_8K
      };

enum {
    CAPLEN_VIRTUALCHANNEL = 12
};

struct VirtualChannelCaps : public Capability {
    uint32_t flags;
    uint32_t VCChunkSize;

    VirtualChannelCaps()
    : Capability(CAPSTYPE_VIRTUALCHANNEL, CAPLEN_VIRTUALCHANNEL)
    , flags(VCCAPS_NO_COMPR) // from a specific list of values (see enum)
    , VCChunkSize(0)
    {
    }

    void emit(OutStream & stream)override {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->flags);
        stream.out_uint32_le(this->VCChunkSize);
    }

    void recv(InStream & stream, uint16_t len)override {
        this->len = len;
        this->flags = stream.in_uint32_le();
        this->VCChunkSize = stream.in_uint32_le();
    }

    void log(const char * msg)override {
        LOG(LOG_INFO, "%s VirtualChannel caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "VirtualChannel caps::flags %u", this->flags);
        LOG(LOG_INFO, "VirtualChannel caps::VCChunkSize %u", this->VCChunkSize);
    }
};


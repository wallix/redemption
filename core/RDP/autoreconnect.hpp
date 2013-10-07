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
*/

#ifndef _REDEMPTION_CORE_RDP_AUTO_RECONNECT_HPP_
#define _REDEMPTION_CORE_RDP_AUTO_RECONNECT_HPP_

namespace RDP {


// 2.2.4.2 Server Auto-Reconnect Packet (ARC_SC_PRIVATE_PACKET)
// ============================================================
// The ARC_SC_PRIVATE_PACKET structure contains server-supplied information
//  used to seamlessly re-establish a connection to a server after network
//  interruption. It is sent as part of the Save Session Info PDU logon
//  information (see section 2.2.10.1.1.4).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             cbLen                             |
// +---------------------------------------------------------------+
// |                            Version                            |
// +---------------------------------------------------------------+
// |                            LogonId                            |
// +---------------------------------------------------------------+
// |                         ArcRandomBits                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cbLen (4 bytes): A 32-bit, unsigned integer. The length in bytes of the
//  Server Auto-Reconnect packet. This field MUST be set to 0x0000001C (28
//  bytes).

// Version (4 bytes): A 32-bit, unsigned integer. The value representing the
//  auto-reconnect version.

// +--------------------------+------------------------------+
// | Value                    | Meaning                      |
// +--------------------------+------------------------------+
// | AUTO_RECONNECT_VERSION_1 | Version 1 of auto-reconnect. |
// | 0x00000001               |                              |
// +--------------------------+------------------------------+

// LogonId (4 bytes): A 32-bit, unsigned integer. The session identifier for
//  reconnection.

// ArcRandomBits (16 bytes): Byte buffer containing a 16-byte, random number
//  generated as a key for secure reconnection (see section 5.5).

struct ServerAutoReconnectPacket_Recv {
    uint32_t cbLen;
    uint32_t Version;
    uint32_t LogonId;
    uint8_t  ArcRandomBits[16];

    ServerAutoReconnectPacket_Recv(Stream & stream) :
    cbLen(0),
    Version(0),
    LogonId(0) {
        memset(ArcRandomBits, 0, sizeof(ArcRandomBits));

        const unsigned expected = 4 +   // cbLen(4)
                                  4 +   // Version(4)
                                  4 +   // LogonId(4)
                                  16;   // ArcRandomBits(16)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Server Auto-Reconnect Packet (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->cbLen   = stream.in_uint32_le();
        this->Version = stream.in_uint32_le();
        this->LogonId = stream.in_uint32_le();

        stream.in_copy_bytes(this->ArcRandomBits, sizeof(this->ArcRandomBits));

        LOG(LOG_INFO, "LogonId=%d", this->LogonId);
        hexdump(this->ArcRandomBits, sizeof(this->ArcRandomBits));
    }
};  // ServerAutoReconnectPacket_Recv

}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_AUTO_RECONNECT_HPP_

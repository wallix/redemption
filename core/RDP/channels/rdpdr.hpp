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

    inline void emit(Stream & stream) const {
        stream.out_uint16_le(static_cast<uint16_t>(this->component));
        stream.out_uint16_le(static_cast<uint16_t>(this->packet_id));
    }

    inline void receive(Stream & stream) {
        this->component = static_cast<Component>(stream.in_uint16_le());
        this->packet_id = static_cast<PacketId>(stream.in_uint16_le());
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

// [MS-RDPEFS] - 2.2.1.3 Device Announce Header (DEVICE_ANNOUNCE)
// ==============================================================

// This header is embedded in the Client Device List Announce message. Its
//  purpose is to describe different types of devices.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           DeviceType                          |
// +---------------------------------------------------------------+
// |                            DeviceId                           |
// +---------------------------------------------------------------+
// |                        PreferredDosName                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                        DeviceDataLength                       |
// +---------------------------------------------------------------+
// |                     DeviceData (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceType (4 bytes): A 32-bit unsigned integer that identifies the device
//  type. This field MUST be set to one of the following values.

//  +-----------------------+----------------------+
//  | Value                 | Meaning              |
//  +-----------------------+----------------------+
//  | RDPDR_DTYP_SERIAL     | Serial port device   |
//  | 0x00000001            |                      |
//  +-----------------------+----------------------+
//  | RDPDR_DTYP_PARALLEL   | Parallel port device |
//  | 0x00000002            |                      |
//  +-----------------------+----------------------+
//  | RDPDR_DTYP_PRINT      | Printer device       |
//  | 0x00000004            |                      |
//  +-----------------------+----------------------+
//  | RDPDR_DTYP_FILESYSTEM | File system device   |
//  | 0x00000008<3>         |                      |
//  +-----------------------+----------------------+
//  | RDPDR_DTYP_SMARTCARD  | Smart card device    |
//  | 0x00000020<4>         |                      |
//  +-----------------------+----------------------+

enum {
      RDPDR_DTYP_SERIAL     = 0x00000001
    , RDPDR_DTYP_PARALLEL   = 0x00000002
    , RDPDR_DTYP_PRINT      = 0x00000004
    , RDPDR_DTYP_FILESYSTEM = 0x00000008
    , RDPDR_DTYP_SMARTCARD  = 0x00000020
};

// DeviceId (4 bytes): A 32-bit unsigned integer that specifies a unique ID
//  that identifies the announced device. This ID MUST be reused if the
//  device is removed by means of the Client Drive Device List Remove packet
//  specified in section 2.2.3.2.

// PreferredDosName (8 bytes): A string of ASCII characters with a maximum
//  length of eight characters that represent the name of the device as it
//  appears on the client. This field MUST not be null-terminated if the
//  device name is 8 characters long. The following characters are considered
//  invalid for the PreferredDosName field:

//  <, >, ", /, \, |

//  If any of these characters are present, the DR_CORE_DEVICE_ANNOUNC_RSP
//  packet for this device (section 2.2.2.1) will be sent with
//  STATUS_ACCESS_DENIED set in the ResultCode field.

//  If DeviceType is set to RDPDR_DTYP_SMARTCARD, the PreferredDosName MUST
//  be set to "SCARD".

//  Note A column character, ":", is valid only when present at the end of
//  the PreferredDosName field, otherwise it is also considered invalid.

// DeviceDataLength (4 bytes): A 32-bit unsigned integer that specifies the
//  number of bytes in the DeviceData field.

// DeviceData (variable): A variable-length byte array whose size is
//  specified by the DeviceDataLength field. The content depends on the
//  DeviceType field. See [MS-RDPEPC] section 2.2.2.1 for the printer device
//  type. See [MS-RDPESP] section 2.2.2.1 for the serial and parallel port
//  device types. See section 2.2.3.1 of this protocol for the file system
//  device type. For a smart card device, the DeviceDataLength field MUST be
//  set to zero. See [MS-RDPESC] for details about the smart card device
//  type.

class DeviceAnnounceHeader {
    uint32_t DeviceType_ = RDPDR_DTYP_SERIAL;
    uint32_t DeviceId_   = 0;

    uint8_t  PreferredDosName[8 /* PreferredDosName(8) */ + 1] = { 0 };

    StaticStream device_data;

public:
    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DeviceType_);
        stream.out_uint32_le(this->DeviceId_);

        stream.out_copy_bytes(this->PreferredDosName, 8 /* PreferredDosName(8) */);

        stream.out_uint32_le(this->device_data.get_capacity());

        stream.out_copy_bytes(this->device_data.get_data(), this->device_data.get_capacity());
    }

    inline void receive(Stream & stream) {
        this->DeviceType_ = stream.in_uint32_le();
        this->DeviceId_   = stream.in_uint32_le();

        stream.in_copy_bytes(this->PreferredDosName, 8 /* PreferredDosName(8) */);
        this->PreferredDosName[8 /* PreferredDosName(8) */ ] = '\0';

        const uint32_t DeviceDataLength = stream.in_uint32_le();

        this->device_data.resize(stream.p, DeviceDataLength);
        stream.in_skip_bytes(DeviceDataLength);
    }

    inline uint32_t DeviceType() const { return this->DeviceType_; }

    inline uint32_t DeviceId() const { return this->DeviceId_; }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "DeviceAnnounceHeader: DeviceType=%u DeviceId=%u PreferredDosName=\"%s\"",
            this->DeviceType_, this->DeviceId_, this->PreferredDosName);
        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
        if (level == LOG_INFO) {
            hexdump(this->device_data.get_data(), this->device_data.get_capacity());
        }
    }
};

// [MS-RDPEFS] - 2.2.2.1 Server Device Announce Response
//  (DR_CORE_DEVICE_ANNOUNCE_RSP)
// =====================================================

// The server responds to a Client Device List Announce Request with this
//  message.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                            DeviceId                           |
// +---------------------------------------------------------------+
// |                           ResultCode                          |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_DEVICE_REPLY.

// DeviceId (4 bytes): A 32-bit unsigned integer. This ID MUST be the same as
//  one of the IDs specified in the Client Device List Announce Request
//  message. The server sends a separate Server Device Announce Response
//  message for each announced device.

// ResultCode (4 bytes): A 32-bit unsigned integer that specifies the
//  NTSTATUS code that indicates the success or failure of device
//  initialization. NTSTATUS codes are specified in [MS-ERREF] section 2.3.

class ServerDeviceAnnounceResponse {
    uint32_t DeviceId   = 0;
    uint32_t ResultCode = 0;

public:
    ServerDeviceAnnounceResponse() = default;

    ServerDeviceAnnounceResponse(uint32_t device_id, uint32_t result_code) :
        DeviceId(device_id), ResultCode(result_code) {}

    inline void emit(Stream & stream) {
        stream.out_uint32_le(this->DeviceId);
        stream.out_uint32_le(this->ResultCode);
    }

    inline void receive(Stream & stream) {
        this->DeviceId   = stream.in_uint32_le();
        this->ResultCode = stream.in_uint32_le();
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "ServerDeviceAnnounceResponse: DeviceId=%u ResultCode=0x%08X",
            this->DeviceId, this->ResultCode);
        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};

// [MS-RDPEFS] - 2.2.2.9 Client Device List Announce Request
//  (DR_CORE_DEVICELIST_ANNOUNCE_REQ)
// =========================================================

// The client announces the list of devices to redirect on the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                          DeviceCount                          |
// +---------------------------------------------------------------+
// |                     DeviceList (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_DEVICELIST_ANNOUNCE.

// DeviceCount (4 bytes): A 32-bit unsigned integer that specifies the number
//  of items in the DeviceList array.

// DeviceList (variable): A variable-length array of DEVICE_ANNOUNCE (section
//  2.2.1.3) headers. This field specifies a list of devices that are being
//  announced. The number of entries is specified by the DeviceCount field.
//  There is no alignment padding between individual DEVICE_ANNOUNCE
//  structures. They are ordered sequentially within this packet.

}

#endif

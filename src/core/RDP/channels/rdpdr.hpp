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
*   Copyright (C) Wallix 2010-2018
*   Author(s): Jonathan Poelen, Clément Moroldo, Christophe Grosjean, Raphaël Zhou
*/

#pragma once

#include "core/error.hpp"
#include "core/ERREF/ntstatus.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr_completion_id_manager.hpp"
#include "core/SMB2/MessageSyntax.hpp"

#include "utils/id_manager.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/utf.hpp"

#include <algorithm>
#include <cinttypes>
#include <vector>

namespace rdpdr {

// [MS-RDPEFS] - 2.2.1.1 Shared Header (RDPDR_HEADER)
// ==================================================

// This header is present at the beginning of every message in this protocol.
//  The purpose of this header is to describe the type of the message.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           Component           |            PacketId           |
// +-------------------------------+-------------------------------+

// Component (2 bytes): A 16-bit unsigned integer that identifies the
//  component to which the packet is sent. This field MUST be set to one of
//  the following values.

//  +-----------------+-------------------------------------------------------+
//  | Value           | Meaning                                               |
//  +-----------------+-------------------------------------------------------+
//  | RDPDR_CTYP_CORE | Device redirector core component; most of the packets |
//  | 0x4472          | in this protocol are sent under this component ID.    |
//  +-----------------+-------------------------------------------------------+
//  | RDPDR_CTYP_PRN  | Printing component. The packets that use this ID are  |
//  | 0x5052          | typically about printer cache management and          |
//  |                 | identifying XPS printers.                             |
//  +-----------------+-------------------------------------------------------+

enum Component : uint16_t {
    RDPDR_CTYP_CORE = 0x4472,
    RDPDR_CTYP_PRT  = 0x5052
};

// PacketId (2 bytes): A 16-bit unsigned integer. The PacketId field is a
//  unique ID that identifies the packet function. This field MUST be set to
//  one of the following values.

//  +--------------------------------+-----------------------------------------+
//  | Value                          | Meaning                                 |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_SERVER_ANNOUNCE     | Server Announce Request, as specified   |
//  | 0x496E                         | in section 2.2.2.2.                     |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_CLIENTID_CONFIRM    | Client Announce Reply and Server Client |
//  | 0x4343                         | ID Confirm, as specified in sections    |
//  |                                | 2.2.2.3 and 2.2.2.6.                    |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_CLIENT_NAME         | Client Name Request, as specified in    |
//  | 0x434E                         | section 2.2.2.4.                        |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_DEVICELIST_ANNOUNCE | Client Device List Announce Request, as |
//  | 0x4441                         | specified in section 2.2.2.9.           |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_DEVICE_REPLY        | Server Device Announce Response, as     |
//  | 0x6472                         | specified in section 2.2.2.1.           |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_DEVICE_IOREQUEST    | Device I/O Request, as specified in     |
//  | 0x4952                         | section 2.2.1.4.                        |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_DEVICE_IOCOMPLETION | Device I/O Response, as specified in    |
//  | 0x4943                         | section 2.2.1.5.                        |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_SERVER_CAPABILITY   | Server Core Capability Request, as      |
//  | 0x5350                         | specified in section 2.2.2.7.           |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_CLIENT_CAPABILITY   | Client Core Capability Response, as     |
//  | 0x4350                         | specified in section 2.2.2.8.           |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_DEVICELIST_REMOVE   | Client Drive Device List Remove, as     |
//  | 0x444D                         | specified in section 2.2.3.2.           |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_PRN_CACHE_DATA           | Add Printer Cachedata, as specified in  |
//  | 0x5043                         | [MS-RDPEPC] section 2.2.2.3.            |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_CORE_USER_LOGGEDON       | Server User Logged On, as specified in  |
//  | 0x554C                         | section 2.2.2.5.                        |
//  +--------------------------------+-----------------------------------------+
//  | PAKID_PRN_USING_XPS            | Server Printer Set XPS Mode, as         |
//  | 0x5543                         | specified in [MS-RDPEPC] section        |
//  |                                | 2.2.2.2.                                |
//  +--------------------------------+-----------------------------------------+

enum PacketId : uint16_t {
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

struct SharedHeader {
    Component component = Component::RDPDR_CTYP_CORE;
    PacketId  packet_id = PacketId::PAKID_CORE_SERVER_ANNOUNCE;

    explicit SharedHeader() = default;

    explicit SharedHeader(Component component, PacketId  packet_id)
    : component(component)
    , packet_id(packet_id) {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(static_cast<uint16_t>(this->component));
        stream.out_uint16_le(static_cast<uint16_t>(this->packet_id));
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;  // Component(2) + PacketId(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated SharedHeader: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->component = static_cast<Component>(stream.in_uint16_le());
        this->packet_id = static_cast<PacketId>(stream.in_uint16_le());
    }

    static const char * get_Component_name(uint16_t component) {
        switch (component) {
            case Component::RDPDR_CTYP_CORE: return "RDPDR_CTYP_CORE";
            case Component::RDPDR_CTYP_PRT:  return "RDPDR_CTYP_PRT";
        }

        return "<unknown>";
    }

    static const char * get_PacketId_name(uint16_t packet_id) {
        switch (packet_id) {
            case PacketId::PAKID_CORE_SERVER_ANNOUNCE:     return "PAKID_CORE_SERVER_ANNOUNCE";
            case PacketId::PAKID_CORE_CLIENTID_CONFIRM:    return "PAKID_CORE_CLIENTID_CONFIRM";
            case PacketId::PAKID_CORE_CLIENT_NAME:         return "PAKID_CORE_CLIENT_NAME";
            case PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE: return "PAKID_CORE_DEVICELIST_ANNOUNCE";
            case PacketId::PAKID_CORE_DEVICE_REPLY:        return "PAKID_CORE_DEVICE_REPLY";
            case PacketId::PAKID_CORE_DEVICE_IOREQUEST:    return "PAKID_CORE_DEVICE_IOREQUEST";
            case PacketId::PAKID_CORE_DEVICE_IOCOMPLETION: return "PAKID_CORE_DEVICE_IOCOMPLETION";
            case PacketId::PAKID_CORE_SERVER_CAPABILITY:   return "PAKID_CORE_SERVER_CAPABILITY";
            case PacketId::PAKID_CORE_CLIENT_CAPABILITY:   return "PAKID_CORE_CLIENT_CAPABILITY";
            case PacketId::PAKID_CORE_DEVICELIST_REMOVE:   return "PAKID_CORE_DEVICELIST_REMOVE";
            case PacketId::PAKID_PRN_CACHE_DATA:           return "PAKID_PRN_CACHE_DATA";
            case PacketId::PAKID_CORE_USER_LOGGEDON:       return "PAKID_CORE_USER_LOGGEDON";
            case PacketId::PAKID_PRN_USING_XPS:            return "PAKID_PRN_USING_XPS";
        }

        return "<unknown>";
    }

    constexpr static size_t size() {
        return 4;   // Component(2) + PacketId(2)
    }

    void log(int level) const {
        LOG(level, "SharedHeader: Component=%s(0x%X) PacketId=%s(0x%X)",
            this->get_Component_name(this->component),
            unsigned(this->component),
            this->get_PacketId_name(this->packet_id),
            unsigned(this->packet_id));
    }

    void log() const {
        LOG(LOG_INFO, "     Shared Header:");
        LOG(LOG_INFO, "          * Component = 0x%04x (2 bytes): %s", this->component, get_Component_name(this->component));
        LOG(LOG_INFO, "          * Packet_id = 0x%04x (2 bytes): %s", this->packet_id, get_PacketId_name(this->packet_id));
    }
};

enum class CapabilityType : uint16_t {
    general     = 1,
    printer     = 2,
    port        = 3,
    drive       = 4,
    smartcard   = 5
};

// [MS-RDPEFS] - 2.2.1.2 Capability Header (CAPABILITY_HEADER)
// ===========================================================

// This is a header that is embedded in the Server Core Capability Request
//  and Client Core Capability Response. The purpose of this header is to
//  describe capabilities for different device types.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         CapabilityType        |        CapabilityLength       |
// +-------------------------------+-------------------------------+
// |                            Version                            |
// +---------------------------------------------------------------+

// CapabilityType (2 bytes): A 16-bit unsigned integer that identifies the
//  type of capability being described. It MUST be set to one of the
//  following values.

//  +--------------------+---------------------------------------------------+
//  | Value              | Meaning                                           |
//  +--------------------+---------------------------------------------------+
//  | CAP_GENERAL_TYPE   | General capability set (GENERAL_CAPS_SET)         |
//  | 0x0001             |                                                   |
//  +--------------------+---------------------------------------------------+
//  | CAP_PRINTER_TYPE   | Print capability set (PRINTER_CAPS_SET)           |
//  | 0x0002             |                                                   |
//  +--------------------+---------------------------------------------------+
//  | CAP_PORT_TYPE      | Port capability set (PORT_CAPS_SET)               |
//  | 0x0003             |                                                   |
//  +--------------------+---------------------------------------------------+
//  | CAP_DRIVE_TYPE     | Drive capability set (DRIVE_CAPS_SET)             |
//  | 0x0004             |                                                   |
//  +--------------------+---------------------------------------------------+
//  | CAP_SMARTCARD_TYPE | Smart card capability set (SMARTCARD_CAPS_SET)<2> |
//  | 0x0005             |                                                   |
//  +--------------------+---------------------------------------------------+


enum {
      CAP_GENERAL_TYPE   = 0x0001
    , CAP_PRINTER_TYPE   = 0x0002
    , CAP_PORT_TYPE      = 0x0003
    , CAP_DRIVE_TYPE     = 0x0004
    , CAP_SMARTCARD_TYPE = 0x0005
};

static inline
const char * get_CapabilityType_name(uint16_t capabilityType) {
    switch (capabilityType) {
        case CAP_GENERAL_TYPE:   return "CAP_GENERAL_TYPE";
        case CAP_PRINTER_TYPE:   return "CAP_PRINTER_TYPE";
        case CAP_PORT_TYPE:      return "CAP_PORT_TYPE";
        case CAP_DRIVE_TYPE:     return "CAP_DRIVE_TYPE";
        case CAP_SMARTCARD_TYPE: return "CAP_SMARTCARD_TYPE";
    }

    return "<unknown>";
}

// CapabilityLength (2 bytes): A 16-bit unsigned integer that specifies that
//  size, in bytes, of the capability message, this header included.

// Version (4 bytes): A 32-bit unsigned integer that specifies the
//  capability-specific version for the specific value of CapabilityType, as
//  described in the table that follows.

//  +-----------------+-----------------------+------------------------------+
//  | CapabilityType  | Version Value(s)      | Meaning                      |
//  | Value           |                       |                              |
//  +-----------------+-----------------------+------------------------------+
//  | CAP_GENERAL_    | GENERAL_CAPABILITY_   | See section 2.2.2.7.1.       |
//  | TYPE            | VERSION_01            |                              |
//  |                 | 0x00000001            |                              |
//  |                 | GENERAL_CAPABILITY_   |                              |
//  |                 | VERSION_02            |                              |
//  |                 | 0x00000002            |                              |
//  +-----------------+-----------------------+------------------------------+
//  | CAP_PRINTER_    | PRINT_CAPABILITY_     | Version 1 of printing        |
//  | TYPE            | VERSION_01            | capabilities.                |
//  |                 | 0x00000001            |                              |
//  +-----------------+-----------------------+------------------------------+
//  | CAP_PORT_       | PORT_CAPABILITY_      | Version 1 of port            |
//  | TYPE            | VERSION_01            | capabilities.                |
//  |                 | 0x00000001            |                              |
//  +-----------------+-----------------------+------------------------------+
//  | CAP_DRIVE_      | DRIVE_CAPABILITY_     | If the client supports       |
//  | TYPE            | VERSION_01            | DRIVE_CAPABILITY_VERSION_02, |
//  |                 | 0x00000001            | then the drive name of the   |
//  |                 | DRIVE_CAPABILITY_     | redirected device can be     |
//  |                 | VERSION_02            | specified by the DeviceData  |
//  |                 | 0x00000002            | field of a DEVICE ANNOUNCE   |
//  |                 |                       | header, as specified in the  |
//  |                 |                       | DeviceAnnounce field         |
//  |                 |                       | description of the Client    |
//  |                 |                       | Device List Announce message |
//  |                 |                       | (section 2.2.3.1).           |
//  +-----------------+-----------------------+------------------------------+
//  | CAP_SMARTCARD_  | SMARTCARD_CAPABILITY_ | Version 1 of smart card      |
//  | TYPE            | VERSION_01            | capabilities.                |
//  |                 | 0x00000001            |                              |
//  +-----------------+-----------------------+------------------------------+

enum {
      GENERAL_CAPABILITY_VERSION_01 = 0x00000001
    , GENERAL_CAPABILITY_VERSION_02 = 0x00000002
};

static inline
const char * get_CapabilityVersion_name(uint16_t version) {
    switch (version) {
        case GENERAL_CAPABILITY_VERSION_01: return "GENERAL_CAPABILITY_VERSION_01";
        case GENERAL_CAPABILITY_VERSION_02: return "GENERAL_CAPABILITY_VERSION_02";
    }

    return "<unknown>";
}

enum {
      PRINT_CAPABILITY_VERSION_01 = 0x00000001
};

enum {
      PORT_CAPABILITY_VERSION_01 = 0x00000001
};

enum {
      DRIVE_CAPABILITY_VERSION_01 = 0x00000001
    , DRIVE_CAPABILITY_VERSION_02 = 0x00000002
};

enum {
      SMARTCARD_CAPABILITY_VERSION_0 = 0x00000001
};


struct CapabilityHeader {

    uint16_t CapabilityType{0};
    uint16_t CapabilityLength{0};
    uint32_t Version{0};

    explicit CapabilityHeader() = default;

    explicit CapabilityHeader( uint16_t CapabilityType
                             , uint32_t Version)
    : CapabilityType(CapabilityType)
    , CapabilityLength(8)
    , Version(Version)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->CapabilityType);
        stream.out_uint16_le(this->CapabilityLength);
        stream.out_uint32_le(this->Version);
    }

    void receive(InStream & stream) {
        this->CapabilityType =  stream.in_uint16_le();
        this->CapabilityLength =  stream.in_uint16_le();
        this->Version =  stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Capability Header:");
        LOG(LOG_INFO, "          * CapabilityType   = 0x%04x (2 bytes): %s", this->CapabilityType, get_CapabilityType_name(this->CapabilityType));
        LOG(LOG_INFO, "          * CapabilityLength = %d (2 bytes)", this->CapabilityLength);
        LOG(LOG_INFO, "          * Version          = 0x%08x (4 bytes): %s", this->Version, get_CapabilityVersion_name(this->Version));
    }
};


// [MS-RDPEFS] - 2.2.3.2 Client Drive Device List Remove
//  (DR_DEVICELIST_REMOVE)
// =====================================================

// The client removes a list of already-announced devices from the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                          DeviceCount                          |
// +---------------------------------------------------------------+
// |                      DeviceIds (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_DEVICELIST_REMOVE.

// DeviceCount (4 bytes): A 32-bit unsigned integer that specifies the number
//  of entries in the DeviceIds field.

// DeviceIds (variable): A variable-length array of 32-bit unsigned integers
//  that specifies device IDs. The IDs specified in this array match the IDs
//  specified in the Client Device List Announce (section 2.2.3.1) packet.

//  Note The client can send the DR_DEVICELIST_REMOVE message for devices
//   that are removed after a session is connected. The server can accept the
//   DR_DEVICE_REMOVE message for any removed device, including file system
//   and port devices. The server can also accept reused DeviceIds of devices
//   that have been removed, providing the implementation uses the
//   DR_DEVICE_REMOVE message to do so.

struct ClientDriveDeviceListRemove {

    uint32_t DeviceCount;
    uint32_t DeviceIds[1592] = { 0 };

    // TODO array_view
    explicit ClientDriveDeviceListRemove( uint32_t DeviceCount
                                        , uint32_t const * DeviceIds)
    : DeviceCount(DeviceCount)
    {
        //assert(this->DeviceCount > 1592);
        for (uint32_t i = 0; i < DeviceCount; i++) {
            this->DeviceIds[i] = DeviceIds[i];
        }
    }

    explicit ClientDriveDeviceListRemove() = default;

    void emit(OutStream & stream) const {
        stream.out_uint32_le(DeviceCount);
        for (uint32_t i = 0; i < DeviceCount; i++) {
            stream.out_uint32_le(DeviceIds[i]);
        }
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveDeviceListRemove: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->DeviceCount = stream.in_uint32_le();
        {
            const unsigned expected = this->DeviceCount * 4;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveDeviceListRemove: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        for (uint32_t i = 0; i < this->DeviceCount; i++) {
            this->DeviceIds[i] = stream.in_uint32_le();
        }
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Device List Remove:");
        LOG(LOG_INFO, "          * DeviceCount = %u (4 bytes)", this->DeviceCount);
        for (uint32_t i = 0; i < this->DeviceCount; i++) {
            LOG(LOG_INFO, "          * DeviceIds   = 0x%08x (4 bytes)", this->DeviceIds[i]);
        }
    }

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

enum RDPDR_DTYP : uint32_t {
      RDPDR_DTYP_UNSPECIFIED = 0x00000000
    , RDPDR_DTYP_SERIAL      = 0x00000001
    , RDPDR_DTYP_PARALLEL    = 0x00000002
    , RDPDR_DTYP_PRINT       = 0x00000004
    , RDPDR_DTYP_FILESYSTEM  = 0x00000008
    , RDPDR_DTYP_SMARTCARD   = 0x00000020
};

static const char * get_DeviceType_name(RDPDR_DTYP DeviceType) noexcept
{
    switch (DeviceType) {
        case RDPDR_DTYP_SERIAL:     return "RDPDR_DTYP_SERIAL";
        case RDPDR_DTYP_PARALLEL:   return "RDPDR_DTYP_PARALLEL";
        case RDPDR_DTYP_PRINT:      return "RDPDR_DTYP_PRINT";
        case RDPDR_DTYP_FILESYSTEM: return "RDPDR_DTYP_FILESYSTEM";
        case RDPDR_DTYP_SMARTCARD:  return "RDPDR_DTYP_SMARTCARD";
        case RDPDR_DTYP_UNSPECIFIED: break;
    }

    return "<unknown>";
}

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
    RDPDR_DTYP DeviceType_ = RDPDR_DTYP_SERIAL;
    uint32_t DeviceId_   = 0;

    uint8_t  PreferredDosName_[8 /* PreferredDosName(8) */ + 1] = { 0 };

    struct { uint8_t const * p; std::size_t sz; } device_data = {nullptr, 0u};

public:
    explicit DeviceAnnounceHeader() = default;

    explicit DeviceAnnounceHeader(RDPDR_DTYP DeviceType, uint32_t DeviceId,
                                  const char * preferred_dos_name,
                                  uint8_t const * device_data_p, size_t device_data_size)
    : DeviceType_(DeviceType)
    , DeviceId_(DeviceId)
    , device_data{device_data_p, device_data_size} {
        memcpy(this->PreferredDosName_, preferred_dos_name,
            strnlen(preferred_dos_name, sizeof(this->PreferredDosName_)-1));
    }

    REDEMPTION_NON_COPYABLE(DeviceAnnounceHeader);

    void emit(OutStream & stream) const {
        stream.out_uint32_le(underlying_cast(this->DeviceType_));
        stream.out_uint32_le(this->DeviceId_);

        stream.out_copy_bytes(this->PreferredDosName_, 8 /* PreferredDosName(8) */);

        stream.out_uint32_le(this->device_data.sz); // DeviceDataLength(4)

        if (this->device_data.p) {
            stream.out_copy_bytes(this->device_data.p, this->device_data.sz);
        }
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 20;  // DeviceType(4) + DeviceId(4) + PreferredDosName(8) + DeviceDataLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceAnnounceHeader (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceType_ = RDPDR_DTYP(stream.in_uint32_le());
        this->DeviceId_   = stream.in_uint32_le();

        stream.in_copy_bytes(this->PreferredDosName_, 8 /* PreferredDosName(8) */);
        //this->PreferredDosName_[8 /* PreferredDosName(8) */ ] = '\0';

//         const uint32_t DeviceDataLength = stream.in_uint32_le();

//         {
//             const unsigned expected = DeviceDataLength;  // DeviceData(variable)
//
//             if (!stream.in_check_rem(expected)) {
//                 LOG(LOG_ERR,
//                     "Truncated DeviceAnnounceHeader (1): expected=%u remains=%zu",
//                     expected, stream.in_remain());
//                 throw Error(ERR_RDPDR_PDU_TRUNCATED);
//             }
//         }

        this->device_data.sz = stream.in_uint32_le();
        //this->device_data = {stream.get_current(), DeviceDataLength};
//         stream.in_skip_bytes(stream.in_remain());           // DeviceDataLength
    }

    RDPDR_DTYP DeviceType() const { return this->DeviceType_; }

    uint32_t DeviceId() const { return this->DeviceId_; }

    const char * PreferredDosName() const {
        return ::char_ptr_cast(this->PreferredDosName_);
    }

    const char * DeviceData() const {
        return ::char_ptr_cast(this->device_data.p);
    }

    size_t DeviceDataLength() const {
        return this->device_data.sz /* DeviceData(variable) */
            ;
    }

    size_t size() const {
        return 20 + // DeviceType(4) + DeviceId(4) + PreferredDosName(8) +
                    // DeviceDataLength(4)
            this->device_data.sz /* DeviceData(variable) */
            ;
    }



    static const char * get_DeviceType_friendly_name(RDPDR_DTYP DeviceType) {
        switch (DeviceType) {
            case RDPDR_DTYP_SERIAL:     return "Serial port";
            case RDPDR_DTYP_PARALLEL:   return "Parallel port";
            case RDPDR_DTYP_PRINT:      return "Printer";
            case RDPDR_DTYP_FILESYSTEM: return "File system";
            case RDPDR_DTYP_SMARTCARD:  return "Smart card";
            case RDPDR_DTYP_UNSPECIFIED: break;
        }

        return "<unknown>";
    }

    void log(int level) const {
        LOG(level, "DeviceAnnounceHeader: DeviceType=%s(%u) DeviceId=%u PreferredDosName=\"%s\"",
            get_DeviceType_name(this->DeviceType_),
            this->DeviceType_, this->DeviceId_, this->PreferredDosName_);
        if (level == LOG_INFO) {
            hexdump(this->device_data.p, this->device_data.sz);
        }
    }

    void log() const {
        LOG(LOG_INFO, "     Device Announce:");
        LOG(LOG_INFO, "          * DeviceType       = 0x%08x (4 bytes): %s", this->DeviceType_, get_DeviceType_name(this->DeviceType_));
        LOG(LOG_INFO, "          * DeviceId         = 0x%08x (4 bytes)", this->DeviceId_);
        LOG(LOG_INFO, "          * PreferredDosName = \"%s\" (8 bytes)", this->PreferredDosName());
        LOG(LOG_INFO, "          * DeviceDataLength = %d (4 bytes)", int(this->device_data.sz));
//         LOG(LOG_INFO, "          * DeviceData       = \"%.*s\" (%d byte(s))", int(this->device_data.sz), this->device_data.p, int(2*this->device_data.sz));
    }
};  // DeviceAnnounceHeader


// 2.2.2.1 Client Device List Announce Request (DR_PRN_DEVICE_ANNOUNCE)
//
// This message is specified in [MS-RDPEFS] section 2.2.2.9 (Client Device List Announce Request). For each redirected printer, a DEVICE_ANNOUNCE header (as specified in [MS-RDPEFS] section 2.2.1.3) is generated by the client printer redirection extension followed by variable printer-specific data.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          DeviceType                           |
// +---------------------------------------------------------------+
// |                           DeviceId                            |
// +---------------------------------------------------------------+
// |                       PreferredDosName                        |
// +---------------------------------------------------------------+
// |                             ...                               |
// +---------------------------------------------------------------+
// |                       DeviceDataLength                        |
// +---------------------------------------------------------------+
// |                            Flags                              |
// +---------------------------------------------------------------+
// |                          CodePage                             |
// +---------------------------------------------------------------+
// |                         PnPNameLen                            |
// +---------------------------------------------------------------+
// |                        DriverNameLen                          |
// +---------------------------------------------------------------+
// |                         PrintNameLen                          |
// +---------------------------------------------------------------+
// |                       CachedFieldsLen                         |
// +---------------------------------------------------------------+
// |                      PnPName (variable)                       |
// +---------------------------------------------------------------+
// |                             ...                               |
// +---------------------------------------------------------------+
// |                     DriverName (variable)                     |
// +---------------------------------------------------------------+
// |                             ...                               |
// +---------------------------------------------------------------+
// |                     PrinterName (variable)                    |
// +---------------------------------------------------------------+
// |                             ...                               |
// +---------------------------------------------------------------+
// |               CachedPrinterConfigData (variable)              |
// +---------------------------------------------------------------+
// |                             ...                               |
// +---------------------------------------------------------------+

// DeviceType (4 bytes): This field is defined in [MS-RDPEFS] section 2.2.1.3. This field MUST be set to RDPDR_DTYP_PRINT.
//
// DeviceId (4 bytes): This field is defined in [MS-RDPEFS] section 2.2.1.3. The DeviceId field MUST be set to the unique device ID to identify this printer device. This field is later used to refer to the same printer device by both the client and the server.
//
// PreferredDosName (8 bytes): This field is defined in [MS-RDPEFS] section 2.2.1.3. The PreferredDosName field MUST be set to the port name on which the printer is installed.
//
// DeviceDataLength (4 bytes): This field is defined in [MS-RDPEFS] section 2.2.1.3. The DeviceDataLength field MUST be set to the length of data fields following the DeviceDataLength field.
//
// Flags (4 bytes): A 32-bit unsigned integer that indicates the properties of the client printer queue. This bit field MUST be a valid combination of any of the following values.

//  +--------------------------------------------+--------------------------------------------+
//  | Value                                      | Meaning                                    |
//  +--------------------------------------------+--------------------------------------------+
//  | RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII          | The DriverName field MUST be in ASCII      |
//  | 0x00000001                                 | characters. If not set, it MUST be in      |
//  |                                            | Unicode.                                   |
//  +--------------------------------------------+--------------------------------------------+
//  | RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER | The printer is set as default. There MUST  |
//  | 0x00000002                                 | be only one printer with this flag set.    |
//  +--------------------------------------------+--------------------------------------------+
//  | RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER | This printer is from the network.          |
//  | 0x00000004                                 |                                            |
//  +--------------------------------------------+--------------------------------------------+
//  | RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER      | This flag is set when the printer to be    |
//  | 0x00000008                                 | redirected is not a local or network       |
//  |                                            | printer but is a terminal server client    |
//  |                                            |printer. This can happen in nested TS       |
//  |                                            |sessions; that is, this can happen when a   |
//  |                                            |TS connection is made from within a TS      |
//  |                                            |session.                                    |
//  +--------------------------------------------+--------------------------------------------+
//  | RDPDR_PRINTER_ANNOUNCE_FLAG_XPSFORMAT      | This client/printer supports XML Paper     |
//  | 0x00000010                                 | Specification (XPS) format.                |
//  +--------------------------------------------+--------------------------------------------+

// TODO strong enum RDPDR_PRINTER_ANNOUNCE_FLAG
enum : uint32_t {
    RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII          = 0x00000001
  , RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER = 0x00000002
  , RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER = 0x00000004
  , RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER      = 0x00000008
  , RDPDR_PRINTER_ANNOUNCE_FLAG_XPSFORMAT      = 0x00000010
};


// CodePage (4 bytes): A 32-bit unsigned integer. Reserved for future use. This field MUST be set to 0.
//
// PnPNameLen (4 bytes):  A 32-bit unsigned integer that specifies the number of bytes in the PnPName field, including its null terminator.
//
// DriverNameLen (4 bytes): A 32-bit unsigned integer that specifies the number of bytes in the DriverName field, including its null terminator.
//
// PrintNameLen (4 bytes): A 32-bit unsigned integer that specifies the number of bytes in the PrintName field, including its null terminator.
//
// CachedFieldsLen (4 bytes): A 32-bit unsigned integer that specifies the number of bytes in the CachedPrinterConfigData field.
//
// PnPName (variable): A null-terminated Unicode string. This field can be set to any valid Unicode string and MUST be ignored on receipt.
//
// DriverName (variable):  An array of characters. The type of characters is determined by the RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII flag. If the flag is set, the DriverName field MUST be a null-terminated ASCII string; otherwise, it MUST be a null-terminated Unicode string. The DriverName field specifies the driver name used by the client for this printer. This name is used by the terminal server to determine the appropriate matching driver for the redirected printer, which is to be used on the server.<1>
//
// PrinterName (variable): The PrinterName field is a null-terminated Unicode string. The client MUST specify the user-assigned printer name of the local printer in this field. This name is used by the server to generate the server-side redirected printer queue.
//
// CachedPrinterConfigData (variable): A variable-length array of bytes. This field is a binary large object (BLOB) of data that describes the cached printer configuration (see section 3.1.1.1).

struct DeviceAnnounceHeaderPrinterSpecificData {
//     RDPDR_DTYP DeviceType = RDPDR_DTYP_UNSPECIFIED;
//     uint32_t DeviceId   = 0;
//
//     uint8_t  PreferredDosName[8 /* PreferredDosName(8) */ + 1] = { 0 };
//
//     size_t DeviceDataLength = 0;

    uint32_t Flags;
    uint32_t CodePage;

    size_t PnPNameLen;
    size_t DriverNameLen;
    size_t PrintNameLen;
    size_t CachedFieldsLen;

    char * PnPName;
    char * DriverName;
    char * PrinterName;

    uint8_t * CachedPrinterConfigData;

    explicit DeviceAnnounceHeaderPrinterSpecificData() = default;

    explicit DeviceAnnounceHeaderPrinterSpecificData(/*RDPDR_DTYP DeviceType,
                                                     uint32_t DeviceId,
                                                     uint8_t * PreferredDosName,
                                                     size_t DeviceDataLength,*/
                                                     uint32_t Flags,
                                                     uint32_t CodePage,
                                                     size_t PnPNameLen,
                                                     size_t DriverNameLen,
                                                     size_t PrintNameLen,
                                                     size_t CachedFieldsLen,
                                                     char * PnPName,
                                                     char * DriverName,
                                                     char * PrinterName )
//         : DeviceType(DeviceType)
//         , DeviceId(DeviceId)
//         , DeviceDataLength(DeviceDataLength)
        : Flags(Flags)
        , CodePage(CodePage)
        , PnPNameLen(PnPNameLen)
        , DriverNameLen(DriverNameLen)
        , PrintNameLen(PrintNameLen)
        , CachedFieldsLen(CachedFieldsLen)
        , PnPName(PnPName)
        , DriverName(DriverName)
        , PrinterName(PrinterName)
    {
//         memcpy(
//             this->PreferredDosName, PreferredDosName,
//             sizeof(PreferredDosName));
    }

    void emit(OutStream & stream) const {
//         stream.out_uint32_le(underlying_cast(this->DeviceType));
//         stream.out_uint32_le(this->DeviceId);
//         stream.out_copy_bytes(this->PreferredDosName, 8);
//         stream.out_uint32_le(this->DeviceDataLength);
        stream.out_uint32_le(this->Flags);
        stream.out_uint32_le(this->CodePage);
        stream.out_uint32_le(this->PnPNameLen);
        stream.out_uint32_le(this->DriverNameLen);
        stream.out_uint32_le(this->PrintNameLen);
        stream.out_uint32_le(this->CachedFieldsLen);
        stream.out_copy_bytes(byte_ptr_cast(this->PnPName), this->PnPNameLen);
        stream.out_copy_bytes(byte_ptr_cast(this->DriverName), this->DriverNameLen);
        stream.out_copy_bytes(byte_ptr_cast(this->PrinterName), this->PrintNameLen);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 44;  // DeviceType(4) + DeviceId(4) + PreferredDosName(8) + DeviceDataLength(4) +
                                           //     Flags(4) + CodePage(4) + PnPNameLen(4) +
                                           //     DriverNameLen(4) + PrintNameLen(4) +
                                           //     CachedFieldsLen(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceAnnounceHeaderPrinterSpecificData: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
//         this->DeviceType = RDPDR_DTYP(stream.in_uint32_le());
//         this->DeviceId = stream.in_uint32_le();
//         stream.in_copy_bytes(this->PreferredDosName, 8);
//         this->DeviceDataLength = stream.in_uint32_le();
        this->Flags = stream.in_uint32_le();
        this->CodePage = stream.in_uint32_le();
        this->PnPNameLen = stream.in_uint32_le();
        this->DriverNameLen = stream.in_uint32_le();
        this->PrintNameLen = stream.in_uint32_le();
        this->CachedFieldsLen = stream.in_uint32_le();

        if (!stream.in_check_rem(this->PnPNameLen )) {
            LOG(LOG_ERR,
                "Truncated DeviceAnnounceHeaderPrinterSpecificData: expected=%zu remains=%zu",
                this->PnPNameLen , stream.in_remain());
            throw Error(ERR_RDPDR_PDU_TRUNCATED);
        }
        stream.in_copy_bytes(this->PnPName, this->PnPNameLen);

        if (!stream.in_check_rem(this->DriverNameLen)) {
            LOG(LOG_ERR,
                "Truncated DeviceAnnounceHeaderPrinterSpecificData: expected=%zu remains=%zu",
                this->DriverNameLen, stream.in_remain());
            throw Error(ERR_RDPDR_PDU_TRUNCATED);
        }
        stream.in_copy_bytes(this->DriverName, this->DriverNameLen);

        if (!stream.in_check_rem(this->PrintNameLen)) {
            LOG(LOG_ERR,
                "Truncated DeviceAnnounceHeaderPrinterSpecificData: expected=%zu remains=%zu",
                this->PrintNameLen, stream.in_remain());
            throw Error(ERR_RDPDR_PDU_TRUNCATED);
        }
        stream.in_copy_bytes(this->PrinterName, this->PrintNameLen);
    }

    void log() const {
        LOG(LOG_INFO, "     Device Announce Printer Specific Data:");
//         LOG(LOG_INFO, "          * DeviceType       = 0x%08x (4 bytes): %s", this->DeviceType, get_DeviceType_name(this->DeviceType));
//         LOG(LOG_INFO, "          * DeviceId         = 0x%08x (4 bytes)", this->DeviceId);
//         LOG(LOG_INFO, "          * DeviceName       = \"%.*s\" (8 bytes)", 8, this->PreferredDosName);
//         LOG(LOG_INFO, "          * DeviceDataLength = %d (4 bytes)", int(this->DeviceDataLength));
#define ASTR(f) (this->Flags & f) ? " " #f : ""
        LOG(LOG_INFO, "          * Flags           = 0x%08x (4 bytes):%s%s%s%s%s", this->Flags,
            ASTR(RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII),
            ASTR(RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER),
            ASTR(RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER),
            ASTR(RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER),
            ASTR(RDPDR_PRINTER_ANNOUNCE_FLAG_XPSFORMAT)
        );
#undef ASTR
        LOG(LOG_INFO, "          * CodePage        = 0x%08x (4 bytes)", this->CodePage);
        LOG(LOG_INFO, "          * PnPNameLen      = %zu (4 bytes)", this->PnPNameLen);
        LOG(LOG_INFO, "          * DriverNameLen   = %zu (4 bytes)", this->DriverNameLen);
        LOG(LOG_INFO, "          * PrintNameLen    = %zu (4 bytes)", this->PrintNameLen);
        LOG(LOG_INFO, "          * CachedFieldsLen = %zu (4 bytes)", this->CachedFieldsLen);
        LOG(LOG_INFO, "          * PnPName         = \"%s\" (%zu bytes)", this->PnPName, this->PnPNameLen);
        LOG(LOG_INFO, "          * DriverName      = \"%s\" (%zu bytes)", this->DriverName, this->DriverNameLen);
        LOG(LOG_INFO, "          * PrinterName     = \"%s\" (%zu bytes)", this->PrinterName, this->PrintNameLen);
//         LOG(LOG_INFO, "          * CachedPrinterConfigData  (%zu bytes)", this->CachedFieldsLen);
    }

};

// [MS-RDPEFS] - 2.2.1.4 Device I/O Request (DR_DEVICE_IOREQUEST)
// ==============================================================

// This header is embedded in all server requests on a specific device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                            DeviceId                           |
// +---------------------------------------------------------------+
// |                             FileId                            |
// +---------------------------------------------------------------+
// |                          CompletionId                         |
// +---------------------------------------------------------------+
// |                         MajorFunction                         |
// +---------------------------------------------------------------+
// |                         MinorFunction                         |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_DEVICE_IOREQUEST.

// DeviceId (4 bytes): A 32-bit unsigned integer that is a unique ID. The
//  value MUST match the DeviceId value in the Client Device List Announce
//  Request (section 2.2.2.9).

// FileId (4 bytes): A 32-bit unsigned integer that specifies a unique ID
//  retrieved from the Device Create Response (section 2.2.1.5.1).

// CompletionId (4 bytes): A 32-bit unsigned integer that specifies a unique
//  ID for each request. The ID is considered valid until a Device I/O
//  Response (section 2.2.1.5) is received. Subsequently, the ID MUST be
//  reused.

// MajorFunction (4 bytes): A 32-bit unsigned integer that identifies the
//  request function. This field MUST have one of the following values.

//  +---------------------------------+----------------------------------+
//  | Value                           | Meaning                          |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_CREATE                   | Create request                   |
//  | 0x00000000                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_CLOSE                    | Close request                    |
//  | 0x00000002                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_READ                     | Read request                     |
//  | 0x00000003                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_WRITE                    | Write request                    |
//  | 0x00000004                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_DEVICE_CONTROL           | Device control request           |
//  | 0x0000000E                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_QUERY_VOLUME_INFORMATION | Query volume information request |
//  | 0x0000000A                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_SET_VOLUME_INFORMATION   | Set volume information request   |
//  | 0x0000000B                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_QUERY_INFORMATION        | Query information request        |
//  | 0x00000005                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_SET_INFORMATION          | Set information request          |
//  | 0x00000006                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_DIRECTORY_CONTROL        | Directory control request        |
//  | 0x0000000C                      |                                  |
//  +---------------------------------+----------------------------------+
//  | IRP_MJ_LOCK_CONTROL             | File lock control request        |
//  | 0x00000011                      |                                  |
//  +---------------------------------+----------------------------------+

enum {
      IRP_MJ_CREATE                   = 0x00000000
    , IRP_MJ_CLOSE                    = 0x00000002
    , IRP_MJ_READ                     = 0x00000003
    , IRP_MJ_WRITE                    = 0x00000004
    , IRP_MJ_DEVICE_CONTROL           = 0x0000000E
    , IRP_MJ_QUERY_VOLUME_INFORMATION = 0x0000000A
    , IRP_MJ_SET_VOLUME_INFORMATION   = 0x0000000B
    , IRP_MJ_QUERY_INFORMATION        = 0x00000005
    , IRP_MJ_SET_INFORMATION          = 0x00000006
    , IRP_MJ_DIRECTORY_CONTROL        = 0x0000000C
    , IRP_MJ_LOCK_CONTROL             = 0x00000011
};

    static const char * get_MajorFunction_name(uint32_t MajorFunction) {
        switch (MajorFunction) {
            case IRP_MJ_CREATE:                   return "IRP_MJ_CREATE";
            case IRP_MJ_CLOSE:                    return "IRP_MJ_CLOSE";
            case IRP_MJ_READ:                     return "IRP_MJ_READ";
            case IRP_MJ_WRITE:                    return "IRP_MJ_WRITE";
            case IRP_MJ_DEVICE_CONTROL:           return "IRP_MJ_DEVICE_CONTROL";
            case IRP_MJ_QUERY_VOLUME_INFORMATION: return "IRP_MJ_QUERY_VOLUME_INFORMATION";
            case IRP_MJ_SET_VOLUME_INFORMATION:   return "IRP_MJ_SET_VOLUME_INFORMATION";
            case IRP_MJ_QUERY_INFORMATION:        return "IRP_MJ_QUERY_INFORMATION";
            case IRP_MJ_SET_INFORMATION:          return "IRP_MJ_SET_INFORMATION";
            case IRP_MJ_DIRECTORY_CONTROL:        return "IRP_MJ_DIRECTORY_CONTROL";
            case IRP_MJ_LOCK_CONTROL:             return "IRP_MJ_LOCK_CONTROL";
        }

        return "<unknown>";
    }

// MinorFunction (4 bytes): A 32-bit unsigned integer. This field is valid
//  only when the MajorFunction field is set to IRP_MJ_DIRECTORY_CONTROL. If
//  the MajorFunction field is set to another value, the MinorFunction field
//  value SHOULD be 0x00000000;<5> otherwise, the MinorFunction field MUST
//  have one of the following values.

//  +--------------------------------+---------------------------------+
//  | Value                          | Meaning                         |
//  +--------------------------------+---------------------------------+
//  | IRP_MN_QUERY_DIRECTORY         | Query directory request         |
//  | 0x00000001                     |                                 |
//  +--------------------------------+---------------------------------+
//  | IRP_MN_NOTIFY_CHANGE_DIRECTORY | Notify change directory request |
//  | 0x00000002                     |                                 |
//  +--------------------------------+---------------------------------+

enum {
      IRP_MN_QUERY_DIRECTORY         = 0x00000001
    , IRP_MN_NOTIFY_CHANGE_DIRECTORY = 0x00000002
};

static const char * get_MinorFunction_name(uint32_t MinorFunction) {
    switch (MinorFunction)
    {
        case IRP_MN_QUERY_DIRECTORY:         return "IRP_MN_QUERY_DIRECTORY";
        case IRP_MN_NOTIFY_CHANGE_DIRECTORY: return "IRP_MN_NOTIFY_CHANGE_DIRECTORY";
    }

    return "<unknown>";
}

class DeviceIORequest {
    uint32_t DeviceId_      = 0;
    uint32_t FileId_        = 0;
    uint32_t CompletionId_  = 0;
    uint32_t MajorFunction_ = 0;
    uint32_t MinorFunction_ = 0;


public:

    explicit DeviceIORequest() = default;

    explicit DeviceIORequest( uint32_t DeviceId_
                            , uint32_t FileId_
                            , uint32_t CompletionId_
                            , uint32_t MajorFunction_
                            , uint32_t MinorFunction_)
      : DeviceId_(DeviceId_)
      , FileId_(FileId_)
      , CompletionId_(CompletionId_)
      , MajorFunction_(MajorFunction_)
      , MinorFunction_(MinorFunction_) {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->DeviceId_);
        stream.out_uint32_le(this->FileId_);
        stream.out_uint32_le(this->CompletionId_);
        stream.out_uint32_le(this->MajorFunction_);
        stream.out_uint32_le(this->MinorFunction_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 20;  // DeviceId(4) + FileId(4) + CompletionId(4) +
                                           //     MajorFunction(4) + MinorFunction(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceIORequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceId_      = stream.in_uint32_le();
        this->FileId_        = stream.in_uint32_le();
        this->CompletionId_  = stream.in_uint32_le();
        this->MajorFunction_ = stream.in_uint32_le();
        this->MinorFunction_ = stream.in_uint32_le();
    }

    uint32_t DeviceId() const { return this->DeviceId_; }

    uint32_t FileId() const { return this->FileId_; }

    uint32_t CompletionId() const { return this->CompletionId_; }

    uint32_t MajorFunction() const { return this->MajorFunction_; }

    uint32_t MinorFunction() const { return this->MinorFunction_; }

    void log(int level) const {
        LOG(level,
            "DeviceIORequest: "
                "DeviceId=%u FileId=%u CompletionId=%u MajorFunction=%s(0x%X) MinorFunction=%s(0x%X)",
            this->DeviceId_, this->FileId_, this->CompletionId_,
            get_MajorFunction_name(this->MajorFunction_), this->MajorFunction_,
            get_MinorFunction_name(this->MinorFunction_), this->MinorFunction_);
    }

    void log() const {
        LOG(LOG_INFO, "     Device I\\O Request:");
        LOG(LOG_INFO, "          * DeviceId      = 0x%08x (4 bytes)", this->DeviceId_);
        LOG(LOG_INFO, "          * FileId        = 0x%08x (4 bytes)", this->FileId_);
        LOG(LOG_INFO, "          * CompletionId  = 0x%08x (4 bytes)", this->CompletionId_);
        LOG(LOG_INFO, "          * MajorFunction = 0x%08x (4 bytes): %s", this->MajorFunction_, get_MajorFunction_name(this->MajorFunction_));
        LOG(LOG_INFO, "          * MinorFunction = 0x%08x (4 bytes): %s", this->MinorFunction_, get_MinorFunction_name(this->MinorFunction_));
    }

    bool map_completion_id(FileSystemCompletionIdManager & id_manager) {
        uint32_t const completion_id = id_manager.get_dest_id_ex(this->CompletionId_);

        if (completion_id != this->CompletionId_) {
            this->CompletionId_ = completion_id;

            return true;
        }

        return false;
    }
};


// [MS-RDPEFS] - 2.2.3.3.1 Server Create Drive Request (DR_DRIVE_CREATE_REQ)
// =========================================================================

// The server opens or creates a file on a redirected file system device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 DeviceCreateRequest (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceCreateRequest (variable): A DR_CREATE_REQ header. The PathLength and
//  Path fields contain the file name of the file to be created. The file
//  name does not contain a drive letter, which means that the drive is
//  specified by the DeviceId field of the request. The DeviceId is
//  associated with a drive letter when the device is announced in the
//  DR_DEVICELIST_ANNOUNCE (section 2.2.3.1) message. The drive letter is
//  contained in the PreferredDosName field.

// [MS-RDPEFS] - 2.2.1.4.1 Device Create Request (DR_CREATE_REQ)
// =============================================================

// This header initiates a create request. This message can have different
//  purposes depending on the device for which it is issued. The device type
//  is determined by the DeviceId field in the DR_DEVICE_IOREQUEST header.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         DesiredAccess                         |
// +---------------------------------------------------------------+
// |                         AllocationSize                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         FileAttributes                        |
// +---------------------------------------------------------------+
// |                          SharedAccess                         |
// +---------------------------------------------------------------+
// |                       CreateDisposition                       |
// +---------------------------------------------------------------+
// |                         CreateOptions                         |
// +---------------------------------------------------------------+
// |                           PathLength                          |
// +---------------------------------------------------------------+
// |                        Path (variable)                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST header. The
//  MajorFunction field in this header MUST be set to IRP_MJ_CREATE.

// DesiredAccess (4 bytes): A 32-bit unsigned integer that specifies the
//  level of access. This field is specified in [MS-SMB2] section 2.2.13.

// AllocationSize (8 bytes): A 64-bit unsigned integer that specifies the
//  initial allocation size for the file.

// FileAttributes (4 bytes): A 32-bit unsigned integer that specifies the
//  attributes for the file being created. This field is specified in
//  [MS-SMB2] section 2.2.13.

// SharedAccess (4 bytes): A 32-bit unsigned integer that specifies the
//  sharing mode for the file being opened. This field is specified in
//  [MS-SMB2] section 2.2.13.

// CreateDisposition (4 bytes): A 32-bit unsigned integer that specifies the
//  action for the client to take if the file already exists. This field is
//  specified in [MS-SMB2] section 2.2.13. For ports and other devices, this
//  field MUST be set to FILE_OPEN (0x00000001).

// CreateOptions (4 bytes): A 32-bit unsigned integer that specifies the
//  options for creating the file. This field is specified in [MS-SMB2]
//  section 2.2.13.

// PathLength (4 bytes): A 32-bit unsigned integer that specifies the number
//  of bytes in the Path field, including the null-terminator.

// Path (variable): A variable-length array of Unicode characters, including
//  the null-terminator, whose size is specified by the PathLength field. The
//  protocol imposes no limitations on the characters used in this field.

class DeviceCreateRequest {
    uint32_t DesiredAccess_     = 0;
    uint64_t AllocationSize_    = 0LLU;
    uint32_t FileAttributes_    = 0;
    uint32_t SharedAccess_      = 0;
    uint32_t CreateDisposition_ = 0;
    uint32_t CreateOptions_     = 0;

    size_t PathLength_UTF16 = 0;
    size_t PathLength_UTF8  = 0;
    uint8_t Path_[65536] = {0};

public:
    explicit DeviceCreateRequest() = default;

    explicit DeviceCreateRequest(uint32_t DesiredAccess,
                                 uint64_t AllocationSize,
                                 uint32_t FileAttributes,
                                 uint32_t SharedAccess,
                                 uint32_t CreateDisposition,
                                 uint32_t CreateOptions,
                                 //size_t PathLength_UTF16,
                                 size_t PathLength_UTF8,
                                 const char * Path_UTF8)
        : DesiredAccess_(DesiredAccess)
        , AllocationSize_(AllocationSize)
        , FileAttributes_(FileAttributes)
        , SharedAccess_(SharedAccess)
        , CreateDisposition_(CreateDisposition)
        , CreateOptions_(CreateOptions)
       // , PathLength_UTF16(PathLength_UTF16)
        // TODO if PathLength_UTF8 > sizeof(this->Path_) ?
        , PathLength_UTF8(std::min(sizeof(this->Path_) - 1, PathLength_UTF8))
        {
            std::memcpy(this->Path_, Path_UTF8, this->PathLength_UTF8);
            this->Path_[this->PathLength_UTF8] = 0;
        }

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->DesiredAccess_);
        stream.out_uint64_le(this->AllocationSize_);
        stream.out_uint32_le(this->FileAttributes_);
        stream.out_uint32_le(this->SharedAccess_);
        stream.out_uint32_le(this->CreateDisposition_);
        stream.out_uint32_le(this->CreateOptions_);

        uint8_t Path_unicode_data[65536];
        size_t size_of_Path_unicode_data = ::UTF8toUTF16(
        		this->Path(),
				Path_unicode_data,
				sizeof(Path_unicode_data));

        assert(size_of_Path_unicode_data <= 65534);

        // Writes null terminator.
        Path_unicode_data[size_of_Path_unicode_data    ] = 0;
        Path_unicode_data[size_of_Path_unicode_data + 1] = 0;
        size_of_Path_unicode_data += 2;

        uint8_t * temp_p = Path_unicode_data;
        for (size_t i = 0; i < size_of_Path_unicode_data; i += 2) {
            if (('/' == temp_p[0]) && (0 == temp_p[1])) {
                temp_p[0] = '\\';
            }
            temp_p += 2;
        }

        stream.out_uint32_le(size_of_Path_unicode_data);

        stream.out_copy_bytes(Path_unicode_data, size_of_Path_unicode_data);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // DesiredAccess(4) + AllocationSize(8) +
                                           //     FileAttributes(4) + SharedAccess(4) +
                                           //     CreateDisposition(4) + CreateOptions(4) +
                                           //     PathLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DesiredAccess_     = stream.in_uint32_le();
        this->AllocationSize_    = stream.in_uint64_le();
        this->FileAttributes_    = stream.in_uint32_le();
        this->SharedAccess_      = stream.in_uint32_le();
        this->CreateDisposition_ = stream.in_uint32_le();
        this->CreateOptions_     = stream.in_uint32_le();

        this->PathLength_UTF16 = stream.in_uint32_le();

        if (this->PathLength_UTF16) {
            {
                const unsigned expected = this->PathLength_UTF16;   // Path(variable)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated DeviceCreateRequest (1): expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDPDR_PDU_TRUNCATED);
                }
            }

            uint8_t const * const Path_unicode_data = stream.get_current();

            this->PathLength_UTF8 = ::UTF16toUTF8(Path_unicode_data, this->PathLength_UTF16, this->Path_,
                65536);
            if (this->PathLength_UTF8) {
                this->PathLength_UTF8--;
            }

            for (size_t i = 0; i < this->PathLength_UTF8; i++) {
                if ('\\' == this->Path_[i]) {
                    this->Path_[i] = '/';
                }
            }

            stream.in_skip_bytes(PathLength_UTF16);
        }
    }

    uint64_t AllocationSize() const { return this->AllocationSize_; }

    uint32_t FileAttributes() const { return this->FileAttributes_; }

    uint32_t SharedAccess() const { return this->SharedAccess_; }

    uint32_t DesiredAccess() const { return this->DesiredAccess_; }

    uint32_t CreateDisposition() const { return this->CreateDisposition_; }

    uint32_t CreateOptions() const { return this->CreateOptions_; }

    array_view_const_char Path() const { return {char_ptr_cast(this->Path_), this->PathLength_UTF8}; }

    size_t PathLength() const { return this->PathLength_UTF16; }

    void log(int level) const {
        LOG(level,
            "DeviceCreateRequest: DesiredAccess=0x%X AllocationSize=%" PRIu64 " "
                "FileAttributes=0x%X SharedAccess=0x%X CreateDisposition=0x%X "
                "CreateOptions=0x%X Path=\"%s\"",
            this->DesiredAccess_, this->AllocationSize_, this->FileAttributes_,
            this->SharedAccess_, this->CreateDisposition_, this->CreateOptions_,
            char_ptr_cast(this->Path_));
    }

    void log() const {
        LOG(LOG_INFO, "     Device Create Request:");
        if (this->CreateOptions_ & smb2::FILE_DIRECTORY_FILE) {
            LOG(LOG_INFO, "          * DesiredAccess     = 0x%08x (4 bytes): %s", this->DesiredAccess_, smb2::get_Directory_Access_Mask_name(this->DesiredAccess_));
        } else {
            LOG(LOG_INFO, "          * DesiredAccess     = 0x%08x (4 bytes): %s", this->DesiredAccess_, smb2::get_File_Pipe_Printer_Access_Mask_name(this->DesiredAccess_));
        }
        LOG(LOG_INFO, "          * AllocationSize    = 0x%" PRIu64 " (8 bytes)", this->AllocationSize_);
        fscc::log_file_attributes(this->FileAttributes_);
        LOG(LOG_INFO, "          * SharedAccess      = 0x%08x (4 bytes): %s", this->SharedAccess_,  smb2::get_ShareAccess_name(this->SharedAccess_));
        LOG(LOG_INFO, "          * CreateDisposition = 0x%08x (4 bytes): %s", this->CreateDisposition_, smb2::get_CreateDisposition_name(this->CreateDisposition_));
        LOG(LOG_INFO, "          * CreateOptions     = 0x%08x (4 bytes): %s", this->CreateOptions_, smb2::get_CreateOptions_name(this->CreateOptions_));
        LOG(LOG_INFO, "          * PathLength        = %d (4 bytes)", int(this->PathLength_UTF16));
        LOG(LOG_INFO, "          * Path              = \"%s\" (%d byte(s))", char_ptr_cast(this->Path_), int(this->PathLength_UTF16));
    }

};  // DeviceCreateRequest

// [MS-RDPEFS] - 2.2.1.4.2 Device Close Request (DR_CLOSE_REQ)
// ===========================================================

// This header initiates a close request. This message can have different
//  purposes depending on the device for which it is issued. The device type
//  is determined by the DeviceId field in the DR_DEVICE_IOREQUEST header.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST header. The
//  MajorFunction field in this header MUST be set to IRP_MJ_CLOSE.

// Padding (32 bytes): An array of 32 bytes. Reserved. This field can be set
//  to any value, and MUST be ignored on receipt.


class DeviceCloseRequest {

public:
    void emit(OutStream & stream) const {
        stream.out_clear_bytes(32); // Padding(32)
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // Padding(32)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCloseRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        stream.in_skip_bytes(32);   // Padding(32)
    }

    void log(int level) const {
        LOG(level, "DeviceCloseRequest:");
    }

    void log() const {
        LOG(LOG_INFO, "     Device Close Request:");
        LOG(LOG_INFO, "          * Padding - (32 bytes) NOT USED");
    }
};

// [MS-RDPEFS] - 2.2.1.4.3 Device Read Request (DR_READ_REQ)
// =========================================================

// This header initiates a read request. This message can have different
//  purposes depending on the device for which it is issued. The device type
//  is determined by the DeviceId field in the DR_DEVICE_IOREQUEST header.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                             Offset                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST header. The
//  MajorFunction field in this header MUST be set to IRP_MJ_READ.

// Length (4 bytes): A 32-bit unsigned integer. This field specifies the
//  maximum number of bytes to be read from the device.

// Offset (8 bytes): A 64-bit unsigned integer. This field specifies the file
//  offset where the read operation is performed.

// Padding (20 bytes): An array of 20 bytes. Reserved. This field can be set
//  to any value and MUST be ignored on receipt.

class DeviceReadRequest {
    uint32_t Length_ = 0;
    uint64_t Offset_ = 0LLU;

public:
    explicit DeviceReadRequest() = default;

    explicit DeviceReadRequest( uint32_t Length
                              , uint64_t Offset)
      : Length_(Length)
      , Offset_(Offset)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length_);
        stream.out_uint64_le(this->Offset_);
        stream.out_clear_bytes(20);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;  // Length(4) + Offset(8)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceReadRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->Length_ = stream.in_uint32_le();
        this->Offset_ = stream.in_uint64_le();
        stream.in_skip_bytes(20);
    }

    uint32_t Length() const { return this->Length_; }

    uint64_t Offset() const { return this->Offset_; }

    void log(int level = LOG_INFO) const {
        LOG(level, "DeviceReadRequest: Length=%u(4 bytes) Offset=%" PRIu64 "(8 bytes) Padding(20 bytes) NOT USED",
            this->Length_, this->Offset_);
    }

//     void log() const {
//         LOG(LOG_INFO, "     Device Read Request:");
//         LOG(LOG_INFO, "          * Length = %u (4 bytes)", this->Length_);
//         LOG(LOG_INFO, "          * Offset = 0x%" PRIx64 " (8 bytes)", this->Offset_);
//         LOG(LOG_INFO, "          * Padding - (20 bytes) NOT USED");
//     }
};



// [MS-RDPEFS] - 2.2.1.4.4 Device Write Request (DR_WRITE_REQ)
// ===========================================================

// This header initiates a write request. This message can have different
//  purposes depending on the device for which it is issued. The device type
//  is determined by the DeviceId field in the DR_DEVICE_IOREQUEST header.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                             Offset                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      WriteData (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST header. The
//  MajorFunction field in this header MUST be set to IRP_MJ_WRITE.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the WriteData field.

// Offset (8 bytes): A 64-bit unsigned integer. This field specifies the file
//  offset at which the data is written.

// Padding (20 bytes): An array of 20 bytes. Reserved. This field can be set
//  to any value, and MUST be ignored on receipt.

// WriteData (variable): A variable-length array of bytes, where the length
//  is specified by the Length field in this packet. This array contains data
//  to be written on the target device.

struct DeviceWriteRequest {

    uint32_t  Length = 0;
    uint64_t  Offset = 0;
    uint8_t const * WriteData = nullptr;

    enum :  unsigned {
      TOTAL_PDU_HEADER_LEN = 56
    , FISRT_PART_DATA_MAX_LEN = 1600 - TOTAL_PDU_HEADER_LEN
    };

    explicit DeviceWriteRequest() = default;

    explicit DeviceWriteRequest( uint32_t Length
                               , uint64_t Offset
                               , uint8_t const * WriteData)
    : Length(Length)
    , Offset(Offset)
    , WriteData(WriteData)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
        stream.out_uint64_le(this->Offset);
        stream.out_clear_bytes(20);
        stream.out_copy_bytes(this->WriteData, this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceWriteRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
        this->Offset = stream.in_uint64_le();
        stream.in_skip_bytes(20);
        {
            int exp = this->Length;
            if (exp > 1600-56) {
                exp = 1600-56;
            }
            const unsigned expected = exp;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceWriteRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->WriteData = stream.get_current();
    }

    void log() const {
        LOG(LOG_INFO, "     Device Write Request:");
        LOG(LOG_INFO, "          * Length    = %d (4 bytes)", int(this->Length));
        LOG(LOG_INFO, "          * Offset    = 0x%" PRIx64 " (8 bytes)", this->Offset);
        LOG(LOG_INFO, "          * Padding - (20 bytes) NOT USED");
        //auto s = char_ptr_cast(this->WriteData);
        int len = int(this->Length);
        LOG(LOG_INFO, "          * WriteData (%d byte(s))", len);
    }
};



// [MS-RDPEFS] - 2.2.1.4.5 Device Control Request (DR_CONTROL_REQ)
// ===============================================================

// This header initiates a device control request. This message can have
//  different purposes depending on the device for which it is issued. The
//  device type is determined by the DeviceId field in the
//  DR_DEVICE_IOREQUEST header.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       OutputBufferLength                      |
// +---------------------------------------------------------------+
// |                       InputBufferLength                       |
// +---------------------------------------------------------------+
// |                         IoControlCode                         |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                     InputBuffer (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST header. The
//  MajorFunction field in this header MUST be set to IRP_MJ_DEVICE_CONTROL.

// OutputBufferLength (4 bytes): A 32-bit unsigned integer that specifies the
//  maximum number of bytes expected in the OutputBuffer field of the Device
//  Control Response (section 2.2.1.5.5).

// InputBufferLength (4 bytes): A 32-bit unsigned integer that specifies the
//  number of bytes in the InputBuffer field.

// IoControlCode (4 bytes): A 32-bit unsigned integer. This field is specific
//  to the redirected device.

// Padding (20 bytes): An array of 20 bytes. Reserved. This field can be set
//  to any value, and MUST be ignored on receipt.

// InputBuffer (variable): A variable-size byte array whose size is specified
//  by the InputBufferLength field.

class DeviceControlRequest {
    uint32_t OutputBufferLength = 0;
    uint32_t IoControlCode_     = 0;

    struct { uint8_t const * p; std::size_t sz; } input_buffer = {nullptr, 0u};

public:
    explicit DeviceControlRequest() = default;

    REDEMPTION_NON_COPYABLE(DeviceControlRequest);

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->OutputBufferLength);

        stream.out_uint32_le(this->input_buffer.sz);    // InputBufferLength(4)

        stream.out_uint32_le(this->IoControlCode_);

        stream.out_clear_bytes(20); // Padding(20)

        stream.out_copy_bytes(this->input_buffer.p, this->input_buffer.sz);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // OutputBufferLength(4) + InputBufferLength(4) +
                                           //     IoControlCode(4) + Padding(20)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceControlRequest (0): "
                        "expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->OutputBufferLength = stream.in_uint32_le();

        const uint32_t InputBufferLength = stream.in_uint32_le();

        this->IoControlCode_ = stream.in_uint32_le();

        stream.in_skip_bytes(20);   // Padding(20)

        {
            const unsigned expected = InputBufferLength;  // InputBuffer(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceControlRequest (1): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->input_buffer = {stream.get_current(), InputBufferLength};
        stream.in_skip_bytes(InputBufferLength);
    }

    uint32_t IoControlCode() const { return this->IoControlCode_; }

    void log(int level) const {
        LOG(level, "DeviceControlRequest: OutputBufferLength=%u InputBufferLength=%zu "
                "IoControlCode=0x%X",
            this->OutputBufferLength, this->input_buffer.sz,
            this->IoControlCode_);
    }

    void log() const {
        LOG(LOG_INFO, "     Device Control Request:");
        LOG(LOG_INFO, "          * OutputBufferLength = %d (4 bytes)", int(this->OutputBufferLength));
        LOG(LOG_INFO, "          * InputBufferLength  = %d (4 bytes)", int(this->input_buffer.sz));
        LOG(LOG_INFO, "          * IoControlCode      = 0x%08x (4 bytes): %s", this->IoControlCode_, fscc::get_FSCTLStructures(this->IoControlCode_));
        LOG(LOG_INFO, "          * Padding - (20 bytes) NOT USED");
    }
};  // DeviceControlRequest




// 2.2.1.5.5 Device Control Response (DR_CONTROL_RSP)

//  A message with this header describes a response to a Device Control Request (section 2.2.1.4.5).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    DeviceIoReply (16 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       OutputBufferLength                      |
// +---------------------------------------------------------------+
// |                     OutputBuffer (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoReply (16 bytes):  A DR_DEVICE_IOCOMPLETION header. The CompletionId field of this header MUST match a Device I/O Request (section 2.2.1.4) that had the MajorFunction field set to IRP_MJ_DEVICE_CONTROL.

// OutputBufferLength (4 bytes):  A 32-bit unsigned integer that specifies the number of bytes in the OutputBuffer field.

// OutputBuffer (variable):  A variable-length array of bytes whose size is specified by the OutputBufferLength field.

struct DriveControlResponse {

    uint32_t OutputBufferLength = 0;

    explicit DriveControlResponse() = default;

    explicit DriveControlResponse(uint32_t OutputBufferLength)
      : OutputBufferLength(OutputBufferLength)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->OutputBufferLength);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DriveControlResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->OutputBufferLength = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Control Response:");
        LOG(LOG_INFO, "          * OutputBufferLength = %u (4 bytes)", this->OutputBufferLength);
    }
};




// [MS-RDPEFS] - 2.2.1.5 Device I/O Response (DR_DEVICE_IOCOMPLETION)
// ==================================================================

// A message with this header indicates that the I/O request is complete. In
//  a Device I/O Response message, a request message is matched to the Device
//  I/O Request (section 2.2.1.4) header based on the CompletionId field
//  value. There is only one response per request.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                            DeviceId                           |
// +---------------------------------------------------------------+
// |                          CompletionId                         |
// +---------------------------------------------------------------+
// |                            IoStatus                           |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_DEVICE_IOCOMPLETION.

// DeviceId (4 bytes): A 32-bit unsigned integer. This field MUST match the
//  DeviceId field in the DR_DEVICE_IOREQUEST header for the corresponding
//  request.

// CompletionId (4 bytes): A 32-bit unsigned integer. This field MUST match
//  the CompletionId field in the DR_DEVICE_IOREQUEST header for the
//  corresponding request. After processing a response packet with this ID,
//  the same ID MUST be reused in another request.

// IoStatus (4 bytes): A 32-bit unsigned integer that specifies the NTSTATUS
//  code that indicates success or failure for the request. NTSTATUS codes
//  are specified in [MS-ERREF] section 2.3.

class DeviceIOResponse {
    uint32_t DeviceId_        = 0;
    uint32_t CompletionId_    = 0;
    erref::NTSTATUS IoStatus_ = erref::NTSTATUS::STATUS_SUCCESS;

public:
    explicit DeviceIOResponse() = default;

    explicit DeviceIOResponse(uint32_t DeviceId, uint32_t CompletionId, erref::NTSTATUS IoStatus)
    : DeviceId_(DeviceId)
    , CompletionId_(CompletionId)
    , IoStatus_(IoStatus)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->DeviceId_);
        stream.out_uint32_le(this->CompletionId_);
        stream.out_uint32_le(static_cast<uint32_t>(this->IoStatus_));
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;   // DeviceId(4) + CompletionId(4) + IoStatus(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceIOResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceId_     = stream.in_uint32_le();
        this->CompletionId_ = stream.in_uint32_le();
        this->IoStatus_     = static_cast<erref::NTSTATUS>(stream.in_uint32_le());
    }

    uint32_t DeviceId() const { return this->DeviceId_; }

    uint32_t CompletionId() const { return this->CompletionId_; }

    erref::NTSTATUS IoStatus() const { return this->IoStatus_; }

    void set_IoStatus(erref::NTSTATUS IoStatus) {
        this->IoStatus_ = IoStatus;
    }

    static size_t size() {
        return 12;  // DeviceId(4) + CompletionId(4) + IoStatus(4)
    }

    void log(int level) const {
        LOG(level, "DeviceIOResponse: DeviceId=%u CompletionId=%u IoStatus=0x%08X",
            this->DeviceId_, this->CompletionId_, static_cast<uint32_t>(this->IoStatus_));
    }

    void log() const {
        LOG(LOG_INFO, "     Device I\\O Response:");
        LOG(LOG_INFO, "          * DeviceId     = 0x%08x (4 bytes)", this->DeviceId_);
        LOG(LOG_INFO, "          * CompletionId = 0x%08x (4 bytes)", this->CompletionId_);
        LOG(LOG_INFO, "          * IoStatus     = 0x%08x (4 bytes): %s", this->IoStatus_, erref::get_NTStatus(this->IoStatus_));
    }

    bool map_completion_id(FileSystemCompletionIdManager & id_manager) {
        uint32_t const completion_id = id_manager.get_src_id(this->CompletionId_);

        id_manager.unreg_src_id(completion_id);

        if (completion_id != this->CompletionId_) {
            this->CompletionId_ = completion_id;

            return true;
        }

        return false;
    }
};

// [MS-RDPEFS] - 2.2.1.5.1 Device Create Response (DR_CREATE_RSP)
// ==============================================================

// A message with this header describes a response to a Device Create Request
//  (section 2.2.1.4.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             FileId                            |
// +---------------+-----------------------------------------------+
// |  Information  |
// |   (optional)  |
// +---------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION header. The
//  CompletionId field of this header MUST match a Device I/O Request
//  (section 2.2.1.4) message that had the MajorFunction field set to
//  IRP_MJ_CREATE.

// FileId (4 bytes): A 32-bit unsigned integer that specifies a unique ID for
//  the created file object. The ID MUST be reused after sending a Device
//  Close Response (section 2.2.1.5.2).

// Information (1 byte): An unsigned 8-bit integer. This field indicates the
//  success of the Device Create Request (section 2.2.1.4.1). The value of
//  the Information field depends on the value of CreateDisposition field in
//  the Device Create Request (section 2.2.1.4.1). If the IoStatus field is
//  set to 0x00000000, this field MAY be skipped,<6> in which case the server
//  MUST assume that the Information field is set to 0x00. The possible
//  values of the Information field are:

//  +------------------+-----------------------------------+
//  | Value            | Meaning                           |
//  +------------------+-----------------------------------+
//  | FILE_SUPERSEDED  | A new file was created.           |
//  | 0x00000000       |                                   |
//  +------------------+-----------------------------------+
//  | FILE_OPENED      | An existing file was opened.      |
//  | 0x00000001       |                                   |
//  +------------------+-----------------------------------+
//  | FILE_OVERWRITTEN | An existing file was overwritten. |
//  | 0x00000003       |                                   |
//  +------------------+-----------------------------------+

// TODO strong enum (FileStatus?)
enum : uint8_t {
      FILE_SUPERSEDED  = 0x00
    , FILE_OPENED      = 0x01
    , FILE_OVERWRITTEN = 0x03
};

//  The values of the CreateDisposition field in the Device Create Request
//  (section 2.2.1.4.1) that determine the value of the Information field
//  are associated as follows:

//  +-------------------------+-------------------------------+
//  | Information field value | CreateDisposition field value |
//  +-------------------------+-------------------------------+
//  | FILE_SUPERSEDED         | FILE_SUPERSEDE                |
//  |                         | FILE_OPEN                     |
//  |                         | FILE_CREATE                   |
//  |                         | FILE_OVERWRITE                |
//  +-------------------------+-------------------------------+
//  | FILE_OPENED             | FILE_OPEN_IF                  |
//  +-------------------------+-------------------------------+
//  | FILE_OVERWRITTEN        | FILE_OVERWRITE_IF             |
//  +-------------------------+-------------------------------+

class DeviceCreateResponse {
    uint32_t FileId_     = 0;
    uint8_t  Information_ = 0;

public:
    explicit DeviceCreateResponse() = default;

    explicit DeviceCreateResponse(uint32_t FileId, uint8_t Information)
    : FileId_(FileId)
    , Information_(Information) {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FileId_);
        stream.out_uint8(this->Information_);
    }

    void receive(InStream & stream, erref::NTSTATUS IoStatus) {
        {
            const unsigned expected =
                    4 +                 // FileId(4)
                    (IoStatus != erref::NTSTATUS::STATUS_SUCCESS ? 1 : 0)  // Information(1)
                ;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FileId_     = stream.in_uint32_le();
        this->Information_ = (IoStatus != erref::NTSTATUS::STATUS_SUCCESS ? stream.in_uint8() : 0x00);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected1 =
                    4                  // FileId(4)
                ;

            const unsigned expected2 =
                4 +                 // FileId(4)
                1  // Information(1)
            ;

            if (!stream.in_check_rem(expected1) && !stream.in_check_rem(expected2)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateResponse: expected= 4 or 5, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }


        this->FileId_     = stream.in_uint32_le();
        if (stream.in_check_rem(1)) {
            this->Information_ = stream.in_uint8();
        }
    }

    uint32_t FileId() const { return this->FileId_; }
    uint32_t Information() const { return this->Information_; }

private:
    static const char * get_Information_name(uint8_t Information) {
        switch (Information) {
            case FILE_SUPERSEDED:  return "FILE_SUPERSEDED";
            case FILE_OPENED:      return "FILE_OPENED";
            case FILE_OVERWRITTEN: return "FILE_OVERWRITTEN";
        }

        return "<unknown>";
    }

public:
    void log(int level) const {
        LOG(level, "DeviceCreateResponse: FileId=%u Information=%s(0x%X)",
            this->FileId_, this->get_Information_name(this->Information_),
            unsigned(this->Information_));
    }

    void log() const {
        LOG(LOG_INFO, "     Device Create Response:");
        LOG(LOG_INFO, "          * FileId      = 0x%08x (4 bytes)", this->FileId_);
        LOG(LOG_INFO, "          * Information = 0x%02x (1 byte) optional", this->Information_);
    }
};

// [MS-RDPEFS] - 2.2.1.5.2 Device Close Response (DR_CLOSE_RSP)
// ============================================================

// This message is a reply to a Device Close Request (section 2.2.1.4.2).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------+-----------------------------------------------+
// |      ...      |
// +---------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION header. The
//  CompletionId field of this header MUST match a Device I/O Request
//  (section 2.2.1.4) message that had the MajorFunction field set to
//  IRP_MJ_CLOSE.

// Padding (5 bytes): An array of 5 bytes. Reserved. This field can be set to
//  any value, and MUST be ignored on receipt.



// [MS-RDPEFS] - 2.2.1.5.3 Device Read Response (DR_READ_RSP)
// ==========================================================

// A message with this header describes a response to a Device Read Request
//  (section 2.2.1.4.3).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                      ReadData (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION header. The
//  CompletionId field of this header MUST match a Device I/O Request
//  (section 2.2.1.4) message that had the MajorFunction field set to
//  IRP_MJ_READ.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the ReadData field.

// ReadData (variable): A variable-length array of bytes that specifies the
//  output data from the read request. The length of ReadData is specified by
//  the Length field in this packet.

struct DeviceReadResponse {

    uint32_t Length = 0;

    explicit DeviceReadResponse() = default;

    explicit DeviceReadResponse( uint32_t Length)
      : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceReadResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
//         {
//             const unsigned expected = Length;
//             if (!stream.in_check_rem(expected)) {
//                 LOG(LOG_ERR,
//                     "Truncated DeviceReadResponse: expected=%u remains=%zu",
//                     expected, stream.in_remain());
//                 throw Error(ERR_RDPDR_PDU_TRUNCATED);
//             }
//         }
        //uint8_t data[0xffff];
        //stream.in_copy_bytes(data, Length);
        //this->ReadData = std::string(char_ptr_cast(data), Length/2);
    }

    void log() const {
        LOG(LOG_INFO, "     Device Read Response:");
        LOG(LOG_INFO, "          * Length   = %u (4 bytes)", this->Length);
    }
};

// [MS-RDPEFS] - 2.2.1.5.4 Device Write Response (DR_WRITE_RSP)
// ============================================================

// A message with this header describes a response to a Device Write Request
//  (section 2.2.1.4.4).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------+-----------------------------------------------+
// |    Padding    |
// |   (optional)  |
// +---------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION header. The
//  CompletionId field of this header MUST match a Device I/O Request
//  (section 2.2.1.4) message that had the MajorFunction field set to
//  IRP_MJ_WRITE.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes written in response to the write request.

// Padding (1 byte): An 8-bit unsigned integer intended to allow the client
//  minor flexibility in determining the overall packet length. This field is
//  unused and can be set to any value. If present, this field MUST be
//  ignored on receipt.

struct DeviceWriteResponse {

    uint32_t Length = 0;

    explicit DeviceWriteResponse() = default;

    explicit DeviceWriteResponse( uint32_t Length)
    : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
        stream.out_clear_bytes(1);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceWriteResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Device Write Response:");
        LOG(LOG_INFO, "          * Length = %u (4 bytes)", this->Length);
        LOG(LOG_INFO, "          * Padding - (1 byte) NOT USED");
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
    uint32_t DeviceId_   = 0;
    erref::NTSTATUS ResultCode_ = erref::NTSTATUS::STATUS_SUCCESS;

public:
    explicit ServerDeviceAnnounceResponse() = default;

    explicit ServerDeviceAnnounceResponse(uint32_t device_id, erref::NTSTATUS result_code) :
        DeviceId_(device_id), ResultCode_(result_code) {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->DeviceId_);
        stream.out_uint32_le(static_cast<uint32_t>(this->ResultCode_));
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 8;   // DeviceId(4) + ResultCode(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDeviceAnnounceResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceId_   = stream.in_uint32_le();
        this->ResultCode_ = erref::NTSTATUS(stream.in_uint32_le());
    }

    uint32_t DeviceId() const { return this->DeviceId_; }

    erref::NTSTATUS ResultCode() const { return this->ResultCode_; }

    void log(int level) const {
        LOG(level, "ServerDeviceAnnounceResponse: DeviceId=%" PRIu32 " ResultCode=0x%08" PRIX32,
            this->DeviceId_, underlying_cast(this->ResultCode_));
    }

    void log() const {
        LOG(LOG_INFO, "     Server Device Announce Response:");
        LOG(LOG_INFO, "          * DeviceId   = 0x%08x (4 bytes)", this->DeviceId_);
        LOG(LOG_INFO, "          * ResultCode = 0x%08x (4 bytes): %s", this->ResultCode_, erref::get_NTStatus(this->ResultCode_));
    }
};

// [MS-RDPEFS] - 2.2.2.2 Server Announce Request
//  (DR_CORE_SERVER_ANNOUNCE_REQ)
// =============================================

// The server initiates the protocol with this message.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +-------------------------------+-------------------------------+
// |          VersionMajor         |          VersionMinor         |
// +-------------------------------+-------------------------------+
// |                            ClientId                           |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_SERVER_ANNOUNCE.

// VersionMajor (2 bytes): A 16-bit unsigned integer that specifies the
//  server major version number. This field MUST be set to 0x0001.

// VersionMinor (2 bytes): A 16-bit unsigned integer that specifies the
//  server minor version number. This field MUST be set to one of several
//  values<7>.

// ClientId (4 bytes): A 32-bit unsigned integer that specifies the unique ID
//  generated by the server as specified in section 3.3.5.1.2.

class ServerAnnounceRequest {
    uint16_t VersionMajor_  = 0;
    uint16_t VersionMinor_ = 0;
    uint16_t ClientId_     = 0;

public:

    explicit ServerAnnounceRequest() = default;

    explicit ServerAnnounceRequest(uint16_t VersionMajor_, uint16_t VersionMinor_, uint16_t ClientId_)
      : VersionMajor_(VersionMajor_)
      , VersionMinor_(VersionMinor_)
      , ClientId_(ClientId_)
      {}


    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->VersionMajor_);
        stream.out_uint16_le(this->VersionMinor_);
        stream.out_uint32_le(this->ClientId_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 8;   // VersionMajor(2) + VersionMajor(2) + ClientId(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerAnnounceRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->VersionMajor_  = stream.in_uint16_le();
        this->VersionMinor_ = stream.in_uint16_le();
        this->ClientId_     = stream.in_uint32_le();
    }

    uint16_t VersionMajor() const { return this->VersionMajor_; }

    uint16_t VersionMinor() const { return this->VersionMinor_; }

    uint16_t ClientId() const { return this->ClientId_; }

    void log(int level) const {
        LOG(level, "ServerAnnounceRequest: VersionMajor=0x%04X VersionMinor=0x%04X ClientId=%u",
            unsigned(this->VersionMajor_), unsigned(this->VersionMinor_), unsigned(this->ClientId_));
    }

    void log() const {
        LOG(LOG_INFO, "     Server Announce Request:");
        LOG(LOG_INFO, "          * VersionMajor = 0x%04X (2 bytes) ", this->VersionMajor_);
        LOG(LOG_INFO, "          * VersionMinor = 0x%04X (2 bytes) ", this->VersionMinor_);
        LOG(LOG_INFO, "          * ClientId     = 0x%08X (4 bytes) ", this->ClientId_);
    }
};

// [MS-RDPEFS] - 2.2.2.3 Client Announce Reply (DR_CORE_CLIENT_ANNOUNCE_RSP)
// =========================================================================

// The client replies to the Server Announce Request message.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +-------------------------------+-------------------------------+
// |          VersionMajor         |          VersionMinor         |
// +-------------------------------+-------------------------------+
// |                            ClientId                           |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_CLIENTID_CONFIRM.

// VersionMajor (2 bytes): A 16-bit unsigned integer that specifies the
//  client major version number. This field MUST be set to 0x0001.

// VersionMinor (2 bytes): A 16-bit unsigned integer that specifies the
//  client minor version number. This field MUST be set to one of the
//  following values.

//  +--------+------------------------+
//  | Value  | Meaning                |
//  +--------+------------------------+
//  | 0x000C | RDP Client 6.0 and 6.1 |
//  +--------+------------------------+
//  | 0x000A | RDP Client 5.2         |
//  +--------+------------------------+
//  | 0x0005 | RDP Client 5.1         |
//  +--------+------------------------+
//  | 0x0002 | RDP Client 5.0         |
//  +--------+------------------------+

// ClientId (4 bytes): A 32-bit unsigned integer that the client MUST set to
//  either the ClientID field, which is supplied by the server in the Server
//  Announce Request message, or a unique ID as specified in section
//  3.2.5.1.3.

class ClientAnnounceReply {

public:
    uint16_t VersionMajor = 0;
    uint16_t VersionMinor = 0;
    uint16_t ClientId     = 0;


    explicit ClientAnnounceReply() = default;

    explicit ClientAnnounceReply(uint16_t VersionMajor, uint16_t VersionMinor, uint16_t ClientId)
    : VersionMajor(VersionMajor)
    , VersionMinor(VersionMinor)
    , ClientId(ClientId) {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->VersionMajor);
        stream.out_uint16_le(this->VersionMinor);
        stream.out_uint32_le(this->ClientId);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 8;   // VersionMajor(2) + VersionMajor(2) + ClientId(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientAnnounceReply: expected=%u remains=%zu",
                    expected, stream.in_remain());

                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->VersionMajor = stream.in_uint16_le();
        this->VersionMinor = stream.in_uint16_le();
        this->ClientId     = stream.in_uint32_le();
    }

    void log(int level) const {
        LOG(level, "ClientAnnounceReply: VersionMajor=0x%04X VersionMinor=0x%04X ClientId=%u",
            unsigned(this->VersionMajor), unsigned(this->VersionMinor), unsigned(this->ClientId));
    }

    void log() const {
        LOG(LOG_INFO, "     Client Announce Reply:");
        LOG(LOG_INFO, "          * VersionMajor = 0x%04x (2 bytes)", this->VersionMajor);
        LOG(LOG_INFO, "          * VersionMinor = 0x%04x (2 bytes)", this->VersionMinor);
        LOG(LOG_INFO, "          * ClientId     = 0x%08x (4 bytes)", this->ClientId);
    }
};

// [MS-RDPEFS] - 2.2.2.4 Client Name Request (DR_CORE_CLIENT_NAME_REQ)
// ===================================================================

// The client announces its machine name.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                          UnicodeFlag                          |
// +---------------------------------------------------------------+
// |                            CodePage                           |
// +---------------------------------------------------------------+
// |                        ComputerNameLen                        |
// +---------------------------------------------------------------+
// |                    ComputerName (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set
//  to RDPDR_CTYP_CORE, and the PacketId field MUST be set to
//  PAKID_CORE_CLIENT_NAME.

// UnicodeFlag (4 bytes): A 32-bit unsigned integer that indicates the format
//  of the ComputerName field. Only the least significant bit of this field is
//  valid (the most significant 31 bits MUST be ignored). This field MUST be
//  set to one of the following values.

//  +------------+----------------------------------------+
//  | Value      | Meaning                                |
//  +------------+----------------------------------------+
//  | 0x00000001 | ComputerName is in Unicode characters. |
//  +------------+----------------------------------------+
//  | 0x00000000 | ComputerName is in ASCII characters.   |
//  +------------+----------------------------------------+

enum {
  ASCII_CHAR   = 0x00000000,
  UNICODE_CHAR = 0x00000001
};

// CodePage (4 bytes): A 32-bit unsigned integer that specifies the code page
//  of the ComputerName field; it MUST be set to 0.

// ComputerNameLen (4 bytes): A 32-bit unsigned integer that specifies the
//  number of bytes in the ComputerName field, including null terminator.

// ComputerName (variable): A variable-length array of ASCII or Unicode
//  characters, the format of which is determined by the UnicodeFlag field.
//  This is a string that identifies the client computer name. The string
//  MUST be null-terminated. The protocol imposes no limitations on the
//  characters used in this field.

class ClientNameRequest {
    uint32_t UnicodeFlag = 0x000007ff /* ComputerName is in Unicode characters. */;
    uint32_t CodePage    = 0;

    size_t ComputerNameLen = 0;

    char ComputerName[500];

public:
    explicit ClientNameRequest() = default;

    explicit ClientNameRequest(const char * computer_name)
    {
//         assert(this->ComputerNameLen <= 500)-1);
        std::memcpy(this->ComputerName, computer_name,  500);
    }

    explicit ClientNameRequest(const char * computer_name, const uint32_t unicodeFlag)
    : UnicodeFlag(unicodeFlag)
    , ComputerNameLen(strlen(computer_name))
    {
        this->ComputerNameLen = std::min(this->ComputerNameLen, sizeof(ComputerName) - 1);
        assert(this->ComputerNameLen == strlen(computer_name));
        std::memcpy(this->ComputerName, computer_name, this->ComputerNameLen);
        std::memset(this->ComputerName + this->ComputerNameLen, 0, sizeof(ComputerName)-this->ComputerNameLen);
    }


    void emit(OutStream & stream)  {
        stream.out_uint32_le(this->UnicodeFlag);
        stream.out_uint32_le(this->CodePage);

        if (this->UnicodeFlag & 0x00000001) {
            // ComputerName is in Unicode characters.

            // The null-terminator is included.
            uint8_t ComputerName_unicode_data[65536];
            size_t size_of_ComputerName_unicode_data = ::UTF8toUTF16(
                {this->ComputerName, this->ComputerNameLen},
                ComputerName_unicode_data, sizeof(ComputerName_unicode_data));
            // Writes null terminator.
            ComputerName_unicode_data[size_of_ComputerName_unicode_data    ] =
            ComputerName_unicode_data[size_of_ComputerName_unicode_data + 1] = 0;
            size_of_ComputerName_unicode_data += 2;

            stream.out_uint32_le(size_of_ComputerName_unicode_data);

            stream.out_copy_bytes(ComputerName_unicode_data, size_of_ComputerName_unicode_data);
        } else {
            // The null-terminator is included.
            this->ComputerName[this->ComputerNameLen] = '\0';
            this->ComputerNameLen += 1;
            stream.out_uint32_le(this->ComputerNameLen);
            stream.out_copy_bytes(byte_ptr_cast(this->ComputerName), this->ComputerNameLen);
        }
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;  // UnicodeFlag(4) + CodePage(4) +
                                           //     ComputerNameLen(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientNameRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->UnicodeFlag = stream.in_uint32_le();
        this->CodePage    = stream.in_uint32_le();

        this->ComputerNameLen = stream.in_uint32_le();
        if (this->ComputerNameLen) {
            {
                const unsigned expected = ComputerNameLen;  // ComputerName(variable)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated ClientNameRequest (1): expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDPDR_PDU_TRUNCATED);
                }
            }

            // Remote Desktop Connection of Windows XP (Shell Version 6.1.7600,
            //  Control Version 6.1.7600) has a bug. The field UnicodeFlag
            //  contains inconsistent data.
            if (this->UnicodeFlag & 0x00000001) {
                // ComputerName is in Unicode characters.

                uint8_t const * ComputerName_unicode_data = stream.get_current();

                const size_t ComputerName_utf8_len = ::UTF16toUTF8(
                    ComputerName_unicode_data,
                    this->ComputerNameLen,
                    byte_ptr_cast(this->ComputerName),
                    sizeof(this->ComputerName));

                this->ComputerName[ComputerName_utf8_len] = '\0';

                stream.in_skip_bytes(ComputerNameLen);
            } else {
                // The null-terminator is included.
                std::memcpy(this->ComputerName, stream.get_current(), this->ComputerNameLen);
            }
        }
    }

    void log(int level) const {
        LOG(level, "ClientNameRequest: UnicodeFlag=0x%X CodePage=%u ComputerName=\"%s\"",
            this->UnicodeFlag, this->CodePage, this->ComputerName);
    }

    void log() const {
        LOG(LOG_INFO, "     Client Name Request:");
        LOG(LOG_INFO, "          * UnicodeFlag     = 0x%08x (4 bytes)", this->UnicodeFlag);
        LOG(LOG_INFO, "          * CodePage        = 0x%08x (4 bytes)", this->CodePage);
        LOG(LOG_INFO, "          * ComputerNameLen = %zu (4 bytes)", this->ComputerNameLen);
        LOG(LOG_INFO, "          * ComputerName    = \"%s\" (%zu byte(s))", this->ComputerName, this->ComputerNameLen);
    }

};  // ClientNameRequest

// [MS-RDPEFS] - 2.2.2.7.1 General Capability Set (GENERAL_CAPS_SET)
// =================================================================

// This packet is embedded into Server Core Capability Request and Client
//  Core Capability Response messages. It describes non–device-specific
//  capabilities.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             osType                            |
// +---------------------------------------------------------------+
// |                           osVersion                           |
// +-------------------------------+-------------------------------+
// |      protocolMajorVersion     |      protocolMinorVersion     |
// +-------------------------------+-------------------------------+
// |                            ioCode1                            |
// +---------------------------------------------------------------+
// |                            ioCode2                            |
// +---------------------------------------------------------------+
// |                          extendedPDU                          |
// +---------------------------------------------------------------+
// |                          extraFlags1                          |
// +---------------------------------------------------------------+
// |                          extraFlags2                          |
// +---------------------------------------------------------------+
// |                      SpecialTypeDeviceCap                     |
// +---------------------------------------------------------------+

// Header (8 bytes): A CAPABILITY_HEADER header. The CapabilityType field of
//  this header MUST be set to CAP_GENERAL_TYPE. The Version field of this
//  header MUST have one of the following values.

//  +-------------------------------+---------------------------------------+
//  | Value                         | Meaning                               |
//  +-------------------------------+---------------------------------------+
//  | GENERAL_CAPABILITY_VERSION_01 | Version 1. The SpecialTypeDeviceCap   |
//  | 0x00000001                    | field of GENERAL_CAPS_SET is not      |
//  |                               | present.                              |
//  +-------------------------------+---------------------------------------+
//  | GENERAL_CAPABILITY_VERSION_02 | Version 2. The SpecialTypeDeviceCap   |
//  | 0x00000002                    | field of GENERAL_CAPS_SET is present. |
//  +-------------------------------+---------------------------------------+

// osType (4 bytes): A 32-bit unsigned integer that is the identifier for the
//  operating system that the capabilities are describing. The value of this
//  field MUST be ignored on receipt.

// osVersion (4 bytes): A 32-bit unsigned integer. This field is unused, and
//  MUST be set to 0.

// protocolMajorVersion (2 bytes): A 16-bit unsigned integer. This field MUST
//  be set to 1.

// protocolMinorVersion (2 bytes): A 16-bit unsigned integer. This field MUST
//  be set to one of the values described by the VersionMinor field of the
//  Server Client ID Confirm (section 2.2.2.6) packet.

// ioCode1 (4 bytes): A 32-bit unsigned integer that identifies a bitmask of
//  the supported I/O requests for the given device. If the bit is set, the
//  I/O request is allowed. The requests are identified by the MajorFunction
//  field in the Device I/O Request (section 2.2.1.4) header. This field MUST
//  be set to a valid combination of the following values.

//  +---------------------------------------+--------------------------------+
//  | Value                                 | Meaning                        |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_CREATE                   | Unused, always set.            |
//  | 0x00000001                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_CLEANUP                  | Unused, always set.            |
//  | 0x00000002                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_CLOSE                    | Unused, always set.            |
//  | 0x00000004                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_READ                     | Unused, always set.            |
//  | 0x00000008                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_WRITE                    | Unused, always set.            |
//  | 0x00000010                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_FLUSH_BUFFERS            | Unused, always set.            |
//  | 0x00000020                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_SHUTDOWN                 | Unused, always set.            |
//  | 0x00000040                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_DEVICE_CONTROL           | Unused, always set.            |
//  | 0x00000080                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_QUERY_VOLUME_INFORMATION | Unused, always set.            |
//  | 0x00000100                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_SET_VOLUME_INFORMATION   | Unused, always set.            |
//  | 0x00000200                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_QUERY_INFORMATION        | Unused, always set.            |
//  | 0x00000400                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_SET_INFORMATION          | Unused, always set.            |
//  | 0x00000800                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_DIRECTORY_CONTROL        | Unused, always set.            |
//  | 0x00001000                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_LOCK_CONTROL             | Unused, always set.            |
//  | 0x00002000                            |                                |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_QUERY_SECURITY           | Enable Query Security requests |
//  | 0x00004000                            | (IRP_MJ_QUERY_SECURITY).       |
//  +---------------------------------------+--------------------------------+
//  | RDPDR_IRP_MJ_SET_SECURITY             | Enable Set Security requests   |
//  | 0x00008000                            | (IRP_MJ_SET_SECURITY).         |
//  +---------------------------------------+--------------------------------+

enum {
      SUPPORT_ALL_REQUEST = 0xFFFF
};

// ioCode2 (4 bytes): A 32-bit unsigned integer that is currently reserved
//  for future use, and MUST be set to 0.

// extendedPDU (4 bytes): A 32-bit unsigned integer that specifies extended
//  PDU flags. This field MUST be set as a bitmask of the following values.

//  +-------------------------------+----------------------------------------+
//  | Value                         | Meaning                                |
//  +-------------------------------+----------------------------------------+
//  | RDPDR_DEVICE_REMOVE_PDUS      | Allow the client to send Client Drive  |
//  | 0x00000001                    | Device List Remove packets.            |
//  +-------------------------------+----------------------------------------+
//  | RDPDR_CLIENT_DISPLAY_NAME_PDU | Unused, always set.                    |
//  | 0x00000002                    |                                        |
//  +-------------------------------+----------------------------------------+
//  | RDPDR_USER_LOGGEDON_PDU       | Allow the server to send a Server User |
//  | 0x00000004                    | Logged On packet.                      |
//  +-------------------------------+----------------------------------------+

enum {
      RDPDR_DEVICE_REMOVE_PDUS      = 0x00000001
    , RDPDR_CLIENT_DISPLAY_NAME_PDU = 0x00000002
    , RDPDR_USER_LOGGEDON_PDU       = 0x00000004
};

// extraFlags1 (4 bytes): A 32-bit unsigned integer that specifies extended
//  flags. The extraFlags1 field MUST be set as a bitmask of the following
//  value.

//  +----------------+-------------------------------------------------------+
//  | Value          | Meaning                                               |
//  +----------------+-------------------------------------------------------+
//  | ENABLE_ASYNCIO | Optionally present only in the Client Core Capability |
//  | 0x00000001     | Response. Allows the server to send multiple          |
//  |                | simultaneous read or write requests on the same file  |
//  |                | from a redirected file system.<8>                     |
//  +----------------+-------------------------------------------------------+


enum {
      ENABLE_ASYNCIO = 0x00000001
};

enum {
      MINOR_VERSION_13 = 0x000D
    , MINOR_VERSION_12 = 0x000C
    , MINOR_VERSION_10 = 0x000A
    , MINOR_VERSION_5  = 0x0005
    , MINOR_VERSION_2  = 0x0002
};

// extraFlags2 (4 bytes): A 32-bit unsigned integer that is currently
//  reserved for future use, and MUST be set to 0.

// SpecialTypeDeviceCap (4 bytes): A 32-bit unsigned integer that specifies
//  the number of special devices to be redirected before the user is logged
//  on. Special devices are those that are safe and/or required to be
//  redirected before a user logs on (such as smart cards and serial ports).

class GeneralCapabilitySet {

public:
    uint32_t osType               = 0;
    uint32_t osVersion            = 0;
    uint16_t protocolMajorVersion = 0x1;
    uint16_t protocolMinorVersion = 0;
    uint32_t ioCode1              = 0;
    uint32_t ioCode2              = 0;
    uint32_t extendedPDU_         = 0;
    uint32_t extraFlags1_         = 0;
    uint32_t extraFlags2          = 0;
    uint32_t SpecialTypeDeviceCap = 0;
    uint32_t version              = 0;


    explicit GeneralCapabilitySet() = default;

    explicit GeneralCapabilitySet(
        uint32_t osType,
        uint16_t protocolMinorVersion,
        uint32_t ioCode1, uint32_t extendedPDU,
        uint32_t extraFlags1, uint32_t SpecialTypeDeviceCap,
        uint32_t version)
    : osType(osType)
    , protocolMinorVersion(protocolMinorVersion)
    , ioCode1(ioCode1)
    , extendedPDU_(extendedPDU)
    , extraFlags1_(extraFlags1)
    , SpecialTypeDeviceCap(SpecialTypeDeviceCap)
    , version(version) {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->osType);
        stream.out_uint32_le(this->osVersion);
        stream.out_uint16_le(this->protocolMajorVersion);
        stream.out_uint16_le(this->protocolMinorVersion);
        stream.out_uint32_le(this->ioCode1);
        stream.out_uint32_le(this->ioCode2);
        stream.out_uint32_le(this->extendedPDU_);
        stream.out_uint32_le(this->extraFlags1_);
        stream.out_uint32_le(this->extraFlags2);
        if (this->version == GENERAL_CAPABILITY_VERSION_02) {
            stream.out_uint32_le(this->SpecialTypeDeviceCap);
        }
    }

    void receive(InStream & stream, uint32_t version) {
        {
            const unsigned expected = 32 +  // osType(4) + osVersion(4) + protocolMajorVersion(2) +
                                            // protocolMinorVersion(2) + ioCode1(4) + ioCode2(4) +
                                            // extendedPDU(4) + extraFlags1(4) + extraFlags2(4)
                ((version == GENERAL_CAPABILITY_VERSION_02) ? 4 /* SpecialTypeDeviceCap(4) */ : 0);

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated GeneralCapabilitySet: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->osType               = stream.in_uint32_le();
        this->osVersion            = stream.in_uint32_le();
        this->protocolMajorVersion = stream.in_uint16_le();
        this->protocolMinorVersion = stream.in_uint16_le();
        this->ioCode1              = stream.in_uint32_le();
        this->ioCode2              = stream.in_uint32_le();
        this->extendedPDU_         = stream.in_uint32_le();
        this->extraFlags1_         = stream.in_uint32_le();
        this->extraFlags2          = stream.in_uint32_le();
        if (version == GENERAL_CAPABILITY_VERSION_02) {
            this->SpecialTypeDeviceCap = stream.in_uint32_le();
        }
    }

    uint32_t extendedPDU() const { return this->extendedPDU_; }

    void set_extendedPDU(uint32_t extendedPDU) {
        this->extendedPDU_ = extendedPDU;
    }

    uint32_t extraFlags1() const { return this->extraFlags1_; }

    void set_extraFlags1(uint32_t extraFlags1) {
        this->extraFlags1_ = extraFlags1;
    }

    static size_t size(uint32_t version) {
        return 32 + // osType(4) + osVersion(4) + protocolMajorVersion(2) +
                    // protocolMinorVersion(2) + ioCode1(4) + ioCode2(4) +
                    // extendedPDU(4) + extraFlags1(4) + extraFlags2(4)
            ((version == GENERAL_CAPABILITY_VERSION_02) ? 4 /* SpecialTypeDeviceCap(4) */ : 0);
    }

    void log(int level) const {
        LOG(level,
            "GeneralCapabilitySet: osType=0x%X osVersion=0x%X "
                "protocolMajorVersion=0x%X protocolMinorVersion=0x%X "
                "ioCode1=0x%X ioCode2=0x%X extendedPDU=0x%X extraFlags1=0x%X "
                "extraFlags2=0x%X SpecialTypeDeviceCap=%u",
            this->osType, this->osVersion, unsigned(this->protocolMajorVersion),
            unsigned(this->protocolMinorVersion), this->ioCode1, this->ioCode2,
            this->extendedPDU_, this->extraFlags1_, this->extraFlags2,
            this->SpecialTypeDeviceCap);
    }

    void log() const {
        LOG(LOG_INFO, "     General Capability Set:");
        LOG(LOG_INFO, "          * osType               = 0x%08x (4 bytes)", this->osType);
        LOG(LOG_INFO, "          * osVersion            = 0x%08x (4 bytes)", this->osVersion);
        LOG(LOG_INFO, "          * protocolMajorVersion = 0x%04x (2 bytes)", this->protocolMajorVersion);
        LOG(LOG_INFO, "          * protocolMinorVersion = 0x%04x (2 bytes)", this->protocolMinorVersion);
        LOG(LOG_INFO, "          * ioCode1              = 0x%08x (4 bytes)", this->ioCode1);
        LOG(LOG_INFO, "          * ioCode2              = 0x%08x (4 bytes)", this->ioCode2);
        LOG(LOG_INFO, "          * extendedPDU          = 0x%08x (4 bytes)", this->extendedPDU_);
        LOG(LOG_INFO, "          * extraFlags1          = 0x%08x (4 bytes)", this->extraFlags1_);
        LOG(LOG_INFO, "          * extraFlags2          = 0x%08x (4 bytes)", this->extraFlags2);
        LOG(LOG_INFO, "          * SpecialTypeDeviceCap = 0x%08x (4 bytes)", this->SpecialTypeDeviceCap);
    }
};  // GeneralCapabilitySet

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

struct ClientDeviceListAnnounceRequest {

    uint32_t DeviceCount = 0;

    explicit ClientDeviceListAnnounceRequest() = default;

    explicit ClientDeviceListAnnounceRequest(uint32_t DeviceCount) : DeviceCount(DeviceCount)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->DeviceCount);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDeviceListAnnounceRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->DeviceCount = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Device List Announce Request:");
        LOG(LOG_INFO, "          * DeviceCount = %u (4 bytes)", this->DeviceCount);
    }

};



// [MS-RDPEFS] - 2.2.3.3.4 Server Drive Write Request (DR_DRIVE_WRITE_REQ)
// =======================================================================

// The server writes to a file on a redirected file system device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 DeviceWriteRequest (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceWriteRequest (variable): A DR_WRITE_REQ header. The Length field
//  contains the number of bytes to be written to the number of bytes to be
//  written to the file. The Offset field specifies the offset within the
//  file at which the write operation starts.

// [MS-RDPEFS] - 2.2.3.3.8 Server Drive Query Information Request
//  (DR_DRIVE_QUERY_INFORMATION_REQ)
// ==============================================================

// The server issues a query information request on a redirected file system
//  device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       FsInformationClass                      |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                     QueryBuffer (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4)
//  header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST
//  be set to IRP_MJ_QUERY_INFORMATION.

// FsInformationClass (4 bytes): A 32-bit unsigned integer. The possible
//  values for this field are defined in [MS-FSCC] section 2.4. This field
//  MUST contain one of the following values.

//  +-----------------------------+-------------------------------------------+
//  | Value                       | Meaning                                   |
//  +-----------------------------+-------------------------------------------+
//  | FileBasicInformation        | This information class is used to query a |
//  | 0x00000004                  | file for the times of creation, last      |
//  |                             | access, last write, and change, in        |
//  |                             | addition to file attribute information.   |
//  +-----------------------------+-------------------------------------------+
//  | FileStandardInformation     | This information class is used to query   |
//  | 0x00000005                  | for file information such as  allocation  |
//  |                             | size, end-of-file position, and number of |
//  |                             | links.                                    |
//  +-----------------------------+-------------------------------------------+
//  | FileAttributeTagInformation | This information class is used to query   |
//  | 0x00000023                  | for file attribute and reparse tag        |
//  |                             | information.                              |
//  +-----------------------------+-------------------------------------------+

enum : uint32_t {
      FileBasicInformation        = 0x00000004
    , FileStandardInformation     = 0x00000005
    , FileAttributeTagInformation = 0x00000023
};

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the QueryBuffer field.

// Padding (24 bytes): An array of 24 bytes. This field is unused and can be
//  set to any value. This field MUST be ignored on receipt.

// QueryBuffer (variable): A variable-length array of bytes. The size of the
//  array is specified by the Length field. The content of this field is
//  based on the value of the FsInformationClass field, which determines the
//  different structures that MUST be contained in the QueryBuffer field. For
//  a complete list of these structures, see [MS-FSCC] section 2.4. The "File
//  information class" table defines all the possible values for the
//  FsInformationClass field.



class ServerDriveQueryInformationRequest {

public:
    uint32_t FsInformationClass_ = 0;

    struct { uint8_t const * p; std::size_t sz; } query_buffer
      = {cbyte_ptr("").to_u8p(), 0u};

public:
    explicit ServerDriveQueryInformationRequest(uint32_t FsInformationClass)
      : FsInformationClass_(FsInformationClass)
    {}

    explicit ServerDriveQueryInformationRequest() = default;

    REDEMPTION_NON_COPYABLE(ServerDriveQueryInformationRequest);

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FsInformationClass_);

        stream.out_uint32_le(this->query_buffer.sz);    // Length(4)

        stream.out_clear_bytes(24); // Padding(24)

        stream.out_copy_bytes(this->query_buffer.p, this->query_buffer.sz);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // FsInformationClass(4) + Length(4) + Padding(24)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryInformationRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FsInformationClass_ = stream.in_uint32_le();

        const uint32_t Length = stream.in_uint32_le();
        assert(!Length);

        stream.in_skip_bytes(24);   // Padding(24)

        {
            const unsigned expected = Length;  // QueryBuffer(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryInformationRequest (1): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->query_buffer = {stream.get_current(), Length};
        stream.in_skip_bytes(Length);
    }

    uint32_t FsInformationClass() const { return this->FsInformationClass_; }

    uint32_t Length() const { return this->query_buffer.sz; }

    static const char * get_FsInformationClass_name(uint32_t FsInformationClass) {
        switch (FsInformationClass) {
            case FileBasicInformation:        return "FileBasicInformation";
            case FileStandardInformation:     return "FileStandardInformation";
            case FileAttributeTagInformation: return "FileAttributeTagInformation";
        }

        return "<unknown>";
    }

    void log(int level) const {
        LOG(level, "ServerDriveQueryInformationRequest: FsInformationClass=%s(0x%X) Length=%zu",
            this->get_FsInformationClass_name(this->FsInformationClass_),
            this->FsInformationClass_, this->query_buffer.sz);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Query Information Request:");
        LOG(LOG_INFO, "          * FsInformationClass = 0x%08x (4 bytes): %s", this->FsInformationClass_, this->get_FsInformationClass_name(this->FsInformationClass_));
        LOG(LOG_INFO, "          * Length             = %zu (4 bytes)", this->query_buffer.sz);
        LOG(LOG_INFO, "          * Padding - (24 bytes) NOT USED");
    }

};  // ServerDriveQueryInformationRequest



// [MS-RDPEFS] - 2.2.3.3.5 Server Drive Control Request
//  (DR_DRIVE_CONTROL_REQ)
// ====================================================

// The server issues a device control request on a redirected file system
//  device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                DeviceControlRequest (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceControlRequest (variable): A DR_CONTROL_REQ header. The packet has a
//  structure as defined in Device Control Request (section 2.2.1.4.5). The
//  possible values for the IoControlCode field are a subset of the file
//  system control (FSCTL) commands specified in [MS-FSCC] section 2.3. The
//  content of the InputBuffer field is defined in the request type messages
//  that are specified in the same section of [MS-FSCC].

// The following list indicates the FSCTL commands supported by this protocol.

// * FSCTL_CREATE_OR_GET_OBJECT_ID
// * FSCTL_DELETE_OBJECT_ID
// * FSCTL_DELETE_REPARSE_POINT
// * FSCTL_FILESYSTEM_GET_STATISTICS
// * FSCTL_FIND_FILES_BY_SID
// * FSCTL_GET_COMPRESSION
// * FSCTL_GET_NTFS_VOLUME_DATA
// * FSCTL_GET_OBJECT_ID
// * FSCTL_GET_REPARSE_POINT
// * FSCTL_GET_RETRIEVAL_POINTERS
// * FSCTL_IS_PATHNAME_VALID
// * FSCTL_LMR_GET_LINK_TRACKING_INFORMATION
// * FSCTL_LMR_SET_LINK_TRACKING_INFORMATION
// * FSCTL_PIPE_TRANSCEIVE
// * FSCTL_PIPE_WAIT
// * FSCTL_QUERY_ALLOCATED_RANGES
// * FSCTL_READ_FILE_USN_DATA
// * FSCTL_RECALL_FILE
// * FSCTL_SET_COMPRESSION
// * FSCTL_SET_ENCRYPTION
// * FSCTL_SET_OBJECT_ID
// * FSCTL_SET_OBJECT_ID_EXTENDED
// * FSCTL_SET_REPARSE_POINT
// * FSCTL_SET_SHORT_NAME_BEHAVIOR
// * FSCTL_SET_SPARSE
// * FSCTL_SET_ZERO_DATA
// * FSCTL_SET_ZERO_ON_DEALLOCATION
// * FSCTL_SIS_COPYFILE
// * FSCTL_WRITE_USN_CLOSE_RECORD

// [MS-RDPEFS] - 2.2.3.3.6 Server Drive Query Volume Information Request
//  (DR_DRIVE_QUERY_VOLUME_INFORMATION_REQ)
// =====================================================================

// The server issues a query volume information request on a redirected file
//  system device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       FsInformationClass                      |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                  QueryVolumeBuffer (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4)
//  header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be
//  set to IRP_MJ_QUERY_VOLUME_INFORMATION.

// FsInformationClass (4 bytes): A 32-bit unsigned integer. The possible
//  values for this field are specified in [MS-FSCC] section 2.5. This field
//  MUST contain one of the following values.

//  +----------------------------+---------------------------------------------+
//  | Value                      | Meaning                                     |
//  +----------------------------+---------------------------------------------+
//  | FileFsVolumeInformation    | Used to query information for a volume on   |
//  | 0x00000001                 | which a file system is mounted.             |
//  +----------------------------+---------------------------------------------+
//  | FileFsSizeInformation      | Used to query sector size information for a |
//  | 0x00000003                 | file system volume.                         |
//  +----------------------------+---------------------------------------------+
//  | FileFsAttributeInformation | Used to query attribute information for a   |
//  | 0x00000005                 | file system.                                |
//  +----------------------------+---------------------------------------------+
//  | FileFsFullSizeInformation  | Used to query sector size information for a |
//  | 0x00000007                 | file system volume.                         |
//  +----------------------------+---------------------------------------------+
//  | FileFsDeviceInformation    | Used to query device information for a file |
//  | 0x00000004                 | system volume.                              |
//  +----------------------------+---------------------------------------------+

enum {
      FileFsVolumeInformation    = 0x00000001
    , FileFsSizeInformation      = 0x00000003
    , FileFsAttributeInformation = 0x00000005
    , FileFsFullSizeInformation  = 0x00000007
    , FileFsDeviceInformation    = 0x00000004
};

static const char * get_FsInformationClass_name(uint32_t FsInformationClass) {
    switch (FsInformationClass) {
        case FileFsVolumeInformation:    return "FileFsVolumeInformation";
        case FileFsSizeInformation:      return "FileFsSizeInformation";
        case FileFsAttributeInformation: return "FileFsAttributeInformation";
        case FileFsFullSizeInformation:  return "FileFsFullSizeInformation";
        case FileFsDeviceInformation:    return "FileFsDeviceInformation";
    }

    return "<unknown>";
}

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the QueryVolumeBuffer field.

// Padding (24 bytes): An array of 24 bytes. This field is unused and can be
//  set to any value. This field MUST be ignored on receipt.

// QueryVolumeBuffer (variable): A variable-length array of bytes. The size
//  of the array is specified by the Length field. The content of this field
//  is based on the value of the FsInformationClass field, which determines
//  the different structures that MUST be contained in the QueryVolumeBuffer
//  field. For a complete list of these structures, refer to [MS-FSCC]
//  section 2.5. The "File system information class" table defines all the
//  possible values for the FsInformationClass field.

class ServerDriveQueryVolumeInformationRequest {
    uint32_t FsInformationClass_ = 0;

    struct { uint8_t const * p; std::size_t sz; } query_volume_buffer = {nullptr, 0u};

public:
    explicit ServerDriveQueryVolumeInformationRequest() = default;

    REDEMPTION_NON_COPYABLE(ServerDriveQueryVolumeInformationRequest);

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FsInformationClass_);

        stream.out_uint32_le(this->query_volume_buffer.sz); // Length(4)

        stream.out_clear_bytes(24); // Padding(24)

        stream.out_copy_bytes(this->query_volume_buffer.p,
            this->query_volume_buffer.sz);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // FsInformationClass(4) + Length(4) + Padding(24)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryVolumeInformationRequest (0): "
                        "expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FsInformationClass_ = stream.in_uint32_le();

        const uint32_t Length = stream.in_uint32_le();
        assert(!Length);

        stream.in_skip_bytes(24);   // Padding(24)

        {
            const unsigned expected = Length;  // QueryVolumeBuffer(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryVolumeInformationRequest (1): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->query_volume_buffer = {stream.get_current(), Length};
        stream.in_skip_bytes(Length);
    }

    uint32_t FsInformationClass() const { return this->FsInformationClass_; }

    uint32_t Length() const { return this->query_volume_buffer.sz; }

    void log(int level) const {
        LOG(level,
            "ServerDriveQueryVolumeInformationRequest: FsInformationClass=%s(0x%X) Length=%zu",
            get_FsInformationClass_name(this->FsInformationClass_),
            this->FsInformationClass_, this->query_volume_buffer.sz);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Query Volume Information Request:");
        LOG(LOG_INFO, "          * FsInformationClass = 0x%08x (4 bytes): %s", this->FsInformationClass_, get_FsInformationClass_name(this->FsInformationClass_));
        LOG(LOG_INFO, "          * Length             = %zu (4 bytes)", this->query_volume_buffer.sz);
        LOG(LOG_INFO, "          * Padding - (4 bytes) NOT USED");
    }
};  // ServerDriveQueryVolumeInformationRequest



// 2.2.3.4.6 Client Drive Query Volume Information Response (DR_DRIVE_QUERY_VOLUME_INFORMATION_RSP)

// This message is sent by the client as a response to the Server Drive Query
// Volume Information Request (section 2.2.3.3.6).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Length                             |
// +---------------------------------------------------------------+
// |                       Buffer (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       Padding (optional)                      |
// +---------------------------------------------------------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION (section 2.2.1.5) header.
// The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST match a
// Device I/O Request (section 2.2.1.4) that has the MajorFunction field set
// to IRP_MJ_QUERY_VOLUME_INFORMATION.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number
// of bytes in the Buffer field.

// Buffer (variable): A variable-length array of bytes whose size is specified
// by the Length field. The content of this field is based on the value of the
// FsInformationClass field in the Server Drive Query Volume Information Request
// message, which determines the different structures that MUST be contained in
// the Buffer field. For a complete list of these structures, refer to [MS-FSCC]
// section 2.5. The "File system information class" table defines all the possible
// values for the FsInformationClass field.

// Padding (1 byte): An optional, 8-bit unsigned integer that is intended to allow
// the client minor flexibility in determining the overall packet length. This field
// is unused and MUST be ignored.

struct ClientDriveQueryVolumeInformationResponse {

    uint32_t Length = 0;

    explicit ClientDriveQueryVolumeInformationResponse() = default;

    explicit ClientDriveQueryVolumeInformationResponse( uint32_t Length)
      : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveQueryVolumeInformationResponse: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Query Volume Information Response:");
        LOG(LOG_INFO, "          * Length = %u (4 bytes)", this->Length);
    }
};

// [MS-RDPEFS] - 2.2.3.3.9 Server Drive Set Information Request
//  (DR_DRIVE_SET_INFORMATION_REQ)
// ============================================================

// The server issues a set information request on a redirected file system
//  device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       FsInformationClass                      |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      SetBuffer (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4)
//  header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be
//  set to IRP_MJ_SET_INFORMATION.

// FsInformationClass (4 bytes): A 32-bit unsigned integer. The possible
//  values for this field are defined in [MS-FSCC] section 2.4. The
//  FsInformationClass field is a 32-bit value, even though the values
//  described in [MS-FSCC] are single byte only. For the purposes of
//  conversion, the highest 24 bits are always set to zero. This field MUST
//  contain one of the following values.

//  +----------------------------+--------------------------------------------+
//  | Value                      | Meaning                                    |
//  +----------------------------+--------------------------------------------+
//  | FileBasicInformation       | This information class is used to set file |
//  | 0x00000004                 | information such as the times of creation, |
//  |                            | last access, last write, and change, in    |
//  |                            | addition to file attributes.               |
//  +----------------------------+--------------------------------------------+
//  | FileEndOfFileInformation   | This information class is used to set end- |
//  | 0x00000014                 | of-file information for a file.            |
//  +----------------------------+--------------------------------------------+
//  | FileDispositionInformation | This information class is used to mark a   |
//  | 0x0000000D                 | file for deletion.                         |
//  +----------------------------+--------------------------------------------+
//  | FileRenameInformation      | This information class is used to rename a |
//  | 0x0000000A                 | file.                                      |
//  +----------------------------+--------------------------------------------+
//  | FileAllocationInformation  | This information class is used to set the  |
//  | 0x00000013                 | allocation size for a file.                |
//  +----------------------------+--------------------------------------------+

enum {
      /*FileBasicInformation       = 0x00000004
    , */FileEndOfFileInformation   = 0x00000014
    , FileDispositionInformation = 0x0000000D
    , FileRenameInformation      = 0x0000000A
    , FileAllocationInformation  = 0x00000013
};

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the SetBuffer field.

// Padding (24 bytes): An array of 24 bytes. This field is unused and can be
//  set to any value. This field MUST be ignored on receipt.

// SetBuffer (variable): A variable-length array of bytes. The size of the
//  array is specified by the Length field. The content of this field is
//  based on the value of the FsInformationClass field, which determines the
//  different structures that MUST be contained in the SetBuffer field. For a
//  complete list of these structures, refer to [MS-FSCC] section 2.4. The
//  "File information class" table defines all the possible values for the
//  FsInformationClass field with the exception of the following values.

//  +----------------------------+--------------------------------------------+
//  | Value of                   |                                            |
//  | FsInformationClass         | Meaning of content of SetBuffer field      |
//  +----------------------------+--------------------------------------------+
//  | FileDispositionInformation | The buffer is empty. The Length field is   |
//  |                            | set to zero. It is implied that the        |
//  |                            | DeletePending field of the                 |
//  |                            | FILE_DISPOSITION_INFORMATION structure, as |
//  |                            | described in [MS-FSCC], is set to 1.       |
//  +----------------------------+--------------------------------------------+
//  | FileRenameInformation      | See RDP_FILE_RENAME_INFORMATION.           |
//  +----------------------------+--------------------------------------------+

class ServerDriveSetInformationRequest {
    uint32_t FsInformationClass_ = 0;
    uint32_t Length_             = 0;

public:
    explicit ServerDriveSetInformationRequest() = default;

    explicit ServerDriveSetInformationRequest(uint32_t FsInformationClass_, uint32_t Length_)
      : FsInformationClass_(FsInformationClass_)
      , Length_(Length_)
    {}


    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FsInformationClass_);
        stream.out_uint32_le(this->Length_);

        stream.out_clear_bytes(24);                        // Padding(24)
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // FsInformationClass(4) + Length(4) +
                                           //     Padding(24)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveSetInformationRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FsInformationClass_ = stream.in_uint32_le();
        this->Length_             = stream.in_uint32_le();

        stream.in_skip_bytes(24);                          // Padding(24)
    }

    uint32_t FsInformationClass() const { return this->FsInformationClass_; }

    uint32_t Length() const { return this->Length_; }

    static const char * get_FsInformationClass_name(uint32_t FsInformationClass) {
        switch (FsInformationClass) {
            case FileBasicInformation:       return "FileBasicInformation";
            case FileEndOfFileInformation:   return "FileEndOfFileInformation";
            case FileDispositionInformation: return "FileDispositionInformation";
            case FileRenameInformation:      return "FileRenameInformation";
            case FileAllocationInformation:  return "FileAllocationInformation";
        }

        return "<unknown>";
    }

    void log(int level) const {
        LOG(level,
            "ServerDriveSetInformationRequest: FsInformationClass=%s(0x%X) "
                "Length=%u",
            this->get_FsInformationClass_name(this->FsInformationClass_),
            this->FsInformationClass_, this->Length_);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Set Information Request:");
        LOG(LOG_INFO, "          * FsInformationClass = 0x%08x (4 bytes): %s", this->FsInformationClass_, get_FsInformationClass_name(this->FsInformationClass_));
        LOG(LOG_INFO, "          * Length = %u (4 bytes)", this->Length_);
        LOG(LOG_INFO, "          * Padding - (24 bytes) NOT USED");
    }
};

// [MS-RDPEFS] - 2.2.3.3.9.1 RDP_FILE_RENAME_INFORMATION
// =====================================================

// RDP_FILE_RENAME_INFORMATION is a structure representing
//  FileRenameInformation as a possible value of the FsInformationClass
//  field. All fields have the same meaning as in FILE_RENAME_INFORMATION in
//  [MS-FSCC] section 2.4.34. The differences are only in the layout of the
//  fields.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   ReplaceIf   | RootDirectory |         FileNameLength        |
// |     Exists    |               |                               |
// +---------------+---------------+-------------------------------+
// |              ...              |      FileName (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// ReplaceIfExists (1 byte): See [MS-FSCC] section 2.4.34 for a description
//  of this field.

// RootDirectory (1 byte): See [MS-FSCC] section 2.4.34 for a description of
//  this field. For network operations, the value of the RootDirectory field
//  in this structure MUST always be zero.

// FileNameLength (4 bytes): See [MS-FSCC] section 2.4.34 for a description
//  of this field.

// FileName (variable): See [MS-FSCC] section 2.4.34 for a description of
//  this field.

class RDPFileRenameInformation {
    bool     replace_if_exists_ = false;
    uint8_t  RootDirectory_     = 0;

    size_t FileNameLength = 0;

    char FileName_[500];

public:
    void emit(OutStream & stream) const {
        stream.out_uint8(this->replace_if_exists_ ? static_cast<uint8_t>(-1) : static_cast<uint8_t>(0));
        stream.out_uint8(this->RootDirectory_);

        uint8_t FileName_unicode_data[1000];
        const size_t size_of_FileName_unicode_data = ::UTF8toUTF16(
            {this->FileName_, this->FileNameLength},
            FileName_unicode_data, sizeof(FileName_unicode_data));

        uint8_t * temp_p = FileName_unicode_data;
        for (size_t i = 0; i < size_of_FileName_unicode_data; i += 2) {
            if (('/' == temp_p[0]) && (0 == temp_p[1])) {
                temp_p[0] = '\\';
            }
            temp_p += 2;
        }

        stream.out_uint32_le(size_of_FileName_unicode_data);    // FileNameLength(4)

        stream.out_copy_bytes(FileName_unicode_data,    // FileName(variable)
            size_of_FileName_unicode_data);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 6;  // ReplaceIfExists(1) + RootDirectory(1) +
                                           //     FileNameLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated RDP_FILE_RENAME_INFORMATION (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->replace_if_exists_ = (stream.in_uint8() != 0);
        this->RootDirectory_     = stream.in_uint8();

        this->FileNameLength = stream.in_uint32_le();

        if (this->FileNameLength) {
            {
                const unsigned expected = this->FileNameLength;  // FileName(variable)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated RDP_FILE_RENAME_INFORMATION (1): expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDPDR_PDU_TRUNCATED);
                }
            }

            uint8_t const * const FileName_unicode_data = stream.get_current();

            const size_t FileName_utf8_len = ::UTF16toUTF8(
                FileName_unicode_data,
                this->FileNameLength / 2,
                byte_ptr_cast (this->FileName_),
                sizeof(this->FileName_));

            this->FileName_[FileName_utf8_len] = '\0';


            for (size_t i = 0; i < FileName_utf8_len; i++) {
                if ('\\' == this->FileName_[i]) {
                    this->FileName_[i] = '/';
                }
            }

            stream.in_skip_bytes(this->FileNameLength);
        }
    }

    bool replace_if_exists() const { return this->replace_if_exists_; }

    uint8_t RootDirectory() const { return this->RootDirectory_; }

    const char * FileName() const { return this->FileName_; }

    void log(int level) const {
        LOG(level,
            "RDP_FILE_RENAME_INFORMATION: ReplaceIfExists=%s RootDirectory=%u FileName=\"%s\"",
            (this->replace_if_exists_ ? "yes" : "no"),
            unsigned(this->RootDirectory_), this->FileName_);
    }

    void log() const {
        LOG(LOG_INFO, "     File Rename Information:");
        LOG(LOG_INFO, "          * ReplaceIfExists = %d (1 byte)", this->replace_if_exists_);
        LOG(LOG_INFO, "          * RootDirectory   = %02x (1 byte)", this->RootDirectory_);
        LOG(LOG_INFO, "          * FileNameLength  = %zu (4 bytes)", this->FileNameLength);
        LOG(LOG_INFO, "          * VolumeLabel     = \"%s\" (%zu byte(s)", this->FileName_, this->FileNameLength);
    }
};

// [MS-RDPEFS] - 2.2.3.3.10 Server Drive Query Directory Request
//  (DR_DRIVE_QUERY_DIRECTORY_REQ)
// =============================================================

// The server issues a query directory request on a redirected file system
//  device. This request is used to obtain a directory enumeration.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        DeviceIoRequest                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       FsInformationClass                      |
// +---------------+-----------------------------------------------+
// |  InitialQuery |                   PathLength                  |
// +---------------+-----------------------------------------------+
// |      ...      |                    Padding                    |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                        Path (variable)                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4)
//  header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be
//  set to IRP_MJ_DIRECTORY_CONTROL, and the MinorFunction field MUST be set
//  to IRP_MN_QUERY_DIRECTORY.

// FsInformationClass (4 bytes): A 32-bit unsigned integer. The possible
//  values are specified in [MS-FSCC] section 2.4. This field MUST contain
//  one of the following values.

//  +------------------------------+-------------------------------------------+
//  | Value                        | Meaning                                   |
//  +------------------------------+-------------------------------------------+
//  | FileDirectoryInformation     | Basic information about a file or         |
//  | 0x00000001                   | directory. Basic information is defined   |
//  |                              | as the file's name, time stamp, and size, |
//  |                              | or its attributes.                        |
//  +------------------------------+-------------------------------------------+
//  | FileFullDirectoryInformation | Full information about a file or          |
//  | 0x00000002                   | directory. Full information is defined as |
//  |                              | all the basic information, plus extended  |
//  |                              | attribute size.                           |
//  +------------------------------+-------------------------------------------+
//  | FileBothDirectoryInformation | Basic information plus extended attribute |
//  | 0x00000003                   | size and short name about a file or       |
//  |                              | directory.                                |
//  +------------------------------+-------------------------------------------+
//  | FileNamesInformation         | Detailed information on the names of      |
//  | 0x0000000C                   | files in a directory.                     |
//  +------------------------------+-------------------------------------------+

enum {
      FileDirectoryInformation     = 0x00000001
    , FileFullDirectoryInformation = 0x00000002
    , FileBothDirectoryInformation = 0x00000003
    , FileNamesInformation         = 0x0000000C
};

// InitialQuery (1 byte): An 8-bit unsigned integer. If the value is zero,
//  the Path field is not included regardless of the PathLength value. If the
//  value is set to zero, the request is for the next file in the directory
//  specified in a previous Server Drive Query Directory Request. If such a
//  file does not exist, the client MUST complete this request with
//  STATUS_NO_MORE_FILES in the IoStatus field of the Client Drive I/O
//  Response packet.

// PathLength (4 bytes): A 32-bit unsigned integer that specifies the number
//  of bytes in the Path field, including the null-terminator.

// Padding (23 bytes): An array of 23 bytes. This field is unused and can be
//  set to any value. This field MUST be ignored on receipt.

// Path (variable): A variable-length array of Unicode characters that
//  specifies the directory on which this operation will be performed. The
//  Path field MUST be null-terminated.

class ServerDriveQueryDirectoryRequest {
    uint32_t FsInformationClass_ = 0;
    uint8_t  InitialQuery_       = 0;

    size_t PathLength = 0;

    char Path_[65536];


public:
    explicit ServerDriveQueryDirectoryRequest(uint32_t FsInformationClass,
                                              uint8_t  InitialQuery)
      : FsInformationClass_(FsInformationClass)
      , InitialQuery_(InitialQuery)
    {
        this->Path_[0] = 0;
    }

    explicit ServerDriveQueryDirectoryRequest() = default;

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FsInformationClass_);
        stream.out_uint8(this->InitialQuery_);

        // The null-terminator is included.
        uint8_t Path_unicode_data[65536];
        size_t size_of_Path_unicode_data = ::UTF8toUTF16(
            {this->Path_, this->PathLength},
            Path_unicode_data, sizeof(Path_unicode_data));

        assert(size_of_Path_unicode_data <= 65534);
        // Writes null terminator.
        Path_unicode_data[size_of_Path_unicode_data    ] =
        Path_unicode_data[size_of_Path_unicode_data + 1] = 0;
        size_of_Path_unicode_data += 2;

        uint8_t * temp_p = Path_unicode_data;
        for (size_t i = 0; i < size_of_Path_unicode_data; i += 2) {
            if (('/' == temp_p[0]) && (0 == temp_p[1])) {
                temp_p[0] = '\\';
            }
            temp_p += 2;
        }

        stream.out_uint32_le(size_of_Path_unicode_data);

        stream.out_clear_bytes(23); // Padding(23)

        stream.out_copy_bytes(Path_unicode_data, size_of_Path_unicode_data);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;  // FsInformationClass(4) + InitialQuery(1) +
                                           //     PathLength(4) + Padding(23)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryDirectoryRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FsInformationClass_ = stream.in_uint32_le();
        this->InitialQuery_       = stream.in_uint8();

        this->PathLength = stream.in_uint32_le();

        stream.in_skip_bytes(23);   // Padding(23)

        if (this->PathLength) {
            {
                const unsigned expected = this->PathLength;   // Path(variable)

                if (!stream.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "Truncated ServerDriveQueryDirectoryRequest (1): "
                            "expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_RDPDR_PDU_TRUNCATED);
                }
            }

            uint8_t const * const Path_unicode_data = stream.get_current();

            const size_t path_utf8_len = ::UTF16toUTF8(Path_unicode_data,
                this->PathLength / 2,
                byte_ptr_cast(this->Path_),
                sizeof(this->Path_));

            this->Path_[path_utf8_len] = '\0';

            for (size_t i = 0; i < this->PathLength/2; i++) {
                if ('\\' == this->Path_[i]) {
                    this->Path_[i] = '/';
                }
            }

            stream.in_skip_bytes(this->PathLength);
        }
    }

    uint32_t FsInformationClass() const { return this->FsInformationClass_; }

    uint8_t  InitialQuery() const { return this->InitialQuery_; }

    const char * Path() const { return this->Path_; }

    static const char * get_FsInformationClass_name(uint32_t FsInformationClass) {
        switch (FsInformationClass) {
            case FileDirectoryInformation:     return "FileDirectoryInformation";
            case FileFullDirectoryInformation: return "FileFullDirectoryInformation";
            case FileBothDirectoryInformation: return "FileBothDirectoryInformation";
            case FileNamesInformation:         return "FileNamesInformation";
        }

        return "<unknown>";
    }

    void log(int level) const {
        LOG(level, "ServerDriveQueryDirectoryRequest: FsInformationClass=%s(0x%X) "
                "InitialQuery=%u Path=\"%s\"",
            this->get_FsInformationClass_name(this->FsInformationClass_),
            this->FsInformationClass_, unsigned(this->InitialQuery_), this->Path_);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Query Directory Request:");
        LOG(LOG_INFO, "          * FsInformationClass = 0x%08x (4 bytes): %s", this->FsInformationClass_, this->get_FsInformationClass_name(this->FsInformationClass_));
        LOG(LOG_INFO, "          * InitialQuery       = 0x%02x (1 byte)", this->InitialQuery_);
        LOG(LOG_INFO, "          * PathLength         = %zu (4 bytes)", this->PathLength);
        LOG(LOG_INFO, "          * Padding - (23 byte) NOT USED");
        LOG(LOG_INFO, "          * path               = \"%s\" (%zu byte(s))", this->Path_, this->PathLength);
    }
};  // ServerDriveQueryDirectoryRequest



// 2.2.3.4.10 Client Drive Query Directory Response (DR_DRIVE_QUERY_DIRECTORY_RSP)

// This message is sent by the client as a response to the Server Drive Query Directory Request (section 2.2.3.3.10).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    DeviceIoReply (16 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Length                             |
// +---------------------------------------------------------------+
// |                       Buffer (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------+-----------------------------------------------+
// |Padding(option)|                                               |
// +---------------+-----------------------------------------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION (section 2.2.1.5) header.
// The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST match a Device I/O Request
// (section 2.2.1.4) that has the MajorFunction field set to IRP_MJ_DIRECTORY_CONTROL
// and the MinorFunction field set to IRP_MN_QUERY_DIRECTORY.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of bytes in the Buffer field.

// Buffer (variable): A variable-length array of bytes, in which the number of bytes is specified
// in the Length field. The content of this field is based on the value of the FsInformationClass
// field in the Server Drive Query Directory Request message, which determines the different structures
// that MUST be contained in the Buffer field. For a complete list of these structures, refer
// to [MS-FSCC] section 2.4. The "File information class" table defines all the possible values
// for the FsInformationClass field.

// Padding (1 byte):  An optional, 8-bit unsigned integer intended to allow the client minor
// flexibility in determining the overall packet length. This field is unused and MUST be ignored.

struct ClientDriveQueryDirectoryResponse {

    uint32_t Length = 0;

    explicit ClientDriveQueryDirectoryResponse() = default;

    explicit ClientDriveQueryDirectoryResponse( uint32_t Length)
      : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveQueryDirectoryResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Query Directory Response:");
        LOG(LOG_INFO, "          * Length = %u (4 bytes)", this->Length);
    }
};



// [MS-RDPEFS] - 2.2.3.4.4 Client Drive Write Response (DR_DRIVE_WRITE_RSP)
// ========================================================================

// This message is sent by the client as a response to the Server Drive Write
//  Request (section 2.2.3.3.4).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 DeviceWriteResponse (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceWriteResponse (variable): Returns the result of DR_DRIVE_WRITE_REQ;
//  it is the same as the common Device Write Response (section 2.2.1.5.4).
//  If successful (that is, if the IoStatus field is equal to
//  STATUS_SUCCESS), then the number of bytes written is specified by the
//  Length field of the Server Drive Write Request (section 2.2.3.3.4)
//  message.

// [MS-RDPEFS] - 2.2.3.4.8 Client Drive Query Information Response
//  (DR_DRIVE_QUERY_INFORMATION_RSP)
// ===============================================================

// This message is sent by the client as a response to the Server Drive Query
//  Information Request (section 2.2.3.3.8).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                       Buffer (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION (section 2.2.1.5)
//  header. The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST
//  match a Device I/O Request (section 2.2.1.4) that has the MajorFunction
//  field set to IRP_MJ_QUERY_INFORMATION.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of
//  bytes in the Buffer field.

// Buffer (variable): A variable-length array of bytes, in which the number
//  of bytes is specified in the Length field. The content of this field is
//  based on the value of the FsInformationClass field in the Server Drive
//  Query Information Request message, which determines the different
//  structures that MUST be contained in the Buffer field. For a complete
//  list of these structures, refer to [MS-FSCC] section 2.4. The "File
//  information class" table defines all the possible values for the
//  FsInformationClass field.

enum : uint32_t {
    FILE_STANDARD_INFORMATION_SIZE = 22,
    FILE_BASIC_INFORMATION_SIZE    = 36
};

struct ClientDriveQueryInformationResponse {

    uint32_t Length = 0;

    explicit ClientDriveQueryInformationResponse() = default;

    explicit ClientDriveQueryInformationResponse(uint32_t Length)
      : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveQueryInformationResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Query Information Response:");
        LOG(LOG_INFO, "          * Length = %d (4 bytes)", int(this->Length));
    }
};



// 2.2.3.4.7 Client Drive Set Volume Information Response (DR_DRIVE_SET_VOLUME_INFORMATION_RSP)

// This message is sent by the client as a response to the Server Drive Set Volume Information Request (section 2.2.3.3.7).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     DeviceIoReply (16 bytes)                  |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Length                             |
// +---------------------------------------------------------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION (section 2.2.1.5) header. The
// CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST match a Device I/O Request
// (section 2.2.1.4) that has the MajorFunction field set to IRP_MJ_SET_VOLUME_INFORMATION.

// Length (4 bytes): A 32-bit unsigned integer. It MUST match the Length field in the Server
// Drive Set Volume Information Request.

struct ClientDriveSetVolumeInformationResponse {

    uint32_t Length = 0;

    explicit ClientDriveSetVolumeInformationResponse() = default;

    explicit ClientDriveSetVolumeInformationResponse(uint32_t Length): Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveSetVolumeInformationResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Set Volume Information Response:");
        LOG(LOG_INFO, "          * Length = %d (4 bytes)", int(Length));
    }
};



// 2.2.3.3.7 Server Drive Set Volume Information Request (DR_DRIVE_SET_VOLUME_INFORMATION_REQ)

//  The server issues a set volume information request on a redirected file system device.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       FsInformationClass                      |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                       Padding (24 bytes)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   SetVolumeBuffer (variable)                  |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4) header. The
// MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be set to IRP_MJ_SET_VOLUME_INFORMATION.

// FsInformationClass (4 bytes): A 32-bit unsigned integer. The possible values for this
// field are defined in [MS-FSCC] section 2.5. This field MUST contain the following value.

//  +------------------------------+-------------------------------------------+
//  | Value                        | Meaning                                   |
//  +------------------------------+-------------------------------------------+
//  | FileFsLabelInformation       | Used to set the label for a file system   |
//  | 0x00000002                   | volume.                                   |
//  +------------------------------+-------------------------------------------+

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of bytes in the
// SetVolumeBuffer field.

// Padding (24 bytes): An array of 24 bytes. This field is unused and MUST be ignored.

// SetVolumeBuffer (variable): A variable-length array of bytes. The size of the array is
// specified by the Length field. The content of this field is based on the value of the
// FsInformationClass field, which determines the different structures that MUST be contained
// in the SetVolumeBuffer field. For a complete list of these structures, refer to [MS-FSCC]
// section 2.5. The "File system information class" table defines all the possible values
// for the FsInformationClass field.

struct ServerDriveSetVolumeInformationRequest {

    uint32_t FsInformationClass = 0;
    uint32_t Length = 0;

    explicit ServerDriveSetVolumeInformationRequest() = default;

    explicit ServerDriveSetVolumeInformationRequest(
        uint32_t FsInformationClass , uint32_t Length)
    : FsInformationClass(FsInformationClass)
    , Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->FsInformationClass);
        stream.out_uint32_le(this->Length);
        stream.out_clear_bytes(24);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveSetVolumeInformationResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->FsInformationClass = stream.in_uint32_le();
        this->Length = stream.in_uint32_le();
        stream.in_skip_bytes(24);
    }

    void log() const {
        LOG(LOG_INFO, "     Serve rDrive Set Volume InformationRequest:");
        LOG(LOG_INFO, "          * FsInformationClass = 0x%08x (4 bytes)", this->FsInformationClass);
        LOG(LOG_INFO, "          * Length             = %d (4 bytes)", int(this->Length));
        LOG(LOG_INFO, "          * Padding - (24 bytes) NOT USED");
    }
};

// [MS-RDPEFS] - 2.2.3.4.9 Client Drive Set Information Response
//  (DR_DRIVE_SET_INFORMATION_RSP)
// =============================================================

// This message is sent by the client as a response to the Server Drive Set
//  Information Request (section 2.2.3.3.9).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         DeviceIoReply                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Length                            |
// +---------------+-----------------------------------------------+
// |    Padding    |
// |   (optional)  |
// +---------------+

// DeviceIoReply (16 bytes): A DR_DEVICE_IOCOMPLETION (section 2.2.1.5)
//  header. The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST
//  match a Device I/O Request (section 2.2.1.4) that has the MajorFunction
//  field set to IRP_MJ_SET_INFORMATION.

// Length (4 bytes): A 32-bit unsigned integer. This field MUST be equal to
//  the Length field in the Server Drive Set Information Request (section
//  2.2.3.3.9).

// Padding (1 byte): An optional, 8-bit unsigned integer that is intended to
//  allow the client minor flexibility in determining the overall packet
//  length. This field is unused, and can be set to any value. If present,
//  this field MUST be ignored on receipt.

struct ClientDriveSetInformationResponse {

    uint32_t Length = 0;

    explicit ClientDriveSetInformationResponse() = default;

    explicit ClientDriveSetInformationResponse( uint32_t Length)
      :  Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
        stream.out_clear_bytes(1);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveSetInformationResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Set Information Response:");
        LOG(LOG_INFO, "          * Length = %d (4 bytes)", int(this->Length));
    }
};



// 2.2.3.3.12 Server Drive Lock Control Request (DR_DRIVE_LOCK_REQ)

// The server issues a request to lock or unlock portions of a file.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    DeviceIoRequest (24 bytes)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                           Operation                           |
// +-+-------------------------------------------------------------+
// |F|                         Padding                             |
// +-+-------------------------------------------------------------+
// |                           NumLocks                            |
// +---------------------------------------------------------------+
// |                      Padding2 (20 bytes)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                        Locks (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4) header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be set to IRP_MJ_LOCK_CONTROL.

// Operation (4 bytes): A 32-bit unsigned integer that specifies the type of the locking operation. It MUST have one of the following values:

//  +------------------------------+-------------------------------------------+
//  | Value                        | Meaning                                   |
//  +------------------------------+-------------------------------------------+
//  | RDP_LOWIO_OP_SHAREDLOCK      | The server is requesting a shared lock.   |
//  | 0x00000002                   |                                           |
//  +------------------------------+-------------------------------------------+
//  | RDP_LOWIO_OP_EXCLUSIVELOCK   | The server is requesting an exclusive     |
//  | 0x00000003                   | lock.                                     |
//  +------------------------------+-------------------------------------------+
//  | RDP_LOWIO_OP_UNLOCK          | The server is requesting to unlock a      |
//  | 0x00000004                   | portion of the file.                      |
//  +------------------------------+-------------------------------------------+
//  | RDP_LOWIO_OP_UNLOCK_MULTIPLE | The server is requesting to unlock .      |
//  | 0x00000005                   | multiple portions of the file             |
//  +------------------------------+-------------------------------------------+

//     If this field has any other value, the request MUST be failed immediately.

// F (1 bit): If this bit is set, the client MUST wait for the locking operation to complete. If this bit is not set and the region cannot be locked, the request SHOULD fail.

// Padding (31 bits): 31 bits of padding. This field is unused and MUST be ignored.

// NumLocks (4 bytes): A 32-bit unsigned integer that specifies the number of RDP_LOCK_INFO structures in the Locks array.

// Padding2 (20 bytes): An array of 20 bytes. Reserved. This field can be set to any value and MUST be ignored.

// Locks (variable): A variable-length array of RDP_LOCK_INFO structures. This field specifies one or more regions of the file to lock or unlock.

struct ServerDriveLockControlRequest {

    uint32_t Operation = 0;
    uint8_t F = 0;
    uint32_t NumLocks = 0;


    explicit ServerDriveLockControlRequest() = default;

    explicit ServerDriveLockControlRequest( uint32_t Operation
                                          , uint8_t F
                                          , uint32_t NumLocks)
      : Operation(Operation)
      , F(F)
      , NumLocks(NumLocks)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Operation);
        stream.out_uint8(this->F << 7);
        stream.out_clear_bytes(3);
        stream.out_uint32_le(this->NumLocks);
        stream.out_clear_bytes(20);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 32;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveLockControlRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Operation = stream.in_uint32_le();
        this->F = stream.in_uint8() >> 7;
        stream.in_skip_bytes(3);
        this->NumLocks = stream.in_uint32_le();
        stream.in_skip_bytes(20);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Lock Control Request:");
        LOG(LOG_INFO, "          * Operation = 0x%08x (4 bytes)", this->Operation);
        LOG(LOG_INFO, "          * F         = 0x%01x (1 bit)", this->F);
        LOG(LOG_INFO, "          * Padding - (7 bits and 3 bytes) NOT USED");
        LOG(LOG_INFO, "          * NumLocks  = %u (4 bytes)", this->Operation);
        LOG(LOG_INFO, "          * Padding - (20 byte) NOT USED");
    }
};



// 2.2.3.4.12 Client Drive Lock Control Response (DR_DRIVE_LOCK_RSP)
//
// This message is sent by the client as a response to the Server Drive Lock Control Request (section 2.2.3.3.12).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    DeviceIoReply (16 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Padding                            |
// +---------------+-----------------------------------------------+
// |      ...      |                                               |
// +---------------+-----------------------------------------------+

// DeviceIoReply (16 bytes):  A DR_DEVICE_IOCOMPLETION (section 2.2.1.5) header. The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST match a Device I/O Request (section 2.2.1.4) that has the MajorFunction field set to IRP_MJ_LOCK_CONTROL.
//
// Padding (5 bytes): 5 bytes of padding. This field is unused and MUST be ignored.

struct ClientDriveLockControlResponse {

    explicit ClientDriveLockControlResponse() = default;

    void emit(OutStream & stream) const {
        stream.out_clear_bytes(5);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 5;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveLockControlResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        stream.in_skip_bytes(5);
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Lock Control Response:");
        LOG(LOG_INFO, "          * Padding - (5 bytes) NOT USED");

    }
};



// [MS-RDPEFS]: Remote Desktop Protocol: File System Virtual Channel Extension

// 2.2.1.6 RDP_LOCK_INFO

// The RDP_LOCK_INFO packet specifies the region of the file to lock or unlock.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Length                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                             Offset                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Length (8 bytes): A 64-bit unsigned integer that specifies the length of the region. A value of zero is valid and MUST result in locking the zero length region.

// Offset (8 bytes): A 64-bit unsigned integer that specifies the offset at which the region starts.

struct RDP_Lock_Info {

    uint64_t Length = 0;
    uint64_t Offset = 0;

    explicit RDP_Lock_Info() = default;

    explicit RDP_Lock_Info( uint64_t Length , uint64_t Offset)
      : Length(Length)
      , Offset(Offset)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint64_le(this->Length);
        stream.out_uint64_le(this->Offset);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 16;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated RDP_Lock_Info (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint64_le();
        this->Offset = stream.in_uint64_le();
    }

    void log() const {
        LOG(LOG_INFO, "     RDP_Lock_Info:");
        LOG(LOG_INFO, "          * Length = 0x%" PRIx64 " (8 bytes)", this->Length);
        LOG(LOG_INFO, "          * Offset = 0x%" PRIx64 " (8 bytes)", this->Offset);
    }
};



// 2.2.3.3.11 Server Drive NotifyChange Directory Request (DR_DRIVE_NOTIFY_CHANGE_DIRECTORY_REQ)
//
// The server issues a notify change directory request on a redirected file system device to request directory change notification.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                  DeviceIoRequest (24 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------+-----------------------------------------------+
// |  WatchTree    |              CompletionFilter                 |
// +---------------+-----------------------------------------------+
// |               |            Padding (27 bytes)                 |
// +---------------+-----------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// DeviceIoRequest (24 bytes): A DR_DEVICE_IOREQUEST (section 2.2.1.4) header. The MajorFunction field in the DR_DEVICE_IOREQUEST header MUST be set to IRP_MJ_DIRECTORY_CONTROL, and the MinorFunction field MUST be set to IRP_MN_NOTIFY_CHANGE_DIRECTORY.
//
// WatchTree (1 byte): An 8-bit unsigned integer. If nonzero, a change anywhere within the tree MUST trigger the notification response; otherwise, only a change in the root directory will do so.
//
// CompletionFilter (4 bytes): A 32-bit unsigned integer. This field has the same meaning as the CompletionFilter field in the SMB2 CHANGE_NOTIFY Request message specified in [MS-SMB2] section 2.2.35.
//
// Padding (27 bytes):  An array of 27 bytes. This field is unused and MUST be ignored.

struct ServerDriveNotifyChangeDirectoryRequest {

    uint8_t WatchTree = 0;
    uint32_t CompletionFilter = 0;

    explicit ServerDriveNotifyChangeDirectoryRequest() = default;

    explicit ServerDriveNotifyChangeDirectoryRequest(uint8_t WatchTree, uint32_t CompletionFilter)
      : WatchTree(WatchTree)
      , CompletionFilter(CompletionFilter)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint8(this->WatchTree);
        stream.out_uint32_le(this->CompletionFilter);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 5;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveNotifyChangeDirectoryRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->WatchTree = stream.in_uint8();
        this->CompletionFilter = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Server Drive Notify Change Directory Request:");
        LOG(LOG_INFO, "          * WatchTree        = 0x%02x (1 byte)", this->WatchTree);
        LOG(LOG_INFO, "          * CompletionFilter = 0x%08x (4 bytes)", this->CompletionFilter);
    }
};

// 2.2.35 SMB2 CHANGE_NOTIFY Request

//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_FILE_NAME    | The client is notified if a file-name   |
//  | 0x00000001                      | changes.                                |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_DIR_NAME     | The client is notified if a directory   |
//  | 0x00000002                      | name changes.                           |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_ATTRIBUTES   | The client is notified if a file's      |
//  | 0x00000004                      | attributes change. Possible file        |
//  |                                 | attribute values are specified in       |
//  |                                 | [MS-FSCC] section 2.6.                  |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_SIZE         | The client is notified if a file's size |
//  | 0x00000008                      | changes.                                |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_LAST_WRITE   | The client is notified if the last      |
//  | 0x00000010                      | write time of a file changes.           |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_LAST_ACCESS  | The client is notified if the last      |
//  | 0x00000020                      | access time of a file changes.          |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_CREATION     | The client is notified if the creation  |
//  | 0x00000040                      | time of a file changes.                 |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_EA           | The client is notified if a file's      |
//  | 0x00000080                      | extended attributes (EAs) change.       |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_SECURITY     | The client is notified of a file's      |
//  | 0x00000100                      | access control list (ACL) settings      |
//  |                                 | change.                                 |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_STREAM_NAME  | The client is notified if a named       |
//  | 0x00000200                      | stream is added to a file.              |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_STREAM_SIZE  | The client is notified if the size of a |
//  | 0x00000400                      | named stream is changed.                |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+
//  | FILE_NOTIFY_CHANGE_STREAM_WRITE | The client is notified if a named       |
//  | 0x00000800                      | stream is modified.                     |
//  |                                 |                                         |
//  |                                 |                                         |
//  +---------------------------------+-----------------------------------------+

enum : uint32_t {

    FILE_NOTIFY_CHANGE_FILE_NAME    = 0x00000001,
    FILE_NOTIFY_CHANGE_DIR_NAME     = 0x00000002,
    FILE_NOTIFY_CHANGE_ATTRIBUTES   = 0x00000004,
    FILE_NOTIFY_CHANGE_SIZE         = 0x00000008,
    FILE_NOTIFY_CHANGE_LAST_WRITE   = 0x00000010,
    FILE_NOTIFY_CHANGE_LAST_ACCESS  = 0x00000020,
    FILE_NOTIFY_CHANGE_CREATION     = 0x00000040,
    FILE_NOTIFY_CHANGE_EA           = 0x00000080,
    FILE_NOTIFY_CHANGE_SECURITY     = 0x00000100,
    FILE_NOTIFY_CHANGE_STREAM_NAME  = 0x00000200,
    FILE_NOTIFY_CHANGE_STREAM_SIZE  = 0x00000400,
    FILE_NOTIFY_CHANGE_STREAM_WRITE = 0x00000800,
};



// 2.2.3.4.11 Client Drive NotifyChange Directory Response (DR_DRIVE_NOTIFY_CHANGE_DIRECTORY_RSP)

// This message is sent by the client as a response to the Server Drive NotifyChange Directory Request (section 2.2.3.3.11).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    DeviceIoReply (16 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Length                             |
// +---------------------------------------------------------------+
// |                       Buffer (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------+-----------------------------------------------+
// |Padding(option)|                                               |
// +---------------+-----------------------------------------------+

// DeviceIoReply (16 bytes):  A DR_DEVICE_IOCOMPLETION (section 2.2.1.5) header.
// The CompletionId field of the DR_DEVICE_IOCOMPLETION header MUST match a
// Device I/O Request (section 2.2.1.4) that has the MajorFunction field set
// to IRP_MJ_DIRECTORY_CONTROL and the MinorFunction field set to
// IRP_MN_NOTIFY_CHANGE_DIRECTORY.

// Length (4 bytes): A 32-bit unsigned integer that specifies the number of bytes
// in the Buffer field.

// Buffer (variable): A variable-length array of bytes, in which the number of bytes
// is specified in the Length field. This field has the same meaning as the Buffer
// field in the SMB2 CHANGE_NOTIFY Response message specified in [MS-SMB2] section
// 2.2.36. This buffer MUST be empty when the Server Close Drive Request
// (section 2.2.3.3.2) message has been issued and no drive-specific events have occurred.

// Padding (1 byte): An optional, 8-bit unsigned integer intended to allow the client
// minor flexibility in determining the overall packet length. This field is unused and
// MUST be ignored.

struct ClientDriveNotifyChangeDirectoryResponse {

    uint32_t Length = 0;

    explicit ClientDriveNotifyChangeDirectoryResponse() = default;

    explicit ClientDriveNotifyChangeDirectoryResponse(uint32_t Length)
      : Length(Length)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientDriveNotifyChangeDirectoryResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->Length = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     Client Drive Notify Change Directory Response:");
        LOG(LOG_INFO, "          * Length = %d (4 bytes)", int(this->Length));
    }
};



// 2.2.2.7 Server Core Capability Request (DR_CORE_CAPABILITY_REQ)
//
//  The server announces its capabilities and requests the same from the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +-------------------------------+-------------------------------+
// |     numCapabilities           |            Padding            |
// +-------------------------------+-------------------------------+
// |                  CapabilityMessage (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set to RDPDR_CTYP_CORE, and the PacketId field MUST be set to PAKID_CORE_SERVER_CAPABILITY.
//
// numCapabilities (2 bytes):  A 16-bit integer that specifies the number of items in the CapabilityMessage array.
//
// Padding (2 bytes): A 16-bit unsigned integer of padding. This field is unused and MUST be ignored.
//
// CapabilityMessage (variable): An array of CAPABILITY_SET structures (section 2.2.1.2.1). The number of capabilities is specified by the numCapabilities field

struct ServerCoreCapabilityRequest {

    uint16_t numCapabilities = 0;

    explicit ServerCoreCapabilityRequest() = default;

    explicit ServerCoreCapabilityRequest(uint16_t numCapabilities)
      : numCapabilities(numCapabilities)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->numCapabilities);
        stream.out_skip_bytes(2);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerCoreCapabilityRequest (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->numCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(2);
    }

    void log() const {
        LOG(LOG_INFO, "     Server Core Capability Request:");
        LOG(LOG_INFO, "          * numCapabilities = %u (2 bytes)", numCapabilities);
        LOG(LOG_INFO, "          * Padding - (2 bytes) NOT USED");
    }
};



// 2.2.2.7 Server Core Capability Request (DR_CORE_CAPABILITY_REQ)
//
//  The server announces its capabilities and requests the same from the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +-------------------------------+-------------------------------+
// |     numCapabilities           |            Padding            |
// +-------------------------------+-------------------------------+
// |                  CapabilityMessage (variable)                 |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Header (4 bytes): An RDPDR_HEADER header. The Component field MUST be set to RDPDR_CTYP_CORE, and the PacketId field MUST be set to PAKID_CORE_SERVER_CAPABILITY.
//
// numCapabilities (2 bytes):  A 16-bit integer that specifies the number of items in the CapabilityMessage array.
//
// Padding (2 bytes): A 16-bit unsigned integer of padding. This field is unused and MUST be ignored.
//
// CapabilityMessage (variable): An array of CAPABILITY_SET structures (section 2.2.1.2.1). The number of capabilities is specified by the numCapabilities field.

struct ClientCoreCapabilityResponse {

    uint16_t numCapabilities = 0;

    explicit ClientCoreCapabilityResponse() = default;

    explicit ClientCoreCapabilityResponse(uint16_t numCapabilities)
      : numCapabilities(numCapabilities)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->numCapabilities);
        stream.out_skip_bytes(2);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ClientCoreCapabilityResponse (0): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }
        this->numCapabilities = stream.in_uint16_le();
        stream.in_skip_bytes(2);
    }

    void log() const {
        LOG(LOG_INFO, "     Client Core Capability Response:");
        LOG(LOG_INFO, "          * numCapabilities = %u (2 bytes)", numCapabilities);
        LOG(LOG_INFO, "          * Padding - (2 bytes) NOT USED");
    }
};



struct RdpDrStatus
{
    struct DeviceIORequestData {
        DeviceIORequest request;
        uint32_t FsInformationClass = -1;
        uint32_t IOControlCode = -1;

        explicit DeviceIORequestData(DeviceIORequest const& request)
          : request(request)
        {}

        explicit DeviceIORequestData() = default;

        uint32_t DeviceId() {
            return this->request.DeviceId();
        }

        uint32_t CompletionId() {
            return this->request.CompletionId();
        }

        uint32_t MajorFunction() {
            return this->request.MajorFunction();
        }

        uint32_t MinorFunction() {
            return this->request.MinorFunction();
        }

        uint32_t FileId() {
            return this->request.FileId();
        }
    };

    std::vector<DeviceIORequestData> requestList;


    struct DevideAnnounceRequestMem {
        int in_wait_data = 0;
    };

    void SetFsInformationClass(uint32_t FsInformationClass) {
        this->requestList[this->requestList.size()-1].FsInformationClass = FsInformationClass;
    }

    void SetIOControlCode(uint32_t IOControlCode) {
        this->requestList[this->requestList.size()-1].IOControlCode = IOControlCode;
    }

    void setDeviceIORequest(DeviceIORequest & request) {
        for (auto & request_data : this->requestList) {
            if (request_data.DeviceId() == request.DeviceId() && request_data.CompletionId() == request.CompletionId()) {
                LOG(LOG_ERR, " Request %s has same ID than back received Request(%s)", get_MajorFunction_name(request.MajorFunction()), get_MajorFunction_name(request_data.MajorFunction()) );
                request_data = DeviceIORequestData(request);
                return;
            }
        }
        if (this->requestList.size() >=  10) {
            this->requestList.erase(this->requestList.begin());
        }
        this->requestList.emplace_back(request);
    }

    DeviceIORequestData get_completion_resquest(uint32_t deviceId, uint32_t completionId) {
        DeviceIORequestData res;

//         LOG(LOG_INFO, "**********************************");
//         LOG(LOG_INFO, "     Request List Size = %d", int(this->requestList.size()));
//         LOG(LOG_INFO, "**********************************");
//         for (size_t i = 0; i < this->requestList.size(); i++) {
//             this->requestList[i].request.log();
//         }
//         LOG(LOG_INFO, "**********************************");

        for (size_t i = 0; i < this->requestList.size(); i++) {
            if (this->requestList[i].DeviceId() == deviceId && this->requestList[i].CompletionId() == completionId) {
                res = this->requestList[i];
                this->requestList.erase(this->requestList.begin()+i);

                return res;
            }
        }

        LOG(LOG_INFO, "Can't find corresponding Request for this Response (DeviceId = %d, CompletionId = %d", int(deviceId), int(completionId));

        return res;
    }



    ~RdpDrStatus() {
        this->requestList.clear();
    }

};



static inline
void streamLog(InStream & stream , RdpDrStatus & status)
{
    InStream s = stream.clone();

    SharedHeader sharedHeader;
    sharedHeader.receive(s);
    if (sharedHeader.packet_id != PacketId::PAKID_CORE_DEVICE_IOCOMPLETION) {
        sharedHeader.log();
    }

    switch (sharedHeader.component) {

        case Component::RDPDR_CTYP_CORE:

            switch (sharedHeader.packet_id) {

                case PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                    {
                        ServerAnnounceRequest sar;
                        sar.receive(s);
                        sar.log();
                    }
                    break;

                case PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                    {
                        ClientAnnounceReply car;
                        car.receive(s);
                        car.log();
                    }
                    break;

                case PacketId::PAKID_CORE_CLIENT_NAME:
                    {
                        ClientNameRequest cnr;
                        cnr.receive(s);
                        cnr.log();
                    }
                    break;

                case PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE:
                    {
                        ClientDeviceListAnnounceRequest cdar;
                        cdar.receive(s);
                        cdar.log();
                        for (uint32_t i = 0; i < cdar.DeviceCount; i++) {
                            DeviceAnnounceHeader dah;
                            dah.receive(s);
                            dah.log();
                        }
                    }
                    break;

                case PacketId::PAKID_CORE_DEVICE_REPLY:
                    {
                        ServerDeviceAnnounceResponse sdar;
                        sdar.receive(s);
                        sdar.log();
                    }
                    break;

                case PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                    {
                        DeviceIORequest dior;
                        dior.receive(s);
                        dior.log();

                        status.setDeviceIORequest(dior);

                        switch (dior.MajorFunction()) {
                            case IRP_MJ_CREATE:
                                {
                                    DeviceCreateRequest dcr;
                                    dcr.receive(s);
                                    dcr.log();
                                }
                                break;
                            case IRP_MJ_CLOSE:
                                {
                                    DeviceCloseRequest dcr;
                                    dcr.receive(s);
                                    dcr.log();
                                }
                                break;
                            case IRP_MJ_READ:
                                {
                                    DeviceReadRequest drr;
                                    drr.receive(s);
                                    drr.log();
                                }
                                break;
                            case IRP_MJ_WRITE:
                                {
                                    DeviceWriteRequest dwr;
                                    dwr.receive(s);
                                    dwr.log();
                                }
                                break;
                            case IRP_MJ_DEVICE_CONTROL:
                                {
                                    DeviceControlRequest dcr;
                                    dcr.receive(s);
                                    dcr.log();

                                    status.SetIOControlCode(dcr.IoControlCode());;

                                    switch (dcr.IoControlCode()) {
                                        case fscc::FSCTL_DELETE_REPARSE_POINT :
//                                             {
//                                                 fscc::ReparseGUIDDataBuffer rgdb;
//                                                 rgdb.receive(s);
//                                                 rgdb.log();
//                                             }
                                            break;
                                         case fscc::FSCTL_CREATE_OR_GET_OBJECT_ID :
                                            break;
                                        default: LOG(LOG_INFO, "     Device Controle UnLogged IO Control Data: Code = 0x%08x", dcr.IoControlCode());
                                            break;
                                    }
                                }
                                break;
                            case IRP_MJ_QUERY_VOLUME_INFORMATION:
                                {
                                    ServerDriveQueryVolumeInformationRequest sdqvir;
                                    sdqvir.receive(s);
                                    sdqvir.log();

                                    status.SetFsInformationClass(sdqvir.FsInformationClass());

                                    if (sdqvir.Length() > 0) {
                                        switch (sdqvir.FsInformationClass()) {
                                            case FileFsVolumeInformation:
                                                {
                                                    fscc::FileFsVolumeInformation ffvi;
                                                    ffvi.receive(s);
                                                    ffvi.log();
                                                }
                                                break;
                                            case FileFsSizeInformation:
                                                {
                                                    fscc::FileFsSizeInformation ffsi;
                                                    ffsi.receive(s);
                                                    ffsi.log();
                                                }
                                                break;
                                            case FileFsAttributeInformation: {
                                                    fscc::FileFsAttributeInformation ffai;
                                                    ffai.receive(s);
                                                    ffai.log();
                                                }
                                                break;
                                            case FileFsFullSizeInformation:
                                                {
                                                    fscc::FileFsFullSizeInformation fffsi;
                                                    fffsi.receive(s);
                                                    fffsi.log();
                                                }
                                                break;
                                            case FileFsDeviceInformation:
                                                {
                                                    fscc::FileFsDeviceInformation ffdi;
                                                    ffdi.receive(s);
                                                    ffdi.log();
                                                }
                                                break;
                                        }
                                    }
                                }
                                break;
                            case IRP_MJ_SET_VOLUME_INFORMATION:
                                {
                                    ServerDriveSetVolumeInformationRequest sdsvir;
                                    sdsvir.receive(s);
                                    sdsvir.log();

                                    status.SetFsInformationClass(sdsvir.FsInformationClass);

                                    fscc::FileFsLabelInformation ffli;
                                    ffli.receive(s);
                                    ffli.log();
                                }
                                break;
                            case IRP_MJ_QUERY_INFORMATION:
                                {
                                    ServerDriveQueryInformationRequest sdqir;
                                    sdqir.receive(s);
                                    sdqir.log();

                                    status.SetFsInformationClass(sdqir.FsInformationClass());

                                    if (sdqir.Length() > 0) {
                                        switch (sdqir.FsInformationClass()) {
                                            case FileBasicInformation:
                                                {
                                                fscc::FileBasicInformation fbi;
                                                fbi.receive(s);
                                                fbi.log();
                                                }
                                                break;
                                            case FileStandardInformation:
                                                {
                                                fscc::FileStandardInformation fsi;
                                                fsi.receive(s);
                                                fsi.log();
                                                }
                                                break;
                                            case FileAttributeTagInformation:
                                                {
                                                fscc::FileAttributeTagInformation fati;
                                                fati.receive(s);
                                                fati.log();
                                                }
                                                break;
                                        }
                                    }
                                }
                                break;
                            case IRP_MJ_SET_INFORMATION:
                                {
                                    ServerDriveSetInformationRequest sdsir;
                                    sdsir.receive(s);
                                    sdsir.log();

                                    status.SetFsInformationClass(sdsir.FsInformationClass());

                                    if (sdsir.Length() > 0) {
                                        switch (sdsir.FsInformationClass()) {
                                            case FileBasicInformation:
                                                {
                                                    fscc::FileBasicInformation fbi;
                                                    fbi.receive(s);
                                                    fbi.log();
                                                }
                                                break;
                                            case FileEndOfFileInformation:
                                                {
                                                    fscc::FileEndOfFileInformation feofi;
                                                    feofi.receive(s);
                                                    feofi.log();
                                                }
                                                break;
                                            case FileDispositionInformation:
                                                {
                                                    fscc::FileDispositionInformation fdi;
                                                    fdi.receive(s);
                                                    fdi.log();
                                                }
                                                break;
                                            case FileRenameInformation:
                                                {
                                                    RDPFileRenameInformation fri;
                                                    fri.receive(s);
                                                    fri.log();
                                                }
                                                break;
                                            case FileAllocationInformation:
                                                {
                                                    fscc::FileAllocationInformation fai;
                                                    fai.receive(s);
                                                    fai.log();
                                                }
                                                break;
                                        }
                                    }
                                }
                                break;
                            case IRP_MJ_DIRECTORY_CONTROL:
                                {
                                    switch (dior.MinorFunction()) {

                                        case IRP_MN_QUERY_DIRECTORY:
                                            {
                                                ServerDriveQueryDirectoryRequest sdqdr;
                                                sdqdr.receive(s);
                                                sdqdr.log();

                                                status.SetFsInformationClass(sdqdr.FsInformationClass());
                                            }
                                            break;
                                        case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                                            {
                                                ServerDriveNotifyChangeDirectoryRequest sdcdr;
                                                sdcdr.receive(s);
                                                sdcdr.log();
                                            }
                                            break;
                                    }
                                }
                                break;
                            case IRP_MJ_LOCK_CONTROL:
                                {
                                    ServerDriveLockControlRequest sdlcr;
                                    sdlcr.receive(s);
                                    sdlcr.log();

                                    for (uint32_t i = 0; i < sdlcr.NumLocks; i++) {
                                        RDP_Lock_Info rdpli;
                                        rdpli.receive(s);
                                        rdpli.log();
                                    }
                                }
                                break;
                        }
                    }

                    break;

                case PacketId::PAKID_CORE_DEVICE_IOCOMPLETION:
                    {
                        DeviceIOResponse dior;
                        dior.receive(s);

                        RdpDrStatus::DeviceIORequestData status_dior = status.get_completion_resquest(dior.DeviceId(), dior.CompletionId());

                        if (status_dior.DeviceId() == 0) {
                            LOG(LOG_INFO, "Can't find corresponding Request for this Response (DeviceId = %u, CompletionId = %u", dior.DeviceId(), dior.CompletionId());

                            return;
                        }

                        sharedHeader.log();
                        dior.log();

                        switch (status_dior.MajorFunction()) {
                        case IRP_MJ_CREATE:
                            {
                                DeviceCreateResponse dcf;
                                dcf.receive(s);
                                dcf.log();
                            }
                            break;
                        case IRP_MJ_CLOSE:
                            {
                                LOG(LOG_INFO, "     Device Close Response:");
                                LOG(LOG_INFO, "          * Padding - (4 bytes) NOT USED");
                            }
                            break;
                        case IRP_MJ_READ:
                            {
                                DeviceReadResponse drr;
                                drr.receive(s);
                                drr.log();
                            }
                            break;
                        case IRP_MJ_WRITE:
                            {
                                DeviceWriteResponse dwr;
                                dwr.receive(s);
                                dwr.log();
                            }
                            break;
                        case IRP_MJ_DEVICE_CONTROL:
                            {
                                DriveControlResponse cdcr;
                                cdcr.receive(s);
                                cdcr.log();

                                //if (cdcr.OutputBufferLength > 0) {
                                    switch (status_dior.IOControlCode) {
                                        case fscc::FSCTL_GET_OBJECT_ID:
                                        case fscc::FSCTL_CREATE_OR_GET_OBJECT_ID:
                                        {
//                                             fscc::FileObjectBuffer_Type1 rgdb;
//                                             rgdb.receive(s);
//                                             rgdb.log();
                                        }
                                            break;
                                        //case fscc::FSCTL_FILESYSTEM_GET_STATISTICS:
                                        //    break;
                                        default: LOG(LOG_INFO, "     Device Controle UnLogged IO Control Code: 0x%08x", status_dior.IOControlCode);
                                            break;
                                    }
                                //}
                            }
                            break;
                        case IRP_MJ_QUERY_VOLUME_INFORMATION:
                            {
                                ClientDriveQueryVolumeInformationResponse cdqvir;
                                cdqvir.receive(s);
                                cdqvir.log();

                                if (cdqvir.Length > 0) {
                                    switch (status_dior.FsInformationClass) {
                                        case FileFsVolumeInformation:
                                            {
                                                fscc::FileFsVolumeInformation ffvi;
                                                ffvi.receive(s);
                                                ffvi.log();
                                            }
                                            break;
                                        case FileFsSizeInformation:
                                            {
                                                fscc::FileFsSizeInformation ffsi;
                                                ffsi.receive(s);
                                                ffsi.log();
                                            }
                                            break;
                                        case FileFsAttributeInformation: {
                                                fscc::FileFsAttributeInformation ffai;
                                                ffai.receive(s);
                                                ffai.log();
                                            }
                                            break;
                                        case FileFsFullSizeInformation:
                                            {
                                                fscc::FileFsFullSizeInformation fffsi;
                                                fffsi.receive(s);
                                                fffsi.log();
                                            }
                                            break;
                                        case FileFsDeviceInformation:
                                            {
                                                fscc::FileFsDeviceInformation ffdi;
                                                ffdi.receive(s);
                                                ffdi.log();
                                            }
                                            break;
                                    }
                                }
                            }
                            break;
                        case IRP_MJ_SET_VOLUME_INFORMATION:
                            {
                                ClientDriveSetVolumeInformationResponse cdsvir;
                                cdsvir.receive(s);
                                cdsvir.log();
                            }
                            break;
                        case IRP_MJ_QUERY_INFORMATION:
                            {
                                rdpdr::ClientDriveQueryInformationResponse cdqir;
                                cdqir.receive(s);
                                cdqir.log();
                                if (cdqir.Length > 0) {
                                    switch (status_dior.FsInformationClass) {
                                        case FileBasicInformation:
                                            {
                                            fscc::FileBasicInformation fbi;
                                            fbi.receive(s);
                                            fbi.log();
                                            }
                                            break;
                                        case FileStandardInformation:
                                            {
                                            fscc::FileStandardInformation fsi;
                                            fsi.receive(s);
                                            fsi.log();
                                            }
                                            break;
                                        case FileAttributeTagInformation:
                                            {
                                            fscc::FileAttributeTagInformation fati;
                                            fati.receive(s);
                                            fati.log();
                                            }
                                            break;
                                    }
                                }
                            }
                            break;
                        case IRP_MJ_SET_INFORMATION:
                            {
                                ClientDriveSetInformationResponse cdsir;
                                cdsir.receive(s);
                                cdsir.log();
                            }
                            break;
                        case IRP_MJ_DIRECTORY_CONTROL:
                            {
                                switch (status_dior.MinorFunction()) {

                                    case IRP_MN_QUERY_DIRECTORY:
                                        {
                                            ClientDriveQueryDirectoryResponse cdqdr;
                                            cdqdr.receive(s);
                                            cdqdr.log();

                                            if (cdqdr.Length > 0) {
                                                switch (status_dior.FsInformationClass) {
                                                    case FileDirectoryInformation:
                                                        {
                                                        fscc::FileDirectoryInformation fdi;
                                                        fdi.receive(s);
                                                        fdi.log();
                                                        }
                                                        break;
                                                    case FileFullDirectoryInformation:
                                                        {
                                                        fscc::FileFullDirectoryInformation ffdi;
                                                        ffdi.receive(s);
                                                        ffdi.log();
                                                        }
                                                        break;
                                                    case FileBothDirectoryInformation:
                                                        {
                                                        fscc::FileBothDirectoryInformation fbdi;
                                                        fbdi.receive(s);
                                                        fbdi.log();
                                                        }
                                                        break;
                                                    case FileNamesInformation:
                                                        {
                                                        fscc::FileNamesInformation fni;
                                                        fni.receive(s);
                                                        fni.log();
                                                        }
                                                        break;
                                                }
                                            }
                                        }
                                        break;
                                    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                                        {
                                            ClientDriveNotifyChangeDirectoryResponse cdncdr;
                                            cdncdr.receive(s);
                                            cdncdr.log();

                                            if (cdncdr.Length >= 8) {
                                                smb2::ChangeNotifyResponse cnr;
                                                cnr.receive(s);
                                                cnr.log();

                                                if (cdncdr.Length >= 16) {
                                                    fscc::FileNotifyInformation fni;
                                                    fni.receive(s);
                                                    fni.log();
                                                }
                                            }


                                        }
                                        break;
                                }
                            }
                            break;
                        case IRP_MJ_LOCK_CONTROL:
                            {
                                LOG(LOG_INFO, "     Client Drive Lock Control Response:");
                                LOG(LOG_INFO, "          * Padding - (5 bytes) NOT USED");
                            }
                            break;
                        default: LOG(LOG_INFO, "     default MajorFunction:");
                            break;
                        }
                    }
                    break;

                case PacketId::PAKID_CORE_SERVER_CAPABILITY:
                    {
                        ServerCoreCapabilityRequest sccr;
                        sccr.receive(s);
                        sccr.log();

                        for (uint16_t i = 0; i < sccr.numCapabilities; i++) {

                            CapabilityHeader ch;
                            ch.receive(s);
                            ch.log();

                            switch (ch.CapabilityType) {
                                case CAP_GENERAL_TYPE:
                                    {
                                        GeneralCapabilitySet gcs;
                                        gcs.receive(s, ch.Version);
                                        gcs.log();
                                    }
                                    break;
                                case CAP_PRINTER_TYPE:
                                case CAP_PORT_TYPE:
                                case CAP_DRIVE_TYPE:
                                case CAP_SMARTCARD_TYPE:
                                    s.in_skip_bytes(ch.CapabilityLength - 8);
                                    break;
                            }
                        }
                    }
                    break;

                case PacketId::PAKID_CORE_CLIENT_CAPABILITY:
                    {
                        ClientCoreCapabilityResponse cccr;
                        cccr.receive(s);
                        cccr.log();

                        for (uint16_t i = 0; i < cccr.numCapabilities; i++) {

                            CapabilityHeader ch;
                            ch.receive(s);
                            ch.log();

                            switch (ch.CapabilityType) {
                                case CAP_GENERAL_TYPE:
                                    {
                                        GeneralCapabilitySet gcs;
                                        gcs.receive(s, ch.Version);
                                        gcs.log();
                                    }
                                    break;
                                case CAP_PRINTER_TYPE:
                                case CAP_PORT_TYPE:
                                case CAP_DRIVE_TYPE:
                                case CAP_SMARTCARD_TYPE:
                                    s.in_skip_bytes(ch.CapabilityLength - 8);
                                    break;
                            }
                        }
                    }
                    break;

                case PacketId::PAKID_CORE_DEVICELIST_REMOVE:
                    {
                        ClientDriveDeviceListRemove cdlr;
                        cdlr.receive(s);
                        cdlr.log();
                    }
                    break;

                case PacketId::PAKID_PRN_CACHE_DATA:
                    break;

                case PacketId::PAKID_CORE_USER_LOGGEDON:
                    break;

                case PacketId::PAKID_PRN_USING_XPS:
                    break;
            }
            break;

        case Component::RDPDR_CTYP_PRT:
            LOG(LOG_WARNING, "Component::RDPDR_CTYP_PRT");
            break;
    }
}

}   // namespace rdpdr

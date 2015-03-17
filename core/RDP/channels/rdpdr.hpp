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

#include <cinttypes>

#include "cast.hpp"
#include "noncopyable.hpp"
#include "stream.hpp"

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

enum class Component : uint16_t {
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

struct SharedHeader {
    Component component = Component::RDPDR_CTYP_CORE;
    PacketId  packet_id = PacketId::PAKID_CORE_SERVER_ANNOUNCE;

    SharedHeader() = default;

    SharedHeader(Component component, PacketId  packet_id)
    : component(component)
    , packet_id(packet_id) {}

    inline void emit(Stream & stream) const {
        stream.out_uint16_le(static_cast<uint16_t>(this->component));
        stream.out_uint16_le(static_cast<uint16_t>(this->packet_id));
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 4;  // Component(2) + PacketId(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated SharedHeader: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->component = static_cast<Component>(stream.in_uint16_le());
        this->packet_id = static_cast<PacketId>(stream.in_uint16_le());
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "SharedHeader: Component=0x%X PacketId=0x%X",
            static_cast<uint16_t>(this->component), static_cast<uint16_t>(this->packet_id));
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};

enum class CapabilityType : uint16_t {
    general     = 1,
    printer     = 2,
    port        = 3,
    drive       = 4,
    smartcard   = 5
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
    DeviceAnnounceHeader() = default;

    DeviceAnnounceHeader(uint32_t DeviceType, uint32_t DeviceId,
                         const char * preferred_dos_name,
                         uint8_t * device_data_p, size_t device_data_size)
    : DeviceType_(DeviceType)
    , DeviceId_(DeviceId)
    , device_data(device_data_p, device_data_size) {
        ::memcpy(this->PreferredDosName, preferred_dos_name,
            std::min<size_t>(::strlen(preferred_dos_name), 8 /*PreferredDosName(8)*/));
    }

    REDEMPTION_NON_COPYABLE(DeviceAnnounceHeader);

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DeviceType_);
        stream.out_uint32_le(this->DeviceId_);

        stream.out_copy_bytes(this->PreferredDosName, 8 /* PreferredDosName(8) */);

        stream.out_uint32_le(this->device_data.get_capacity());

        stream.out_copy_bytes(this->device_data.get_data(), this->device_data.get_capacity());
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 20;  // DeviceType(4) + DeviceId(4) + PreferredDosName(8) + DeviceDataLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceAnnounceHeader (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceType_ = stream.in_uint32_le();
        this->DeviceId_   = stream.in_uint32_le();

        stream.in_copy_bytes(this->PreferredDosName, 8 /* PreferredDosName(8) */);
        this->PreferredDosName[8 /* PreferredDosName(8) */ ] = '\0';

        const uint32_t DeviceDataLength = stream.in_uint32_le();

        {
            const unsigned expected = DeviceDataLength;  // DeviceData(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceAnnounceHeader (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

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
        return ((length < size) ? length : size - 1);
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
};  // DeviceAnnounceHeader

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

class DeviceIORequest {
    uint32_t DeviceId_      = 0;
    uint32_t FileId_        = 0;
    uint32_t CompletionId_   = 0;
    uint32_t MajorFunction_ = 0;
    uint32_t MinorFunction  = 0;

public:
    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DeviceId_);
        stream.out_uint32_le(this->FileId_);
        stream.out_uint32_le(this->CompletionId_);
        stream.out_uint32_le(this->MajorFunction_);
        stream.out_uint32_le(this->MinorFunction);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 20;  // DeviceId(4) + FileId(4) + CompletionId(4) +
                                           //     MajorFunction(4) + MinorFunction(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceIORequest: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DeviceId_      = stream.in_uint32_le();
        this->FileId_        = stream.in_uint32_le();
        this->CompletionId_  = stream.in_uint32_le();
        this->MajorFunction_ = stream.in_uint32_le();
        this->MinorFunction  = stream.in_uint32_le();
    }

    uint32_t DeviceId() const { return this->DeviceId_; }

    uint32_t FileId() const { return this->FileId_; }

    uint32_t CompletionId() const { return this->CompletionId_; }

    uint32_t MajorFunction() const { return this->MajorFunction_; }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "DeviceIORequest: "
                "DeviceId=%u FileId=%u CompletionId=%u MajorFunction=0x%X MinorFunction=0x%X",
            this->DeviceId_, this->FileId_, this->CompletionId_, this->MajorFunction_,
            this->MinorFunction);
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};

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
    uint32_t DesiredAccess;
    uint64_t AllocationSize;
    uint32_t FileAttributes;
    uint32_t SharedAccess;
    uint32_t CreateDisposition_;
    uint32_t CreateOptions_;

    std::string path;

public:
    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DesiredAccess);
        stream.out_uint64_le(this->AllocationSize);
        stream.out_uint32_le(this->FileAttributes);
        stream.out_uint32_le(this->SharedAccess);
        stream.out_uint32_le(this->CreateDisposition_);
        stream.out_uint32_le(this->CreateOptions_);

        const size_t maximum_length_of_Path_in_bytes = this->path.length() * 2;

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_Path_in_bytes));
        const size_t size_of_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->path.c_str()), unicode_data,
            maximum_length_of_Path_in_bytes);

        stream.out_uint32_le(size_of_unicode_data);

        stream.out_copy_bytes(unicode_data, size_of_unicode_data);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 32;  // DesiredAccess(4) + AllocationSize(8) +
                                           //     FileAttributes(4) + SharedAccess(4) +
                                           //     CreateDisposition(4) + CreateOptions(4) +
                                           //     PathLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateRequest (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->DesiredAccess      = stream.in_uint32_le();
        this->AllocationSize     = stream.in_uint64_le();
        this->FileAttributes     = stream.in_uint32_le();
        this->SharedAccess       = stream.in_uint32_le();
        this->CreateDisposition_ = stream.in_uint32_le();
        this->CreateOptions_     = stream.in_uint32_le();

        const uint16_t PathLength = stream.in_uint32_le();

        {
            const unsigned expected = PathLength;   // Path(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateRequest (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(PathLength));

        stream.in_copy_bytes(unicode_data, PathLength);

        const size_t maximum_length_of_utf8_character_in_bytes = 4;

        const size_t size_of_utf8_string =
                    PathLength / 2 * maximum_length_of_utf8_character_in_bytes + 1;
        uint8_t * const utf8_string = static_cast<uint8_t *>(
            ::alloca(size_of_utf8_string));
        const size_t length_of_utf8_string = ::UTF16toUTF8(
            unicode_data, PathLength / 2, utf8_string, size_of_utf8_string);
        this->path.assign(::char_ptr_cast(utf8_string),
            length_of_utf8_string);
    }

    const char * Path() const { return this->path.c_str(); }

    uint32_t CreateDisposition() const { return this->CreateDisposition_; }

    uint32_t CreateOptions() const { return this->CreateOptions_; }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "DeviceCreateRequest: DesiredAccess=0x%X AllocationSize=%" PRIu64 " "
                "FileAttributes=0x%X SharedAccess=0x%X CreateDisposition=0x%X "
                "CreateOptions=0x%X path=\"%s\"",
            this->DesiredAccess, this->AllocationSize, this->FileAttributes,
            this->SharedAccess, this->CreateDisposition_, this->CreateOptions_,
            this->path.c_str());
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
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
    inline void emit(Stream & stream) const {
        stream.out_clear_bytes(32); // Padding(32)
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 32;  // Padding(32)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCloseRequest: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        stream.in_skip_bytes(32);   // Padding(32)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size, "DeviceCloseRequest:");
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
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
    uint32_t DeviceId_     = 0;
    uint32_t CompletionId_ = 0;
    uint32_t IoStatus      = 0;

public:
    DeviceIOResponse() = default;

    DeviceIOResponse(uint32_t DeviceId, uint32_t CompletionId, uint32_t IoStatus)
    : DeviceId_(DeviceId)
    , CompletionId_(CompletionId)
    , IoStatus(IoStatus) {}

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DeviceId_);
        stream.out_uint32_le(this->CompletionId_);
        stream.out_uint32_le(this->IoStatus);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 12;   // DeviceId(4) + CompletionId(4) + IoStatus(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceIOResponse: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->DeviceId_     = stream.in_uint32_le();
        this->CompletionId_ = stream.in_uint32_le();
        this->IoStatus      = stream.in_uint32_le();
    }

    uint32_t DeviceId() const { return this->DeviceId_; }

    uint32_t CompletionId() const { return this->CompletionId_; }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "DeviceIOResponse: DeviceId=%u CompletionId=%u IoStatus=0x%08X",
            this->DeviceId_, this->CompletionId_, this->IoStatus);
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
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

enum {
      FILE_SUPERSEDED  = 0x00000000
    , FILE_OPENED      = 0x00000001
    , FILE_OVERWRITTEN = 0x00000003
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
    uint32_t FileId;
    uint8_t  Information;

public:
    DeviceCreateResponse() = default;

    DeviceCreateResponse(uint32_t FileId, uint8_t Information)
    : FileId(FileId)
    , Information(Information) {}

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->FileId);
        stream.out_uint8(this->Information);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 5;   // FileId(4) + Information(1)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCreateResponse: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->FileId      = stream.in_uint32_le();
        this->Information = stream.in_uint8();
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "DeviceCreateResponse: FileId=%u Information=0x%X",
            this->FileId, this->Information);
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
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

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->DeviceId);
        stream.out_uint32_le(this->ResultCode);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 8;   // DeviceId(4) + ResultCode(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDeviceAnnounceResponse: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->DeviceId   = stream.in_uint32_le();
        this->ResultCode = stream.in_uint32_le();
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "ServerDeviceAnnounceResponse: DeviceId=%u ResultCode=0x%08X",
            this->DeviceId, this->ResultCode);
        return ((length < size) ? length : size - 1);
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

enum {
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
    uint32_t FsInformationClass_ = 0;

    StaticStream query_buffer;

public:
    ServerDriveQueryInformationRequest() = default;

    REDEMPTION_NON_COPYABLE(ServerDriveQueryInformationRequest);

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->FsInformationClass_);

        stream.out_uint32_le(this->query_buffer.get_capacity());

        stream.out_clear_bytes(24); // Padding(24)

        stream.out_copy_bytes(this->query_buffer.get_data(), this->query_buffer.get_capacity());
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 32;  // FsInformationClass(4) + Length(4) + Padding(24)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryInformationRequest (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->FsInformationClass_ = stream.in_uint32_le();

        const uint32_t Length = stream.in_uint32_le();

        stream.in_skip_bytes(24);   // Padding(24)

        {
            const unsigned expected = Length;  // QueryBuffer(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated ServerDriveQueryInformationRequest (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->query_buffer.resize(stream.p, Length);
        stream.in_skip_bytes(Length);
    }

    uint32_t FsInformationClass() const { return this->FsInformationClass_; }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "ServerDriveQueryInformationRequest: FsInformationClass=%u Length=%zu",
            this->FsInformationClass_, this->query_buffer.get_capacity());
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};

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

}   // namespace rdpdr

#endif

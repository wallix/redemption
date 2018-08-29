/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Clément Moroldo

   Unit test to writing RDP orders to file and rereading them
*/


#pragma once

#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <linux/hdreg.h>
#include <unordered_map>

#include "utils/log.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr.hpp"

#include "client_redemption/client_input_output_api/client_iodisk_api.hpp"
#include "client_redemption/client_input_output_api/rdp_disk_config.hpp"


// [MS-RDPEFS]: Rmote Desktop Protocol: File System Virtual Channel Extension
//
//
// 1.3.1 Protocol Initialization
//
// The following figure shows the initial packet sequence that initializes the protocol. The sequence of messages complies with the following set of rules. The first packet exchange, Server Announce Request/Client Announce Reply, simply consists of the client and server sides of the protocol exchanging version information that tells each side to which version it is speaking. The client sends a Client Name Request after sending a Client Announce Reply message. The Client Name Request contains a friendly display name for the client machine.
//
// The next exchange, Server Core Capability Request/Client Core Capability Response, is used to exchange capabilities between the client and the server to ensure that each side records what kinds of packets are supported by the remote side.
//
// After sending its Server Core Capability Request message, the server also sends a Server Client ID Confirm message confirming the client ID that was exchanged in the Server Announce Request/Client Announce Reply sequence.
//
// The last initialization message sequence is initiated by the client with the Client Device List Announce Request. This packet contains information for each device that is redirected. The packet contains all redirected devices, including non–file system devices. For example, it includes the list of printers (as specified in [MS-RDPEPC]), ports (as specified in [MS-RDPESP]), and smart cards (as specified in [MS-RDPESC]). Each client device is initialized separately. The server sends a Server Device Announce Response message that indicates success or failure for that initialization.
//
// +-----------+                                                 +-----------+
// | Server FS |                                                 | TS Client |
// |  Driver   |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       +------------------Server Announce Request------------------> |
//       |                                                             |
//       | <-----------------Client Announce Reply---------------------+
//       |                                                             |
//       | <------------------Client name Request----------------------+
//       |                                                             |
//       +-----------------Server Capability Request-----------------> |
//       |                                                             |
//       +-----------------Server Client ID Confirm------------------> |
//       |                                                             |
//       | <------------Client Core Capability Response----------------+
//       |                                                             |
//       | <---------Client Dev ice List Announce Request--------------+
//       |                                                             |
//       +--------Server Device Announce Response (device #1)--------> |
//       |                                                             |
//       +--------Server Device Announce Response (device #2)--------> |
//       |                                                             |
//
// Figure 1: Protocol initialization
//
// In general, there is no distinguishable difference between the initial connection of the protocol and subsequent reconnections. After every disconnection, the protocol is torn down and completely re-initialized on the next connection. However, there is one difference in the protocol initialization sequence upon reconnection: if a user is already logged on, the server sends a Server User Logged On message according to the rules specified in section 3.3.5.1.5.
//
//
// 1.3.2 Drive Redirection
//
// Drives can be announced or deleted at any point in time after the connection has been established. For example, Drive redirection sequence shows the sequence for adding and removing a file system drive. The first message pair, Client Device List Announce Request/Server Device Announce Response, is optional. If the device has been announced already in the Client Device List Announce as part of the protocol initialization, this pair is not required. But if the device has been discovered on the client after the initial sequence, this pair of messages is used to announce the device to the server. The client announces only one drive at a time in this case.
//
// The next pair of messages describes a series of I/O request messages exchanged between the client and the server. This set of messages describes the actual file system functionality redirection. Finally, the Client Drive Device List Remove message announces to the server that the file system drive has been removed from the client, and that all I/O to that device will fail in the future.
//
// +-----------+                                                 +-----------+
// | Server FS |                                                 | TS Client |
// |  Driver   |                                                 |           |
// +-----+-----+                                                 +-----+-----+
//       |                                                             |
//       |                                                             |
//       | <------------Client Drive Divece List Announce--------------+
//       |                                                             |
//       +---------------Server Device Announce Response-------------> |
//       |                                                             |
//       +------------------Server Drive I/O Request-----------------> |
//       |                                                             |
//       | <---------------Client Device I/O Response------------------+
//       |                                                             |
//       | <-------------Client Drive Device List Remove---------------+
//       |                                                             |
//
// Figure 2: Drive redirection sequence



class ClientChannelRDPDRManager : public ClientChannelManager {

    RDPVerbose verbose;

    ClientIODiskAPI * impl_io_disk;

public:

    bool drives_created = false;
    uint16_t protocol_minor_version = 0;
    uint32_t next_file_id = 0;

    std::unique_ptr<uint8_t[]> ReadData;
    int last_read_data_portion_length = 0;

    uint32_t get_file_id() {
        this->next_file_id++;
        return this->next_file_id;
    }

    std::unordered_map<int, std::string> paths;


    struct DeviceData {
        char name[8] = {0};
        uint32_t ID = 0;
        rdpdr::RDPDR_DTYP type = rdpdr::RDPDR_DTYP::RDPDR_DTYP_UNSPECIFIED;

        DeviceData(const char * name, uint32_t ID, rdpdr::RDPDR_DTYP type)
          : ID(ID)
          , type(type)
        {
            for (int i = 0; i < 8; i++) {
                this->name[i] = name[i];
            }
        }
    };

    std::vector<DeviceData> device_list;

    bool fileSystemCapacity[6] = { true, true, false, false, false, false };


    int writeData_to_wait = 0;
    int file_to_write_id = 0;

    uint32_t current_dir_id = 0;
    std::vector<std::string> elem_in_path;
    uint16_t server_capability_number = 0;

    uint32_t ioCode1 = 0;
    uint32_t extendedPDU = 0;
    uint32_t extraFlags1 = 0;
    uint32_t SpecialTypeDeviceCap = 0;
    uint32_t general_capability_version = 0;

    const uint32_t channel_flags;



    ClientChannelRDPDRManager(RDPVerbose verbose, ClientRedemptionAPI * client, ClientIODiskAPI * impl_io_disk, RDPDiskConfig & config);

    ~ClientChannelRDPDRManager();

    void receive(InStream & chunk) /*NOLINT*/;

    void FremoveDriveDevice(const DeviceData * devices, const size_t deviceCount);

};

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


#include "client_redemption/client_channels/client_rdpdr_channel.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


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
//       | <----------Client Device List Announce Request--------------+
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
//       | <------------Client Drive Device List Announce--------------+
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



ClientRDPDRChannel::ClientRDPDRChannel(RDPVerbose verbose, ClientChannelMod * callback,  RDPDiskConfig & config)
    : verbose(verbose)
    , impl_io_disk(nullptr)
    , callback(callback)
    , ioCode1(config.ioCode1)
    , extendedPDU(config.extendedPDU)
    , extraFlags1(config.extraFlags1)
    , SpecialTypeDeviceCap(config.SpecialTypeDeviceCap)
    , general_capability_version(config.general_capability_version)
    , channel_flags(CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST)
{
    this->fileSystemCapacity[rdpdr::CAP_PRINTER_TYPE]   = config.enable_printer_type;
    this->fileSystemCapacity[rdpdr::CAP_DRIVE_TYPE]     = config.enable_drive_type;
    this->fileSystemCapacity[rdpdr::CAP_SMARTCARD_TYPE] = config.enable_smart_card_type;
    this->fileSystemCapacity[rdpdr::CAP_PORT_TYPE]      = config.enable_port_type;

    for (size_t i = 0; i < config.device_list.size(); i++) {
        DeviceData hard1(config.device_list[i].name, i+1, config.device_list[i].type);
        device_list.push_back(hard1);
    }
}

ClientRDPDRChannel::~ClientRDPDRChannel() {
    this->elem_in_path.clear();
}

void ClientRDPDRChannel::set_api(ClientIODiskAPI * impl_io_disk) {
    this->impl_io_disk = impl_io_disk;
}

void ClientRDPDRChannel::set_share_dir(const std::string & share_dir) {
    this->share_dir = share_dir;
}

void ClientRDPDRChannel::receive(InStream & chunk) /*NOLINT*/
{
    if (!this->impl_io_disk) {
        return ;
    }

    if (this->writeData_to_wait) {
        size_t length(chunk.in_remain());

        this->writeData_to_wait -= length;
        std::string file_to_write = this->paths.at(this->file_to_write_id);
        this->impl_io_disk->write_file(file_to_write.c_str(), char_ptr_cast(chunk.get_current()), length);

        return;
    }

    rdpdr::SharedHeader header;
    header.receive(chunk);

    switch (header.component) {

        case rdpdr::Component::RDPDR_CTYP_CORE:

            switch (header.packet_id) {
                case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Announce Request");
                    }

                    this->process_server_annnounce_request(chunk);
                    break;

                case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Core Capability Request");
                    }

                    this->process_core_server_capabilities(chunk);
                    break;

                case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Client ID Confirm");
                    }

                    this->process_core_clientID_confirm();
                    break;

                case rdpdr::PAKID_CORE_DEVICE_REPLY:
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Device Announce Response");
                    }

                    this->process_core_device_reply(chunk);
                    break;

                case rdpdr::PAKID_CORE_USER_LOGGEDON:
                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server User Logged On");
                    }
                    break;

                case rdpdr::PAKID_CORE_DEVICE_IOREQUEST:
                    this->process_core_device_iorequest(chunk);
                    break;

                default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT RDPDR_CTYP_CORE unknow packetId = %x",       header.packet_id);
                break;
            }
            break;

        case rdpdr::Component::RDPDR_CTYP_PRT:
        {
            //hexdump_c(chunk_series.get_data(), chunk_size);
            chunk.in_skip_bytes(4);

            switch (header.packet_id) {
                case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                {
                    if (bool(this->verbose & RDPVerbose::printer)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Announce Request ");
                    }
                }
                    break;

                case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                {
                    uint16_t capa  = chunk.in_uint16_le();
                    chunk.in_skip_bytes(2);
                    bool driveEnable = false;
                    for (int i = 0; i < capa; i++) {
                        uint16_t type = chunk.in_uint16_le();
                        uint16_t size = chunk.in_uint16_le() - 4;
                        chunk.in_skip_bytes(size);
                        this->fileSystemCapacity[type] = true;
                        if (type == 0x4) {
                            driveEnable = true;
                        }
                    }

                    if (bool(this->verbose & RDPVerbose::printer)) {
                        if (driveEnable) {
                            LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Core Capability Request - Drive Capability Enable");
                        } else {
                            LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Core Capability Request - Drive Not Allowed");
                        }
                    }
                }
                    break;

                case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                    if (bool(this->verbose & RDPVerbose::printer)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server User Logged On");
                    }
                    break;

                case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
                {
                    rdpdr::ServerDeviceAnnounceResponse sdar;
                    sdar.receive(chunk);

                    if (sdar.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS) {
                        this->drives_created = true;
                    } else {
                        this->drives_created = false;
                        LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: Can't create virtual disk ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                    }
                    if (bool(this->verbose & RDPVerbose::printer)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Device Announce Response ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                    }
                }
                    break;

                case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                {
                    if (bool(this->verbose & RDPVerbose::printer)) {
                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Client ID Confirm");
                    }
                }
                    break;

                case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                {
                    rdpdr::DeviceIORequest deviceIORequest;
                    deviceIORequest.receive(chunk);

                    StaticOutStream<1024> out_stream;
                    rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                    , rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                    sharedHeader.emit(out_stream);

                    rdpdr::DeviceIOResponse deviceIOResponse( deviceIORequest.DeviceId()
                                                            , deviceIORequest.CompletionId()
                                                            , erref::NTSTATUS::STATUS_SUCCESS);

                    switch (deviceIORequest.MajorFunction()) {

                        case rdpdr::IRP_MJ_CREATE:
                            if (bool(this->verbose & RDPVerbose::printer)) {
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Create Request");
                            }
                            break;

                        case rdpdr::IRP_MJ_READ:
                            if (bool(this->verbose & RDPVerbose::printer)) {
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Read Request");
                            }
                            break;

                        case rdpdr::IRP_MJ_CLOSE:
                            if (bool(this->verbose & RDPVerbose::printer)) {
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Close Request");
                            }
                            break;

                        default:
                            LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: DEFAULT PRINTER unknow MajorFunction = %x", deviceIORequest.MajorFunction());
                            //hexdump_c(chunk_series.get_data(), chunk_size);
                            break;
                    }
                }
                    break;

                default :
                    LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: DEFAULT PRINTER unknow packetId = %x", header.packet_id);
                    break;
            }
        }
            break;

        default: LOG(LOG_WARNING, "SERVER >> RDPDR: DEFAULT RDPDR unknow component = %x", header.component);
            break;
    }
}

void ClientRDPDRChannel::FremoveDriveDevice(const DeviceData * devices, const size_t deviceCount) {
    StaticOutStream<1024> out_stream;

    rdpdr::SharedHeader sharedHeader( rdpdr::RDPDR_CTYP_CORE
                                    , rdpdr::PAKID_CORE_DEVICELIST_REMOVE);
    sharedHeader.emit(out_stream);
    out_stream.out_uint32_le(deviceCount);
    for (size_t i = 0; i < deviceCount; i++) {
        out_stream.out_uint32_le(devices[i].ID);
    }

    int total_length(out_stream.get_offset());
    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , total_length
                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                        CHANNELS::CHANNEL_FLAG_FIRST
                                        );
    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Client Drive Device List Remove");
    }
}

void ClientRDPDRChannel::process_server_annnounce_request(InStream & chunk) {
    rdpdr::ServerAnnounceRequest sar;
    sar.receive(chunk);
    this->protocol_minor_version = sar.VersionMinor();

    {
        StaticOutStream<32> stream;

        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
        sharedHeader.emit(stream);

        rdpdr::ClientAnnounceReply clientAnnounceReply( sar.VersionMajor()
                                                        , this->protocol_minor_version
                                                        , sar.ClientId());
        clientAnnounceReply.emit(stream);

        int total_length(stream.get_offset());
        InStream chunk_to_send(stream.get_bytes());

        this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , total_length
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Announce Reply");
        }
    }

    {
        StaticOutStream<LOGIN_NAME_MAX*2+32> stream;

        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
        sharedHeader.emit(stream);
        char username[LOGIN_NAME_MAX];
        gethostname(username, LOGIN_NAME_MAX);

        rdpdr::ClientNameRequest clientNameRequest(username, rdpdr::UNICODE_CHAR);
        clientNameRequest.emit(stream);

        int total_length(stream.get_offset());
        InStream chunk_to_send(stream.get_bytes());

        this->callback->send_to_mod_channel( channel_names::rdpdr
                                            , chunk_to_send
                                            , total_length
                                            , this->channel_flags
                                            );
        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Name Request");
        }
    }
}

void ClientRDPDRChannel::process_core_server_capabilities(InStream & chunk) {
    rdpdr::ServerCoreCapabilityRequest sccr;
    sccr.receive(chunk);
    this->server_capability_number = sccr.numCapabilities;

    bool driveEnable = false;
    for (int i = 0; i < this->server_capability_number ; i++) {
            rdpdr::CapabilityHeader ch;
            ch.receive(chunk);

            switch (ch.CapabilityType) {
            case rdpdr::CAP_GENERAL_TYPE:
                {
                    rdpdr::GeneralCapabilitySet gcs;
                    gcs.receive(chunk, ch.Version);
                }
                break;
            case rdpdr::CAP_DRIVE_TYPE:
                driveEnable = true;
                [[fallthrough]];
            case rdpdr::CAP_PRINTER_TYPE:
                [[fallthrough]];
            case rdpdr::CAP_PORT_TYPE:
                [[fallthrough]];
            case rdpdr::CAP_SMARTCARD_TYPE:
                chunk.in_skip_bytes(ch.CapabilityLength - 8);
                this->fileSystemCapacity[ch.CapabilityType] = true;
                break;
        }
    }

    if (!driveEnable) {
        LOG(LOG_WARNING, "SERVER >> RDPDR Channel: Server Core Capability Request - Drive Not Allowed");
    }
}

void ClientRDPDRChannel::process_core_clientID_confirm() {
    {
        StaticOutStream<1024> out_stream;
        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY);
        sharedHeader.emit(out_stream);

        rdpdr::ClientCoreCapabilityResponse cccr(this->server_capability_number);// 5 capabilities.
        cccr.emit(out_stream);

        rdpdr::CapabilityHeader ch(rdpdr::CAP_GENERAL_TYPE, rdpdr::GENERAL_CAPABILITY_VERSION_02);
        ch.CapabilityLength = 36 + 8;   // CapabilityType(2) + CapabilityLength(2) +
                                        //     Version(4) + ...
        ch.emit(out_stream);

        rdpdr::GeneralCapabilitySet general_capability_set(
                0x2,        // osType
                this->protocol_minor_version,        // protocolMinorVersion -
                this->ioCode1,                     // ioCode1
                this->extendedPDU,
                this->extraFlags1,                     // extraFlags1
                this->SpecialTypeDeviceCap,                     // SpecialTypeDeviceCap
                this->general_capability_version
            );
        general_capability_set.emit(out_stream);

        if (this->fileSystemCapacity[rdpdr::CAP_PRINTER_TYPE]) {
            rdpdr::CapabilityHeader ch(rdpdr::CAP_PRINTER_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
            ch.emit(out_stream);
        }

        if (this->fileSystemCapacity[rdpdr::CAP_PORT_TYPE]) {
            rdpdr::CapabilityHeader ch(rdpdr::CAP_PORT_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
            ch.emit(out_stream);
        }

        if (this->fileSystemCapacity[rdpdr::CAP_DRIVE_TYPE]) {
            rdpdr::CapabilityHeader ch(rdpdr::CAP_DRIVE_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
            ch.emit(out_stream);
        }

        if (this->fileSystemCapacity[rdpdr::CAP_SMARTCARD_TYPE]) {
            rdpdr::CapabilityHeader ch(rdpdr::CAP_SMARTCARD_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
            ch.emit(out_stream);
        }

        int total_length(out_stream.get_offset());
        InStream chunk_to_send(out_stream.get_data(), total_length);

        this->callback->send_to_mod_channel( channel_names::rdpdr
                                            , chunk_to_send
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                            CHANNELS::CHANNEL_FLAG_FIRST
                                            );
        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Core Capability Response");
        }
    }

    {
        StaticOutStream<256> out_stream;
        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                        , rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);
        sharedHeader.emit(out_stream);

        rdpdr::ClientDeviceListAnnounceRequest cdlar(this->device_list.size());
        cdlar.emit(out_stream);

        for (auto const& device : this->device_list) {

            if (device.type == rdpdr::RDPDR_DTYP_PRINT) {

                rdpdr::DeviceAnnounceHeader dah( device.type
                                            , device.ID
                                            , device.name
                                            , nullptr, 24 + 0 + 4 + 2 + 8 + 0);
                dah.emit(out_stream);

                rdpdr::DeviceAnnounceHeaderPrinterSpecificData dahp(
                    rdpdr::RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII
                    , 0
                    , 4       // PnPNameLen
                    , 2       // DriverNameLen
                    , 8  // PrintNameLen
                    , 0       // CachedFieldsLen
                    , const_cast<char*>("\x00\x61\x00\x00") /*NOLINT*/ // nPName
                    , const_cast<char*>("\x61\x00") /*NOLINT*/   // DriverName
                    , const_cast<char*>("\x00\x61\x00\x61\x00\x61\x00\x00") /*NOLINT*/ // PrintName
                    );
                dahp.emit(out_stream);

            } else {
                rdpdr::DeviceAnnounceHeader dah( device.type
                                                , device.ID
                                                , device.name
                                                , nullptr, 0);
                dah.emit(out_stream);
            }
        }

        int total_length(out_stream.get_offset());
        InStream chunk_to_send(out_stream.get_bytes());

        this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , total_length
                                        , this->channel_flags
                                        );
        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Device List Announce Request");
        }
    }
}

void ClientRDPDRChannel::process_core_device_reply(InStream & chunk) {
    rdpdr::ServerDeviceAnnounceResponse sdar;
    sdar.receive(chunk);

    if (sdar.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS) {
        this->drives_created = true;
    } else {
        this->drives_created = false;
        LOG(LOG_WARNING, "SERVER >> RDPDR Channel: Can't create virtual disk ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
    }
//     if (bool(this->verbose & RDPVerbose::rdpdr)) {
//         LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Device Announce Response ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
//     }
}

void ClientRDPDRChannel::process_iorequest_lock_control(rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream) {


deviceIOResponse.emit(out_stream);
    rdpdr::ClientDriveLockControlResponse cdlcr;
    cdlcr.emit(out_stream);

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );
}

void ClientRDPDRChannel::process_iorequest_create(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    rdpdr::DeviceCreateRequest request;
    request.receive(chunk);
    request.log();

    if (id == 0) {
        std::string new_path(this->share_dir + request.Path().data());

        if (this->impl_io_disk->ifile_good(new_path.c_str())) {
            id = this->get_file_id();
            this->paths.emplace(id, new_path);
        } else {
            if (request.CreateDisposition() & smb2::FILE_CREATE) {
                id = this->get_file_id();
                this->paths.emplace(id, new_path);
                if (request.CreateOptions() & smb2::FILE_DIRECTORY_FILE) {
//                     LOG(LOG_WARNING, "new directory: \"%s\"", new_path);
                    this->impl_io_disk->marke_dir(new_path.c_str());
                } else {
//                     LOG(LOG_WARNING, "new file: \"%s\"", new_path);

                    if (! ( this->impl_io_disk->ofile_good(new_path.c_str())) ) {
//                         LOG(LOG_WARNING, "  Can't open create such file: \'%s\'.", new_path.c_str());
                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                    }
                }
            } else {
                LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", new_path.c_str());
                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
            }
        }
    }

    uint8_t Information(rdpdr::FILE_SUPERSEDED);
    if (request.CreateDisposition() & smb2::FILE_OPEN_IF) {
        Information = rdpdr::FILE_OPENED;
    }

    rdpdr::DeviceCreateResponse deviceCreateResponse( id
                                                    , Information);

    deviceIOResponse.emit(out_stream);
    deviceCreateResponse.emit(out_stream);

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );

    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Create Response");
    }
}

void ClientRDPDRChannel::process_iorequest_query_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    rdpdr::ServerDriveQueryInformationRequest sdqir;
    sdqir.receive(chunk);

    switch (sdqir.FsInformationClass()) {

        case rdpdr::FileBasicInformation:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Basic Query Information Request");
            }
            {

            std::string file_to_request = this->paths.at(id);

            if (! (this->impl_io_disk->ifile_good(file_to_request.c_str())) ) {
                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", file_to_request.c_str());
            }

            deviceIOResponse.emit(out_stream);

            rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_BASIC_INFORMATION_SIZE);
            cdqir.emit(out_stream);

            ClientIODiskAPI::FileStat fileStat = this->impl_io_disk->get_file_stat(file_to_request.c_str());

            fscc::FileBasicInformation fileBasicInformation(fileStat.LastWriteTime, fileStat.LastAccessTime, fileStat.LastWriteTime, fileStat.ChangeTime, fileStat.FileAttributes);

            fileBasicInformation.emit(out_stream);

            InStream chunk_to_send(out_stream.get_bytes());

            this->callback->send_to_mod_channel(
                channel_names::rdpdr,
                chunk_to_send,
                out_stream.get_offset(),
                this->channel_flags);
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Basic Query Information Response");
            }
            }
            break;

        case rdpdr::FileStandardInformation:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Standard Information Request");
            }
            {
            deviceIOResponse.emit(out_stream);

            rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_STANDARD_INFORMATION_SIZE);
            cdqir.emit(out_stream);

            ClientIODiskAPI::FileStat fileStat = this->impl_io_disk->get_file_stat(this->paths.at(id).c_str());

            fscc::FileStandardInformation fsi( fileStat.AllocationSize
                                                , fileStat.EndOfFile
                                                , fileStat.NumberOfLinks
                                                , fileStat.DeletePending
                                                , fileStat.Directory);
            fsi.emit(out_stream);

            InStream chunk_to_send(out_stream.get_bytes());

            this->callback->send_to_mod_channel( channel_names::rdpdr
                                                , chunk_to_send
                                                , out_stream.get_offset()
                                                , this->channel_flags
                                                );
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Standard Information Response");
            }
            }
            break;

        case rdpdr::FileAttributeTagInformation:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query File Attribute Tag Information Request");
            }
            {
                std::string file_to_request = this->paths.at(id);
                if (!(this->impl_io_disk->ifile_good(file_to_request.c_str()))) {
                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                    //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", file_to_request.c_str());
                }
                deviceIOResponse.emit(out_stream);


                ClientIODiskAPI::FileStat fileStat = this->impl_io_disk->get_file_stat(file_to_request.c_str());

                uint32_t fileAttributes(fileStat.FileAttributes & fscc::FILE_ATTRIBUTE_ARCHIVE);

                rdpdr::ClientDriveQueryInformationResponse cdqir(8);
                cdqir.emit(out_stream);

                fscc::FileAttributeTagInformation fati( fileAttributes
                                                    , 0);
                fati.emit(out_stream);

                InStream chunk_to_send(out_stream.get_bytes());

                this->callback->send_to_mod_channel( channel_names::rdpdr
                                                    , chunk_to_send
                                                    , out_stream.get_offset()
                                                    , this->channel_flags
                                                    );

                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query File Attribute Tag Information Response");
                }
            }
            break;

        default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request             unknow FsInformationClass = %x",       sdqir.FsInformationClass());
            break;
    }
}

void ClientRDPDRChannel::process_iorequest_close(rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    this->paths.erase(id);

    deviceIOResponse.emit(out_stream);

    out_stream.out_uint32_le(0);

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                       , chunk_to_send
                                       , out_stream.get_offset()
                                       , this->channel_flags
                                       );
    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Close Response");
    }
}

void ClientRDPDRChannel::process_iorequest_read(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    rdpdr::DeviceReadRequest drr;
    drr.receive(chunk);

    std::string file_to_tread = this->paths.at(id);
    int portion_length = 0;
    int file_size = this->impl_io_disk->get_file_size(file_to_tread.c_str());
    if (file_size < 0) {
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_tread.c_str());
    } else {

        portion_length = std::min(static_cast<int>(drr.Length()), file_size);
        int offset(drr.Offset());

        if (this->last_read_data_portion_length != portion_length) {
            this->ReadData = std::make_unique<uint8_t[]>(portion_length);
            this->last_read_data_portion_length = portion_length;
        }

        this->impl_io_disk->read_data(file_to_tread, offset, {ReadData.get(), static_cast<size_t>(portion_length)});
    }

    deviceIOResponse.emit(out_stream);
    rdpdr::DeviceReadResponse deviceReadResponse(portion_length);
    deviceReadResponse.emit(out_stream);

    this->callback->process_client_channel_out_data( channel_names::rdpdr
                                        , 20 + portion_length
                                        , out_stream
                                        , out_stream.get_capacity() - 20
                                        , {ReadData.get(), size_t(portion_length)}
                                        , 0);
    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Read Response");
    }
}

void ClientRDPDRChannel::process_iorequest_directory_control(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id, uint32_t minor_function) {

    switch (minor_function) {

        case rdpdr::IRP_MN_QUERY_DIRECTORY:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Directory Request");
            }
            {
            std::string slash("/");
            std::string asterix("*");

            rdpdr::ServerDriveQueryDirectoryRequest sdqdr;
            sdqdr.receive(chunk);

            ClientIODiskAPI::FileStat child;

            std::string path = sdqdr.Path();
            std::string endPath;
            if (path.length() > 0) {
                endPath = path.substr(path.length() -1, path.length());
            }

            std::string str_file_name;

            if (sdqdr.InitialQuery() && endPath != asterix) {

                std::string tmp_path = path;
                int tmp_path_index = tmp_path.find('/');
                while (tmp_path_index != -1) {
                    tmp_path = tmp_path.substr(tmp_path_index+1, tmp_path.length());
                    tmp_path_index = tmp_path.find('/');
                }
                str_file_name = std::move(tmp_path);

                std::string str_file_path_slash(this->share_dir + path);

                if (this->impl_io_disk->dir_good(str_file_path_slash.c_str())) {
                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                    //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                }

            } else {

                std::string str_dir_path;
                if (this->paths.end() != this->paths.find(id)) {
                    str_dir_path = this->paths.at(id);
                } else {
                    LOG(LOG_WARNING, " Device I/O Query Directory Request Unknow ID (%u).", id);
                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                }

                if (str_dir_path.length() > 0) {
                    std::string test = str_dir_path.substr(str_dir_path.length() -1, str_dir_path.length());
                    if (test == slash) {
                        str_dir_path = str_dir_path.substr(0, str_dir_path.length()-1);
                    }
                }

                if (sdqdr.InitialQuery()) {

                    if (!(this->impl_io_disk->set_elem_from_dir(this->elem_in_path, str_dir_path))) {
                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                    }
                }

                if (this->elem_in_path.empty()) {
                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_MORE_FILES);
                } else {
                    str_file_name = this->elem_in_path[0];
                    this->elem_in_path.erase(this->elem_in_path.begin());

                    std::string str_file_path_slash(str_dir_path + "/");
                    str_file_path_slash += str_file_name;
                    struct stat buff_child;
                    if (stat(str_file_path_slash.c_str(), &buff_child)) {
                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                        //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                    } else {
                            child = this->impl_io_disk->get_file_stat(str_file_path_slash.c_str());
                    }
                }
            }

            deviceIOResponse.emit(out_stream);

            switch (sdqdr.FsInformationClass()) {

                case rdpdr::FileDirectoryInformation:
                {
                    fscc::FileDirectoryInformation fbdi(child.CreationTime,
                                                        child.LastAccessTime,
                                                        child.LastWriteTime,
                                                        child.ChangeTime,
                                                        child.EndOfFile,
                                                        child.AllocationSize,
                                                        child.FileAttributes,
                                                        str_file_name);

                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(fbdi.size());
                    cdqdr.emit(out_stream);

                    fbdi.emit(out_stream);
                }
                    break;
                case rdpdr::FileFullDirectoryInformation:
                {
                    fscc::FileFullDirectoryInformation ffdi(child.CreationTime,
                                                            child.LastAccessTime,
                                                            child.LastWriteTime,
                                                            child.ChangeTime,
                                                            child.EndOfFile,
                                                            child.AllocationSize,
                                                            child.FileAttributes,
                                                            str_file_name);

                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffdi.size());
                    cdqdr.emit(out_stream);

                    ffdi.emit(out_stream);
                }
                    break;
                case rdpdr::FileBothDirectoryInformation:
                {
                    fscc::FileBothDirectoryInformation fbdi(child.CreationTime,                                                                                 child.LastAccessTime, child.LastWriteTime, child.ChangeTime, child.EndOfFile, child.AllocationSize, child.FileAttributes, str_file_name);

                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(fbdi.size());
                    cdqdr.emit(out_stream);

                    fbdi.emit(out_stream);
                }
                    break;
                case rdpdr::FileNamesInformation:
                {
                    fscc::FileNamesInformation ffi(str_file_name);

                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffi.size());
                    cdqdr.emit(out_stream);

                    ffi.emit(out_stream);
                }
                    break;
                default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow  FsInformationClass = 0x%x", sdqdr.FsInformationClass());
                        break;
            }

            InStream chunk_to_send(out_stream.get_bytes());

            this->callback->send_to_mod_channel( channel_names::rdpdr
                                                    , chunk_to_send
                                                    , out_stream.get_offset()
                                                    , this->channel_flags
                                                );
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Directory Response");
            }
            }
            break;

        case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Notify Change Directory Request");
            }
            {
                rdpdr::ServerDriveNotifyChangeDirectoryRequest sdncdr;
                sdncdr.receive(chunk);

                if (sdncdr.WatchTree) {

    //                                                 deviceIOResponse.emit(out_stream);
    //
    //                                                 fscc::FileNotifyInformation fni();
    //                                                 fni.emit(out_stream);
    //
    //                                                 InStream chunk_to_send(out_stream.get_bytes());
    //
    //                                                 this->callback->send_to_mod_channel( channel_names::rdpdr
    //                                                                                     , chunk_to_send
    //                                                                                     , out_stream.get_offset()
    //                                                                                     , CHANNELS::CHANNEL_FLAG_LAST |
    //                                                                                       CHANNELS::CHANNEL_FLAG_FIRST
    //                                                                                     );

                    LOG(LOG_WARNING, "CLIENT >> RDPDR: Device I/O Must Send Notify Change Directory Response");
                }
            }
            break;

        default: break;
    }
}

void ClientRDPDRChannel::process_iorequest_query_volume_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    rdpdr::ServerDriveQueryVolumeInformationRequest sdqvir;
    sdqvir.receive(chunk);

    uint32_t VolumeSerialNumber = 0;

    std::string str_path;

    if (this->paths.end() != this->paths.find(id)) {
        str_path = this->paths.at(id);
    } else {
        LOG(LOG_WARNING, " Device I/O Query Volume Information Request Unknow ID (%u).", id);
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
    }

    if ((this->impl_io_disk->ifile_good(str_path.c_str()) )) {
        this->impl_io_disk->get_file_statvfs(str_path.c_str());
    } else {
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
        LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buffvfs).", str_path.c_str());
    }

    int device = this->impl_io_disk->get_device(str_path.c_str());
    if (device < 0) {
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
        //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (hd_driveid).", str_path.c_str());
    } else {
        VolumeSerialNumber = this->impl_io_disk->get_volume_serial_number(device);
    }

    deviceIOResponse.emit(out_stream);

    ClientIODiskAPI::FileStatvfs fileStatvfs;

    if (deviceIOResponse.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
        switch (sdqvir.FsInformationClass()) {
            case rdpdr::FileFsVolumeInformation:
            {
                fscc::FileFsVolumeInformation ffvi(fileStatvfs.VolumeCreationTime, VolumeSerialNumber, 0, fileStatvfs.VolumeLabel);

                rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffvi.size());
                cdqvir.emit(out_stream);

                ffvi.emit(out_stream);
            }
                break;

            case rdpdr::FileFsSizeInformation:
            {
                fscc::FileFsSizeInformation ffsi(fileStatvfs.TotalAllocationUnits, fileStatvfs.AvailableAllocationUnits, fileStatvfs.SectorsPerAllocationUnit, fileStatvfs.BytesPerSector);

                rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffsi.size());
                cdqvir.emit(out_stream);

                ffsi.emit(out_stream);
            }
                break;

            case rdpdr::FileFsAttributeInformation:
            {
                fscc::FileFsAttributeInformation ffai(fileStatvfs.FileSystemAttributes, fileStatvfs.MaximumComponentNameLength, fileStatvfs.FileSystemName);

                rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffai.size());
                cdqvir.emit(out_stream);

                ffai.emit(out_stream);
            }
                break;
            case rdpdr::FileFsFullSizeInformation:
            {
                fscc::FileFsFullSizeInformation fffsi(fileStatvfs.TotalAllocationUnits, fileStatvfs.CallerAvailableAllocationUnits, fileStatvfs.ActualAvailableAllocationUnits, fileStatvfs.SectorsPerAllocationUnit, fileStatvfs.BytesPerSector);

                rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(fffsi.size());
                cdqvir.emit(out_stream);

                fffsi.emit(out_stream);
            }
                break;

            case rdpdr::FileFsDeviceInformation:
            {
                fscc::FileFsDeviceInformation ffdi(fscc::FILE_DEVICE_DISK, fscc::FILE_REMOTE_DEVICE | fscc::FILE_DEVICE_IS_MOUNTED);

                rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffdi.size());
                cdqvir.emit(out_stream);

                ffdi.emit(out_stream);
            }
                break;

            default:
                LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow FsInformationClass = 0x%x", sdqvir.FsInformationClass());
                break;
        }
    }

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );

    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Volume Information Response");
    }
}

void ClientRDPDRChannel::provess_iorequest_write(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {
    rdpdr::DeviceWriteRequest dwr;
    dwr.receive(chunk);

    size_t WriteDataLen(dwr.Length);

    if (dwr.Length > CHANNELS::CHANNEL_CHUNK_LENGTH) {

        this->writeData_to_wait = dwr.Length - rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
        this->file_to_write_id = id;
        WriteDataLen = rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
    }

    std::string file_to_write = this->paths.at(id);

    if (this->impl_io_disk->write_file(file_to_write.c_str(), char_ptr_cast(dwr.WriteData), WriteDataLen ) ) {
        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_write.c_str());
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
    }

    deviceIOResponse.emit(out_stream);
    rdpdr::DeviceWriteResponse dwrp(dwr.Length);
    dwrp.emit(out_stream);

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );
    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Response");
    }
}

void ClientRDPDRChannel::process_iorequest_set_information(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream, uint32_t id) {

    rdpdr::ServerDriveSetInformationRequest sdsir;
    sdsir.receive(chunk);

    std::string file_to_request = this->paths.at(id);

    if (! (this->impl_io_disk->ifile_good(file_to_request.c_str()))) {
        LOG(LOG_WARNING, "  Can't open such file of directory : \'%s\'.", file_to_request.c_str());
        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
    }


    rdpdr::ClientDriveSetInformationResponse cdsir(sdsir.Length());


    switch (sdsir.FsInformationClass()) {

        case rdpdr::FileRenameInformation:
        {
            rdpdr::RDPFileRenameInformation rdpfri;
            rdpfri.receive(chunk);

            std::string fileName(this->share_dir + rdpfri.FileName());

            if (this->impl_io_disk->rename_file(file_to_request.c_str(), fileName.c_str())) {
                LOG(LOG_WARNING, "  Can't rename such file of directory : \'%s\' to \'%s\'.", file_to_request.c_str(), fileName.c_str());
                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_OBJECT_NAME_INVALID);
            }

            deviceIOResponse.emit(out_stream);
            cdsir.emit(out_stream);
        }
            break;

        case rdpdr::FileAllocationInformation :
            deviceIOResponse.emit(out_stream);
            cdsir.emit(out_stream);
            break;

        case rdpdr::FileEndOfFileInformation:
            deviceIOResponse.emit(out_stream);
            cdsir.emit(out_stream);
            break;

        case rdpdr::FileDispositionInformation:
        {
            uint8_t DeletePending = 1;

            std::string file_to_request = this->paths.at(id);

            if (this->impl_io_disk->remove_file(file_to_request.c_str())) {
                DeletePending = 0;
                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                LOG(LOG_WARNING, "  Can't delete such file of directory : \'%s\'.", file_to_request.c_str());
            }

            deviceIOResponse.emit(out_stream);
            cdsir.emit(out_stream);
            fscc::FileDispositionInformation fdi(DeletePending);
            fdi.emit(out_stream);
        }
            break;

        case rdpdr::FileBasicInformation:
        {
            deviceIOResponse.emit(out_stream);
            cdsir.emit(out_stream);
        }
            break;

        default:  LOG(LOG_WARNING, "SERVER >> RDPDR: unknow FsInformationClass = 0x%x", sdsir.FsInformationClass());

            break;
    }

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                    , chunk_to_send
                                    , out_stream.get_offset()
                                    , this->channel_flags
                                    );
    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Set Information Response");
    }
}

void ClientRDPDRChannel::process_iorequest_device_control(InStream & chunk, rdpdr::DeviceIOResponse &  deviceIOResponse, OutStream & out_stream) {
    rdpdr::DeviceControlRequest dcr;
    dcr.receive(chunk);

    deviceIOResponse.emit(out_stream);

    switch (dcr.IoControlCode()) {
        case fscc::FSCTL_CREATE_OR_GET_OBJECT_ID :
        {
            rdpdr::DriveControlResponse cdcr(64);
            cdcr.emit(out_stream);

            uint8_t ObjectId[16] =  { 0 };
            ObjectId[0] = 1;
            uint8_t BirthVolumeId[16] =  { 0 };
            BirthVolumeId[15] = 1;
            uint8_t BirthObjectId[16] =  { 0 };
            BirthObjectId[15] = 1;

            fscc::FileObjectBuffer_Type1 rgdb(ObjectId, BirthVolumeId, BirthObjectId);
            rgdb.emit(out_stream);
        }
        break;

        case fscc::FSCTL_GET_OBJECT_ID :
        {
            rdpdr::DriveControlResponse cdcr(64);
            cdcr.emit(out_stream);

            uint8_t ObjectId[16] =  { 0 };
            ObjectId[0] = 1;
            uint8_t BirthVolumeId[16] =  { 0 };
            uint8_t BirthObjectId[16] =  { 0 };

            fscc::FileObjectBuffer_Type1 rgdb(ObjectId, BirthVolumeId, BirthObjectId);
            rgdb.emit(out_stream);

        }
            break;

        default: LOG(LOG_INFO, "     Device Controle UnLogged IO Control Data: Code = 0x%08x", dcr.IoControlCode());
            break;
    }

    InStream chunk_to_send(out_stream.get_bytes());

    this->callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , out_stream.get_offset()
                                        , this->channel_flags
                                        );

    if (bool(this->verbose & RDPVerbose::rdpdr)) {
        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Client Drive Control Response");
    }
}

void ClientRDPDRChannel::process_core_device_iorequest(InStream & chunk) {
    rdpdr::DeviceIORequest deviceIORequest;
    deviceIORequest.receive(chunk);

    StaticOutStream<1024> out_stream;
    rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                    , rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
    sharedHeader.emit(out_stream);

    uint32_t id = deviceIORequest.FileId();

    rdpdr::DeviceIOResponse deviceIOResponse( deviceIORequest.DeviceId()
                                            , deviceIORequest.CompletionId()
                                            , erref::NTSTATUS::STATUS_SUCCESS);

    switch (deviceIORequest.MajorFunction()) {

        case rdpdr::IRP_MJ_LOCK_CONTROL:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Lock Control Request");
            }

            this->process_iorequest_lock_control(deviceIOResponse, out_stream);
            break;

        case rdpdr::IRP_MJ_CREATE:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Create Request");
            }

            this->process_iorequest_create(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_QUERY_INFORMATION:
            this->process_iorequest_query_information(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_CLOSE:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Close Request");
            }

            this->process_iorequest_close(deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_READ:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Read Request");
            }

            this->process_iorequest_read(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
            this->process_iorequest_directory_control(chunk, deviceIOResponse, out_stream, id, deviceIORequest.MinorFunction());
            break;

        case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Volume Information Request");
            }

            this->process_iorequest_query_volume_information(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_WRITE:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Request");
            }

            this->provess_iorequest_write(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_SET_INFORMATION:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Server Drive Set Information Request");
            }

            this->process_iorequest_set_information(chunk, deviceIOResponse, out_stream, id);
            break;

        case rdpdr::IRP_MJ_DEVICE_CONTROL:
            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Control Request");
            }

            this->process_iorequest_device_control(chunk, deviceIOResponse, out_stream);
            break;

        default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request unknow MajorFunction = %x",       deviceIORequest.MajorFunction());
            break;
    }
}

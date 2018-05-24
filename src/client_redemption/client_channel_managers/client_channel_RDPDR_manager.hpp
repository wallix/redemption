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
   Copyright (C) Wallix 2010
   Author(s): Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

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

#include "client_redemption/client_input_output_api.hpp"



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



class ClientChannelRDPDRManager {

    RDPVerbose verbose;
    ClientRedemptionAPI * client;

    ClientIODiskAPI * impl_io_disk;

public:
    struct FileSystemData {

        struct DeviceData {
            char name[8] = {0};
            uint32_t ID = 0;
            rdpdr::RDPDR_DTYP type = rdpdr::RDPDR_DTYP::RDPDR_DTYP_UNSPECIFIED;
        };

        bool drives_created = false;
        bool fileSystemCapacity[6] = { false };
        size_t devicesCount = 1;
        DeviceData devices[2];

        uint32_t next_file_id = 0;

        uint32_t get_file_id() {
            this->next_file_id++;
            return this->next_file_id;
        }

        std::unordered_map<int, std::string> paths;

        int writeData_to_wait = 0;
        int file_to_write_id = 0;

        uint32_t current_dir_id = 0;
        std::vector<std::string> elem_in_path;

        uint16_t protocol_minor_version = 0;

    } fileSystemData;

    uint16_t server_capability_number;



    ClientChannelRDPDRManager(RDPVerbose verbose, ClientRedemptionAPI * client, ClientIODiskAPI * impl_io_disk)
      : verbose(verbose)
      , client(client)
      , impl_io_disk(impl_io_disk)
      , server_capability_number(0)
    {
        std::string tmp(this->client->SHARE_DIR);
        int pos(tmp.find('/'));

        this->fileSystemData.devicesCount = 0;

        while (pos != -1) {
            tmp = tmp.substr(pos+1, tmp.length());
            pos = tmp.find('/');
        }
        size_t size(tmp.size());
        if (size > 7) {
            size = 7;
        }
        for (size_t i = 0; i < size; i++) {
            this->fileSystemData.devices[this->fileSystemData.devicesCount].name[i] = tmp.data()[i];
        }
        this->fileSystemData.devices[this->fileSystemData.devicesCount].ID = 1;
        this->fileSystemData.devices[this->fileSystemData.devicesCount].type = rdpdr::RDPDR_DTYP_FILESYSTEM;
        this->fileSystemData.devicesCount++;


        std::string name_printer("printer");
        const char * char_name_printer = name_printer.c_str();
        size = name_printer.size();
        if (size > 7) {
            size = 7;
        }
        for (size_t i = 0; i < size; i++) {
            this->fileSystemData.devices[this->fileSystemData.devicesCount].name[i] = char_name_printer[i];
        }
        this->fileSystemData.devices[this->fileSystemData.devicesCount].ID = 2;
        this->fileSystemData.devices[this->fileSystemData.devicesCount].type = rdpdr::RDPDR_DTYP_PRINT;
        this->fileSystemData.devicesCount++;
    }

    ~ClientChannelRDPDRManager() {
        this->fileSystemData.elem_in_path.clear();
    }


    void receive(InStream & chunk) {
        if (this->impl_io_disk == nullptr) {
            return ;
        }

        if (this->fileSystemData.writeData_to_wait) {
            size_t length(chunk.in_remain());

            this->fileSystemData.writeData_to_wait -= length;

            std::string file_to_write = this->fileSystemData.paths.at(this->fileSystemData.file_to_write_id);

            std::ofstream oFile(file_to_write.c_str(), std::ios::binary | std::ios::app);
            if (oFile.good()) {
                oFile.write(reinterpret_cast<const char *>(chunk.get_current()), length);
                oFile.close();
            }  else {
                LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_write.c_str());
            }

            return;
        }

        rdpdr::SharedHeader header;
        header.receive(chunk);

        switch (header.component) {

            case rdpdr::Component::RDPDR_CTYP_CORE:

                switch (header.packet_id) {
                    case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                        {
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Announce Request");
                        }

                        uint16_t versionMajor = chunk.in_uint16_le();
                        this->fileSystemData.protocol_minor_version = chunk.in_uint16_le();
                        uint32_t clientID = chunk.in_uint32_le();

                        StaticOutStream<32> stream;

                        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                        , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
                        sharedHeader.emit(stream);

                        rdpdr::ClientAnnounceReply clientAnnounceReply( versionMajor
                                                                      , this->fileSystemData.protocol_minor_version
                                                                      , clientID);
                        clientAnnounceReply.emit(stream);

                        int total_length(stream.get_offset());
                        InStream chunk_to_send(stream.get_data(), stream.get_offset());

                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                        , chunk_to_send
                                                        , total_length
                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                        CHANNELS::CHANNEL_FLAG_FIRST
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
                        InStream chunk_to_send(stream.get_data(), stream.get_offset());

                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                            , chunk_to_send
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                            );
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Name Request");
                        }
                        }
                        break;

                    case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                        {
                        this->server_capability_number  = chunk.in_uint16_le();
                        chunk.in_skip_bytes(2);
                        bool driveEnable = false;
                        for (int i = 0; i < this->server_capability_number ; i++) {
                            uint16_t type = chunk.in_uint16_le();
                            uint16_t size = chunk.in_uint16_le() - 4;
                            chunk.in_skip_bytes(size);
                            this->fileSystemData.fileSystemCapacity[type] = true;
                            if (type == rdpdr::CAP_DRIVE_TYPE) {
                                driveEnable = true;
                            }
                        }

                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            if (driveEnable) {
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Core Capability Request - Drive Capability Enable");
                                //this->show_in_stream(0, chunk_series, chunk_size);
                            } else {
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Core Capability Request - Drive Not Allowed");
                                //this->show_in_stream(0, chunk_series, chunk_size);
                            }
                        }
                        }
                        break;

                    case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Client ID Confirm");
                        }
                        //this->show_in_stream(0, chunk_series, chunk_size);
                        {
                        StaticOutStream<1024> out_stream;
                        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                        , rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY);
                        sharedHeader.emit(out_stream);

                        out_stream.out_uint16_le(this->server_capability_number);    // 5 capabilities.
                        out_stream.out_clear_bytes(2);  // Padding(2)

                        // General capability set
                        out_stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
                        out_stream.out_uint16_le(36 + 8);
                                /*rdpdr::GeneralCapabilitySet::size(
                                    general_capability_version) +
                                8   // CapabilityType(2) + CapabilityLength(2) +
                                    //     Version(4)
                            );*/
                        out_stream.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_02);

                        rdpdr::GeneralCapabilitySet general_capability_set(
                                0x2,        // osType
                                this->fileSystemData.protocol_minor_version,        // protocolMinorVersion -
                                rdpdr::SUPPORT_ALL_REQUEST,     // ioCode1
                                rdpdr::RDPDR_DEVICE_REMOVE_PDUS |           // extendedPDU -
                                    rdpdr::RDPDR_CLIENT_DISPLAY_NAME_PDU  |
                                    rdpdr::RDPDR_USER_LOGGEDON_PDU,
                                rdpdr::ENABLE_ASYNCIO,        // extraFlags1
                                0,                          // SpecialTypeDeviceCap
                                rdpdr::GENERAL_CAPABILITY_VERSION_02
                            );

                        general_capability_set.emit(out_stream);

                        if (this->fileSystemData.fileSystemCapacity[rdpdr::CAP_PRINTER_TYPE] == true) {
                            rdpdr::CapabilityHeader ch(rdpdr::CAP_PRINTER_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch.emit(out_stream);
                        }

                        if (this->fileSystemData.fileSystemCapacity[rdpdr::CAP_PORT_TYPE] == true) {
                            rdpdr::CapabilityHeader ch(rdpdr::CAP_PORT_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch.emit(out_stream);
                        }

                        if (this->fileSystemData.fileSystemCapacity[rdpdr::CAP_DRIVE_TYPE] == true) {
                            rdpdr::CapabilityHeader ch(rdpdr::CAP_DRIVE_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch.emit(out_stream);
                        }

                        if (this->fileSystemData.fileSystemCapacity[rdpdr::CAP_SMARTCARD_TYPE] == true) {
                            rdpdr::CapabilityHeader ch(rdpdr::CAP_SMARTCARD_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch.emit(out_stream);
                        }

                        int total_length(out_stream.get_offset());
                        InStream chunk_to_send(out_stream.get_data(), total_length);

                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
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

                        rdpdr::ClientDeviceListAnnounceRequest cdlar(this->fileSystemData.devicesCount);
                        cdlar.emit(out_stream);

                        for (size_t i = 0; i < this->fileSystemData.devicesCount; i++) {

                            if (this->fileSystemData.devices[i].type == rdpdr::RDPDR_DTYP_PRINT) {

                                rdpdr::DeviceAnnounceHeader dah( this->fileSystemData.devices[i].type
                                                               , this->fileSystemData.devices[i].ID
                                                               , this->fileSystemData.devices[i].name
                                                               , nullptr, 24 + 0 + 4 + 2 + 8 + 0);
                                dah.emit(out_stream);

                                rdpdr::DeviceAnnounceHeaderPrinterSpecificData dahp(
//                                                     this->fileSystemData.devices[i].type
//                                                   , this->fileSystemData.devices[i].ID
//                                                   , this->fileSystemData.devices[i].name
//                                                     24 + 0 + 4 + 2 + sizeof(printName)
                                                rdpdr::RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII
                                                , 0
                                                , 4       // PnPNameLen
                                                , 2       // DriverNameLen
                                                , 8  // PrintNameLen
                                                , 0       // CachedFieldsLen
                                                , const_cast<char*>("\x00\x61\x00\x00") // nPName
                                                , const_cast<char*>("\x61\x00")   // DriverName
                                                , const_cast<char*>("\x00\x61\x00\x61\x00\x61\x00\x00") // PrintName
                                                );
                                dahp.emit(out_stream);

                            } else {
                                rdpdr::DeviceAnnounceHeader dah( this->fileSystemData.devices[i].type
                                                               , this->fileSystemData.devices[i].ID
                                                               , this->fileSystemData.devices[i].name
                                                               , nullptr, 0);
                                dah.emit(out_stream);
                            }
                        }

                        int total_length(out_stream.get_offset());
                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                        , chunk_to_send
                                                        , total_length
                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                        );
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Device List Announce Request");
                        }
                        }
                        break;

                    case rdpdr::PAKID_CORE_DEVICE_REPLY:
                        {
                        rdpdr::ServerDeviceAnnounceResponse sdar;
                        sdar.receive(chunk);

                        if (sdar.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS) {
                            this->fileSystemData.drives_created = true;
                        } else {
                            this->fileSystemData.drives_created = false;
                            LOG(LOG_WARNING, "SERVER >> RDPDR Channel: Can't create virtual disk ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                        }
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Device Announce Response ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                        }
                        }
                        break;

                    case rdpdr::PAKID_CORE_USER_LOGGEDON:
                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server User Logged On");
                        }
                        break;

                    case rdpdr::PAKID_CORE_DEVICE_IOREQUEST:
                        {
                        rdpdr::DeviceIORequest deviceIORequest;
                        deviceIORequest.receive(chunk);

                        if (deviceIORequest.DeviceId() ==  2) {
                            this->verbose = RDPVerbose::rdpdr;
                        }

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
                            {
                                deviceIOResponse.emit(out_stream);

                                rdpdr::ClientDriveLockControlResponse cdlcr;
                                cdlcr.emit(out_stream);

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                            }
                                break;

                            case rdpdr::IRP_MJ_CREATE:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Create Request");
                                }
                                {
                                rdpdr::DeviceCreateRequest request;
                                request.receive(chunk);

                                if (id == 0) {

                                    std::string new_path(this->client->SHARE_DIR + request.Path());

                                    if (this->impl_io_disk->ifile_good(new_path.c_str())) {
                                        id = this->fileSystemData.get_file_id();
                                        this->fileSystemData.paths.emplace(id, new_path);
                                    } else {
//                                         LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Create Request 3");
                                        if (request.CreateDisposition() & smb2::FILE_CREATE) {

                                            id = this->fileSystemData.get_file_id();
                                            this->fileSystemData.paths.emplace(id, new_path);

                                            if (request.CreateOptions() & smb2::FILE_DIRECTORY_FILE) {
                                                LOG(LOG_WARNING, "new directory: \"%s\"", new_path);
                                                this->impl_io_disk->marke_dir(new_path.c_str());
                                            } else {
                                                //LOG(LOG_WARNING, "new file: \"%s\"", new_path);

                                                if (! ( this->impl_io_disk->ofile_good(new_path.c_str())) ) {
                                                    LOG(LOG_WARNING, "  Can't open create such file: \'%s\'.", new_path.c_str());
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                }
                                            }
                                        } else {
                                            //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", new_path.c_str());
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

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );

                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Create Response");
                                }
                                }
                                break;

                            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                                {
                                rdpdr::ServerDriveQueryInformationRequest sdqir;
                                sdqir.receive(chunk);

                                switch (sdqir.FsInformationClass()) {

                                    case rdpdr::FileBasicInformation:
                                        if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Basic Query Information Request");
                                        }
                                        {

                                        std::string file_to_request = this->fileSystemData.paths.at(id);

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

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
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

                                        ClientIODiskAPI::FileStat fileStat = this->impl_io_disk->get_file_stat(this->fileSystemData.paths.at(id).c_str());

                                        fscc::FileStandardInformation fsi( fileStat.AllocationSize
                                                                         , fileStat.EndOfFile
                                                                         , fileStat.NumberOfLinks
                                                                         , fileStat.DeletePending
                                                                         , fileStat.Directory);
                                        fsi.emit(out_stream);

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
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
                                            std::string file_to_request = this->fileSystemData.paths.at(id);

                                            std::ifstream file(file_to_request.c_str());
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

                                            InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                            this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                                , chunk_to_send
                                                                                , out_stream.get_offset()
                                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                                    CHANNELS::CHANNEL_FLAG_FIRST
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
                                break;

                            case rdpdr::IRP_MJ_CLOSE:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Close Request");
                                }
                                {

                                this->fileSystemData.paths.erase(id);

                                deviceIOResponse.emit(out_stream);

                                out_stream.out_uint32_le(0);

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Close Response");
                                }
                                }
                                break;

                            case rdpdr::IRP_MJ_READ:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Read Request");
                                }
                                {
                                rdpdr::DeviceReadRequest drr;
                                drr.receive(chunk);

                                uint32_t file_size{drr.Length()};
                                uint64_t offset{drr.Offset()};
                                std::unique_ptr<uint8_t[]> ReadData = std::make_unique<uint8_t[]>(file_size);
                                std::string file_to_tread = this->fileSystemData.paths.at(id);

                                deviceIOResponse.set_IoStatus(this->impl_io_disk->read_data(
                                    file_to_tread, offset, {ReadData.get(), file_size}, true));

                                deviceIOResponse.emit(out_stream);
                                rdpdr::DeviceReadResponse deviceReadResponse(file_size);
                                deviceReadResponse.emit(out_stream);

                                this->process_client_clipboard_out_data( channel_names::rdpdr
                                                                    , 20 + file_size
                                                                    , out_stream
                                                                    , out_stream.get_capacity() - 20
                                                                    , ReadData.get()
                                                                    , file_size
                                                                    , 0);
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Read Response");
                                }
                                }
                                break;

                            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:

                                switch (deviceIORequest.MinorFunction()) {

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

                                        struct stat buff_child;
                                        std::string str_file_name;

                                        if (sdqdr.InitialQuery() && endPath != asterix) {

                                            std::string tmp_path = path;
                                            int tmp_path_index = tmp_path.find("/");
                                            while (tmp_path_index != -1) {
                                                tmp_path = tmp_path.substr(tmp_path_index+1, tmp_path.length());
                                                tmp_path_index = tmp_path.find("/");
                                            }
                                            str_file_name = tmp_path;

                                            std::string str_file_path_slash(this->client->SHARE_DIR + path);

                                            if (this->impl_io_disk->dir_good(str_file_path_slash.c_str())) {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                                            }

                                        } else {

                                            std::string str_dir_path;
                                            if (this->fileSystemData.paths.end() != this->fileSystemData.paths.find(id)) {
                                                str_dir_path = this->fileSystemData.paths.at(id);
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

                                                if (!(this->impl_io_disk->set_elem_from_dir(this->fileSystemData.elem_in_path, str_dir_path))) {
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                }
                                            }

                                            if (this->fileSystemData.elem_in_path.size() == 0) {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_MORE_FILES);
                                            } else {
                                                str_file_name = this->fileSystemData.elem_in_path[0];
                                                this->fileSystemData.elem_in_path.erase(this->fileSystemData.elem_in_path.begin());

                                                std::string str_file_path_slash(str_dir_path + "/" + str_file_name);
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
                                                                                    str_file_name.c_str());

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
                                                                                        str_file_name.c_str());

                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffdi.size());
                                                cdqdr.emit(out_stream);

                                                ffdi.emit(out_stream);
                                            }
                                                break;
                                            case rdpdr::FileBothDirectoryInformation:
                                            {
                                                fscc::FileBothDirectoryInformation fbdi(child.CreationTime,                                                                                 child.LastAccessTime, child.LastWriteTime, child.ChangeTime, child.EndOfFile, child.AllocationSize, child.FileAttributes, str_file_name.c_str());

                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(fbdi.size());
                                                cdqdr.emit(out_stream);

                                                fbdi.emit(out_stream);
                                            }
                                                break;
                                            case rdpdr::FileNamesInformation:
                                            {
                                                fscc::FileNamesInformation ffi(str_file_name.c_str());

                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffi.size());
                                                cdqdr.emit(out_stream);

                                                ffi.emit(out_stream);
                                            }
                                                break;
                                            default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow  FsInformationClass = 0x%x", sdqdr.FsInformationClass());
                                                    break;
                                        }

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                                CHANNELS::CHANNEL_FLAG_FIRST
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
//                                                 InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
//
//                                                 this->client->mod->send_to_mod_channel( channel_names::rdpdr
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
                                break;

                            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Volume Information Request");
                                }
                                {
                                    rdpdr::ServerDriveQueryVolumeInformationRequest sdqvir;
                                    sdqvir.receive(chunk);

                                    uint32_t VolumeSerialNumber             = 0;

                                    ClientIODiskAPI::FileStatvfs fileStatvfs;

                                    std::string str_path;

                                    if (this->fileSystemData.paths.end() != this->fileSystemData.paths.find(id)) {
                                        str_path = this->fileSystemData.paths.at(id);
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

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Volume Information Response");
                                    }

                                }
                                break;

                            case rdpdr::IRP_MJ_WRITE:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Request");
                                }
                                {
                                    rdpdr::DeviceWriteRequest dwr;
                                    dwr.receive(chunk);

                                    size_t WriteDataLen(dwr.Length);

                                    if (dwr.Length > CHANNELS::CHANNEL_CHUNK_LENGTH) {

                                        this->fileSystemData.writeData_to_wait = dwr.Length - rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
                                        this->fileSystemData.file_to_write_id = id;
                                        WriteDataLen = rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
                                    }

                                    std::string file_to_write = this->fileSystemData.paths.at(id);

                                    if (this->impl_io_disk->write_file(file_to_write.c_str(), reinterpret_cast<const char *>(dwr.WriteData), WriteDataLen ) ) {
                                        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_write.c_str());
                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                    }

                                    deviceIOResponse.emit(out_stream);
                                    rdpdr::DeviceWriteResponse dwrp(dwr.Length);
                                    dwrp.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Response");
                                    }
                                }

                                break;

                            case rdpdr::IRP_MJ_SET_INFORMATION:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Server Drive Set Information Request");
                                }
                                {
                                    rdpdr::ServerDriveSetInformationRequest sdsir;
                                    sdsir.receive(chunk);

                                    std::string file_to_request = this->fileSystemData.paths.at(id);

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

                                            std::string fileName(this->client->SHARE_DIR + rdpfri.FileName());

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

                                            std::string file_to_request = this->fileSystemData.paths.at(id);

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

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                                    if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Set Information Response");
                                    }
                                }
                                break;

                            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Control Request");
                                }
                                {
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

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                }
                                if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Client Drive Control Response");
                                }
                                break;

                            default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request unknow MajorFunction = %x",       deviceIORequest.MajorFunction());
                                break;
                        }

                        if (deviceIORequest.DeviceId() ==  2) {
                            this->verbose = RDPVerbose::none;
                        }

                        } break;

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
                            this->fileSystemData.fileSystemCapacity[type] = true;
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
                            this->fileSystemData.drives_created = true;
                        } else {
                            this->fileSystemData.drives_created = false;
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

    void FremoveDriveDevice(const FileSystemData::DeviceData * devices, const size_t deviceCount) {
        StaticOutStream<1024> out_stream;

        rdpdr::SharedHeader sharedHeader( rdpdr::RDPDR_CTYP_CORE
                                        , rdpdr::PAKID_CORE_DEVICELIST_REMOVE);
        sharedHeader.emit(out_stream);
        out_stream.out_uint32_le(deviceCount);
        for (size_t i = 0; i < deviceCount; i++) {
            out_stream.out_uint32_le(devices[i].ID);
        }

        int total_length(out_stream.get_offset());
        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

        this->client->mod->send_to_mod_channel( channel_names::rdpdr
                                            , chunk_to_send
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                            CHANNELS::CHANNEL_FLAG_FIRST
                                            );
        if (bool(this->verbose & RDPVerbose::rdpdr)) {
            LOG(LOG_INFO, "CLIENT >> RDPDR: Client Drive Device List Remove");
        }
    }

    void process_client_clipboard_out_data(const CHANNELS::ChannelNameId & front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  uint8_t const * data, const size_t data_len, uint32_t flags){

        // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Dat

        // Virtual channel data can span multiple Virtual Channel PDUs (section 3.1.5.2.1).
        // If this is the case, the embedded length field of the channelPduHeader field
        // (the Channel PDU Header structure is specified in section 2.2.6.1.1) specifies
        // the total length of the uncompressed virtual channel data spanned across all of
        // the associated Virtual Channel PDUs. This length is referred to as totalLength.
        // For example, assume that the virtual channel chunking size specified in the Virtual
        // Channel Capability Set (section 2.2.7.1.10) is 1,000 bytes and that 2,062 bytes need
        // to be transmitted on a given virtual channel. In this example,
        // the following sequence of Virtual Channel PDUs will be sent (only relevant fields are listed):

        //    Virtual Channel PDU 1:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_FIRST
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 2:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = 0
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 3:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_LAST
        //    Actual virtual channel data is 62 bytes.

    //     // The size of the virtual channel data in the last PDU (the data in the virtualChannelData field)
        // is determined by subtracting the offset of the virtualChannelData field in the encapsulating
        // Virtual Channel PDU from the total size specified in the tpktHeader field. This length is
        // referred to as chunkLength.

        // Upon receiving each Virtual Channel PDU, the server MUST dispatch the virtual channel data to
        // the appropriate virtual channel endpoint. The sequencing of the chunk (whether it is first,
        // intermediate, or last), totalLength, chunkLength, and the virtualChannelData fields MUST
        // be dispatched to the virtual channel endpoint so that the data can be correctly reassembled.
        // If the CHANNEL_FLAG_SHOW_PROTOCOL (0x00000010) flag is specified in the Channel PDU Header,
        // then the channelPduHeader field MUST also be dispatched to the virtual channel endpoint.

        // A reassembly buffer MUST be created by the virtual channel endpoint using the size specified
        // by totalLength when the first chunk is received. After the reassembly buffer has been created
        // the first chunk MUST be copied into the front of the buffer. Subsequent chunks MUST then be
        // copied into the reassembly buffer in the order in which they are received. Upon receiving the
        // last chunk of virtual channel data, the reassembled data is processed by the virtual channel endpoint.

        if (data_len > first_part_data_size ) {

            int real_total = data_len - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
                out_stream_first_part.out_copy_bytes(data, first_part_data_size);

                data_sent += first_part_data_size;
                InStream chunk_first(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_first
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_FIRST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_first_part.get_data(), out_stream_first_part.get_offset());


            for (int i = 0; i < cmpt_PDU_part; i++) {

            // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_data(), out_stream_next_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_next
                                                    , total_length
                                                    , flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_next_part.get_data(), out_stream_next_part.get_offset());
            }

            // Last part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
                out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);

                data_sent += remains_PDU;
                InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_last
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_last_part.get_data(), out_stream_last_part.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data, data_len);
            InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

            this->client->mod->send_to_mod_channel( front_channel_name
                                                , chunk
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                                  flags
                                                );
        }
    }
};

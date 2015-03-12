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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef REDEMPTION_CORE_RDP_CHANNELS_RDPDRFILESYSTEMDRIVEMANAGER_HPP
#define REDEMPTION_CORE_RDP_CHANNELS_RDPDRFILESYSTEMDRIVEMANAGER_HPP

#include "rdpdr.hpp"
#include "defines.hpp"
#include "FSCC/FileInformation.hpp"

class FileSystemDriveManager {
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;

    typedef std::tuple<uint32_t, std::string, std::string> managed_drive_type;
    typedef std::vector<managed_drive_type>
        managed_drive_collection_type; // DeviceId, name, path
    managed_drive_collection_type managed_drives;

    const char * get_drive_by_id(uint32_t DeviceId) {
        for (managed_drive_collection_type::iterator iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {
            if (DeviceId == std::get<0>(*iter)) {
                return std::get<2>(*iter).c_str();
            }
        }

        throw Error(ERR_RDP_PROTOCOL);
    }

public:
    FileSystemDriveManager() {
/*
        managed_drives.push_back(
            std::make_tuple(this->next_managed_drive_id++,
                            "WABLNCH",
                            DRIVE_REDIRECTION_PATH "/wablnch"
                            ));
*/
    }

    uint32_t AnnounceDrivePartially(Stream & client_device_list_announce) {
        uint32_t announced_drive_count = 0;

        for (managed_drive_collection_type::iterator iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {

            rdpdr::DeviceAnnounceHeader device_announce_header(rdpdr::RDPDR_DTYP_FILESYSTEM,
                                                               std::get<0>(*iter),
                                                               std::get<1>(*iter).c_str(),
                                                               nullptr, 0);

            device_announce_header.emit(client_device_list_announce);

            announced_drive_count++;
        }

        return announced_drive_count;
    }

    bool IsManagedDrive(uint32_t DeviceId) {
        if (DeviceId >= FIRST_MANAGED_DRIVE_ID) {
            for (managed_drive_collection_type::iterator iter = this->managed_drives.begin();
                 iter != this->managed_drives.end(); ++iter) {
                if (DeviceId == std::get<0>(*iter)) {
                    return true;
                }
            }
        }

        return false;
    }

    void ProcessDeviceIORequest(rdpdr::DeviceIORequest const & device_io_request,
                                Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
                                uint32_t verbose) {
        switch (device_io_request.MajorFunction()) {
            case rdpdr::IRP_MJ_CREATE:
            {
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: Device Create Request");
                }

                rdpdr::DeviceCreateRequest device_create_request;

                device_create_request.receive(in_stream);
                if (verbose) {
                    device_create_request.log(LOG_INFO);
                }

                // fake response!!!
                const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                               rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                sh_s.emit(out_stream);

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                device_io_response.emit(out_stream);

                const rdpdr::DeviceCreateResponse device_create_response(2 /*Fake file Id*/,
                                                                         0x0);
                device_create_response.emit(out_stream);

                out_stream.mark_end();
            }
            break;

            case rdpdr::IRP_MJ_CLOSE:
            {
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: Device Close Request");
                }

                // fake response!!!
                const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                               rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                sh_s.emit(out_stream);

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                device_io_response.emit(out_stream);

                // Device Close Response (DR_CLOSE_RSP)
                out_stream.out_clear_bytes(5);  // Padding(5);

                out_stream.mark_end();
            }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
            {
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Information Request");
                }

                rdpdr::ServerDriveQueryInformationRequest
                    server_drive_query_information_request;

                server_drive_query_information_request.receive(in_stream);
                if (verbose) {
                    server_drive_query_information_request.log(LOG_INFO);
                }

                // fake response!!!
                const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                               rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                sh_s.emit(out_stream);

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                device_io_response.emit(out_stream);

                fscc::FileBasicInformation file_basic_information(13931567276093341732ull,
                                                                  12544016655938670606ull,
                                                                  17344912407709899956ull,
                                                                  30426978ull,
                                                                  0x0);
                file_basic_information.emit(out_stream);

                out_stream.mark_end();
            }
            break;

            default:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "undecoded Device I/O Request - MajorFunction=0x%X",
                        device_io_request.MajorFunction());
                }
            break;
        }
    }
};  // FileSystemDriveManager

#endif  // REDEMPTION_CORE_RDP_CHANNELS_RDPDRFILESYSTEMDRIVEMANAGER_HPP

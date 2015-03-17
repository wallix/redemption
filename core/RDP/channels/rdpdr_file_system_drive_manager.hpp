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
#include "SMB2/MessageSyntax.hpp"

#define EPOCH_DIFF 11644473600LL

#define FILE_TIME_SYSTEM_TO_RDP(_t) \
    (((uint64_t)(_t) + EPOCH_DIFF) * 10000000LL)
#define FILE_TIME_RDP_TO_SYSTEM(_t) \
    (((_t) == 0LL || (_t) == (uint64_t)(-1LL)) ? 0 : (time_t)((_t) / 10000000LL - EPOCH_DIFF))

class ManagedFileSystemObject {
public:
    virtual ~ManagedFileSystemObject() = default;

    virtual uint32_t FileId() = 0;

    virtual bool ProcessServerCreateDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceCreateRequest const & device_create_request,
        const char * path, Stream & in_stream, Stream & out_stream,
        uint32_t & out_flags, uint32_t verbose) = 0;

    virtual void ProcessServerCloseDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request, const char * path,
        Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveQueryInformationRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
        const char * path, Stream & in_stream, Stream & out_stream,
        uint32_t & out_flags, uint32_t verbose) = 0;
};

class ManagedDirectory : public ManagedFileSystemObject {
    DIR * dir = nullptr;

public:
    ManagedDirectory() {
LOG(LOG_INFO, ">>>>>>>>>> ManagedDirectory::ManagedDirectory() : <%p>", this);
    }

    virtual ~ManagedDirectory() {
LOG(LOG_INFO, ">>>>>>>>>> ManagedDirectory::~ManagedDirectory(): <%p>", this);

        if (this->dir) {
            ::closedir(this->dir);
        }
    }

    virtual uint32_t FileId() override {
        REDASSERT(this->dir);
        return static_cast<uint32_t>(::dirfd(this->dir));
    }

    virtual bool ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            const char * path, Stream & in_stream, Stream & out_stream,
            uint32_t & out_flags, uint32_t verbose) override {
        REDASSERT(!this->dir);

        this->dir = ::opendir(path);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                (
                 this->dir  ?
                 0x00000000 :   // STATUS_SUCCESS
                 0xC0000001     // STATUS_UNSUCCESSFUL
                )
            );
        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerCreateDriveRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->dir ? ::dirfd(this->dir) : -1),
                0x0
            );
        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerCreateDriveRequest");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        out_stream.mark_end();

        return (this->dir != nullptr);
    }

    virtual void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
            uint32_t verbose) override {
        REDASSERT(this->dir);

        ::closedir(this->dir);

        this->dir = nullptr;

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
            device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerCloseDriveRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        out_stream.mark_end();

        REDASSERT(!this->dir);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, Stream & in_stream, Stream & out_stream,
            uint32_t & out_flags, uint32_t verbose) override {
        REDASSERT(this->dir);

        struct stat sb;
        ::fstat(::dirfd(this->dir), &sb);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
            device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        out_stream.out_uint32_le(fscc::FileBasicInformation::size());   // Length(4)

        fscc::FileBasicInformation file_basic_information(FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                                                          FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                                                          FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                                                          FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                                                          fscc::FILE_ATTRIBUTE_DIRECTORY |
                                                              (sb.st_mode & S_IWUSR ? 0 : fscc::FILE_ATTRIBUTE_READONLY));

        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
            file_basic_information.log(LOG_INFO);
        }
        file_basic_information.emit(out_stream);

        out_stream.mark_end();
    }
};  // ManagedDirectory

class ManagedFile : public ManagedFileSystemObject {
    int fd = -1;

public:
    virtual ~ManagedFile() {
        if (this->fd != -1) {
            ::close(this->fd);
        }
    }

    virtual uint32_t FileId() override {
        REDASSERT(this->fd > -1);
        return static_cast<uint32_t>(this->fd);
    }

    virtual bool ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            const char * path, Stream & in_stream, Stream & out_stream,
            uint32_t & out_flags, uint32_t verbose) override {
        REDASSERT(this->fd == -1);

        return false;
    }

    virtual void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
            uint32_t verbose) {
        REDASSERT(this->fd != -1);

        REDASSERT(this->fd == -1);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, Stream & in_stream, Stream & out_stream,
            uint32_t & out_flags, uint32_t verbose) override {
        REDASSERT(this->fd != -1);
    }
};

class FileSystemDriveManager {
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;

    typedef std::tuple<uint32_t, std::string, std::string> managed_drive_type;
    typedef std::vector<managed_drive_type>
        managed_drive_collection_type; // DeviceId, name, path.
    managed_drive_collection_type managed_drives;

    typedef std::tuple<uint32_t, std::unique_ptr<ManagedFileSystemObject>> managed_file_system_object_type;
    typedef std::vector<managed_file_system_object_type>
        managed_file_system_object_collection_type;   // FileId, object.
    managed_file_system_object_collection_type managed_file_system_objects;

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

private:
    void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
            uint32_t verbose) {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(in_stream);
        if (verbose) {
            device_create_request.log(LOG_INFO);
        }

        std::string fullpath = path;
        fullpath += '/';
        fullpath += device_create_request.Path();

        bool is_directory = false;

        struct stat sb;
        if (::stat(fullpath.c_str(), &sb) == 0) {
            is_directory = ((sb.st_mode & S_IFMT) == S_IFDIR);
        }
        else {
            is_directory = (device_create_request.CreateOptions() & smb2::FILE_DIRECTORY_FILE);
        }

        if (is_directory) {
            std::unique_ptr<ManagedFileSystemObject> managed_file_system_object =
                std::make_unique<ManagedDirectory>();

            if (managed_file_system_object->ProcessServerCreateDriveRequest(
                    device_io_request, device_create_request, path, in_stream,
                    out_stream, out_flags, verbose)) {
                this->managed_file_system_objects.push_back(std::make_tuple(
                    device_io_request.DeviceId(),
                    std::move(managed_file_system_object)
                    ));
            }
        }
        else {
            REDASSERT(false);
        }
    }

    void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
            uint32_t verbose) {
        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.DeviceId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerCloseDriveRequest(
                    device_io_request, path, in_stream, out_stream, out_flags,
                    verbose);
                this->managed_file_system_objects.erase(iter);
                break;
            }
        }
    }

    void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream, Stream & out_stream, uint32_t & out_flags,
            uint32_t verbose) {
        rdpdr::ServerDriveQueryInformationRequest
            server_drive_query_information_request;

        server_drive_query_information_request.receive(in_stream);
        if (verbose) {
            server_drive_query_information_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.DeviceId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveQueryInformationRequest(
                    device_io_request, server_drive_query_information_request, path,
                    in_stream, out_stream, out_flags, verbose);
                break;
            }
        }
    }

public:
    void ProcessDeviceIORequest(
            rdpdr::DeviceIORequest const & device_io_request, Stream & in_stream,
            Stream & out_stream, uint32_t & out_flags, uint32_t verbose) {
        uint32_t DeviceId = device_io_request.DeviceId();
        if (DeviceId < FIRST_MANAGED_DRIVE_ID) {
            return;
        }
        managed_drive_collection_type::iterator iter;
        for (iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {
            if (DeviceId == std::get<0>(*iter)) {
                break;
            }
        }
        if (iter == this->managed_drives.end()) return;

        std::string path = std::get<2>(*iter);

        switch (device_io_request.MajorFunction()) {
            case rdpdr::IRP_MJ_CREATE:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: Server Create Drive Request");
                }

                this->ProcessServerCreateDriveRequest(device_io_request,
                    path.c_str(), in_stream, out_stream, out_flags, verbose);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: Server Close Drive Request");
                }

                this->ProcessServerCloseDriveRequest(device_io_request,
                    path.c_str(), in_stream, out_stream, out_flags, verbose);
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Information Request");
                }

                this->ProcessServerDriveQueryInformationRequest(device_io_request,
                    path.c_str(), in_stream, out_stream, out_flags, verbose);
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

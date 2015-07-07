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

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/statvfs.h>

#include "channel_list.hpp"
#include "fileutils.hpp"
#include "rdp/rdp_asynchronous_task.hpp"
#include "rdpdr.hpp"
#include "defines.hpp"
#include "FSCC/FileInformation.hpp"
#include "make_unique.hpp"
#include "SMB2/MessageSyntax.hpp"
#include "to_server_sender.hpp"
#include "winpr/pattern.hpp"

#define EPOCH_DIFF 11644473600LL

#define FILE_TIME_SYSTEM_TO_RDP(_t) \
    (((uint64_t)(_t) + EPOCH_DIFF) * 10000000LL)
#define FILE_TIME_RDP_TO_SYSTEM(_t) \
    (((_t) == 0LL || (_t) == (uint64_t)(-1LL)) ? 0 : (time_t)((_t) / 10000000LL - EPOCH_DIFF))


class ManagedFileSystemObject {
protected:
    std::string full_path;

    int fd = -1;

    bool delete_pending = false;

public:
    virtual ~ManagedFileSystemObject() = default;

    static inline const char * get_open_flag_name(int flag) {
        switch (flag) {
            case O_RDONLY: return "O_RDONLY";
            case O_WRONLY: return "O_WRONLY";
            case O_RDWR:   return "O_RDWR";
        }

        return "<unknown>";
    }

    inline int FileDescriptor() const {
        REDASSERT(this->fd > -1);

        return this->fd;
    }

    virtual bool IsDirectory() const = 0;

    virtual void ProcessServerCreateDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceCreateRequest const & device_create_request,
        int drive_access_mode, const char * path, Stream & in_stream,
        bool & out_drive_created,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerCloseDriveRequest(
        rdpdr::DeviceIORequest const & device_io_request, const char * path,
        Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveReadRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceReadRequest const & device_read_request,
        const char * path, Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveControlRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceControlRequest const & device_control_request,
        const char * path, Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveQueryVolumeInformationRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryVolumeInformationRequest const &
            server_drive_query_volume_information_request,
        const char * path, Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveQueryInformationRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
        const char * path, Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

    virtual void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveSetInformationRequest const & server_drive_set_information_request,
            const char * path, int drive_access_mode, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->fd > -1);

        if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
            MakeClientDriveIoResponse(device_io_request,
                                      "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                                      0xC000000D,   // STATUS_INVALID_PARAMETER
                                      to_server_sender,
                                      out_asynchronous_task,
                                      verbose
                                     );

            return;
        }

        switch (server_drive_set_information_request.FsInformationClass())
        {
            case rdpdr::FileBasicInformation:
            {
                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }

                struct timeval times[2] = { { 0, 0 }, { 0, 0 } };

                auto file_time_rdp_to_system_timeval = [](uint64_t rdp_time, timeval & out_system_tiem) {
                    if ((rdp_time == 0LL) || (rdp_time == ((uint64_t)-1LL))) {
                        out_system_tiem.tv_sec  = 0;
                        out_system_tiem.tv_usec = 0;
                    }
                    else {
                        out_system_tiem.tv_sec  = (time_t)(rdp_time / 10000000LL - EPOCH_DIFF);
                        out_system_tiem.tv_usec = rdp_time % 10000000LL;
                    }
                };

                file_time_rdp_to_system_timeval(file_basic_information.LastAccessTime(), times[0]);
                file_time_rdp_to_system_timeval(file_basic_information.LastWriteTime(),  times[1]);

                ::futimes(this->fd, times);

                struct stat64 sb;
                ::fstat64(this->fd, &sb);

                const mode_t mode =
                    ((file_basic_information.FileAttributes() & fscc::FILE_ATTRIBUTE_READONLY) ?
                     (sb.st_mode & (~S_IWUSR)) :
                     (sb.st_mode | S_IWUSR)
                    );
                ::chmod(this->full_path.c_str(), mode);

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileEndOfFileInformation:
            {
                int64_t EndOfFile = in_stream.in_sint64_le();

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "EndOfFile=%" PRId64,
                        EndOfFile);
                }

//                ::posix_fallocate(this->fd, 0, EndOfFile);
                ::ftruncate(this->fd, EndOfFile);

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileDispositionInformation:
                this->delete_pending = true;

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "DeletePending=yes");
                }

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            break;

            case rdpdr::FileRenameInformation:
            {
                rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                rdp_file_rename_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    rdp_file_rename_information.log(LOG_INFO);
                }

                REDASSERT(!rdp_file_rename_information.RootDirectory());

                std::string new_full_path(path);
                new_full_path += rdp_file_rename_information.FileName();

                if (!::access(new_full_path.c_str(), F_OK)) {
                    if (!rdp_file_rename_information.replace_if_exists()) {
                        MakeClientDriveIoResponse(device_io_request,
                                                  "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                                                  (this->IsDirectory() ?
                                                   0xC0000033 :  // STATUS_OBJECT_NAME_INVALID
                                                   0xC0000035    // STATUS_OBJECT_NAME_COLLISION
                                                  ),
                                                  to_server_sender,
                                                  out_asynchronous_task,
                                                  verbose
                                                 );

                        return;
                    }
                }

                ::rename(this->full_path.c_str(), new_full_path.c_str());

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileAllocationInformation:
            {
                int64_t AllocationSize = in_stream.in_sint64_le();

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "AllocationSize=%" PRId64,
                        AllocationSize);
                }

                ::ftruncate(this->fd, AllocationSize);

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                        "Unknown FsInformationClass - %s(0x%X)",
                    server_drive_set_information_request.get_FsInformationClass_name(
                        server_drive_set_information_request.FsInformationClass()),
                    server_drive_set_information_request.FsInformationClass());
                REDASSERT(false);

                MakeClientDriveIoUnsuccessfulResponse(device_io_request,
                                                      "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                                                      to_server_sender,
                                                      out_asynchronous_task,
                                                      verbose);

                // Unsupported.
                REDASSERT(false);
            break;
        }

    }

    virtual void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode,
            bool first_chunk, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) = 0;

    virtual void ProcessServerDriveQueryDirectoryRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
        const char * path, Stream & in_stream,
        ToServerSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        uint32_t verbose) = 0;

protected:
    static void MakeClientDriveIoResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            uint32_t IoStatus,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        BStream out_stream(65536);

        const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                               );
        shared_header.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                         device_io_request.CompletionId(),
                                                         IoStatus
                                                        );
        if (verbose) {
            LOG(LOG_INFO, message);
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

public:
    static inline void MakeClientDriveIoUnsuccessfulResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        MakeClientDriveIoResponse(device_io_request,
                                  message,
                                  0xC0000001, // STATUS_UNSUCCESSFUL
                                  to_server_sender,
                                  out_asynchronous_task,
                                  verbose);
    }
};  // ManagedFileSystemObject

class ManagedDirectory : public ManagedFileSystemObject {
    DIR * dir = nullptr;

    std::string pattern;

public:
    ManagedDirectory() {
        //LOG(LOG_INFO, "ManagedDirectory::ManagedDirectory() : <%p>", this);
    }

    virtual ~ManagedDirectory() {
        //LOG(LOG_INFO, "ManagedDirectory::~ManagedDirectory(): <%p> fd=%d",
        //    this, (this->dir ? ::dirfd(this->dir) : -1));

        if (this->dir) {
            ::closedir(this->dir);
        }

        // Not yet supported.
//REDASSERT(!this->delete_pending);
        if (this->delete_pending) {
            ::recursive_delete_directory(this->full_path.c_str());
        }
    }

    virtual bool IsDirectory() const override { return true; }

    virtual void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, Stream & in_stream,
            bool & out_drive_created,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(!this->dir);

        out_drive_created = false;

        this->full_path = path;
        this->full_path += device_create_request.Path();

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                this, this->full_path.c_str(), get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        const uint32_t DesiredAccess = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const int last_error = [] (const char * path,
                                   uint32_t DesiredAccess,
                                   uint32_t CreateDisposition,
                                   int drive_access_mode,
                                   DIR *& out_dir) -> int {
            if (((drive_access_mode != O_RDWR) && (drive_access_mode != O_RDONLY) &&
                 smb2::read_access_is_required(DesiredAccess, /*strict_check = */false)) ||
                ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY) &&
                 smb2::write_access_is_required(DesiredAccess, /*strict_check = */false))) {
                out_dir = nullptr;
                return EACCES;
            }

            if ((::access(path, F_OK) != 0) &&
                (CreateDisposition == smb2::FILE_CREATE)) {
                if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
                    out_dir = nullptr;
                    return EACCES;
                }

                ::mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP);
            }

            out_dir = ::opendir(path);
            return ((out_dir != nullptr) ? 0 : errno);
        } (full_path.c_str(), DesiredAccess, CreateDisposition, drive_access_mode, this->dir);

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: <%p> dir=<%p> FileId=%d errno=%d",
                this, this->dir, (this->dir ? ::dirfd(this->dir) : -1), (this->dir ? 0 : last_error));
        }

        const uint32_t IoStatus = [] (const DIR * const dir, int last_error) -> uint32_t {
            if (dir) { return 0x00000000 /* STATUS_SUCCESS */; }

            switch (last_error) {
                case ENOENT:
                    return 0xC000000F;  // STATUS_NO_SUCH_FILE

                case EACCES:
                    return 0xC0000022;  // STATUS_ACCESS_DENIED
            }

            return 0xC0000001;  // STATUS_UNSUCCESSFUL
        } (this->dir, last_error);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
            device_io_request.CompletionId(), IoStatus);
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

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);

        //if (this->dir) {
        //    LOG(LOG_INFO, "ManagedDirectory::ProcessServerCreateDriveRequest(): <%p> fd=%d",
        //        this, ::dirfd(this->dir));
        //}

        if (this->dir != nullptr) {
            this->fd = ::dirfd(this->dir);

            out_drive_created = true;
        }
    }

    virtual void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->dir);
        //LOG(LOG_INFO, "ManagedDirectory::ProcessServerCloseDriveRequest(): <%p> fd=%d",
        //    this, ::dirfd(this->dir));

        ::closedir(this->dir);

        this->dir = nullptr;

        BStream out_stream(65536);

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

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);

        REDASSERT(!this->dir);
    }

    virtual void ProcessServerDriveReadRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->dir);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(
                device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                0x00000000  // STATUS_SUCCESS
            );
        if (verbose) {
            LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveReadRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        out_stream.out_uint32_le(0);    // Length(4)

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryVolumeInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryVolumeInformationRequest const &
                server_drive_query_volume_information_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->dir);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        switch (server_drive_query_volume_information_request.FsInformationClass()) {
            case rdpdr::FileFsVolumeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsVolumeInformation file_fs_volume_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),   // VolumeCreationTime(8)
                        svfsb.f_fsid,                           // VolumeSerialNumber(4)
                        1,                                      // SupportsObjects(1) - FALSE
                        "REDEMPTION"
                    );

                out_stream.out_uint32_le(file_fs_volume_information.size());    // Length(4)

                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_volume_information.log(LOG_INFO);
                }
                file_fs_volume_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsSizeInformation file_fs_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // AvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_size_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_size_information.log(LOG_INFO);
                }
                file_fs_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsAttributeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsAttributeInformation file_fs_attribute_information(
                        fscc::FILE_CASE_SENSITIVE_SEARCH |      // FileSystemAttributes(4)
                            fscc::FILE_CASE_PRESERVED_NAMES |
                            //fscc::FILE_READ_ONLY_VOLUME |
                            fscc::FILE_UNICODE_ON_DISK,
                        svfsb.f_namemax,                        // MaximumComponentNameLength(4)
                        "FAT32"                                 // FileSystemName(variable)
                    );

                out_stream.out_uint32_le(file_fs_attribute_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_attribute_information.log(LOG_INFO);
                }
                file_fs_attribute_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsFullSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsFullSizeInformation file_fs_full_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // CallerAvailableAllocationUnits(8)
                        svfsb.f_bfree,  // ActualAvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_full_size_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_full_size_information.log(LOG_INFO);
                }
                file_fs_full_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsDeviceInformation:
            {
                LOG(LOG_INFO, "+ + + + + + + + + + ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest() - FileFsDeviceInformation - Using ToServerSender + + + + + + + + + +");
                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsDeviceInformation file_fs_device_information(
                        fscc::FILE_DEVICE_DISK, 0
                    );

                out_stream.out_uint32_le(file_fs_device_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_device_information.log(LOG_INFO);
                }
                file_fs_device_information.emit(out_stream);
            }
            break;

            default:
            {
                LOG(LOG_ERR,
                    "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_volume_information_request.FsInformationClass());

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0xC0000001 /* STATUS_UNSUCCESSFUL*/);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);
            }
            break;
        }

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveControlRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->dir);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(
                device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                0x00000000  // STATUS_SUCCESS
            );
        if (verbose) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerDriveControlRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->dir);

        BStream out_stream;

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        struct stat64 sb;
        ::fstat64(::dirfd(this->dir), &sb);

        switch (server_drive_query_information_request.FsInformationClass()) {
            case rdpdr::FileBasicInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileBasicInformation::size());   // Length(4)

                fscc::FileBasicInformation file_basic_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // CreationTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),                           // LastAccessTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // LastWriteTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),                           // ChangeTime(8)
                        fscc::FILE_ATTRIBUTE_DIRECTORY |                                // FileAttributes(4)
                            (sb.st_mode & S_IWUSR ? 0 : fscc::FILE_ATTRIBUTE_READONLY)
                    );

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }
                file_basic_information.emit(out_stream);
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileStandardInformation::size());    // Length(4)

                fscc::FileStandardInformation file_standard_information(
                        sb.st_blocks * 512 /* Block size */,    // AllocationSize
                        sb.st_size,                             // EndOfFile
                        sb.st_nlink,                            // NumberOfLinks
                        0,                                      // DeletePending
                        0                                       // Directory
                    );

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    file_standard_information.log(LOG_INFO);
                }
                file_standard_information.emit(out_stream);
            }
            break;

            case rdpdr::FileAttributeTagInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileAttributeTagInformation::size());    // Length(4)

                fscc::FileAttributeTagInformation file_attribute_tag_information(
                        fscc::FILE_ATTRIBUTE_DIRECTORY |                                    // FileAttributes
                            (sb.st_mode & S_IWUSR ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        0                                                                   // ReparseTag
                    );

                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    file_attribute_tag_information.log(LOG_INFO);
                }
                file_attribute_tag_information.emit(out_stream);
            }
            break;

            default:
            {
                LOG(LOG_ERR,
                    "ManagedDirectory::ProcessServerDriveQueryInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_information_request.FsInformationClass());
                REDASSERT(false);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0xC0000001  // STATUS_UNSUCCESSFUL
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedDirectory::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);
            }
            break;
        }

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

/*
    virtual void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveSetInformationRequest const & server_drive_set_information_request,
            const char * path, int drive_access_mode, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->dir);

        if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
            MakeClientDriveIoResponse(device_io_request,
                                      "ManagedDirectory::ProcessServerDriveSetInformationRequest",
                                      0xC000000D,   // STATUS_INVALID_PARAMETER
                                      to_server_sender,
                                      out_asynchronous_task,
                                      verbose
                                     );

            return;
        }

        switch (server_drive_set_information_request.FsInformationClass())
        {
            case rdpdr::FileBasicInformation:
            {
                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveSetInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }

                struct timeval times[2] = { { 0, 0 }, { 0, 0 } };

                auto file_time_rdp_to_system_timeval = [](uint64_t rdp_time, timeval & out_system_tiem) {
                    if ((rdp_time == 0LL) || (rdp_time == ((uint64_t)-1LL))) {
                        out_system_tiem.tv_sec  = 0;
                        out_system_tiem.tv_usec = 0;
                    }
                    else {
                        out_system_tiem.tv_sec  = (time_t)(rdp_time / 10000000LL - EPOCH_DIFF);
                        out_system_tiem.tv_usec = rdp_time % 10000000LL;
                    }
                };

                file_time_rdp_to_system_timeval(file_basic_information.LastAccessTime(), times[0]);
                file_time_rdp_to_system_timeval(file_basic_information.LastWriteTime(),  times[1]);

                ::futimes(::dirfd(this->dir), times);

                struct stat64 sb;
                ::fstat64(::dirfd(this->dir), &sb);

                const mode_t mode =
                    ((file_basic_information.FileAttributes() & fscc::FILE_ATTRIBUTE_READONLY) ?
                     (sb.st_mode & (~S_IWUSR)) :
                     (sb.st_mode | S_IWUSR)
                    );
                ::chmod(this->full_path.c_str(), mode);

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFile::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileRenameInformation:
            {
                rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                rdp_file_rename_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveSetInformationRequest");
                    rdp_file_rename_information.log(LOG_INFO);
                }

                REDASSERT(!rdp_file_rename_information.RootDirectory());

                std::string new_full_path(path);
                new_full_path += rdp_file_rename_information.FileName();

                if (!::access(new_full_path.c_str(), F_OK)) {
                    if (!rdp_file_rename_information.replace_if_exists()) {
                        MakeClientDriveIoResponse(device_io_request,
                                                  "ManagedDirectory::ProcessServerDriveSetInformationRequest",
                                                  0xC0000035,   // STATUS_OBJECT_NAME_COLLISION
                                                  to_server_sender,
                                                  out_asynchronous_task,
                                                  verbose
                                                 );

                        return;
                    }
                }

                ::rename(this->full_path.c_str(), new_full_path.c_str());

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileDispositionInformation:
            {
                this->delete_pending = true;

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedDirectory::ProcessServerDriveSetInformationRequest: "
                        "Unknown FsInformationClass - %s(0x%X)",
                    server_drive_set_information_request.get_FsInformationClass_name(
                        server_drive_set_information_request.FsInformationClass()),
                    server_drive_set_information_request.FsInformationClass());
                REDASSERT(false);

                MakeClientDriveIoUnsuccessfulResponse(device_io_request,
                                                      "ManagedDirectory::ProcessServerDriveSetInformationRequest",
                                                      to_server_sender,
                                                      out_asynchronous_task,
                                                      verbose);

                // Unsupported.
                REDASSERT(false);
            break;
        }
    }
*/

    virtual void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode, bool first_chunk,
            Stream & in_stream, ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        MakeClientDriveIoUnsuccessfulResponse(
            device_io_request,
            "ManagedDirectory::ProcessServerDriveWriteRequest",
            to_server_sender,
            out_asynchronous_task,
            verbose);

        // Unsupported.
        REDASSERT(false);
    }

    virtual void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        if (server_drive_query_directory_request.InitialQuery()) {
            ::rewinddir(this->dir);

            const char * separator = strrchr(server_drive_query_directory_request.Path(), '/');
            REDASSERT(separator);
            this->pattern = (++separator);
        }

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                    "full_path=\"%s\" pattern=\"%s\"",
                this->full_path.c_str(), this->pattern.c_str());
        }

        struct dirent * ent = nullptr;

        do {
            TODO("Non reentrant function 'readdir' called. For threadsafe applications it is recommended to use the reentrant replacement function 'readdir_r'");
            ent = ::readdir(this->dir);
            if (!ent) { break; }

            if (::FilePatternMatchA(ent->d_name, this->pattern.c_str()))
                break;
        }
        while (ent);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        if (!ent) {
            const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                device_io_request.CompletionId(), 0x80000006 /* STATUS_NO_MORE_FILES */);
            if (verbose) {
                LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            out_stream.out_uint32_le(0);    // Length(4)
            out_stream.out_clear_bytes(1);  // Padding(1)
        }
        else {
            std::string file_full_path = this->full_path;
            if ((file_full_path.back() != '/') && (ent->d_name[0] != '/')) {
                file_full_path += '/';
            }
            file_full_path += ent->d_name;
            if (verbose) {
                LOG(LOG_INFO,
                    "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                        "<%p> full_path=\"%s\"",
                    this, file_full_path.c_str());
            }

            struct stat64 sb;

            ::stat64(file_full_path.c_str(), &sb);

            switch (server_drive_query_directory_request.FsInformationClass()) {
                case rdpdr::FileFullDirectoryInformation:
                {
                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                        device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    const fscc::FileFullDirectoryInformation file_full_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        (S_ISDIR(sb.st_mode) ? fscc::FILE_ATTRIBUTE_DIRECTORY : 0) |
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        ent->d_name
                        );
                    if (verbose) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_full_directory_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_full_directory_information.size());   // Length(4)

                    file_full_directory_information.emit(out_stream);
                }
                break;

                case rdpdr::FileBothDirectoryInformation:
                {
                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                        device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    const fscc::FileBothDirectoryInformation file_both_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        (S_ISDIR(sb.st_mode) ? fscc::FILE_ATTRIBUTE_DIRECTORY : 0) |
                            ((sb.st_mode & S_IWUSR) ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        ent->d_name
                        );
                    if (verbose) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_both_directory_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_both_directory_information.size());   // Length(4)

                    file_both_directory_information.emit(out_stream);
                }
                break;

                case rdpdr::FileNamesInformation:
                {
                    LOG(LOG_INFO, "+ + + + + + + + + + ManagedDirectory::ProcessServerDriveQueryDirectoryRequest() - FileNamesInformation - Using ToServerSender + + + + + + + + + +");
                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                        device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    const fscc::FileNamesInformation file_name_information(ent->d_name);
                    if (verbose) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_name_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_name_information.size()); // Length(4)

                    file_name_information.emit(out_stream);
                }
                break;

                default:
                {
                    LOG(LOG_ERR,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                            "Unknown FsInformationClass(0x%X)",
                        server_drive_query_directory_request.FsInformationClass());
                    REDASSERT(false);

                    const rdpdr::DeviceIOResponse device_io_response(
                            device_io_request.DeviceId(),
                            device_io_request.CompletionId(),
                            0xC0000001  // STATUS_UNSUCCESSFUL
                        );
                    if (verbose) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);
                }
                break;
            }
        }

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }
};  // ManagedDirectory

class ManagedFile : public ManagedFileSystemObject {
    std::unique_ptr<InFileTransport> in_file_transport; // For read operations only.

    off64_t size = 0;

public:
    ManagedFile() {
        //LOG(LOG_INFO, "ManagedFile::ManagedFile(): <%p>", this);
    }

    virtual ~ManagedFile() {
        //LOG(LOG_INFO, "ManagedFile::~ManagedFile(): <%p> fd=%d",
        //    this, this->fd);

        // File descriptor will be closed when in_file_transport is destroyed.

        REDASSERT((this->fd <= -1) || in_file_transport);

        if (this->delete_pending) {
            ::unlink(this->full_path.c_str());
        }
    }

    virtual bool IsDirectory() const override { return false; }

    virtual void ProcessServerCreateDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, Stream & in_stream,
            bool & out_drive_created,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->fd == -1);

        out_drive_created = false;

        this->full_path = path;
        this->full_path += device_create_request.Path();

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerCreateDriveRequest: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                this, this->full_path.c_str(),
                get_open_flag_name(drive_access_mode), drive_access_mode);
        }

        const uint32_t DesiredAccess     = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const int last_error = [] (const char * path,
                                   uint32_t DesiredAccess,
                                   uint32_t CreateDisposition,
                                   int drive_access_mode,
                                   void * log_this,
                                   uint32_t verbose,
                                   int & out_fd,
                                   off64_t & out_size) -> int {
            out_fd = -1;
            out_size = 0;

            if (((drive_access_mode != O_RDWR) && (drive_access_mode != O_RDONLY) &&
                 smb2::read_access_is_required(DesiredAccess, /*strict_check = */false)) ||
                ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY) &&
                 smb2::write_access_is_required(DesiredAccess, /*strict_check = */false))) {
                return EACCES;
            }

            int open_flags = O_LARGEFILE;

            const bool strict_check = true;

            if (smb2::read_access_is_required(DesiredAccess, strict_check) &&
                smb2::write_access_is_required(DesiredAccess, strict_check)) {
                open_flags |= O_RDWR;
            }
            else if (smb2::write_access_is_required(DesiredAccess, strict_check)) {
                open_flags |= O_WRONLY;
            }
            else/* if (smb2::read_access_is_required(DesiredAccess, strict_check))*/ {
                open_flags |= O_RDONLY;
            }

            if ((DesiredAccess & smb2::FILE_APPEND_DATA) &&
                !(DesiredAccess & smb2::FILE_WRITE_DATA)) {
                open_flags |= O_APPEND;
            }

            if (CreateDisposition == smb2::FILE_SUPERSEDE) {
                open_flags |= (O_TRUNC | O_CREAT);
            }
            else if (CreateDisposition == smb2::FILE_CREATE) {
                open_flags |= (O_CREAT | O_EXCL);
            }
            else if (CreateDisposition == smb2::FILE_OPEN_IF) {
                open_flags |= O_CREAT;
            }
            else if (CreateDisposition == smb2::FILE_OVERWRITE) {
                open_flags |= O_TRUNC;
            }
            else if (CreateDisposition == smb2::FILE_OVERWRITE_IF) {
                open_flags |= (O_TRUNC | O_CREAT);
            }

            if (verbose) {
                LOG(LOG_INFO,
                    "ManagedFile::ProcessServerCreateDriveRequest: <%p> open_flags=0x%X",
                    log_this, open_flags);
            }

            out_fd = ::open(path, open_flags, S_IRUSR | S_IWUSR | S_IRGRP);
            if (out_fd > -1) {
                struct stat64 st;
                if (fstat64(out_fd, &st)) {
                    ::close(out_fd);
                    out_fd = -1;
                }
                else {
                    out_size = st.st_size;
                }
            }
            return ((out_fd > -1) ? 0 : errno);
        } (this->full_path.c_str(), DesiredAccess, CreateDisposition, drive_access_mode,
           this, verbose, this->fd, this->size);

        if (this->fd > -1) {
            this->in_file_transport = std::make_unique<InFileTransport>(this->fd);
        }

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerCreateDriveRequest: <%p> FileId=%d errno=%d",
                this, this->fd, ((this->fd == -1) ? last_error : 0));
        }

        const uint32_t IoStatus = [] (int fd, int last_error) -> uint32_t {
            if (fd > -1) { return 0x00000000 /* STATUS_SUCCESS */; }

            switch (last_error) {
                case ENOENT:
                    return 0xC000000F;  // STATUS_NO_SUCH_FILE

                case EACCES:
                    return 0xC0000022;  // STATUS_ACCESS_DENIED
            }

            return 0xC0000001;  // STATUS_UNSUCCESSFUL
        } (this->fd, last_error);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
            device_io_request.CompletionId(), IoStatus);
        if (verbose) {
            LOG(LOG_INFO, "ManagedFile::ProcessServerCreateDriveRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->fd),
                0x0
            );
        if (verbose) {
            LOG(LOG_INFO, "ManagedFile::ProcessServerCreateDriveRequest");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);

        //if (this->fd > -1) {
        //    LOG(LOG_INFO, "ManagedFile::ProcessServerCreateDriveRequest(): <%p> fd=%d",
        //        this, this->fd);
        //}

        out_drive_created = (this->fd != -1);
    }   // ProcessServerCreateDriveRequest

    virtual void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->fd > -1);
        //LOG(LOG_INFO, "ManagedFile::ProcessServerCloseDriveRequest(): <%p> fd=%d",
        //    this, this->fd);

        ::close(this->fd);

        this->fd = -1;

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
            device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
        if (verbose) {
            LOG(LOG_INFO, "ManagedFile::ProcessServerCloseDriveRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);

        REDASSERT(this->fd == -1);
    }

    virtual void ProcessServerDriveReadRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->fd > -1);

        const uint32_t Length = device_read_request.Length();

        const uint64_t Offset = device_read_request.Offset();

        off64_t remaining_number_of_bytes_to_read = std::min<off64_t>(this->size - Offset, Length);


        out_asynchronous_task = std::make_unique<RdpdrDriveReadTask>(this->in_file_transport.get(),
            this->fd, device_io_request.DeviceId(), device_io_request.CompletionId(),
            static_cast<uint32_t>(remaining_number_of_bytes_to_read), Offset, to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveControlRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->fd > -1);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(
                device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                0x00000000  // STATUS_SUCCESS
            );
        if (verbose) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveControlRequest");
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);

        if (verbose) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryVolumeInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryVolumeInformationRequest const &
                server_drive_query_volume_information_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->fd > -1);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        switch (server_drive_query_volume_information_request.FsInformationClass()) {
            case rdpdr::FileFsVolumeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsVolumeInformation file_fs_volume_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),   // VolumeCreationTime(8)
                        svfsb.f_fsid,                           // VolumeSerialNumber(4)
                        1,                                      // SupportsObjects(1) - FALSE
                        "REDEMPTION"
                    );

                out_stream.out_uint32_le(file_fs_volume_information.size());    // Length(4)

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_volume_information.log(LOG_INFO);
                }
                file_fs_volume_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsSizeInformation:
            {
                LOG(LOG_INFO, "+ + + + + + + + + + ManagedFile::ProcessServerDriveQueryVolumeInformationRequest() - FileFsSizeInformation - Using ToServerSender + + + + + + + + + +");
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsSizeInformation file_fs_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // AvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_size_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_size_information.log(LOG_INFO);
                }
                file_fs_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsAttributeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsAttributeInformation file_fs_attribute_information(
                        fscc::FILE_CASE_SENSITIVE_SEARCH |      // FileSystemAttributes(4)
                            fscc::FILE_CASE_PRESERVED_NAMES |
                            //fscc::FILE_READ_ONLY_VOLUME |
                            fscc::FILE_UNICODE_ON_DISK,
                        svfsb.f_namemax,                        // MaximumComponentNameLength(4)
                        "FAT32"                                 // FileSystemName(variable)
                    );

                out_stream.out_uint32_le(file_fs_attribute_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_attribute_information.log(LOG_INFO);
                }
                file_fs_attribute_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsFullSizeInformation:
            {
                LOG(LOG_INFO, "+ + + + + + + + + + ManagedFile::ProcessServerDriveQueryVolumeInformationRequest() - FileFsFullSizeInformation - Using ToServerSender + + + + + + + + + +");
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsFullSizeInformation file_fs_full_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // CallerAvailableAllocationUnits(8)
                        svfsb.f_bfree,  // ActualAvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_full_size_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_full_size_information.log(LOG_INFO);
                }
                file_fs_full_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsDeviceInformation:
            {
                LOG(LOG_INFO, "+ + + + + + + + + + ManagedFile::ProcessServerDriveQueryVolumeInformationRequest() - FileFsDeviceInformation - Using ToServerSender + + + + + + + + + +");
                const rdpdr::DeviceIOResponse device_io_response(
                        device_io_request.DeviceId(),
                        device_io_request.CompletionId(),
                        0x00000000                          // STATUS_SUCCESS
                    );
                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                const fscc::FileFsDeviceInformation file_fs_device_information(
                        fscc::FILE_DEVICE_DISK, 0
                    );

                out_stream.out_uint32_le(file_fs_device_information.size()); // Length(4)

                if (verbose) {
                    LOG(LOG_INFO,
                        "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_device_information.log(LOG_INFO);
                }
                file_fs_device_information.emit(out_stream);
            }
            break;

            default:
            {
                LOG(LOG_ERR,
                    "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_volume_information_request.FsInformationClass());

                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0xC0000001 /* STATUS_UNSUCCESSFUL*/);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryVolumeInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);
            }
            break;
        }

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->fd > -1);

        BStream out_stream(65536);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
        sh_s.emit(out_stream);

        struct stat64 sb;
        ::fstat64(this->fd, &sb);

        switch (server_drive_query_information_request.FsInformationClass()) {
            case rdpdr::FileBasicInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileBasicInformation::size());   // Length(4)

                fscc::FileBasicInformation file_basic_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                       // CreationTime
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),                       // LastAccessTime
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                       // LastWriteTime
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),                       // ChangeTime
                        (sb.st_mode & S_IWUSR ? 0 : fscc::FILE_ATTRIBUTE_READONLY)  // FileAttributes
                    );

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }
                file_basic_information.emit(out_stream);
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileStandardInformation::size());    // Length(4)

                fscc::FileStandardInformation file_standard_information(
                        sb.st_blocks * 512 /* Block size */,    // AllocationSize
                        sb.st_size,                             // EndOfFile
                        sb.st_nlink,                            // NumberOfLinks
                        0,                                      // DeletePending
                        0                                       // Directory
                    );

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    file_standard_information.log(LOG_INFO);
                }
                file_standard_information.emit(out_stream);
            }
            break;

            case rdpdr::FileAttributeTagInformation:
            {
                const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                    device_io_request.CompletionId(), 0x00000000 /* STATUS_SUCCESS */);
                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    device_io_response.log(LOG_INFO);
                }
                device_io_response.emit(out_stream);

                out_stream.out_uint32_le(fscc::FileAttributeTagInformation::size());    // Length(4)

                fscc::FileAttributeTagInformation file_attribute_tag_information(
                        fscc::FILE_ATTRIBUTE_DIRECTORY |                                    // FileAttributes
                            (sb.st_mode & S_IWUSR ? 0 : fscc::FILE_ATTRIBUTE_READONLY),
                        0                                                                   // ReparseTag
                    );

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveQueryInformationRequest");
                    file_attribute_tag_information.log(LOG_INFO);
                }
                file_attribute_tag_information.emit(out_stream);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFile::ProcessServerDriveQueryInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_information_request.FsInformationClass());
                throw Error(ERR_RDP_PROTOCOL);
            //break;
        }

        out_stream.mark_end();

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
            verbose);
    }

/*
    virtual void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveSetInformationRequest const & server_drive_set_information_request,
            const char * path, int drive_access_mode, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        REDASSERT(this->fd > -1);

        if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
            MakeClientDriveIoResponse(device_io_request,
                                      "ManagedFile::ProcessServerDriveSetInformationRequest",
                                      0xC000000D,   // STATUS_INVALID_PARAMETER
                                      to_server_sender,
                                      out_asynchronous_task,
                                      verbose
                                     );

            return;
        }

        switch (server_drive_set_information_request.FsInformationClass())
        {
            case rdpdr::FileBasicInformation:
            {
                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveSetInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }

                struct timeval times[2] = { { 0, 0 }, { 0, 0 } };

                auto file_time_rdp_to_system_timeval = [](uint64_t rdp_time, timeval & out_system_tiem) {
                    if ((rdp_time == 0LL) || (rdp_time == ((uint64_t)-1LL))) {
                        out_system_tiem.tv_sec  = 0;
                        out_system_tiem.tv_usec = 0;
                    }
                    else {
                        out_system_tiem.tv_sec  = (time_t)(rdp_time / 10000000LL - EPOCH_DIFF);
                        out_system_tiem.tv_usec = rdp_time % 10000000LL;
                    }
                };

                file_time_rdp_to_system_timeval(file_basic_information.LastAccessTime(), times[0]);
                file_time_rdp_to_system_timeval(file_basic_information.LastWriteTime(),  times[1]);

                ::futimes(this->fd, times);

                struct stat64 sb;
                ::stat64(this->full_path.c_str(), &sb);

                const mode_t mode =
                    ((file_basic_information.FileAttributes() & fscc::FILE_ATTRIBUTE_READONLY) ?
                     (sb.st_mode & (~S_IWUSR)) :
                     (sb.st_mode | S_IWUSR)
                    );
                ::chmod(this->full_path.c_str(), mode);

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFile::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            case rdpdr::FileRenameInformation:
            {
                rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                rdp_file_rename_information.receive(in_stream);

                if (verbose) {
                    LOG(LOG_INFO, "ManagedFile::ProcessServerDriveSetInformationRequest");
                    rdp_file_rename_information.log(LOG_INFO);
                }

                REDASSERT(!rdp_file_rename_information.RootDirectory());

                std::string new_full_path(path);
                new_full_path += rdp_file_rename_information.FileName();

                if (!::access(new_full_path.c_str(), F_OK)) {
                    if (!rdp_file_rename_information.replace_if_exists()) {
                        MakeClientDriveIoResponse(device_io_request,
                                                  "ManagedFile::ProcessServerDriveSetInformationRequest",
                                                  0xC0000035,   // STATUS_OBJECT_NAME_COLLISION
                                                  to_server_sender,
                                                  out_asynchronous_task,
                                                  verbose
                                                 );

                        return;
                    }
                }

                ::rename(this->full_path.c_str(), new_full_path.c_str());

                {
                    BStream out_stream(65536);

                    const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                            rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                           );
                    shared_header.emit(out_stream);

                    const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                                     device_io_request.CompletionId(),
                                                                     0x00000000 // STATUS_SUCCESS
                                                                    );
                    if (verbose) {
                        LOG(LOG_INFO, "ManagedFile::ProcessServerDriveSetInformationRequest");
                        device_io_response.log(LOG_INFO);
                    }
                    device_io_response.emit(out_stream);

                    out_stream.out_uint32_le(server_drive_set_information_request.Length());    // Length(4)

                    // Padding(1), optional

                    out_stream.mark_end();

                    uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

                    out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                        out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                        verbose);
                }
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFile::ProcessServerDriveSetInformationRequest: "
                        "Unknown FsInformationClass - %s(0x%X)",
                    server_drive_set_information_request.get_FsInformationClass_name(
                        server_drive_set_information_request.FsInformationClass()),
                    server_drive_set_information_request.FsInformationClass());
                REDASSERT(false);

                // Set file attributes is not yet supported.
                MakeClientDriveIoUnsuccessfulResponse(
                    device_io_request,
                    "ManagedFile::ProcessServerDriveSetInformationRequest",
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);

                // Unsupported.
                REDASSERT(false);
            break;
        }
    }
*/

    virtual void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode,
            bool first_chunk, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        REDASSERT(this->fd > -1);

        static uint32_t Length = 0;
        static uint64_t Offset = 0;

        static uint32_t remaining_number_of_bytes_to_write = 0;
        static uint32_t current_offset                     = 0;

        if (first_chunk) {
            Length = in_stream.in_uint32_le();
            Offset = in_stream.in_uint64_le();

            remaining_number_of_bytes_to_write = Length;
            current_offset                     = Offset;

            in_stream.in_skip_bytes(20);  // Padding(20)

            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveWriteRequest(): "
                    "Length=%u Offset=%" PRIu64,
                Length, Offset);
        }

        REDASSERT(remaining_number_of_bytes_to_write >= in_stream.in_remain());

        LOG(LOG_INFO,
            "ManagedFile::ProcessServerDriveWriteRequest(): "
                "CurrentOffset=%" PRIu64 " InRemain=%" PRIu64 " RemainingNumberOfBytesToWrite=%" PRIu64,
            current_offset, in_stream.in_remain(), remaining_number_of_bytes_to_write);

        off64_t seek_result = ::lseek64(this->fd, current_offset, SEEK_SET);
        REDASSERT(seek_result == current_offset);
        ::write(this->fd, in_stream.p, in_stream.in_remain());

        remaining_number_of_bytes_to_write -= in_stream.in_remain();
        current_offset                     += in_stream.in_remain();


        if (!remaining_number_of_bytes_to_write) {
            BStream out_stream(65536);

            const rdpdr::SharedHeader shared_header(rdpdr::Component::RDPDR_CTYP_CORE,
                                                    rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
                                                   );
            shared_header.emit(out_stream);

            const rdpdr::DeviceIOResponse device_io_response(device_io_request.DeviceId(),
                                                             device_io_request.CompletionId(),
                                                             0x00000000 // STATUS_SUCCESS
                                                            );
            if (verbose) {
                LOG(LOG_INFO, "ManagedFile::ProcessServerDriveWriteRequest");
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            out_stream.out_uint32_le(Length);   // Length(4)
            out_stream.out_uint8(0);            // Padding(1), optional

            out_stream.mark_end();

            uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

            out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                out_flags, out_stream.get_data(), out_stream.size(), to_server_sender,
                verbose);
        }
    }

    virtual void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) override {
        LOG(LOG_INFO, "+ + + + + + + + + + ManagedFile::ProcessServerDriveQueryDirectoryRequest() Using ToServerSender + + + + + + + + + +");
        MakeClientDriveIoUnsuccessfulResponse(device_io_request,
                                              "ManagedFile::ProcessServerDriveQueryDirectoryRequest",
                                              to_server_sender,
                                              out_asynchronous_task,
                                              verbose);

        // Unsupported.
        REDASSERT(false);
    }
};  // ManagedFile

class FileSystemDriveManager {
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    const uint32_t INVALID_MANAGED_DRIVE_ID = 0xFFFFFFFF;

#ifdef __clang__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-private-field"
# endif
    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;
#ifdef __clang__
    #pragma GCC diagnostic pop
# endif

    typedef std::tuple<uint32_t, std::string, std::string, int>
        managed_drive_type; // DeviceId, name, path, access mode.
    typedef std::vector<managed_drive_type> managed_drive_collection_type;
    managed_drive_collection_type managed_drives;

    typedef std::tuple<uint32_t, std::unique_ptr<ManagedFileSystemObject>>
        managed_file_system_object_type;    // FileId, object.
    typedef std::vector<managed_file_system_object_type>
        managed_file_system_object_collection_type;
    managed_file_system_object_collection_type managed_file_system_objects;

    uint32_t wab_agent_drive_id = INVALID_MANAGED_DRIVE_ID;

public:
    uint32_t AnnounceDrivePartially(Stream & client_device_list_announce,
            bool device_capability_version_02_supported, uint32_t verbose) {
        uint32_t announced_drive_count = 0;

        for (managed_drive_collection_type::iterator iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {
            rdpdr::DeviceAnnounceHeader device_announce_header(rdpdr::RDPDR_DTYP_FILESYSTEM,
                                                               std::get<0>(*iter),
                                                               std::get<1>(*iter).c_str(),
                                                               reinterpret_cast<uint8_t const *>(
                                                                   std::get<1>(*iter).c_str()),
                                                               std::get<1>(*iter).length() + 1
                                                              );

            if (verbose) {
                LOG(LOG_INFO, "FileSystemDriveManager::AnnounceDrivePartially");
                device_announce_header.log(LOG_INFO);
            }

            if (client_device_list_announce.has_room(device_announce_header.size())) {
                device_announce_header.emit(client_device_list_announce);

                announced_drive_count++;
            }
            else {
                LOG(LOG_ERR,
                    "FileSystemDriveManager::AnnounceDrivePartially: "
                        "Too much data for Announce Driver buffer! "
                        "length=%u limite=%u",
                    client_device_list_announce.get_offset() + device_announce_header.size(),
                    client_device_list_announce.get_capacity());
                throw Error(ERR_RDP_PROTOCOL);
            }
        }

        return announced_drive_count;
    }

private:
    uint32_t EnableDrive(const char * drive_name, const char * relative_directory_path,
                         bool read_only, uint32_t verbose) {
        uint32_t drive_id = INVALID_MANAGED_DRIVE_ID;

        std::string absolute_directory_path = DRIVE_REDIRECTION_PATH "/";
        absolute_directory_path += relative_directory_path;

        struct stat sb;

        if ((::stat(absolute_directory_path.c_str(), &sb) == 0) &&
            S_ISDIR(sb.st_mode)) {
            if (verbose) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::EnableDrive: "
                        "drive_name=\"%s\" directory_path=\"%s\"",
                    drive_name, absolute_directory_path.c_str());
            }

            drive_id = this->next_managed_drive_id++;

            managed_drives.push_back(
                    std::make_tuple(drive_id,
                                    drive_name,
                                    absolute_directory_path.c_str(),
                                    (read_only ? O_RDONLY : O_RDWR)
                                   )
                );
        }
        else {
            LOG(LOG_WARNING,
                "FileSystemDriveManager::EnableDrive: "
                    "Directory path \"%s\" is not accessible!",
                absolute_directory_path.c_str());
        }

        return drive_id;
    }

public:
    bool EnableDrive(const char * relative_directory_path, uint32_t verbose) {
        bool read_only = false;
        if (*relative_directory_path == '*') {
            read_only = true;
            relative_directory_path++;
        }

        if (!::strcasecmp(relative_directory_path, "wabagt") ||
            !::strcasecmp(relative_directory_path, "wablnch")) {
                LOG(LOG_WARNING,
                    "FileSystemDriveManager::EnableDrive: "
                        "Directory path \"%s\" is reserved!",
                    relative_directory_path);

            return false;
        }

        const unsigned   relative_directory_path_length =
            ::strlen(relative_directory_path);
        char           * drive_name                     =
            reinterpret_cast<char *>(::alloca(relative_directory_path_length + 1));

        ::strcpy(drive_name, relative_directory_path);
        for (unsigned i = 0; i < relative_directory_path_length; i++) {
            if ((drive_name[i] >= 0x61) && (drive_name[i] <= 0x7A)) {
                drive_name[i] -= 0x20;
            }
        }

        return (this->EnableDrive(
                    drive_name,
                    relative_directory_path,
                    read_only,      // read-only
                    verbose
                ) != INVALID_MANAGED_DRIVE_ID);
    }

    bool EnableWABAgentDrive(uint32_t verbose) {
        if (this->wab_agent_drive_id == INVALID_MANAGED_DRIVE_ID) {
            this->wab_agent_drive_id = this->EnableDrive(
                    "WABAGT",
                    "wabagt",
                    true,   // read-only
                    verbose
                );
        }

        return (this->wab_agent_drive_id != INVALID_MANAGED_DRIVE_ID);
    }

    // "WABLNCH", "/wablnch"

/*
private:
    const char * get_drive_directory_path_by_id(uint32_t DeviceId) const {
        for (managed_drive_collection_type::const_iterator iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {
            if (DeviceId == std::get<0>(*iter)) {
                return std::get<2>(*iter).c_str();
            }
        }

        throw Error(ERR_RDP_PROTOCOL);
    }
*/

public:
    bool HasManagedDrive() const {
        return (this->managed_drives.size() > 0);
    }

    bool IsManagedDrive(uint32_t DeviceId) const {
        if (DeviceId >= FIRST_MANAGED_DRIVE_ID) {
            for (managed_drive_collection_type::const_iterator iter = this->managed_drives.begin();
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
            int drive_access_mode, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(in_stream);
        if (verbose) {
            device_create_request.log(LOG_INFO);
        }

        std::string full_path    = path;
        std::string request_path = device_create_request.Path();
        if ((full_path.back() != '/') && (request_path.front() != '/')) {
            full_path += '/';
        }
        full_path += request_path;
        if (verbose) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::ProcessServerCreateDriveRequest: "
                    "full_path=\"%s\" drive_access_mode=%s(%d)",
                full_path.c_str(),
                ManagedFileSystemObject::get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        bool is_directory = false;

        struct stat sb;
        if (::stat(full_path.c_str(), &sb) == 0) {
            is_directory = ((sb.st_mode & S_IFMT) == S_IFDIR);
        }
        else {
            is_directory = (device_create_request.CreateOptions() & smb2::FILE_DIRECTORY_FILE);
        }

        std::unique_ptr<ManagedFileSystemObject> managed_file_system_object;
        if (is_directory) {
            managed_file_system_object = std::make_unique<ManagedDirectory>();
        }
        else {
            managed_file_system_object = std::make_unique<ManagedFile>();
        }
        bool drive_created = false;
        managed_file_system_object->ProcessServerCreateDriveRequest(
                device_io_request, device_create_request, drive_access_mode,
                path, in_stream, drive_created, to_server_sender,
                out_asynchronous_task, verbose);
        if (drive_created) {
            this->managed_file_system_objects.push_back(std::make_tuple(
                static_cast<uint32_t>(managed_file_system_object->FileDescriptor()),
                std::move(managed_file_system_object)
                ));
        }
    }

    void ProcessServerCloseDriveRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerCloseDriveRequest(
                    device_io_request, path, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
                this->managed_file_system_objects.erase(iter);
                break;
            }
        }
    }

    void ProcessServerDriveReadRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::DeviceReadRequest device_read_request;

        device_read_request.receive(in_stream);
        if (verbose) {
            device_read_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveReadRequest(
                    device_io_request, device_read_request, path, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveControlRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::DeviceControlRequest device_control_request;

        device_control_request.receive(in_stream);
        if (verbose) {
            device_control_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveControlRequest(
                    device_io_request, device_control_request, path, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveQueryVolumeInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::ServerDriveQueryVolumeInformationRequest
            server_drive_query_volume_information_request;

        server_drive_query_volume_information_request.receive(in_stream);
        if (verbose) {
            server_drive_query_volume_information_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveQueryVolumeInformationRequest(
                    device_io_request, server_drive_query_volume_information_request, path,
                    in_stream, to_server_sender, out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::ServerDriveQueryInformationRequest
            server_drive_query_information_request;

        server_drive_query_information_request.receive(in_stream);
        if (verbose) {
            server_drive_query_information_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveQueryInformationRequest(
                    device_io_request, server_drive_query_information_request, path,
                    in_stream, to_server_sender, out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            int drive_access_mode, bool first_chunk, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveWriteRequest(
                    device_io_request, path,
                    drive_access_mode, first_chunk, in_stream, to_server_sender,
                    out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            int drive_access_mode, Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::ServerDriveSetInformationRequest
            server_drive_set_information_request;

        server_drive_set_information_request.receive(in_stream);
        if (verbose) {
            server_drive_set_information_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveSetInformationRequest(
                    device_io_request, server_drive_set_information_request, path,
                    drive_access_mode, in_stream, to_server_sender,
                    out_asynchronous_task, verbose);
                break;
            }
        }
    }

    void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
        rdpdr::ServerDriveQueryDirectoryRequest
            server_drive_query_directory_request;

        server_drive_query_directory_request.receive(in_stream);
        if (verbose) {
            server_drive_query_directory_request.log(LOG_INFO);
        }

        for (managed_file_system_object_collection_type::iterator iter = this->managed_file_system_objects.begin();
             iter != this->managed_file_system_objects.end(); ++iter) {
            if (device_io_request.FileId() == std::get<0>(*iter)) {
                std::get<1>(*iter)->ProcessServerDriveQueryDirectoryRequest(
                    device_io_request, server_drive_query_directory_request, path,
                    in_stream, to_server_sender, out_asynchronous_task, verbose);
                break;
            }
        }
    }

public:
    void ProcessDeviceIORequest(
            rdpdr::DeviceIORequest const & device_io_request,
            bool first_chunk,
            Stream & in_stream,
            ToServerSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            uint32_t verbose) {
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
        if (iter == this->managed_drives.end()) { return; }

        const std::string path              = std::get<2>(*iter);
        const int         drive_access_mode = std::get<3>(*iter);

        switch (device_io_request.MajorFunction()) {
            case rdpdr::IRP_MJ_CREATE:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Create Drive Request");
                }

                this->ProcessServerCreateDriveRequest(device_io_request,
                    path.c_str(), drive_access_mode, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Close Drive Request");
                }

                this->ProcessServerCloseDriveRequest(device_io_request,
                    path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_READ:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Read Request");
                }

                this->ProcessServerDriveReadRequest(device_io_request,
                    path.c_str(), in_stream, to_server_sender,
                    out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_WRITE:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Write Request");
                }

                this->ProcessServerDriveWriteRequest(device_io_request,
                    path.c_str(), drive_access_mode, first_chunk, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Control Request");
                }

                this->ProcessServerDriveControlRequest(device_io_request,
                    path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Volume Information Request");
                }

                this->ProcessServerDriveQueryVolumeInformationRequest(device_io_request,
                    path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Query Information Request");
                }

                this->ProcessServerDriveQueryInformationRequest(device_io_request,
                    path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_SET_INFORMATION:
                if (verbose) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::ProcessDeviceIORequest: "
                            "Server Drive Set Information Request");
                }
                this->ProcessServerDriveSetInformationRequest(device_io_request,
                    path.c_str(), drive_access_mode, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                switch (device_io_request.MinorFunction()) {
                    case rdpdr::IRP_MN_QUERY_DIRECTORY:
                        if (verbose) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::ProcessDeviceIORequest: "
                                    "Directory control request - Query directory request");
                        }

                        this->ProcessServerDriveQueryDirectoryRequest(device_io_request,
                            path.c_str(), in_stream,
                            to_server_sender, out_asynchronous_task, verbose);
                    break;

                    case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                        if (verbose) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::ProcessDeviceIORequest: "
                                    "Directory control request - Notify change directory request");
                        }

                        // Not yet supported!
                    break;

                    default:
                        LOG(LOG_ERR,
                            "FileSystemDriveManager::ProcessDeviceIORequest: "
                                "Unknown Directory control request - MinorFunction=0x%X",
                            device_io_request.MinorFunction());
                        throw Error(ERR_RDP_PROTOCOL);
                    //break;
                }
            break;

            default:
                LOG(LOG_ERR,
                    "FileSystemDriveManager::ProcessDeviceIORequest: "
                        "Undecoded Device I/O Request - MajorFunction=0x%X",
                    device_io_request.MajorFunction());
                REDASSERT(false);

                ManagedFileSystemObject::MakeClientDriveIoUnsuccessfulResponse(
                    device_io_request,
                    "FileSystemDriveManager::ProcessDeviceIORequest",
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            break;
        }
    }

    void DisableWABAgentDrive(ToServerSender & to_server_sender, uint32_t verbose = 0) {
        if (this->wab_agent_drive_id == INVALID_MANAGED_DRIVE_ID) {
            return;
        }

        const uint32_t old_wab_agent_drive_id = this->wab_agent_drive_id;

        this->wab_agent_drive_id = INVALID_MANAGED_DRIVE_ID;

        managed_drive_collection_type::iterator iter;
        for (iter = this->managed_drives.begin();
             iter != this->managed_drives.end(); ++iter) {
            if (old_wab_agent_drive_id == std::get<0>(*iter)) {
                this->managed_drives.erase(iter);

                break;
            }
        }

        BStream out_stream(1024);

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);
        sh_s.emit(out_stream);

        out_stream.out_uint32_le(1);                        // DeviceCount(4)
        out_stream.out_uint32_le(old_wab_agent_drive_id);   // DeviceIds(variable)

        out_stream.mark_end();

        if (verbose) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::DisableWABAgentDrive");
        }

        to_server_sender(
                out_stream.size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.size()
            );
    }
};  // FileSystemDriveManager

#endif  // REDEMPTION_CORE_RDP_CHANNELS_RDPDRFILESYSTEMDRIVEMANAGER_HPP

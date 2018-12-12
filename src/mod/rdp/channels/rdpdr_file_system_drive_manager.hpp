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


#pragma once

#include "core/channel_list.hpp"
#include "core/app_path.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/SMB2/MessageSyntax.hpp"
#include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/winpr/pattern.hpp"

#include <sys/types.h>
#include <dirent.h>
#include <sys/statvfs.h>

#include <vector>
#include <string>
#include <algorithm>

#ifdef __EMSCRIPTEN__
inline int futimes(int /*fd*/, const timeval /*tv*/[2])
{
    return 0;
}
#endif

template<typename T> T Flag(bool condition, T value)
{
    return (condition) ? value : T(0);
}

#define EPOCH_DIFF 11644473600LL

#define FILE_TIME_SYSTEM_TO_RDP(_t) \
    ((static_cast<uint64_t>(_t) + EPOCH_DIFF) * 10000000LL)
#define FILE_TIME_RDP_TO_SYSTEM(_t) \
    (((_t) == 0LL || (_t) == static_cast<uint64_t>(-1LL)) \
    ? 0 : static_cast<time_t>((_t) / 10000000LL - EPOCH_DIFF))

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

    inline int file_descriptor() const {
        assert(this->fd > -1);

        return this->fd;
    }

    virtual bool is_directory() const = 0;

    virtual bool is_session_probe_image() const { return false; }

    virtual void process_server_create_drive_request(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceCreateRequest const & device_create_request,
        int drive_access_mode, const char * path, InStream & in_stream,
        bool & out_drive_created,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        bool is_session_probe_image_flag,
        RDPVerbose verbose) = 0;

    virtual void process_server_close_drive_request(
        rdpdr::DeviceIORequest const & device_io_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

    virtual void process_server_drive_read_request(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::DeviceReadRequest const & device_read_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

    virtual void process_server_drive_control_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)device_control_request;
        (void)in_stream;
        assert(this->fd > -1);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFileSystemObject::ProcessServerDriveControlRequest",
            erref::NTSTATUS::STATUS_SUCCESS,
            verbose);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFileSystemObject::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryVolumeInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryVolumeInformationRequest const &
                server_drive_query_volume_information_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)in_stream;
        assert(this->fd > -1);

        StaticOutStream<65536> out_stream;

        switch (server_drive_query_volume_information_request.FsInformationClass()) {
            case rdpdr::FileFsVolumeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);
                struct stat64 sb;
                ::stat64(path, &sb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                const fscc::FileFsVolumeInformation file_fs_volume_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),   // VolumeCreationTime(8)
                        svfsb.f_fsid,                           // VolumeSerialNumber(4)
                        1,                                      // SupportsObjects(1) - FALSE
                        "REDEMPTION"
                    );

                out_stream.out_uint32_le(file_fs_volume_information.size());    // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_volume_information.log(LOG_INFO);
                }
                file_fs_volume_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                const fscc::FileFsSizeInformation file_fs_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // AvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_size_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
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

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                const fscc::FileFsAttributeInformation file_fs_attribute_information(
                        fscc::FILE_CASE_SENSITIVE_SEARCH |      // FileSystemAttributes(4)
                            fscc::FILE_CASE_PRESERVED_NAMES |
                            //fscc::FILE_READ_ONLY_VOLUME |
                            fscc::FILE_UNICODE_ON_DISK,
                        svfsb.f_namemax,                        // MaximumComponentNameLength(4)
                        "FAT32"                                 // FileSystemName(variable)
                    );

                out_stream.out_uint32_le(file_fs_attribute_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_attribute_information.log(LOG_INFO);
                }
                file_fs_attribute_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsFullSizeInformation:
            {
                struct statvfs svfsb;
                ::statvfs(path, &svfsb);

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                const fscc::FileFsFullSizeInformation file_fs_full_size_information(
                        svfsb.f_blocks, // TotalAllocationUnits(8)
                        svfsb.f_bavail, // CallerAvailableAllocationUnits(8)
                        svfsb.f_bfree,  // ActualAvailableAllocationUnits(8)
                        1,              // SectorsPerAllocationUnit(4)
                        svfsb.f_bsize   // BytesPerSector(4)
                    );

                out_stream.out_uint32_le(file_fs_full_size_information.size()); // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_full_size_information.log(LOG_INFO);
                }
                file_fs_full_size_information.emit(out_stream);
            }
            break;

            case rdpdr::FileFsDeviceInformation:
            {
                LOG(LOG_INFO,
                    "+ + + + + + + + + + ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest() - FileFsDeviceInformation - Using VirtualChannelDataSender + + + + + + + + + +");

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                const fscc::FileFsDeviceInformation file_fs_device_information(
                        fscc::FILE_DEVICE_DISK, 0
                    );

                out_stream.out_uint32_le(file_fs_device_information.size());    // Length(4)

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest");
                    file_fs_device_information.log(LOG_INFO);
                }
                file_fs_device_information.emit(out_stream);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest: "
                        "Unknown FsInformationClass(0x%X)",
                    server_drive_query_volume_information_request.FsInformationClass());

                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryVolumeInformationRequest",
                    erref::NTSTATUS::STATUS_UNSUCCESSFUL,
                    verbose);
            break;
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveQueryInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryInformationRequest const & server_drive_query_information_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)in_stream;
        assert(this->fd > -1);

        StaticOutStream<65536> out_stream;

        struct stat64 sb;
        ::fstat64(this->fd, &sb);

        switch (server_drive_query_information_request.FsInformationClass()) {
            case rdpdr::FileBasicInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                out_stream.out_uint32_le(fscc::FileBasicInformation::size());   // Length(4)

                fscc::FileBasicInformation file_basic_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // CreationTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),                           // LastAccessTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),                           // LastWriteTime(8)
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),                           // ChangeTime(8)
                        // FileAttributes(4)
                        Flag(this->is_directory(), fscc::FILE_ATTRIBUTE_DIRECTORY)
                        | Flag(!(sb.st_mode & S_IWUSR),fscc::FILE_ATTRIBUTE_READONLY)
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }
                file_basic_information.emit(out_stream);
            }
            break;

            case rdpdr::FileStandardInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                out_stream.out_uint32_le(fscc::FileStandardInformation::size());    // Length(4)

                const size_t block_size = 512;

                fscc::FileStandardInformation file_standard_information(
                        sb.st_blocks * block_size,  // AllocationSize
                        sb.st_size,                 // EndOfFile
                        sb.st_nlink,                // NumberOfLinks
                        0,                          // DeletePending
                        0                           // Directory
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_standard_information.log(LOG_INFO);
                }
                file_standard_information.emit(out_stream);
            }
            break;

            case rdpdr::FileAttributeTagInformation:
            {
                this->MakeClientDriveIoResponse(
                    out_stream,
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    verbose);

                out_stream.out_uint32_le(fscc::FileAttributeTagInformation::size());    // Length(4)

                fscc::FileAttributeTagInformation file_attribute_tag_information(
                        // FileAttributes
                         fscc::FILE_ATTRIBUTE_DIRECTORY
                        | Flag(!(sb.st_mode & S_IWUSR), fscc::FILE_ATTRIBUTE_READONLY),
                    0                                                                   // ReparseTag
                    );

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest");
                    file_attribute_tag_information.log(LOG_INFO);
                }
                file_attribute_tag_information.emit(out_stream);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveQueryInformationRequest: "
                        "Unknown FsInformationClass=%s(0x%X)",
                    server_drive_query_information_request.get_FsInformationClass_name(
                        server_drive_query_information_request.FsInformationClass()),
                    server_drive_query_information_request.FsInformationClass());
                throw Error(ERR_RDP_PROTOCOL);
            //break;
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    virtual void ProcessServerDriveSetInformationRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveSetInformationRequest const & server_drive_set_information_request,
            const char * path, int drive_access_mode, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        assert(this->fd > -1);

        if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
            this->SendClientDriveIoResponse(
                device_io_request,
                "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                erref::NTSTATUS::STATUS_INVALID_PARAMETER,
                to_server_sender,
                out_asynchronous_task,
                verbose);

            return;
        }

        switch (server_drive_set_information_request.FsInformationClass())
        {
            case rdpdr::FileBasicInformation:
            {
                fscc::FileBasicInformation file_basic_information;

                file_basic_information.receive(in_stream);

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    file_basic_information.log(LOG_INFO);
                }

                struct stat64 sb;
                ::fstat64(this->fd, &sb);

                struct timeval times[2] = { { sb.st_atime, 0 }, { sb.st_mtime, 0 } };

                auto file_time_rdp_to_system_timeval = [](uint64_t rdp_time, timeval & out_system_tiem) {
                    if ((rdp_time != 0LL) && (rdp_time != static_cast<uint64_t>(-1LL))) {
                        out_system_tiem.tv_sec  = static_cast<time_t>(rdp_time / 10000000LL - EPOCH_DIFF);
                        out_system_tiem.tv_usec = rdp_time % 10000000LL;
                    }
                };

                file_time_rdp_to_system_timeval(file_basic_information.LastAccessTime(), times[0]);
                file_time_rdp_to_system_timeval(file_basic_information.LastWriteTime(),  times[1]);

                ::futimes(this->fd, times);

                const mode_t mode =
                    ((file_basic_information.FileAttributes() & fscc::FILE_ATTRIBUTE_READONLY) ?
                     (sb.st_mode & (~S_IWUSR)) :
                     (sb.st_mode | S_IWUSR)
                    );
                ::chmod(this->full_path.c_str(), mode);

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileEndOfFileInformation:
            {
                int64_t EndOfFile = in_stream.in_sint64_le();

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "EndOfFile=%" PRId64,
                        EndOfFile);
                }

                int truncate_result = ::ftruncate(this->fd, EndOfFile);
                (void)truncate_result;

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileDispositionInformation:
                this->delete_pending = true;

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "DeletePending=yes");
                }

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            break;

            case rdpdr::FileRenameInformation:
            {
                rdpdr::RDPFileRenameInformation rdp_file_rename_information;

                //auto in_stream_p = in_stream.get_current();

                rdp_file_rename_information.receive(in_stream);

                //LOG(LOG_INFO, "FileRenameInformation: size=%u",
                //    (unsigned int)(in_stream.get_current() - in_stream_p));
                //hexdump(in_stream_p, in_stream.get_current() - in_stream_p);

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO, "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest");
                    rdp_file_rename_information.log(LOG_INFO);
                }

                assert(!rdp_file_rename_information.RootDirectory());

                std::string new_full_path(path);
                new_full_path += rdp_file_rename_information.FileName();

                if (!::access(new_full_path.c_str(), F_OK)) {
                    if (!rdp_file_rename_information.replace_if_exists()) {
                        this->SendClientDriveIoResponse(
                            device_io_request,
                            "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                            (this->is_directory() ?
                             erref::NTSTATUS::STATUS_OBJECT_NAME_INVALID :
                             erref::NTSTATUS::STATUS_OBJECT_NAME_COLLISION
                            ),
                            to_server_sender,
                            out_asynchronous_task,
                            verbose);

                        return;
                    }
                }

                (void)::rename(this->full_path.c_str(), new_full_path.c_str());

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            case rdpdr::FileAllocationInformation:
            {
                int64_t AllocationSize = in_stream.in_sint64_le();

                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                            "AllocationSize=%" PRId64,
                        AllocationSize);
                }

                int truncate_result = ::ftruncate(this->fd, AllocationSize);
                (void)truncate_result;

                this->SendClientDriveSetInformationResponse(
                    device_io_request,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    server_drive_set_information_request.Length(),
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            }
            break;

            default:
                LOG(LOG_ERR,
                    "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest: "
                        "Unknown FsInformationClass - %s(0x%X)",
                    server_drive_set_information_request.get_FsInformationClass_name(
                        server_drive_set_information_request.FsInformationClass()),
                    server_drive_set_information_request.FsInformationClass());
                assert(false);

                SendClientDriveIoUnsuccessfulResponse(device_io_request,
                                                      "ManagedFileSystemObject::ProcessServerDriveSetInformationRequest",
                                                      to_server_sender,
                                                      out_asynchronous_task,
                                                      verbose);

                // Unsupported.
                assert(false);
            break;
        }
    }

    virtual void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode, bool first_chunk,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) {
        (void)path;
        (void)in_stream;
        (void)drive_access_mode;
        (void)first_chunk;

        SendClientDriveIoUnsuccessfulResponse(
            device_io_request,
            "ManagedFileSystemObject::ProcessServerDriveWriteRequest",
            to_server_sender,
            out_asynchronous_task,
            verbose);

        // Unsupported.
        assert(false);
    }

    virtual void ProcessServerDriveQueryDirectoryRequest(
        rdpdr::DeviceIORequest const & device_io_request,
        rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
        const char * path, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
        RDPVerbose verbose) = 0;

protected:
    static void MakeClientDriveIoResponse(
            OutStream & out_stream,
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            erref::NTSTATUS IoStatus,
            RDPVerbose verbose) {
        const rdpdr::SharedHeader shared_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION
            );
        shared_header.emit(out_stream);

        const rdpdr::DeviceIOResponse device_io_response(
                device_io_request.DeviceId(),
                device_io_request.CompletionId(),
                IoStatus
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "%s", message);
            device_io_response.log(LOG_INFO);
        }
        device_io_response.emit(out_stream);
    }

    static inline void SendClientDriveIoResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            erref::NTSTATUS IoStatus,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

    static void SendClientDriveSetInformationResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            erref::NTSTATUS IoStatus,
            uint32_t Length,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        out_stream.out_uint32_le(Length);   // Length(4)

        // Padding(1), optional

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

public:
    static void SendClientDriveLockControlResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            erref::NTSTATUS IoStatus,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        StaticOutStream<65536> out_stream;

        MakeClientDriveIoResponse(out_stream,
            device_io_request, message, IoStatus, verbose);

        out_stream.out_clear_bytes(5);  // Padding(5)

        uint32_t out_flags =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(),
            to_server_sender, verbose);
    }

    static inline void SendClientDriveIoUnsuccessfulResponse(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * message,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        SendClientDriveIoResponse(
            device_io_request,
            message,
            erref::NTSTATUS::STATUS_UNSUCCESSFUL,
            to_server_sender,
            out_asynchronous_task,
            verbose);
    }
};  // ManagedFileSystemObject

class ManagedDirectory final : public ManagedFileSystemObject {
    DIR * dir = nullptr;

    std::string pattern;

public:
    //ManagedDirectory() {
    //    LOG(LOG_INFO, "ManagedDirectory::ManagedDirectory() : <%p>", this);
    //}

    ~ManagedDirectory() override {
        //LOG(LOG_INFO, "ManagedDirectory::~ManagedDirectory(): <%p> fd=%d",
        //    this, (this->dir ? ::dirfd(this->dir) : -1));

        if (this->dir) {
            ::closedir(this->dir);
        }

        if (this->delete_pending) {
            ::recursive_delete_directory(this->full_path.c_str());
        }
    }

    bool is_directory() const override { return true; }

    void process_server_create_drive_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, InStream & in_stream,
            bool & out_drive_created,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            bool is_session_probe_image_flag,
            RDPVerbose verbose
      ) override {
        (void)in_stream;
        (void)is_session_probe_image_flag;
        assert(!this->dir);

        out_drive_created = false;

        {
            this->full_path = path;
            auto av = device_create_request.Path();
            this->full_path.append(av.data(), av.size());
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                static_cast<void*>(this), this->full_path, get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        const uint32_t DesiredAccess     = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const erref::NTSTATUS IoStatus = [&] () {
            if (((drive_access_mode != O_RDWR)
                && (drive_access_mode != O_RDONLY)
                && smb2::read_access_is_required(DesiredAccess, /*strict_check = */false))
            ||  ((drive_access_mode != O_RDWR)
                && (drive_access_mode != O_WRONLY)
                && smb2::write_access_is_required(DesiredAccess, /*strict_check = */false))) {
                return erref::NTSTATUS::STATUS_ACCESS_DENIED;
            }

            if ((::access(full_path.c_str(), F_OK) != 0)
            && (CreateDisposition == smb2::FILE_CREATE)) {
                if ((drive_access_mode != O_RDWR) && (drive_access_mode != O_WRONLY)) {
                    return erref::NTSTATUS::STATUS_ACCESS_DENIED;
                }

                ::mkdir(full_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP);
            }

            this->dir = ::opendir(full_path.c_str());
            if (this->dir == nullptr){
                switch (errno) {
                case EACCES:
                    return erref::NTSTATUS::STATUS_ACCESS_DENIED;
                case ENOENT:
                    return erref::NTSTATUS::STATUS_NO_SUCH_FILE;
                default:
                    return erref::NTSTATUS::STATUS_UNSUCCESSFUL;
                break;
                }
            }
            return erref::NTSTATUS::STATUS_SUCCESS;
        } ();

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerCreateDriveRequest: <%p> dir=<%p> FileId=%d errno=%d",
                static_cast<void*>(this),
                static_cast<void*>(this->dir),
                (this->dir ? ::dirfd(this->dir) : -1),
                (this->dir ? 0 : errno));
        }

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::process_server_create_drive_request",
            IoStatus,
            verbose);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->dir ? ::dirfd(this->dir) : -1),
                0x0
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "ManagedDirectory::process_server_create_drive_request");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        //if (this->dir) {
        //    LOG(LOG_INFO, "ManagedDirectory::process_server_create_drive_request(): <%p> fd=%d",
        //        this, ::dirfd(this->dir));
        //}

        if (this->dir != nullptr) {
            this->fd = ::dirfd(this->dir);

            out_drive_created = true;
        }
    }

    void process_server_close_drive_request(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;

        assert(this->dir);

        //LOG(LOG_INFO, "ManagedDirectory::process_server_close_drive_request(): <%p> fd=%d",
        //    this, ::dirfd(this->dir));

        ::closedir(this->dir);

        this->dir = nullptr;
        this->fd  = -1;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::ProcessServerCloseDriveRequest",
            erref::NTSTATUS::STATUS_SUCCESS,
            verbose);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        assert(!this->dir);
    }

    void process_server_drive_read_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)device_read_request;
        (void)path;
        (void)in_stream;
        assert(this->dir);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedDirectory::ProcessServerDriveReadRequest",
            erref::NTSTATUS::STATUS_SUCCESS,
            verbose);

        out_stream.out_uint32_le(0);    // Length(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

    void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;

        if (server_drive_query_directory_request.InitialQuery()) {
            ::rewinddir(this->dir);

            const char * separator = strrchr(server_drive_query_directory_request.Path(), '/');
            assert(separator);
            this->pattern = (++separator);
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                    "full_path=\"%s\" pattern=\"%s\"",
                this->full_path, this->pattern);
        }

        dirent * entry = nullptr;
        while ((entry = ::readdir(this->dir))) {
            // TODO this function is incompatible with utf8
            if (::FilePatternMatchA(entry->d_name, this->pattern.c_str())) {
                break;
            }
        }

        StaticOutStream<65536> out_stream;

        if (!entry) {
            this->MakeClientDriveIoResponse(
                out_stream,
                device_io_request,
                "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                erref::NTSTATUS::STATUS_NO_MORE_FILES,
                verbose);

            out_stream.out_uint32_le(0);    // Length(4)
            out_stream.out_clear_bytes(1);  // Padding(1)
        }
        else {
            std::string file_full_path = this->full_path;
            if ((file_full_path.back() != '/') && (entry->d_name[0] != '/')) {
                file_full_path += '/';
            }
            file_full_path += entry->d_name;
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                        "<%p> full_path=\"%s\"",
                    static_cast<void*>(this), file_full_path);
            }

            struct stat64 sb;

            ::stat64(file_full_path.c_str(), &sb);

            switch (server_drive_query_directory_request.FsInformationClass()) {
                case rdpdr::FileFullDirectoryInformation:
                {
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        erref::NTSTATUS::STATUS_SUCCESS,
                        verbose);

                    const fscc::FileFullDirectoryInformation file_full_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        Flag(S_ISDIR(sb.st_mode),fscc::FILE_ATTRIBUTE_DIRECTORY)
                        | Flag(!(sb.st_mode & S_IWUSR),fscc::FILE_ATTRIBUTE_READONLY),
                        entry->d_name
                        );
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
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
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        erref::NTSTATUS::STATUS_SUCCESS,
                        verbose);

                    const fscc::FileBothDirectoryInformation file_both_directory_information(
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_atime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_mtime),
                        FILE_TIME_SYSTEM_TO_RDP(sb.st_ctime),
                        sb.st_size, sb.st_blocks * 512 /* Block size */,
                        Flag(S_ISDIR(sb.st_mode),fscc::FILE_ATTRIBUTE_DIRECTORY)
                        | Flag(!(sb.st_mode & S_IWUSR), fscc::FILE_ATTRIBUTE_READONLY),
                        entry->d_name
                        );
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
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
                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        erref::NTSTATUS::STATUS_SUCCESS,
                        verbose);

                    const fscc::FileNamesInformation file_name_information(entry->d_name);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        LOG(LOG_INFO,
                            "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest");
                        file_name_information.log(LOG_INFO);
                    }

                    out_stream.out_uint32_le(file_name_information.size()); // Length(4)

//auto out_stream_p = out_stream.get_current();
                    file_name_information.emit(out_stream);
//LOG(LOG_INFO, "FileNamesInformation: size=%u",
//    static_cast<unsigned int>(out_stream.get_current() - out_stream_p));
//hexdump(out_stream_p, out_stream.get_current() - out_stream_p);
                }
                break;

                default:
                {
                    LOG(LOG_ERR,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest: "
                            "Unknown FsInformationClass(0x%X)",
                        server_drive_query_directory_request.FsInformationClass());
                    assert(false);

                    this->MakeClientDriveIoResponse(
                        out_stream,
                        device_io_request,
                        "ManagedDirectory::ProcessServerDriveQueryDirectoryRequest",
                        erref::NTSTATUS::STATUS_UNSUCCESSFUL,
                        verbose);
                }
                break;
            }
        }

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }
};  // ManagedDirectory

class ManagedFile final : public ManagedFileSystemObject {
    bool is_session_probe_image_flag = false;

public:
    //ManagedFile() {
    //    LOG(LOG_INFO, "ManagedFile::ManagedFile(): <%p>", this);
    //}

    ~ManagedFile() override {
        //LOG(LOG_INFO, "ManagedFile::~ManagedFile(): <%p> fd=%d",
        //    this, this->fd);

        // File descriptor will be closed when in_file_transport is destroyed.

        assert(this->fd <= -1);

        if (this->fd <= -1) {
          ::close(this->fd);
        }

        if (this->delete_pending) {
            ::unlink(this->full_path.c_str());
        }
    }

    bool is_directory() const override { return false; }

    bool is_session_probe_image() const override { return this->is_session_probe_image_flag; }

    void process_server_create_drive_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceCreateRequest const & device_create_request,
            int drive_access_mode, const char * path, InStream & in_stream,
            bool & out_drive_created,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            bool is_session_probe_image_flag,
            RDPVerbose verbose
      ) override {
        (void)in_stream;
        assert(this->fd == -1);

        out_drive_created = false;

        this->is_session_probe_image_flag = is_session_probe_image_flag;

        {
            this->full_path = path;
            auto av = device_create_request.Path();
            this->full_path.append(av.data(), av.size());
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::process_server_create_drive_request: "
                    "<%p> full_path=\"%s\" drive_access_mode=%s(%d)",
                static_cast<void*>(this), this->full_path,
                get_open_flag_name(drive_access_mode), drive_access_mode);
        }

        const uint32_t DesiredAccess     = device_create_request.DesiredAccess();
        const uint32_t CreateDisposition = device_create_request.CreateDisposition();

        const int last_error = [this] (
            uint32_t DesiredAccess,
            uint32_t CreateDisposition,
            int drive_access_mode,
            RDPVerbose verbose
        ) -> int {
            this->fd = -1;

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

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedFile::ProcessServerCreateDriveRequest: <%p> open_flags=0x%X",
                    static_cast<void*>(this), unsigned(open_flags));
            }

            this->fd = ::open(this->full_path.c_str(), open_flags, S_IRUSR | S_IWUSR | S_IRGRP);
            return (fd != -1 ? 0 : errno);
        } (DesiredAccess, CreateDisposition, drive_access_mode, verbose);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerCreateDriveRequest: <%p> FileId=%d errno=%d",
                static_cast<void*>(this), this->fd, ((this->fd == -1) ? last_error : 0));
        }

        const erref::NTSTATUS IoStatus = [] (int fd, int last_error) -> erref::NTSTATUS {
            if (fd > -1) { return erref::NTSTATUS::STATUS_SUCCESS; }

            switch (last_error) {
                case ENOENT:
                    return erref::NTSTATUS::STATUS_NO_SUCH_FILE;

                case EACCES:
                    return erref::NTSTATUS::STATUS_ACCESS_DENIED;
            }

            return erref::NTSTATUS::STATUS_UNSUCCESSFUL;
        } (this->fd, last_error);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::process_server_create_drive_request",
            IoStatus,
            verbose);

        const rdpdr::DeviceCreateResponse device_create_response(
                static_cast<uint32_t>(this->fd),
                0x0
            );
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO, "ManagedFile::process_server_create_drive_request");
            device_create_response.log(LOG_INFO);
        }
        device_create_response.emit(out_stream);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        //if (this->fd > -1) {
        //    LOG(LOG_INFO, "ManagedFile::process_server_create_drive_request(): <%p> fd=%d",
        //        this, this->fd);
        //}

        out_drive_created = (this->fd != -1);
    }   // process_server_create_drive_request

    void process_server_close_drive_request(
            rdpdr::DeviceIORequest const & device_io_request, const char * path,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;
        assert(this->fd > -1);

        //LOG(LOG_INFO, "ManagedFile::process_server_close_drive_request(): <%p> fd=%d",
        //    this, this->fd);

        ::close(this->fd);

        this->fd = -1;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::process_server_close_drive_request",
            erref::NTSTATUS::STATUS_SUCCESS,
            verbose);

        // Device Close Response (DR_CLOSE_RSP)
        out_stream.out_clear_bytes(5);  // Padding(5);

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);

        assert(this->fd == -1);
    }

    void process_server_drive_read_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceReadRequest const & device_read_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)in_stream;
        assert(this->fd > -1);

        const uint32_t Length = device_read_request.Length();

        const uint64_t Offset = device_read_request.Offset();

        struct stat64 sb;
        if (::fstat64(this->fd, &sb)) {
            StaticOutStream<512> out_stream;

            ManagedFileSystemObject::MakeClientDriveIoResponse(
                  out_stream
                , device_io_request
                , "ManagedFile::ProcessServerDriveReadRequest"
                , erref::NTSTATUS::STATUS_UNSUCCESSFUL
                , verbose);

            out_stream.out_uint32_le(0);    // Length(4)

            uint32_t out_flags =
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

            out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                out_flags, out_stream.get_data(), out_stream.get_offset(),
                to_server_sender, verbose);

            return;
        }

        off64_t remaining_number_of_bytes_to_read = std::min<off64_t>(
            sb.st_size - Offset, Length);

        out_asynchronous_task = std::make_unique<RdpdrDriveReadTask>(
            this->fd, device_io_request.DeviceId(),
            device_io_request.CompletionId(),
            static_cast<uint32_t>(remaining_number_of_bytes_to_read),
            Offset, to_server_sender, verbose);
    }

    void process_server_drive_control_request(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::DeviceControlRequest const & device_control_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)device_control_request;
        (void)path;
        (void)in_stream;
        assert(this->fd > -1);

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerDriveControlRequest",
            erref::NTSTATUS::STATUS_SUCCESS,
            verbose);

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveControlRequest: OutputBufferLength=0");
        }
        out_stream.out_uint32_le(0);    // OutputBufferLength(4)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }

private:
    uint32_t Length = 0;

    uint32_t remaining_number_of_bytes_to_write = 0;
    uint32_t current_offset                     = 0;

public:
    void ProcessServerDriveWriteRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            const char * path, int drive_access_mode,
            bool first_chunk, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)path;
        (void)drive_access_mode;
        assert(this->fd > -1);

        if (first_chunk) {
            Length = in_stream.in_uint32_le();
            uint64_t const
            Offset = in_stream.in_uint64_le();

            remaining_number_of_bytes_to_write = Length;
            current_offset                     = Offset;

            in_stream.in_skip_bytes(20);  // Padding(20)

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "ManagedFile::ProcessServerDriveWriteRequest(): "
                        "Length=%u Offset=%" PRIu64,
                    Length, Offset);
            }
        }

        assert(remaining_number_of_bytes_to_write >= in_stream.in_remain());

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "ManagedFile::ProcessServerDriveWriteRequest(): "
                    "CurrentOffset=%" PRIu32
                    " InRemain=%zu RemainingNumberOfBytesToWrite=%" PRIu32,
                current_offset, in_stream.in_remain(),
                remaining_number_of_bytes_to_write);
        }

        off64_t seek_result = ::lseek64(this->fd, current_offset, SEEK_SET);
        (void)seek_result;
        assert(seek_result == off64_t(current_offset));
        int write_result = ::write(this->fd, in_stream.get_current(), in_stream.in_remain());
        (void)write_result;

        remaining_number_of_bytes_to_write -= in_stream.in_remain();
        current_offset                     += in_stream.in_remain();


        if (!remaining_number_of_bytes_to_write) {
            StaticOutStream<65536> out_stream;

            this->MakeClientDriveIoResponse(
                out_stream,
                device_io_request,
                "ManagedFile::ProcessServerDriveQueryInformationRequest",
                erref::NTSTATUS::STATUS_SUCCESS,
                verbose);

            out_stream.out_uint32_le(Length);   // Length(4)
            out_stream.out_uint8(0);            // Padding(1), optional

            uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

            out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
                out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
                verbose);
        }
    }

    void ProcessServerDriveQueryDirectoryRequest(
            rdpdr::DeviceIORequest const & device_io_request,
            rdpdr::ServerDriveQueryDirectoryRequest const & server_drive_query_directory_request,
            const char * path, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose
      ) override {
        (void)server_drive_query_directory_request;
        (void)path;
        (void)in_stream;

        StaticOutStream<65536> out_stream;

        this->MakeClientDriveIoResponse(
            out_stream,
            device_io_request,
            "ManagedFile::ProcessServerDriveQueryDirectoryRequest",
            erref::NTSTATUS::STATUS_NO_MORE_FILES,
            verbose);

        out_stream.out_uint32_le(0);    // Length(4)
        out_stream.out_uint8(0);        // Padding(1)

        uint32_t out_flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        out_asynchronous_task = std::make_unique<RdpdrSendDriveIOResponseTask>(
            out_flags, out_stream.get_data(), out_stream.get_offset(), to_server_sender,
            verbose);
    }
};  // ManagedFile

class FileSystemDriveManager {
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    const uint32_t INVALID_MANAGED_DRIVE_ID = 0xFFFFFFFF;

    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;

    struct managed_drive_type
    {
        uint32_t device_id;
        std::string name;
        std::string path;
        int access_mode;
    };
    using managed_drive_collection_type = std::vector<managed_drive_type>;
    managed_drive_collection_type managed_drives;

    struct managed_file_system_object_type
    {
        uint32_t file_id;
        std::unique_ptr<ManagedFileSystemObject> object;
    };
    using managed_file_system_object_collection_type = std::vector<managed_file_system_object_type>;
    managed_file_system_object_collection_type managed_file_system_objects;

    uint32_t session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

    SessionProbeLauncher* session_probe_drive_access_notifier = nullptr;
    SessionProbeLauncher* session_probe_image_read_notifier   = nullptr;

public:
    void announce_drive(bool device_capability_version_02_supported,
            VirtualChannelDataSender& to_server_sender, RDPVerbose verbose) {
        (void)device_capability_version_02_supported;
        uint8_t   virtual_channel_data[CHANNELS::CHANNEL_CHUNK_LENGTH];
        OutStream virtual_channel_stream(virtual_channel_data);

        for (managed_drive_type const & managed_drive : this->managed_drives) {

            rdpdr::SharedHeader client_message_header(
                rdpdr::Component::RDPDR_CTYP_CORE,
                rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);

            client_message_header.emit(virtual_channel_stream);

            virtual_channel_stream.out_uint32_le(
                 1  // DeviceCount(4)
                );

            rdpdr::DeviceAnnounceHeader device_announce_header(
                    rdpdr::RDPDR_DTYP_FILESYSTEM,   // DeviceType
                    managed_drive.device_id,
                    managed_drive.name.c_str(),     // PreferredDosName
                    byte_ptr_cast(managed_drive.name.c_str()),
                    managed_drive.name.length() + 1
                );

            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO, "FileSystemDriveManager::AnnounceDrive");
                device_announce_header.log(LOG_INFO);
            }

            device_announce_header.emit(virtual_channel_stream);

            to_server_sender(virtual_channel_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                virtual_channel_data,
                virtual_channel_stream.get_offset());

            virtual_channel_stream.rewind();
        }
    }

    struct DriveName
    {
        explicit DriveName()
        {
            this->name_[0] = 0;
            this->upper_name_[0] = 0;
        }

        DriveName(array_view_const_char name, bool reserved = false) noexcept
        : read_only_(false)
        {
            if (!name.empty() && name[0] == '*') {
                name = name.array_from_offset(1);
                this->read_only_ = true;
            }

            if (name.size() > 7) {
                LOG(LOG_ERR,
                    "FileSystemDriveManager::enable_drive: "
                        "Drive name \"%.*s\" too long.",
                    int(name.size()), name.data());
                this->name_[0] = 0;
                return;
            }

            for (std::size_t i = 0; i < name.size(); ++i) {
                this->name_[i] = name[i];
                this->upper_name_[i] = name[i];
                if (name[i] >= 'a' && name[i] <= 'z') {
                    this->upper_name_[i] -= 0x20;
                }
            }
            this->name_[name.size()] = 0;
            this->upper_name_[name.size()] = 0;

            if (!reserved
             && (!strcmp("SESPRO", this->upper_name_)
              || !strcmp("WABLNCH", this->upper_name_))
            ){
                LOG(LOG_WARNING,
                    "FileSystemDriveManager::enable_drive: "
                        "Drive name \"%.*s\" is reserved!",
                    int(name.size()), name.data());
            }
        }

        DriveName(char const* name, bool reserved = false) noexcept
        : DriveName({name, strlen(name)}, reserved)
        {}

        char const* upper_name() const noexcept
        {
            return this->upper_name_;
        }

        char const* name() const noexcept
        {
            return this->name_;
        }

        bool is_valid() const noexcept
        {
            return bool(this->name_[0]);
        }

        bool is_read_only() const noexcept
        {
            return this->read_only_;
        }

    private:
        char name_[8];
        char upper_name_[8];
        bool read_only_;
    };

private:
    uint32_t enable_drive(DriveName drive_name, std::string directory_drive_path,
                         bool read_only, RDPVerbose verbose) {
        uint32_t drive_id = INVALID_MANAGED_DRIVE_ID;

        struct stat sb;

        if (!directory_drive_path.empty()) {
            directory_drive_path += '/';
        }
        directory_drive_path += drive_name.name();

        if (((::stat(directory_drive_path.c_str(), &sb) == 0) && S_ISDIR(sb.st_mode))) {
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::enable_drive: "
                        "directory_path=\"%s\"",
                    directory_drive_path);
            }

            drive_id = this->next_managed_drive_id++;

            this->managed_drives.push_back({
                drive_id, drive_name.upper_name(), directory_drive_path, (read_only ? O_RDONLY : O_RDWR)
            });
        }
        else {
            LOG(LOG_WARNING,
                "FileSystemDriveManager::enable_drive: "
                    "Directory path \"%s\" is not accessible!",
                directory_drive_path);
        }

        return drive_id;
    }

public:
    bool enable_drive_client(DriveName drive_name, const char * directory_path, RDPVerbose verbose)
    {
        return drive_name.is_valid()
            && this->enable_drive(
                    drive_name,
                    directory_path,
                    false,
                    verbose);
    }

    bool enable_drive(DriveName const& drive_name, std::string directory_drive_path, RDPVerbose verbose) {
        return drive_name.is_valid()
            && (this->enable_drive(
                    drive_name,
                    std::move(directory_drive_path),
                    drive_name.is_read_only(),
                    verbose
                ) != INVALID_MANAGED_DRIVE_ID);
    }

    bool enable_session_probe_drive(std::string directory, RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            this->session_probe_drive_id = this->enable_drive(
                DriveName("sespro", true),
                std::move(directory),
                true,       // read-only
                verbose
            );
        }

        return (this->session_probe_drive_id != INVALID_MANAGED_DRIVE_ID);
    }

public:
    uint32_t get_session_probe_drive_id() const { return this->session_probe_drive_id; }

    bool has_managed_drive() const {
        return !this->managed_drives.empty();
    }

private:
    managed_drive_collection_type::const_iterator
    find_drive_by_id(uint32_t DeviceId) const {
        return std::find_if(
            this->managed_drives.cbegin(),
            this->managed_drives.cend(),
            [DeviceId](managed_drive_type const & managed_drive) {
                return DeviceId == managed_drive.device_id;
            }
        );
    }

public:
    bool is_managed_drive(uint32_t DeviceId) const {
        return DeviceId >= FIRST_MANAGED_DRIVE_ID
            && this->find_drive_by_id(DeviceId) != this->managed_drives.cend();
    }

private:
    void process_server_create_drive_request(
            rdpdr::DeviceIORequest const & device_io_request,
            std::string const & path, int drive_access_mode, InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        rdpdr::DeviceCreateRequest device_create_request;

        device_create_request.receive(in_stream);
        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            device_create_request.log(LOG_INFO);
        }

        std::string full_path    = path;
        {
            auto const  request_path = device_create_request.Path();
            if ((full_path.back() != '/') && (request_path.data()[0] != '/')) {
                full_path += '/';
            }
            full_path.append(request_path.data(), request_path.size());
        }

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::process_server_create_drive_request: "
                    "full_path=\"%s\" drive_access_mode=%s(%d)",
                full_path,
                ManagedFileSystemObject::get_open_flag_name(drive_access_mode),
                drive_access_mode);
        }

        bool is_directory = false;

        struct stat sb;
        if (::stat(full_path.c_str(), &sb) == 0) {
            is_directory = ((sb.st_mode & S_IFMT) == S_IFDIR);
        }
        else {
            is_directory =
                (device_create_request.CreateOptions() &
                 smb2::FILE_DIRECTORY_FILE);
        }

        bool is_session_probe_image_flag = false;

        std::unique_ptr<ManagedFileSystemObject> managed_file_system_object;
        if (is_directory) {
            managed_file_system_object = std::make_unique<ManagedDirectory>();
        }
        else {
            is_session_probe_image_flag =
                ((device_io_request.DeviceId() == this->session_probe_drive_id) &&
                 !::strcmp(device_create_request.Path().data(), "/BIN"));

            managed_file_system_object = std::make_unique<ManagedFile>();
        }
        bool drive_created = false;
        managed_file_system_object->process_server_create_drive_request(
                device_io_request, device_create_request, drive_access_mode,
                path.c_str(), in_stream, drive_created, to_server_sender,
                out_asynchronous_task, is_session_probe_image_flag, verbose);
        if (drive_created) {
            this->managed_file_system_objects.push_back({
                static_cast<uint32_t>(managed_file_system_object->file_descriptor()),
                std::move(managed_file_system_object)
            });
        }
    }

public:
    void process_device_IO_request(
            rdpdr::DeviceIORequest const & device_io_request,
            bool first_chunk,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender,
            std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
            RDPVerbose verbose) {
        uint32_t DeviceId = device_io_request.DeviceId();
        if (DeviceId < FIRST_MANAGED_DRIVE_ID) {
            return;
        }
        auto drive_iter = this->find_drive_by_id(DeviceId);
        if (drive_iter == this->managed_drives.end()) {
            LOG(LOG_WARNING,
                "FileSystemDriveManager::process_device_IO_request: "
                    "Unknown device. DeviceId=%u",
                DeviceId);
            return;
        }

        std::string const & path              = drive_iter->path;
        int const           drive_access_mode = drive_iter->access_mode;

        managed_file_system_object_collection_type::const_iterator file_iter;
        if (device_io_request.MajorFunction() != rdpdr::IRP_MJ_CREATE) {
            file_iter = std::find_if(
                this->managed_file_system_objects.begin(),
                this->managed_file_system_objects.end(),
                [&device_io_request](managed_file_system_object_type const & file) {
                    return device_io_request.FileId() == file.file_id;
                }
            );
            if (file_iter == this->managed_file_system_objects.end()) {
                LOG(LOG_WARNING,
                    "FileSystemDriveManager::process_device_IO_request: "
                        "Unknown file. FileId=%u",
                    device_io_request.FileId());
                return;
            }
        }

        switch (device_io_request.MajorFunction()) {
            case rdpdr::IRP_MJ_CREATE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Create Drive Request");
                }

                if (this->session_probe_drive_access_notifier) {
                    if (DeviceId == this->session_probe_drive_id) {
                        if (!this->session_probe_drive_access_notifier->on_drive_access()) {
                            this->session_probe_drive_access_notifier = nullptr;
                        }
                    }
                }

                this->process_server_create_drive_request(device_io_request,
                    path, drive_access_mode, in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_CLOSE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Close Drive Request");
                }

                file_iter->object->process_server_close_drive_request(
                    device_io_request, path.c_str(), in_stream,
                    to_server_sender, out_asynchronous_task, verbose);
                if(file_iter + 1 != this->managed_file_system_objects.end()) {
                    this->managed_file_system_objects[
                        file_iter - this->managed_file_system_objects.begin()
                    ] = std::move(this->managed_file_system_objects.back());
                }
                this->managed_file_system_objects.pop_back();
            break;

            case rdpdr::IRP_MJ_READ:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Read Request");
                }

                {
                    rdpdr::DeviceReadRequest device_read_request;

                    device_read_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        device_read_request.log(LOG_INFO);
                    }
                    if (this->session_probe_image_read_notifier) {
                        if (file_iter->object->is_session_probe_image()) {
                            if (!this->session_probe_image_read_notifier->on_image_read(
                                    device_read_request.Offset(),
                                    device_read_request.Length())) {
                                this->session_probe_image_read_notifier = nullptr;
                            }
                        }
                    }

                    file_iter->object->process_server_drive_read_request(
                        device_io_request, device_read_request, path.c_str(),
                        in_stream, to_server_sender, out_asynchronous_task,
                        verbose);
                }
            break;

            case rdpdr::IRP_MJ_WRITE:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Write Request");
                }

                file_iter->object->ProcessServerDriveWriteRequest(
                    device_io_request, path.c_str(), drive_access_mode,
                    first_chunk, in_stream, to_server_sender,
                    out_asynchronous_task, verbose);
            break;

            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Control Request");
                }

                {
                    rdpdr::DeviceControlRequest device_control_request;

                    device_control_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        device_control_request.log(LOG_INFO);
                    }

                    file_iter->object->process_server_drive_control_request(
                        device_io_request, device_control_request,
                        path.c_str(), in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Query Volume Information Request");
                }

                {
                    rdpdr::ServerDriveQueryVolumeInformationRequest
                        server_drive_query_volume_information_request;

                    server_drive_query_volume_information_request.receive(
                        in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_query_volume_information_request.log(
                            LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveQueryVolumeInformationRequest(
                        device_io_request,
                        server_drive_query_volume_information_request,
                        path.c_str(), in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Query Information Request");
                }

                {
                    rdpdr::ServerDriveQueryInformationRequest
                        server_drive_query_information_request;

                    server_drive_query_information_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_query_information_request.log(LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveQueryInformationRequest(
                        device_io_request,
                        server_drive_query_information_request, path.c_str(),
                        in_stream, to_server_sender, out_asynchronous_task,
                        verbose);
                }
            break;

            case rdpdr::IRP_MJ_SET_INFORMATION:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Set Information Request");
                }

                {
                    rdpdr::ServerDriveSetInformationRequest
                        server_drive_set_information_request;

                    server_drive_set_information_request.receive(in_stream);
                    if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                        server_drive_set_information_request.log(LOG_INFO);
                    }

                    file_iter->object->ProcessServerDriveSetInformationRequest(
                        device_io_request,
                        server_drive_set_information_request, path.c_str(),
                        drive_access_mode, in_stream, to_server_sender,
                        out_asynchronous_task, verbose);
                }
            break;

            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:
                switch (device_io_request.MinorFunction()) {
                    case rdpdr::IRP_MN_QUERY_DIRECTORY:
                        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::process_device_IO_request: "
                                    "Directory control request - "
                                    "Query directory request");
                        }

                        {
                            rdpdr::ServerDriveQueryDirectoryRequest
                                server_drive_query_directory_request;

                            //auto in_stream_p = in_stream.get_current();

                            server_drive_query_directory_request.receive(
                                in_stream);

                            //LOG(LOG_INFO,
                            //    "ServerDriveQueryDirectoryRequest: size=%u",
                            //    (unsigned int)(in_stream.get_current() - in_stream_p));
                            //hexdump(in_stream_p,
                            //    in_stream.get_current() - in_stream_p);

                            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                                server_drive_query_directory_request.log(
                                    LOG_INFO);
                            }

                            file_iter->object->ProcessServerDriveQueryDirectoryRequest(
                                device_io_request,
                                server_drive_query_directory_request,
                                path.c_str(), in_stream, to_server_sender,
                                out_asynchronous_task, verbose);
                        }
                    break;

                    case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                            LOG(LOG_INFO,
                                "FileSystemDriveManager::process_device_IO_request: "
                                    "Directory control request - "
                                    "Notify change directory request");
                        }

                        // Not yet supported!
                    break;

                    default:
                        LOG(LOG_ERR,
                            "FileSystemDriveManager::process_device_IO_request: "
                                "Unknown Directory control request - "
                                "MinorFunction=0x%X",
                            device_io_request.MinorFunction());
                        throw Error(ERR_RDP_PROTOCOL);
                    //break;
                }
            break;

            case rdpdr::IRP_MJ_LOCK_CONTROL:
                if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                    LOG(LOG_INFO,
                        "FileSystemDriveManager::process_device_IO_request: "
                            "Server Drive Lock Control Request");
                }

                ManagedFileSystemObject::SendClientDriveLockControlResponse(
                    device_io_request,
                    "FileSystemDriveManager::process_device_IO_request",
                    erref::NTSTATUS::STATUS_SUCCESS,
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);

            break;

            default:
                LOG(LOG_ERR,
                    "FileSystemDriveManager::process_device_IO_request: "
                        "Undecoded Device I/O Request - "
                        "MajorFunction=%s(0x%X)",
                    rdpdr::get_MajorFunction_name(
                        device_io_request.MajorFunction()),
                    device_io_request.MajorFunction());
                assert(false);

                ManagedFileSystemObject::SendClientDriveIoUnsuccessfulResponse(
                    device_io_request,
                    "FileSystemDriveManager::process_device_IO_request",
                    to_server_sender,
                    out_asynchronous_task,
                    verbose);
            break;
        }
    }

    void remove_session_probe_drive(RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            return;
        }

        const uint32_t old_session_probe_drive_id = this->session_probe_drive_id;

        this->session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

        auto iter = this->find_drive_by_id(old_session_probe_drive_id);
        if (iter != this->managed_drives.end()) {
            if(iter + 1 != this->managed_drives.end()) {
                this->managed_drives[
                    iter - this->managed_drives.begin()
                ] = std::move(this->managed_drives.back());
            }
            this->managed_drives.pop_back();
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::remove_session_probe_drive: Drive removed.");
            }
        }
    }

    void disable_session_probe_drive(VirtualChannelDataSender & to_server_sender,
            RDPVerbose verbose) {
        if (this->session_probe_drive_id == INVALID_MANAGED_DRIVE_ID) {
            return;
        }

        const uint32_t old_session_probe_drive_id = this->session_probe_drive_id;

        this->session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

        StaticOutStream<1024> out_stream;

        const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                       rdpdr::PacketId::PAKID_CORE_DEVICELIST_REMOVE);
        sh_s.emit(out_stream);

        out_stream.out_uint32_le(1);                            // DeviceCount(4)
        out_stream.out_uint32_le(old_session_probe_drive_id);   // DeviceIds(variable)

        to_server_sender(
                out_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset()
            );

        if (bool(verbose & RDPVerbose::fsdrvmgr)) {
            LOG(LOG_INFO,
                "FileSystemDriveManager::disable_session_probe_drive: Remove request sent.");
        }

        auto iter = this->find_drive_by_id(old_session_probe_drive_id);
        if (iter != this->managed_drives.end()) {
            if(iter + 1 != this->managed_drives.end()) {
                this->managed_drives[
                    iter - this->managed_drives.begin()
                ] = std::move(this->managed_drives.back());
            }
            this->managed_drives.pop_back();
            if (bool(verbose & RDPVerbose::fsdrvmgr)) {
                LOG(LOG_INFO,
                    "FileSystemDriveManager::disable_session_probe_drive: Drive removed.");
            }
        }
    }

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->session_probe_drive_access_notifier = launcher;
        this->session_probe_image_read_notifier   = launcher;
    }
};  // FileSystemDriveManager

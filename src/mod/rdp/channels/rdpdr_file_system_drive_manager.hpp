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

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "mod/rdp/rdp_verbose.hpp"

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <string_view>


class AsynchronousTaskContainer;
class VirtualChannelDataSender;
class SessionProbeLauncher;
class InStream;

namespace rdpdr
{
    class DeviceIORequest;
}


class FileSystemDriveManager
{
public:
    FileSystemDriveManager(AsynchronousTaskContainer& async_task_container, RDPVerbose verbose);
    ~FileSystemDriveManager();

    void announce_drive(
        bool device_capability_version_02_supported,
        VirtualChannelDataSender& to_server_sender
    );

    struct DriveName
    {
        explicit DriveName() noexcept
        {
            this->name_[0] = 0;
            this->upper_name_[0] = 0;
        }

        DriveName(chars_view name, bool reserved = false) noexcept;
        DriveName(std::string_view name, bool reserved = false) noexcept;

        [[nodiscard]] std::array<char, 8> const& upper_name() const noexcept
        {
            return this->upper_name_;
        }

        [[nodiscard]] zstring_view name() const noexcept
        {
            return zstring_view(zstring_view::is_zero_terminated(), this->name_, this->len_);
        }

        [[nodiscard]] bool is_valid() const noexcept
        {
            return bool(this->name_[0]);
        }

        [[nodiscard]] bool is_read_only() const noexcept
        {
            return this->read_only_;
        }

    private:
        char name_[8];
        std::array<char, 8> upper_name_ {};
        uint8_t len_ = 0;
        bool read_only_;
    };

public:
    bool enable_drive_client(DriveName drive_name, std::string_view directory_path);
    bool enable_drive(DriveName const& drive_name, std::string_view directory_drive_path);
    bool enable_session_probe_drive(std::string_view directory);

    [[nodiscard]] uint32_t get_session_probe_drive_id() const { return this->session_probe_drive_id; }

    [[nodiscard]] bool has_managed_drive() const { return !this->managed_drives.empty(); }

    [[nodiscard]] bool is_managed_drive(uint32_t DeviceId) const;

    void process_device_IO_request(
            rdpdr::DeviceIORequest const & device_io_request,
            bool first_chunk,
            InStream & in_stream,
            VirtualChannelDataSender & to_server_sender
    );

    void remove_session_probe_drive();

    void disable_session_probe_drive(VirtualChannelDataSender & to_server_sender);

    void set_session_probe_launcher(SessionProbeLauncher* launcher)
    {
        this->session_probe_drive_access_notifier = launcher;
        this->session_probe_image_read_notifier   = launcher;
    }

private:
    uint32_t enable_drive(
        DriveName drive_name,
        std::string_view directory_drive_path,
        bool read_only);

    void process_server_create_drive_request(
        rdpdr::DeviceIORequest const & device_io_request,
        std::string const & path, int drive_access_mode, InStream & in_stream,
        VirtualChannelDataSender & to_server_sender
    );

private:
    const uint32_t FIRST_MANAGED_DRIVE_ID = 32767;

    const uint32_t INVALID_MANAGED_DRIVE_ID = 0xFFFFFFFF;

    uint32_t next_managed_drive_id = FIRST_MANAGED_DRIVE_ID;

    struct managed_drive_type;
    using managed_drive_collection_type = std::vector<managed_drive_type>;
    managed_drive_collection_type managed_drives;

    struct managed_file_system_object_type;
    using managed_file_system_object_collection_type = std::vector<managed_file_system_object_type>;
    managed_file_system_object_collection_type managed_file_system_objects;

    uint32_t session_probe_drive_id = INVALID_MANAGED_DRIVE_ID;

    SessionProbeLauncher* session_probe_drive_access_notifier = nullptr;
    SessionProbeLauncher* session_probe_image_read_notifier   = nullptr;

    AsynchronousTaskContainer& async_task_container;
    RDPVerbose verbose;

    [[nodiscard]] managed_drive_collection_type::const_iterator
    find_drive_by_id(uint32_t DeviceId) const;
};  // FileSystemDriveManager

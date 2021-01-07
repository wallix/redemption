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


#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "test_only/transport/test_transport.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/channels/asynchronous_task_container.hpp"

#include "./test_channel.hpp"
#include "acl/auth_api.hpp"


namespace
{
    namespace data_normal
    {
        #include "fixtures/test_rdpdr_channel.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = true;
            file_system_virtual_channel_params.file_system_write_authorized = true;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = true;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_normal
    namespace data_no_drive
    {
        #include "fixtures/test_rdpdr_channel_no_drive.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = false;
            file_system_virtual_channel_params.file_system_write_authorized = false;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = true;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_no_drive
    namespace data_no_print
    {
        #include "fixtures/test_rdpdr_channel_no_print.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = true;
            file_system_virtual_channel_params.file_system_write_authorized = true;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = false;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_no_print
    namespace data_no_drive_no_print
    {
        #include "fixtures/test_rdpdr_channel_no_drive_no_print.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = false;
            file_system_virtual_channel_params.file_system_write_authorized = false;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = false;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_no_drive_no_print
    namespace data_device_remote
    {
        #include "fixtures/test_rdpdr_channel_device_remove.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = true;
            file_system_virtual_channel_params.file_system_write_authorized = true;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = true;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_device_remote
    namespace data_fragment_header
    {
        #include "fixtures/test_rdpdr_channel_fragmented_header.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = true;
            file_system_virtual_channel_params.file_system_write_authorized = true;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = true;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = true;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_fragment_header
    namespace data_capability_negotiation
    {
        #include "fixtures/test_rdpdr_channel_capability_negotiation.hpp"
        const auto file_system_virtual_channel_params = []{ /*NOLINT*/
            FileSystemVirtualChannelParams file_system_virtual_channel_params;
            file_system_virtual_channel_params.file_system_read_authorized  = true;
            file_system_virtual_channel_params.file_system_write_authorized = true;
            file_system_virtual_channel_params.parallel_port_authorized     = true;
            file_system_virtual_channel_params.print_authorized             = true;
            file_system_virtual_channel_params.serial_port_authorized       = true;
            file_system_virtual_channel_params.smart_card_authorized        = false;
            return file_system_virtual_channel_params;
        }();
    } // namespace data_capability_negotiation
} // namespace

RED_AUTO_TEST_CASE(TestRdpdrChannel)
{
    struct D
    {
        char const* name;
        chars_view indata;
        chars_view outdata;
        FileSystemVirtualChannelParams const& file_system_virtual_channel_params;
        bool enable_drive;
    };
#define F(name, enable_drive) D{#name,        \
    cstr_array_view(name::indata),            \
    cstr_array_view(name::outdata),           \
    name::file_system_virtual_channel_params, \
    enable_drive}
    RED_TEST_CONTEXT_DATA(D const& d, d.name, {
        F(data_normal, true),
        F(data_no_drive, true),
        F(data_no_print, true),
        F(data_no_drive_no_print, true),
        F(data_device_remote, true),
        F(data_fragment_header, true),
        F(data_capability_negotiation, false)
    })
    {
        WorkingDirectory wd(d.name);

        RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

        EventContainer events;
        NullSessionLog session_log;

        AsynchronousTaskContainer async_task_container(events);
        FileSystemDriveManager file_system_drive_manager(async_task_container, verbose);

        if (d.enable_drive)
        {
            auto exportdir = wd.create_subdirectory("export");
            auto sharedir = wd.create_subdirectory("share");

            file_system_drive_manager.enable_drive("export"_av, wd.dirname().string());
            file_system_drive_manager.enable_drive("share"_av, wd.dirname().string());
        }

        TestTransport t(d.indata, d.outdata);

        TestToClientSender to_client_sender(t);
        TestToServerSender to_server_sender(t);

        const char * client_name                  = "rzh";
        uint32_t     random_number                = 5245;
        const char * proxy_managed_drive_prefix   = "";

        FileSystemVirtualChannel file_system_virtual_channel(
            events, &to_client_sender, &to_server_sender,
            file_system_drive_manager, false, "", client_name, random_number, proxy_managed_drive_prefix, d.file_system_virtual_channel_params,
            session_log, verbose);

        RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);

        RED_CHECK_WORKSPACE(wd);
    }
}

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


#define RED_TEST_MODULE TestRDPDRChannel
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "test_only/transport/test_transport.hpp"
#include "mod/rdp/channels/rdpdr_channel.hpp"
#include "mod/rdp/channels/rdpdr_file_system_drive_manager.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"

#include "./test_channel.hpp"
#include "test_only/front/fake_front.hpp"

namespace
{
    FileSystemVirtualChannel::Params file_system_param(
        ReportMessageApi& report_message, RDPVerbose verbose)
    {
        FileSystemVirtualChannel::Params file_system_virtual_channel_params(report_message);

        file_system_virtual_channel_params.exchanged_data_limit         = 0;
        file_system_virtual_channel_params.verbose                      = verbose;

        file_system_virtual_channel_params.client_name                  = "rzh";

        file_system_virtual_channel_params.random_number                = 5245;

        file_system_virtual_channel_params.dont_log_data_into_syslog    = false;
        file_system_virtual_channel_params.dont_log_data_into_wrm       = false;

        file_system_virtual_channel_params.proxy_managed_drive_prefix   = "";

        return file_system_virtual_channel_params;
    }
}


RED_AUTO_TEST_CASE(TestRdpdrChannel)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = true;
    file_system_virtual_channel_params.file_system_write_authorized = true;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = true;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;


    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender,
        file_system_drive_manager, front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelNoDrive)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = false;
    file_system_virtual_channel_params.file_system_write_authorized = false;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = true;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;

    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel_no_drive.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelNoPrint)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = true;
    file_system_virtual_channel_params.file_system_write_authorized = true;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = false;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;

    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel_no_print.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelNoDriveNoPrint)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = false;
    file_system_virtual_channel_params.file_system_write_authorized = false;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = false;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;

    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel_no_drive_no_print.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelDeviceRemove)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = true;
    file_system_virtual_channel_params.file_system_write_authorized = true;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = true;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;

    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel_device_remove.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelFragmentedHeader)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = true;
    file_system_virtual_channel_params.file_system_write_authorized = true;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = true;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = true;

    FileSystemDriveManager file_system_drive_manager;

    mkdir("/tmp/export", 0664); file_system_drive_manager.enable_drive("export", "/tmp", verbose);
    mkdir("/tmp/share", 0664); file_system_drive_manager.enable_drive("share", "/tmp", verbose);

    #include "fixtures/test_rdpdr_channel_fragmented_header.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

RED_AUTO_TEST_CASE(TestRdpdrChannelCapabilityNegotiation)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 800, 600};
    FakeFront front(screen_info);
    RDPVerbose verbose = RDPVerbose::rdpdr | RDPVerbose::rdpdr_dump;

    NullReportMessage report_message;
    FileSystemVirtualChannel::Params file_system_virtual_channel_params
      = file_system_param(report_message, verbose);

    file_system_virtual_channel_params.file_system_read_authorized  = true;
    file_system_virtual_channel_params.file_system_write_authorized = true;

    file_system_virtual_channel_params.parallel_port_authorized     = true;
    file_system_virtual_channel_params.print_authorized             = true;
    file_system_virtual_channel_params.serial_port_authorized       = true;
    file_system_virtual_channel_params.smart_card_authorized        = false;

    FileSystemDriveManager file_system_drive_manager;

    #include "fixtures/test_rdpdr_channel_capability_negotiation.hpp"
    TestTransport t(indata, sizeof(indata)-1, outdata, sizeof(outdata)-1);

    TestToClientSender to_client_sender(t);
    TestToServerSender to_server_sender(t);

    SessionReactor session_reactor;
    FileSystemVirtualChannel file_system_virtual_channel(
        session_reactor, &to_client_sender, &to_server_sender, file_system_drive_manager,
        front, file_system_virtual_channel_params);

    RED_CHECK_EXCEPTION_ERROR_ID(CHECK_CHANNEL(t, file_system_virtual_channel), ERR_TRANSPORT_NO_MORE_DATA);
}

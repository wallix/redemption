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

#define RED_TEST_MODULE TestRDPDRChannelchannel
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "client_redemption/client_channels/fake_client_mod.hpp"
#include "client_redemption/client_channels/client_rdpdr_channel.hpp"

namespace rdpdr
{
    inline std::ostream& operator<<(std::ostream& out, rdpdr::Component const & e)
    {
        return out << +underlying_cast(e); // '+' for transform u8/s8 to int
    }

    inline std::ostream& operator<<(std::ostream& out, rdpdr::PacketId const & e)
    {
        return out << +underlying_cast(e); // '+' for transform u8/s8 to int
    }
}


RED_AUTO_TEST_CASE(TestRDPDRChannelInitialization)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    StaticOutStream<512> out_serverAnnounce;
    rdpdr::SharedHeader header_serverAnnounce(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE);
    header_serverAnnounce.emit(out_serverAnnounce);
    out_serverAnnounce.out_uint16_le(0x0001);
    out_serverAnnounce.out_uint16_le(0x000c);
    out_serverAnnounce.out_uint32_le(0x00000002);
    InStream chunk_serverAnnounce(out_serverAnnounce.get_bytes());

    RED_CHECK_EQUAL(channel.protocol_minor_version, 0);

    channel.receive(chunk_serverAnnounce);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 2);

    RED_CHECK_EQUAL(channel.protocol_minor_version, 0x000c);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_clientIDConfirm(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_clientIDConfirm.in_uint16_le(), rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(stream_clientIDConfirm.in_uint16_le(), rdpdr::PAKID_CORE_CLIENTID_CONFIRM);
    RED_CHECK_EQUAL(stream_clientIDConfirm.in_uint16_le(), 0x0001);
    RED_CHECK_EQUAL(stream_clientIDConfirm.in_uint16_le(), 0x000c);
    RED_CHECK_EQUAL(stream_clientIDConfirm.in_uint32_le(), 0x00000002);

    pdu_data = mod.stream();
    InStream stream_clientName(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_clientName.in_uint16_le(), rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(stream_clientName.in_uint16_le(), rdpdr::PAKID_CORE_CLIENT_NAME);
    RED_CHECK_EQUAL(stream_clientName.in_uint32_le(), 0x00000001);
    RED_CHECK_EQUAL(stream_clientName.in_uint32_le(), 0x00000000);
    RED_CHECK_EQUAL(pdu_data->size, 16+stream_clientName.in_uint32_le());


    StaticOutStream<512> out_serverCapabilityRequest;
    rdpdr::SharedHeader header_serverCapabilityRequest(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY);
    header_serverCapabilityRequest.emit(out_serverCapabilityRequest);
    out_serverCapabilityRequest.out_uint16_le(5);
    out_serverCapabilityRequest.out_skip_bytes(2);
    out_serverCapabilityRequest.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
    out_serverCapabilityRequest.out_uint16_le(44);
    out_serverCapabilityRequest.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_02);
    out_serverCapabilityRequest.out_uint32_le(0x00000002);
    out_serverCapabilityRequest.out_uint32_le(0x00000000);
    out_serverCapabilityRequest.out_uint16_le(0x0001);
    out_serverCapabilityRequest.out_uint16_le(0x000c);
    out_serverCapabilityRequest.out_uint32_le(0x0000ffff);
    out_serverCapabilityRequest.out_uint32_le(0x00000000);
    out_serverCapabilityRequest.out_uint32_le(0x00000007);
    out_serverCapabilityRequest.out_uint32_le(0x00000000);
    out_serverCapabilityRequest.out_uint32_le(0x00000000);
    out_serverCapabilityRequest.out_uint32_le(0x00000002);

    out_serverCapabilityRequest.out_uint16_le(rdpdr::CAP_PRINTER_TYPE);
    out_serverCapabilityRequest.out_uint16_le(8);
    out_serverCapabilityRequest.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_01);

    out_serverCapabilityRequest.out_uint16_le(rdpdr::CAP_PORT_TYPE);
    out_serverCapabilityRequest.out_uint16_le(8);
    out_serverCapabilityRequest.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_01);

    out_serverCapabilityRequest.out_uint16_le(rdpdr::CAP_DRIVE_TYPE);
    out_serverCapabilityRequest.out_uint16_le(8);
    out_serverCapabilityRequest.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_02);

    out_serverCapabilityRequest.out_uint16_le(rdpdr::CAP_SMARTCARD_TYPE);
    out_serverCapabilityRequest.out_uint16_le(8);
    out_serverCapabilityRequest.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_01);

    InStream chunk_serverCapabilityRequest(out_serverCapabilityRequest.get_bytes());

    channel.receive(chunk_serverCapabilityRequest);

    RED_CHECK_EQUAL(channel.server_capability_number, 5);
    //RED_CHECK_EQUAL(channel.fileSystemCapacity[rdpdr::CAP_GENERAL_TYPE], true);
    RED_CHECK_EQUAL(channel.fileSystemCapacity[rdpdr::CAP_PRINTER_TYPE], true);
    RED_CHECK_EQUAL(channel.fileSystemCapacity[rdpdr::CAP_PORT_TYPE], true);
    RED_CHECK_EQUAL(channel.fileSystemCapacity[rdpdr::CAP_DRIVE_TYPE], true);
    RED_CHECK_EQUAL(channel.fileSystemCapacity[rdpdr::CAP_SMARTCARD_TYPE], true);


    StaticOutStream<512> out_serverClientIDConfirm;
    rdpdr::SharedHeader header_serverClientIDConfirm(rdpdr::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    header_serverClientIDConfirm.emit(out_serverClientIDConfirm);
    out_serverAnnounce.out_uint16_le(0x0001);
    out_serverAnnounce.out_uint16_le(0x000c);
    out_serverAnnounce.out_uint32_le(0x00000002);

    InStream chunk_serverClientIDConfirm(out_serverClientIDConfirm.get_bytes());

    channel.receive(chunk_serverClientIDConfirm);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 4);

    pdu_data = mod.stream();
    InStream stream_clientCapability(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 84);
    rdpdr::SharedHeader header_clientCapability;
    header_clientCapability.receive(stream_clientCapability);
    RED_CHECK_EQUAL(header_clientCapability.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_clientCapability.packet_id, rdpdr::PAKID_CORE_CLIENT_CAPABILITY);

    RED_CHECK_EQUAL(header_clientCapability.packet_id, rdpdr::PAKID_CORE_CLIENT_CAPABILITY);

    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 5);
    stream_clientCapability.in_skip_bytes(2);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), rdpdr::CAP_GENERAL_TYPE);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 44);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), rdpdr::GENERAL_CAPABILITY_VERSION_02);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000002);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000000);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 0x0001);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 0x000c);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x0000ffff);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000000);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000007);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000001);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000000);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), 0x00000000);

    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), rdpdr::CAP_PRINTER_TYPE);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 8);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), rdpdr::GENERAL_CAPABILITY_VERSION_01);

    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), rdpdr::CAP_PORT_TYPE);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 8);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), rdpdr::GENERAL_CAPABILITY_VERSION_01);

    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), rdpdr::CAP_DRIVE_TYPE);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 8);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), rdpdr::GENERAL_CAPABILITY_VERSION_01);

    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), rdpdr::CAP_SMARTCARD_TYPE);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint16_le(), 8);
    RED_CHECK_EQUAL(stream_clientCapability.in_uint32_le(), rdpdr::GENERAL_CAPABILITY_VERSION_01);

    pdu_data = mod.stream();
    InStream stream_deviceListAnnounce(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceListAnnounce;
    header_deviceListAnnounce.receive(stream_deviceListAnnounce);
    RED_CHECK_EQUAL(header_deviceListAnnounce.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceListAnnounce.packet_id, rdpdr::PAKID_CORE_DEVICELIST_ANNOUNCE);
}


RED_AUTO_TEST_CASE(TestRDPDRChannelCreateFileOrDir)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000000,
                                0x00000001,
                                rdpdr::IRP_MJ_CREATE,
                                0x00000000);
    dior.emit(out_stream);

    const char * path = "/xd/fg";
    rdpdr::DeviceCreateRequest dcr(
                                smb2::FILE_READ_ATTRIBUTES,
                                0,
                                0,
                                smb2::FILE_SHARE_READ | smb2::FILE_SHARE_WRITE | smb2::FILE_SHARE_DELETE,
                                smb2::FILE_OPEN,
                                smb2::FILE_DIRECTORY_FILE,
                                6,
                                path);
    dcr.emit(out_stream);

    InStream chunk_create(out_stream.get_bytes());

    channel.receive(chunk_create);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 21);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::DeviceCreateResponse dcresponse;
    dcresponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(dcresponse.FileId(),     0x00000001);
    RED_CHECK_EQUAL(dcresponse.Information(), 0x01);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelLockControl)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000000,
                                0x00000001,
                                rdpdr::IRP_MJ_LOCK_CONTROL,
                                0x00000000);
    dior.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 21);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::ClientDriveLockControlResponse cdlcr;
    cdlcr.receive(stream_deviceIOCompletion);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileBasicInformation)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000001,
                                0x00000001,
                                rdpdr::IRP_MJ_QUERY_INFORMATION,
                                0x00000000);
    dior.emit(out_stream);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileBasicInformation);
    sdqir.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 56);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::ClientDriveQueryInformationResponse cdqir;
    cdqir.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(cdqir.Length, rdpdr::FILE_BASIC_INFORMATION_SIZE);
    fscc::FileBasicInformation fileBasicInformation;
    fileBasicInformation.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(fileBasicInformation.CreationTime(), 0x00000000);
    RED_CHECK_EQUAL(fileBasicInformation.LastAccessTime(), 0x00000000);
    RED_CHECK_EQUAL(fileBasicInformation.LastWriteTime(), 0x00000000);
    RED_CHECK_EQUAL(fileBasicInformation.ChangeTime(), 0x00000000);
    RED_CHECK_EQUAL(fileBasicInformation.FileAttributes(), 0x00000000);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileStandardInformation)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000001,
                                0x00000001,
                                rdpdr::IRP_MJ_QUERY_INFORMATION,
                                0x00000000);
    dior.emit(out_stream);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileStandardInformation);
    sdqir.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 42);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::ClientDriveQueryInformationResponse cdqir;
    cdqir.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(cdqir.Length, rdpdr::FILE_STANDARD_INFORMATION_SIZE);
    fscc::FileStandardInformation fsi;
    fsi.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(fsi.AllocationSize, 0x00000000);
    RED_CHECK_EQUAL(fsi.EndOfFile, 0x00000000);
    RED_CHECK_EQUAL(fsi.NumberOfLinks, 0x00000000);
    RED_CHECK_EQUAL(fsi.DeletePending, 0x0);
    RED_CHECK_EQUAL(fsi.Directory, 0x00);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileAttributeTagInformation)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000001,
                                0x00000001,
                                rdpdr::IRP_MJ_QUERY_INFORMATION,
                                0x00000000);
    dior.emit(out_stream);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileAttributeTagInformation);
    sdqir.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 28);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<uint32_t>(deviceIOResponse.IoStatus()), static_cast<uint32_t>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::ClientDriveQueryInformationResponse cdqir;
    cdqir.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(cdqir.Length, fscc::FileAttributeTagInformation::size());
    fscc::FileAttributeTagInformation fati;
    fati.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(fati.FileAttributes, 0x00000000);
    RED_CHECK_EQUAL(fati.ReparseTag, 0x00000000);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelClose)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000001,
                                0x00000001,
                                rdpdr::IRP_MJ_CLOSE,
                                0x00000000);
    dior.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 20);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
}

RED_AUTO_TEST_CASE(TestRDPDRChannelRead)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");
    fakeIODisk.fil_size = 4;

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0x00000001,
                                0x00000001,
                                0x00000001,
                                rdpdr::IRP_MJ_READ,
                                0x00000000);
    dior.emit(out_stream);

    rdpdr::DeviceReadRequest drr(4, 0);
    drr.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 24);
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
    rdpdr::DeviceIOResponse deviceIOResponse;
    deviceIOResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceIOResponse.DeviceId(),     0x00000001);
    RED_CHECK_EQUAL(deviceIOResponse.CompletionId(), 0x00000001);
    RED_CHECK_EQUAL(static_cast<int>(deviceIOResponse.IoStatus()), static_cast<int>(erref::NTSTATUS::STATUS_SUCCESS));
    rdpdr::DeviceReadResponse deviceReadResponse;
    deviceReadResponse.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(deviceReadResponse.Length,     4);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelDirectoryControl)
{
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeIODisk fakeIODisk;
    RDPDiskConfig config;
    config.add_drive("", rdpdr::RDPDR_DTYP_FILESYSTEM);
    ClientRDPDRChannel channel(to_verbose_flags(0x0), &callback, config);
    channel.set_api(&fakeIODisk);

    channel.paths.emplace(1, "test");

    StaticOutStream<512> out_stream;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_stream);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_DIRECTORY_CONTROL, rdpdr::IRP_MN_QUERY_DIRECTORY);
    dior.emit(out_stream);

    rdpdr::ServerDriveQueryDirectoryRequest sdqdr(rdpdr::FileDirectoryInformation, 0);
    sdqdr.emit(out_stream);

    InStream chunk(out_stream.get_bytes());

    channel.receive(chunk);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

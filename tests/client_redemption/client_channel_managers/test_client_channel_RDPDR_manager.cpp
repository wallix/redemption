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

#define RED_TEST_MODULE TestCLIPRDRChannelManager
#include "system/redemption_unit_tests.hpp"


#include "client_redemption/client_channel_managers/fake_client_mod.hpp"
#include "client_redemption/client_channel_managers/client_channel_RDPDR_manager.hpp"

namespace rdpdr {
    std::ostream& operator<<(std::ostream& out, rdpdr::Component const & e)
    {
        return out << +underlying_cast(e); // '+' for transform u8/s8 to int
    }
    std::ostream& operator<<(std::ostream& out, rdpdr::PacketId const & e)
    {
        return out << +underlying_cast(e); // '+' for transform u8/s8 to int
    }
}


RED_AUTO_TEST_CASE(TestRDPDRChannelInitialization)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);



    StaticOutStream<512> out_serverAnnounce;
    rdpdr::SharedHeader header_serverAnnounce(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE);
    header_serverAnnounce.emit(out_serverAnnounce);
    out_serverAnnounce.out_uint16_be(1);
    out_serverAnnounce.out_uint16_be(1);
    out_serverAnnounce.out_uint32_be(2);
    InStream chunk_serverAnnounce(out_serverAnnounce.get_data(), out_serverAnnounce.get_offset());

    manager.receive(chunk_serverAnnounce);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 2);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_clientIDConfirm(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_clientIDConfirm;
    header_clientIDConfirm.receive(stream_clientIDConfirm);
    RED_CHECK_EQUAL(header_clientIDConfirm.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_clientIDConfirm.packet_id, rdpdr::PAKID_CORE_CLIENTID_CONFIRM);

    pdu_data = client.stream();
    InStream stream_clientName(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_clientName;
    header_clientName.receive(stream_clientName);
    RED_CHECK_EQUAL(header_clientName.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_clientName.packet_id, rdpdr::PAKID_CORE_CLIENT_NAME);



    StaticOutStream<512> out_serverCapabilityRequest;
    rdpdr::SharedHeader header_serverCapabilityRequest(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY);
    header_serverCapabilityRequest.emit(out_serverCapabilityRequest);
    out_serverCapabilityRequest.out_uint16_le(0);
    out_serverCapabilityRequest.out_uint16_le(0);
    InStream chunk_serverCapabilityRequest(out_serverCapabilityRequest.get_data(), out_serverCapabilityRequest.get_offset());

    manager.receive(chunk_serverCapabilityRequest);

    StaticOutStream<512> out_serverClientIDConfirm;
    rdpdr::SharedHeader header_serverClientIDConfirm(rdpdr::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    header_serverClientIDConfirm.emit(out_serverClientIDConfirm);
    InStream chunk_serverClientIDConfirm(out_serverClientIDConfirm.get_data(), out_serverClientIDConfirm.get_offset());

    manager.receive(chunk_serverClientIDConfirm);

    RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);

    pdu_data = client.stream();
    InStream stream_clientCapability(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_clientCapability;
    header_clientCapability.receive(stream_clientCapability);
    RED_CHECK_EQUAL(header_clientCapability.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_clientCapability.packet_id, rdpdr::PAKID_CORE_CLIENT_CAPABILITY);

    pdu_data = client.stream();
    InStream stream_deviceListAnnounce(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceListAnnounce;
    header_deviceListAnnounce.receive(stream_deviceListAnnounce);
    RED_CHECK_EQUAL(header_deviceListAnnounce.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceListAnnounce.packet_id, rdpdr::PAKID_CORE_DEVICELIST_ANNOUNCE);
}


RED_AUTO_TEST_CASE(TestRDPDRChannelCreateFileOrDir)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 0, 1, rdpdr::IRP_MJ_CREATE, 0);
    dior.emit(out_create);

    const char * path = "/xd/fg";
    rdpdr::DeviceCreateRequest dcr(0x00120089,
      0,
      0,
      0x00000007,
      0x00000001,
      0x00000064,
      6,
      path);
      dcr.emit(out_create);

    InStream chunk_create(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk_create);

    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelLockControl)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 0, 1, rdpdr::IRP_MJ_LOCK_CONTROL, 0);
    dior.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileBasicInformation)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_QUERY_INFORMATION, 0);
    dior.emit(out_create);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileBasicInformation);
    sdqir.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileStandardInformation)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_QUERY_INFORMATION, 0);
    dior.emit(out_create);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileStandardInformation);
    sdqir.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelQueryInformationFileAttributeTagInformation)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_QUERY_INFORMATION, 0);
    dior.emit(out_create);

    rdpdr::ServerDriveQueryInformationRequest sdqir(rdpdr::FileAttributeTagInformation);
    sdqir.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelClose)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_CLOSE, 0);
    dior.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelRead)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_READ, 0);
    dior.emit(out_create);

    rdpdr::DeviceReadRequest drr(0, 0);
    drr.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

RED_AUTO_TEST_CASE(TestRDPDRChannelDirectoryControl)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    manager.fileSystemData.paths.emplace<int, std::string>(1, "test");

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 1, 1, rdpdr::IRP_MJ_DIRECTORY_CONTROL, rdpdr::IRP_MN_QUERY_DIRECTORY);
    dior.emit(out_create);

    rdpdr::ServerDriveQueryDirectoryRequest sdqdr(rdpdr::FileDirectoryInformation, 0);
    sdqdr.emit(out_create);

    InStream chunk(out_create.get_data(), out_create.get_offset());

    manager.receive(chunk);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_deviceIOCompletion(pdu_data->data, pdu_data->size);
    rdpdr::SharedHeader header_deviceIOCompletion;
    header_deviceIOCompletion.receive(stream_deviceIOCompletion);
    RED_CHECK_EQUAL(header_deviceIOCompletion.component, rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(header_deviceIOCompletion.packet_id, rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}






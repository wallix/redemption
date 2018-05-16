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
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    StaticOutStream<512> out_serverAnnounce;
    rdpdr::SharedHeader header_serverAnnounce(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE);
    header_serverAnnounce.emit(out_serverAnnounce);
    out_serverAnnounce.out_uint16_be(1);
    out_serverAnnounce.out_uint32_be(2);
    InStream chunk_serverAnnounce(out_serverAnnounce.get_data(), out_serverAnnounce.get_offset());

    manager.receive(chunk_serverAnnounce);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 2);
    RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_CLIENTID_CONFIRM);
    RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_CLIENT_NAME);


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
    RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_CLIENT_CAPABILITY);
    RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_DEVICELIST_ANNOUNCE);
}


RED_AUTO_TEST_CASE(TestRDPDRChannelCreateFileOrDir)
{
    FakeClient client;
    FakeIODisk fakeIODisk;
    ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);

    StaticOutStream<512> out_create;
    rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
    header.emit(out_create);

    rdpdr::DeviceIORequest dior(0, 0, 1, 0, 0);
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
    InStream stream(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream.in_uint16_le(), rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(client.get_next_pdu_type(), rdpdr::RDPDR_CTYP_CORE);
    RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
}

// RED_AUTO_TEST_CASE(TestRDPDRChannelLockControl)
// {
//     FakeClient client;
//     FakeIODisk fakeIODisk;
//     ClientChannelRDPDRManager manager(to_verbose_flags(0x0), &client, &fakeIODisk);
//
//     StaticOutStream<512> out_create;
//     rdpdr::SharedHeader header(rdpdr::Component::RDPDR_CTYP_CORE, rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST);
//     header.emit(out_create);
//
//     rdpdr::DeviceIORequest dior(0, 0, 1, 0, 0);
//     dior.emit(out_create);
//
//     const char * path = "/xd/fg";
//     rdpdr::DeviceCreateRequest dcr(0x00120089,
//       0,
//       0,
//       0x00000007,
//       0x00000001,
//       0x00000064,
//       6,
//       path);
//       dcr.emit(out_create);
//
//     InStream chunk_create(out_create.get_data(), out_create.get_offset());
//
//     manager.receive(chunk_create);
//     RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);
//     RED_CHECK_EQUAL(client.get_next_pdu_type(), rdpdr::RDPDR_CTYP_CORE);
//     RED_CHECK_EQUAL(client.get_next_pdu_sub_type(), rdpdr::PAKID_CORE_DEVICE_IOCOMPLETION);
// }


//     uint32_t formatIDs[] = { RDPECLIP::CF_TEXT };
//     std::string format_name("\0\0");
//     const uint16_t * formatListDataName[] = {reinterpret_cast<const uint16_t *>(format_name.data())};
//     const size_t size_names[] = {2};
//     RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListDataName, size_names, 1);
//     format_list_pdu_long.emit(out_FormatListPDU);
//     InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());
//
//     clientChannelCLIPRDRManager.receive(chunk_FormatListPDU, flag_channel);
//     RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);
//
//     RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
//
//     RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_LOCK_CLIPDATA);
//
//     RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
//     RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);
//
//
//     StaticOutStream<512> out_FormatDataResponse;
//     RDPECLIP::FormatDataResponsePDU_Text fdr(5);
//     fdr.emit(out_FormatDataResponse);
//     uint8_t data_resp[] = {97, 99, 97, 98};
//     out_FormatDataResponse.out_copy_bytes(data_resp, 5);
//     InStream chunk_FormatDataResponse(out_FormatDataResponse.get_data(), out_FormatDataResponse.get_offset());
//
//     clientChannelCLIPRDRManager.receive(chunk_FormatDataResponse, flag_channel);
//     RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);
//
//     RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_UNLOCK_CLIPDATA);





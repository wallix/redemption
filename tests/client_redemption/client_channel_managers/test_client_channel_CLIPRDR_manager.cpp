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
#include "client_redemption/client_channel_managers/client_channel_CLIPRDR_manager.hpp"



RED_AUTO_TEST_CASE(TestCLIPRDRChannelInitialization)
{
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    FakeClient client;
    FakeClientIOClipboard clip_io;
    ClientChannelCLIPRDRManager clientChannelCLIPRDRManager(to_verbose_flags(0x0), &client, &clip_io);

    StaticOutStream<512> out_clipboardCapabilitiesPDU;
    RDPECLIP::ClipboardCapabilitiesPDU clipboardCapabilitiesPDU(1, RDPECLIP::GeneralCapabilitySet::size());
    clipboardCapabilitiesPDU.emit(out_clipboardCapabilitiesPDU);
    RDPECLIP::GeneralCapabilitySet generalCapabilitySet(RDPECLIP::CB_CAPS_VERSION_1, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    generalCapabilitySet.emit(out_clipboardCapabilitiesPDU);
    InStream chunk_clipboardCapabilitiesPDU(out_clipboardCapabilitiesPDU.get_data(), out_clipboardCapabilitiesPDU.get_offset());

    clientChannelCLIPRDRManager.server_use_long_format_names = false;
    RED_CHECK_EQUAL(clientChannelCLIPRDRManager.server_use_long_format_names, false);
    clientChannelCLIPRDRManager.receive(chunk_clipboardCapabilitiesPDU, flag_channel);
    RED_CHECK_EQUAL(clientChannelCLIPRDRManager.server_use_long_format_names, true);


    RDPECLIP::ServerMonitorReadyPDU serverMonitorReadyPDU;
    StaticOutStream<512> out_serverMonitorReadyPDU;
    serverMonitorReadyPDU.emit(out_serverMonitorReadyPDU);
    InStream chunk_serverMonitorReadyPDU(out_serverMonitorReadyPDU.get_data(), out_serverMonitorReadyPDU.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_serverMonitorReadyPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 2);

    RED_CHECK_EQUAL(client.get_next_pdu_type()/*clipboard_caps_pdu.header.msgType()*/, RDPECLIP::CB_CLIP_CAPS);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_LIST);
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelCopyFromServerToCLient)
{
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    FakeClient client;
    FakeClientIOClipboard clip_io;
    ClientChannelCLIPRDRManager clientChannelCLIPRDRManager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &client, &clip_io);


    StaticOutStream<512> out_FormatListPDU;
    uint32_t formatIDs[] = { RDPECLIP::CF_TEXT };
    std::string format_name("\0\0");
    const uint16_t * formatListDataName[] = {reinterpret_cast<const uint16_t *>(format_name.data())};
    const size_t size_names[] = {2};
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListDataName, size_names, 1);
    format_list_pdu_long.emit(out_FormatListPDU);
    InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_FormatListPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_LOCK_CLIPDATA);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);


    StaticOutStream<512> out_FormatDataResponse;
    RDPECLIP::FormatDataResponsePDU_Text fdr(5);
    fdr.emit(out_FormatDataResponse);
    uint8_t data_resp[] = {97, 99, 97, 98};
    out_FormatDataResponse.out_copy_bytes(data_resp, sizeof(data_resp));
    InStream chunk_FormatDataResponse(out_FormatDataResponse.get_data(), out_FormatDataResponse.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_FormatDataResponse, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_UNLOCK_CLIPDATA);
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelCopyFromClientToServer)
{
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    FakeClient client;
    FakeClientIOClipboard clip_io;
    ClientChannelCLIPRDRManager clientChannelCLIPRDRManager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &client, &clip_io);

    clip_io._bufferTypeID = RDPECLIP::CF_TEXT;

    StaticOutStream<512> out_FormatListResponsePDU;
    RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
    formatListResponsePDU.emit(out_FormatListResponsePDU);
    InStream chunk_formatListResponsePDU(out_FormatListResponsePDU.get_data(), out_FormatListResponsePDU.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_formatListResponsePDU, flag_channel);


    StaticOutStream<512> out_lockClipboardDataPDU;
    RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
    lockClipboardDataPDU.emit(out_lockClipboardDataPDU);
    InStream chunk_lockClipboardDataPDU(out_lockClipboardDataPDU.get_data(), out_lockClipboardDataPDU.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_lockClipboardDataPDU, flag_channel);


    StaticOutStream<512> out_formatDataRequestPDU;
    RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(RDPECLIP::CF_TEXT);
    formatDataRequestPDU.emit(out_formatDataRequestPDU);
    InStream chunk_formatDataRequestPDU(out_formatDataRequestPDU.get_data(), out_formatDataRequestPDU.get_offset());

    clientChannelCLIPRDRManager.receive(chunk_formatDataRequestPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    RED_CHECK_EQUAL(client.get_next_pdu_type(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
}


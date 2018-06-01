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
    SessionReactor session_reactor;
    FakeClient client(session_reactor);
    FakeClientIOClipboard clip_io;
    ClientChannelCLIPRDRManager manager(/*to_verbose_flags(0x0)*/RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump, &client, &clip_io);


    StaticOutStream<512> out_clipboardCapabilitiesPDU;
    RDPECLIP::ClipboardCapabilitiesPDU clipboardCapabilitiesPDU(1, RDPECLIP::GeneralCapabilitySet::size());
    clipboardCapabilitiesPDU.emit(out_clipboardCapabilitiesPDU);
    RDPECLIP::GeneralCapabilitySet generalCapabilitySet(RDPECLIP::CB_CAPS_VERSION_1, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    generalCapabilitySet.emit(out_clipboardCapabilitiesPDU);
    InStream chunk_clipboardCapabilitiesPDU(out_clipboardCapabilitiesPDU.get_data(), out_clipboardCapabilitiesPDU.get_offset());

    manager.server_use_long_format_names = false;
    manager.receive(chunk_clipboardCapabilitiesPDU, flag_channel);
    RED_CHECK_EQUAL(manager.server_use_long_format_names, true);

    RDPECLIP::ServerMonitorReadyPDU serverMonitorReadyPDU;
    StaticOutStream<512> out_serverMonitorReadyPDU;
    serverMonitorReadyPDU.emit(out_serverMonitorReadyPDU);
    InStream chunk_serverMonitorReadyPDU(out_serverMonitorReadyPDU.get_data(), out_serverMonitorReadyPDU.get_offset());


    manager.receive(chunk_serverMonitorReadyPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 2);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 24);
    InStream stream_clipCaps(pdu_data->data, pdu_data->size);
    RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu;
    clipboard_caps_pdu.recv(stream_clipCaps);
    RED_CHECK_EQUAL(clipboard_caps_pdu.header.msgType(), RDPECLIP::CB_CLIP_CAPS);
    RED_CHECK_EQUAL(clipboard_caps_pdu.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(clipboard_caps_pdu.header.dataLen(), 16);
    RED_CHECK_EQUAL(clipboard_caps_pdu.cCapabilitiesSets, 1);
    RDPECLIP::GeneralCapabilitySet general_cap_set;
    general_cap_set.recv(stream_clipCaps);
    RED_CHECK_EQUAL(general_cap_set.capabilitySetType, RDPECLIP::CB_CAPSTYPE_GENERAL);
    RED_CHECK_EQUAL(general_cap_set.lengthCapability, RDPECLIP::GeneralCapabilitySet::size());
    RED_CHECK_EQUAL(general_cap_set.version_, RDPECLIP::CB_CAPS_VERSION_2);
    RED_CHECK_EQUAL(general_cap_set.generalFlags_, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS | RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

    pdu_data = client.stream();
    InStream stream_formatList(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 96);

    RDPECLIP::CliprdrHeader format_list_header;
    format_list_header.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_header.msgType(), RDPECLIP::CB_FORMAT_LIST);
    RED_CHECK_EQUAL(format_list_header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(format_list_header.dataLen(), 88);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long0;
    format_list_pdu_long0.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long0.formatIDs, 48026);
    RED_CHECK_EQUAL(format_list_pdu_long0.formatDataNameUTF16Len, 26);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long1;
    format_list_pdu_long1.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long1.formatIDs, 48025);
    RED_CHECK_EQUAL(format_list_pdu_long1.formatDataNameUTF16Len, 42);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long2;
    format_list_pdu_long2.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long2.formatIDs, 1);
    RED_CHECK_EQUAL(format_list_pdu_long2.formatDataNameUTF16Len, 2);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long3;
    format_list_pdu_long3.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long3.formatIDs, 3);
    RED_CHECK_EQUAL(format_list_pdu_long3.formatDataNameUTF16Len, 2);
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelCopyFromServerToCLient)
{
   const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                       CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
   SessionReactor session_reactor;
   FakeClient client(session_reactor);
   FakeClientIOClipboard clip_io;
   ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &client, &clip_io);

   StaticOutStream<512> out_FormatListPDU;
   RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, 6);
   format_list_header.emit(out_FormatListPDU);

   RDPECLIP::FormatListPDU_LongName format_list_pdu_long(RDPECLIP::CF_TEXT, "", 2);
   format_list_pdu_long.emit(out_FormatListPDU);

   InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());

   manager.receive(chunk_FormatListPDU, flag_channel);
   RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

   FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
   RED_CHECK_EQUAL(pdu_data->size, 8);
   InStream stream_formatListResponse(pdu_data->data, pdu_data->size);
   RDPECLIP::CliprdrHeader header_formatListResponse;
   header_formatListResponse.recv(stream_formatListResponse);
   RED_CHECK_EQUAL(header_formatListResponse.msgType(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
   RED_CHECK_EQUAL(header_formatListResponse.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
   RED_CHECK_EQUAL(header_formatListResponse.dataLen(), 0);

   pdu_data = client.stream();
   InStream stream_lockClipdata(pdu_data->data, pdu_data->size);
   RED_CHECK_EQUAL(pdu_data->size, 12);
   RDPECLIP::LockClipboardDataPDU lcd;
   lcd.recv(stream_lockClipdata);
   RED_CHECK_EQUAL(lcd.header.msgType(), RDPECLIP::CB_LOCK_CLIPDATA);
   RED_CHECK_EQUAL(lcd.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
   RED_CHECK_EQUAL(lcd.header.dataLen(), 4);
   RED_CHECK_EQUAL(lcd.streamDataID, 0x00000000);

   pdu_data = client.stream();
   RED_CHECK_EQUAL(pdu_data->size, 12);
   InStream stream_formataDataRequest(pdu_data->data, pdu_data->size);
   RDPECLIP::FormatDataRequestPDU fdreq;
   fdreq.recv(stream_formataDataRequest);
   RED_CHECK_EQUAL(fdreq.header.msgType(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
   RED_CHECK_EQUAL(fdreq.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
   RED_CHECK_EQUAL(fdreq.header.dataLen(), 4);
   RED_CHECK_EQUAL(fdreq.requestedFormatId, RDPECLIP::CF_TEXT);

   StaticOutStream<512> out_FormatDataResponse;
   RDPECLIP::FormatDataResponsePDU_Text fdr(5);
   fdr.emit(out_FormatDataResponse);
   uint8_t data_resp[] = {97, 99, 97, 98};
   out_FormatDataResponse.out_copy_bytes(data_resp, sizeof(data_resp));
   InStream chunk_FormatDataResponse(out_FormatDataResponse.get_data(), out_FormatDataResponse.get_offset());

   manager.receive(chunk_FormatDataResponse, flag_channel);
   RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);

   pdu_data = client.stream();
   RED_CHECK_EQUAL(pdu_data->size, 12);
   InStream stream_unlock(pdu_data->data, pdu_data->size);
   RDPECLIP::UnlockClipboardDataPDU ucdp;
   ucdp.recv(stream_unlock);
   RED_CHECK_EQUAL(ucdp.header.msgType(), RDPECLIP::CB_UNLOCK_CLIPDATA);
   RED_CHECK_EQUAL(ucdp.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
   RED_CHECK_EQUAL(ucdp.header.dataLen(), 4);
   RED_CHECK_EQUAL(ucdp.streamDataID, 0x00000000);
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelCopyFromClientToServer)
{
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    SessionReactor session_reactor;
    FakeClient client(session_reactor);
    FakeClientIOClipboard clip_io;
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &client, &clip_io);

    clip_io._bufferTypeID = RDPECLIP::CF_TEXT;
    clip_io._chunk = std::make_unique<uint8_t[]>(4);
    clip_io._chunk[0] = 97;
    clip_io._chunk[1] = 99;
    clip_io._chunk[2] = 97;
    clip_io._chunk[3] = 98;
    clip_io._cliboard_data_length = 4;

    StaticOutStream<512> out_FormatListResponsePDU;
    RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
    formatListResponsePDU.emit(out_FormatListResponsePDU);
    InStream chunk_formatListResponsePDU(out_FormatListResponsePDU.get_data(), out_FormatListResponsePDU.get_offset());

    manager.receive(chunk_formatListResponsePDU, flag_channel);


    StaticOutStream<512> out_lockClipboardDataPDU;
    RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
    lockClipboardDataPDU.emit(out_lockClipboardDataPDU);
    InStream chunk_lockClipboardDataPDU(out_lockClipboardDataPDU.get_data(), out_lockClipboardDataPDU.get_offset());

    manager.receive(chunk_lockClipboardDataPDU, flag_channel);


    StaticOutStream<512> out_formatDataRequestPDU;
    RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(RDPECLIP::CF_TEXT);
    formatDataRequestPDU.emit(out_formatDataRequestPDU);
    InStream chunk_formatDataRequestPDU(out_formatDataRequestPDU.get_data(), out_formatDataRequestPDU.get_offset());

    manager.receive(chunk_formatDataRequestPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_formatDataResponse(pdu_data->data, pdu_data->size);
    RDPECLIP::FormatDataResponsePDU_Text frd_text;
    frd_text.recv(stream_formatDataResponse);
    RED_CHECK_EQUAL(frd_text.header.msgType(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(frd_text.header.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(frd_text.header.dataLen(), 4);

    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint8(), 97);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint8(), 99);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint8(), 97);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint8(), 98);
}


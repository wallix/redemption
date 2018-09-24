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
   Copyright (C) Wallix 2018
   Author(s): Clément Moroldo
*/

#define RED_TEST_MODULE TestCLIPRDRChannelManager
#include "system/redemption_unit_tests.hpp"

#include <stdio.h>
#include <string.h>

#include "client_redemption/client_channel_managers/fake_client_mod.hpp"
#include "client_redemption/client_channel_managers/client_channel_CLIPRDR_manager.hpp"


RED_AUTO_TEST_CASE(TestCLIPRDRChannelInitialization)
{
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    SessionReactor session_reactor;
    char const * arg_ext = "arg";
    char const * argv[] = {arg_ext};
    int argc = 1;
    FakeClient client(session_reactor, argv, argc, to_verbose_flags(0x0));
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(/*to_verbose_flags(0x0)*/RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump, &client, &clip_io, conf);

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
    RED_CHECK_EQUAL(format_list_pdu_long0.formatID, 48025);
    RED_CHECK_EQUAL(format_list_pdu_long0.formatDataNameUTF16Len, 42);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long1;
    format_list_pdu_long1.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long1.formatID, 48026);
    RED_CHECK_EQUAL(format_list_pdu_long1.formatDataNameUTF16Len, 26);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long2;
    format_list_pdu_long2.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long2.formatID, 1);
    RED_CHECK_EQUAL(format_list_pdu_long2.formatDataNameUTF16Len, 2);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long3;
    format_list_pdu_long3.recv(stream_formatList);
    RED_CHECK_EQUAL(format_list_pdu_long3.formatID, 3);
    RED_CHECK_EQUAL(format_list_pdu_long3.formatDataNameUTF16Len, 2);
}







RED_AUTO_TEST_CASE(TestCLIPRDRChannelTextCopyFromServerToCLient)
{
    // Format List PDU
    StaticOutStream<512> out_FormatListPDU;
    // CliprdrHeader(uint16_t msgType, uint16_t msgFlags, )
    uint16_t msgType = RDPECLIP::CB_FORMAT_LIST;
    uint16_t msgFlags = 0;
    uint32_t dataLen = 6;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long(RDPECLIP::CF_TEXT, "", 0);
    RED_CHECK_EQUAL(format_list_pdu_long.len(), dataLen);

    RDPECLIP::CliprdrHeader format_list_header(msgType, msgFlags, format_list_pdu_long.len());
    format_list_header.emit(out_FormatListPDU);
    format_list_pdu_long.emit(out_FormatListPDU);

    uint8_t clip_data_part1[1592] = {0};
    uint8_t clip_data_part2[208]  = {0};
    uint8_t clip_data_total[1800] = {0};

    char clip_txt_part1[1592] = {0};
    char clip_txt_part2[208]  = {0};
    char clip_txt_total[1800] = {0};

    memset (clip_data_part1, 97, 1592);
    memset (clip_data_part2, 97, 208 );
    memset (clip_data_total, 97, 1800);
    memset (clip_txt_part1, 'a', 1592);
    memset (clip_txt_part2, 'a', 208 );
    memset (clip_txt_total, 'a', 1800);

    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST
                           | CHANNELS::CHANNEL_FLAG_FIRST
                           | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    SessionReactor session_reactor;
    char const * arg_ext = "arg";
    char const * argv[2] = {arg_ext};
    int argc = 1;
    FakeClient client(session_reactor, argv, argc, to_verbose_flags(0x0));
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});

    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr, &client, &clip_io, conf);
    InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());
    manager.receive(chunk_FormatListPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

//    // Format List Response PDU
    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 8);
    InStream stream_formatListResponse(pdu_data->data, pdu_data->size);
    RDPECLIP::CliprdrHeader header_formatListResponse;
    header_formatListResponse.recv(stream_formatListResponse);
    RED_CHECK_EQUAL(header_formatListResponse.msgType(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    RED_CHECK_EQUAL(header_formatListResponse.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(header_formatListResponse.dataLen(), 0);

    // Lock Clipboard Data PDU (Optional)
    pdu_data = client.stream();
    InStream stream_lockClipdata(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 12);
    RDPECLIP::LockClipboardDataPDU lcd;
    lcd.recv(stream_lockClipdata);
    RED_CHECK_EQUAL(lcd.header.msgType(), RDPECLIP::CB_LOCK_CLIPDATA);
    RED_CHECK_EQUAL(lcd.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(lcd.header.dataLen(), 4);
    RED_CHECK_EQUAL(lcd.streamDataID, 0x00000000);

    // Format Data Request PDU
    pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);

    InStream stream_formataDataRequest(pdu_data->data, pdu_data->size);
    RDPECLIP::FormatDataRequestPDU fdreq;
    fdreq.recv(stream_formataDataRequest);
    RED_CHECK_EQUAL(fdreq.header.msgType(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
    RED_CHECK_EQUAL(fdreq.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(fdreq.header.dataLen(), 4);
    RED_CHECK_EQUAL(fdreq.requestedFormatId, RDPECLIP::CF_TEXT);

    // Format Data Response PDU part 1
    StaticOutStream<1600> out_FormatDataResponsep_part1;

    RDPECLIP::FormatDataResponsePDU_Text fdr(sizeof(clip_data_total));
    fdr.emit(out_FormatDataResponsep_part1);

    out_FormatDataResponsep_part1.out_copy_bytes(clip_data_part1, sizeof(clip_data_part1));

    InStream chunk_FormatDataResponse_part1(out_FormatDataResponsep_part1.get_data(), out_FormatDataResponsep_part1.get_offset());

    manager.receive(chunk_FormatDataResponse_part1, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

    RED_CHECK_EQUAL(sizeof(clip_data_part1), manager._cb_buffers.size);
    RED_CHECK_EQUAL(sizeof(clip_data_total), manager._cb_buffers.sizeTotal);

    std::string data_sent_to_local_clipboard_part1(reinterpret_cast<char *>( manager._cb_buffers.data.get()));
    std::string data_sent_expected_part1(clip_txt_part1);
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);

    // Format Data Response PDU part 2
    StaticOutStream<1600> out_FormatDataResponsep_part2;
    out_FormatDataResponsep_part2.out_copy_bytes(clip_data_part2, sizeof(clip_data_part2));
    InStream chunk_FormatDataResponse_part2(out_FormatDataResponsep_part2.get_data(), out_FormatDataResponsep_part2.get_offset());
    manager.receive(chunk_FormatDataResponse_part2, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);

    RED_CHECK_EQUAL(0, manager._cb_buffers.size);
    RED_CHECK_EQUAL(0, manager._cb_buffers.sizeTotal);

    std::string data_sent_to_local_clipboard(reinterpret_cast<char *>( manager._cb_buffers.data.get()), sizeof(clip_txt_total));
    std::string data_sent_expected(clip_txt_total);
    RED_CHECK_EQUAL(data_sent_to_local_clipboard, data_sent_expected);

    // Unlock Clipboard Data PDU
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



RED_AUTO_TEST_CASE(TestCLIPRDRChannelTextCopyFromClientToServer)
{
    uint8_t clip_data_part1[1592] = {0};
    uint8_t clip_data_part2[208]  = {0};
    uint8_t clip_data_total[1800] = {0};

    char clip_txt_part1[1592] = {0};
    char clip_txt_part2[208]  = {0};
    char clip_txt_total[1800] = {0};

    memset (clip_data_part1, 97, 1592);
    memset (clip_data_part2, 97, 208 );
    memset (clip_data_total, 97, 1800);
    memset (clip_txt_part1, 'a', 1592);
    memset (clip_txt_part2, 'a', 208) ;
    memset (clip_txt_total, 'a', 1800);

    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    SessionReactor session_reactor;
    char const * arg_ext = "arg";
    char const * argv[2] = {arg_ext};
    int argc = 1;
    FakeClient client(session_reactor, argv, argc, to_verbose_flags(0x0));
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &client, &clip_io, conf);

    // COPY from clipboard
    clip_io._bufferTypeID = RDPECLIP::CF_TEXT;
    clip_io._chunk = std::make_unique<uint8_t[]>(sizeof(clip_data_total));
    for (size_t i = 0; i < sizeof(clip_data_total); i++) {
        clip_io._chunk[i] = clip_data_total[i];
    }
    clip_io._cliboard_data_length = sizeof(clip_data_total);
    manager.send_FormatListPDU();

    RED_CHECK_EQUAL(client.get_total_stream_produced(), 1);

    // Format List PDU
    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    InStream stream_formatListPDU(pdu_data->data, pdu_data->size);
    RDPECLIP::CliprdrHeader format_list_header;
    format_list_header.recv(stream_formatListPDU);
    RED_CHECK_EQUAL(format_list_header.msgType(), RDPECLIP::CB_FORMAT_LIST);
    RED_CHECK_EQUAL(format_list_header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(format_list_header.dataLen(), 6);
    RDPECLIP::FormatListPDU_LongName format;
    format.recv(stream_formatListPDU);
    RED_CHECK_EQUAL(format.formatID, RDPECLIP::CF_TEXT);
    RED_CHECK_EQUAL(format.formatDataNameUTF16Len, 2);

    // Format List Response PDU
    StaticOutStream<512> out_FormatListResponsePDU;
    RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
    formatListResponsePDU.emit(out_FormatListResponsePDU);
    InStream chunk_formatListResponsePDU(out_FormatListResponsePDU.get_data(), out_FormatListResponsePDU.get_offset());
    manager.receive(chunk_formatListResponsePDU, flag_channel);

    // Lock Clipboard Data PDU
    StaticOutStream<512> out_lockClipboardDataPDU;
    RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
    lockClipboardDataPDU.emit(out_lockClipboardDataPDU);
    InStream chunk_lockClipboardDataPDU(out_lockClipboardDataPDU.get_data(), out_lockClipboardDataPDU.get_offset());
    manager.receive(chunk_lockClipboardDataPDU, flag_channel);

    // Format Data Request PDU
    StaticOutStream<512> out_formatDataRequestPDU;
    RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(RDPECLIP::CF_TEXT);
    formatDataRequestPDU.emit(out_formatDataRequestPDU);
    InStream chunk_formatDataRequestPDU(out_formatDataRequestPDU.get_data(), out_formatDataRequestPDU.get_offset());
    manager.receive(chunk_formatDataRequestPDU, flag_channel);

    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

    // Format Data Response PDU par 1
    pdu_data = client.stream();
    InStream stream_formatDataResponse_part1(pdu_data->data, pdu_data->size);
    RDPECLIP::FormatDataResponsePDU_Text frd_text;
    frd_text.recv(stream_formatDataResponse_part1);
    RED_CHECK_EQUAL(frd_text.header.msgType(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(frd_text.header.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(frd_text.header.dataLen(), sizeof(clip_data_total));
    std::string data_sent_to_local_clipboard_part1(reinterpret_cast<const char *>( stream_formatDataResponse_part1.get_current()), sizeof(clip_txt_part1));
    std::string data_sent_expected_part1(clip_txt_part1, sizeof(clip_txt_part1));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);

    // Format Data Response PDU par 2
    pdu_data = client.stream();
    InStream stream_formatDataResponse_part2(pdu_data->data, pdu_data->size);
    std::string data_sent_to_local_clipboard_part2(reinterpret_cast<const char *>( stream_formatDataResponse_part2.get_current()), sizeof(clip_txt_part2));
    std::string data_sent_expected_part2(clip_txt_part2, sizeof(clip_txt_part2));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part2, data_sent_expected_part2);
}


RED_AUTO_TEST_CASE(TestCLIPRDRChannelFileCopyFromServerToCLient)
{
    uint8_t clip_data_part1[1588] = {0};
    uint8_t clip_data_part2[212]  = {0};
    uint8_t clip_data_total[1800] = {0};

    char clip_txt_part1[1588] = {0};
    char clip_txt_part2[212]  = {0};
    char clip_txt_total[1800] = {0};

    memset (clip_data_part1, 97, 1588);
    memset (clip_data_part2, 97, 212 );
    memset (clip_data_total, 97, 1800);
    memset (clip_txt_part1, 'a', 1588);
    memset (clip_txt_part2, 'a', 212 );
    memset (clip_txt_total, 'a', 1800);

    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
    SessionReactor session_reactor;
    char const * arg_ext = "arg";
    char const * argv[2] = {arg_ext};
    int argc = 1;
    FakeClient client(session_reactor, argv, argc, to_verbose_flags(0x0));
    FakeClientIOClipboard clip_io;
    clip_io.resize_chunk(sizeof(clip_data_total));
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr, &client, &clip_io, conf);

//    // Format List PDU
    StaticOutStream<512> out_FormatListPDU;

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long(
            ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW,
            RDPECLIP::FILEGROUPDESCRIPTORW.data(),
            RDPECLIP::FILEGROUPDESCRIPTORW.size());
    RDPECLIP::CliprdrHeader format_list_header(RDPECLIP::CB_FORMAT_LIST, 0, format_list_pdu_long.len());

    format_list_header.emit(out_FormatListPDU);
    format_list_pdu_long.emit(out_FormatListPDU);
    InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());
    manager.receive(chunk_FormatListPDU, flag_channel);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 3);

    // Format List Response PDU
    FakeRDPChannelsMod::PDUData * pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 8);
    InStream stream_formatListResponse(pdu_data->data, pdu_data->size);
    RDPECLIP::CliprdrHeader header_formatListResponse;
    header_formatListResponse.recv(stream_formatListResponse);
    RED_CHECK_EQUAL(header_formatListResponse.msgType(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    RED_CHECK_EQUAL(header_formatListResponse.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(header_formatListResponse.dataLen(), 0);

    // Lock Clipboard Data PDU (Optional)
    pdu_data = client.stream();
    InStream stream_lockClipdata(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 12);
    RDPECLIP::LockClipboardDataPDU lcd;
    lcd.recv(stream_lockClipdata);
    RED_CHECK_EQUAL(lcd.header.msgType(), RDPECLIP::CB_LOCK_CLIPDATA);
    RED_CHECK_EQUAL(lcd.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(lcd.header.dataLen(), 4);
    RED_CHECK_EQUAL(lcd.streamDataID, 0x00000000);

    // Format Data Request PDU
    pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_formataDataRequest(pdu_data->data, pdu_data->size);
    RDPECLIP::FormatDataRequestPDU fdreq;
    fdreq.recv(stream_formataDataRequest);
    RED_CHECK_EQUAL(fdreq.header.msgType(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
    RED_CHECK_EQUAL(fdreq.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(fdreq.header.dataLen(), 4);
    RED_CHECK_EQUAL(fdreq.requestedFormatId, ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);

    // Format Data Response PDU
    StaticOutStream<1600> out_FormatDataResponse;
    RDPECLIP::FormatDataResponsePDU_FileList fdr(1);
    fdr.emit(out_FormatDataResponse);
    RDPECLIP::FileDescriptor fd("file_name.name", sizeof(clip_data_total), 0);
    fd.emit(out_FormatDataResponse);
    InStream chunk_FormatDataResponse(out_FormatDataResponse.get_data(), out_FormatDataResponse.get_offset());
    manager.receive(chunk_FormatDataResponse, flag_channel);
    RED_CHECK_EQUAL(fdr.header.msgType(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(fdr.header.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(fdr.header.dataLen(), 596);
    RED_CHECK_EQUAL(manager.paste_data_len, sizeof(clip_data_total));
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 4);

    // File Content Request Size
    pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 36);
    InStream fileContentsRequest_size(pdu_data->data, pdu_data->size);
    RDPECLIP::FileContentsRequestPDU fcrq_s;
    fcrq_s.recv(fileContentsRequest_size);
    RED_CHECK_EQUAL(fcrq_s.header.msgType(), RDPECLIP::CB_FILECONTENTS_REQUEST);
    RED_CHECK_EQUAL(fcrq_s.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(fcrq_s.header.dataLen(), 28);
    RED_CHECK_EQUAL(fcrq_s.streamID, 1);
    RED_CHECK_EQUAL(fcrq_s.lindex, 0);
    RED_CHECK_EQUAL(fcrq_s.flag, RDPECLIP::FILECONTENTS_SIZE);
    RED_CHECK_EQUAL(fcrq_s.sizeRequested, 0);
    RED_CHECK_EQUAL(fcrq_s.cbRequested, 8);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);

    // File Content Response Size
    StaticOutStream<512> out_FormatDataResponse_size;
    RDPECLIP::FileContentsResponse_Size fcrp_s(1, sizeof(clip_data_total));
    fcrp_s.emit(out_FormatDataResponse_size);
    InStream chunk_FileContentResponse_size(out_FormatDataResponse_size.get_data(), out_FormatDataResponse_size.get_offset());
    manager.receive(chunk_FileContentResponse_size, flag_channel);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 5);

    // File Content Request Range
    pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 36);
    InStream fileContentsRequest_range(pdu_data->data, pdu_data->size);
    RDPECLIP::FileContentsRequestPDU fcrq_r;
    fcrq_r.recv(fileContentsRequest_range);
    RED_CHECK_EQUAL(fcrq_r.header.msgType(), RDPECLIP::CB_FILECONTENTS_REQUEST);
    RED_CHECK_EQUAL(fcrq_r.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(fcrq_r.header.dataLen(), 28);
    RED_CHECK_EQUAL(fcrq_r.streamID, 1);
    RED_CHECK_EQUAL(fcrq_r.lindex, 0);
    RED_CHECK_EQUAL(fcrq_r.flag, RDPECLIP::FILECONTENTS_RANGE);
    RED_CHECK_EQUAL(fcrq_r.sizeRequested, 0);
    RED_CHECK_EQUAL(fcrq_r.cbRequested, sizeof(clip_data_total));
    RED_CHECK_EQUAL(manager._waiting_for_data, true);
    RED_CHECK_EQUAL(0, clip_io.offset);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);

    // File Content Response Range part 1
    StaticOutStream<1600> out_FormatDataRequest_range_part1;
    RDPECLIP::FileContentsResponse_Range fcrp_r(1, sizeof(clip_data_total));
    fcrp_r.emit(out_FormatDataRequest_range_part1);
    out_FormatDataRequest_range_part1.out_copy_bytes(clip_data_part1, sizeof(clip_data_part1));
    InStream chunk_FileContentResponse_range_part1(
            out_FormatDataRequest_range_part1.get_data(),
            out_FormatDataRequest_range_part1.get_offset());
    manager.receive(chunk_FileContentResponse_range_part1, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);
    RED_CHECK_EQUAL(manager._waiting_for_data, true);
    RED_CHECK_EQUAL(sizeof(clip_data_part1), clip_io.offset);
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.size);
    std::string data_sent_to_local_clipboard_part1(reinterpret_cast<char *>( clip_io._chunk.get()),  sizeof(clip_txt_part1));
    std::string data_sent_expected_part1(clip_txt_part1,  sizeof(clip_txt_part1));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 5);

    // File Content Response Range part 2
    StaticOutStream<1600> out_FormatDataRequest_range_part2;
    out_FormatDataRequest_range_part2.out_copy_bytes(clip_data_part2, sizeof(clip_data_part2));
    InStream chunk_FileContentResponse_range_part2(
                        out_FormatDataRequest_range_part2.get_data(),
                        out_FormatDataRequest_range_part2.get_offset());
    manager.receive(chunk_FileContentResponse_range_part2, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);
    RED_CHECK_EQUAL(manager._waiting_for_data, false);
    RED_CHECK_EQUAL(client.get_total_stream_produced(), 6);

    // Unlock Clipboard Data PDU (Optional)
    pdu_data = client.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_unlock(pdu_data->data, pdu_data->size);
    RDPECLIP::UnlockClipboardDataPDU ucdp;
    ucdp.recv(stream_unlock);
    RED_CHECK_EQUAL(ucdp.header.msgType(), RDPECLIP::CB_UNLOCK_CLIPDATA);
    RED_CHECK_EQUAL(ucdp.header.msgFlags(), RDPECLIP::CB_RESPONSE_NONE);
    RED_CHECK_EQUAL(ucdp.header.dataLen(), 4);
    RED_CHECK_EQUAL(ucdp.streamDataID, 1);

    // Clip Data buffered
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.size);
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.offset);
    RED_CHECK_EQUAL("file_name.name", clip_io.fileName);
    std::string data_sent_to_local_clipboard_total(reinterpret_cast<char *>(clip_io._chunk.get()),  sizeof(clip_txt_total));
    std::string data_sent_expected_total(clip_txt_total,  sizeof(clip_txt_total));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_total, data_sent_expected_total);
}

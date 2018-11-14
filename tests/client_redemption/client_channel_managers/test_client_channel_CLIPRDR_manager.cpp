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
    LOG(LOG_INFO, "TestCLIPRDRChannelInitialization");
    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(/*to_verbose_flags(0x0)*/RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump, &callback, &clip_io, conf);
    manager.server_use_long_format_names = false;

    // Server Capabilities
    StaticOutStream<512> out_clipboardCapabilitiesPDU;
    out_clipboardCapabilitiesPDU.out_uint16_le(RDPECLIP::CB_CLIP_CAPS);
    out_clipboardCapabilitiesPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_clipboardCapabilitiesPDU.out_uint32_le(RDPECLIP::GeneralCapabilitySet::size() + 4);
    out_clipboardCapabilitiesPDU.out_uint16_le(1);
    out_clipboardCapabilitiesPDU.out_clear_bytes(2);
    out_clipboardCapabilitiesPDU.out_uint16_le(RDPECLIP::CB_CAPSTYPE_GENERAL);
    out_clipboardCapabilitiesPDU.out_uint16_le(RDPECLIP::GeneralCapabilitySet::size());
    out_clipboardCapabilitiesPDU.out_uint32_le(RDPECLIP::CB_CAPS_VERSION_1);
    out_clipboardCapabilitiesPDU.out_uint32_le(RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    InStream chunk_clipboardCapabilitiesPDU(out_clipboardCapabilitiesPDU.get_data(), out_clipboardCapabilitiesPDU.get_offset());

    manager.receive(chunk_clipboardCapabilitiesPDU, flag_channel);
    RED_CHECK_EQUAL(manager.server_use_long_format_names, true);

    // Monitor Ready PDU
    StaticOutStream<512> out_serverMonitorReadyPDU;
    out_serverMonitorReadyPDU.out_uint16_le(RDPECLIP::CB_MONITOR_READY);
    out_serverMonitorReadyPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_serverMonitorReadyPDU.out_uint32_le(0);
    InStream chunk_serverMonitorReadyPDU(out_serverMonitorReadyPDU.get_data(), out_serverMonitorReadyPDU.get_offset());

    manager.receive(chunk_serverMonitorReadyPDU, flag_channel);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 2);

    // Client Capabilities
    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 24);
    InStream stream_clipCaps(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint16_le(), RDPECLIP::CB_CLIP_CAPS);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint32_le(), 16);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint16_le(), 1);
    stream_clipCaps.in_skip_bytes(2);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint16_le(), RDPECLIP::CB_CAPSTYPE_GENERAL);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint16_le(), 12);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint32_le(), RDPECLIP::CB_CAPS_VERSION_2);
    RED_CHECK_EQUAL(stream_clipCaps.in_uint32_le(), RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS | RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

    // Format List PDU
    pdu_data = mod.stream();
    InStream stream_formatList(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 96);
    auto expected_init_format_list_pdu =
    /* 0000 */ "\x02\x00\x00\x00\x58\x00\x00\x00\x99\xbb\x00\x00\x46\x00\x69\x00" // ....X.......F.i.
    /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
    /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
    /* 0030 */ "\x72\x00\x57\x00\x00\x00\x9a\xbb\x00\x00\x46\x00\x69\x00\x6c\x00" // r.W.......F.i.l.
    /* 0040 */ "\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00\x74\x00" // e.C.o.n.t.e.n.t.
    /* 0050 */ "\x73\x00\x00\x00\x01\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00" // s...............
                ""_av
    ;
    RED_CHECK_MEM(expected_init_format_list_pdu, make_array_view(stream_formatList.get_data(), stream_formatList.get_capacity()));
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelTextCopyFromServerToCLient)
{
    LOG(LOG_INFO, "TestCLIPRDRChannelTextCopyFromServerToCLient");
    uint8_t clip_data_part1[1592] = {0};
    uint8_t clip_data_part2[208]  = {0};
    uint8_t clip_data_total[1800] = {0};

    char clip_txt_part1[1592 + 1] = {0};
    char clip_txt_total[1800] = {0};

    memset (clip_data_part1, 97, 1592);
    memset (clip_data_part2, 97, 208 );
    memset (clip_data_total, 97, 1800);
    memset (clip_txt_part1, 'a', 1592);
    memset (clip_txt_total, 'a', 1800);

    const int flag_channel = CHANNELS::CHANNEL_FLAG_LAST
                           | CHANNELS::CHANNEL_FLAG_FIRST
                           | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});

    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr, &callback, &clip_io, conf);

    // Format List PDU
    StaticOutStream<512> out_FormatListPDU;
    out_FormatListPDU.out_uint16_le(RDPECLIP::CB_FORMAT_LIST);
    out_FormatListPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_FormatListPDU.out_uint32_le(6);
    out_FormatListPDU.out_uint32_le(RDPECLIP::CF_TEXT);
    out_FormatListPDU.out_uint16_le(0);
    InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());
    manager.receive(chunk_FormatListPDU, flag_channel);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);

    // Format List Response PDU
    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 8);
    InStream stream_formatListResponse(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint16_le(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint32_le(), 0);

    // Lock Clipboard Data PDU (Optional)
    pdu_data = mod.stream();
    InStream stream_lockClipdata(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 12);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint16_le(), RDPECLIP::CB_LOCK_CLIPDATA);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint32_le(), 0x00000000);

    // Format Data Request PDU
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_formatDataRequest(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatDataRequest.in_uint16_le(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
    RED_CHECK_EQUAL(stream_formatDataRequest.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_formatDataRequest.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_formatDataRequest.in_uint32_le(), RDPECLIP::CF_TEXT);

    // Format Data Response PDU part 1
    StaticOutStream<1600> out_FormatDataResponsep_part1;
    out_FormatDataResponsep_part1.out_uint16_le(RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    out_FormatDataResponsep_part1.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_FormatDataResponsep_part1.out_uint32_le(sizeof(clip_data_total));
    out_FormatDataResponsep_part1.out_copy_bytes(clip_data_part1, sizeof(clip_data_part1));
    InStream chunk_FormatDataResponse_part1(out_FormatDataResponsep_part1.get_data(), out_FormatDataResponsep_part1.get_offset());
    manager.receive(chunk_FormatDataResponse_part1, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

    // Manager state check
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);
    RED_CHECK_EQUAL(sizeof(clip_data_part1), manager._cb_buffers.size);
    RED_CHECK_EQUAL(sizeof(clip_data_total), manager._cb_buffers.sizeTotal);
    std::string data_sent_to_local_clipboard_part1(char_ptr_cast(manager._cb_buffers.data.get()));
    std::string data_sent_expected_part1(clip_txt_part1);
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);

    // Format Data Response PDU part 2
    StaticOutStream<1600> out_FormatDataResponsep_part2;
    out_FormatDataResponsep_part2.out_copy_bytes(clip_data_part2, sizeof(clip_data_part2));
    InStream chunk_FormatDataResponse_part2(out_FormatDataResponsep_part2.get_data(), out_FormatDataResponsep_part2.get_offset());
    manager.receive(chunk_FormatDataResponse_part2, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 4);

    // Manager state check
    RED_CHECK_EQUAL(0, manager._cb_buffers.size);
    RED_CHECK_EQUAL(0, manager._cb_buffers.sizeTotal);

    // check clip io data
    std::string data_sent_to_local_clipboard(clip_io.data_text);
    std::string data_sent_expected(clip_txt_total, sizeof(clip_txt_total));
    RED_CHECK_EQUAL(clip_io.data_text, data_sent_expected);

    // Unlock Clipboard Data PDU
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_unlock(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_unlock.in_uint16_le(), RDPECLIP::CB_UNLOCK_CLIPDATA);
    RED_CHECK_EQUAL(stream_unlock.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_unlock.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_unlock.in_uint32_le(), 0x00000000);
}



RED_AUTO_TEST_CASE(TestCLIPRDRChannelTextCopyFromClientToServer)
{
    LOG(LOG_INFO, "TestCLIPRDRChannelTextCopyFromClientToServer");
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

    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &callback, &clip_io, conf);

    // COPY from clipboard
    clip_io._bufferTypeID = RDPECLIP::CF_TEXT;
    clip_io._chunk = std::make_unique<uint8_t[]>(sizeof(clip_data_total));
    for (size_t i = 0; i < sizeof(clip_data_total); i++) {
        clip_io._chunk[i] = clip_data_total[i];
    }
    clip_io._cliboard_data_length = sizeof(clip_data_total);
    manager.send_FormatListPDU();

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    // Format List PDU
    LOG(LOG_INFO, "Format List PDU");
    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    InStream stream_formatListPDU(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatListPDU.in_uint16_le(), RDPECLIP::CB_FORMAT_LIST);
    RED_CHECK_EQUAL(stream_formatListPDU.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_formatListPDU.in_uint32_le(), 6);
    RED_CHECK_EQUAL(stream_formatListPDU.in_uint32_le(), RDPECLIP::CF_TEXT);
    RED_CHECK_EQUAL(stream_formatListPDU.in_uint16_le(), 0);

    // Format List Response PDU
    LOG(LOG_INFO, "Format List Response PDU");
    StaticOutStream<512> out_FormatListResponsePDU;
    out_FormatListResponsePDU.out_uint16_le(RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    out_FormatListResponsePDU.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_FormatListResponsePDU.out_uint32_le(0);
    InStream chunk_formatListResponsePDU(out_FormatListResponsePDU.get_data(), out_FormatListResponsePDU.get_offset());
    manager.receive(chunk_formatListResponsePDU, flag_channel);

    // Lock Clipboard Data PDU
    StaticOutStream<512> out_lockClipboardDataPDU;
    out_lockClipboardDataPDU.out_uint16_le(RDPECLIP::CB_LOCK_CLIPDATA);
    out_lockClipboardDataPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_lockClipboardDataPDU.out_uint32_le(4);
    out_lockClipboardDataPDU.out_uint32_le(0);
    InStream chunk_lockClipboardDataPDU(out_lockClipboardDataPDU.get_data(), out_lockClipboardDataPDU.get_offset());
    manager.receive(chunk_lockClipboardDataPDU, flag_channel);

    // Format Data Request PDU
    StaticOutStream<512> out_formatDataRequestPDU;
    out_formatDataRequestPDU.out_uint16_le(RDPECLIP::CB_FORMAT_DATA_REQUEST);
    out_formatDataRequestPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_formatDataRequestPDU.out_uint32_le(4);
    out_formatDataRequestPDU.out_uint32_le(RDPECLIP::CF_TEXT);
    InStream chunk_formatDataRequestPDU(out_formatDataRequestPDU.get_data(), out_formatDataRequestPDU.get_offset());
    manager.receive(chunk_formatDataRequestPDU, flag_channel);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);

    // Format Data Response PDU par 1
    pdu_data = mod.stream();
    InStream stream_formatDataResponse_part1(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint16_le(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint32_le(), sizeof(clip_data_total));
    std::string data_sent_to_local_clipboard_part1(char_ptr_cast( stream_formatDataResponse_part1.get_current()), sizeof(clip_txt_part1));
    std::string data_sent_expected_part1(clip_txt_part1, sizeof(clip_txt_part1));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);

    // Format Data Response PDU par 2
    pdu_data = mod.stream();
    InStream stream_formatDataResponse_part2(pdu_data->data, pdu_data->size);
    std::string data_sent_to_local_clipboard_part2(char_ptr_cast( stream_formatDataResponse_part2.get_current()), sizeof(clip_txt_part2));
    std::string data_sent_expected_part2(clip_txt_part2, sizeof(clip_txt_part2));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part2, data_sent_expected_part2);
}


RED_AUTO_TEST_CASE(TestCLIPRDRChannelFileCopyFromServerToCLient)
{
    LOG(LOG_INFO, "TestCLIPRDRChannelFileCopyFromServerToCLient");

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
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientIOClipboard clip_io;
    clip_io.resize_chunk(sizeof(clip_data_total));
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr, &callback, &clip_io, conf);

    // Format List PDU
    StaticOutStream<512> out_FormatListPDU;
    out_FormatListPDU.out_uint16_le(RDPECLIP::CB_FORMAT_LIST);
    out_FormatListPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_FormatListPDU.out_uint32_le(46);
    out_FormatListPDU.out_uint32_le(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    uint8_t file_groupe_descr_data[50] = {0};
    size_t file_groupe_descr_size = ::UTF8toUTF16(RDPECLIP::FILEGROUPDESCRIPTORW, file_groupe_descr_data, RDPECLIP::FILEGROUPDESCRIPTORW.size() *2);
    out_FormatListPDU.out_copy_bytes(file_groupe_descr_data, file_groupe_descr_size+2);
    //out_FormatListPDU.out_uint16_le(0);
    InStream chunk_FormatListPDU(out_FormatListPDU.get_data(), out_FormatListPDU.get_offset());
    manager.receive(chunk_FormatListPDU, flag_channel);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);

    // Format List Response PDU
    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 8);
    InStream stream_formatListResponse(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint16_le(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_formatListResponse.in_uint32_le(), 0);

    // Lock Clipboard Data PDU (Optional)
    pdu_data = mod.stream();
    InStream stream_lockClipdata(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(pdu_data->size, 12);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint16_le(), RDPECLIP::CB_LOCK_CLIPDATA);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_lockClipdata.in_uint32_le(), 0x00000000);

    // Format Data Request PDU
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_formataDataRequest(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formataDataRequest.in_uint16_le(), RDPECLIP::CB_FORMAT_DATA_REQUEST);
    RED_CHECK_EQUAL(stream_formataDataRequest.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_formataDataRequest.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_formataDataRequest.in_uint32_le(), ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);

    // Format Data Response PDU
    StaticOutStream<1600> out_FormatDataResponse;
    out_FormatDataResponse.out_uint16_le(RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    out_FormatDataResponse.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_FormatDataResponse.out_uint32_le(596);

    out_FormatDataResponse.out_uint32_le(1);

    out_FormatDataResponse.out_uint32_le(RDPECLIP::FD_SHOWPROGRESSUI |RDPECLIP::FD_FILESIZE | RDPECLIP::FD_WRITESTIME | RDPECLIP::FD_ATTRIBUTES);
    out_FormatDataResponse.out_clear_bytes(32);
    out_FormatDataResponse.out_uint32_le(0);
    out_FormatDataResponse.out_clear_bytes(16);
    out_FormatDataResponse.out_uint64_le(RDPECLIP::TIME64_FILE_LIST);
    out_FormatDataResponse.out_uint32_le(0);
    out_FormatDataResponse.out_uint32_le(sizeof(clip_data_total));
    auto file_name = "file_name.name"_av;
    uint8_t utf16_file_name[520] = { 0 };
    size_t utf16_len = ::UTF8toUTF16(file_name, utf16_file_name, file_name.size() *2);
    out_FormatDataResponse.out_copy_bytes(utf16_file_name, utf16_len);
    out_FormatDataResponse.out_clear_bytes(520-utf16_len);
    InStream chunk_FormatDataResponse(out_FormatDataResponse.get_data(), 608/*out_FormatDataResponse.get_offset()*/);
    manager.receive(chunk_FormatDataResponse, flag_channel);

    // Manager and client state checks
    RED_CHECK_EQUAL(manager.paste_data_len, sizeof(clip_data_total));
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 4);

    // File Content Request Size
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 36);
    InStream fileContentsRequest_size(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint16_le(), RDPECLIP::CB_FILECONTENTS_REQUEST);
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 28);

    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 1); // streamId
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 0); // lindex
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), RDPECLIP::FILECONTENTS_SIZE); // dwFlags
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 0); // nPositionLow
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 0); // nPositionHigh
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 8); // cbRequested
    RED_CHECK_EQUAL(fileContentsRequest_size.in_uint32_le(), 0x00000000); // clipDataId

    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);

    // File Content Response Size
    StaticOutStream<512> out_fileContentsResponse_size;
    out_fileContentsResponse_size.out_uint16_le(RDPECLIP::CB_FILECONTENTS_RESPONSE);
    out_fileContentsResponse_size.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_fileContentsResponse_size.out_uint32_le(16);
    out_fileContentsResponse_size.out_uint32_le(1);
    out_fileContentsResponse_size.out_uint64_le(sizeof(clip_data_total));
    out_fileContentsResponse_size.out_uint32_le(0);
    InStream chunk_FileContentResponse_size(out_fileContentsResponse_size.get_data(), out_fileContentsResponse_size.get_offset());
    manager.receive(chunk_FileContentResponse_size, flag_channel);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 5);

    // File Content Request Range
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 36);
    InStream fileContentsRequest_range(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint16_le(), RDPECLIP::CB_FILECONTENTS_REQUEST);
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 28);

    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 1); // streamId
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 0); // lindex
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), RDPECLIP::FILECONTENTS_RANGE); // dwFlags
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), sizeof(clip_data_total)); // nPositionLow
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 0); //nPositionHigh
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 0);  // cbRequested
    RED_CHECK_EQUAL(fileContentsRequest_range.in_uint32_le(), 0x00000000); // clipDataId

    // Manager and clipboard_io checks
    RED_CHECK_EQUAL(manager._waiting_for_data, false);
    RED_CHECK_EQUAL(0, clip_io.offset);
    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);

    // File Content Response Range part 1
    StaticOutStream<1600> out_fileContentsResponse_range_part1;
    out_fileContentsResponse_range_part1.out_uint16_le(RDPECLIP::CB_FILECONTENTS_RESPONSE);
    out_fileContentsResponse_range_part1.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_fileContentsResponse_range_part1.out_uint32_le(4 + sizeof(clip_data_total));
    out_fileContentsResponse_range_part1.out_uint32_le(1);  // streamID
    out_fileContentsResponse_range_part1.out_copy_bytes(clip_data_part1, sizeof(clip_data_part1));

    InStream chunk_FileContentResponse_range_part1(
            out_fileContentsResponse_range_part1.get_data(),
            out_fileContentsResponse_range_part1.get_offset());

    manager.receive(chunk_FileContentResponse_range_part1, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_RANGE);
    RED_CHECK_EQUAL(manager._waiting_for_data, true);
    RED_CHECK_EQUAL(sizeof(clip_data_part1), clip_io.offset);
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.size);
    std::string data_sent_to_local_clipboard_part1(char_ptr_cast(clip_io._chunk.get()),  sizeof(clip_txt_part1));
    std::string data_sent_expected_part1(clip_txt_part1,  sizeof(clip_txt_part1));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 5);

    // File Content Response Range part 2
    StaticOutStream<1600> out_FormatDataRequest_range_part2;
    out_FormatDataRequest_range_part2.out_copy_bytes(clip_data_part2, sizeof(clip_data_part2));
    InStream chunk_FileContentResponse_range_part2(
                        out_FormatDataRequest_range_part2.get_data(),
                        out_FormatDataRequest_range_part2.get_offset());
    manager.receive(chunk_FileContentResponse_range_part2, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

    RED_CHECK_EQUAL(manager.file_content_flag, RDPECLIP::FILECONTENTS_SIZE);
    RED_CHECK_EQUAL(manager._waiting_for_data, false);
    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 6);

    // Unlock Clipboard Data PDU (Optional)
    pdu_data = mod.stream();
    RED_CHECK_EQUAL(pdu_data->size, 12);
    InStream stream_unlock(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_unlock.in_uint16_le(), RDPECLIP::CB_UNLOCK_CLIPDATA);
    RED_CHECK_EQUAL(stream_unlock.in_uint16_le(), RDPECLIP::CB_RESPONSE__NONE_);
    RED_CHECK_EQUAL(stream_unlock.in_uint32_le(), 4);
    RED_CHECK_EQUAL(stream_unlock.in_uint32_le(), 1);

    // Clip Data buffered
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.size);
    RED_CHECK_EQUAL(sizeof(clip_data_total), clip_io.offset);
    RED_CHECK_EQUAL("file_name.name", clip_io.fileName);
    std::string data_sent_to_local_clipboard_total(char_ptr_cast(clip_io._chunk.get()),  sizeof(clip_txt_total));
    std::string data_sent_expected_total(clip_txt_total,  sizeof(clip_txt_total));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_total, data_sent_expected_total);
}

RED_AUTO_TEST_CASE(TestCLIPRDRChannelFileCopyFromClientToServer)
{
    LOG(LOG_INFO, "TestCLIPRDRChannelFileCopyFromClientToServer");

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
    FakeRDPChannelsMod mod;
    ClientChannelMod callback;
    callback.set_mod(&mod);
    FakeClientIOClipboard clip_io;
    RDPClipboardConfig conf;
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    conf.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    conf.add_format(RDPECLIP::CF_TEXT, {});
    conf.add_format(RDPECLIP::CF_METAFILEPICT, {});
    ClientChannelCLIPRDRManager manager(RDPVerbose::cliprdr/*to_verbose_flags(0x0)*/, &callback, &clip_io, conf);

    // COPY from clipboard
    clip_io._bufferTypeID = ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
    clip_io._bufferTypeNameIndex = ClientIOClipboardAPI::FILEGROUPDESCRIPTORW_BUFFER_TYPE;
    clip_io.fileName = "filename.name";
    clip_io.size = sizeof(clip_data_total);
    clip_io._chunk = std::make_unique<uint8_t[]>(sizeof(clip_data_total));
    for (size_t i = 0; i < sizeof(clip_data_total); i++) {
        clip_io._chunk[i] = clip_data_total[i];
    }
    clip_io._cliboard_data_length = sizeof(clip_data_total);
    clip_io._cItems = 1;
    manager.send_FormatListPDU();

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 1);

    // Format List PDU
    FakeRDPChannelsMod::PDUData * pdu_data = mod.stream();
    InStream stream_formatListPDU(pdu_data->data, pdu_data->size);
    const char * expected_format_list_pdu =
    /* 0000 */ "\x02\x00\x00\x00\x2e\x00\x00\x00\x99\xbb\x00\x00\x46\x00\x69\x00" // ............F.i.
    /* 0010 */ "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" // l.e.G.r.o.u.p.D.
    /* 0020 */ "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" // e.s.c.r.i.p.t.o.
    /* 0030 */ "\x72\x00\x57\x00\x00\x00"                                         // r.W...
    ;
    std::string expected_format_list_str(expected_format_list_pdu, sizeof(expected_format_list_pdu));
    std::string format_list_str(char_ptr_cast(stream_formatListPDU.get_data()), sizeof(expected_format_list_pdu));
    RED_CHECK_EQUAL(expected_format_list_str, format_list_str);

    // Format List Response PDU
    StaticOutStream<512> out_FormatListResponsePDU;
    out_FormatListResponsePDU.out_uint16_le(RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    out_FormatListResponsePDU.out_uint16_le(RDPECLIP::CB_RESPONSE_OK);
    out_FormatListResponsePDU.out_uint32_le(0);
    InStream chunk_formatListResponsePDU(out_FormatListResponsePDU.get_data(), out_FormatListResponsePDU.get_offset());
    manager.receive(chunk_formatListResponsePDU, flag_channel);

    // Lock Clipboard Data PDU
    StaticOutStream<512> out_lockClipboardDataPDU;
    out_lockClipboardDataPDU.out_uint16_le(RDPECLIP::CB_LOCK_CLIPDATA);
    out_lockClipboardDataPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_lockClipboardDataPDU.out_uint32_le(4);
    out_lockClipboardDataPDU.out_uint32_le(0);
    InStream chunk_lockClipboardDataPDU(out_lockClipboardDataPDU.get_data(), out_lockClipboardDataPDU.get_offset());
    manager.receive(chunk_lockClipboardDataPDU, flag_channel);

    // Format Data Request PDU
    StaticOutStream<512> out_formatDataRequestPDU;
    out_formatDataRequestPDU.out_uint16_le(RDPECLIP::CB_FORMAT_DATA_REQUEST);
    out_formatDataRequestPDU.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_formatDataRequestPDU.out_uint32_le(4);
    out_formatDataRequestPDU.out_uint32_le(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW);
    InStream chunk_formatDataRequestPDU(out_formatDataRequestPDU.get_data(), out_formatDataRequestPDU.get_offset());
    manager.receive(chunk_formatDataRequestPDU, flag_channel);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 2);

    // Format Data Response PDU
    pdu_data = mod.stream();
    InStream stream_formatDataResponse(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint16_le(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), 596);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), 1);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), RDPECLIP::FD_SHOWPROGRESSUI |RDPECLIP::FD_FILESIZE | RDPECLIP::FD_WRITESTIME | RDPECLIP::FD_ATTRIBUTES);
    stream_formatDataResponse.in_skip_bytes(32);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), fscc::FILE_ATTRIBUTE_ARCHIVE);
    stream_formatDataResponse.in_skip_bytes(16);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint64_le(), RDPECLIP::TIME64_FILE_LIST);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), 0);
    RED_CHECK_EQUAL(stream_formatDataResponse.in_uint32_le(), sizeof(clip_data_total));

    // File Content Request Size
    StaticOutStream<512> out_fileContentsRequest_size;
    out_fileContentsRequest_size.out_uint16_le(RDPECLIP::CB_FILECONTENTS_REQUEST);
    out_fileContentsRequest_size.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_fileContentsRequest_size.out_uint32_le(24);
    out_fileContentsRequest_size.out_uint32_le(1);
    out_fileContentsRequest_size.out_uint32_le(0);
    out_fileContentsRequest_size.out_uint32_le(RDPECLIP::FILECONTENTS_SIZE);
    out_fileContentsRequest_size.out_uint32_le(0);            // low bits size
    out_fileContentsRequest_size.out_uint32_le(0);            // hight bits size
    out_fileContentsRequest_size.out_uint32_le(0x00000008);
    InStream chunk_fileContentsRequest_size(out_fileContentsRequest_size.get_data(), out_fileContentsRequest_size.get_offset());
    manager.receive(chunk_fileContentsRequest_size, flag_channel);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 3);

    // File Content Response Size
    pdu_data = mod.stream();
    InStream stream_fileContentsResponse_Size(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint16_le(), RDPECLIP::CB_FILECONTENTS_RESPONSE);
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint32_le(), 16);
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint32_le(), 1);
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint32_le(), sizeof(clip_data_total));
    RED_CHECK_EQUAL(stream_fileContentsResponse_Size.in_uint32_le(), 0);

    // File Content Request Range
    StaticOutStream<512> out_fileContentsRequest_Range;
    out_fileContentsRequest_Range.out_uint16_le(RDPECLIP::CB_FILECONTENTS_REQUEST);
    out_fileContentsRequest_Range.out_uint16_le(RDPECLIP::CB_RESPONSE__NONE_);
    out_fileContentsRequest_Range.out_uint32_le(24);
    out_fileContentsRequest_Range.out_uint32_le(1);
    out_fileContentsRequest_Range.out_uint32_le(0);
    out_fileContentsRequest_Range.out_uint32_le(RDPECLIP::FILECONTENTS_RANGE);
    out_fileContentsRequest_Range.out_uint32_le(0);            // low bits size
    out_fileContentsRequest_Range.out_uint32_le(0);            // hight bits size
    out_fileContentsRequest_Range.out_uint32_le(65536);
    InStream chunk_fileContentsRequest_range(out_fileContentsRequest_Range.get_data(), out_fileContentsRequest_Range.get_offset());
    manager.receive(chunk_fileContentsRequest_range, flag_channel);

    RED_CHECK_EQUAL(mod.get_total_stream_produced(), 5);

    // Format Data Response PDU par 1
    pdu_data = mod.stream();
    InStream stream_formatDataResponse_part1(pdu_data->data, pdu_data->size);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint16_le(), RDPECLIP::CB_FILECONTENTS_RESPONSE);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint16_le(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint32_le(), sizeof(clip_data_total)+4);
    RED_CHECK_EQUAL(stream_formatDataResponse_part1.in_uint32_le(), 1);
    std::string data_sent_to_local_clipboard_part1(char_ptr_cast( stream_formatDataResponse_part1.get_current()), sizeof(clip_txt_part1));
    std::string data_sent_expected_part1(clip_txt_part1, sizeof(clip_txt_part1));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part1, data_sent_expected_part1);

    // Format Data Response PDU par 2
    pdu_data = mod.stream();
    InStream stream_formatDataResponse_part2(pdu_data->data, pdu_data->size);
    std::string data_sent_to_local_clipboard_part2(char_ptr_cast( stream_formatDataResponse_part2.get_current()), sizeof(clip_txt_part2));
    std::string data_sent_expected_part2(clip_txt_part2, sizeof(clip_txt_part2));
    RED_CHECK_EQUAL(data_sent_to_local_clipboard_part2, data_sent_expected_part2);
}


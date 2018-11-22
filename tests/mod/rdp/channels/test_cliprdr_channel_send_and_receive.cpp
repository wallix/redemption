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
    Author(s): Clément Moroldo
*/


#define RED_TEST_MODULE TestCLIPRDRChannel
#include "system/redemption_unit_tests.hpp"

#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"
// #include "mod/rdp/channels/fake_clipboard_data.hpp"



RED_AUTO_TEST_CASE(TestCliprdrChannelClipboardCapabilitiesReceive)
{
    StaticOutStream<64> stream;

    stream.out_uint16_le(1); // cCapabilitiesSets
    stream.out_uint16_le(0); // pad1(2)

    RDPECLIP::GeneralCapabilitySet caps(RDPECLIP::CB_CAPS_VERSION_1, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    caps.emit(stream);

    InStream chunk(stream.get_data(), stream.get_offset());

    ClipboardSideData state("client");

    ClipboardCapabilitiesReceive receiver(state, chunk, RDPVerbose::none);

    RED_CHECK_EQUAL(state.use_long_format_names, true);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelFilecontentsRequestReceive)
{
    StaticOutStream<64> stream;

    RDPECLIP::FileContentsRequestPDU file_contents_request_pdu(1, 2, RDPECLIP::FILECONTENTS_RANGE, 4, 0, 6, 7, true);

    file_contents_request_pdu.emit(stream);

    ClipboardSideData state("client");

    InStream chunk(stream.get_data(), 28);

    FilecontentsRequestReceive receiver(state, chunk, RDPVerbose::cliprdr, 28);

    RED_CHECK_EQUAL(receiver.dwFlags, RDPECLIP::FILECONTENTS_RANGE);
    RED_REQUIRE_EQUAL(receiver.streamID, 1 );

    ClipboardSideData::file_contents_request_info & fcri = state.file_contents_request_info_inventory[receiver.streamID];

    RED_CHECK_EQUAL(fcri.lindex, 2);
    RED_CHECK_EQUAL(fcri.position, 4);
    RED_CHECK_EQUAL(fcri.cbRequested, 6);
    RED_CHECK_EQUAL(fcri.clipDataId, 7);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelFilecontentsRequestSend)
{
//     ClipboardSideData server_state("server");
//
//     FilecontentsRequestSendBack sender(server_state, RDPVerbose::none, RDPECLIP::FILECONTENTS_SIZE, 1);
//
//     InStream stream(sender.out_stream.get_data(), sender.out_stream.get_offset());
//
//     RDPECLIP::CliprdrHeader header;
//     header.recv(stream);
//     RDPECLIP::FileContentsResponseSize pdu;
//     pdu.receive(stream);
//
//     RED_CHECK_EQUAL(header.msgType(), RDPECLIP::CB_FILECONTENTS_RESPONSE);
//     RED_CHECK_EQUAL(header.msgFlags(), RDPECLIP::CB_RESPONSE_FAIL);
//     RED_CHECK_EQUAL(header.dataLen(), 16);
//
//     RED_CHECK_EQUAL(pdu.streamID, 1);
//     RED_CHECK_EQUAL(pdu._size, 0);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataRequestReceive)
{
    StaticOutStream<64> stream;

    RDPECLIP::FileContentsRequestPDU file_contents_request_pdu(1, 2, 3, 4, 0, 6, 7, true);

    stream.out_uint32_le(3);

    InStream chunk(stream.get_data(), 28);

    ClipboardData state;

    FormatDataRequestReceive receiver(state, state.server_data, RDPVerbose::none, chunk);

    RED_CHECK_EQUAL(state.requestedFormatId, 3);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataRequestSend)
{
    ClientFormatDataRequestSendBack sender(RDPVerbose::none);

    InStream stream(sender.out_stream.get_data(), sender.out_stream.get_offset());

    RDPECLIP::CliprdrHeader header;
    header.recv(stream);

    RED_CHECK_EQUAL(header.msgType(), RDPECLIP::CB_FORMAT_DATA_RESPONSE);
    RED_CHECK_EQUAL(header.msgFlags(), RDPECLIP::CB_RESPONSE_FAIL);
    RED_CHECK_EQUAL(header.dataLen(), 0);

    RED_CHECK_EQUAL(sender.total_length, 8);
    RED_CHECK_EQUAL(sender.flags, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataResponseReceive)
{
    {
    std::string text("text de test");
    uint32_t cItems = 0;
    uint8_t utf16text[50] = {0};
    size_t utf16size = ::UTF8toUTF16(text, utf16text, text.length() *2);

    StaticOutStream<1600> out_stream;
    out_stream.out_copy_bytes(utf16text, utf16size+2);


    InStream stream(out_stream.get_data(), out_stream.get_offset());

    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE,
                                   RDPECLIP::CB_RESPONSE_OK,
                                   text.length());
    bool param_dont_log_data_into_syslog = false;
    uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
    StaticOutStream<1600> file_descriptor_stream;
    RDPVerbose verbose = RDPVerbose::none;

    ClipboardData clip_data;
    clip_data.requestedFormatId = RDPECLIP::CF_UNICODETEXT;

    ClientFormatDataResponseReceive receiver(clip_data.client_data,
                                             clip_data,
                                             stream,
                                             header,
                                             param_dont_log_data_into_syslog,
                                             flags,
                                             verbose);

    RED_CHECK_EQUAL(receiver.data_to_dump, "text de test");
    RED_CHECK_EQUAL(clip_data.client_data.file_stream_data_inventory.size(), cItems);
    }

    {
    StaticOutStream<1600> out_stream;

    std::string file_name_1("file1.txt");
    std::string file_name_2("file2.txt");
    uint32_t cItems = 2;
    size_t file_size_1 = 42;
    size_t file_size_2 = 84;
    RDPECLIP::FileDescriptor fd1(file_name_1, file_size_1, fscc::FILE_ATTRIBUTE_ARCHIVE);
    RDPECLIP::FileDescriptor fd2(file_name_2, file_size_2, fscc::FILE_ATTRIBUTE_ARCHIVE);

    out_stream.out_uint32_le(cItems);
    fd1.emit(out_stream);
    fd2.emit(out_stream);

    InStream stream(out_stream.get_data(), out_stream.get_offset());

    RDPECLIP::CliprdrHeader header (RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, (cItems*RDPECLIP::FileDescriptor::size())+4);
    bool param_dont_log_data_into_syslog = false;
    uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
    StaticOutStream<1600> file_descriptor_stream;
    RDPVerbose verbose = RDPVerbose::none;

    uint32_t clipDataId = 7;
    uint32_t requestedFormatId = 48025;

    ClipboardData clip_data;
    clip_data.requestedFormatId = requestedFormatId;
    clip_data.client_data.file_list_format_id = requestedFormatId;
    uint64_t sequential_access_offset = 0;
    clip_data.client_data.clipDataId = clipDataId;
    std::vector<ClipboardSideData::file_info_type> file_info_type_init;
    clip_data.client_data.file_stream_data_inventory[clipDataId] = file_info_type_init;

    ClientFormatDataResponseReceive receiver(clip_data.client_data,
                                             clip_data,
                                             stream,
                                             header,
                                             param_dont_log_data_into_syslog,
                                             flags,
                                             verbose);

    RED_CHECK_EQUAL(receiver.data_to_dump, "");
    std::vector<ClipboardSideData::file_info_type> & file_info_type_vec = clip_data.client_data.file_stream_data_inventory[clip_data.client_data.clipDataId];
    RED_REQUIRE_EQUAL(file_info_type_vec.size(), cItems);

    ClipboardSideData::file_info_type & file_info_type_1 = file_info_type_vec[0];
    RED_CHECK_EQUAL(file_info_type_1.size, 42);
    RED_CHECK_EQUAL(file_info_type_1.file_name, file_name_1);
    RED_CHECK_EQUAL(file_info_type_1.sequential_access_offset, sequential_access_offset);

    ClipboardSideData::file_info_type & file_info_type_2 = file_info_type_vec[1];
    RED_CHECK_EQUAL(file_info_type_2.size, 84);
    RED_CHECK_EQUAL(file_info_type_2.file_name, file_name_2);
    RED_CHECK_EQUAL(file_info_type_2.sequential_access_offset, sequential_access_offset);
    }
    {
    StaticOutStream<1600> pre_stream;

    std::string file_name_1("file1.txt");
    std::string file_name_2("file2.txt");
    uint32_t cItems = 2;
    RDPECLIP::FileDescriptor fd1(file_name_1, 42, fscc::FILE_ATTRIBUTE_ARCHIVE);
    RDPECLIP::FileDescriptor fd2(file_name_2, 84, fscc::FILE_ATTRIBUTE_ARCHIVE);

    fd1.emit(pre_stream);
    fd2.emit(pre_stream);

    InStream stream(pre_stream.get_data(), cItems*RDPECLIP::FileDescriptor::size());

    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, (cItems*RDPECLIP::FileDescriptor::size())+4);
    bool param_dont_log_data_into_syslog = false;
    uint32_t flags = CHANNELS::CHANNEL_FLAG_LAST;

    RDPVerbose verbose = RDPVerbose::none;

    uint32_t clipDataId = 7;
    uint32_t requestedFormatId = 48025;

    ClipboardData clip_data;
    clip_data.requestedFormatId = requestedFormatId;
    clip_data.client_data.file_list_format_id = requestedFormatId;
    uint64_t sequential_access_offset = 0;
    clip_data.client_data.clipDataId = clipDataId;
    std::vector<ClipboardSideData::file_info_type> file_info_type_init;
    clip_data.client_data.file_stream_data_inventory[clipDataId] = file_info_type_init;
    size_t size_part_1 = 150;
    clip_data.client_data.file_descriptor_stream.rewind();
    clip_data.client_data.file_descriptor_stream.out_copy_bytes(pre_stream.get_data(), size_part_1);
    stream.in_skip_bytes(size_part_1);

    ClientFormatDataResponseReceive receiver(clip_data.client_data,
                                             clip_data,
                                             stream,
                                             header,
                                             param_dont_log_data_into_syslog,
//                                              client_file_list_format_id,
                                             flags,
//                                              file_descriptor_stream,
                                             verbose);

    RED_CHECK_EQUAL(receiver.data_to_dump, "");
    std::vector<ClipboardSideData::file_info_type> & file_info_type_vec = clip_data.client_data.file_stream_data_inventory[clip_data.client_data.clipDataId];
    RED_REQUIRE_EQUAL(file_info_type_vec.size(), cItems);

    ClipboardSideData::file_info_type & file_info_type_1 = file_info_type_vec[0];
    RED_CHECK_EQUAL(file_info_type_1.size, 42);
    RED_CHECK_EQUAL(file_info_type_1.file_name, file_name_1);
    RED_CHECK_EQUAL(file_info_type_1.sequential_access_offset, sequential_access_offset);

    ClipboardSideData::file_info_type & file_info_type_2 = file_info_type_vec[1];
    RED_CHECK_EQUAL(file_info_type_2.size, 84);
    RED_CHECK_EQUAL(file_info_type_2.file_name, file_name_2);
    RED_CHECK_EQUAL(file_info_type_2.sequential_access_offset, sequential_access_offset);
    }
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListReceive) {

    std::map<uint32_t, std::string> format_name_inventory;

    const bool use_long_format_name = true;

    StaticOutStream<1600> stream;
    uint32_t client_file_list_format_id = 49263;

    RDPECLIP::FormatListPDUEx fl;
    fl.add_format_name(RDPECLIP::CF_TEXT, "");
    fl.add_format_name(client_file_list_format_id, RDPECLIP::FILEGROUPDESCRIPTORW.data());

    RDPECLIP::CliprdrHeader in_header(RDPECLIP::CB_FORMAT_LIST, RDPECLIP::CB_RESPONSE__NONE_, fl.size(use_long_format_name));
    fl.emit(stream, use_long_format_name);

    InStream chunk(stream.get_data(), stream.get_offset());

    ClientFormatListReceive received(use_long_format_name, use_long_format_name, in_header, chunk, format_name_inventory, RDPVerbose::none);

    RED_CHECK_EQUAL(received.client_file_list_format_id, client_file_list_format_id);
    RED_CHECK_EQUAL(format_name_inventory[RDPECLIP::CF_TEXT], "");
    RED_CHECK_EQUAL(format_name_inventory[client_file_list_format_id], RDPECLIP::FILEGROUPDESCRIPTORW.data());
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListSend) {

    FormatListSendBack sender;

    InStream stream(sender.out_stream.get_data(), sender.out_stream.get_offset());

    RDPECLIP::CliprdrHeader header;
    header.recv(stream);

    RED_CHECK_EQUAL(header.msgType(), RDPECLIP::CB_FORMAT_LIST_RESPONSE);
    RED_CHECK_EQUAL(header.msgFlags(), RDPECLIP::CB_RESPONSE_OK);
    RED_CHECK_EQUAL(header.dataLen(), 0);
}


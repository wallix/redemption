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


#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"


struct FakeDataSender : VirtualChannelDataSender
{
    struct PDUData
    {
        uint8_t data[1600] = {0};
        size_t size = 0;

        const_bytes_view av() const noexcept
        {
            return {data, size};
        }
    };

    std::array<PDUData, 2> streams;
    int index = 0;

    void operator()(
        uint32_t /*total_length*/, uint32_t /*flags*/,
        const_bytes_view chunk_data) override
    {
        RED_REQUIRE(this->index < this->streams.size());
        RED_REQUIRE(chunk_data.size() < std::size(streams[this->index].data));
        this->streams[this->index].size = chunk_data.size();
        std::memcpy(streams[this->index].data, chunk_data.data(), chunk_data.size());
        ++this->index;
    }
};


RED_AUTO_TEST_CASE(TestCliprdrChannelClipboardCapabilitiesReceive)
{
    StaticOutStream<64> stream;

    stream.out_uint16_le(1); // cCapabilitiesSets
    stream.out_uint16_le(0); // pad1(2)

    RDPECLIP::GeneralCapabilitySet caps(RDPECLIP::CB_CAPS_VERSION_1, RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    caps.emit(stream);

    InStream chunk(stream.get_bytes());

    ClipboardSideData state;

    ClipboardCapabilitiesReceive receiver(state, chunk, RDPVerbose::none);

    RED_CHECK(state.use_long_format_names);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelFilecontentsRequestSend)
{
    FakeDataSender data_sender;
    const uint32_t streamID = 1;

    FilecontentsRequestSendBack sender(RDPECLIP::FILECONTENTS_SIZE, streamID, &data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK_MEM(data_sender.streams[0].av(),
        "\x09\x00\x02\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00"
        ""_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataRequestReceive)
{
    StaticOutStream<64> stream;

    RDPECLIP::FileContentsRequestPDU file_contents_request_pdu(1, 2, 3, 4, 0, 6, 7, true);

    stream.out_uint32_le(3);

    InStream chunk(stream.get_bytes());

    ClipboardData state;

    FormatDataRequestReceive receiver(state, RDPVerbose::none, chunk);

    RED_CHECK_EQUAL(state.requestedFormatId, 3);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataRequestSend)
{
    FakeDataSender data_sender;

    FormatDataRequestSendBack sender(&data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK_MEM(data_sender.streams[0].av(), "\x05\x00\x02\x00\x00\x00\x00\x00"_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataResponseReceive)
{
    {
    auto text = "text de test"_av;
    uint8_t utf16text[50] = {0};
    size_t utf16size = ::UTF8toUTF16(text, utf16text, text.size() *2);

    StaticOutStream<1600> out_stream;
    out_stream.out_copy_bytes(utf16text, utf16size+2);

    InStream stream(out_stream.get_bytes());

    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE,
                                   RDPECLIP::CB_RESPONSE_OK,
                                   text.size());
    uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

    ClipboardData clip_data;
    clip_data.requestedFormatId = RDPECLIP::CF_UNICODETEXT;

    FormatDataResponseReceive receiver(clip_data.requestedFormatId, stream, flags);
    clip_data.requestedFormatId = 0;

    RED_CHECK_EQUAL(receiver.data_to_dump, "text de test");
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

    InStream stream(out_stream.get_bytes());

    RDPECLIP::CliprdrHeader header (RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, (cItems*RDPECLIP::FileDescriptor::size())+4);
    bool param_dont_log_data_into_syslog = false;
    uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
    RDPVerbose verbose = RDPVerbose::none;

    uint32_t requestedFormatId = 48025;

    ClipboardData clip_data;
    clip_data.requestedFormatId = requestedFormatId;
    clip_data.client_data.file_list_format_id = requestedFormatId;

    std::vector<CliprdFileInfo> files;
    FormatDataResponseReceiveFileList receiver(
        files,
        stream,
        header,
        param_dont_log_data_into_syslog,
        clip_data.client_data.file_list_format_id,
        flags,
        clip_data.client_data.file_descriptor_stream,
        verbose,
        "server");
    clip_data.requestedFormatId = 0;

    RED_REQUIRE_EQUAL(files.size(), cItems);

    CliprdFileInfo& file_info_type_1 = files[0];
    RED_CHECK_EQUAL(file_info_type_1.file_size, 42);
    RED_CHECK_EQUAL(file_info_type_1.file_name, file_name_1);

    CliprdFileInfo& file_info_type_2 = files[1];
    RED_CHECK_EQUAL(file_info_type_2.file_size, 84);
    RED_CHECK_EQUAL(file_info_type_2.file_name, file_name_2);
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

    InStream stream({pre_stream.get_data(), cItems*RDPECLIP::FileDescriptor::size()});

    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, (cItems*RDPECLIP::FileDescriptor::size())+4);
    bool param_dont_log_data_into_syslog = false;
    uint32_t flags = CHANNELS::CHANNEL_FLAG_LAST;

    RDPVerbose verbose = RDPVerbose::none;

    uint32_t requestedFormatId = 48025;

    ClipboardData clip_data;
    clip_data.requestedFormatId = requestedFormatId;
    clip_data.client_data.file_list_format_id = requestedFormatId;
    size_t size_part_1 = 150;
    clip_data.client_data.file_descriptor_stream.rewind();
    clip_data.client_data.file_descriptor_stream.out_copy_bytes(pre_stream.get_data(), size_part_1);
    stream.in_skip_bytes(size_part_1);

    std::vector<CliprdFileInfo> files;
    FormatDataResponseReceiveFileList receiver(
        files,
        stream,
        header,
        param_dont_log_data_into_syslog,
        clip_data.client_data.file_list_format_id,
        flags,
        clip_data.client_data.file_descriptor_stream,
        verbose,
        "server");
    clip_data.requestedFormatId = 0;

    RED_REQUIRE_EQUAL(files.size(), cItems);

    CliprdFileInfo & file_info_type_1 = files[0];
    RED_CHECK_EQUAL(file_info_type_1.file_size, 42);
    RED_CHECK_EQUAL(file_info_type_1.file_name, file_name_1);

    CliprdFileInfo & file_info_type_2 = files[1];
    RED_CHECK_EQUAL(file_info_type_2.file_size, 84);
    RED_CHECK_EQUAL(file_info_type_2.file_name, file_name_2);
    }
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListReceive)
{
    Cliprdr::FormatNameInventory format_name_inventory;

    const bool use_long_format_name = true;

    StaticOutStream<1600> stream;
    uint32_t client_file_list_format_id = 49263;

    RDPECLIP::FormatListPDUEx fl;
    fl.add_format_name(RDPECLIP::CF_TEXT, "");
    fl.add_format_name(client_file_list_format_id, RDPECLIP::FILEGROUPDESCRIPTORW.data());

    RDPECLIP::CliprdrHeader in_header(RDPECLIP::CB_FORMAT_LIST, RDPECLIP::CB_RESPONSE__NONE_, fl.size(use_long_format_name));
    fl.emit(stream, use_long_format_name);

    InStream chunk(stream.get_bytes());

    FormatListReceive received(use_long_format_name, in_header, chunk, format_name_inventory, RDPVerbose::none);

    Cliprdr::FormatNameInventory::FormatName const* format_name;

    RED_CHECK(received.file_list_format_id == client_file_list_format_id);
    RED_REQUIRE(!!(format_name = format_name_inventory.find(RDPECLIP::CF_TEXT)));
    RED_CHECK_SMEM(format_name->utf8_name(), ""_av);
    RED_REQUIRE(!!(format_name = format_name_inventory.find(client_file_list_format_id)));
    RED_CHECK_SMEM(format_name->utf8_name(), Cliprdr::file_group_descriptor_w_utf8);
    RED_CHECK(format_name->utf8_name_equal(Cliprdr::file_group_descriptor_w_utf8));
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListSend) {

    FakeDataSender data_sender;

    FormatListSendBack sender(&data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK_MEM(data_sender.streams[0].av(), "\x03\x00\x01\x00\x00\x00\x00\x00"_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelServerMonitorReadySendBack) {

    FakeDataSender sender;
    const bool use_long_format_name = true;

    ServerMonitorReadySendBack pdu(RDPVerbose::none, use_long_format_name, &sender);

    RED_REQUIRE_EQUAL(sender.index, 2);
    RED_CHECK_MEM(sender.streams[0].av(),
        "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00"
        "\x01\x00\x00\x00\x02\x00\x00\x00"
        ""_av);
    RED_CHECK_MEM(sender.streams[1].av(),
        "\x02\x00\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00\x00\x00"_av);
}

// RED_AUTO_TEST_CASE(TestCliprdrChannelFileContentsResponseReceive)
// {
//     ClipboardData state;
//     const uint32_t streamID = 1;
//     const uint32_t lindex = 2;
//     const uint32_t position = 0;
//     const uint32_t cbRequested = 4;
//     const uint32_t clipDataId = 5;
//     const uint32_t offset = 5;
//
//     const uint8_t data[5] = {0xaf, 0xaf, 0xaf, 0xaf, 0xaf};
//
//     state.client_data.set_file_contents_request_info_inventory(
//         lindex,
//         position,
//         cbRequested,
//         clipDataId,
//         offset,
//         streamID
//       );
//
//     ClipboardSideData::file_contents_request_info& file_contents_request =
//         state.client_data.file_contents_request_info_inventory[streamID];
//
//     ClipboardSideData::file_info_inventory_type & file_info_inventory =
//                 state.client_data.file_stream_data_inventory[file_contents_request.clipDataId];
//     file_info_inventory.push_back({
//         "file_name",
//         5,
//         5,
//         SslSha256() });
//
//     CliprdFileInfo & file_info = file_info_inventory[file_contents_request.lindex];
//
//     RDPECLIP::FileContentsResponseRange pdu(streamID);
//     StaticOutStream<64> stream;
//     pdu.emit(stream);
//
//     stream.out_copy_bytes(data, 5);
//
//     InStream chunk(stream.get_bytes());
//
//     const RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 9);
//
//     FileContentsResponseReceive receiver(state.client_data, header, CHANNELS::CHANNEL_FLAG_FIRST, chunk);
//
//     RED_CHECK(receiver.must_log_file_info_type);
// }

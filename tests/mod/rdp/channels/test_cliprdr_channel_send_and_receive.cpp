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

        bytes_view av() const noexcept
        {
            return {data, size};
        }
    };

    std::array<PDUData, 2> streams;
    unsigned index = 0;

    void operator()(
        uint32_t /*total_length*/, uint32_t /*flags*/,
        bytes_view chunk_data) override
    {
        RED_REQUIRE(this->index < this->streams.size());
        RED_REQUIRE(chunk_data.size() < std::size(streams[this->index].data));
        this->streams[this->index].size = chunk_data.size();
        std::memcpy(streams[this->index].data, chunk_data.data(), chunk_data.size());
        ++this->index;
    }
};

RED_AUTO_TEST_CASE(TestCliprdrChannelFilecontentsRequestSend)
{
    FakeDataSender data_sender;
    const uint32_t streamID = 1;

    FilecontentsRequestSendBack sender(RDPECLIP::FILECONTENTS_SIZE, streamID, &data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK(data_sender.streams[0].av() ==
        "\x09\x00\x02\x00\x0c\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00"
        ""_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatDataRequestSend)
{
    FakeDataSender data_sender;

    FormatDataRequestSendBack sender(&data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK(data_sender.streams[0].av() == "\x05\x00\x02\x00\x00\x00\x00\x00"_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListReceive)
{
    Cliprdr::FormatNameInventory format_name_inventory;

    const bool use_long_format_name = true;

    StaticOutStream<1600> stream;
    uint32_t client_file_list_format_id = 49263;

    Cliprdr::format_list_serialize_with_header(
        stream, Cliprdr::IsLongFormat(use_long_format_name),
        std::array{
            Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}},
            Cliprdr::FormatNameRef{client_file_list_format_id,
                Cliprdr::formats::file_group_descriptor_w.ascii_name},
        });

    InStream chunk(stream.get_produced_bytes());

    RDPECLIP::CliprdrHeader in_header;
    in_header.recv(chunk);

    FormatListReceive received(use_long_format_name, in_header, chunk, format_name_inventory, RDPVerbose::cliprdr);

    Cliprdr::FormatName const* format_name;

    RED_CHECK(received.file_list_format_id == client_file_list_format_id);
    RED_REQUIRE(!!(format_name = format_name_inventory.find(RDPECLIP::CF_TEXT)));
    RED_CHECK(format_name->utf8_name() == ""_av);
    RED_REQUIRE(!!(format_name = format_name_inventory.find(client_file_list_format_id)));
    RED_CHECK(format_name->utf8_name() == Cliprdr::formats::file_group_descriptor_w.ascii_name);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelClientFormatListSend)
{
    FakeDataSender data_sender;

    format_list_send_back(&data_sender);

    RED_REQUIRE_EQUAL(data_sender.index, 1);
    RED_CHECK(data_sender.streams[0].av() == "\x03\x00\x01\x00\x00\x00\x00\x00"_av);
}

RED_AUTO_TEST_CASE(TestCliprdrChannelServerMonitorReadySendBack)
{
    FakeDataSender sender;
    const bool use_long_format_name = true;

    ServerMonitorReadySendBack pdu(RDPVerbose::none, use_long_format_name, &sender);

    RED_REQUIRE_EQUAL(sender.index, 2);
    RED_CHECK(sender.streams[0].av() ==
        "\x07\x00\x00\x00\x10\x00\x00\x00\x01\x00\x00\x00\x01\x00\x0c\x00"
        "\x01\x00\x00\x00\x02\x00\x00\x00"
        ""_av);
    RED_CHECK(sender.streams[1].av() ==
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
//     InStream chunk(stream.get_produced_bytes());
//
//     const RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 9);
//
//     FileContentsResponseReceive receiver(state.client_data, header, CHANNELS::CHANNEL_FLAG_FIRST, chunk);
//
//     RED_CHECK(receiver.must_log_file_info_type);
// }

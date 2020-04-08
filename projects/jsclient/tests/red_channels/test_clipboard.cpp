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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "binding_channel.hpp"

#include "red_channels/clipboard.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "utils/literals/utf16.hpp"

#include <tuple>
#include <vector>

namespace
{

struct DataChan : DataChan_tuple
{
    using DataChan_tuple::tuple;

    DataChan(bytes_view av, size_t total_len, uint32_t channel_flags)
    : DataChan(
        channel_names::cliprdr,
        {av.begin(), av.end()},
        total_len != ~0u ? total_len : av.size(),
        channel_flags)
    {}

    friend std::ostream& operator<<(std::ostream& out, DataChan const& x)
    {
        DataChan_tuple const& t = x;
        out << "DataChan{" << std::get<0>(t) << ", {";
        InStream in_stream(std::get<1>(t));
        RDPECLIP::CliprdrHeader header;
        header.recv(in_stream);
        out << "0x" << std::hex << header.msgType() << ", 0x" << header.msgFlags()
            << std::dec << ", " << header.dataLen() << ", ";
        print_bytes(out, in_stream.remaining_bytes());
        out << "}, " << std::get<2>(t) << ", 0x" << std::hex << std::get<3>(t) << std::dec << "}";
        return out;
    }
};

MAKE_BINDING_CALLBACKS(
    DataChan,
    (JS_x_f(setGeneralCapability, return generalFlags, uint32_t generalFlags))
    (JS_c(formatListStart))
    (JS_d(formatListFormat, uint8_t, uint32_t formatId, uint32_t customFormatId, bool isUTF8))
    (JS_c(formatListStop))
    (JS_d(formatDataResponse, uint8_t, uint32_t remainingDataLen, uint32_t formatId, uint32_t channelFlags))
    (JS_x(formatDataResponseFileStart, uint32_t countFile))
    (JS_d(formatDataResponseFile, uint8_t, uint32_t attr, uint32_t flags, uint32_t sizeLow, uint32_t sizeHigh, uint32_t lastWriteTimeLow, uint32_t lastWriteTimeHigh))
    (JS_c(formatDataResponseFileStop))
    (JS_d(fileContentsResponse, uint8_t, uint32_t streamId, uint32_t remainingDataLen, uint32_t channelFlags))
    (JS_x(formatDataRequest, uint32_t formatId))
    (JS_x(fileContentsRequest, uint32_t streamId, uint32_t type, uint32_t lindex, uint32_t nposLow, uint32_t nposHigh, uint32_t szRequested))
    (JS_x(receiveResponseFail, uint32_t messageType))
    (JS_x(lock, uint32_t lockId))
    (JS_x(unlock, uint32_t lockId))
    (JS_c(free))
)

constexpr int first_last_show_proto_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST
    | CHANNELS::CHANNEL_FLAG_FIRST
    | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
;

constexpr int first_last_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST
    | CHANNELS::CHANNEL_FLAG_FIRST
;

enum class Padding : unsigned;

struct Serializer
{
    StaticOutStream<65536> out_stream;

    Serializer(uint16_t msgType, uint16_t msgFlags, bytes_view data, Padding padding_data)
    {
        RDPECLIP::CliprdrHeader header(msgType, msgFlags, data.size());
        header.emit(out_stream);
        out_stream.out_copy_bytes(data);
        auto padding = unsigned(padding_data);
        while (padding--) {
            out_stream.out_uint8(0);
        }
    }

    operator bytes_view () const
    {
        return out_stream.get_produced_bytes();
    }
};

void clip_receive(
    redjs::ClipboardChannel& clip,
    uint16_t msgType, uint16_t msgFlags,
    bytes_view data = {},
    Padding padding_data = Padding{},
    uint32_t channel_flags = first_last_show_proto_channel_flags)
{
    clip.receive(Serializer(msgType, msgFlags, data, padding_data), channel_flags);
}

DataChan data_chan(
    uint16_t msgType, uint16_t msgFlags,
    bytes_view data = {},
    Padding padding_data = Padding{},
    std::size_t len = ~0u, uint32_t channel_flags = first_last_show_proto_channel_flags)
{
    return DataChan{Serializer(msgType, msgFlags, data, padding_data), len, channel_flags};
}



std::unique_ptr<redjs::ClipboardChannel> clip;

auto test_init_channel(Callback& cb, emscripten::val&& v)
{
    return redjs::ClipboardChannel(cb, std::move(v), true);
}

}


#define RECEIVE_DATAS(...) ::clip_receive(clip, __VA_ARGS__); CTX_CHECK_DATAS()
#define CALL_CB(...) clip.__VA_ARGS__; CTX_CHECK_DATAS()

RED_AUTO_TEST_CHANNEL(TestClipboardChannel, test_init_channel, clip)
{
    using namespace RDPECLIP;
    namespace cbchan = redjs::channels::clipboard;
    init_js_channel();

    const bool is_utf = true;
    const bool not_utf = false;

    RECEIVE_DATAS(CB_CLIP_CAPS, CB_RESPONSE__NONE_,
        "\x01\x00\x00\x00\x01\x00\x0c\x00\x02\x00\x00\x00\x12\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(setGeneralCapability{
            RDPECLIP::CB_USE_LONG_FORMAT_NAMES |
            RDPECLIP::CB_CAN_LOCK_CLIPDATA
        });
    };

    RECEIVE_DATAS(CB_MONITOR_READY, CB_RESPONSE__NONE_)
    {
        CHECK_NEXT_DATA(data_chan(CB_CLIP_CAPS, CB_RESPONSE__NONE_,
            "\x01\0\0\0\x01\0\x0C\0\x02\0\0\0\x12\0\0\0"_av));
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST, CB_ASCII_NAMES, ""_av));
    };

    RECEIVE_DATAS(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK)
    {
    };

    // copy (server -> client)

    RECEIVE_DATAS(CB_FORMAT_LIST, CB_RESPONSE__NONE_,
        "\x0d\x00\x00\x00\x00\x00"
        "\x10\x00\x00\x00\x00\x00"
        "\x01\x00\x00\x00\x00\x00"
        "\x07\x00\x00\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(formatListStart{});
        CHECK_NEXT_DATA(formatListFormat{""_av, CF_UNICODETEXT, 0, is_utf});
        CHECK_NEXT_DATA(formatListFormat{""_av, CF_LOCALE, 0, is_utf});
        CHECK_NEXT_DATA(formatListFormat{""_av, CF_TEXT, 0, is_utf});
        CHECK_NEXT_DATA(formatListFormat{""_av, CF_OEMTEXT, 0, is_utf});
        CHECK_NEXT_DATA(formatListStop{});
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK));
    };

    CALL_CB(send_request_format(CF_UNICODETEXT, cbchan::CustomFormat::None))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_, "\x0d\0\0\0"_av));
    };

    auto copy1 = "plop\0"_utf16_le;
    RECEIVE_DATAS(CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK, copy1, Padding(4))
    {
        // "\0\0" terminal automatically removed
        CHECK_NEXT_DATA(formatDataResponse(
            copy1.drop_back(2), 0, CF_UNICODETEXT, first_last_channel_flags));
    };

    // paste (client -> server)

    CALL_CB(send_format(CF_UNICODETEXT, cbchan::Charset::Utf16, ""_av))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST, CB_RESPONSE__NONE_, "\x0d\0\0\0\0\0"_av));
    };

    RECEIVE_DATAS(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK, ""_av, Padding(4))
    {
    };

    RECEIVE_DATAS(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_, "\x0d\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(formatDataRequest{CF_UNICODETEXT});
    };

    const auto paste1 = "xyz\0"_utf16_le;
    CALL_CB(send_header(CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK, paste1.size(), 0))
    {
        CHECK_NEXT_DATA(DataChan{"\x05\0\1\0\x08\0\0\0"_av, paste1.size() + 8,
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL});
    };

    CALL_CB(send_data(paste1, 0, CHANNELS::CHANNEL_FLAG_LAST))
    {
        CHECK_NEXT_DATA(DataChan{paste1, 0,
            CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL});
    };

    // file copy (server -> client)

    RECEIVE_DATAS(CB_FORMAT_LIST, CB_RESPONSE__NONE_,
        "\x6e\xc0\x00\x00\x46\x00\x69\x00" //n...F.i. !
        "\x6c\x00\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00" //l.e.G.r.o.u.p.D. !
        "\x65\x00\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00" //e.s.c.r.i.p.t.o. !
        "\x72\x00\x57\x00\x00\x00" //r.W... !
        ""_av, Padding(4))
    {
        CHECK_NEXT_DATA(formatListStart{});
        CHECK_NEXT_DATA(formatListFormat{"FileGroupDescriptorW"_utf16_le, 49262,
            uint32_t(cbchan::CustomFormat::FileGroupDescriptorW), not_utf});
        CHECK_NEXT_DATA(formatListStop{});
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK));
    };

    CALL_CB(send_request_format(49262, cbchan::CustomFormat::FileGroupDescriptorW))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_,
            "\x6e\xc0\x00\x00"_av));
    };

    RECEIVE_DATAS(CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK,
        "\x01\x00\x00\x00\x00\x00\x00\x00" //....T........... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x0c\x00\x00\x00\x61\x00\x62\x00\x63\x00\x00\x00\x00\x00\x00\x00" //....a.b.c....... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //............ !
        ""_av
        , Padding(4))
    {
        CHECK_NEXT_DATA(formatDataResponseFileStart{1});
        CHECK_NEXT_DATA(formatDataResponseFile{"abc"_utf16_le,
            /*.attr=*/0, /*.flags=*/0,
            /*.sizeLow=*/12, /*.sizeHigh=*/0,
            /*.lastWriteTimeLow=*/0, /*.lastWriteTimeHigh=*/0});
        CHECK_NEXT_DATA(formatDataResponseFileStop{});
    };

    CALL_CB(send_data(
        "\x08\x00\x01\x00\x1c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00" //................ !
        "\x00\x00\x00\x00"_av))
    {
        CHECK_NEXT_DATA(data_chan(CB_FILECONTENTS_REQUEST, CB_RESPONSE_OK,
            "\0\0\0\0\0\0\0\0\x02\0\0\0\0\0\0\0\0\0\0\0\x05\0\0\0\0\0\0\0"_av));
    };

    RECEIVE_DATAS(CB_FILECONTENTS_RESPONSE, CB_RESPONSE__NONE_,
        "\x12\x34\x56\x78""abcdefghijkl"_av, Padding(4))
    {
        CHECK_NEXT_DATA(fileContentsResponse{"abcdefghijkl"_av, 0x78563412, 0,
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST});
    };

    // response fail

    RECEIVE_DATAS(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_FAIL, ""_av, Padding(4))
    {
        CHECK_NEXT_DATA(receiveResponseFail{CB_FORMAT_LIST_RESPONSE});
    };

    // lock / unlock

    RECEIVE_DATAS(CB_LOCK_CLIPDATA, CB_RESPONSE_OK, "\x12\x34\x56\x78"_av, Padding(4))
    {
        CHECK_NEXT_DATA(lock{0x78563412});
    };

    RECEIVE_DATAS(CB_UNLOCK_CLIPDATA, CB_RESPONSE_OK, "\x12\x34\x56\x78"_av, Padding(4))
    {
        CHECK_NEXT_DATA(unlock{0x78563412});
    };

    // long list

    CALL_CB(send_request_format(49262, cbchan::CustomFormat::FileGroupDescriptorW))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_,
            "\x6e\xc0\x00\x00"_av));
    };

    auto file_group_with_3_files =
        "\x05\x00\x01\x00\xf0\x0a\x00\x00"
        "\x03\x00\x00\x00\x00\x00\x00\x00" //....T........... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x0c\x00\x00\x00\x61\x00\x62\x00\x63\x00\x00\x00\x00\x00\x00\x00" //....a.b.c....... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //............ !
        "\x00\x00\x00\x00" //....T........... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x0c\x00\x00\x00\x64\x00\x65\x00\x66\x00\x00\x00\x00\x00\x00\x00" //....d.e.f....... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //............ !
        "\x00\x00\x00\x00" //....T........... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x0c\x00\x00\x00\x67\x00\x68\x00\x69\x00\x00\x00\x00\x00\x00\x00" //....g.h.i....... !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................ !
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //............ !
        ""_av;
    clip.receive(file_group_with_3_files.first(1600),
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    CTX_CHECK_DATAS()
    {
        CHECK_NEXT_DATA(formatDataResponseFileStart{3});
        CHECK_NEXT_DATA(formatDataResponseFile{"abc"_utf16_le,
            /*.attr=*/0, /*.flags=*/0,
            /*.sizeLow=*/12, /*.sizeHigh=*/0,
            /*.lastWriteTimeLow=*/0, /*.lastWriteTimeHigh=*/0});
        CHECK_NEXT_DATA(formatDataResponseFile{"def"_utf16_le,
            /*.attr=*/0, /*.flags=*/0,
            /*.sizeLow=*/12, /*.sizeHigh=*/0,
            /*.lastWriteTimeLow=*/0, /*.lastWriteTimeHigh=*/0});
    };
    clip.receive(file_group_with_3_files.from_offset(1600),
        CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
    CTX_CHECK_DATAS()
    {
        CHECK_NEXT_DATA(formatDataResponseFile{"ghi"_utf16_le,
            /*.attr=*/0, /*.flags=*/0,
            /*.sizeLow=*/12, /*.sizeHigh=*/0,
            /*.lastWriteTimeLow=*/0, /*.lastWriteTimeHigh=*/0});
        CHECK_NEXT_DATA(formatDataResponseFileStop{});
    };
}

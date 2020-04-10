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
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "utils/stream.hpp"

#include <vector>


struct FilecontentsRequestSendBack
{
    FilecontentsRequestSendBack(uint32_t dwFlags, uint32_t streamID, VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        StaticOutStream<256> out_stream;

        switch (dwFlags)
        {
            case RDPECLIP::FILECONTENTS_RANGE:
            {
                RDPECLIP::FileContentsResponseRange pdu(streamID);
                RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                RDPECLIP::CB_RESPONSE_FAIL,
                                                pdu.packet_size());
                header.emit(out_stream);
                pdu.emit(out_stream);
            }
            break;

            case RDPECLIP::FILECONTENTS_SIZE:
            {
                RDPECLIP::FileContentsResponseSize pdu(streamID, 0);
                RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                RDPECLIP::CB_RESPONSE_FAIL,
                                                pdu.packet_size());
                header.emit(out_stream);
                pdu.emit(out_stream);
            }
            break;
        }

        sender->operator()(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_produced_bytes());
    }
};

struct FormatDataRequestSendBack
{
    FormatDataRequestSendBack(VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
        header.emit(out_stream);

        sender->operator()(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_produced_bytes());
    }
};

struct CliprdFileInfo
{
    uint64_t file_size;
    std::string file_name;
};

struct FormatDataResponseReceiveFileList
{
    FormatDataResponseReceiveFileList(std::vector<CliprdFileInfo>& files, InStream & chunk, const RDPECLIP::CliprdrHeader & in_header, bool param_dont_log_data_into_syslog, const uint32_t file_list_format_id, const uint32_t flags, OutStream & file_descriptor_stream, const RDPVerbose verbose, char const* direction)
    {
        auto receive_file = [&](InStream& in_stream){
            RDPECLIP::FileDescriptor fd;

            fd.receive(in_stream);

            if (bool(verbose & RDPVerbose::cliprdr)) {
                fd.log(LOG_INFO);
            }

            files.push_back(CliprdFileInfo{fd.file_size(), std::move(fd.file_name)});
        };

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!(in_header.msgFlags() & RDPECLIP::CB_RESPONSE_FAIL) && (in_header.dataLen() >= 4 /* cItems(4) */)) {
                const uint32_t cItems = chunk.in_uint32_le();

                if (!param_dont_log_data_into_syslog) {
                    LOG(LOG_INFO,
                        "Sending %sFileGroupDescriptorW(%u) clipboard data to %s. "
                            "cItems=%u",
                        ((flags & CHANNELS::CHANNEL_FLAG_LAST) ? "" : "(chunked) "),
                        file_list_format_id, direction, cItems);
                }
            }
        }
        else if (file_descriptor_stream.get_offset()) {
            const uint32_t complementary_data_length =
                RDPECLIP::FileDescriptor::size() -
                    file_descriptor_stream.get_offset();

            assert(chunk.in_remain() >= complementary_data_length);

            file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                complementary_data_length);

            chunk.in_skip_bytes(complementary_data_length);

            InStream in_stream(file_descriptor_stream.get_produced_bytes());

            receive_file(in_stream);

            file_descriptor_stream.rewind();
        }

        while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
            receive_file(chunk);
        }

        if (chunk.in_remain()) {
            file_descriptor_stream.rewind();
            file_descriptor_stream.out_copy_bytes(chunk.in_skip_bytes(chunk.in_remain()));
        }
    }
};

struct ServerMonitorReadySendBack
{
    ServerMonitorReadySendBack(const RDPVerbose verbose, const bool use_long_format_names, VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                "Send Clipboard Capabilities PDU.");

        RDPECLIP::GeneralCapabilitySet general_cap_set(
            RDPECLIP::CB_CAPS_VERSION_1,
            RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
        RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1);
        RDPECLIP::CliprdrHeader caps_clipboard_header(RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE__NONE_,
            clipboard_caps_pdu.size() + general_cap_set.size());

        StaticOutStream<1024> caps_stream;

        caps_clipboard_header.emit(caps_stream);
        clipboard_caps_pdu.emit(caps_stream);
        general_cap_set.emit(caps_stream);

        sender->operator()(
            caps_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            caps_stream.get_produced_bytes());

        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                "Send Format List PDU.");

        StaticOutStream<256> list_stream;
        Cliprdr::format_list_serialize_with_header(
            list_stream, Cliprdr::IsLongFormat(use_long_format_names),
            std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

        sender->operator()(
            list_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            list_stream.get_produced_bytes());
    }
};


struct FormatListReceive
{
    uint32_t file_list_format_id = 0;

    FormatListReceive(
        const bool use_long_format,
        const RDPECLIP::CliprdrHeader & in_header,
        InStream & chunk,
        Cliprdr::FormatNameInventory& format_name_inventory,
        const RDPVerbose verbose)
    {
        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "%s variant of Format List PDU is used for exchanging updated format names.",
            use_long_format
                ? "Long Format Name"
            : (in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES)
                ? "Short Format Name (ASCII 8)"
                : "Short Format Name"
        );

        auto buf = chunk.remaining_bytes();
        InStream in_stream(buf.first(std::min<size_t>(in_header.dataLen(), buf.size())));

        Cliprdr::format_list_extract(
            in_stream,
            Cliprdr::IsLongFormat(use_long_format),
            Cliprdr::IsAscii(bool(in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES)),
            [&](uint32_t format_id, auto name) {
                auto&& format_name = format_name_inventory.push(format_id, name);
                auto&& utf8_name = format_name.utf8_name();

                LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "formatId=%s(%u) wszFormatName=\"%.*s\"",
                    RDPECLIP::get_FormatId_name(format_id),
                    format_id, int(utf8_name.size()), utf8_name.data());

                if (ranges_equal(utf8_name, Cliprdr::formats::file_group_descriptor_w.ascii_name)) {
                    this->file_list_format_id = format_id;
                }
            }
        );

        chunk.in_skip_bytes(in_stream.get_offset());

        // some version of server add "\0\0\0\0" and total_len == chunk_size + 4
        LOG_IF(chunk.in_remain() != 4 && chunk.in_remain() != 0,
            LOG_WARNING, "Truncated CLIPRDR_FORMAT_LIST remains=%zu", chunk.in_remain());
    }
};

inline void format_list_send_back(VirtualChannelDataSender* sender)
{
    if (!sender) {
        return ;
    }

    RDPECLIP::FormatListResponsePDU pdu;

    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, pdu.size());

    StaticOutStream<256> out_stream;

    header.emit(out_stream);
    pdu.emit(out_stream);

    sender->operator()(
        out_stream.get_offset(),
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        out_stream.get_produced_bytes());
}

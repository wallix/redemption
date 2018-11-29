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

#include "core/channel_list.hpp"
#include "core/RDP/clipboard.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "utils/stream.hpp"
#include "system/ssl_sha256.hpp"

#include <map>
#include <vector>

#define FILE_LIST_FORMAT_NAME "FileGroupDescriptorW"



struct ClipboardSideData {

     struct file_contents_request_info
    {
        uint32_t lindex;

        uint64_t position;

        uint32_t cbRequested;

        uint32_t clipDataId;

        uint32_t offset;
    };
    using file_contents_request_info_inventory_type = std::map<uint32_t /*streamId*/, file_contents_request_info>;

    struct file_info_type
    {
        std::string file_name;

        uint64_t size;

        uint64_t sequential_access_offset;

        SslSha256 sha256;
    };
    using file_info_inventory_type = std::vector<file_info_type>;

    using file_stream_data_inventory_type = std::map<uint32_t /*clipDataId*/, file_info_inventory_type>;

    uint16_t message_type = 0;
    bool use_long_format_names = false;
    uint32_t clipDataId = 0;
    uint32_t file_list_format_id = 0;
    uint32_t dataLen = 0;
    uint32_t streamId = 0;
    StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

    file_contents_request_info_inventory_type file_contents_request_info_inventory;
    file_stream_data_inventory_type file_stream_data_inventory;

    std::string provider_name;

    ClipboardSideData(std::string provider_name)
      : provider_name(std::move(provider_name))
    {}

     void set_file_contents_request_info_inventory(uint32_t lindex, uint64_t position, uint32_t cbRequested, uint32_t clipDataId, uint32_t offset, uint32_t streamID) {
        this->file_contents_request_info_inventory[streamID] =
        {
            lindex,
            position,
            cbRequested,
            clipDataId,
            offset
        };
    }

    void update_file_contents_request_inventory(RDPECLIP::FileDescriptor const& fd) {
        file_info_inventory_type & file_info_inventory =
                this->file_stream_data_inventory[this->clipDataId];
            file_info_inventory.push_back({ fd.fileName(), fd.file_size(), 0, SslSha256() });
    }
};

struct ClipboardData {
    using format_name_inventory_type = std::map<uint32_t, std::string>;

    ClipboardSideData server_data;
    ClipboardSideData client_data;

    uint32_t requestedFormatId = 0;

    format_name_inventory_type format_name_inventory;

    ClipboardData()
      : server_data("server")
      , client_data("client") {}
};

struct ClipboardCapabilitiesReceive {

    ClipboardCapabilitiesReceive(ClipboardSideData & clip_data, InStream& chunk, const RDPVerbose verbose) {
        {
            const unsigned int expected = 4;   //     cCapabilitiesSets(2) +
                                                //     pad1(2)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_%s_clipboard_capabilities_pdu: "
                        "Truncated CLIPRDR_CAPS, need=%u remains=%zu", clip_data.provider_name,
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_%s_message: "
                    "Clipboard Capabilities PDU", clip_data.provider_name);
        }

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        assert(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType() == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_%s_clipboard_capabilities_pdu: "
                            "General Capability Set", clip_data.provider_name);
                    general_caps.log(LOG_INFO);
                }

                clip_data.use_long_format_names =
                    bool(general_caps.generalFlags() &
                     RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }
    }
};

struct FilecontentsRequestReceive {

    uint32_t dwFlags = 0;
    uint32_t streamID = 0;

    FilecontentsRequestReceive(ClipboardSideData & clip_state, InStream& chunk, const RDPVerbose verbose, uint32_t dataLen) {
        LOG(LOG_INFO, "dataLen=%u FileContentsRequestPDU::minimum_size()=%zu", dataLen, RDPECLIP::FileContentsRequestPDU::minimum_size());
        if (dataLen >= RDPECLIP::FileContentsRequestPDU::minimum_size()) {
            RDPECLIP::FileContentsRequestPDU file_contents_request_pdu;

            file_contents_request_pdu.receive(chunk);
            if (bool(verbose & RDPVerbose::cliprdr)) {
                file_contents_request_pdu.log(LOG_INFO);
            }

            this->dwFlags = file_contents_request_pdu.dwFlags();
            this->streamID = file_contents_request_pdu.streamId();

            if ((RDPECLIP::FILECONTENTS_RANGE == this->dwFlags) && file_contents_request_pdu.clipDataId()) {

                clip_state.set_file_contents_request_info_inventory(
                    file_contents_request_pdu.lindex(),
                    file_contents_request_pdu.position(),
                    file_contents_request_pdu.cbRequested(),
                    file_contents_request_pdu.clipDataId(), 0, this->streamID);
            }
        }
    }
};

struct ClientFilecontentsRequestSendBack {

    ClientFilecontentsRequestSendBack(const RDPVerbose verbose, uint32_t dwFlags, uint32_t streamID, BaseVirtualChannel * base_channel) {
        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_client_filecontents_request: "
                    "Requesting the contents of server file is denied.");
        }

        StaticOutStream<256> out_stream;

        switch (dwFlags) {

            case RDPECLIP::FILECONTENTS_RANGE:
                {
                    RDPECLIP::FileContentsResponseRange pdu(streamID);
                    RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                    RDPECLIP::CB_RESPONSE_FAIL,
                                                    pdu.size());
                    header.emit(out_stream);
                    pdu.emit(out_stream);
                }
                break;

            case RDPECLIP::FILECONTENTS_SIZE:
                {
                    RDPECLIP::FileContentsResponseSize pdu(streamID, 0);
                    RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                    RDPECLIP::CB_RESPONSE_FAIL,
                                                    pdu.size());
                    header.emit(out_stream);
                    pdu.emit(out_stream);
                }
                break;
        }

        LOG(LOG_INFO, "will call send_message_to_client");

        base_channel->send_message_to_client(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_data(),
            out_stream.get_offset());
    }
};

struct ServerFilecontentsRequestSendBack {

    ServerFilecontentsRequestSendBack(const RDPVerbose verbose, uint32_t dwFlags, uint32_t streamID, BaseVirtualChannel * base_channel) {
        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_filecontents_request: "
                    "Requesting the contents of server file is denied.");
        }

        StaticOutStream<256> out_stream;

        switch (dwFlags) {

            case RDPECLIP::FILECONTENTS_RANGE:
                {
                    RDPECLIP::FileContentsResponseRange pdu(streamID);
                    RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                    RDPECLIP::CB_RESPONSE_FAIL,
                                                    pdu.size());
                    header.emit(out_stream);
                    pdu.emit(out_stream);
                }
                break;

            case RDPECLIP::FILECONTENTS_SIZE:
                {
                    RDPECLIP::FileContentsResponseSize pdu(streamID, 0);
                    RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                    RDPECLIP::CB_RESPONSE_FAIL,
                                                    pdu.size());
                    header.emit(out_stream);
                    pdu.emit(out_stream);
                }
                break;
        }

        base_channel->send_message_to_server(
                        out_stream.get_offset(),
                        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                        out_stream.get_data(),
                        out_stream.get_offset());
    }
};

struct FormatDataRequestReceive {

    FormatDataRequestReceive(ClipboardData & clip_data, ClipboardSideData & clip_side_data, const RDPVerbose verbose, InStream& chunk) {
        {
            const unsigned int expected = 4;   //     requestedFormatId(4)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_%s_format_data_request_pdu: "
                        "Truncated CLIPRDR_FORMAT_DATA_REQUEST, need=%u remains=%zu"
                    , clip_side_data.provider_name, expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        clip_data.requestedFormatId = chunk.in_uint32_le();

        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_%s_format_data_request_pdu: "
                    "requestedFormatId=%s(%u)", clip_side_data.provider_name,
                RDPECLIP::get_FormatId_name(clip_data.requestedFormatId),
                clip_data.requestedFormatId);
        }
    }
};

struct ServerFormatDataRequestSendBack {

    ServerFormatDataRequestSendBack(const RDPVerbose verbose, BaseVirtualChannel * base_channel) {
        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                        "Client to server Clipboard operation is not allowed.");
        }

        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
        header.emit(out_stream);

        base_channel->send_message_to_server(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_data(),
            out_stream.get_offset());

    }
};

struct ClientFormatDataRequestSendBack {

    uint32_t total_length = 0;
    const uint32_t flags = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

    ClientFormatDataRequestSendBack(const RDPVerbose verbose, BaseVirtualChannel * base_channel) {
        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                    "Serveur to client Clipboard operation is not allowed.");
        }

        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
        header.emit(out_stream);

        base_channel->send_message_to_client(
                out_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset());
    }
};

struct ClientFormatDataResponseReceive {

    std::string  data_to_dump;

    ClientFormatDataResponseReceive(ClipboardSideData & clip_side_data, ClipboardData & clip_data, InStream & chunk, const RDPECLIP::CliprdrHeader & in_header, bool param_dont_log_data_into_syslog, const uint32_t flags, const RDPVerbose verbose) {
        std::vector<RDPECLIP::FileDescriptor> fds;

        if (clip_side_data.file_list_format_id && (clip_data.requestedFormatId == clip_side_data.file_list_format_id)) {
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                if (!(in_header.msgFlags() & RDPECLIP::CB_RESPONSE_FAIL) && (in_header.dataLen() >= 4 /* cItems(4) */)) {
                    const uint32_t cItems = chunk.in_uint32_le();

                    if (!param_dont_log_data_into_syslog) {
                        LOG(LOG_INFO,
                            "Sending %sFileGroupDescriptorW(%u) clipboard data to server. "
                                "cItems=%u",
                            ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                                "" : "(chunked) "),
                            clip_side_data.file_list_format_id, cItems);
                    }
                }
            } else {

                if (clip_side_data.file_descriptor_stream.get_offset()) {
                    const uint32_t complementary_data_length =
                        RDPECLIP::FileDescriptor::size() -
                            clip_side_data.file_descriptor_stream.get_offset();

                    assert(chunk.in_remain() >= complementary_data_length);

                    clip_side_data.file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                        complementary_data_length);

                    chunk.in_skip_bytes(complementary_data_length);

                    RDPECLIP::FileDescriptor fd;

                    InStream in_stream(
                        clip_side_data.file_descriptor_stream.get_data(),
                        clip_side_data.file_descriptor_stream.get_offset()
                    );
                    fd.receive(in_stream);
                    if (bool(verbose & RDPVerbose::cliprdr)) {
                        fd.log(LOG_INFO);
                    }

                    fds.push_back(fd);

                    clip_side_data.file_descriptor_stream.rewind();
                }
            }

             while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                    fd.receive(chunk);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                fds.push_back(fd);
            }

            if (chunk.in_remain()) {
                clip_side_data.file_descriptor_stream.rewind();

                clip_side_data.file_descriptor_stream.out_copy_bytes(
                    chunk.get_current(), chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                clip_data.requestedFormatId  = 0;
            }

            for (RDPECLIP::FileDescriptor const& fd : fds) {
//                 const bool from_remote_session = false;
                clip_side_data.update_file_contents_request_inventory(fd);
            }

        } else {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
//                 InStream chunk_serie = chunk.clone();

                constexpr size_t const max_length_of_data_to_dump = 256;

                switch (clip_data.requestedFormatId) {
        /*
                    case RDPECLIP::CF_TEXT:
                    {
                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(), max_length_of_data_to_dump);
                        const std::string data_to_dump(
                            ::char_ptr_cast(chunk.get_current()),
                            length_of_data_to_dump);
                        LOG(LOG_INFO, "%s", data_to_dump);
                    }
                    break;
        */
                    case RDPECLIP::CF_UNICODETEXT:
                    {
                        assert(!(chunk.in_remain() & 1));

                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(), max_length_of_data_to_dump * 2);

                        constexpr size_t size_of_utf8_string =
                            max_length_of_data_to_dump *
                                maximum_length_of_utf8_character_in_bytes;

                        uint8_t utf8_string[size_of_utf8_string + 1] {};
                        const size_t length_of_utf8_string = ::UTF16toUTF8(
                            chunk.get_current(), length_of_data_to_dump / 2,
                            utf8_string, size_of_utf8_string);
                        data_to_dump.assign(
                            ::char_ptr_cast(utf8_string),
                            ((length_of_utf8_string && !utf8_string[length_of_utf8_string - 1]) ?
                                length_of_utf8_string - 1 :
                                length_of_utf8_string));
                    }
                    break;

                    case RDPECLIP::CF_LOCALE:
                    {
                        const uint32_t locale_identifier = chunk.in_uint32_le();
                        data_to_dump = std::to_string(locale_identifier);
                    }
                    break;
                }
            }
        }
    }
};

struct ServerFormatDataResponseReceive {

    std::string  data_to_dump;

    std::vector<RDPECLIP::FileDescriptor> fds;

    ServerFormatDataResponseReceive(const uint32_t requestedFormatId, InStream & chunk, const RDPECLIP::CliprdrHeader & in_header, bool param_dont_log_data_into_syslog, const uint32_t server_file_list_format_id, const uint32_t flags, OutStream & file_descriptor_stream, const RDPVerbose verbose) {

        if (server_file_list_format_id && (requestedFormatId == server_file_list_format_id)) {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                if (!(in_header.msgFlags() & RDPECLIP::CB_RESPONSE_FAIL) && (in_header.dataLen() >= 4 /* cItems(4) */)) {
                    const uint32_t cItems = chunk.in_uint32_le();

                    if (!param_dont_log_data_into_syslog) {
                        LOG(LOG_INFO,
                            "Sending %sFileGroupDescriptorW(%u) clipboard data to client. "
                                "cItems=%u",
                            ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                                "" : "(chunked) "),
                            server_file_list_format_id, cItems);
                    }
                }
            } else {

                if (file_descriptor_stream.get_offset()) {
                    const uint32_t complementary_data_length =
                        RDPECLIP::FileDescriptor::size() -
                            file_descriptor_stream.get_offset();

                    assert(chunk.in_remain() >= complementary_data_length);

                    file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                        complementary_data_length);

                    chunk.in_skip_bytes(complementary_data_length);

                    RDPECLIP::FileDescriptor fd;

                    InStream in_stream(
                        file_descriptor_stream.get_data(),
                        file_descriptor_stream.get_offset()
                    );
                    fd.receive(in_stream);
                    if (bool(verbose & RDPVerbose::cliprdr)) {
                        fd.log(LOG_INFO);
                    }

                    this->fds.push_back(fd);

                    file_descriptor_stream.rewind();
                }
            }

             while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                    fd.receive(chunk);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                this->fds.push_back(fd);
            }

            if (chunk.in_remain()) {
                file_descriptor_stream.rewind();

                file_descriptor_stream.out_copy_bytes(
                    chunk.get_current(), chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

        } else {

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
//                 InStream chunk_serie = chunk.clone();

                constexpr size_t const max_length_of_data_to_dump = 256;

                switch (requestedFormatId) {
        /*
                    case RDPECLIP::CF_TEXT:
                    {
                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(), max_length_of_data_to_dump);
                        const std::string data_to_dump(
                            ::char_ptr_cast(chunk.get_current()),
                            length_of_data_to_dump);
                        LOG(LOG_INFO, "%s", data_to_dump);
                    }
                    break;
        */
                    case RDPECLIP::CF_UNICODETEXT:
                    {
                        assert(!(chunk.in_remain() & 1));

                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(), max_length_of_data_to_dump * 2);

                        constexpr size_t size_of_utf8_string =
                            max_length_of_data_to_dump *
                                maximum_length_of_utf8_character_in_bytes;

                        uint8_t utf8_string[size_of_utf8_string + 1] {};
                        const size_t length_of_utf8_string = ::UTF16toUTF8(
                            chunk.get_current(), length_of_data_to_dump / 2,
                            utf8_string, size_of_utf8_string);
                        data_to_dump.assign(
                            ::char_ptr_cast(utf8_string),
                            ((length_of_utf8_string && !utf8_string[length_of_utf8_string - 1]) ?
                                length_of_utf8_string - 1 :
                                length_of_utf8_string));
                    }
                    break;

                    case RDPECLIP::CF_LOCALE:
                    {
                        const uint32_t locale_identifier = chunk.in_uint32_le();

                        data_to_dump = std::to_string(locale_identifier);
                    }
                    break;
                }
            }
        }
    }
};


struct ServerMonitorReadySendBack {

    ServerMonitorReadySendBack(const RDPVerbose verbose, const bool current_use_long_format_names, BaseVirtualChannel * base_channel) {
        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                    "Send Clipboard Capabilities PDU.");
        }

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

        base_channel->send_message_to_server(
            caps_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            caps_stream.get_data(),
            caps_stream.get_offset());

        if (bool(verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                    "Send Format List PDU.");
        }

        RDPECLIP::FormatListPDUEx format_list_pdu;
        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

        const bool use_long_format_names = current_use_long_format_names;
        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

        RDPECLIP::CliprdrHeader list_clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
            format_list_pdu.size(use_long_format_names));

        StaticOutStream<256> list_stream;

        list_clipboard_header.emit(list_stream);
        format_list_pdu.emit(list_stream, use_long_format_names);

        base_channel->send_message_to_server(
            list_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            list_stream.get_data(),
            list_stream.get_offset());
    }

};



struct ServerFormatListReceive {

    uint32_t server_file_list_format_id = 0;

    ServerFormatListReceive(const bool client_use_long_format_names,const bool server_use_long_format_names, const RDPECLIP::CliprdrHeader & in_header, InStream & chunk,  std::map<uint32_t, std::string> & format_name_inventory, const RDPVerbose verbose) {
        if (!client_use_long_format_names ||
            !server_use_long_format_names) {
            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Short Format Name%s variant of Format List PDU is used "
                        "for exchanging updated format names.",
                    ((in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)"
                                                           : ""));
            }

            for (uint32_t remaining_data_length = in_header.dataLen();
                 remaining_data_length; ) {

                LOG(LOG_INFO, "remaining_data_length=%u chunk.in_remain=%zu", remaining_data_length, chunk.in_remain());


                const     uint32_t formatId           = chunk.in_uint32_le();
                constexpr size_t   format_name_length =
                        32      // formatName(32)
                           / 2  // size_of(Unicode characters)(2)
                    ;

                constexpr size_t size_of_utf8_string =
                    format_name_length *
                        maximum_length_of_utf8_character_in_bytes;
                uint8_t utf8_string[size_of_utf8_string + 1];
                ::memset(utf8_string, 0, sizeof(utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.get_current(), format_name_length, utf8_string,
                    size_of_utf8_string);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_FormatId_name(formatId), formatId,
                        utf8_string);
                }

                format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

                remaining_data_length -=
                          4     // formatId(4)
                        + 32    // formatName(32)
                    ;

                if (((sizeof(FILE_LIST_FORMAT_NAME) - 1) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->server_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(
                        32  // formatName(32)
                    );
            }
        }
        else {
            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }

            const size_t max_length_of_format_name = 256;

            for (uint32_t remaining_data_length = in_header.dataLen();
                 remaining_data_length; ) {
                const uint32_t formatId                     =
                    chunk.in_uint32_le();
                const size_t   format_name_length           =
                    ::UTF16StrLen(chunk.get_current()) + 1;
                const size_t   adjusted_format_name_length =
                    std::min(format_name_length - 1,
                        max_length_of_format_name);

                constexpr size_t size_of_utf8_string =
                    max_length_of_format_name *
                    maximum_length_of_utf8_character_in_bytes;
                uint8_t utf8_string[size_of_utf8_string + 1];
                ::memset(utf8_string, 0, sizeof(utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.get_current(), adjusted_format_name_length,
                    utf8_string, size_of_utf8_string);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_FormatId_name(formatId), formatId, utf8_string);
                }

                format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

                remaining_data_length -=
                          4                      /* formatId(4) */
                        + format_name_length * 2 /* wszFormatName(variable) */
                    ;

                if (((sizeof(FILE_LIST_FORMAT_NAME) - 1) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->server_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(format_name_length * 2);
            }
        }
    }
};



struct ClientFormatListReceive {

    uint32_t client_file_list_format_id = 0;

    ClientFormatListReceive(const bool client_use_long_format_names,const bool server_use_long_format_names, const RDPECLIP::CliprdrHeader & in_header, InStream & chunk,  std::map<uint32_t, std::string> & format_name_inventory, const RDPVerbose verbose) {
        if (!client_use_long_format_names ||
            !server_use_long_format_names) {
            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Short Format Name%s variant of Format List PDU is used "
                        "for exchanging updated format names.",
                    ((in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)"
                                                           : ""));
            }

            for (uint32_t remaining_data_length = in_header.dataLen();
                 remaining_data_length; ) {
                {
                    const unsigned int expected = 36;   // formatId(4) + formatName(32)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_WARNING,
                            "ClipboardVirtualChannel::process_client_format_list_pdu: "
                                "Truncated (SHORT) CLIPRDR_FORMAT_LIST, "
                                "need=%u remains=%zu",
                            expected, chunk.in_remain());
                        break;
                    }
                }

                const     uint32_t formatId           = chunk.in_uint32_le();
                constexpr size_t   format_name_length =
                        32      // formatName(32)
                           / 2  // size_of(Unicode characters)(2)
                    ;

                constexpr size_t size_of_utf8_string =
                    format_name_length *
                    maximum_length_of_utf8_character_in_bytes;
                uint8_t utf8_string[size_of_utf8_string + 1];
                ::memset(utf8_string, 0, sizeof(utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.get_current(), format_name_length, utf8_string,
                    size_of_utf8_string);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_FormatId_name(formatId), formatId,
                        utf8_string);
                }

                format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

                remaining_data_length -=
                          4     // formatId(4)
                        + 32    // formatName(32)
                    ;

                if (((sizeof(FILE_LIST_FORMAT_NAME) - 1) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->client_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(
                        32  // formatName(32)
                    );
            }
        }
        else {
            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }

            for (uint32_t remaining_data_length = in_header.dataLen();
                 remaining_data_length; ) {
                {
                    const unsigned int expected = 6;    // formatId(4) + min_len(formatName)(2)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_WARNING,
                            "ClipboardVirtualChannel::process_client_format_list_pdu: "
                                "Truncated (LONG) CLIPRDR_FORMAT_LIST, "
                                "need=%u remains=%zu",
                            expected, chunk.in_remain());
                        break;
                    }
                }

                const size_t max_length_of_format_name = 256;

                const uint32_t formatId                    =
                    chunk.in_uint32_le();
                const size_t   format_name_length          =
                    ::UTF16StrLen(chunk.get_current()) + 1;
                const size_t   adjusted_format_name_length =
                    std::min(format_name_length - 1,
                             max_length_of_format_name);

                constexpr size_t size_of_utf8_string =
                    max_length_of_format_name *
                        maximum_length_of_utf8_character_in_bytes;
                uint8_t utf8_string[size_of_utf8_string + 1];
                ::memset(utf8_string, 0, sizeof(utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.get_current(), adjusted_format_name_length,
                    utf8_string, size_of_utf8_string);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_FormatId_name(formatId), formatId,
                        utf8_string);
                }

                format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

                remaining_data_length -=
                          4                      /* formatId(4) */
                        + format_name_length * 2 /* wszFormatName(variable) */
                    ;

                if (((sizeof(FILE_LIST_FORMAT_NAME) - 1) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->client_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(format_name_length * 2);
            }
        }
    }
};

struct ClientFormatListSendBack {

    ClientFormatListSendBack(BaseVirtualChannel * base_channel) {

        RDPECLIP::FormatListResponsePDU pdu;

        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, pdu.size());

        StaticOutStream<256> out_stream;

        header.emit(out_stream);
        pdu.emit(out_stream);

        base_channel->send_message_to_client(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_data(),
            out_stream.get_offset());
    }
};

struct ServerFormatListSendBack {

    ServerFormatListSendBack(BaseVirtualChannel * base_channel) {

        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 0);

        StaticOutStream<256> out_stream;

        header.emit(out_stream);

        base_channel->send_message_to_server(
                out_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_data(),
                out_stream.get_offset());
    }
};

struct LockClipDataReceive {

    LockClipDataReceive(ClipboardSideData & clip_receiver_side_data, ClipboardSideData & clip_sender_side_data, InStream & chunk, const RDPVerbose verbose, const RDPECLIP::CliprdrHeader & header) {
        if (header.dataLen() >= 4 /* clipDataId(4) */) {
            {
                const unsigned int expected = 4;
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "ClipboardVirtualChannel::process_%s_message: "
                            "Truncated CLIPRDR_LOCK_CLIPDATA, "
                            "need=%u remains=%zu", clip_sender_side_data.provider_name,
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

           clip_receiver_side_data.clipDataId  = chunk.in_uint32_le();

            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_%s_message: "
                        "clipDataId=%u", clip_sender_side_data.provider_name, clip_receiver_side_data.clipDataId);
            }

            clip_receiver_side_data.file_stream_data_inventory[clip_receiver_side_data.clipDataId] = ClipboardSideData::file_info_inventory_type();
        }
    }
};

struct UnlockClipDataReceive {

    UnlockClipDataReceive(ClipboardSideData & clip_receiver_side_data, ClipboardSideData & clip_sender_side_data, InStream & chunk, const RDPVerbose verbose, const RDPECLIP::CliprdrHeader & header) {
         if (header.dataLen() >= 4 /* clipDataId(4) */) {
            {
                const unsigned int expected = 4;    // clipDataId(4)
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "ClipboardVirtualChannel::process_%s_message: "
                            "Truncated CLIPRDR_UNLOCK_CLIPDATA, "
                            "need=%u remains=%zu",clip_sender_side_data.provider_name,
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            uint32_t const clipDataId = chunk.in_uint32_le();

            if (bool(verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_%s_message: "
                        "clipDataId=%u",clip_sender_side_data.provider_name, clipDataId);
            }

            clip_receiver_side_data.file_stream_data_inventory.erase(clipDataId);
        }
    }
};

struct FileContentsResponseReceive {

    bool must_log_file_info_type = false;
    ClipboardSideData::file_info_type file_info;

    FileContentsResponseReceive(ClipboardSideData & clip_side_data, const RDPECLIP::CliprdrHeader & header, const uint32_t flags, InStream & chunk) {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

            clip_side_data.dataLen = header.dataLen();

            if (clip_side_data.dataLen >= 4) {
                clip_side_data.streamId = chunk.in_uint32_le();
            }
        }

        if (clip_side_data.file_contents_request_info_inventory.end() !=
            clip_side_data.file_contents_request_info_inventory.find(clip_side_data.streamId))
        {
            ClipboardSideData::file_contents_request_info& file_contents_request =
                clip_side_data.file_contents_request_info_inventory[clip_side_data.streamId];

            {
                ClipboardSideData::file_info_inventory_type& file_info_inventory =
                    clip_side_data.file_stream_data_inventory[
                        file_contents_request.clipDataId];

                this->file_info = file_info_inventory[file_contents_request.lindex];

                uint64_t const file_contents_request_position_current = file_contents_request.position + file_contents_request.offset;

                if (chunk.in_remain()) {
                    if (this->file_info.sequential_access_offset == file_contents_request_position_current) {

                        uint32_t const length_ = std::min({
                                static_cast<uint32_t>(chunk.in_remain()),
                                static_cast<uint32_t>(this->file_info.size - this->file_info.sequential_access_offset),
                                file_contents_request.cbRequested - file_contents_request.offset
                            });

                        this->file_info.sha256.update({ chunk.get_current(), length_ });

                        file_contents_request.offset       += length_;
                        this->file_info.sequential_access_offset += length_;

                        this->must_log_file_info_type = this->file_info.sequential_access_offset == this->file_info.size;
                    }
                }
            }
        }
    }
};

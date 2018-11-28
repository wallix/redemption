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
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "core/channel_list.hpp"
#include "core/front_api.hpp"
#include "core/RDP/clipboard.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/stream.hpp"
#include "utils/key_qvalue_pairs.hpp"

#include <map>
#include <memory>

#define FILE_LIST_FORMAT_NAME "FileGroupDescriptorW"

class ClipboardVirtualChannel final : public BaseVirtualChannel
{
private:
    using format_name_inventory_type = std::map<uint32_t, std::string>;
    format_name_inventory_type format_name_inventory;

    uint16_t client_message_type = 0;
    uint16_t server_message_type = 0;

    bool client_use_long_format_names = false;
    bool server_use_long_format_names = false;

    uint32_t client_file_list_format_id = 0;
    uint32_t server_file_list_format_id = 0;

    uint32_t requestedFormatId = 0;

    const bool param_clipboard_down_authorized;
    const bool param_clipboard_up_authorized;
    const bool param_clipboard_file_authorized;

    const bool param_dont_log_data_into_syslog;
    const bool param_dont_log_data_into_wrm;

    StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

    FrontAPI& front;

    SessionProbeLauncher* clipboard_monitor_ready_notifier = nullptr;
    SessionProbeLauncher* clipboard_initialize_notifier    = nullptr;
    SessionProbeLauncher* format_list_notifier             = nullptr;
    SessionProbeLauncher* format_list_response_notifier    = nullptr;
    SessionProbeLauncher* format_data_request_notifier     = nullptr;

    const bool proxy_managed;   // Has not client.

    std::unique_ptr<uint8_t[]> first_client_format_list_pdu;
    size_t                     first_client_format_list_pdu_length = 0;
    uint32_t                   first_client_format_list_pdu_flags  = 0;

public:
    struct Params : public BaseVirtualChannel::Params {
        bool clipboard_down_authorized;
        bool clipboard_up_authorized;
        bool clipboard_file_authorized;

        bool dont_log_data_into_syslog;
        bool dont_log_data_into_wrm;

        Params(ReportMessageApi & report_message) : BaseVirtualChannel::Params(report_message) {}
    };

    ClipboardVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        const Params & params)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         params)
    , param_clipboard_down_authorized(params.clipboard_down_authorized)
    , param_clipboard_up_authorized(params.clipboard_up_authorized)
    , param_clipboard_file_authorized(params.clipboard_file_authorized)
    , param_dont_log_data_into_syslog(params.dont_log_data_into_syslog)
    , param_dont_log_data_into_wrm(params.dont_log_data_into_wrm)

    , front(front)
    , proxy_managed(to_client_sender_ == nullptr) {
    }

protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "CLIPBOARD_LIMIT";
    }

public:
    bool use_long_format_names() const {
        return (this->client_use_long_format_names &&
            this->server_use_long_format_names);
    }

private:
    template<class PDU, class... Args>
    void send_pdu_to_client(bool response_ok, Args&&... args) {
        PDU             pdu(response_ok);

        StaticOutStream<256> out_stream;

        pdu.emit(out_stream, args...);

        const uint32_t total_length      = out_stream.get_offset();
        const uint32_t flags             =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
        const uint8_t* chunk_data        = out_stream.get_data();
        const uint32_t chunk_data_length = total_length;

        this->send_message_to_client(
            total_length,
            flags,
            chunk_data,
            chunk_data_length);
    }

    template<class PDU, class... Args>
    void send_pdu_to_server(bool response_ok, Args&&... args) {
        PDU             pdu(response_ok);

        StaticOutStream<256> out_stream;

        pdu.emit(out_stream, args...);

        const uint32_t total_length      = out_stream.get_offset();
        const uint32_t flags             =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
        const uint8_t* chunk_data        = out_stream.get_data();
        const uint32_t chunk_data_length = total_length;

        this->send_message_to_server(
            total_length,
            flags,
            chunk_data,
            chunk_data_length);
    }

    bool process_client_clipboard_capabilities_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        {
            const unsigned int expected = 10;   // msgFlags(2) + dataLen(4) +
                                                //     cCapabilitiesSets(2) +
                                                //     pad1(2)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_clipboard_capabilities_pdu: "
                        "Truncated CLIPRDR_CAPS, need=%u remains=%zu",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        assert(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_clipboard_capabilities_pdu: "
                            "General Capability Set");
                    general_caps.log(LOG_INFO);
                }

                this->client_use_long_format_names =
                    (general_caps.generalFlags() &
                     RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }

        return true;
    }

    bool process_client_file_contents_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;
        (void)chunk;

        if (!this->param_clipboard_file_authorized) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_file_contents_request_pdu: "
                        "Requesting the contents of server file is denied.");
            }
            this->send_pdu_to_client<RDPECLIP::FileContentsResponse>(false);

            return false;
        }

        return true;
    }

    bool process_client_format_data_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (!this->param_clipboard_down_authorized) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                        "Serveur to client Clipboard operation is not allowed.");
            }

            this->send_pdu_to_client<RDPECLIP::FormatDataResponsePDU>(
                false, static_cast<uint8_t *>(nullptr), 0);

            return false;
        }

        {
            const unsigned int expected = 10;   // msgFlags(2) + dataLen(4) +
                                                //     requestedFormatId(4)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                        "Truncated CLIPRDR_FORMAT_DATA_REQUEST, need=%u remains=%zu",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        this->requestedFormatId = chunk.in_uint32_le();

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                    "requestedFormatId=%s(%u)",
                RDPECLIP::get_Format_name(this->requestedFormatId),
                this->requestedFormatId);
        }

        return true;
    }

    bool process_client_format_data_response_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            const auto saved_chunk_p = chunk.get_current();

            {
                const unsigned int expected = 6;    // msgFlags(2) +
                                                    //     dataLen(4)
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
                            "Truncated CB_FORMAT_DATA_RESPONSE, "
                            "need=%u remains=%zu",
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            const uint16_t msgFlags = chunk.in_uint16_le();
            if (msgFlags & RDPECLIP::CB_RESPONSE_OK) {
                const uint32_t dataLen = chunk.in_uint32_le();

                constexpr size_t const max_length_of_data_to_dump = 256;
                          std::string  data_to_dump;

                switch (this->requestedFormatId)
                {
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

                char const* type = (data_to_dump.empty() ?
                    "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION" :
                    "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX");

                std::string format_name = this->format_name_inventory[requestedFormatId];
                if (format_name.empty()) {
                    format_name = RDPECLIP::get_Format_name(this->requestedFormatId);
                }

                auto const size_str = std::to_string(dataLen);

                std::string info;
                ::key_qvalue_pairs(
                        info,
                        {
                            { "type", type },
                            { "format", format_name.c_str() },
                            { "size", size_str }
                        }
                    );
                if (!data_to_dump.empty()) {
                    ::key_qvalue_pairs(
                            info,
                            {
                                { "partial_data", data_to_dump }
                            }
                        );
                }

                this->report_message.log5(info);

                if (!this->param_dont_log_data_into_syslog) {
                    LOG(LOG_INFO, "%s", info);
                }

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message(type);
                    message += "=";
                    message += format_name.c_str();
                    message += "\x01";
                    message += size_str;
                    if (!data_to_dump.empty()) {
                        message += "\x01";
                        message += data_to_dump;
                    }

                    this->front.session_update(message);
                }
            }

            chunk.rewind(saved_chunk_p - chunk.get_data());
        }   // if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&

        //LOG(LOG_INFO,
        //    "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
        //    "requestedFormatId=%u client_file_list_format_id=%u",
        //    this->requestedFormatId, this->client_file_list_format_id);

        if (this->client_file_list_format_id &&
            (this->requestedFormatId == this->client_file_list_format_id)) {
            const auto saved_chunk_p = chunk.get_current();

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                {
                    const unsigned int expected = 6;    // msgFlags(2) +
                                                        //     dataLen(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
                                "Truncated Packed File List, "
                                "need=%u remains=%zu",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                const uint16_t msgFlags = chunk.in_uint16_le(); // msgFlags(2)
                const uint32_t dataLen  = chunk.in_uint32_le(); // dataLen(4)

                if (!(msgFlags & RDPECLIP::CB_RESPONSE_FAIL) && (dataLen >= 4 /* cItems(4) */)) {
                    const uint32_t cItems = chunk.in_uint32_le();

                    if (!this->param_dont_log_data_into_syslog) {
                        LOG(LOG_INFO,
                            "Sending %sFileGroupDescriptorW(%u) clipboard data to server. "
                                "cItems=%u",
                            ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                             "" : "(chunked) "),
                            this->client_file_list_format_id, cItems);
                    }
                }
            }
            else if (this->file_descriptor_stream.get_offset()) {
                const uint32_t complementary_data_length =
                    RDPECLIP::FileDescriptor::size() -
                        this->file_descriptor_stream.get_offset();

                assert(chunk.in_remain() >= complementary_data_length);

                this->file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                    complementary_data_length);

                chunk.in_skip_bytes(complementary_data_length);

                RDPECLIP::FileDescriptor fd;

                InStream in_stream(
                    this->file_descriptor_stream.get_data(),
                    this->file_descriptor_stream.get_offset()
                );
                fd.receive(in_stream);
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                this->log_file_descriptor(fd);

                this->file_descriptor_stream.rewind();
            }

            while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                //{
                //    auto chunk_p_ = chunk.get_current();

                    fd.receive(chunk);

                //    LOG(LOG_INFO, "FileDescriptor: size=%u",
                //        (unsigned int)(chunk.get_current() - chunk_p_));
                //    hexdump(chunk_p_, chunk.get_current() - chunk_p_);
                //}

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                this->log_file_descriptor(fd);
            }

            if (chunk.in_remain()) {
                this->file_descriptor_stream.rewind();

                this->file_descriptor_stream.out_copy_bytes(
                    chunk.get_current(), chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->requestedFormatId = 0;
            }

            chunk.rewind(saved_chunk_p - chunk.get_data());
        }   // if (this->client_file_list_format_id &&

        return true;
    }   // process_client_format_data_response_pdu

private:
    void log_file_descriptor(RDPECLIP::FileDescriptor fd)
    {
        auto const file_size_str = std::to_string(fd.file_size());
        auto const info = key_qvalue_pairs({
            {"type", "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION"},
            {"file_name", fd.fileName()},
            {"size", file_size_str}
            });

        this->report_message.log5(info);

        if (!this->param_dont_log_data_into_syslog){
            LOG(LOG_INFO, "%s", info);
        }

        if (!this->param_dont_log_data_into_wrm) {
            std::string message("CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION=");
            message += fd.fileName();
            message += "\x01";
            message += file_size_str;

            this->front.session_update(message);
        }
    }

public:
    bool process_client_format_list_pdu(uint32_t total_length, uint32_t flags,
        InStream& chunk)
    {
        (void)total_length;

        if (!this->param_clipboard_down_authorized &&
            !this->param_clipboard_up_authorized &&
            !this->format_list_response_notifier) {
            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_client_format_list_pdu: "
                    "Clipboard is fully disabled.");

            this->send_pdu_to_client<RDPECLIP::FormatListResponsePDU>(
                true);

            return false;
        }

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            LOG(LOG_ERR,
                "ClipboardVirtualChannel::process_client_format_list_pdu: "
                    "!!!CHUNKED!!! Format List PDU is not yet supported!");

            this->send_pdu_to_client<RDPECLIP::FormatListResponsePDU>(
                true);

            return false;
        }

        {
            const unsigned int expected = 6;    // msgFlags(2) +
                                                //     dataLen(4)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Truncated CLIPRDR_FORMAT_LIST, "
                        "need=%u remains=%zu",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        const uint16_t msgFlags = chunk.in_uint16_le();
        const uint32_t dataLen  = chunk.in_uint32_le();

        if (!this->first_client_format_list_pdu) {
            this->first_client_format_list_pdu_length =
                    2 + // msgType(2)
                    2 + // msgFlags(2)
                    4 + // dataLen(4)
                    dataLen
                ;
            this->first_client_format_list_pdu =
                std::make_unique<uint8_t[]>(
                    this->first_client_format_list_pdu_length);

            ::memcpy(this->first_client_format_list_pdu.get(),
                chunk.get_data(), this->first_client_format_list_pdu_length);

            first_client_format_list_pdu_flags = flags;
        }

        this->format_name_inventory.clear();

        if (!this->client_use_long_format_names ||
            !this->server_use_long_format_names) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Short Format Name%s variant of Format List PDU is used "
                        "for exchanging updated format names.",
                    ((msgFlags & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)"
                                                           : ""));
            }

            for (uint32_t remaining_data_length = dataLen;
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

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_Format_name(formatId), formatId,
                        utf8_string);
                }

                this->format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

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
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }



            for (uint32_t remaining_data_length = dataLen;
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

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_Format_name(formatId), formatId,
                        utf8_string);
                }

                this->format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

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

        return true;
    }

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) override
    {
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (bool(this->verbose & RDPVerbose::cliprdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* msgType(2) */)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_message: "
                        "Truncated msgType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->client_message_type = chunk.in_uint16_le();
        }

        bool send_message_to_server = true;

        switch (this->client_message_type)
        {
            case RDPECLIP::CB_CLIP_CAPS:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Clipboard Capabilities PDU");
                }

                send_message_to_server =
                    this->process_client_clipboard_capabilities_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_LIST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Format List PDU");
                }

                send_message_to_server =
                    this->process_client_format_list_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Format Data Request PDU");
                }

                send_message_to_server =
                    this->process_client_format_data_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "File Contents Request PDU");
                }

                send_message_to_server =
                    this->process_client_file_contents_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Format Data Response PDU");
                }

                send_message_to_server =
                    this->process_client_format_data_response_pdu(
                        total_length, flags, chunk);

                if (send_message_to_server &&
                    (flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    this->update_exchanged_data(total_length);
                }
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "File Contents Response PDU");
                }

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }
            break;

            default:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Delivering unprocessed messages %s(%u) to server.",
                        RDPECLIP::get_msgType_name(this->client_message_type),
                        static_cast<unsigned>(this->client_message_type));
                }
            break;
        }   // switch (this->client_message_type)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_client_message

    bool process_server_clipboard_capabilities_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        assert(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_clipboard_capabilities_pdu: "
                            "General Capability Set");
                    general_caps.log(LOG_INFO);
                }

                this->server_use_long_format_names =
                    (general_caps.generalFlags() &
                     RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }

        if (this->proxy_managed) {
            return false;
        }

        return true;
    }

    bool process_server_file_contents_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;
        (void)chunk;

        if (!this->param_clipboard_file_authorized) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_file_contents_request_pdu: "
                        "Requesting the contents of client file is denied.");
            }
            this->send_pdu_to_server<RDPECLIP::FileContentsResponse>(false);

            return false;
        }

        return true;
    }

    bool process_server_format_data_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        this->requestedFormatId = chunk.in_uint32_le();

        if (this->format_data_request_notifier &&
            (this->requestedFormatId == RDPECLIP::CF_TEXT)) {
            if (!this->format_data_request_notifier->on_server_format_data_request()) {
                this->format_data_request_notifier = nullptr;
            }

            return false;
        }

        if (!this->param_clipboard_up_authorized) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                        "Client to server Clipboard operation is not allowed.");
            }

            this->send_pdu_to_server<RDPECLIP::FormatDataResponsePDU>(
                false, static_cast<uint8_t *>(nullptr), 0);

            return false;
        }

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                    "requestedFormatId=%s(%u)",
                RDPECLIP::get_Format_name(this->requestedFormatId),
                this->requestedFormatId);
        }

        return true;
    }

    bool process_server_format_data_response_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk)
    {
        (void)total_length;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            const auto saved_chunk_p = chunk.get_current();

            const uint16_t msgFlags = chunk.in_uint16_le();
            if (msgFlags & RDPECLIP::CB_RESPONSE_OK) {
                const uint32_t dataLen = chunk.in_uint32_le();

                constexpr size_t const max_length_of_data_to_dump = 256;
                          std::string  data_to_dump;

                switch (this->requestedFormatId) {
/*
                    case RDPECLIP::CF_TEXT:
                    {
                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(),
                            max_length_of_data_to_dump);
                        const std::string data_to_dump(::char_ptr_cast(
                            chunk.get_current()), length_of_data_to_dump);
                        LOG(LOG_INFO, "%s", data_to_dump);
                    }
                    break;
*/

                    case RDPECLIP::CF_UNICODETEXT:
                    {
                        assert(!(chunk.in_remain() & 1));

                        const size_t length_of_data_to_dump = std::min(
                            chunk.in_remain(),
                            max_length_of_data_to_dump * 2);

                        constexpr size_t size_of_utf8_string =
                            max_length_of_data_to_dump *
                                maximum_length_of_utf8_character_in_bytes;

                        uint8_t utf8_string[size_of_utf8_string + 1];
                        ::memset(utf8_string, 0, sizeof(utf8_string));
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

                char const* type = (data_to_dump.empty() ?
                    "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION" :
                    "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX");

                std::string format_name = this->format_name_inventory[requestedFormatId];
                if (format_name.empty()) {
                    format_name = RDPECLIP::get_Format_name(this->requestedFormatId);
                }

                auto const size_str = std::to_string(dataLen);

                std::string info;
                ::key_qvalue_pairs(
                        info,
                        {
                            { "type", type },
                            { "format", format_name.c_str() },
                            { "size", size_str }
                        }
                    );
                if (!data_to_dump.empty()) {
                    ::key_qvalue_pairs(
                            info,
                            {
                                { "partial_data", data_to_dump }
                            }
                        );
                }

                this->report_message.log5(info);

                if (!this->param_dont_log_data_into_syslog) {
                    LOG(LOG_INFO, "%s", info);
                }

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message(type);
                    message += "=";
                    message += format_name.c_str();
                    message += "\x01";
                    message += size_str;
                    if (!data_to_dump.empty()) {
                        message += "\x01";
                        message += data_to_dump;
                    }

                    this->front.session_update(message);
                }
            }

            chunk.rewind(saved_chunk_p - chunk.get_data());
        }   // if ((flags & CHANNELS::CHANNEL_FLAG_FIRST) &&

        //LOG(LOG_INFO,
        //    "ClipboardVirtualChannel::process_server_format_data_response_pdu: "
        //    "requestedFormatId=%u server_file_list_format_id=%u",
        //    this->requestedFormatId, this->server_file_list_format_id);

        if (this->server_file_list_format_id &&
            (this->requestedFormatId == this->server_file_list_format_id)) {
            const auto saved_chunk_p = chunk.get_current();

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                chunk.in_skip_bytes(6 /* msgFlags(2) + dataLen(4) */);

                const uint32_t cItems = chunk.in_uint32_le();

                if (!this->param_dont_log_data_into_syslog) {
                    LOG(LOG_INFO,
                        "Sending %sFileGroupDescriptorW(%u) clipboard data to client. "
                            "cItems=%u",
                        ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                         "" : "(chunked) "),
                        this->server_file_list_format_id, cItems);
                }
            }
            else if (this->file_descriptor_stream.get_offset()) {
                const uint32_t complementary_data_length =
                    RDPECLIP::FileDescriptor::size() -
                        this->file_descriptor_stream.get_offset();

                assert(chunk.in_remain() >= complementary_data_length);

                this->file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                    complementary_data_length);

                chunk.in_skip_bytes(complementary_data_length);

                RDPECLIP::FileDescriptor fd;

                InStream in_stream(
                    this->file_descriptor_stream.get_data(),
                    this->file_descriptor_stream.get_offset()
                );
                fd.receive(in_stream);
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                auto const file_size_str = std::to_string(fd.file_size());
                auto const info = key_qvalue_pairs({
                    {"type", "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"},
                    {"file_name", fd.fileName()},
                    {"size", file_size_str}
                    });

                this->report_message.log5(info);

                if (!this->param_dont_log_data_into_syslog){
                    LOG(LOG_INFO, "%s", info);
                }

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION=");
                    message += fd.fileName();
                    message += "\x01";
                    message += file_size_str;

                    this->front.session_update(message);
                }

                this->file_descriptor_stream.rewind();
            }

            while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                fd.receive(chunk);
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    fd.log(LOG_INFO);
                }

                auto const file_size_str = std::to_string(fd.file_size());
                auto const info = key_qvalue_pairs({
                    {"type", "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"},
                    {"file_name", fd.fileName()},
                    {"size", file_size_str}
                    });

                this->report_message.log5(info);

                if (!this->param_dont_log_data_into_syslog){
                    LOG(LOG_INFO, "%s", info);
                }

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION=");
                    message += fd.fileName();
                    message += "\x01";
                    message += file_size_str;

                    this->front.session_update(message);
                }
            }

            if (chunk.in_remain()) {
                this->file_descriptor_stream.rewind();

                this->file_descriptor_stream.out_copy_bytes(
                    chunk.get_current(), chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->requestedFormatId = 0;
            }

            chunk.rewind(saved_chunk_p - chunk.get_data());
        }

        return true;
    }   // process_server_format_data_response_pdu

    bool process_server_format_list_pdu(uint32_t total_length, uint32_t flags,
        InStream& chunk)
    {
        (void)total_length;
        (void)flags;

        if (!this->param_clipboard_down_authorized &&
            !this->param_clipboard_up_authorized) {
            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_server_format_list_pdu: "
                    "Clipboard is fully disabled.");

            this->send_pdu_to_server<RDPECLIP::FormatListResponsePDU>(
                true);

            return false;
        }

        const uint16_t msgFlags = chunk.in_uint16_le();
        const uint32_t dataLen  = chunk.in_uint32_le();

        this->format_name_inventory.clear();

        if (!this->client_use_long_format_names ||
            !this->server_use_long_format_names) {
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Short Format Name%s variant of Format List PDU is used "
                        "for exchanging updated format names.",
                    ((msgFlags & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)"
                                                           : ""));
            }

            for (uint32_t remaining_data_length = dataLen;
                 remaining_data_length; ) {
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

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_Format_name(formatId), formatId,
                        utf8_string);
                }

                this->format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

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
            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }

            const size_t max_length_of_format_name = 256;

            for (uint32_t remaining_data_length = dataLen;
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

                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_format_list_pdu: "
                            "formatId=%s(%u) wszFormatName=\"%s\"",
                        RDPECLIP::get_Format_name(formatId), formatId, utf8_string);
                }

                this->format_name_inventory[formatId] = ::char_ptr_cast(utf8_string);

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

        return true;
    }   // process_server_format_list_pdu

    bool process_server_monitor_ready_pdu(uint32_t total_length, uint32_t flags,
        InStream& chunk)
    {
        (void)total_length;
        (void)flags;
        (void)chunk;

        if (this->proxy_managed) {
            // Client Clipboard Capabilities PDU.
            {
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                            "Send Clipboard Capabilities PDU.");
                }

                RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1,
                    RDPECLIP::GeneralCapabilitySet::size());
                RDPECLIP::GeneralCapabilitySet general_cap_set(
                    RDPECLIP::CB_CAPS_VERSION_1,
                    RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

                StaticOutStream<1024> out_stream;

                clipboard_caps_pdu.emit(out_stream);
                general_cap_set.emit(out_stream);

                const uint32_t total_length      = out_stream.get_offset();
                const uint32_t flags             =
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
                const uint8_t* chunk_data        = out_stream.get_data();
                const uint32_t chunk_data_length = total_length;

                this->send_message_to_server(
                    total_length,
                    flags,
                    chunk_data,
                    chunk_data_length);
            }

            this->client_use_long_format_names = true;

            // Format List PDU.
            {
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                            "Send Clipboard Capabilities PDU.");
                }

                RDPECLIP::FormatListPDU format_list_pdu;
                StaticOutStream<1024> out_stream;

                const bool unicodetext = false;

                format_list_pdu.emit_2(out_stream, unicodetext,
                    this->use_long_format_names());

                const uint32_t total_length      = out_stream.get_offset();
                const uint32_t flags             =
                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
                const uint8_t* chunk_data        = out_stream.get_data();
                const uint32_t chunk_data_length = total_length;

                this->send_message_to_server(
                    total_length,
                    flags,
                    chunk_data,
                    chunk_data_length);
            }

            return false;
        }
        else {
            if (this->clipboard_monitor_ready_notifier) {
                if (!this->clipboard_monitor_ready_notifier->on_clipboard_monitor_ready()) {
                    this->clipboard_monitor_ready_notifier = nullptr;
                }
            }
        }

        return true;
    }

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (bool(this->verbose & RDPVerbose::cliprdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* msgType(2) */)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_server_message: "
                        "Truncated msgType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->server_message_type = chunk.in_uint16_le();
        }

        bool send_message_to_client = true;

        switch (this->server_message_type)
        {
            case RDPECLIP::CB_CLIP_CAPS:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Clipboard Capabilities PDU");
                }

                send_message_to_client =
                    this->process_server_clipboard_capabilities_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "File Contents Request PDU");
                }

                send_message_to_client =
                    this->process_server_file_contents_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "File Contents Response PDU");
                }

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format Data Request PDU");
                }

                send_message_to_client =
                    this->process_server_format_data_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format Data Response PDU");
                }

                send_message_to_client =
                    this->process_server_format_data_response_pdu(
                        total_length, flags, chunk);

                if (send_message_to_client &&
                    (flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    this->update_exchanged_data(total_length);
                }
            break;

            case RDPECLIP::CB_FORMAT_LIST:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format List PDU");
                }

                send_message_to_client =
                    this->process_server_format_list_pdu(
                        total_length, flags, chunk);

                if (this->format_list_notifier) {
                    if (!this->format_list_notifier->on_server_format_list()) {
                        this->format_list_notifier = nullptr;
                    }
                }
            break;

            case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format List Response PDU");
                }

                if (this->clipboard_initialize_notifier) {
                    if (!this->clipboard_initialize_notifier->on_clipboard_initialize()) {
                        this->clipboard_initialize_notifier = nullptr;
                    }
                }
                else if (this->format_list_response_notifier) {
                    if (!this->format_list_response_notifier->on_server_format_list_response()) {
                        this->format_list_response_notifier = nullptr;
                    }
                }
            break;

            case RDPECLIP::CB_MONITOR_READY:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Monitor Ready PDU");
                }

                send_message_to_client =
                    this->process_server_monitor_ready_pdu(
                        total_length, flags, chunk);
            break;

            default:
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Delivering unprocessed messages %s(%u) to client.",
                        RDPECLIP::get_msgType_name(this->server_message_type),
                        static_cast<unsigned>(this->server_message_type));
                }
            break;
        }   // switch (this->server_message_type)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data,
                chunk_data_length);
        }   // switch (this->server_message_type)
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->clipboard_monitor_ready_notifier = launcher;
        this->clipboard_initialize_notifier    = launcher;
        this->format_list_notifier             = launcher;
        this->format_list_response_notifier    = launcher;
        this->format_data_request_notifier     = launcher;
    }

    void empty_client_clipboard() {
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::empty_client_clipboard");
        }

        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            0, 0);

        StaticOutStream<256> out_s;

        clipboard_header.emit(out_s);

        const size_t totalLength = out_s.get_offset();

        this->send_message_to_server(
            totalLength,
            this->first_client_format_list_pdu_flags,
            out_s.get_data(),
            totalLength);
    }
};  // class ClipboardVirtualChannel


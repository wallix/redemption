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

#ifndef REDEMPTION_MOD_RDP_CHANNELS_CLIPRDRCHANNEL_HPP
#define REDEMPTION_MOD_RDP_CHANNELS_CLIPRDRCHANNEL_HPP

#include "base_channel.hpp"
#include "front_api.hpp"

#define FILE_LIST_FORMAT_NAME "FileGroupDescriptorW"

class ClipboardVirtualChannel : public BaseVirtualChannel
{
private:
    uint16_t client_message_type = 0;
    uint16_t server_message_type = 0;

    bool client_use_long_format_name = false;
    bool server_use_long_format_name = false;

    uint32_t client_file_list_format_id = 0;
    uint32_t server_file_list_format_id = 0;

    uint32_t requestedFormatId = 0;

    const bool param_clipboard_down_authorized;
    const bool param_clipboard_up_authorized;
    const bool param_clipboard_file_authorized;

    const bool param_dont_log_data_into_syslog;
    const bool param_dont_log_data_into_wrm;

    std::unique_ptr<uint8_t[]> file_descriptor_data;
    FixedSizeStream            file_descriptor_stream;

    FrontAPI& front;

public:
    struct Params : public BaseVirtualChannel::Params {
        bool clipboard_down_authorized;
        bool clipboard_up_authorized;
        bool clipboard_file_authorized;

        bool dont_log_data_into_syslog;
        bool dont_log_data_into_wrm;
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
    , file_descriptor_data(
          std::make_unique<uint8_t[]>(RDPECLIP::FileDescriptor::size()))
    , file_descriptor_stream(
          file_descriptor_data.get(), RDPECLIP::FileDescriptor::size())
    , front(front) {}

    inline virtual const char*
        get_reporting_reason_exchanged_data_limit_reached() const override
    {
        return "CLIPBOARD_LIMIT";
    }

private:
    template<class PDU, class... Args>
    void send_pdu_to_client(bool response_ok, Args&&... args) {
        PDU             pdu(response_ok);

        uint8_t         out_data[256];
        WriteOnlyStream out_stream(out_data, sizeof(out_data));

        pdu.emit(out_stream, args...);

        const uint32_t total_length      = out_stream.size();
        const uint32_t flags             =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
        const uint8_t* chunk_data        = out_data;
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

        uint8_t         out_data[256];
        WriteOnlyStream out_stream(out_data, sizeof(out_data));

        pdu.emit(out_stream, args...);

        const uint32_t total_length      = out_stream.size();
        const uint32_t flags             =
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
        const uint8_t* chunk_data        = out_data;
        const uint32_t chunk_data_length = total_length;

        this->send_message_to_server(
            total_length,
            flags,
            chunk_data,
            chunk_data_length);
    }

    bool process_client_clipboard_capabilities_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        {
            const unsigned int expected = 10;   // msgFlags(2) + dataLen(4) +
                                                //     cCapabilitiesSets(2) +
                                                //     pad1(2)
            if (!chunk.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_clipboard_capabilities_pdu: "
                        "Truncated CLIPRDR_CAPS, need=%u remains=%u",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        REDASSERT(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_clipboard_capabilities_pdu: "
                            "General Capability Set");
                    general_caps.log(LOG_INFO);
                }

                this->client_use_long_format_name =
                    (general_caps.generalFlags() &
                     RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }

        return true;
    }

    bool process_client_file_contents_request_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_file_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
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
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_down_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
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
                        "Truncated CLIPRDR_FORMAT_DATA_REQUEST, need=%u remains=%u",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        this->requestedFormatId = chunk.in_uint32_le();

        return true;
    }

    bool process_client_format_data_response_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (this->param_dont_log_data_into_syslog) {
            return true;
        }

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            const auto saved_chunk_p = chunk.p;

            {
                const unsigned int expected = 6;    // msgFlags(2) +
                                                    //     dataLen(4)
                if (!chunk.in_check_rem(expected)) {
                    LOG(LOG_ERR,
                        "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
                            "Truncated CB_FORMAT_DATA_RESPONSE, "
                            "need=%u remains=%u",
                        expected, chunk.in_remain());
                    throw Error(ERR_RDP_DATA_TRUNCATED);
                }
            }

            chunk.in_skip_bytes(2 /* msgFlags(2) */);

            const uint32_t dataLen = chunk.in_uint32_le();

            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "Sending %s(%u) clipboard data to server (%u) bytes%s",
                    RDPECLIP::get_Format_name(this->requestedFormatId),
                    this->requestedFormatId, dataLen,
                    (((this->requestedFormatId == RDPECLIP::CF_TEXT) ||
                      (this->requestedFormatId == RDPECLIP::CF_UNICODETEXT)) ?
                     ":" : "."));
            }

            const size_t max_length_of_data_to_dump = 256;

            if (this->requestedFormatId == RDPECLIP::CF_TEXT) {
                const size_t length_of_data_to_dump =
                    std::min(chunk.in_remain(), max_length_of_data_to_dump);
                const std::string data_to_dump(::char_ptr_cast(chunk.p),
                    length_of_data_to_dump);
                LOG(LOG_INFO, data_to_dump.c_str());
            }
            else if (this->requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                REDASSERT(!(chunk.in_remain() & 1));

                const size_t length_of_data_to_dump =
                    std::min(chunk.in_remain(),
                             max_length_of_data_to_dump * 2);

                const size_t size_of_utf8_string =
                    length_of_data_to_dump / 2 *
                        maximum_length_of_utf8_character_in_bytes;

                uint8_t * const utf8_string = static_cast<uint8_t *>(
                    ::alloca(size_of_utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.p, length_of_data_to_dump / 2, utf8_string,
                    size_of_utf8_string);
                const std::string data_to_dump(::char_ptr_cast(utf8_string),
                    length_of_utf8_string);
                LOG(LOG_INFO, data_to_dump.c_str());
            }

            chunk.p = saved_chunk_p;
        }

        //LOG(LOG_INFO,
        //    "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
        //    "requestedFormatId=%u server_file_list_format_id=%u",
        //    this->requestedFormatId, this->server_file_list_format_id);

        if (this->client_file_list_format_id &&
            (this->requestedFormatId == this->client_file_list_format_id)) {
            const auto saved_chunk_p = chunk.p;

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                {
                    const unsigned int expected = 10;   // msgFlags(2) +
                                                        //     dataLen(4) +
                                                        //     cItems(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "ClipboardVirtualChannel::process_client_format_data_response_pdu: "
                                "Truncated Packed File List, "
                                "need=%u remains=%u",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                chunk.in_skip_bytes(6 /* msgFlags(2) + dataLen(4) */);

                const uint32_t cItems = chunk.in_uint32_le();

                LOG(LOG_INFO,
                    "Sending %sFileGroupDescriptorW(%u) clipboard data to server. "
                        "cItems=%u",
                    ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                     "" : "(chunked) "),
                    this->client_file_list_format_id, cItems);
            }
            else if (this->file_descriptor_stream.get_offset()) {
                const uint32_t complementary_data_length =
                    RDPECLIP::FileDescriptor::size() -
                        this->file_descriptor_stream.get_offset();

                REDASSERT(chunk.in_remain() >= complementary_data_length);

                this->file_descriptor_stream.out_copy_bytes(chunk.p,
                    complementary_data_length);

                chunk.in_skip_bytes(complementary_data_length);

                this->file_descriptor_stream.mark_end();
                this->file_descriptor_stream.rewind();

                RDPECLIP::FileDescriptor fd;

                fd.receive(this->file_descriptor_stream);
                fd.log(LOG_INFO);

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("SendFileToServerClipboard=");
                    message += fd.fileName();

                    this->front.session_update(message.c_str());
                }

                this->file_descriptor_stream.reset();
            }

            while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                fd.receive(chunk);
                fd.log(LOG_INFO);

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("SendFileToServerClipboard=");
                    message += fd.fileName();

                    this->front.session_update(message.c_str());
                }
            }

            if (chunk.in_remain()) {
                this->file_descriptor_stream.reset();

                this->file_descriptor_stream.out_copy_bytes(
                    chunk.p, chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->requestedFormatId = 0;
            }

            chunk.p = saved_chunk_p;
        }

        return true;
    }   // process_client_format_data_response_pdu

    bool process_client_format_list_pdu(uint32_t total_length, uint32_t flags,
        ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_down_authorized &&
            !this->param_clipboard_up_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Clipboard is fully disabled.");
            }

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
                        "need=%u remains=%u",
                    expected, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }
        }

        const uint16_t msgFlags = chunk.in_uint16_le();
        const uint32_t dataLen  = chunk.in_uint32_le();

        if (!this->client_use_long_format_name ||
            !this->server_use_long_format_name) {
            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_client_format_list_pdu: "
                    "Short Format Name%s variant of Format List PDU is used "
                    "for exchanging updated format names.",
                ((msgFlags & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)" : ""));

            // File List data cannot be detected!!!
            this->client_file_list_format_id = 0;
        }
        else {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }

            for (uint32_t remaining_data_length = dataLen;
                 remaining_data_length; ) {
                {
                    const unsigned int expected = 4;    // formatId(4)
                    if (!chunk.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "ClipboardVirtualChannel::process_client_format_list_pdu: "
                                "Truncated CLIPRDR_FORMAT_LIST, "
                                "need=%u remains=%u",
                            expected, chunk.in_remain());
                        throw Error(ERR_RDP_DATA_TRUNCATED);
                    }
                }

                const uint32_t formatId           = chunk.in_uint32_le();
                const size_t   format_name_length = UTF16StrLen(chunk.p) + 1;

                const size_t size_of_utf8_string =
                    format_name_length *
                    maximum_length_of_utf8_character_in_bytes;
                uint8_t * const utf8_string = static_cast<uint8_t *>(
                    ::alloca(size_of_utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.p, format_name_length, utf8_string,
                    size_of_utf8_string);

                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_list_pdu: "
                            "formatId=%d wszFormatName=\"%s\"",
                        formatId, utf8_string);
                }

                remaining_data_length -=
                          4                      /* formatId(4) */
                        + format_name_length * 2 /* wszFormatName(variable) */
                    ;

                if ((sizeof(FILE_LIST_FORMAT_NAME) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->client_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(format_name_length * 2);
            }
        }

        return true;
    }

public:
    virtual void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) override
    {
        if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_client_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        ReadOnlyStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* msgType(2) */)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_message: "
                        "Truncated msgType, need=2 remains=%u",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->client_message_type = chunk.in_uint16_le();
        }

        bool send_message_to_server = true;

        switch (this->client_message_type)
        {
            case RDPECLIP::CB_CLIP_CAPS:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Clipboard Capabilities PDU");
                }

                send_message_to_server =
                    this->process_client_clipboard_capabilities_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_LIST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Format List PDU");
                }

                send_message_to_server =
                    this->process_client_format_list_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "Format Data Request PDU");
                }

                send_message_to_server =
                    this->process_client_format_data_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "File Contents Request PDU");
                }

                send_message_to_server =
                    this->process_client_file_contents_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
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
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "File Contents Response PDU");
                }

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }
            break;
        }   // switch (this->client_message_type)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_client_message

    bool process_server_clipboard_capabilities_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        REDASSERT(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_clipboard_capabilities_pdu: "
                            "General Capability Set");
                    general_caps.log(LOG_INFO);
                }

                this->server_use_long_format_name =
                    (general_caps.generalFlags() &
                     RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }

        return true;
    }

    bool process_server_file_contents_request_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_file_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
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
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_up_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                        "Client to server Clipboard operation is not allowed.");
            }

            this->send_pdu_to_server<RDPECLIP::FormatDataResponsePDU>(
                false, static_cast<uint8_t *>(nullptr), 0);

            return false;
        }

        chunk.in_skip_bytes(6); // msgFlags(2) + dataLen(4)

        this->requestedFormatId = chunk.in_uint32_le();

        return true;
    }

    bool process_server_format_data_response_pdu(uint32_t total_length,
        uint32_t flags, ReadOnlyStream& chunk)
    {
        if (this->param_dont_log_data_into_syslog) {
            return true;
        }

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            const auto saved_chunk_p = chunk.p;

            chunk.in_skip_bytes(2 /* msgFlags(2) */);

            const uint32_t dataLen = chunk.in_uint32_le();

            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO, "Sending %s(%u) clipboard data to client (%u) bytes%s",
                    RDPECLIP::get_Format_name(this->requestedFormatId),
                    this->requestedFormatId, dataLen,
                    (((this->requestedFormatId == RDPECLIP::CF_TEXT) ||
                      (this->requestedFormatId == RDPECLIP::CF_UNICODETEXT)) ?
                     ":" : "."));
            }

            const size_t max_length_of_data_to_dump = 256;

            if (this->requestedFormatId == RDPECLIP::CF_TEXT) {
                const size_t length_of_data_to_dump =
                    std::min(chunk.in_remain(), max_length_of_data_to_dump);
                const std::string data_to_dump(::char_ptr_cast(chunk.p),
                    length_of_data_to_dump);
                LOG(LOG_INFO, data_to_dump.c_str());
            }
            else if (this->requestedFormatId == RDPECLIP::CF_UNICODETEXT) {
                REDASSERT(!(chunk.in_remain() & 1));

                const size_t length_of_data_to_dump =
                    std::min(chunk.in_remain(),
                             max_length_of_data_to_dump * 2);

                const size_t size_of_utf8_string =
                    length_of_data_to_dump / 2 *
                        maximum_length_of_utf8_character_in_bytes;

                uint8_t * const utf8_string = static_cast<uint8_t *>(
                    ::alloca(size_of_utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.p, length_of_data_to_dump / 2, utf8_string,
                    size_of_utf8_string);
                const std::string data_to_dump(::char_ptr_cast(utf8_string),
                    length_of_utf8_string);
                LOG(LOG_INFO, data_to_dump.c_str());
            }

            chunk.p = saved_chunk_p;
        }

        //LOG(LOG_INFO,
        //    "ClipboardVirtualChannel::process_server_format_data_response_pdu: "
        //    "requestedFormatId=%u server_file_list_format_id=%u",
        //    this->requestedFormatId, this->server_file_list_format_id);

        if (this->server_file_list_format_id &&
            (this->requestedFormatId == this->server_file_list_format_id)) {
            const auto saved_chunk_p = chunk.p;

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                chunk.in_skip_bytes(6 /* msgFlags(2) + dataLen(4) */);

                const uint32_t cItems = chunk.in_uint32_le();

                LOG(LOG_INFO,
                    "Sending %sFileGroupDescriptorW(%u) clipboard data to client. "
                        "cItems=%u",
                    ((flags & CHANNELS::CHANNEL_FLAG_LAST) ?
                     "" : "(chunked) "),
                    this->server_file_list_format_id, cItems);
            }
            else if (this->file_descriptor_stream.get_offset()) {
                const uint32_t complementary_data_length =
                    RDPECLIP::FileDescriptor::size() -
                        this->file_descriptor_stream.get_offset();

                REDASSERT(chunk.in_remain() >= complementary_data_length);

                this->file_descriptor_stream.out_copy_bytes(chunk.p,
                    complementary_data_length);

                chunk.in_skip_bytes(complementary_data_length);

                this->file_descriptor_stream.mark_end();
                this->file_descriptor_stream.rewind();

                RDPECLIP::FileDescriptor fd;

                fd.receive(this->file_descriptor_stream);
                fd.log(LOG_INFO);

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("SendFileToClientClipboard=");
                    message += fd.fileName();

                    this->front.session_update(message.c_str());
                }

                this->file_descriptor_stream.reset();
            }

            while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
                RDPECLIP::FileDescriptor fd;

                fd.receive(chunk);
                fd.log(LOG_INFO);

                if (!this->param_dont_log_data_into_wrm) {
                    std::string message("SendFileToClientClipboard=");
                    message += fd.fileName();

                    this->front.session_update(message.c_str());
                }
            }

            if (chunk.in_remain()) {
                this->file_descriptor_stream.reset();

                this->file_descriptor_stream.out_copy_bytes(
                    chunk.p, chunk.in_remain());

                chunk.in_skip_bytes(chunk.in_remain());
            }

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->requestedFormatId = 0;
            }

            chunk.p = saved_chunk_p;
        }

        return true;
    }   // process_server_format_data_response_pdu

    bool process_server_format_list_pdu(uint32_t total_length, uint32_t flags,
        ReadOnlyStream& chunk)
    {
        if (!this->param_clipboard_down_authorized &&
            !this->param_clipboard_up_authorized) {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Clipboard is fully disabled.");
            }

            this->send_pdu_to_server<RDPECLIP::FormatListResponsePDU>(
                true);

            return false;
        }

        const uint16_t msgFlags = chunk.in_uint16_le();
        const uint32_t dataLen  = chunk.in_uint32_le();

        if (!this->client_use_long_format_name ||
            !this->server_use_long_format_name) {
            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_server_format_list_pdu: "
                    "Short Format Name%s variant of Format List PDU is used "
                    "for exchanging updated format names.",
                ((msgFlags & RDPECLIP::CB_ASCII_NAMES) ? " (ASCII 8)" : ""));

            // File List data cannot be detected!!!
            this->server_file_list_format_id = 0;
        }
        else {
            if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                LOG(LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_list_pdu: "
                        "Long Format Name variant of Format List PDU is used "
                        "for exchanging updated format names.");
            }

            for (uint32_t remaining_data_length = dataLen;
                 remaining_data_length; ) {
                const uint32_t formatId           = chunk.in_uint32_le();
                const size_t   format_name_length = UTF16StrLen(chunk.p) + 1;

                const size_t size_of_utf8_string =
                    format_name_length *
                    maximum_length_of_utf8_character_in_bytes;
                uint8_t * const utf8_string = static_cast<uint8_t *>(
                    ::alloca(size_of_utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    chunk.p, format_name_length, utf8_string,
                    size_of_utf8_string);

                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_format_list_pdu: "
                            "formatId=%d wszFormatName=\"%s\"",
                        formatId, utf8_string);
                }

                remaining_data_length -=
                          4                      /* formatId(4) */
                        + format_name_length * 2 /* wszFormatName(variable) */
                    ;

                if ((sizeof(FILE_LIST_FORMAT_NAME) == length_of_utf8_string) &&
                    !memcmp(FILE_LIST_FORMAT_NAME, utf8_string, length_of_utf8_string)) {
                    this->server_file_list_format_id = formatId;
                }

                chunk.in_skip_bytes(format_name_length * 2);
            }
        }

        return true;
    }

    virtual void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
            LOG(LOG_INFO,
                "ClipboardVirtualChannel::process_server_message: "
                    "total_length=%u flags=0x%08X chunk_data_length=%u",
                total_length, flags, chunk_data_length);
        }

        if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR_DUMP) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        ReadOnlyStream chunk(chunk_data, chunk_data_length);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!chunk.in_check_rem(2 /* msgType(2) */)) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_server_message: "
                        "Truncated msgType, need=2 remains=%u",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            this->server_message_type = chunk.in_uint16_le();
        }

        bool send_message_to_client = true;

        switch (this->server_message_type)
        {
            case RDPECLIP::CB_CLIP_CAPS:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Clipboard Capabilities PDU");
                }

                send_message_to_client =
                    this->process_server_clipboard_capabilities_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_LIST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format List PDU");
                }

                send_message_to_client =
                    this->process_server_format_list_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "Format Data Request PDU");
                }

                send_message_to_client =
                    this->process_server_format_data_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_server_message: "
                            "File Contents Request PDU");
                }

                send_message_to_client =
                    this->process_server_file_contents_request_pdu(
                        total_length, flags, chunk);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
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

            case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                    LOG(LOG_INFO,
                        "ClipboardVirtualChannel::process_client_message: "
                            "File Contents Response PDU");
                }

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }
        }   // switch (this->server_message_type)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data,
                chunk_data_length);
        }   // switch (this->server_message_type)
    }   // process_server_message
};  // class ClipboardVirtualChannel

#endif  // #ifndef REDEMPTION_MOD_RDP_CHANNELS_CLIPRDRCHANNEL_HPP

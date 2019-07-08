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
#include "core/front_api.hpp"
#include "core/RDP/clipboard.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "system/ssl_sha256.hpp"
#include "utils/arcsight.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/stream.hpp"
#include "utils/difftimeval.hpp"
#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"
#include "mod/rdp/channels/channel_file.hpp"
#include "mod/icap_files_service.hpp"
#include "core/session_reactor.hpp"
#include "mod/rdp/channels/clipboard_virtual_channels_params.hpp"
#include "core/stream_throw_helpers.hpp"

#include <memory>
#include <unordered_map>


class ClipboardVirtualChannel final : public BaseVirtualChannel
{
private:
    ClipboardData clip_data;
    using format_name_inventory_type = std::unordered_map<uint32_t, std::string>;

    std::vector<RDPECLIP::FileDescriptor> file_descr_list;

    format_name_inventory_type format_name_inventory;


    const ClipboardVirtualChannelParams params;

    FrontAPI& front;

    SessionProbeLauncher* clipboard_monitor_ready_notifier = nullptr;
    SessionProbeLauncher* clipboard_initialize_notifier    = nullptr;
    SessionProbeLauncher* format_list_notifier             = nullptr;
    SessionProbeLauncher* format_list_response_notifier    = nullptr;
    SessionProbeLauncher* format_data_request_notifier     = nullptr;

    const bool proxy_managed;   // Has not client.

    SessionReactor& session_reactor;

    ChannelFile channel_file;

    static constexpr uint32_t last_lindex_unknown = ~uint32_t{};
    uint32_t last_lindex = last_lindex_unknown;
    uint64_t last_lindex_total_send = 0;
    uint64_t last_lindex_packet_remaining = 0;


public:

    ClipboardVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        FrontAPI& front,
        SessionReactor& session_reactor,
        const BaseVirtualChannel::Params & base_params,
        const ClipboardVirtualChannelParams & params,
        ICAPService * icap_service)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         base_params)
    , params(params)
    , front(front)
    , proxy_managed(to_client_sender_ == nullptr)
    , session_reactor(session_reactor)
    , channel_file(icap_service, params.validator_params)
    {}


protected:
    const char* get_reporting_reason_exchanged_data_limit_reached() const
        override
    {
        return "CLIPBOARD_LIMIT";
    }

public:
    void empty_client_clipboard() {
        LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::empty_client_clipboard");

        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_, 0);

        StaticOutStream<256> out_s;

        clipboard_header.emit(out_s);

        const size_t totalLength = out_s.get_offset();

        this->send_message_to_server(
            totalLength,
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_s.get_data(),
            totalLength);
    }

    bool use_long_format_names() const {
        return (this->clip_data.client_data.use_long_format_names &&
            this->clip_data.server_data.use_long_format_names);
    }


public:
    bool process_client_format_list_pdu(uint32_t total_length, uint32_t flags,
        InStream& chunk, const RDPECLIP::CliprdrHeader & in_header)
    {
        (void)total_length;

        this->file_descr_list.clear();
        this->last_lindex = last_lindex_unknown;
        this->last_lindex_total_send = 0;
        this->last_lindex_packet_remaining = 0;

        ClientFormatListReceive receiver(this->clip_data.client_data.use_long_format_names,
                                         this->clip_data.server_data.use_long_format_names,
                                         in_header,
                                         chunk,
                                         this->format_name_inventory,
                                         verbose);

        if (!this->params.clipboard_down_authorized
        &&  !this->params.clipboard_up_authorized
        &&  !this->format_list_response_notifier) {

            LOG(LOG_WARNING,"ClipboardVirtualChannel::process_client_format_list_pdu: Clipboard is fully disabled.");

            ClientFormatListSendBack sender(this);
            return false;
        }

        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            LOG(LOG_ERR,
                "ClipboardVirtualChannel::process_client_format_list_pdu: "
                    "!!!CHUNKED!!! Format List PDU is not yet supported!");
            ClientFormatListSendBack sender(this);
            return false;
        }

        this->format_name_inventory.clear();
        if (receiver.client_file_list_format_id) {
            this->clip_data.client_data.file_list_format_id = receiver.client_file_list_format_id;
        }
        return true;
    }

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length) override
    {
        LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_client_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%u",
            total_length, flags, chunk_data_length);

        if (bool(this->verbose & RDPVerbose::cliprdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);
        RDPECLIP::CliprdrHeader header;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            /* msgType(2) + msgFlags(2) + dataLen(4) */
            ::check_throw(chunk, 8, "ClipboardVirtualChannel::process_client_message", ERR_RDP_DATA_TRUNCATED);
            header.recv(chunk);
            this->clip_data.client_data.message_type = header.msgType();
        }

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            log_client_message_type(
                "process_client_message",
                this->clip_data.client_data.message_type, flags);
        }

        bool send_message_to_server = true;

        switch (this->clip_data.client_data.message_type)
        {
            case RDPECLIP::CB_FORMAT_LIST:
                send_message_to_server =
                    this->process_client_format_list_pdu(
                        total_length, flags, chunk, header);
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
                FormatDataRequestReceive receiver(this->clip_data, this->clip_data.client_data, this->verbose, chunk);
                if (!this->params.clipboard_down_authorized) {

                    ClientFormatDataRequestSendBack sender(this->verbose, this);
                }
                send_message_to_server = this->params.clipboard_down_authorized;
            }
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                    ClientFormatDataResponseReceive receiver(
                        this->clip_data.client_data,
                        this->clip_data,
                        chunk,
                        header,
                        this->params.dont_log_data_into_syslog,
                        flags,
                        this->verbose);

                    if (!this->clip_data.client_data.file_list_format_id
                        || !(this->clip_data.requestedFormatId  == this->clip_data.client_data.file_list_format_id)) {

                        const bool is_from_remote_session = false;
                        this->log_siem_info(flags, header, this->clip_data.requestedFormatId, receiver.data_to_dump, is_from_remote_session);

                        this->file_descr_list.insert(
                            this->file_descr_list.end(),
                            receiver.files_descriptors.begin(),
                            receiver.files_descriptors.end());
                    }

                    send_message_to_server = true;
                }

                if (send_message_to_server &&
                    bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    this->update_exchanged_data(total_length);
                }
            break;

            case RDPECLIP::CB_CLIP_CAPS:
            {
                ClipboardCapabilitiesReceive receiver(this->clip_data.client_data, chunk, this->verbose);
                send_message_to_server = true;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST: {
                FilecontentsRequestReceive receiver(this->clip_data.client_data, chunk, this->verbose, header.dataLen());
                if (!this->params.clipboard_file_authorized) {
                    ClientFilecontentsRequestSendBack sender(this->verbose, receiver.dwFlags, receiver.streamID, this);
                } else if (receiver.dwFlags == RDPECLIP::FILECONTENTS_RANGE) {
                    const RDPECLIP::FileDescriptor & desc = this->file_descr_list[receiver.lindex];

                    this->last_lindex_packet_remaining = receiver.requested;

                    if (this->last_lindex != receiver.lindex) {
                        this->last_lindex = receiver.lindex;
                        this->channel_file.new_file(desc.file_name.c_str(), desc.file_size(), ChannelFile::FILE_FROM_SERVER, this->session_reactor.get_current_time());
                    }
                }
                send_message_to_server = this->params.clipboard_file_authorized;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }

                FileContentsResponseReceive receive(this->clip_data.client_data, header, flags, chunk);

                if (this->clip_data.server_data.last_dwFlags == RDPECLIP::FILECONTENTS_RANGE) {
                    auto data = chunk.remaining_bytes();
                    auto data_len = std::min<size_t>(data.size(), this->last_lindex_packet_remaining);
                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        auto file_size = this->file_descr_list[this->last_lindex].file_size();
                        data_len = std::min<size_t>(data_len, file_size - this->last_lindex_total_send);
                    }
                    data = data.first(data_len);
                    this->channel_file.set_data(data);
                    this->last_lindex_packet_remaining -= data_len;
                    this->last_lindex_total_send += data_len;

                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        auto file_size = this->file_descr_list[this->last_lindex].file_size();
                        if (this->last_lindex_total_send == file_size) {
                            this->channel_file.set_end_of_file();
                        }
                    }
                    send_message_to_server = !this->channel_file.is_enable_interuption();
                }
                if (receive.must_log_file_info_type) {
                    const bool from_remote_session = false;
                    this->log_file_info(receive.file_info, from_remote_session);
                }
            }
            break;

            case RDPECLIP::CB_LOCK_CLIPDATA: {
                LockClipDataReceive receiver(this->clip_data.server_data, this->clip_data.client_data, chunk, this->verbose, header);
            }
            break;

            case RDPECLIP::CB_UNLOCK_CLIPDATA: {
                UnlockClipDataReceive receive(this->clip_data.server_data, this->clip_data.client_data, chunk, this->verbose, header);
//                     this->clip_data.server_data.file_stream_data_inventory.erase(receive.clipDataId);
            }
            break;
        }   // switch (this->client_message_type)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data,
                chunk_data_length);
        }
    }   // process_client_message

    bool process_server_format_data_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk, const RDPECLIP::CliprdrHeader & /*in_header*/)
    {
        (void)total_length;
        (void)flags;
        FormatDataRequestReceive receiver(this->clip_data, this->clip_data.server_data, this->verbose, chunk);

        if (this->format_data_request_notifier &&
            (this->clip_data.requestedFormatId  == RDPECLIP::CF_TEXT)) {
            if (!this->format_data_request_notifier->on_server_format_data_request()) {
                this->format_data_request_notifier = nullptr;
            }

            return false;
        }

        if (!this->params.clipboard_up_authorized) {
            LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                    "Client to server Clipboard operation is not allowed.");

            ServerFormatDataRequestSendBack sender(this->verbose, this);

            return false;
        }

        LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                "requestedFormatId=%s(%u)",
            RDPECLIP::get_FormatId_name(this->clip_data.requestedFormatId ),
            this->clip_data.requestedFormatId );

        return true;
    }   // process_server_format_data_request_pdu



    bool process_server_format_data_response_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk, const RDPECLIP::CliprdrHeader & in_header)
    {
        (void)total_length;

        ServerFormatDataResponseReceive receiver(
            this->clip_data.requestedFormatId ,
            chunk,
            in_header,
            this->params.dont_log_data_into_syslog,
            this->clip_data.server_data.file_list_format_id,
            flags,
            this->clip_data.server_data.file_descriptor_stream,
            this->verbose
        );

        this->file_descr_list.insert(
            this->file_descr_list.end(),
            receiver.files_descriptors.begin(),
            receiver.files_descriptors.end());

        const bool is_from_remote_session = true;
        this->log_siem_info(flags, in_header, this->clip_data.requestedFormatId, receiver.data_to_dump, is_from_remote_session);

        return true;
    }   // process_server_format_data_response_pdu

    bool process_server_format_list_pdu(uint32_t total_length, uint32_t flags,
        InStream& chunk, const RDPECLIP::CliprdrHeader & in_header)
    {
        (void)total_length;
        (void)flags;

        this->file_descr_list.clear();
        this->last_lindex = last_lindex_unknown;
        this->last_lindex_total_send = 0;
        this->last_lindex_packet_remaining = 0;

        if (!this->params.clipboard_down_authorized
        &&  !this->params.clipboard_up_authorized) {
            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_server_format_list_pdu: "
                    "Clipboard is fully disabled.");

            ServerFormatListSendBack sender(this);

            return false;
        }

        this->format_name_inventory.clear();

        ServerFormatListReceive receiver(this->clip_data.client_data.use_long_format_names,
                                    this->clip_data.server_data.use_long_format_names,
                                    in_header,
                                    chunk,
                                    this->format_name_inventory,
                                    verbose);

        if (receiver.server_file_list_format_id) {
            this->clip_data.server_data.file_list_format_id = receiver.server_file_list_format_id;
        }

        return true;
    }   // process_server_format_list_pdu

    void process_server_message(uint32_t total_length,
        uint32_t flags, const uint8_t* chunk_data,
        uint32_t chunk_data_length,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%u",
            total_length, flags, chunk_data_length);

        if (bool(this->verbose & RDPVerbose::cliprdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = false;
            ::msgdump_c(send, from_or_to_client, total_length, flags,
                chunk_data, chunk_data_length);
        }

        InStream chunk(chunk_data, chunk_data_length);
        RDPECLIP::CliprdrHeader header;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            /* msgType(2) + msgFlags(2) + dataLen(4) */
            ::check_throw(chunk, 8, "ClipboardVirtualChannel::process_server_message", ERR_RDP_DATA_TRUNCATED);
            header.recv(chunk);
            this->clip_data.server_data.message_type = header.msgType();
        }

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            log_client_message_type(
                "process_server_message",
                this->clip_data.server_data.message_type, flags);
        }

        bool send_message_to_client = true;

        switch (this->clip_data.server_data.message_type)
        {
            case RDPECLIP::CB_MONITOR_READY: {
                if (this->proxy_managed) {
                    this->clip_data.client_data.use_long_format_names = true;
                    ServerMonitorReadySendBack sender(this->verbose, this->use_long_format_names(), this);
                }

                if (this->clipboard_monitor_ready_notifier) {
                    if (!this->clipboard_monitor_ready_notifier->on_clipboard_monitor_ready()) {
                        this->clipboard_monitor_ready_notifier = nullptr;
                    }
                }

                send_message_to_client = !this->proxy_managed;
            }
            break;

            case RDPECLIP::CB_FORMAT_LIST:
                send_message_to_client =
                    this->process_server_format_list_pdu(
                        total_length, flags, chunk, header);

                if (this->format_list_notifier) {
                    if (!this->format_list_notifier->on_server_format_list()) {
                        this->format_list_notifier = nullptr;
                    }
                }
            break;

            case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
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

            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                send_message_to_client =
                    this->process_server_format_data_request_pdu(
                        total_length, flags, chunk, header);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                send_message_to_client =
                    this->process_server_format_data_response_pdu(
                        total_length, flags, chunk, header);

                if (send_message_to_client &&
                    (flags & CHANNELS::CHANNEL_FLAG_FIRST)) {
                    this->update_exchanged_data(total_length);
                }
            break;

            case RDPECLIP::CB_CLIP_CAPS:
            {
                ClipboardCapabilitiesReceive receiver(this->clip_data.server_data, chunk, this->verbose);
                send_message_to_client = !this->proxy_managed;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST: {
                FilecontentsRequestReceive receiver(this->clip_data.server_data, chunk, this->verbose, header.dataLen());

                if (receiver.dwFlags == RDPECLIP::FILECONTENTS_RANGE) {
                    const RDPECLIP::FileDescriptor & desc = this->file_descr_list[receiver.lindex];

                    this->last_lindex_packet_remaining = receiver.requested;

                    if (this->last_lindex != receiver.lindex) {
                        this->last_lindex = receiver.lindex;
                        this->channel_file.new_file(desc.file_name.c_str(), desc.file_size() ,ChannelFile::FILE_FROM_CLIENT, this->session_reactor.get_current_time());
                    }
                }

                if (!this->params.clipboard_file_authorized) {
                    ServerFilecontentsRequestSendBack sender(this->verbose, receiver.dwFlags, receiver.streamID, this);
                }
                send_message_to_client = this->params.clipboard_file_authorized;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    this->update_exchanged_data(total_length);
                }

                FileContentsResponseReceive receive(this->clip_data.server_data, header, flags, chunk);

                if (this->clip_data.client_data.last_dwFlags == RDPECLIP::FILECONTENTS_RANGE) {
                    auto data = chunk.remaining_bytes();
                    auto data_len = std::min<size_t>(data.size(), this->last_lindex_packet_remaining);
                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        auto file_size = this->file_descr_list[this->last_lindex].file_size();
                        data_len = std::min<size_t>(data_len, file_size - this->last_lindex_total_send);
                    }
                    data = data.first(data_len);
                    this->channel_file.set_data(data);
                    this->last_lindex_packet_remaining -= data_len;
                    this->last_lindex_total_send += data_len;

                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        auto file_size = this->file_descr_list[this->last_lindex].file_size();
                        if (this->last_lindex_total_send == file_size) {
                            this->channel_file.set_end_of_file();
                        }
                    }

                    send_message_to_client = !this->channel_file.is_enable_interuption();
                }

                if (receive.must_log_file_info_type) {
                    const bool from_remote_session = true;
                    this->log_file_info(receive.file_info, from_remote_session);
                }
            }
            break;

            case RDPECLIP::CB_LOCK_CLIPDATA: {
                LockClipDataReceive receiver(this->clip_data.client_data, this->clip_data.server_data, chunk, this->verbose, header);
            }
            break;

            case RDPECLIP::CB_UNLOCK_CLIPDATA: {
                UnlockClipDataReceive receive(this->clip_data.client_data, this->clip_data.server_data, chunk, this->verbose, header);
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

    void log_client_message_type(char const* funcname, uint16_t message_type, uint32_t flags)
    {
        const auto first_last = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
        LOG(LOG_INFO, "ClipboardVirtualChannel::%s: %s (%u)%s)",
            funcname, RDPECLIP::get_msgType_name(message_type), message_type,
            ((flags & first_last) == first_last) ? " FIRST|LAST"
            : (flags & CHANNELS::CHANNEL_FLAG_FIRST) ? "FIRST"
            : (flags & CHANNELS::CHANNEL_FLAG_LAST) ? "LAST"
            : "");
    }

    void DLP_antivirus_check_channels_files()
    {
        if (!this->channel_file.receive_response()) {
            return;
        }

        if (!this->channel_file.is_valid() && this->channel_file.is_enable_validation()) {
            auto const info = key_qvalue_pairs({
                {"type", "FILE_SCAN_RESULT" },
                {"file_name", this->channel_file.get_file_name()},
                {"size", std::to_string(this->channel_file.get_file_size())},
                {"result", this->channel_file.get_result_content()}
            });

            ArcsightLogInfo arc_info;
            arc_info.name = "FILE_SCAN_RESULT";
            arc_info.signatureID = ArcsightLogInfo::ID::FILE_SCAN_RESULT;
            arc_info.ApplicationProtocol = "rdp";
            arc_info.fileName = this->channel_file.get_file_name();
            arc_info.fileSize = this->channel_file.get_file_size();
            arc_info.direction_flag = this->channel_file.get_direction() == ChannelFile::FILE_FROM_SERVER ? ArcsightLogInfo::Direction::SERVER_SRC : ArcsightLogInfo::Direction::SERVER_DST;
            arc_info.message = this->channel_file.get_result_content();

            this->report_message.log6(info, arc_info, tvtime());
        }

        if (this->channel_file.is_enable_interuption() && this->channel_file.is_save_files()) {
            if (this->channel_file.is_valid()) {

                switch (this->channel_file.get_direction()) {

                    case ChannelFile::FILE_FROM_SERVER:
                        this->send_filtered_file_content_message_to_client();
                        break;

                    case ChannelFile::FILE_FROM_CLIENT:
                        this->send_filtered_file_content_message_to_server();
                        break;

                    default: break;
                }

            } else {
                RDPECLIP::CliprdrHeader fileRangeHeader(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 4);
                StaticOutStream<16> out_stream;
                RDPECLIP::FileContentsResponseRange fileRange{safe_int(this->channel_file.get_streamID())};

                fileRangeHeader.emit(out_stream);
                fileRange.emit(out_stream);

                switch (this->channel_file.get_direction()) {

                    case ChannelFile::FILE_FROM_SERVER:
                        this->send_message_to_client(out_stream.get_offset(),
                                                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                                                    out_stream.get_data(),
                                                    out_stream.get_offset());
                        break;

                    case ChannelFile::FILE_FROM_CLIENT:
                        this->send_message_to_server(out_stream.get_offset(),
                                                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
                                                    out_stream.get_data(),
                                                    out_stream.get_offset());
                        break;

                    default: break;
                }
            }
        }
    }
//
    void send_filtered_file_content_message_to_server() {

        RDPECLIP::CliprdrHeader fileRangeHeader(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->channel_file.get_file_size()+4);
        StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;
        RDPECLIP::FileContentsResponseRange fileRange{safe_int(this->channel_file.get_streamID())};

        uint32_t first_part_data_size(this->channel_file.get_file_size());
        int total_length(first_part_data_size + 12);
        if (first_part_data_size > CHANNELS::CHANNEL_CHUNK_LENGTH - 12) {
            first_part_data_size = CHANNELS::CHANNEL_CHUNK_LENGTH - 12;
        }
        fileRangeHeader.emit(out_stream_first_part);
        fileRange.emit(out_stream_first_part);

        std::unique_ptr<uint8_t[]> data_file = std::make_unique<uint8_t[]>(this->channel_file.get_file_size());

        LOG(LOG_INFO, "send_filtered_file_content_message_to_server");

        this->channel_file.read_data(data_file.get(), this->channel_file.get_file_size());

        const_bytes_view data {data_file.get(), this->channel_file.get_file_size()};

        if (this->channel_file.get_file_size() > first_part_data_size ) {

            int real_total = data.size() - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
            out_stream_first_part.out_copy_bytes(data.data(), first_part_data_size);

            data_sent += first_part_data_size;
            InStream chunk_first(out_stream_first_part.get_bytes());

            this->send_message_to_server(total_length, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());

            for (int i = 0; i < cmpt_PDU_part; i++) {

                // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data.data() + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_bytes());

                this->send_message_to_server(total_length, CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());
            }

            // Last part
            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
            out_stream_last_part.out_copy_bytes(data.data() + data_sent, remains_PDU);

            InStream chunk_last(out_stream_last_part.get_bytes());

            this->send_message_to_server(total_length, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data.data(), data.size());
            InStream chunk(out_stream_first_part.get_bytes());

            this->send_message_to_server(total_length, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, out_stream_first_part.get_data(),
                out_stream_first_part.get_offset());
        }
    }

    void send_filtered_file_content_message_to_client() {

        RDPECLIP::CliprdrHeader fileRangeHeader(RDPECLIP::CB_FILECONTENTS_RESPONSE, RDPECLIP::CB_RESPONSE_OK, this->channel_file.get_file_size()+4);
        StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;
        RDPECLIP::FileContentsResponseRange fileRange{safe_int(this->channel_file.get_streamID())};

        uint32_t first_part_data_size(this->channel_file.get_file_size());
        int total_length(first_part_data_size + 12);
        if (first_part_data_size > CHANNELS::CHANNEL_CHUNK_LENGTH - 12) {
            first_part_data_size = CHANNELS::CHANNEL_CHUNK_LENGTH - 12;
        }
        fileRangeHeader.emit(out_stream_first_part);
        fileRange.emit(out_stream_first_part);

        std::unique_ptr<uint8_t[]> data_file = std::make_unique<uint8_t[]>(this->channel_file.get_file_size());

        LOG(LOG_INFO, "send_filtered_file_content_message_to_client");

        this->channel_file.read_data(data_file.get(), this->channel_file.get_file_size());

        const_bytes_view data {data_file.get(), this->channel_file.get_file_size()};

        if (data.size() > first_part_data_size ) {

            int real_total = data.size() - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
            out_stream_first_part.out_copy_bytes(data.data(), first_part_data_size);

            data_sent += first_part_data_size;
            InStream chunk_first(out_stream_first_part.get_bytes());

            this->send_message_to_client(total_length, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());

            for (int i = 0; i < cmpt_PDU_part; i++) {

                // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data.data() + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_bytes());

                this->send_message_to_client(total_length, CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());
            }

            // Last part
            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
            out_stream_last_part.out_copy_bytes(data.data() + data_sent, remains_PDU);

            InStream chunk_last(out_stream_last_part.get_bytes());

            this->send_message_to_client(total_length, CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, chunk_first.get_data(),
                chunk_first.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data.data(), data.size());
            InStream chunk(out_stream_first_part.get_bytes());

            this->send_message_to_client(total_length, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL, out_stream_first_part.get_data(),
                out_stream_first_part.get_offset());
        }
    }

private:
    void log_file_info(ClipboardSideData::file_info_type & file_info, bool from_remote_session)
    {
        const char* type = (
                  from_remote_session
                ? "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                : "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION"
            );

        uint8_t digest[SslSha256::DIGEST_LENGTH] = { 0 };

        file_info.sha256.final(digest);

        char digest_s[128];
        snprintf(digest_s, sizeof(digest_s),
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
            digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
            digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
            digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);

        auto const file_size_str = std::to_string(file_info.size);

        auto const info = key_qvalue_pairs({
                { "type", type },
                { "file_name", file_info.file_name },
                { "size", file_size_str },
                { "sha256", digest_s }
            });

        ArcsightLogInfo arc_info;
        arc_info.name = type;
        arc_info.ApplicationProtocol = "rdp";
        arc_info.fileName = file_info.file_name;
        arc_info.fileSize = file_info.size;
        arc_info.direction_flag = from_remote_session ? ArcsightLogInfo::Direction::SERVER_SRC : ArcsightLogInfo::Direction::SERVER_DST;

        this->report_message.log6(info, arc_info, tvtime());

        if (!this->params.dont_log_data_into_syslog) {
            LOG(LOG_INFO, "%s", info);
        }

        if (!this->params.dont_log_data_into_wrm) {
            std::string message = str_concat(
                type, '=', file_info.file_name, '\x01', file_size_str, '\x01', digest_s);
            this->front.session_update(message);
        }
    }

    void log_siem_info(uint32_t flags, const RDPECLIP::CliprdrHeader & in_header, const uint32_t requestedFormatId, const std::string & data_to_dump, const bool is_from_remote_session) {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

            if (in_header.msgFlags() & RDPECLIP::CB_RESPONSE_OK) {

                std::string type;
                if (is_from_remote_session) {
                    type = (data_to_dump.empty() ?
                        "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION" :
                        "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX");
                } else {
                    type = (data_to_dump.empty() ?
                        "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION" :
                        "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX");
                }

                std::string format_name = this->format_name_inventory[requestedFormatId];
                if (format_name.empty()) {
                    format_name = RDPECLIP::get_FormatId_name(requestedFormatId );
                }

                bool const log_current_activity = (
                        (!this->params.log_only_relevant_clipboard_activities)
                         ||    (strcasecmp("Preferred DropEffect", format_name.c_str()) != 0
                             && strcasecmp("FileGroupDescriptorW", format_name.c_str()) != 0)
                    );

                str_append(format_name, '(', std::to_string(requestedFormatId), ')');

                auto const size_str = std::to_string(in_header.dataLen());

                std::string info;
                ::key_qvalue_pairs(
                        info,
                        {
                            { "type", type },
                            { "format", format_name },
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

                ArcsightLogInfo arc_info;
                arc_info.name = data_to_dump.empty() ? "CB_COPYING_PASTING_DATA" : "CB_COPYING_PASTING_DATA_EX";
                arc_info.signatureID = data_to_dump.empty() ? ArcsightLogInfo::ID::CB_COPYING_PASTING_DATA : ArcsightLogInfo::ID::CB_COPYING_PASTING_DATA_EX;
                arc_info.ApplicationProtocol = "rdp";
                arc_info.message = info;
                arc_info.direction_flag = is_from_remote_session ? ArcsightLogInfo::Direction::SERVER_SRC : ArcsightLogInfo::Direction::SERVER_DST;

                if (log_current_activity) {
                    this->report_message.log6(info, arc_info, tvtime());
                }

                if (!this->params.dont_log_data_into_syslog) {
                    LOG(LOG_INFO, "%s", info);
                }

                if (!this->params.dont_log_data_into_wrm) {
                    std::string message = str_concat(type, '=', format_name, '\x01', size_str);
                    if (!data_to_dump.empty()) {
                        str_append(message, '\x01', data_to_dump);
                    }

                    this->front.session_update(message);
                }
            }
        }
    }

};  // class ClipboardVirtualChannel

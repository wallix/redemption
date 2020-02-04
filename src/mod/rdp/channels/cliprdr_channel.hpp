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

#include "core/error.hpp"
#include "core/session_reactor.hpp"
#include "core/log_id.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/clipboard_virtual_channels_params.hpp"
#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "utils/difftimeval.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/cast.hpp"

#include <vector>
#include <string>


class ClipboardVirtualChannel final : public BaseVirtualChannel
{
    using StreamId = ClipboardSideData::StreamId;
    using FileGroupId = ClipboardSideData::FileGroupId;

    ClipboardData clip_data;

    std::vector<CliprdFileInfo> file_descr_list;

    Cliprdr::FormatNameInventory format_name_inventory;


    const ClipboardVirtualChannelParams params;

    SessionProbeLauncher* clipboard_monitor_ready_notifier = nullptr;
    SessionProbeLauncher* clipboard_initialize_notifier    = nullptr;
    SessionProbeLauncher* format_list_notifier             = nullptr;
    SessionProbeLauncher* format_list_response_notifier    = nullptr;
    SessionProbeLauncher* format_data_request_notifier     = nullptr;

    const bool proxy_managed;   // Has not client.

    SessionReactor& session_reactor;

    FileValidatorService * file_validator;

    FdxCapture * fdx_capture;
    bool always_file_storage;

    enum class Direction : bool
    {
        FileFromServer,
        FileFromClient,
    };

public:
    struct FileRecord
    {
        FdxCapture * fdx_capture;
        bool always_file_storage;
    };

    ClipboardVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        SessionReactor& session_reactor,
        const BaseVirtualChannel::Params & base_params,
        const ClipboardVirtualChannelParams & params,
        FileValidatorService * file_validator_service,
        FileRecord filre_record)
    : BaseVirtualChannel(to_client_sender_,
                         to_server_sender_,
                         base_params)
    , params([&]{
        auto p = params;
        if (!file_validator_service) {
            p.validator_params.up_target_name.clear();
            p.validator_params.down_target_name.clear();
        }
        return p;
    }())
    , proxy_managed(to_client_sender_ == nullptr)
    , session_reactor(session_reactor)
    , file_validator(file_validator_service)
    , fdx_capture(filre_record.fdx_capture)
    , always_file_storage(filre_record.always_file_storage)
    {}

    ~ClipboardVirtualChannel()
    {
        try {
            for (ClipboardSideData* side_data : {
                &this->clip_data.client_data,
                &this->clip_data.server_data
            }) {
                for (auto& file : side_data->get_file_contents_list()) {
                    auto& file_data = file.file_data;
                    if (file_data.tfl_file) {
                        if (file_data.tfl_file->trans.is_open()) {
                            this->_close_tfl(file_data);
                        }
                    }
                }
            }
        }
        catch (Error const& err) {
            LOG(LOG_ERR, "ClipboardVirtualChannel: error on close tfls: %s", err.errmsg());
        }
    }

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
            out_s.get_bytes());
    }

    [[nodiscard]] bool use_long_format_names() const {
        return (this->clip_data.client_data.use_long_format_names &&
            this->clip_data.server_data.use_long_format_names);
    }

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override
    {
        this->log_process_message(total_length, flags, chunk_data, Direction::FileFromClient);

        InStream chunk(chunk_data);
        RDPECLIP::CliprdrHeader header;
        bool send_message_to_server = true;

        switch (this->process_header_message(
            this->clip_data.client_data, flags, chunk, header, Direction::FileFromClient
        ))
        {
            case RDPECLIP::CB_FORMAT_LIST:
                send_message_to_server = this->process_format_list_pdu(
                    flags, chunk, header,
                    this->to_client_sender_ptr(),
                    this->clip_data.client_data,
                    this->params.clipboard_down_authorized || this->params.clipboard_up_authorized || this->format_list_response_notifier);
            break;

            case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
                FormatDataRequestReceive receiver(this->clip_data, this->verbose, chunk);
                if (!this->params.clipboard_down_authorized) {
                    LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                        "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                            "Server to client Clipboard operation is not allowed.");

                    FormatDataRequestSendBack sender(this->to_client_sender_ptr());
                }
                send_message_to_server = this->params.clipboard_down_authorized;
            }
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                const bool is_from_remote_session = false;
                send_message_to_server = this->process_format_data_response_pdu(
                    flags, chunk, header, is_from_remote_session);
            }
            break;

            case RDPECLIP::CB_CLIP_CAPS:
            {
                ClipboardCapabilitiesReceive(this->clip_data.client_data, chunk, this->verbose);
                send_message_to_server = true;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                send_message_to_server = this->process_filecontents_request_pdu(
                    flags, chunk, this->to_client_sender_ptr(),
                    this->clip_data.client_data, Direction::FileFromServer);
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
                const bool is_from_remote_session = false;
                send_message_to_server = this->process_filecontents_response_pdu(
                    flags, chunk, header, this->clip_data.server_data, is_from_remote_session);
            }
            break;

            case RDPECLIP::CB_LOCK_CLIPDATA:
                this->process_lock_pdu(chunk, this->clip_data.client_data);
            break;

            case RDPECLIP::CB_UNLOCK_CLIPDATA:
                this->process_unlock_pdu(chunk, this->clip_data.client_data);
            break;
        }   // switch (this->client_message_type)

        if (send_message_to_server) {
            this->send_message_to_server(total_length, flags, chunk_data);
        }
    }   // process_client_message

    bool process_server_format_data_request_pdu(uint32_t total_length,
        uint32_t flags, InStream& chunk, const RDPECLIP::CliprdrHeader & /*in_header*/)
    {
        (void)total_length;
        (void)flags;
        FormatDataRequestReceive receiver(this->clip_data, this->verbose, chunk);

        if (this->format_data_request_notifier &&
            (this->clip_data.requestedFormatId == RDPECLIP::CF_TEXT)) {
            if (!this->format_data_request_notifier->on_server_format_data_request()) {
                this->format_data_request_notifier = nullptr;
            }

            return false;
        }

        if (!this->params.clipboard_up_authorized) {
            LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                    "Client to server Clipboard operation is not allowed.");

            FormatDataRequestSendBack sender(this->to_server_sender_ptr());

            return false;
        }

        return true;
    }   // process_server_format_data_request_pdu

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override
    {
        (void)out_asynchronous_task;

        this->log_process_message(total_length, flags, chunk_data, Direction::FileFromServer);

        InStream chunk(chunk_data);
        RDPECLIP::CliprdrHeader header;
        bool send_message_to_client = true;

        switch (this->process_header_message(
            this->clip_data.server_data, flags, chunk, header, Direction::FileFromServer
        ))
        {
            case RDPECLIP::CB_MONITOR_READY: {
                if (this->proxy_managed) {
                    this->clip_data.server_data.use_long_format_names = true;
                    ServerMonitorReadySendBack sender(this->verbose, this->use_long_format_names(), this->to_server_sender_ptr());
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
                send_message_to_client = this->process_format_list_pdu(
                    flags, chunk, header,
                    this->to_server_sender_ptr(),
                    this->clip_data.server_data,
                    this->params.clipboard_down_authorized || this->params.clipboard_up_authorized);

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
                send_message_to_client = this->process_server_format_data_request_pdu(
                    total_length, flags, chunk, header);
            break;

            case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
                const bool is_from_remote_session = true;
                send_message_to_client = this->process_format_data_response_pdu(
                    flags, chunk, header, is_from_remote_session);
            }
            break;

            case RDPECLIP::CB_CLIP_CAPS:
            {
                ClipboardCapabilitiesReceive(this->clip_data.server_data, chunk, this->verbose);
                send_message_to_client = !this->proxy_managed;
            }
            break;

            case RDPECLIP::CB_FILECONTENTS_REQUEST:
                send_message_to_client = this->process_filecontents_request_pdu(
                    flags, chunk, this->to_server_sender_ptr(),
                    this->clip_data.server_data, Direction::FileFromClient);
            break;

            case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
                const bool from_remote_session = true;
                send_message_to_client = this->process_filecontents_response_pdu(
                    flags, chunk, header, this->clip_data.client_data, from_remote_session);
            }
            break;

            case RDPECLIP::CB_LOCK_CLIPDATA:
                this->process_lock_pdu(chunk, this->clip_data.server_data);
            break;

            case RDPECLIP::CB_UNLOCK_CLIPDATA:
                this->process_unlock_pdu(chunk, this->clip_data.server_data);
            break;
        }   // switch (this->server_message_type)

        if (send_message_to_client) {
            this->send_message_to_client(total_length, flags, chunk_data);
        }   // switch (this->server_message_type)
    }   // process_server_message

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->clipboard_monitor_ready_notifier = launcher;
        this->clipboard_initialize_notifier    = launcher;
        this->format_list_notifier             = launcher;
        this->format_list_response_notifier    = launcher;
        this->format_data_request_notifier     = launcher;
    }

    void DLP_antivirus_check_channels_files()
    {
        if (!this->file_validator) {
            return ;
        }

        auto receive_data = [this]{
            for (;;) {
                switch (this->file_validator->receive_response()) {
                    case FileValidatorService::ResponseType::WaitingData:
                        return false;
                    case FileValidatorService::ResponseType::HasContent:
                        return true;
                    case FileValidatorService::ResponseType::Error:
                        ;
                }
            }
        };

        using ValidationResult = LocalFileValidatorProtocol::ValidationResult;

        while (receive_data()){
            bool is_accepted = false;
            switch (this->file_validator->last_result_flag()) {
                case ValidationResult::Wait:
                    return;
                case ValidationResult::IsAccepted:
                    is_accepted = true;
                    [[fallthrough]];
                case ValidationResult::IsRejected:
                case ValidationResult::Error:
                    ;
            }

            auto file_validator_id = this->file_validator->last_file_id();
            auto& result_content = this->file_validator->get_content();

            bool is_client_text = this->clip_data.client_data.remove_text_id(file_validator_id);
            bool is_server_text = not is_client_text
                               && this->clip_data.server_data.remove_text_id(file_validator_id);
            bool is_text = is_client_text || is_server_text;

            if (is_text) {
                if (!is_accepted || this->params.validator_params.log_if_accepted) {
                    auto str_direction = is_client_text ? "UP"_av : "DOWN"_av;
                    char buf[24];
                    unsigned n = std::snprintf(buf, std::size(buf), "%" PRIu32,
                        underlying_cast(file_validator_id));
                    this->report_message.log6(LogId::TEXT_VERIFICATION, this->session_reactor.get_current_time(), {
                        KVLog("direction"_av, str_direction),
                        KVLog("copy_id"_av, {buf, n}),
                        KVLog("status"_av, result_content),
                    });
                }
                continue;
            }

            Direction direction = Direction::FileFromClient;
            auto* file = this->clip_data.server_data.find_file_by_file_validator_id(file_validator_id);
            if (!file) {
                file = this->clip_data.client_data.find_file_by_file_validator_id(file_validator_id);
                direction = Direction::FileFromServer;
            }
            if (!file) {
                LOG(LOG_ERR, "FileValidatorValidator::receive_response: invalid id %u", file_validator_id);
                auto& target_name = (direction == Direction::FileFromClient)
                    ? this->params.validator_params.up_target_name
                    : this->params.validator_params.down_target_name;
                this->report_message.log6(LogId::FILE_VERIFICATION_ERROR, this->session_reactor.get_current_time(), {
                    KVLog("icap_service"_av, target_name),
                    KVLog("status"_av, "Invalid file id"_av),
                });
                continue;
            }

            auto& file_data = file->file_data;
            file_data.on_failure = !is_accepted;
            file_data.file_validator_id = FileValidatorId();

            if (!is_accepted || this->params.validator_params.log_if_accepted) {
                auto str_direction = (direction == Direction::FileFromClient) ? "UP"_av : "DOWN"_av;

                this->report_message.log6(LogId::FILE_VERIFICATION, this->session_reactor.get_current_time(), {
                    KVLog("direction"_av, str_direction),
                    KVLog("file_name"_av, file_data.file_name),
                    KVLog("status"_av, result_content),
                });
            }

            if (file->is_wait_validator()) {
                if (file_data.tfl_file) {
                    if (this->always_file_storage
                     || this->file_validator->last_result_flag() != ValidationResult::IsAccepted
                    ) {
                        this->_close_tfl(file_data);
                    }
                    else {
                        this->fdx_capture->cancel_tfl(*file_data.tfl_file);
                    }
                    file_data.tfl_file.reset();
                }

                if (direction == Direction::FileFromClient) {
                    this->clip_data.server_data.remove_file(file);
                }
                else {
                    this->clip_data.client_data.remove_file(file);
                }
            }
        }
    }

private:
    void log_process_message(
        uint32_t total_length, uint32_t flags, bytes_view chunk_data, Direction direction)
    {
        LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_%s_message: "
                "total_length=%u flags=0x%08X chunk_data_length=%zu",
            (direction == Direction::FileFromClient)
                ? "client" : "server",
            total_length, flags, chunk_data.size());

        if (bool(this->verbose & RDPVerbose::cliprdr_dump)) {
            const bool send              = false;
            const bool from_or_to_client = (direction == Direction::FileFromClient);
            ::msgdump_c(send, from_or_to_client, total_length, flags, chunk_data);
        }
    }

    uint16_t process_header_message(
        ClipboardSideData& side_data,
        uint32_t flags, InStream& chunk, RDPECLIP::CliprdrHeader& header, Direction direction)
    {
        char const* funcname = (direction == Direction::FileFromClient)
            ? "ClipboardVirtualChannel::process_client_message"
            : "ClipboardVirtualChannel::process_server_message";

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            /* msgType(2) + msgFlags(2) + dataLen(4) */
            ::check_throw(chunk, 8, funcname, ERR_RDP_DATA_TRUNCATED);
            header.recv(chunk);
            side_data.current_message_type = header.msgType();
        }

        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            const auto first_last = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
            LOG(LOG_INFO, "%s: %s (%u)%s)",
                funcname,
                RDPECLIP::get_msgType_name(side_data.current_message_type),
                side_data.current_message_type,
                ((flags & first_last) == first_last) ? " FIRST|LAST"
                : (flags & CHANNELS::CHANNEL_FLAG_FIRST) ? "FIRST"
                : (flags & CHANNELS::CHANNEL_FLAG_LAST) ? "LAST"
                : "");
        }

        return side_data.current_message_type;
    }

    void process_lock_pdu(InStream& chunk, ClipboardSideData& side_data)
    {
        RDPECLIP::LockClipboardDataPDU pdu;
        pdu.recv(chunk);
        side_data.push_lock_id(pdu.clipDataId);

        if (bool(verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }

    void process_unlock_pdu(InStream& chunk, ClipboardSideData& side_data)
    {
        RDPECLIP::UnlockClipboardDataPDU pdu;
        pdu.recv(chunk);
        side_data.remove_lock_id(pdu.clipDataId);

        if (bool(verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }

    bool process_filecontents_response_pdu(
        uint32_t flags, InStream& chunk,
        RDPECLIP::CliprdrHeader const& in_header, ClipboardSideData& side_data,
        const bool from_remote_session)
    {
        auto& from_server = from_remote_session
            ? this->clip_data.server_data : this->clip_data.client_data;

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            check_throw(chunk, 4, "process_filecontents_response_pdu", ERR_RDP_DATA_TRUNCATED);
            from_server.file_contents_stream_id = safe_int(chunk.in_uint32_le());
            LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                "File Contents Response: streamId=%u", from_server.file_contents_stream_id);
        }

        auto* file = side_data.find_file_by_stream_id(from_server.file_contents_stream_id);

        if (!file) {
            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                " Unknown stream id %u", from_server.file_contents_stream_id);
            throw Error(ERR_RDP_PROTOCOL);
        }

        if (in_header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
            if (file->is_file_range()) {
                auto& file_data = file->file_data;

                if (bool(file_data.file_validator_id)) {
                    this->file_validator->send_eof(file_data.file_validator_id);
                    file->set_wait_validator();
                }
            }
        }
        else if (file->is_file_range()) {
            auto& file_data = file->file_data;
            auto data_fragment = file->receive_data(chunk.remaining_bytes());

            if (file_data.tfl_file) {
                file_data.tfl_file->trans.send(data_fragment);
            }

            if (bool(file_data.file_validator_id)) {
                this->file_validator->send_data(file_data.file_validator_id, data_fragment);
            }

            if ((flags & CHANNELS::CHANNEL_FLAG_LAST) && file_data.file_offset == file_data.file_size) {
                this->log_file_info(file_data, from_remote_session);

                if (bool(file_data.file_validator_id)) {
                    this->file_validator->send_eof(file_data.file_validator_id);
                }
                else {
                    if (file_data.tfl_file) {
                        if (this->always_file_storage || file_data.on_failure) {
                            this->fdx_capture->close_tfl(*file_data.tfl_file, file_data.file_name,
                                Mwrm3::TransferedStatus::Completed,
                                Mwrm3::Sha256Signature{file_data.sig.digest_as_av()});
                        }
                        else {
                            this->fdx_capture->cancel_tfl(*file_data.tfl_file);
                        }
                        file_data.tfl_file.reset();
                    }
                    side_data.remove_file(file);
                }
            }
        }
        else /*if (file->is_file_size())*/ {
            if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " Unsupported partial FILECONTENTS_SIZE packet");
                throw Error(ERR_RDP_UNSUPPORTED);
            }
            if (size_t(file->file_group_id) >= this->file_descr_list.size()) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " Invalid lindex");
                throw Error(ERR_RDP_UNSUPPORTED);
            }
            check_throw(chunk, 8, "process_filecontents_response_pdu", ERR_RDP_DATA_TRUNCATED);
            this->file_descr_list[safe_int(file->file_group_id)].file_size = chunk.in_uint64_le();
            side_data.remove_file(file);
        }

        return true;
    }

    bool process_filecontents_request_pdu(
        uint32_t flags, InStream& chunk, VirtualChannelDataSender* sender,
        ClipboardSideData& side_data, Direction direction)
    {
        RDPECLIP::FileContentsRequestPDU file_contents_request_pdu;
        file_contents_request_pdu.receive(chunk);

        if (bool(verbose & RDPVerbose::cliprdr)) {
            file_contents_request_pdu.log(LOG_INFO);
        }

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST))
          != (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) {
            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu: Unsupported partial packet");
            throw Error(ERR_RDP_PROTOCOL);
        }

        if (!this->params.clipboard_file_authorized) {
            FilecontentsRequestSendBack(
                file_contents_request_pdu.dwFlags(),
                file_contents_request_pdu.streamId(),
                sender);
            return false;
        }

        std::string const& target_name = (direction == Direction::FileFromServer)
            ? this->params.validator_params.down_target_name
            : this->params.validator_params.up_target_name;

        auto stream_id = StreamId(file_contents_request_pdu.streamId());
        auto lindex = FileGroupId(file_contents_request_pdu.lindex());
        if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
            auto offset = file_contents_request_pdu.position();
            if (offset) {
                auto* file = side_data.find_continuation_stream_id(stream_id);
                if (file) {
                    if (!file_contents_request_pdu.has_optional_clipDataId()) {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " Require clipDataId");
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                    if (file_contents_request_pdu.clipDataId() != file->file_data.clip_data_id) {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " Invalid clipDataId (%u != %u)", file_contents_request_pdu.clipDataId(),
                            file->file_data.clip_data_id);
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                    if (file->is_file_size()) {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " is a FILECONTENTS_SIZE, expected FILECONTENTS_RANGE");
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                    if (file->file_data.file_offset != offset) {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " Unsupported random access for a FILECONTENTS_RANGE");
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                }
                else {
                    file = side_data.find_file_by_offset(lindex, offset);
                    if (!file) {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " Unsupported random access for a FILECONTENTS_RANGE");
                        throw Error(ERR_RDP_UNSUPPORTED);
                    }
                    if (file_contents_request_pdu.has_optional_clipDataId()) {
                        file->file_data.clip_data_id = file_contents_request_pdu.clipDataId();
                    }
                    file->stream_id = stream_id;
                }

                file->update_requested(file_contents_request_pdu.cbRequested());
            }
            else {
                std::size_t ifilegroup = safe_int(lindex);
                if (ifilegroup >= this->file_descr_list.size()) {
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                        " Invalid lindex %u", lindex);
                    throw Error(ERR_RDP_PROTOCOL);
                }
                CliprdFileInfo const& desc = this->file_descr_list[ifilegroup];
                FileValidatorId file_validator_id{};
                if (!target_name.empty()) {
                    LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                        "ClipboardVirtualChannel::Validator::open_file");
                    file_validator_id = this->file_validator->open_file(desc.file_name, target_name);
                }

                side_data.push_file_content_range(
                    stream_id, lindex,
                    file_contents_request_pdu.has_optional_clipDataId(),
                    file_contents_request_pdu.clipDataId(),
                    file_validator_id,
                    this->fdx_capture
                        ? std::unique_ptr<FdxCapture::TflFile>(new FdxCapture::TflFile( /*NOLINT*/
                            this->fdx_capture->new_tfl(direction == Direction::FileFromServer
                                ? Mwrm3::Direction::ServerToClient
                                : Mwrm3::Direction::ClientToServer
                        )))
                        : std::unique_ptr<FdxCapture::TflFile>(),
                    desc.file_name, desc.file_size,
                    file_contents_request_pdu.cbRequested());
            }
        }
        else {
            side_data.push_file_content_size(stream_id, lindex);
        }

        return true;
    }

    bool process_format_data_response_pdu(uint32_t flags, InStream& chunk, const RDPECLIP::CliprdrHeader & in_header, bool is_from_remote_session)
    {
        auto& side_data = is_from_remote_session
            ? this->clip_data.server_data
            : this->clip_data.client_data;
        auto requested_format_id = this->clip_data.requestedFormatId;
        auto file_list_format_id = side_data.file_list_format_id;

        if (file_list_format_id && requested_format_id == file_list_format_id) {
            FormatDataResponseReceiveFileList receiver(
                this->file_descr_list,
                chunk,
                in_header,
                this->params.dont_log_data_into_syslog,
                side_data.file_list_format_id,
                flags,
                side_data.file_descriptor_stream,
                this->verbose,
                is_from_remote_session ? "client" : "server"
            );

            this->log_siem_info(flags, in_header, this->clip_data.requestedFormatId, std::string{}, is_from_remote_session);
        }
        else {
            auto original_chunk = chunk.clone();
            FormatDataResponseReceive receiver(requested_format_id, chunk, flags);

            this->log_siem_info(flags, in_header, this->clip_data.requestedFormatId, receiver.data_to_dump, is_from_remote_session);

            std::string const& target_name = is_from_remote_session
                ? this->params.validator_params.down_target_name
                : this->params.validator_params.up_target_name;

            if (!target_name.empty()) {
                switch (requested_format_id) {
                    case RDPECLIP::CF_TEXT:
                    case RDPECLIP::CF_UNICODETEXT: {
                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                            if (bool(side_data.clip_text_id)) {
                                this->file_validator->send_eof(side_data.clip_text_id);
                                side_data.push_clip_text_to_list();
                            }
                            side_data.clip_text_id = this->file_validator->open_text(
                                RDPECLIP::CF_TEXT == requested_format_id
                                    ? 0u : side_data.clip_text_locale_identifier,
                                target_name);
                        }
                        uint8_t utf8_buf[32*1024];
                        auto utf8_av = UTF16toUTF8_buf(
                            original_chunk.remaining_bytes(),
                            make_array_view(utf8_buf));
                        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                            if (not utf8_av.empty() && utf8_av.back() == '\0') {
                                utf8_av = utf8_av.drop_back(1);
                            }
                        }
                        this->file_validator->send_data(side_data.clip_text_id, utf8_av);
                        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                            this->file_validator->send_eof(side_data.clip_text_id);
                            side_data.push_clip_text_to_list();
                        }
                        break;
                    }
                    case RDPECLIP::CF_LOCALE:
                        side_data.clip_text_locale_identifier = original_chunk.in_uint32_le();
                        break;
                }
            }
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            this->clip_data.requestedFormatId = 0;
        }

        return true;
    }

    bool process_format_list_pdu(
        uint32_t flags, InStream& chunk, const RDPECLIP::CliprdrHeader & in_header,
        VirtualChannelDataSender* sender, ClipboardSideData& side_data, bool clip_enabled)
    {
        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            LOG(LOG_ERR, "Format List PDU is not yet supported!");
            FormatListSendBack pdu(sender);
            return false;
        }

        this->file_descr_list.clear();

        if (!clip_enabled) {
            LOG(LOG_WARNING, "Clipboard is fully disabled.");
            FormatListSendBack pdu(sender);
            return false;
        }

        this->format_name_inventory.clear();

        FormatListReceive receiver(
            this->use_long_format_names(),
            in_header,
            chunk,
            this->format_name_inventory,
            this->verbose);

        side_data.file_list_format_id = receiver.file_list_format_id;
        return true;
    }

    void log_file_info(ClipboardSideData::FileContent::FileData& file_data, bool from_remote_session)
    {
        const char* type = (
                  from_remote_session
                ? "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                : "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION"
            );

        if (!file_data.sig.has_digest()) {
            file_data.sig.broken();
        }

        static_assert(SslSha256::DIGEST_LENGTH == decltype(file_data.sig)::digest_len);
        auto& digest = file_data.sig.digest();
        char digest_s[128];
        size_t digest_s_len = snprintf(digest_s, sizeof(digest_s),
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
            digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
            digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
            digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);

        char file_size[128];
        size_t file_size_len = std::snprintf(file_size, std::size(file_size), "%lu", file_data.file_size);

        this->report_message.log6(from_remote_session
            ? LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION
            : LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION,
            this->session_reactor.get_current_time(), {
            KVLog("file_name"_av, file_data.file_name),
            KVLog("size"_av, {file_size, file_size_len}),
            KVLog("sha256"_av, {digest_s, digest_s_len}),
        });

        LOG_IF(!this->params.dont_log_data_into_syslog, LOG_INFO,
            "type=%s file_name=%s size=%s sha256=%s",
            type, file_data.file_name, file_size, digest_s);
    }

    void log_siem_info(uint32_t flags, const RDPECLIP::CliprdrHeader & in_header, const uint32_t requestedFormatId, const std::string & data_to_dump, const bool is_from_remote_session) {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

            if (in_header.msgFlags() & RDPECLIP::CB_RESPONSE_OK) {

                const auto type = (is_from_remote_session)
                    ? (data_to_dump.empty()
                        ? "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION"_av
                        : "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX"_av)
                    : (data_to_dump.empty()
                        ? "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION"_av
                        : "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"_av);

                auto* format_name = this->format_name_inventory.find(requestedFormatId);
                auto utf8_format = (format_name && !format_name->utf8_name().empty())
                    ? format_name->utf8_name()
                    : RDPECLIP::get_FormatId_name_av(requestedFormatId);

                bool const log_current_activity = (
                    !this->params.log_only_relevant_clipboard_activities
                 || !format_name
                 || (!ranges_equal(utf8_format,
                        Cliprdr::formats::file_group_descriptor_w.ascii_name)
                  && !ranges_equal(utf8_format,
                        Cliprdr::formats::preferred_drop_effect.ascii_name)
                ));

                auto format = str_concat(utf8_format.as_chars(),
                    '(', std::to_string(requestedFormatId), ')');

                auto const size_str = std::to_string(in_header.dataLen());

                if (log_current_activity) {
                    if (data_to_dump.empty()) {
                        this->report_message.log6(is_from_remote_session
                            ? LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION
                            : LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION,
                            this->session_reactor.get_current_time(), {
                            KVLog("format"_av, format),
                            KVLog("size"_av, size_str),
                        });
                    }
                    else {
                        this->report_message.log6(is_from_remote_session
                            ? LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX
                            : LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX,
                            this->session_reactor.get_current_time(), {
                            KVLog("format"_av, format),
                            KVLog("size"_av, size_str),
                            KVLog("partial_data"_av, data_to_dump),
                        });
                    }
                }

                LOG_IF(!this->params.dont_log_data_into_syslog, LOG_INFO,
                    "type=%s format=%s size=%s %s%s",
                    type.data(), format, size_str,
                    data_to_dump.empty() ? "" : " partial_data",
                    data_to_dump.c_str());
            }
        }
    }

private:
    void _close_tfl(ClipboardSideData::FileContent::FileData& file_data)
    {
        auto status = Mwrm3::TransferedStatus::Completed;
        if (!file_data.sig.has_digest()) {
            file_data.sig.broken();
            status = Mwrm3::TransferedStatus::Broken;
        }
        this->fdx_capture->close_tfl(*file_data.tfl_file, file_data.file_name,
            status, Mwrm3::Sha256Signature{file_data.sig.digest_as_av()});
    }
};  // class ClipboardVirtualChannel

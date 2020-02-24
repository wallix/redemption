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

/*

        Client                      Server

 CB_FORMAT_LIST           ->
                          <- CB_LOCK_CLIPDATA (id = 1)
                          <- CB_UNLOCK_CLIPDATA (id = 0)
                          <- CB_FORMAT_LIST_RESPONSE
                          <- CB_FORMAT_DATA_REQUEST
 CB_FORMAT_DATA_RESPONSE  ->

*/

class ClipboardVirtualChannel final : public BaseVirtualChannel
{
    using StreamId = ClipboardSideData::StreamId;
    using FileGroupId = ClipboardSideData::FileGroupId;

    ClipboardData clip_data;

    std::vector<CliprdFileInfo> file_descr_list;

    Cliprdr::FormatNameInventory format_name_inventory;

    class D;
    std::unique_ptr<D> d;


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

public:
    // TODO private
    enum class Direction : bool
    {
        FileFromServer,
        FileFromClient,
    };

public:
    struct FileStorage
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
        FileStorage file_storage);

    ~ClipboardVirtualChannel();

    void empty_client_clipboard()
    {
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

    [[nodiscard]] bool use_long_format_names() const;

public:
    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        // process_server_message
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task) override;

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->clipboard_monitor_ready_notifier = launcher;
        this->clipboard_initialize_notifier    = launcher;
        this->format_list_notifier             = launcher;
        this->format_list_response_notifier    = launcher;
        this->format_data_request_notifier     = launcher;
    }

    void DLP_antivirus_check_channels_files();

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
};  // class ClipboardVirtualChannel

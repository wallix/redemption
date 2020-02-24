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

#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/clipboard_virtual_channels_params.hpp"
#include "core/RDP/clipboard/format_name.hpp"

#include <vector>
#include <memory>

/*

        Client                      Server

 CB_FORMAT_LIST           ->
                          <- CB_LOCK_CLIPDATA (id = 1)
                          <- CB_UNLOCK_CLIPDATA (id = 0)
                          <- CB_FORMAT_LIST_RESPONSE
                          <- CB_FORMAT_DATA_REQUEST
 CB_FORMAT_DATA_RESPONSE  ->

*/

class FileValidatorService;
class FdxCapture;
class CliprdFileInfo;
class SessionProbeLauncher;

class ClipboardVirtualChannel final : public BaseVirtualChannel
{
    // TODO private
public:
    enum class StreamId : uint32_t;
    enum class FileGroupId : uint32_t;

private:
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

    void empty_client_clipboard();

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
};  // class ClipboardVirtualChannel

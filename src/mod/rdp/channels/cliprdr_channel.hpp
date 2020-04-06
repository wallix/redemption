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
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_name.hpp"
#include "capture/fdx_capture.hpp"
#include "mod/file_validator_service.hpp"

#include <vector>
#include <memory>
#include <string>


class FileValidatorService;
class FdxCapture;
class CliprdFileInfo;
class SessionProbeLauncher;

class ClipboardVirtualChannel final : public BaseVirtualChannel
{
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

    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data,
        std::unique_ptr<AsynchronousTask> & out_asynchronous_task, SesmanInterface & sesman) override;

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->clipboard_monitor_ready_notifier = launcher;
        this->clipboard_initialize_notifier    = launcher;
        this->format_list_notifier             = launcher;
        this->format_list_response_notifier    = launcher;
        this->format_data_request_notifier     = launcher;
    }

    void DLP_antivirus_check_channels_files();

private:
    enum class StreamId : uint32_t;
    enum class FileGroupId : uint32_t;

    std::vector<CliprdFileInfo> file_descr_list;

    Cliprdr::FormatNameInventory format_name_inventory;

    const ClipboardVirtualChannelParams params;

    SessionProbeLauncher* clipboard_monitor_ready_notifier = nullptr;
    SessionProbeLauncher* clipboard_initialize_notifier    = nullptr;
    SessionProbeLauncher* format_list_notifier             = nullptr;
    SessionProbeLauncher* format_list_response_notifier    = nullptr;
    SessionProbeLauncher* format_data_request_notifier     = nullptr;

    SessionReactor& session_reactor;

    FileValidatorService * file_validator;

    FdxCapture * fdx_capture;

    const bool always_file_storage;
    bool can_lock = false;
    const bool proxy_managed;   // Has not client.

private:
    enum class LockId : uint32_t;

    struct ClipCtx
    {
        struct Sig
        {
            void reset();

            void update(bytes_view data);

            void final();

            void broken();

            bool has_digest() const noexcept;

            static const std::size_t digest_len = SslSha256::DIGEST_LENGTH;

            auto& digest() const noexcept;

            bytes_view digest_as_av() const noexcept;

        private:
            enum class Status : uint8_t;

            SslSha256_Delayed sha256;
            uint8_t array[digest_len];
            Status status;
        };

        using StreamId = ClipboardVirtualChannel::StreamId;
        using FileGroupId = ClipboardVirtualChannel::FileGroupId;

        struct FileContentsSize
        {
            StreamId stream_id;
            FileGroupId lindex;
        };

        struct FileContentsRequestedRange
        {
            StreamId stream_id;
            FileGroupId lindex;
            uint64_t file_size_requested;
            uint64_t file_size;
            std::string file_name;
        };

        struct FileContentsRange
        {
            // FileContentsSize, FileContentsRequestedRange, FileContentsRange
            StreamId stream_id;
            FileGroupId lindex;
            // TextData and FileContentsRange
            FileValidatorId file_validator_id;
            // FileContentsRange
            enum class ValidatorState : uint8_t {
                Wait,
                Failure,
                Success,
                WaitValidatorBeforeTransfer,
                TransferAfterValidation,
            };
            ValidatorState validator_state;
            uint64_t file_offset;
            // FileContentsRequestedRange, FileContentsRange
            uint64_t file_size_requested;
            uint64_t file_size;
            std::string file_name;

            std::unique_ptr<FdxCapture::TflFile> tfl_file;
            std::vector<uint8_t> file_content;
            Sig sig = Sig();
        };

        struct TextData
        {
            FileValidatorId file_validator_id;
            bool is_unicode;
        };

        enum class TransferState :  uint8_t {
            Empty,
            Size,
            Range,
            RequestedRange,
            WaitingContinuationRange,
            Text,
        };

        struct NoLockData
        {
            TransferState transfer_state = TransferState::Empty;

            operator TransferState() const
            {
                return this->transfer_state;
            }

            // TextData
            bool is_unicode;

            // TextData, FileContentsSize, FileContentsRequestedRange, FileContentsRange
            FileContentsRange data;

            using ValidatorState = FileContentsRange::ValidatorState;

            void init_empty();
            void init_text(FileValidatorId file_validator_id, bool is_unicode);
            void init_size(StreamId stream_id, FileGroupId lindex);
            void init_requested_range(
                StreamId stream_id, FileGroupId lindex,
                uint64_t file_size_requested, uint64_t file_size, std::string_view file_name);
            void requested_range_to_range(
                FileValidatorId file_validator_id, std::unique_ptr<FdxCapture::TflFile>&& tfl,
                ValidatorState validator_state);

            void set_waiting_continuation_range();
            void set_range();
        };

        struct LockedData
        {
            struct LockedSize
            {
                LockId lock_id;
                FileContentsSize file_contents_size;
            };

            struct LockedRange
            {
                enum State : bool { WaitingResponse, WaitingRequest, };

                LockId lock_id;
                State state;
                FileContentsRange file_contents_range;
            };

            struct LockedRequestedRange
            {
                LockId lock_id;
                FileContentsRequestedRange file_contents_requested_range;

                bool is_stream_id(StreamId id)
                {
                    return file_contents_requested_range.stream_id == id
                    && not file_contents_requested_range.file_name.empty();
                }

                void disable()
                {
                    this->file_contents_requested_range.file_name.clear();
                }
            };

            struct LockedFileList
            {
                LockId lock_id;
                std::vector<CliprdFileInfo> files;
            };

            std::vector<LockedSize> sizes;
            std::vector<LockedRange> ranges;

            LockedRequestedRange requested_range {};

            std::vector<LockedFileList> lock_list;

            void clear();

            LockedFileList* search_lock_by_id(LockId lock_id);

            LockedRange* search_range_by_id(StreamId stream_id);

            LockedRange* search_range_by_validator_id(FileValidatorId id);

            void remove_locked_file_contents_range(LockedRange* p);

            LockedRange* search_range_by_offset(LockId lock_id, FileGroupId ifile, uint64_t offset);

            LockedSize* search_size_by_id(StreamId stream_id);

            void remove_locked_file_contents_size(LockedSize* p);

            bool contains_stream_id(StreamId stream_id);
        };

        struct OptionalLockId
        {
            void disable();

            void enable(bool activate);

            [[nodiscard]]
            bool is_enabled() const;

            [[nodiscard]]
            bool has_lock() const;

            [[nodiscard]]
            bool has_unused_lock() const;

            void set_used();

            void set_lock_id(LockId id);

            void unset_lock_id();

            [[nodiscard]]
            LockId lock_id() const;

        private:
            enum State : uint8_t;

            State _state = State();
            LockId _lock_id;
        };

        ClipCtx(std::string const& target_name, bool verify_before_download);

        uint16_t message_type = 0;

        bool use_long_format_names = false;
        bool has_current_file_contents_stream_id = false;
        const bool verify_before_download;
        StreamId current_file_contents_stream_id;
        uint32_t current_file_list_format_id;
        uint32_t requested_format_id;

        uint32_t clip_text_locale_identifier = 0;

        OptionalLockId optional_lock_id;

        Cliprdr::FormatNameInventory current_format_list;

        const std::string validator_target_name;

        std::vector<CliprdFileInfo> files;

        NoLockData nolock_data;
        LockedData locked_data;

        StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

        void clear();
    };

    struct FileValidatorDataList;
    struct TextValidatorDataList;

    using ClientCtx = ClipCtx;
    using ServerCtx = ClipCtx;

    ClientCtx client_ctx;
    ServerCtx server_ctx;

    std::vector<FileValidatorDataList> file_validator_list;
    std::vector<TextValidatorDataList> text_validator_list;

    FileValidatorDataList* search_file_validator_by_id(FileValidatorId id);

    void remove_file_validator(FileValidatorDataList* p);

    TextValidatorDataList* search_text_validator_by_id(FileValidatorId id);

    void remove_text_validator(TextValidatorDataList* p);

    class D;
    friend class D;
}; // class ClipboardVirtualChannel


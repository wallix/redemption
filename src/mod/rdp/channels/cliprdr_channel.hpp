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
#include "mod/rdp/rdp_verbose.hpp"
#include "mod/file_validator_service.hpp"
#include "core/RDP/clipboard/format_name.hpp"
#include "core/events.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "system/ssl_sha256.hpp"

#include <vector>
#include <memory>
#include <string>


namespace gdi
{
    class OsdApi;
}
class FdxCapture;
class CliprdFileInfo;
class SessionProbeLauncher;
class AuthApi;
class SessionLogApi;

class ClipboardVirtualChannel final : public BaseVirtualChannel
{
public:
    struct FileStorage
    {
        FdxCapture * fdx_capture;
        bool always_file_storage;
        std::string tmp_dir;
    };

    ClipboardVirtualChannel(
        VirtualChannelDataSender* to_client_sender_,
        VirtualChannelDataSender* to_server_sender_,
        EventContainer& events,
        gdi::OsdApi& osd_api,
        const ClipboardVirtualChannelParams& params,
        FileValidatorService* file_validator_service,
        FileStorage file_storage,
        SessionLogApi& session_log,
        RDPVerbose verbose);

    ~ClipboardVirtualChannel();

    void empty_client_clipboard();

    [[nodiscard]] bool use_long_format_names() const;

    void process_client_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    void process_server_message(uint32_t total_length,
        uint32_t flags, bytes_view chunk_data) override;

    void set_session_probe_launcher(SessionProbeLauncher* launcher) {
        this->clipboard_monitor_ready_notifier = launcher;
        this->clipboard_initialize_notifier    = launcher;
        this->format_list_notifier             = launcher;
        this->format_list_response_notifier    = launcher;
        this->format_data_request_notifier     = launcher;
    }

    void DLP_antivirus_check_channels_files();

private:
    std::vector<CliprdFileInfo> file_descr_list;

    Cliprdr::FormatNameInventory format_name_inventory;

    const ClipboardVirtualChannelParams params;

    SessionProbeLauncher* clipboard_monitor_ready_notifier = nullptr;
    SessionProbeLauncher* clipboard_initialize_notifier    = nullptr;
    SessionProbeLauncher* format_list_notifier             = nullptr;
    SessionProbeLauncher* format_list_response_notifier    = nullptr;
    SessionProbeLauncher* format_data_request_notifier     = nullptr;

    FileValidatorService * file_validator;

    FdxCapture * fdx_capture;
    std::string tmp_dir;

    SessionLogApi & session_log;

    const RDPVerbose verbose;

    const bool always_file_storage;
    bool can_lock = false;
    const bool proxy_managed;   // Has not client.

private:
    enum class LockId : uint32_t;
    enum class StreamId : uint32_t;
    enum class FileGroupId : uint32_t;

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

            enum class Status : uint8_t;
            operator Status () const { return this->status; }

        private:
            SslSha256_Delayed sha256;
            uint8_t array[digest_len];
            Status status;
        };

        struct FileContentsSize
        {
            StreamId stream_id;
            FileGroupId lindex;
        };

        struct FileContentsRequestedRange
        {
            StreamId stream_id;
            FileGroupId lindex;
            uint32_t file_size_requested;
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
            };
            ValidatorState validator_state;
            uint64_t file_offset;
            // FileContentsRequestedRange
            uint32_t first_file_size_requested;
            // FileContentsRequestedRange, FileContentsRange
            uint32_t file_size_requested;
            uint64_t file_size;
            // GetRange
            uint32_t response_size;
            // FileContentsRequestedRange, FileContentsRange
            std::string file_name;

            struct TflFile;
            std::unique_ptr<TflFile> tfl_file_ptr;
            Sig sig = Sig();

            bool is_finalized() const
            {
                return this->sig.has_digest();
            }
        };

        enum class TransferState : uint8_t;
        struct NoLockData
        {
            TransferState transfer_state = TransferState();

            operator TransferState() const
            {
                return this->transfer_state;
            }

            // TextData, FileContentsSize, FileContentsRequestedRange, FileContentsRange
            struct FileData : FileContentsRange
            {
                struct FileContent
                {
                    void append(bytes_view data);

                    bytes_view read(std::size_t n);

                    void clear();

                    uint64_t size() const noexcept { return this->data_len; }
                    uint64_t offset() const noexcept { return this->data_pos; }
                    void set_offset(uint64_t offset);
                    void set_read_mode();

                    char const* tmp_dir;

                private:
                    struct Buffer
                    {
                        std::unique_ptr<uint8_t[]> buf;
                        std::size_t pos = 0;
                        std::size_t capacity = 0;

                        uint8_t* current() const;
                        std::size_t remaining() const;
                    };

                    std::unique_ptr<uint8_t[]> memory_buffer;
                    Buffer requested_buffer;
                    unique_fd fd = invalid_fd();
                    uint64_t data_len = 0;
                    uint64_t data_pos = 0;
                };

                FileContent file_content;
            };
            FileData data;

            struct D;
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

        ClipCtx(
            std::string const& target_name,
            bool verify_file_before_transfer,
            bool verify_text_before_transfer,
            uint64_t max_file_size_rejected,
            char const* tmp_dir);

        uint16_t message_type = 0;

        bool use_long_format_names = false;
        bool has_current_file_contents_stream_id = false;
        const bool verify_file_before_transfer;
        const bool verify_text_before_transfer;
        uint64_t max_file_size_rejected;
        StreamId current_file_contents_stream_id;
        uint32_t current_file_list_format_id = 0;
        uint32_t requested_format_id;

        uint32_t clip_text_locale_identifier = 0;

        OptionalLockId optional_lock_id;

        Cliprdr::FormatNameInventory current_format_list;

        const std::string validator_target_name;

        std::vector<CliprdFileInfo> files;

        NoLockData nolock_data;
        LockedData locked_data;

        StaticOutStream</*RDPECLIP::FileDescriptor::size()=*/592> file_descriptor_stream;

        void clear();

        struct D;
    };

    struct FileValidatorDataList;
    struct TextValidatorDataList;

    using ClientCtx = ClipCtx;
    using ServerCtx = ClipCtx;

    ClientCtx client_ctx;
    ServerCtx server_ctx;

    struct OSD
    {
        EventsGuard events_guard;
        gdi::OsdApi& osd_api;
        const std::chrono::seconds delay;
        const bool enable_osd;
        Language lang;

        enum class MsgType : bool { Nothing, WaitValidator };

        EventRef event_ref {};
        MsgType msg_type = MsgType::Nothing;

        class D;
    };

    OSD osd;

    std::vector<FileValidatorDataList> file_validator_list;
    std::vector<TextValidatorDataList> text_validator_list;

    FileValidatorDataList* search_file_validator_by_id(FileValidatorId id);

    void remove_file_validator(FileValidatorDataList* p);

    TextValidatorDataList* search_text_validator_by_id(FileValidatorId id);

    void remove_text_validator(TextValidatorDataList* p);
}; // class ClipboardVirtualChannel

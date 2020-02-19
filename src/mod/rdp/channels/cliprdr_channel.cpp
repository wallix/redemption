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

#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "utils/sugar/not_null_ptr.hpp"


namespace
{
    enum LockId : uint32_t;

    struct ClipCtx
    {
        struct Sig
        {
            void update(bytes_view data)
            {
                this->sha256.update(data);
            }

            void final()
            {
                assert(this->status == Status::Update);
                this->sha256.final(this->array);
                this->status = Status::Final;
            }

            void broken()
            {
                assert(this->status == Status::Update);
                this->sha256.final(this->array);
                this->status = Status::Broken;
            }

            bool has_digest() const noexcept
            {
                return this->status != Status::Update;
            }

            static const std::size_t digest_len = SslSha256::DIGEST_LENGTH;

            auto& digest() const noexcept
            {
                assert(this->has_digest());
                return this->array;
            }

            bytes_view digest_as_av() const noexcept
            {
                return make_array_view(this->digest());
            }

        private:
            enum class Status : uint8_t {
                Update,
                Broken,
                Final,
            };

            SslSha256 sha256;
            uint8_t array[digest_len];
            Status status = Status::Update;
        };

        using StreamId = ClipboardSideData::StreamId;
        using FileGroupId = ClipboardSideData::FileGroupId;

        struct FileContentsSize
        {
            StreamId stream_id;
            FileGroupId lindex;
        };

        struct FileContentsRange
        {
            StreamId stream_id;
            FileGroupId lindex;
            uint64_t file_offset;
            uint64_t file_size_requested;
            uint64_t file_size;

            FileValidatorId file_validator_id;

            std::unique_ptr<FdxCapture::TflFile> tfl_file;

            Sig sig;

            bool on_failure = false;

            // TODO merge with on_failure
            enum WaitValidator : bool { Yes, No };
            // ignored if file_validator_id == 0
            // TODO WaitValidator::No <=> FileValidatorId(0)
            WaitValidator wait_validator = WaitValidator::No;

            void set_wait_validator()
            {
                this->wait_validator = WaitValidator::Yes;
            }
        };

        // TODO within Ctx
        // struct ValidatorWaitingList
        // {
        //
        // };

        bool use_long_format_names;
        bool has_current_lock = false;
        // if true, current_lock_id id pushed to lock_list (avoid duplication)
        bool current_lock_id_is_used;
        uint32_t current_lock_id;


        uint32_t current_file_list_format_id;
        uint32_t next_file_list_format_id;
        Cliprdr::FormatNameInventory current_format_list;
        Cliprdr::FormatNameInventory next_format_list;

        std::string validator_target_name;

        StreamId current_file_contents_stream_id;
        bool has_current_file_contents_stream_id;

        StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

        std::vector<CliprdFileInfo> files;

        uint32_t requested_format_id;

        enum class TransferState :  uint8_t { Empty, Size, Range, WaitingContinuationRange, };

        TransferState transfert_state = TransferState::Empty;

        union FileContentsData
        {
            char dummy;
            FileContentsSize size;
            FileContentsRange range;
        };

        FileContentsData file_contents_data;

        struct LockedFileContentsSize
        {
            LockId lock_id;
            FileContentsSize file_contents_size;
        };

        enum LockedTransferState : bool { WaitingResponse, WaitingRequest, };

        struct LockedFileContentsRange
        {
            LockId lock_id;
            LockedTransferState state;
            FileContentsRange file_contents_range;
        };

        std::vector<LockedFileContentsSize> locked_file_contents_sizes;
        std::vector<LockedFileContentsRange> locked_file_contents_ranges;

        struct LockData
        {
            uint32_t lock_id;
            int rdp_protocol_ref;
            int file_verification_ref;

            int count_ref() const
            {
                return this->rdp_protocol_ref + this->file_verification_ref;
            }

            std::vector<CliprdFileInfo> files;
        };

        std::vector<LockData> lock_list;

        void dec_lock(not_null_ptr<LockData> plock_data)
        {
            if (--plock_data->rdp_protocol_ref) {
                // TODO remove
            }
        }

        LockData* search_lock_by_id(LockId lock_id)
        {
            for (auto& r : this->lock_list) {
                if (r.lock_id == lock_id) {
                    return &r;
                }
            }
            return nullptr;
        }

        LockedFileContentsRange* search_range_by_id(StreamId stream_id)
        {
            for (auto& r : this->locked_file_contents_ranges) {
                if (r.file_contents_range.stream_id == stream_id
                 && r.state == LockedTransferState::WaitingResponse) {
                    return &r;
                }
            }
            return nullptr;
        }

        LockedFileContentsRange* search_range_by_offset(LockId lock_id, uint64_t offset)
        {
            for (auto& r : this->locked_file_contents_ranges) {
                if (r.lock_id == lock_id
                 && r.state == LockedTransferState::WaitingRequest
                 && r.file_contents_range.file_offset == offset
                ) {
                    return &r;
                }
            }
            return nullptr;
        }

        LockedFileContentsSize* search_size_by_id(StreamId stream_id)
        {
            for (auto& r : this->locked_file_contents_sizes) {
                if (r.file_contents_size.stream_id == stream_id) {
                    return &r;
                }
            }
            return nullptr;
        }

        bool contains_stream_id(StreamId stream_id)
        {
            return this->search_range_by_id(stream_id)
                || this->search_size_by_id(stream_id);
        }
    };

    struct ClientCtx : ClipCtx {};
    struct ServerCtx : ClipCtx {};
}

struct ClipboardVirtualChannel::D
{
    ClientCtx client;
    ServerCtx server;
    ClipboardVirtualChannel& self;
    // boost::sml::sm<cliprdr_tt> sm{DataCtxRef{data_ctx}};

    // TODO ValidatorWaitingList

    D(ClipboardVirtualChannel& self)
    : self(self)
    {}

    void format_list(
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, bytes_view chunk_data)
    {
        clip.next_format_list.clear();
        clip.next_file_list_format_id = 0;

        InStream in_stream(chunk_data);
        FormatListReceive receiver(
            clip.use_long_format_names,
            in_header,
            in_stream,
            clip.next_format_list,
            self.verbose);
        clip.next_file_list_format_id = receiver.file_list_format_id;
    }

    void format_list_response(ClipCtx& clip)
    {
        // TODO cleat id error
        clip.current_file_list_format_id = clip.next_file_list_format_id;
        clip.current_format_list.swap(clip.next_format_list);
        clip.files.clear();
        // TODO move to format_list ?
        clip.has_current_lock = false;
        // TODO reset file_contents_data
        clip.transfert_state = ClipCtx::TransferState::Empty;
    }

    void lock(ClipCtx& clip, bytes_view chunk_data)
    {
        RDPECLIP::LockClipboardDataPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);

        if (bool(this->self.verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }

        clip.has_current_lock = true;
        clip.current_lock_id = pdu.clipDataId;
        clip.current_lock_id_is_used = false;
    }

    void unlock(ClipCtx& clip, bytes_view chunk_data)
    {
        RDPECLIP::UnlockClipboardDataPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);

        if (bool(this->self.verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }

        if (clip.has_current_lock && clip.current_lock_id == pdu.clipDataId) {
            clip.has_current_lock = false;
        }
        else {
            // TODO remove waiting file content with lock_id = pdu.clipDataId

            auto p = std::find_if(
                clip.lock_list.begin(), clip.lock_list.end(),
                [&](ClipCtx::LockData const& l){ return l.lock_id == pdu.clipDataId; });

            if (p == clip.lock_list.end()) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_unlock_pdu:"
                    " unknown clipDataId (%u)", pdu.clipDataId);
                throw Error(ERR_RDP_PROTOCOL);
            }

            --p->rdp_protocol_ref;
            if (not p->count_ref()) {
                *p = std::move(clip.lock_list.back());
                clip.lock_list.pop_back();
            }
        }
    }

    void format_data_request(ClipCtx& clip, bytes_view chunk_data)
    {
        RDPECLIP::FormatDataRequestPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);
        clip.requested_format_id = pdu.requestedFormatId;

        if (bool(this->self.verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }

    void format_data_response(
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data)
    {
        // TODO reset other fields

        // TODO check flags: start / done / wait, exception file_contents_* ?

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            clip.files.clear();
        }

        if (clip.current_file_list_format_id
         && clip.current_file_list_format_id == clip.requested_format_id
        ) {
            InStream in_stream(chunk_data);
            FormatDataResponseReceiveFileList receiver(
                clip.files,
                in_stream,
                in_header,
                this->self.params.dont_log_data_into_syslog,
                clip.current_file_list_format_id,
                flags,
                clip.file_descriptor_stream,
                this->self.verbose,
                /*is_from_remote_session*/ &clip == &this->server ? "client" : "server"
            );

            if (clip.has_current_lock && not clip.current_lock_id_is_used) {
                clip.lock_list.push_back(ClipCtx::LockData{
                    clip.lock_list.back().lock_id, 1, 0, std::move(clip.files)});
                clip.current_lock_id_is_used = true;
            }
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            clip.requested_format_id = 0;
        }
    }

    void filecontents_request(ClipCtx& clip, uint32_t flags, bytes_view chunk_data)
    {
        InStream in_stream(chunk_data);
        RDPECLIP::FileContentsRequestPDU file_contents_request_pdu;
        file_contents_request_pdu.receive(in_stream);

        if (bool(self.verbose & RDPVerbose::cliprdr)) {
            file_contents_request_pdu.log(LOG_INFO);
        }

        auto const first_and_last = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;

        if ((flags & first_and_last) != first_and_last) {
            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu: Unsupported partial packet");
            throw Error(ERR_RDP_UNSUPPORTED);
        }

        // TODO
        // if (!this->self.params.clipboard_file_authorized) {
        //     FilecontentsRequestSendBack(
        //         file_contents_request_pdu.dwFlags(),
        //         file_contents_request_pdu.streamId(),
        //         sender);
        //     return false;
        // }

        const auto lindex = file_contents_request_pdu.lindex();
        const auto stream_id = StreamId(file_contents_request_pdu.streamId());
        const auto ifile = FileGroupId(lindex);

        auto update_continuation_range = [&](ClipCtx::FileContentsRange& rng){
            if (rng.file_offset != file_contents_request_pdu.position()) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " Unsupported random access for a FILECONTENTS_RANGE");
                throw Error(ERR_RDP_UNSUPPORTED);
            }
            rng.stream_id = stream_id;
            rng.file_size_requested = file_contents_request_pdu.cbRequested();
        };

        auto new_range = [&]{
            FileValidatorId file_validator_id{};
            if (!clip.validator_target_name.empty()) {
                LOG_IF(bool(this->self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::Validator::open_file");
                file_validator_id = this->self.file_validator->open_file(
                    clip.files[lindex].file_name, clip.validator_target_name);
            }

            return ClipCtx::FileContentsRange{
                stream_id,
                ifile,
                0,
                file_contents_request_pdu.cbRequested(),
                clip.files[file_contents_request_pdu.lindex()].file_size,
                file_validator_id,
                this->self.fdx_capture
                    ? std::unique_ptr<FdxCapture::TflFile>(new FdxCapture::TflFile(
                        this->self.fdx_capture->new_tfl((&clip == &this->server)
                            ? Mwrm3::Direction::ServerToClient
                            : Mwrm3::Direction::ClientToServer
                    )))
                    : std::unique_ptr<FdxCapture::TflFile>()
            };
        };

        if (not file_contents_request_pdu.has_optional_clipDataId()) {
            if (file_contents_request_pdu.lindex() >= clip.files.size()) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " Invalid lindex %u", lindex);
                throw Error(ERR_RDP_PROTOCOL);
            }

            switch (clip.transfert_state)
            {
                case ClipCtx::TransferState::Empty:
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        if (file_contents_request_pdu.position() != 0) {
                            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                " Unsupported random access for a FILECONTENTS_RANGE");
                            throw Error(ERR_RDP_UNSUPPORTED);
                        }
                        new (&clip.file_contents_data.range) ClipCtx::FileContentsRange{
                            new_range()};
                        clip.transfert_state = ClipCtx::TransferState::Range;
                    }
                    else {
                        new (&clip.file_contents_data.size) ClipCtx::FileContentsSize{
                            stream_id, ifile
                        };
                        clip.transfert_state = ClipCtx::TransferState::Size;
                    }
                    break;
                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::Range:
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                        " double request");
                    throw Error(ERR_RDP_PROTOCOL);
                case ClipCtx::TransferState::WaitingContinuationRange:
                    update_continuation_range(clip.file_contents_data.range);
                    break;
            }
        }
        else {
            const auto lock_id = LockId(file_contents_request_pdu.clipDataId());

            auto* lock_data = clip.search_lock_by_id(lock_id);

            if (not lock_data) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " unknown clipDataId (%u)", lock_id);
                throw Error(ERR_RDP_PROTOCOL);
            }

            if (file_contents_request_pdu.lindex() >= lock_data->files.size()) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " Invalid lindex %u", lindex);
                throw Error(ERR_RDP_PROTOCOL);
            }

            if (clip.contains_stream_id(stream_id)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " streamId already used (%u)", stream_id);
                throw Error(ERR_RDP_PROTOCOL);
            }

            if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                auto* r = clip.search_range_by_offset(
                    lock_id, file_contents_request_pdu.position());
                if (r) {
                    update_continuation_range(r->file_contents_range);
                }
                else {
                    clip.locked_file_contents_ranges.emplace_back(
                        ClipCtx::LockedFileContentsRange{
                            lock_id, ClipCtx::LockedTransferState::WaitingResponse, new_range()});
                    ++lock_data->rdp_protocol_ref;
                }
            }
            else {
                clip.locked_file_contents_sizes.push_back(
                    {lock_id, {stream_id, ifile}});
                ++lock_data->rdp_protocol_ref;
            }
        }
    }

    void filecontents_response(
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data)
    {
        InStream in_stream(chunk_data);

        // TODO check first / cont / last status (otherwise, invalid current_file_contents_stream_id)

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            ::check_throw(in_stream, 4, "FileContentsResponse::receive", ERR_RDP_DATA_TRUNCATED);
            if (clip.has_current_file_contents_stream_id) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " streamId already defined");
                throw Error(ERR_RDP_PROTOCOL);
            }
            clip.current_file_contents_stream_id = StreamId(in_stream.in_uint32_le());
            clip.has_current_file_contents_stream_id = true;
            LOG_IF(bool(this->self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                "File Contents Response: streamId=%u", clip.current_file_contents_stream_id);
        }

        auto const stream_id = clip.current_file_contents_stream_id;

        auto update_file_size_or_throw = [](
            std::vector<CliprdFileInfo>& files,
            ClipCtx::FileContentsSize& file_contents_size,
            uint32_t flags, bytes_view chunk_data
        ){
            if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " Unsupported partial FILECONTENTS_SIZE packet");
                throw Error(ERR_RDP_UNSUPPORTED);
            }

            InStream in_stream(chunk_data);
            check_throw(in_stream, 8, "process_filecontents_response_pdu", ERR_RDP_DATA_TRUNCATED);
            files[size_t(file_contents_size.lindex)].file_size = in_stream.in_uint64_le();
        };

        auto update_file_range_data = [](
            ClipCtx::FileContentsRange& file_contents_range,
            bytes_view data
        ){
            if (data.size() >= file_contents_range.file_size_requested) {
                data = data.first(file_contents_range.file_size_requested);
            }

            file_contents_range.sig.update(data);
            file_contents_range.file_offset += data.size();
            file_contents_range.file_size_requested -= data.size();

            if (0 == file_contents_range.file_size_requested
             && file_contents_range.file_offset == file_contents_range.file_size
            ) {
                file_contents_range.sig.final();
                file_contents_range.set_wait_validator();
            }

            if (bool(file_contents_range.tfl_file)) {
                file_contents_range.tfl_file->trans.send(data);
            }
        };

        auto finalize_transfer = [this, &clip](
            std::vector<CliprdFileInfo>& files,
            ClipCtx::FileContentsRange& file_contents_range
        ){
            this->log_file_info(files, file_contents_range, (&clip == &this->server));

            if (bool(file_contents_range.file_validator_id)) {
                this->self.file_validator->send_eof(file_contents_range.file_validator_id);
                // TODO move to validator list
            }
            else {
                if (file_contents_range.tfl_file) {
                    if (this->self.always_file_storage || file_contents_range.on_failure) {
                        std::string_view file_name
                            = files[size_t(file_contents_range.lindex)].file_name;
                        this->self.fdx_capture->close_tfl(
                            *file_contents_range.tfl_file,
                            file_name,
                            Mwrm3::TransferedStatus::Completed,
                            Mwrm3::Sha256Signature{file_contents_range.sig.digest_as_av()});
                    }
                    else {
                        this->self.fdx_capture->cancel_tfl(*file_contents_range.tfl_file);
                    }
                    file_contents_range.tfl_file.reset();
                }
                // TODO remove element
            }
        };

        switch (clip.transfert_state)
        {
        case ClipCtx::TransferState::Empty:
        case ClipCtx::TransferState::WaitingContinuationRange:
            break;
        case ClipCtx::TransferState::Size:
            // TODO check flag = last
            if (clip.file_contents_data.size.stream_id == stream_id) {
                if (in_header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) {
                    update_file_size_or_throw(
                        clip.files, clip.file_contents_data.size, flags, chunk_data);
                }
                clip.transfert_state = ClipCtx::TransferState::WaitingContinuationRange;
                clip.has_current_file_contents_stream_id = false;
                return ;
            }
            break;
        case ClipCtx::TransferState::Range:
            if (clip.file_contents_data.range.stream_id == stream_id) {
                if (in_header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
                    clip.has_current_file_contents_stream_id = false;
                    clip.transfert_state = ClipCtx::TransferState::WaitingContinuationRange;
                    if (bool(clip.file_contents_data.range.file_validator_id)) {
                        this->self.file_validator->send_eof(
                            clip.file_contents_data.range.file_validator_id);
                        clip.file_contents_data.range.set_wait_validator();
                        // TODO move to another list ?
                    }
                    // TODO tfl error
                }
                else {
                    // TODO check response status
                    auto& rng = clip.file_contents_data.range;
                    update_file_range_data(rng, chunk_data);
                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)
                        && rng.file_offset == rng.file_size
                    ) {
                        finalize_transfer(clip.files, rng);
                    }
                }
                return ;
            }
            break;
        }

        if (auto* locked_contents_range = clip.search_range_by_id(stream_id)) {
            auto& rng = locked_contents_range->file_contents_range;

            if (in_header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
                not_null_ptr lock_data = clip.search_lock_by_id(locked_contents_range->lock_id);
                clip.dec_lock(lock_data);
                clip.has_current_file_contents_stream_id = false;

                if (bool(rng.file_validator_id)) {
                    this->self.file_validator->send_eof(rng.file_validator_id);
                    rng.set_wait_validator();
                    // TODO move to another list ?
                }
                // TODO remove element ?
                // TODO tfl error
            }
            else {
                update_file_range_data(rng, chunk_data);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    not_null_ptr lock_data = clip.search_lock_by_id(locked_contents_range->lock_id);
                    clip.has_current_file_contents_stream_id = false;

                    if (rng.file_offset == rng.file_size) {
                        finalize_transfer(lock_data->files, rng);
                    }

                    clip.dec_lock(lock_data);
                    // TODO remove range
                }
            }
        }
        else if (auto* locked_contents_size = clip.search_size_by_id(stream_id)) {
            // TODO check flag = last
            not_null_ptr lock_data = clip.search_lock_by_id(locked_contents_size->lock_id);
            if (in_header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
                clip.dec_lock(lock_data);
            }
            else {
                update_file_size_or_throw(
                    lock_data->files, locked_contents_size->file_contents_size, flags, chunk_data);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    clip.dec_lock(lock_data);
                    // TODO remove size
                }
            }
        }
        else {
            // TODO error
        }
    }

    void log_file_info(
        std::vector<CliprdFileInfo>& files,
        ClipCtx::FileContentsRange& file_contents_range,
        bool from_remote_session)
    {
        const char* type = (
            from_remote_session
                ? "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
                : "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION"
        );

        if (!file_contents_range.sig.has_digest()) {
            file_contents_range.sig.broken();
        }

        static_assert(SslSha256::DIGEST_LENGTH == decltype(file_contents_range.sig)::digest_len);
        auto& digest = file_contents_range.sig.digest();
        char digest_s[128];
        size_t digest_s_len = snprintf(digest_s, sizeof(digest_s),
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            digest[ 0], digest[ 1], digest[ 2], digest[ 3], digest[ 4], digest[ 5], digest[ 6], digest[ 7],
            digest[ 8], digest[ 9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15],
            digest[16], digest[17], digest[18], digest[19], digest[20], digest[21], digest[22], digest[23],
            digest[24], digest[25], digest[26], digest[27], digest[28], digest[29], digest[30], digest[31]);

        char file_size[128];
        size_t file_size_len = std::snprintf(file_size, std::size(file_size), "%lu", file_contents_range.file_size);

        std::string_view file_name = files[size_t(file_contents_range.lindex)].file_name;
        this->self.report_message.log6(
            from_remote_session
                ? LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION
                : LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION,
            this->self.session_reactor.get_current_time(), {
            KVLog("file_name"_av, file_name),
            KVLog("size"_av, {file_size, file_size_len}),
            KVLog("sha256"_av, {digest_s, digest_s_len}),
        });

        LOG_IF(!this->self.params.dont_log_data_into_syslog, LOG_INFO,
            "type=%s file_name=%.*s size=%s sha256=%s",
            type, int(file_name.size()), file_name.data(), file_size, digest_s);
    }

    void DLP_antivirus_check_channels_files()
    {
        if (!this->self.file_validator) {
            return ;
        }

        auto receive_data = [this]{
            for (;;) {
                switch (this->self.file_validator->receive_response()) {
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
            switch (this->self.file_validator->last_result_flag()) {
                case ValidationResult::Wait:
                    return;
                case ValidationResult::IsAccepted:
                    is_accepted = true;
                    [[fallthrough]];
                case ValidationResult::IsRejected:
                case ValidationResult::Error:
                    ;
            }

            auto file_validator_id = this->self.file_validator->last_file_id();
            auto& result_content = this->self.file_validator->get_content();

            bool is_client_text = this->clip_data.client_data.remove_text_id(file_validator_id);
            bool is_server_text = not is_client_text
                               && this->clip_data.server_data.remove_text_id(file_validator_id);
            bool is_text = is_client_text || is_server_text;

            if (is_text) {
                if (!is_accepted || this->self.params.validator_params.log_if_accepted) {
                    auto str_direction = is_client_text ? "UP"_av : "DOWN"_av;
                    char buf[24];
                    unsigned n = std::snprintf(buf, std::size(buf), "%" PRIu32,
                        underlying_cast(file_validator_id));
                    this->self.report_message.log6(LogId::TEXT_VERIFICATION, this->session_reactor.get_current_time(), {
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
                    ? this->self.params.validator_params.up_target_name
                    : this->self.params.validator_params.down_target_name;
                this->self.report_message.log6(LogId::FILE_VERIFICATION_ERROR, this->session_reactor.get_current_time(), {
                    KVLog("icap_service"_av, target_name),
                    KVLog("status"_av, "Invalid file id"_av),
                });
                continue;
            }

            auto& file_data = file->file_data;
            file_data.on_failure = !is_accepted;
            file_data.file_validator_id = FileValidatorId();

            if (!is_accepted || this->self.params.validator_params.log_if_accepted) {
                auto str_direction = (direction == Direction::FileFromClient) ? "UP"_av : "DOWN"_av;

                this->self.report_message.log6(LogId::FILE_VERIFICATION, this->self.session_reactor.get_current_time(), {
                    KVLog("direction"_av, str_direction),
                    KVLog("file_name"_av, file_data.file_name),
                    KVLog("status"_av, result_content),
                });
            }

            if (file->is_wait_validator()) {
                if (file_data.tfl_file) {
                    if (this->self.always_file_storage
                     || this->self.file_validator->last_result_flag() != ValidationResult::IsAccepted
                    ) {
                        this->self._close_tfl(file_data);
                    }
                    else {
                        this->self.fdx_capture->cancel_tfl(*file_data.tfl_file);
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

    // void process_client_message(uint32_t total_length, uint32_t flags, bytes_view chunk_data) {}
    // void process_server_message(uint32_t total_length, uint32_t flags, bytes_view chunk_data) {}
};

ClipboardVirtualChannel::ClipboardVirtualChannel(
    VirtualChannelDataSender* to_client_sender_,
    VirtualChannelDataSender* to_server_sender_,
    SessionReactor& session_reactor,
    const BaseVirtualChannel::Params & base_params,
    const ClipboardVirtualChannelParams & params,
    FileValidatorService * file_validator_service,
    FileStorage file_storage)
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
, fdx_capture(file_storage.fdx_capture)
, always_file_storage(file_storage.always_file_storage)
{
    this->d.reset(new D{*this});
}

ClipboardVirtualChannel::~ClipboardVirtualChannel()
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

void ClipboardVirtualChannel::process_server_message(
    uint32_t total_length, uint32_t flags, bytes_view chunk_data,
    std::unique_ptr<AsynchronousTask> & out_asynchronous_task)
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

        case RDPECLIP::CB_FORMAT_LIST: {
            auto pkt_data = chunk.remaining_bytes();
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

            if (send_message_to_client) {
                this->d->format_list(header, this->d->server, pkt_data);
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
            this->d->format_list_response(this->d->client);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            auto pkt_data = chunk.remaining_bytes();
            send_message_to_client = this->process_server_format_data_request_pdu(
                total_length, flags, chunk, header);

            if (send_message_to_client) {
                this->d->format_data_request(this->d->server, pkt_data);
            }
        }
        break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            auto pkt_data = chunk.remaining_bytes();
            const bool is_from_remote_session = true;
            send_message_to_client = this->process_format_data_response_pdu(
                flags, chunk, header, is_from_remote_session);

            if (send_message_to_client) {
                this->d->format_data_response(header, this->d->client, flags, pkt_data);
            }
        }
        break;

        case RDPECLIP::CB_CLIP_CAPS: {
            ClipboardCapabilitiesReceive(this->clip_data.server_data, chunk, this->verbose);
            send_message_to_client = !this->proxy_managed;
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto pkt_data = chunk.remaining_bytes();
            send_message_to_client = this->process_filecontents_request_pdu(
                flags, chunk, this->to_server_sender_ptr(),
                this->clip_data.server_data, Direction::FileFromClient);

            if (send_message_to_client) {
                this->d->filecontents_request(this->d->server, flags, pkt_data);
            }
        }
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
}

void ClipboardVirtualChannel::process_client_message(
    uint32_t total_length, uint32_t flags, bytes_view chunk_data)
{
    this->log_process_message(total_length, flags, chunk_data, Direction::FileFromClient);

    InStream chunk(chunk_data);
    RDPECLIP::CliprdrHeader header;
    bool send_message_to_server = true;

    switch (this->process_header_message(
        this->clip_data.client_data, flags, chunk, header, Direction::FileFromClient
    ))
    {
        case RDPECLIP::CB_FORMAT_LIST: {
            auto pkt_data = chunk.remaining_bytes();
            send_message_to_server = this->process_format_list_pdu(
                flags, chunk, header,
                this->to_client_sender_ptr(),
                this->clip_data.client_data,
                this->params.clipboard_down_authorized || this->params.clipboard_up_authorized || this->format_list_response_notifier);


            if (send_message_to_server) {
                this->d->format_list(header, this->d->client, pkt_data);
            }
        }
        break;
        case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            this->d->format_list_response(this->d->server);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            auto pkt_data = chunk.remaining_bytes();
            FormatDataRequestReceive receiver(this->clip_data, this->verbose, chunk);
            if (!this->params.clipboard_down_authorized) {
                LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                        "Server to client Clipboard operation is not allowed.");

                FormatDataRequestSendBack sender(this->to_client_sender_ptr());
            }
            send_message_to_server = this->params.clipboard_down_authorized;

            if (send_message_to_server) {
                this->d->format_data_request(this->d->client, pkt_data);
            }
        }
        break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            auto pkt_data = chunk.remaining_bytes();
            const bool is_from_remote_session = false;
            send_message_to_server = this->process_format_data_response_pdu(
                flags, chunk, header, is_from_remote_session);
            if (send_message_to_server) {
                this->d->format_data_response(header, this->d->server, flags, pkt_data);
            }
        }
        break;

        case RDPECLIP::CB_CLIP_CAPS: {
            ClipboardCapabilitiesReceive(this->clip_data.client_data, chunk, this->verbose);
            send_message_to_server = true;
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto pkt_data = chunk.remaining_bytes();
            send_message_to_server = this->process_filecontents_request_pdu(
                flags, chunk, this->to_client_sender_ptr(),
                this->clip_data.client_data, Direction::FileFromServer);

            if (send_message_to_server) {
                this->d->filecontents_request(this->d->client, flags, pkt_data);
            }
        }
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
}

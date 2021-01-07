/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2020
Author(s): Jonathan Poelen, Christophe Grosjean, Raphael Zhou
*/

#include "acl/auth_api.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"
#include "capture/fdx_capture.hpp"
#include "core/error.hpp"
#include "utils/timebase.hpp"
#include "core/log_id.hpp"
#include "core/RDP/clipboard.hpp"
#include "utils/log.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/unordered_erase.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "utils/timeval_ops.hpp"
#include "utils/translation.hpp"
#include "gdi/osd_api.hpp"

#include <cassert>
#include <cinttypes>

/*

        Client                      Server

 CB_FORMAT_LIST           ->
                          <- CB_LOCK_CLIPDATA (id = 1)
                          <- CB_UNLOCK_CLIPDATA (id = 0)
                          <- CB_FORMAT_LIST_RESPONSE
                          <- CB_FORMAT_DATA_REQUEST
 CB_FORMAT_DATA_RESPONSE  ->

*/

namespace
{
    enum class ClipDirection : bool
    {
        FileFromServer,
        FileFromClient,
    };

    using Direction = ClipDirection;

    constexpr uint32_t send_first_last_flags
        = CHANNELS::CHANNEL_FLAG_FIRST
        | CHANNELS::CHANNEL_FLAG_LAST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    constexpr uint32_t send_first_flags
        = CHANNELS::CHANNEL_FLAG_FIRST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    constexpr uint32_t send_last_flags
        = CHANNELS::CHANNEL_FLAG_LAST
        | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;

    void send_data_response_fail(VirtualChannelDataSender& receiver)
    {
        StaticOutStream<32> out_stream;
        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FORMAT_DATA_RESPONSE,
            RDPECLIP::CB_RESPONSE_FAIL,
            0);
        header.emit(out_stream);
        auto data = out_stream.get_produced_bytes();
        receiver(data.size(), send_first_last_flags, data);
    }
} // anonymous namespace

enum class ClipboardVirtualChannel::ClipCtx::Sig::Status : uint8_t
{
    Update,
    Final,
    Broken,
};

void ClipboardVirtualChannel::ClipCtx::Sig::reset()
{
    this->sha256.init();
    this->status = Status::Update;
}

void ClipboardVirtualChannel::ClipCtx::Sig::update(bytes_view data)
{
    this->sha256.update(data);
}

void ClipboardVirtualChannel::ClipCtx::Sig::final()
{
    assert(this->status == Status::Update);
    this->sha256.final(this->array);
    this->status = Status::Final;
}

void ClipboardVirtualChannel::ClipCtx::Sig::broken()
{
    assert(this->status == Status::Update);
    this->sha256.final(this->array);
    this->status = Status::Broken;
}

bool ClipboardVirtualChannel::ClipCtx::Sig::has_digest() const noexcept
{
    return this->status != Status::Update;
}

auto& ClipboardVirtualChannel::ClipCtx::Sig::digest() const noexcept
{
    assert(this->has_digest());
    return this->array;
}

bytes_view ClipboardVirtualChannel::ClipCtx::Sig::digest_as_av() const noexcept
{
    return make_array_view(this->digest());
}


struct ClipboardVirtualChannel::ClipCtx::FileContentsRange::TflFile
{
    FdxCapture::TflFile tfl_file;

    inline void send(bytes_view data)
    {
        this->tfl_file.trans.send(data);
    }
};

struct ClipboardVirtualChannel::OSD::D
{
    static void osd_prepare_message(ClipboardVirtualChannel& self, std::string const& filename)
    {
        assert(
            &filename == &self.client_ctx.nolock_data.data.file_name
         || &filename == &self.server_ctx.nolock_data.data.file_name);

        if (self.osd.enable_osd) {
            self.osd.msg_type = OSD::MsgType::WaitValidator;
            auto const timer = self.osd.events_guard.get_current_time() + self.osd.delay;
            if (!self.osd.event_ref.reset_timeout(timer)) {
                self.osd.event_ref = self.osd.events_guard.create_event_timeout(
                    "FileVerifOSD",
                    timer,
                    [&self, &filename](Event& event){
                        self.osd.osd_api.display_osd_message(str_concat(
                            TR(trkeys::file_verification_wait, self.osd.lang), filename));
                        event.garbage = true;
                    });
            }
        }
    }

    static void osd_disable_message(ClipboardVirtualChannel& self, std::string_view filename)
    {
        osd_dlp_response(self, filename, false);
    }

    static void osd_dlp_response(
        ClipboardVirtualChannel& self,
        std::string_view filename, bool is_accepted)
    {
        if (self.osd.msg_type == OSD::MsgType::WaitValidator) {
            if (not is_accepted) {
                self.osd.osd_api.display_osd_message(str_concat(
                    TR(trkeys::file_verification_rejected, self.osd.lang), filename));
            }
            else if (!self.osd.event_ref.has_event()) {
                self.osd.osd_api.display_osd_message(str_concat(
                    TR(trkeys::file_verification_accepted, self.osd.lang), filename));
            }
            self.osd.event_ref.garbage();
            self.osd.msg_type = OSD::MsgType::Nothing;
        }
    }
};

enum class ClipboardVirtualChannel::ClipCtx::TransferState : uint8_t
{
    Empty,
    Size,
    Range,
    GetRange,
    SyncRange,
    RequestedRange,
    RequestedSyncRange,
    WaitingContinuationRange,
    WaitingFileValidator,

    Text,
    TextAccumulate,
    WaitingTextValidator,
    RejectedText,
};

struct ClipboardVirtualChannel::ClipCtx::NoLockData::D
{
    static void init_empty(NoLockData& nolock_data, ClipboardVirtualChannel& self)
    {
        assert(!nolock_data.data.tfl_file_ptr);
        nolock_data.transfer_state = TransferState::Empty;
        OSD::D::osd_disable_message(self, nolock_data.data.file_name);
    }

    static void init_text(NoLockData& nolock_data, FileValidatorId file_validator_id)
    {
        assert(nolock_data.transfer_state == TransferState::Empty);
        nolock_data.data.file_validator_id = file_validator_id;
        nolock_data.transfer_state = TransferState::Text;
    }

    static void init_text_validation(NoLockData& nolock_data, FileValidatorId file_validator_id)
    {
        assert(nolock_data.transfer_state == TransferState::Empty);
        nolock_data.data.file_validator_id = file_validator_id;
        nolock_data.transfer_state = TransferState::TextAccumulate;
        nolock_data.data.file_content.clear();
    }

    static void init_size(
        NoLockData& nolock_data,
        StreamId stream_id, FileGroupId lindex)
    {
        assert(nolock_data.transfer_state == TransferState::Empty);
        nolock_data.data.stream_id = stream_id;
        nolock_data.data.lindex = lindex;
        nolock_data.transfer_state = TransferState::Size;
    }

    static void init_requested_range(
        NoLockData& nolock_data,
        StreamId stream_id, FileGroupId lindex,
        uint32_t file_size_requested, uint64_t file_size, std::string_view file_name)
    {
        assert(nolock_data.transfer_state == TransferState::Empty);
        nolock_data.data.stream_id = stream_id;
        nolock_data.data.lindex = lindex;
        nolock_data.data.first_file_size_requested = file_size_requested;
        nolock_data.data.file_size = file_size;
        nolock_data.data.file_name = file_name;
        nolock_data.transfer_state = TransferState::RequestedRange;
    }

private:
    static void _requested_to_rng_base(
        NoLockData& nolock_data,
        FileValidatorId file_validator_id, std::unique_ptr<FileContentsRange::TflFile>&& tfl)
    {
        assert(nolock_data.transfer_state == TransferState::RequestedRange);

        nolock_data.data.file_validator_id = file_validator_id;
        nolock_data.data.file_offset = 0;
        nolock_data.data.file_size_requested = uint32_t(std::min(
            uint64_t(nolock_data.data.first_file_size_requested), nolock_data.data.file_size));
        nolock_data.data.tfl_file_ptr = std::move(tfl);
        nolock_data.data.file_content.clear();
        nolock_data.data.sig.reset();
        nolock_data.data.validator_state = FileContentsRange::ValidatorState::Wait;
    }

public:
    static void requested_range_to_range(
        NoLockData& nolock_data,
        FileValidatorId file_validator_id, std::unique_ptr<FileContentsRange::TflFile>&& tfl)
    {
        _requested_to_rng_base(nolock_data, file_validator_id, std::move(tfl));
        nolock_data.transfer_state = TransferState::Range;
    }

    static void requested_range_to_get_range(
        NoLockData& nolock_data,
        FileValidatorId file_validator_id, std::unique_ptr<FileContentsRange::TflFile>&& tfl)
    {
        _requested_to_rng_base(nolock_data, file_validator_id, std::move(tfl));
        nolock_data.transfer_state = TransferState::GetRange;
    }

    static void set_waiting_continuation_range(NoLockData& nolock_data)
    {
        assert(
            nolock_data.transfer_state == TransferState::Range
         || nolock_data.transfer_state == TransferState::GetRange
         || nolock_data.transfer_state == TransferState::RequestedSyncRange);
        nolock_data.transfer_state = TransferState::WaitingContinuationRange;
    }

    static void set_waiting_validator(NoLockData& nolock_data)
    {
        assert(nolock_data.transfer_state == TransferState::GetRange);
        nolock_data.transfer_state = TransferState::WaitingFileValidator;
    }

    static void set_sync_range(NoLockData& nolock_data)
    {
        assert(
            nolock_data.transfer_state == TransferState::GetRange
         || nolock_data.transfer_state == TransferState::RequestedSyncRange
         || nolock_data.transfer_state == TransferState::WaitingFileValidator);
        nolock_data.transfer_state = TransferState::SyncRange;
    }

    static void set_requested_sync_range(NoLockData& nolock_data)
    {
        assert(nolock_data.transfer_state == TransferState::SyncRange);
        nolock_data.transfer_state = TransferState::RequestedSyncRange;
    }

    static void set_range(NoLockData& nolock_data)
    {
        assert(
            nolock_data.transfer_state == TransferState::WaitingContinuationRange
         || nolock_data.transfer_state == TransferState::SyncRange);
        nolock_data.transfer_state = TransferState::Range;
    }

    static void set_waiting_text_validator(NoLockData& nolock_data)
    {
        assert(nolock_data.transfer_state == TransferState::TextAccumulate);
        nolock_data.transfer_state = TransferState::WaitingTextValidator;
    }

    static void set_rejected_text(NoLockData& nolock_data)
    {
        assert(
            nolock_data.transfer_state == TransferState::Empty
         || nolock_data.transfer_state == TransferState::TextAccumulate);
        nolock_data.transfer_state = TransferState::RejectedText;
    }
};

namespace
{
    constexpr std::size_t file_content_memory_max = 64*1024;
    constexpr std::size_t requested_len_max = 512*1024;
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent
::append(bytes_view data)
{
    assert(0 == this->data_pos);

    if (this->data_len + data.size() <= file_content_memory_max) {
        if (!this->memory_buffer) {
            this->memory_buffer.reset(new uint8_t[file_content_memory_max]); /* NOLINT */
        }
        memcpy(this->memory_buffer.get() + this->data_len, data.data(), data.size());
    }
    else {
        auto write = [this](bytes_view d){
            if (ssize_t(d.size()) != ::write(this->fd.fd(), d.data(), d.size())) {
                const int errnum = errno;
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::FileContent::append: error on temporary file: %s",
                    strerror(errnum));
                this->fd.close();
                throw Error(ERR_UNEXPECTED, errnum);
            }
        };

        if (!this->fd) {
            this->fd.reset(::open(this->tmp_dir, O_EXCL | O_RDWR | O_TMPFILE, 0600));
            if (!this->fd) {
                const int errnum = errno;
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::FileContent::append: error on temporary file: %s",
                    strerror(errnum));
                throw Error(ERR_UNEXPECTED, errnum);
            }

            write({this->memory_buffer.get(), this->data_len});
        }

        write(data);
    }

    this->data_len += data.size();
}

uint8_t* ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent::Buffer
::current() const
{
    return this->buf.get() + this->pos;
}

size_t ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent::Buffer
::remaining() const
{
    return this->capacity - this->pos;
}

bytes_view ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent
::read(std::size_t n)
{
    if (REDEMPTION_UNLIKELY(this->data_pos + n > this->data_len || n > requested_len_max)) {
        LOG(LOG_ERR,
            "ClipboardVirtualChannel::FileContent::read: request too large: %zu", n);
        throw Error(ERR_RDP_DATA_TRUNCATED);
    }

    bytes_view r;

    if (this->data_len <= file_content_memory_max) {
        r = {this->memory_buffer.get() + this->data_pos, n};
    }
    else {
        if (!this->requested_buffer.buf) {
            this->requested_buffer.buf.reset(new uint8_t[requested_len_max]);
        }

        if (this->requested_buffer.remaining() >= n) {
            r = {this->requested_buffer.current(), n};
        }
        else {
            memmove(
                this->requested_buffer.buf.get(),
                this->requested_buffer.current(),
                this->requested_buffer.remaining());
            const auto pos = this->requested_buffer.pos;
            const auto len = std::min(requested_len_max - pos, this->data_len - this->data_pos);
            this->requested_buffer.capacity = pos + len;
            this->requested_buffer.pos = 0;

            if (ssize_t(len) != ::read(this->fd.fd(), this->requested_buffer.current() + pos, len)) {
                const int errnum = errno;
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::FileContent::read: error on temporary file: %s",
                    strerror(errnum));
                this->fd.close();
                throw Error(ERR_UNEXPECTED, errnum);
            }

            r = {this->requested_buffer.current(), n};
        }
        this->requested_buffer.pos += n;
    }

    this->data_pos += n;
    return r;
}

namespace
{
    void seek(unique_fd& fd, uint64_t offset)
    {
        if (-1 == lseek64(fd.fd(), offset, SEEK_SET)) {
            const int errnum = errno;
            LOG(LOG_ERR,
                "ClipboardVirtualChannel::FileContent::set_offset: error on temporary file: %s",
                strerror(errnum));
            fd.close();
            throw Error(ERR_UNEXPECTED, errnum);
        }
    }
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent
::set_offset(uint64_t offset)
{
    if (offset == this->data_pos) {
        return ;
    }

    assert(offset < this->data_pos);

    this->data_pos = offset;

    if (this->data_len > file_content_memory_max) {
        this->requested_buffer.pos = 0;
        this->requested_buffer.capacity = 0;
        seek(this->fd, offset);
    }
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent
::set_read_mode()
{
    assert(0 == this->data_pos);

    if (this->data_len <= file_content_memory_max) {
        return ;
    }

    this->data_pos = 0;
    this->requested_buffer.pos = 0;
    this->requested_buffer.capacity = 0;
    seek(this->fd, 0);
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::FileData::FileContent
::clear()
{
    this->data_len = 0;
    this->data_pos = 0;
    this->requested_buffer.capacity = 0;
    this->requested_buffer.pos = 0;
    this->fd.close();
}

void ClipboardVirtualChannel::ClipCtx::LockedData::clear()
{
    this->sizes.clear();
    this->ranges.clear();
    this->lock_list.clear();
    this->requested_range.disable();
}

ClipboardVirtualChannel::ClipCtx::LockedData::LockedFileList*
ClipboardVirtualChannel::ClipCtx::LockedData::search_lock_by_id(LockId lock_id)
{
    for (auto& r : this->lock_list) {
        if (r.lock_id == lock_id) {
            return &r;
        }
    }
    return nullptr;
}

ClipboardVirtualChannel::ClipCtx::LockedData::LockedRange*
ClipboardVirtualChannel::ClipCtx::LockedData::search_range_by_id(StreamId stream_id)
{
    for (auto& r : this->ranges) {
        if (r.file_contents_range.stream_id == stream_id
         && r.state == LockedRange::State::WaitingResponse
        ) {
            return &r;
        }
    }
    return nullptr;
}

ClipboardVirtualChannel::ClipCtx::LockedData::LockedRange*
ClipboardVirtualChannel::ClipCtx::LockedData::search_range_by_validator_id(FileValidatorId id)
{
    for (auto& r : this->ranges) {
        if (r.file_contents_range.file_validator_id == id) {
            return &r;
        }
    }
    return nullptr;
}

void ClipboardVirtualChannel::ClipCtx::LockedData::remove_locked_file_contents_range(LockedRange* p)
{
    unordered_erase_element(this->ranges, p);
}

ClipboardVirtualChannel::ClipCtx::LockedData::LockedRange*
ClipboardVirtualChannel::ClipCtx::LockedData::search_range_by_offset(LockId lock_id, FileGroupId ifile, uint64_t offset)
{
    for (auto& r : this->ranges) {
        if (r.lock_id == lock_id
         && r.state == LockedRange::State::WaitingRequest
         && r.file_contents_range.lindex == ifile
         && r.file_contents_range.file_offset == offset
        ) {
            return &r;
        }
    }
    return nullptr;
}

ClipboardVirtualChannel::ClipCtx::LockedData::LockedSize*
ClipboardVirtualChannel::ClipCtx::LockedData::search_size_by_id(StreamId stream_id)
{
    for (auto& r : this->sizes) {
        if (r.file_contents_size.stream_id == stream_id) {
            return &r;
        }
    }
    return nullptr;
}

void ClipboardVirtualChannel::ClipCtx::LockedData::remove_locked_file_contents_size(LockedSize* p)
{
    unordered_erase_element(this->sizes, p);
}

bool ClipboardVirtualChannel::ClipCtx::LockedData::contains_stream_id(StreamId stream_id)
{
    return this->search_range_by_id(stream_id)
        || this->search_size_by_id(stream_id);
}


enum ClipboardVirtualChannel::ClipCtx::OptionalLockId::State : uint8_t
{
    Disabled    = 0b000,
    Enabled     = 0b001,
    HasLock     = 0b011/*bug gcc-9 | Enabled*/,
    // lock_id is pushed to lock_list (avoid duplication)
    LockIsUsed  = 0b111/*bug gcc-9 | HasLock*/,
};

void ClipboardVirtualChannel::ClipCtx::OptionalLockId::disable()
{
    this->_state = Disabled;
}

void ClipboardVirtualChannel::ClipCtx::OptionalLockId::enable(bool activate)
{
    this->_state = activate ? Enabled : Disabled;
}

[[nodiscard]]
bool ClipboardVirtualChannel::ClipCtx::OptionalLockId::is_enabled() const
{
    return bool(this->_state & State::Enabled);
}

[[nodiscard]]
bool ClipboardVirtualChannel::ClipCtx::OptionalLockId::has_lock() const
{
    return State(this->_state & State::HasLock) == State::HasLock;
}

[[nodiscard]]
bool ClipboardVirtualChannel::ClipCtx::OptionalLockId::has_unused_lock() const
{
    return this->_state == State::HasLock;
}

void ClipboardVirtualChannel::ClipCtx::OptionalLockId::set_used()
{
    assert(this->_state == HasLock);
    this->_state = State::LockIsUsed;
}

void ClipboardVirtualChannel::ClipCtx::OptionalLockId::set_lock_id(LockId id)
{
    assert(bool(this->_state));
    this->_lock_id = id;
    this->_state = HasLock;
}

void ClipboardVirtualChannel::ClipCtx::OptionalLockId::unset_lock_id()
{
    this->_state = State(this->_state & State::Enabled);
}

[[nodiscard]]
ClipboardVirtualChannel::LockId
ClipboardVirtualChannel::ClipCtx::OptionalLockId::lock_id() const
{
    assert(bool(this->_state));
    return this->_lock_id;
}


ClipboardVirtualChannel::ClipCtx::ClipCtx(
    std::string const& target_name,
    bool verify_file_before_transfer,
    bool verify_text_before_transfer,
    uint64_t max_file_size_rejected,
    char const* tmp_dir)
: verify_file_before_transfer(verify_file_before_transfer && not target_name.empty())
, verify_text_before_transfer(verify_text_before_transfer && not target_name.empty())
, max_file_size_rejected(max_file_size_rejected)
, validator_target_name(target_name)
{
    static_assert(RDPECLIP::FileDescriptor::size()
        == decltype(this->file_descriptor_stream)::original_capacity());
    this->nolock_data.data.file_content.tmp_dir = tmp_dir;
}

void ClipboardVirtualChannel::ClipCtx::clear()
{
    assert(this->nolock_data == TransferState::Empty);
    this->message_type = 0;
    this->has_current_file_contents_stream_id = false;
    this->clip_text_locale_identifier = 0;
    this->optional_lock_id.disable();
    this->current_format_list.clear();
    this->files.clear();
    this->file_descriptor_stream.rewind();
    this->locked_data.clear();
}


struct ClipboardVirtualChannel::FileValidatorDataList
{
    FileValidatorId file_validator_id;
    Direction direction;
    std::unique_ptr<ClipCtx::FileContentsRange::TflFile> tfl_file_ptr;
    std::string file_name;
    ClipCtx::Sig sig;
};

struct ClipboardVirtualChannel::TextValidatorDataList
{
    FileValidatorId file_validator_id;
    Direction direction;
};

ClipboardVirtualChannel::FileValidatorDataList*
ClipboardVirtualChannel::search_file_validator_by_id(FileValidatorId id)
{
    for (auto& r : this->file_validator_list) {
        if (r.file_validator_id == id) {
            return &r;
        }
    }
    return nullptr;
}

void ClipboardVirtualChannel::remove_file_validator(FileValidatorDataList* p)
{
    unordered_erase_element(this->file_validator_list, p);
}

ClipboardVirtualChannel::TextValidatorDataList*
ClipboardVirtualChannel::search_text_validator_by_id(FileValidatorId id)
{
    for (auto& r : this->text_validator_list) {
        if (r.file_validator_id == id) {
            return &r;
        }
    }
    return nullptr;
}

void ClipboardVirtualChannel::remove_text_validator(TextValidatorDataList* p)
{
    unordered_erase_element(this->text_validator_list, p);
}


namespace
{
    chars_view to_dlpav_str_direction(Direction direction)
    {
        return (direction == Direction::FileFromClient)
            ? "UP"_av
            : "DOWN"_av;
    }

    void dlpav_report_text(
        FileValidatorId file_validator_id,
        SessionLogApi& session_log,
        Direction direction,
        std::string_view result_content)
    {
        char buf[24];
        unsigned n = std::snprintf(buf, std::size(buf), "%" PRIu32,
            underlying_cast(file_validator_id));
        session_log.log6(LogId::TEXT_VERIFICATION, {
            KVLog("direction"_av, to_dlpav_str_direction(direction)),
            KVLog("copy_id"_av, {buf, n}),
            KVLog("status"_av, result_content),
        });
    }

    void dlpav_report_file(
        std::string_view file_name,
        SessionLogApi& session_log,
        Direction direction,
        std::string_view result_content)
    {
        session_log.log6(LogId::FILE_VERIFICATION, {
            KVLog("direction"_av, to_dlpav_str_direction(direction)),
            KVLog("file_name"_av, file_name),
            KVLog("status"_av, result_content),
        });
    }

    [[nodiscard]]
    uint16_t process_header_message(
        uint16_t current_message_type, uint32_t total_length, uint32_t flags, InStream& chunk,
        RDPECLIP::CliprdrHeader& header, Direction direction, RDPVerbose verbose)
    {
        char const* funcname = (direction == Direction::FileFromClient)
            ? "ClipboardVirtualChannel::process_client_message"
            : "ClipboardVirtualChannel::process_server_message";

        auto dump = [&]{
            if (bool(verbose & RDPVerbose::cliprdr_dump)) {
                const bool send              = false;
                const bool from_or_to_client = (direction == Direction::FileFromClient);
                ::msgdump_c(
                    send, from_or_to_client, total_length,
                    flags, chunk.remaining_bytes());
            }
        };

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            /* msgType(2) + msgFlags(2) + dataLen(4) */
            if (!chunk.in_check_rem(8)) {
                LOG(LOG_INFO, "%s: total_length=%u flags=0x%08X chunk_data_length=%zu",
                    funcname, total_length, flags, chunk.in_remain());

                dump();

                LOG(LOG_ERR, "Truncated %s: expected=8 remains=%zu",
                    funcname, chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            header.recv(chunk);
            current_message_type = header.msgType();
        }

        if (bool(verbose & RDPVerbose::cliprdr)) {
            const auto first_last = CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST;
            LOG(LOG_INFO, "%s: total_length=%u flags=0x%08X chunk_data_length=%zu %s (%u)%s%s",
                funcname, total_length, flags, chunk.in_remain(),
                RDPECLIP::get_msgType_name(current_message_type),
                current_message_type,
                ((flags & first_last) == first_last) ? " FIRST|LAST"
                : (flags & CHANNELS::CHANNEL_FLAG_FIRST) ? " FIRST"
                : (flags & CHANNELS::CHANNEL_FLAG_LAST) ? " LAST"
                : "",
                not (flags & CHANNELS::CHANNEL_FLAG_FIRST) ? ""
                : (header.msgFlags() == RDPECLIP::CB_RESPONSE_OK) ? " RESPONSE_OK"
                : (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) ? " RESPONSE_FAIL"
                : ""
            );
        }

        dump();

        return current_message_type;
    }

    bool check_header_response(RDPECLIP::CliprdrHeader const& in_header, uint32_t flags)
    {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            auto mask = RDPECLIP::CB_RESPONSE_FAIL | RDPECLIP::CB_RESPONSE_OK;
            if ((in_header.msgFlags() & mask) != RDPECLIP::CB_RESPONSE_OK) {
                return false;
            }
        }

        return true;
    }

    constexpr uint32_t channel_chunk_length = CHANNELS::CHANNEL_CHUNK_LENGTH;
    const char zeros_buf[channel_chunk_length] {};

    constexpr uint32_t size_of_cliprdr_header_with_stream_id
        = RDPECLIP::CliprdrHeader::size() + 4 /* streamId */;

    void filecontents_response_zerodata(
        VirtualChannelDataSender& sender, uint32_t response_size)
    {
        uint32_t offset = 0;
        uint32_t remaining = response_size - size_of_cliprdr_header_with_stream_id;
        constexpr auto max_len = channel_chunk_length;
        for (; remaining > max_len; remaining -= max_len, offset += max_len) {
            sender(response_size, 0, bytes_view(zeros_buf, max_len));
        }
        sender(response_size, send_last_flags, bytes_view(zeros_buf, remaining));
    }
}

struct ClipboardVirtualChannel::ClipCtx::D
{
    using Self = D;

    static void filecontents_response_data(
        VirtualChannelDataSender& sender,
        uint32_t response_size,
        NoLockData::FileData::FileContent& file_content)
    {
        assert(response_size - size_of_cliprdr_header_with_stream_id
            <= file_content.size());

        uint32_t remaining = response_size - size_of_cliprdr_header_with_stream_id;
        constexpr uint32_t max_len = channel_chunk_length;
        for (; remaining > max_len; remaining -= max_len) {
            sender(response_size, 0, file_content.read(max_len));
        }
        sender(response_size, send_last_flags, file_content.read(remaining));
    }

    static void send_text_and_reset(
        VirtualChannelDataSender& sender, uint32_t response_size,
        NoLockData::FileData::FileContent& file_content,
        bool is_last)
    {
        file_content.set_read_mode();

        uint64_t remaining = file_content.size();
        constexpr uint64_t max_len = channel_chunk_length;

        auto data = file_content.read(std::min(max_len, remaining));

        if (remaining <= max_len) {
            sender(response_size, is_last ? send_first_last_flags : send_first_flags, data);
        }
        else {
            sender(response_size, send_first_flags, data);
            remaining -= max_len;
            for (; remaining > max_len; remaining -= max_len) {
                sender(response_size, 0, file_content.read(max_len));
            }
            sender(response_size, is_last ? send_last_flags : 0, file_content.read(remaining));
        }

        file_content.clear();
    }

    template<std::size_t N>
    static uint32_t send_response_datas_with_lengths_and_last_flag(
        VirtualChannelDataSender& sender,
        StreamId stream_id,
        bytes_view const (&datas)[N],
        uint32_t total_packet_size,
        uint32_t data_len,
        uint32_t last_flag)
    {
        assert(total_packet_size >= data_len);

        StaticOutStream<channel_chunk_length> out_stream;
        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FILECONTENTS_RESPONSE,
            RDPECLIP::CB_RESPONSE_OK,
            data_len);
        header.emit(out_stream);

        out_stream.out_uint32_le(safe_int(stream_id));

        auto flags = send_first_flags;

        for (bytes_view data : datas) {
            if (data.size() > out_stream.tailroom()) {
                auto chunk_size = out_stream.tailroom();
                out_stream.out_copy_bytes(data.first(chunk_size));
                sender(total_packet_size, flags, out_stream.get_produced_bytes());

                flags = 0;
                out_stream.rewind();
                data = data.drop_front(chunk_size);

                while (data.size() > channel_chunk_length) {
                    sender(total_packet_size, flags, data.first(channel_chunk_length));
                    data = data.drop_front(channel_chunk_length);
                }
            }

            out_stream.out_copy_bytes(data);
        }

        sender(total_packet_size, flags | last_flag, out_stream.get_produced_bytes());

        return total_packet_size;
    }

    template<std::size_t N>
    static uint32_t send_response_datas(
        VirtualChannelDataSender& sender,
        StreamId stream_id,
        bytes_view const (&datas)[N])
    {
        uint32_t packet_len = /*streamId=*/4;
        for (bytes_view const& data : datas) {
            packet_len = checked_int(packet_len + data.size());
        }

        uint32_t total_packet_size
            = checked_int(packet_len + RDPECLIP::CliprdrHeader::size());

        return send_response_datas_with_lengths_and_last_flag(
            sender, stream_id, datas,
            total_packet_size, packet_len,
            CHANNELS::CHANNEL_FLAG_LAST);
    }

    static void send_filecontents_range(
        VirtualChannelDataSender& sender,
        StreamId stream_id,
        FileGroupId lindex,
        uint64_t offset,
        uint32_t requested_len)
    {
        StaticOutStream<128> out_stream;

        RDPECLIP::FileContentsRequestPDU new_request_pdu(
            (safe_int(stream_id)), // extra parent to fix gcc-9.1
            safe_int(lindex),
            RDPECLIP::FILECONTENTS_RANGE,
            uint32_t(offset),
            uint32_t(offset >> 32),
            requested_len,
            0, false);

        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FILECONTENTS_REQUEST,
            RDPECLIP::CB_RESPONSE_OK,
            new_request_pdu.size());

        header.emit(out_stream);
        new_request_pdu.emit(out_stream);

        sender(
            out_stream.get_offset(),
            send_first_last_flags,
            out_stream.get_produced_bytes());
    }

    [[nodiscard]]
    static uint32_t send_filecontents_response_header(
        VirtualChannelDataSender& sender,
        StreamId stream_id,
        uint32_t data_len)
    {
        StaticOutStream<128> out_stream;
        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FILECONTENTS_RESPONSE,
            RDPECLIP::CB_RESPONSE_OK,
            data_len + 4u /* streamId */);
        header.emit(out_stream);
        out_stream.out_uint32_le(safe_int(stream_id));

        uint32_t response_size = checked_int(data_len + out_stream.get_offset());
        sender(response_size, send_first_flags, out_stream.get_produced_bytes());

        return response_size;
    }

    static Direction to_direction(ClipboardVirtualChannel& self, ClipCtx const& clip)
    {
        return (&clip == &self.server_ctx)
            ? Direction::FileFromServer
            : Direction::FileFromClient;
    }

    static VirtualChannelDataSender& get_sender(
        ClipboardVirtualChannel& self, ClipCtx const& clip)
    {
        return (&clip == &self.server_ctx)
            ? *self.to_client_sender_ptr()
            : *self.to_server_sender_ptr();
    }

    static void _close_file_rng_tfl(
        ClipboardVirtualChannel& self,
        ClipCtx::FileContentsRange& file_rng,
        Mwrm3::TransferedStatus transfered_status)
    {
        using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;

        if (file_rng.tfl_file_ptr) {
            if (self.always_file_storage
             || ValidatorState::Failure == file_rng.validator_state
            ) {
                self.fdx_capture->close_tfl(
                    file_rng.tfl_file_ptr->tfl_file,
                    file_rng.file_name,
                    transfered_status,
                    Mwrm3::Sha256Signature{file_rng.sig.digest_as_av()});
            }
            else {
                self.fdx_capture->cancel_tfl(file_rng.tfl_file_ptr->tfl_file);
            }
            file_rng.tfl_file_ptr.reset();
        }
    }

    static void _close_file_rng(
        ClipboardVirtualChannel& self,
        // clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng,
        Mwrm3::TransferedStatus transfered_status)
    {
        if (bool(file_rng.file_validator_id)) {
            self.file_validator->send_eof(file_rng.file_validator_id);
            self.file_validator_list.push_back({
                file_rng.file_validator_id,
                Self::to_direction(self, clip),
                std::move(file_rng.tfl_file_ptr),
                file_rng.file_name,
                file_rng.sig
            });
            file_rng.file_validator_id = FileValidatorId();
        }
        else {
            Self::_close_file_rng_tfl(self, file_rng, transfered_status);
        }
    }

    static void broken_file_transfer(
        ClipboardVirtualChannel& self,
        // clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng)
    {
        file_rng.sig.broken();
        Self::_close_file_rng_tfl(self, file_rng, Mwrm3::TransferedStatus::Broken);

        if (bool(file_rng.file_validator_id)) {
            self.file_validator->send_eof(file_rng.file_validator_id);
            self.file_validator_list.push_back({
                file_rng.file_validator_id,
                Self::to_direction(self, clip),
                nullptr,
                file_rng.file_name,
                file_rng.sig
            });
            file_rng.file_validator_id = FileValidatorId();
        }
    }

    static void stop_validation_before_transfer(
        ClipboardVirtualChannel& self,
        // clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng)
    {
        Self::log_file_info(self, file_rng, (&clip == &self.server_ctx));

        if (bool(file_rng.file_validator_id)) {
            self.file_validator->send_abort(file_rng.file_validator_id);
            file_rng.file_validator_id = FileValidatorId();
        }

        Self::_close_file_rng_tfl(self, file_rng, Mwrm3::TransferedStatus::Broken);
    }

    static void stop_wait_validation(
        ClipboardVirtualChannel& self,
        // clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng)
    {
        assert(bool(file_rng.file_validator_id));

        self.file_validator->send_abort(file_rng.file_validator_id);
        file_rng.file_validator_id = FileValidatorId();

        Self::stop_valid_transfer(self, clip, file_rng);
    }

    static void stop_valid_transfer(
        ClipboardVirtualChannel& self,
        // clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng)
    {
        assert(!bool(file_rng.file_validator_id));

        if (file_rng.file_offset < file_rng.file_size) {
            Self::log_file_info(self, file_rng, (&clip == &self.server_ctx));
        }

        switch (file_rng.sig) {
            case ClipCtx::Sig::Status::Update:
                file_rng.sig.broken();
                [[fallthrough]];

            case ClipCtx::Sig::Status::Broken:
                Self::_close_file_rng_tfl(self, file_rng, Mwrm3::TransferedStatus::Broken);
                break;

            case ClipCtx::Sig::Status::Final:
                Self::_close_file_rng_tfl(self, file_rng, Mwrm3::TransferedStatus::Completed);
                break;
        }
    }

    static void finalize_text_transfer(
        ClipboardVirtualChannel& self, ClipCtx& clip, FileValidatorId file_validator_id)
    {
        if (bool(file_validator_id)) {
            self.file_validator->send_eof(file_validator_id);
            self.text_validator_list.push_back({
                file_validator_id,
                Self::to_direction(self, clip),
            });
        }
    }

    static void stop_nolock_data(
        ClipboardVirtualChannel& self, ClipCtx& clip,
        VirtualChannelDataSender* receiver)
    {
        auto& file_rng = clip.nolock_data.data;

        switch (clip.nolock_data)
        {
            case ClipCtx::TransferState::WaitingContinuationRange:
            case ClipCtx::TransferState::Range:
                Self::broken_file_transfer(self, clip, file_rng);
                break;

            case ClipCtx::TransferState::GetRange:
                if (receiver) {
                    filecontents_response_zerodata(*receiver, file_rng.response_size);
                }
                Self::stop_validation_before_transfer(self, clip, file_rng);
                break;

            case ClipCtx::TransferState::WaitingFileValidator:
                if (receiver) {
                    filecontents_response_zerodata(*receiver, file_rng.response_size);
                }
                Self::stop_wait_validation(self, clip, file_rng);
                break;

            case ClipCtx::TransferState::RequestedSyncRange:
            case ClipCtx::TransferState::SyncRange:
                Self::stop_valid_transfer(self, clip, file_rng);
                break;

            case ClipCtx::TransferState::Text:
                Self::finalize_text_transfer(
                    self, clip, file_rng.file_validator_id);
                break;

            case ClipCtx::TransferState::WaitingTextValidator:
            case ClipCtx::TransferState::TextAccumulate:
                self.file_validator->send_abort(file_rng.file_validator_id);
                file_rng.file_validator_id = FileValidatorId();
                [[fallthrough]];
            case ClipCtx::TransferState::RejectedText:
                if (receiver) {
                    send_data_response_fail(*receiver);
                }
                break;

            case ClipCtx::TransferState::Size:
            case ClipCtx::TransferState::RequestedRange:
            case ClipCtx::TransferState::Empty:
                break;
        }

        NoLockData::D::init_empty(clip.nolock_data, self);
    }

    static void reset_clip(
        ClipboardVirtualChannel& self, ClipCtx& clip,
        VirtualChannelDataSender* sender)
    {
        self.can_lock = false;
        Self::stop_nolock_data(self, clip, sender);
        for (ClipCtx::LockedData::LockedRange& locked_rng : clip.locked_data.ranges) {
            Self::broken_file_transfer(self, clip, locked_rng.file_contents_range);
        }
        clip.clear();
    }

    static bool clip_caps(
        ClipboardVirtualChannel& self, ClipCtx& clip,
        bytes_view chunk_data, char const* name,
        VirtualChannelDataSender* sender)
    {
        Self::reset_clip(self, clip, sender);

        auto general_flags = RDPECLIP::extract_clipboard_general_flags_capability(
            chunk_data, bool(self.verbose & RDPVerbose::cliprdr));

        bool verify_file_before_transfer
            = self.client_ctx.verify_file_before_transfer || self.server_ctx.verify_file_before_transfer;
        if (verify_file_before_transfer) {
            general_flags &= ~RDPECLIP::CB_CAN_LOCK_CLIPDATA;
            ServerMonitorReadySendBack::send_capabilities(
                name, self.verbose, general_flags, sender);
        }

        clip.optional_lock_id.enable(bool(general_flags & RDPECLIP::CB_CAN_LOCK_CLIPDATA));
        clip.use_long_format_names = bool(general_flags & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);

        return not verify_file_before_transfer;
    }

    static void monitor_ready(
        ClipboardVirtualChannel& self, ClipCtx& clip,
        VirtualChannelDataSender* sender)
    {
        bool has_lock_support = self.can_lock;
        Self::reset_clip(self, clip, sender);
        clip.optional_lock_id.enable(has_lock_support);
    }

    static bool format_list(
        ClipboardVirtualChannel& self,
        uint32_t flags, RDPECLIP::CliprdrHeader const& in_header,
        VirtualChannelDataSender* sender, VirtualChannelDataSender* receiver,
        bool clip_enabled, ClipCtx& clip, bytes_view chunk_data)
    {
        // TODO fix that
        if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
            LOG(LOG_ERR, "Format List PDU is not yet supported!");
            format_list_send_back(sender);
            return false;
        }

        clip.current_format_list.clear();
        clip.current_file_list_format_id = 0;
        clip.optional_lock_id.unset_lock_id();
        clip.files.clear();

        if (!clip_enabled) {
            LOG(LOG_WARNING, "Clipboard is fully disabled.");
            format_list_send_back(sender);
            return false;
        }

        Self::stop_nolock_data(self, clip, receiver);

        InStream in_stream(chunk_data);
        FormatListReceive format_list_receive(
            clip.use_long_format_names,
            in_header,
            in_stream,
            clip.current_format_list,
            self.verbose);

        clip.current_file_list_format_id = format_list_receive.file_list_format_id;

        return true;
    }

    static void format_list_response(
        RDPECLIP::CliprdrHeader const& in_header, uint32_t flags, ClipCtx& clip)
    {
        if (not check_header_response(in_header, flags)) {
            clip.current_file_list_format_id = 0;
            return ;
        }
    }

    static void lock(ClipboardVirtualChannel& self, ClipCtx& clip, bytes_view chunk_data)
    {
        RDPECLIP::LockClipboardDataPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);

        if (bool(self.verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }

        if (not self.can_lock) {
            return ;
        }

        clip.optional_lock_id.set_lock_id(LockId(pdu.clipDataId));
    }

    static void unlock(ClipboardVirtualChannel& self, ClipCtx& clip, bytes_view chunk_data)
    {
        RDPECLIP::UnlockClipboardDataPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);

        if (bool(self.verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }

        if (not self.can_lock) {
            return ;
        }

        auto lock_id = LockId(pdu.clipDataId);

        if (clip.optional_lock_id.has_lock() && clip.optional_lock_id.lock_id() == lock_id) {
            clip.optional_lock_id.unset_lock_id();
        }
        else {
            auto p = std::find_if(
                clip.locked_data.lock_list.begin(), clip.locked_data.lock_list.end(),
                [&](ClipCtx::LockedData::LockedFileList const& l){
                    return l.lock_id == lock_id;
                });

            if (p != clip.locked_data.lock_list.end()) {
                unordered_erase_element(clip.locked_data.lock_list, &*p);
            }

            unordered_erase_if(
                clip.locked_data.sizes,
                [lock_id](ClipCtx::LockedData::LockedSize const& size){
                    return size.lock_id == lock_id;
                });

            unordered_erase_if(
                clip.locked_data.ranges,
                [&](ClipCtx::LockedData::LockedRange& rng){
                    if (rng.lock_id == lock_id) {
                        Self::broken_file_transfer(self, clip, rng.file_contents_range);
                        return true;
                    }
                    return false;
                });
        }

        if (clip.locked_data.requested_range.lock_id == lock_id) {
            clip.locked_data.requested_range.disable();
        }
    }

    static void format_data_request(ClipCtx& clip, bytes_view chunk_data, RDPVerbose verbose)
    {
        RDPECLIP::FormatDataRequestPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);
        clip.requested_format_id = pdu.requestedFormatId;

        if (bool(verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }

    static bool format_data_response(
        ClipboardVirtualChannel& self,
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data,
        uint32_t total_length, VirtualChannelDataSender& receiver)
    {
        if (not check_header_response(in_header, flags)) {
            clip.requested_format_id = 0;
            return true;
        }

        bool is_client_to_server = (&clip == &self.client_ctx);

        auto& current_format_list = is_client_to_server
            ? self.client_ctx.current_format_list
            : self.server_ctx.current_format_list;

        auto reset_nolock_data = [&self, &clip, &receiver]{
            auto& file_rng = clip.nolock_data.data;

            switch (clip.nolock_data)
            {
                case ClipCtx::TransferState::WaitingFileValidator:
                    Self::stop_wait_validation(self, clip, file_rng);
                    break;

                case ClipCtx::TransferState::SyncRange:
                    Self::stop_valid_transfer(self, clip, file_rng);
                    break;

                case ClipCtx::TransferState::WaitingContinuationRange:
                    Self::broken_file_transfer(self, clip, file_rng);
                    break;

                case ClipCtx::TransferState::WaitingTextValidator:
                    self.file_validator->send_abort(file_rng.file_validator_id);
                    file_rng.file_validator_id = FileValidatorId();
                    [[fallthrough]];
                case ClipCtx::TransferState::RejectedText:
                    send_data_response_fail(receiver);
                    break;

                case ClipCtx::TransferState::Empty:
                    return;

                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::GetRange:
                case ClipCtx::TransferState::RequestedRange:
                case ClipCtx::TransferState::RequestedSyncRange:
                case ClipCtx::TransferState::Text:
                case ClipCtx::TransferState::TextAccumulate:
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_format_data_response_pdu:"
                        " invalid state %d", int(clip.nolock_data.transfer_state));
                    throw Error(ERR_RDP_PROTOCOL);
            }

            NoLockData::D::init_empty(clip.nolock_data, self);
        };

        if (clip.current_file_list_format_id
         && clip.current_file_list_format_id == clip.requested_format_id
        ) {
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                reset_nolock_data();
                clip.files.clear();
            }

            InStream in_stream(chunk_data);
            FormatDataResponseReceiveFileList receiver(
                clip.files,
                in_stream,
                in_header,
                self.params.dont_log_data_into_syslog,
                clip.current_file_list_format_id,
                flags,
                clip.file_descriptor_stream,
                self.verbose,
                is_client_to_server ? "client to server" : "server to client"
            );

            if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)
             && clip.optional_lock_id.has_unused_lock()
            ) {
                clip.locked_data.lock_list.push_back(ClipCtx::LockedData::LockedFileList{
                    clip.optional_lock_id.lock_id(), std::move(clip.files)});
                clip.optional_lock_id.set_used();
            }

            Self::log_siem_info(
                self, current_format_list, flags, in_header, true, clip.requested_format_id,
                bytes_view{}, is_client_to_server);
        }
        else if (!clip.validator_target_name.empty()
            && (is_client_to_server
                ? self.params.validator_params.enable_clipboard_text_up
                : self.params.validator_params.enable_clipboard_text_down
            )
        ) {
            switch (clip.requested_format_id) {
                case RDPECLIP::CF_TEXT:
                case RDPECLIP::CF_UNICODETEXT: {
                    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                        reset_nolock_data();

                        if (clip.verify_text_before_transfer
                         && in_header.dataLen() > clip.max_file_size_rejected
                        ) {
                            LOG(LOG_WARNING,
                                "ClipboardVirtualChannel::process_format_data_response:"
                                    " text too big are automatically rejected");
                            NoLockData::D::set_rejected_text(clip.nolock_data);
                        }
                        else {
                            const auto validator_id = (RDPECLIP::CF_TEXT == clip.requested_format_id)
                                ? self.file_validator->open_text(clip.clip_text_locale_identifier,
                                                                 clip.validator_target_name)
                                : self.file_validator->open_unicode(clip.validator_target_name);

                            if (clip.verify_text_before_transfer) {
                                clip.nolock_data.data.file_name = "clipboard text";
                                OSD::D::osd_prepare_message(self, clip.nolock_data.data.file_name);
                                NoLockData::D::init_text_validation(clip.nolock_data, validator_id);
                                StaticOutStream<32> out_stream;
                                in_header.emit(out_stream);
                                clip.nolock_data.data.response_size = total_length;
                                clip.nolock_data.data.file_content
                                    .append(out_stream.get_produced_bytes());
                            }
                            else {
                                NoLockData::D::init_text(clip.nolock_data, validator_id);
                            }
                        }

                        Self::log_siem_info(
                            self, current_format_list, flags, in_header, false,
                            clip.requested_format_id, chunk_data, is_client_to_server);
                    }

                    switch (clip.nolock_data) {
                        case ClipCtx::TransferState::Text: {
                            self.file_validator->send_data(
                                clip.nolock_data.data.file_validator_id, chunk_data);

                            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                                Self::finalize_text_transfer(
                                    self, clip, clip.nolock_data.data.file_validator_id);
                                NoLockData::D::init_empty(clip.nolock_data, self);
                            }
                            break;
                        }

                        case ClipCtx::TransferState::TextAccumulate: {
                            self.file_validator->send_data(
                                clip.nolock_data.data.file_validator_id, chunk_data);
                            clip.nolock_data.data.file_content.append(chunk_data);

                            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                                clip.requested_format_id = 0;
                                self.file_validator->send_eof(
                                    clip.nolock_data.data.file_validator_id);
                                NoLockData::D::set_waiting_text_validator(clip.nolock_data);
                            }

                            return false;
                        }

                        case ClipCtx::TransferState::RejectedText: {
                            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                                clip.requested_format_id = 0;
                                NoLockData::D::init_empty(clip.nolock_data, self);
                                send_data_response_fail(receiver);
                            }

                            return false;
                        }

                        case ClipCtx::TransferState::Empty:
                            break;

                        default:
                            assert(false);
                    }

                    break;
                }
                case RDPECLIP::CF_LOCALE: {
                    if (flags & CHANNELS::CHANNEL_FLAG_LAST && chunk_data.size() >= 4) {
                        Self::log_siem_info(
                            self, current_format_list, flags, in_header, false,
                            clip.requested_format_id, chunk_data, is_client_to_server);
                        InStream in_stream(chunk_data);
                        clip.clip_text_locale_identifier = in_stream.in_uint32_le();
                    }
                    else {
                        LOG(LOG_ERR, "ClipboardVirtualChannel::format_data_response:"
                            " CF_LOCALE invalid format");
                        throw Error(ERR_RDP_PROTOCOL);
                    }
                    break;
                }
                default:
                    Self::log_siem_info(
                        self, current_format_list, flags, in_header, false,
                        clip.requested_format_id, chunk_data, is_client_to_server);
                    break;
            }
        }
        else {
            Self::log_siem_info(
                self, current_format_list, flags, in_header, false, clip.requested_format_id,
                chunk_data, is_client_to_server);
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            clip.requested_format_id = 0;
        }

        return true;
    }

    static bool filecontents_request(
        ClipboardVirtualChannel& self,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data,
        VirtualChannelDataSender& sender,
        VirtualChannelDataSender& receiver)
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

        auto send_error = [&]{
            FilecontentsRequestSendBack(
                file_contents_request_pdu.dwFlags(),
                file_contents_request_pdu.streamId(),
                &sender);
            return false;
        };

        if (!self.params.clipboard_file_authorized) {
            return send_error();
        }

        const auto lindex = file_contents_request_pdu.lindex();
        const auto stream_id = StreamId(file_contents_request_pdu.streamId());
        const auto ifile = FileGroupId(lindex);

        auto update_continuation_range = [&](ClipCtx::FileContentsRange& file_rng) {
            file_rng.stream_id = stream_id;
            file_rng.file_size_requested = std::min(
                file_rng.file_size - file_rng.file_offset,
                uint64_t(file_contents_request_pdu.cbRequested()));
        };

        auto check_valid_lindex = [&](std::vector< CliprdFileInfo > const &files){
            if (lindex < files.size()) {
                return true;
            }

            LOG(LOG_WARNING,
                "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " Invalid lindex %u", lindex);
            return send_error();
        };

        // ignore lock if don't have CB_CAN_LOCK_CLIPDATA
        if (not self.can_lock) {
            auto init_contents_size = [&]{
                if (check_valid_lindex(clip.files)) {
                    NoLockData::D::init_size(clip.nolock_data, stream_id, ifile);
                    return true;
                }
                return send_error();
            };

            auto init_contents_range = [&]{
                if (check_valid_lindex(clip.files)) {
                    if (file_contents_request_pdu.position() != 0) {
                        LOG(LOG_WARNING,
                            "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                " Unsupported random access for a FILECONTENTS_RANGE");
                        return send_error();
                    }

                    if (file_contents_request_pdu.cbRequested() > clip.max_file_size_rejected) {
                        LOG(LOG_WARNING,
                            "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                " file too big are automatically rejected");
                        return send_error();
                    }

                    NoLockData::D::init_requested_range(
                        clip.nolock_data,
                        stream_id,
                        ifile,
                        file_contents_request_pdu.cbRequested(),
                        clip.files[lindex].file_size,
                        clip.files[lindex].file_name);
                    return true;
                }

                return send_error();
            };

            NoLockData::FileData& file_rng = clip.nolock_data.data;

            switch (clip.nolock_data)
            {
                case ClipCtx::TransferState::SyncRange: {
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        if (file_rng.lindex != ifile) {
                            Self::stop_valid_transfer(self, clip, file_rng);
                            NoLockData::D::init_empty(clip.nolock_data, self);
                            return init_contents_range();
                        }

                        auto& file_contents = file_rng.file_content;

                        assert(file_rng.file_offset == file_contents.size());
                        assert(file_rng.file_offset >= file_contents.offset());

                        const uint64_t req_offset = file_contents_request_pdu.position();
                        const uint64_t req_len = file_contents_request_pdu.cbRequested();
                        const uint64_t req_end_offset = req_offset + req_len;

                        if (req_offset <= file_contents.offset()) {
                            file_contents.set_offset(req_offset);

                            // has the requested buffer
                            if (file_rng.is_finalized() || req_end_offset < file_contents.size()) {
                                auto len = std::min(file_contents.size() - req_offset, req_len);
                                using Views = bytes_view[];
                                send_response_datas(
                                    sender, stream_id,
                                    Views{ file_contents.read(len) });

                                return false;
                            }

                            // request after buffer (validator ok before receiving the whole file)
                            if (req_offset == file_contents.size()) {
                                update_continuation_range(file_rng);
                                NoLockData::D::set_range(clip.nolock_data);
                                return true;
                            }

                            // insufficient data, synchronize buffer
                            if (req_offset < file_contents.size()) {
                                auto already_known = file_contents.size() - req_offset;
                                uint64_t offset = req_offset + already_known;
                                uint32_t requested_len = checked_int(req_len - already_known);

                                send_filecontents_range(
                                    receiver,
                                    file_rng.stream_id,
                                    file_rng.lindex,
                                    offset,
                                    requested_len);

                                file_rng.stream_id = stream_id;
                                file_rng.file_size_requested = requested_len;
                                file_rng.first_file_size_requested = req_len;

                                NoLockData::D::set_requested_sync_range(clip.nolock_data);

                                return false;
                            }
                        }

                        Self::stop_valid_transfer(self, clip, file_rng);
                        NoLockData::D::init_empty(clip.nolock_data, self);
                        LOG(LOG_WARNING,
                            "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                " Unsupported random access for a FILECONTENTS_RANGE (2)");
                        return send_error();
                    }
                    else {
                        Self::stop_valid_transfer(self, clip, file_rng);
                        NoLockData::D::init_empty(clip.nolock_data, self);
                        return init_contents_size();
                    }
                }

                case ClipCtx::TransferState::WaitingContinuationRange:
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        if (file_rng.file_offset == file_contents_request_pdu.position()
                         && file_rng.lindex == ifile
                        ) {
                            update_continuation_range(file_rng);
                            NoLockData::D::set_range(clip.nolock_data);
                            return true;
                        }
                        else {
                            Self::broken_file_transfer(self, clip, file_rng);
                            NoLockData::D::init_empty(clip.nolock_data, self);
                            return init_contents_range();
                        }
                    }
                    else {
                        Self::broken_file_transfer(self, clip, file_rng);
                        NoLockData::D::init_empty(clip.nolock_data, self);
                        return init_contents_size();
                    }

                case ClipCtx::TransferState::WaitingFileValidator:
                    Self::stop_wait_validation(self, clip, clip.nolock_data.data);
                    NoLockData::D::init_empty(clip.nolock_data, self);
                    [[fallthrough]];

                case ClipCtx::TransferState::Empty:
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        return init_contents_range();
                    }
                    return init_contents_size();

                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::RequestedRange:
                case ClipCtx::TransferState::RequestedSyncRange:
                case ClipCtx::TransferState::GetRange:
                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::Text:
                case ClipCtx::TransferState::WaitingTextValidator:
                case ClipCtx::TransferState::TextAccumulate:
                case ClipCtx::TransferState::RejectedText:
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                        " double request");
                    throw Error(ERR_RDP_PROTOCOL);
            }
        }
        else if (not file_contents_request_pdu.has_optional_clipDataId()) {
            LOG(LOG_WARNING, "Not lock_id with CB_CAN_LOCK_CLIPDATA");
            send_error();
            return false;
        }
        else {
            const auto lock_id = LockId(file_contents_request_pdu.clipDataId());

            auto* lock_data = clip.locked_data.search_lock_by_id(lock_id);

            if (not lock_data) {
                LOG(LOG_WARNING, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " unknown clipDataId (%u)", lock_id);
                return send_error();
            }

            if (clip.locked_data.contains_stream_id(stream_id)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " streamId already used (%u)", stream_id);
                throw Error(ERR_RDP_PROTOCOL);
            }

            if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                auto* r = clip.locked_data.search_range_by_offset(
                    lock_id, ifile, file_contents_request_pdu.position());

                if (r) {
                    update_continuation_range(r->file_contents_range);
                    r->state = ClipCtx::LockedData::LockedRange::State::WaitingResponse;
                }
                else if (check_valid_lindex(lock_data->files)) {
                    clip.locked_data.requested_range
                        = ClipCtx::LockedData::LockedRequestedRange{
                            lock_id, {
                                stream_id,
                                ifile,
                                file_contents_request_pdu.cbRequested(),
                                lock_data->files[lindex].file_size,
                                lock_data->files[lindex].file_name
                            }
                        };
                }
                else {
                    return false;
                }
            }
            else if (check_valid_lindex(lock_data->files)) {
                clip.locked_data.sizes.push_back({lock_id, {stream_id, ifile}});
            }
            else {
                return false;
            }
        }

        return true;
    }

    static bool filecontents_response(
        ClipboardVirtualChannel& self,
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data, uint32_t total_length,
        VirtualChannelDataSender& sender, VirtualChannelDataSender& receiver)
    {
        bool send_message_is_ok = true;
        const bool is_ok = check_header_response(in_header, flags);

        InStream in_stream(chunk_data);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            ::check_throw(in_stream, 4,
                "FileContentsResponse::receive", ERR_RDP_DATA_TRUNCATED);
            if (clip.has_current_file_contents_stream_id) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                    " streamId already defined");
                throw Error(ERR_RDP_PROTOCOL);
            }
            clip.current_file_contents_stream_id = safe_int(in_stream.in_uint32_le());
            clip.has_current_file_contents_stream_id = true;
            LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                "File Contents Response: streamId=%u", clip.current_file_contents_stream_id);
        }
        else if (not clip.has_current_file_contents_stream_id) {
            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                " unknown streamId");
            throw Error(ERR_RDP_PROTOCOL);
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            clip.has_current_file_contents_stream_id = false;
        }

        auto const stream_id = clip.current_file_contents_stream_id;

        auto new_file_validator_id = [&self, &clip](std::string_view file_name){
            FileValidatorId file_validator_id{};
            if (!clip.validator_target_name.empty()) {
                LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::Validator::open_file");
                file_validator_id = self.file_validator->open_file(
                    file_name, clip.validator_target_name);
            }
            return file_validator_id;
        };

        auto new_tfl = [&]{
            return self.fdx_capture
                ? std::unique_ptr<FileContentsRange::TflFile>(new FileContentsRange::TflFile{
                    self.fdx_capture->new_tfl((&clip == &self.server_ctx)
                        ? Mwrm3::Direction::ServerToClient
                        : Mwrm3::Direction::ClientToServer
                    )
                })
                : std::unique_ptr<FileContentsRange::TflFile>();
        };

        using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;

        auto update_file_size_or_throw = [&in_header, flags](
            std::vector<CliprdFileInfo>& files, FileGroupId lindex, bytes_view chunk_data
        ){
            if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                    " Unsupported partial FILECONTENTS_SIZE packet");
                throw Error(ERR_RDP_UNSUPPORTED);
            }

            if (in_header.msgFlags() != RDPECLIP::CB_RESPONSE_OK) {
                return false;
            }

            InStream in_stream(chunk_data);
            check_throw(in_stream, 8, "process_filecontents_response", ERR_RDP_DATA_TRUNCATED);
            files[size_t(lindex)].file_size = in_stream.in_uint64_le();

            return true;
        };

        auto update_file_range_data = [&self](
            ClipCtx::FileContentsRange& file_rng, bytes_view data
        ){
            if (data.size() >= file_rng.file_size_requested) {
                data = data.first(file_rng.file_size_requested);
            }

            file_rng.sig.update(data);
            file_rng.file_offset += data.size();
            file_rng.file_size_requested -= data.size();

            if (bool(file_rng.tfl_file_ptr)) {
                file_rng.tfl_file_ptr->send(data);
            }

            if (bool(file_rng.file_validator_id)) {
                self.file_validator->send_data(
                    file_rng.file_validator_id, data);
            }

            return data;
        };

        auto set_finalize_file_transfer = [&](ClipCtx::FileContentsRange& file_rng){
            if (file_rng.file_offset < file_rng.file_size) {
                if (file_rng.file_size != CliprdFileInfo::invalid_size) {
                    return false;
                }
                file_rng.file_size = file_rng.file_offset;
            }

            file_rng.sig.final();
            Self::log_file_info(self, file_rng, (&clip == &self.server_ctx));

            return true;
        };

        auto finalize_file_transfer = [&](ClipCtx::FileContentsRange& file_rng){
            if (set_finalize_file_transfer(file_rng)) {
                Self::_close_file_rng(self, clip, file_rng,
                    Mwrm3::TransferedStatus::Completed);

                return true;
            }

            return false;
        };

        auto send_file_contents_request = [&](ClipCtx::FileContentsRange& file_rng){
            if (file_rng.file_size > clip.max_file_size_rejected) {
                LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::process_filecontents_response:"
                    " file too large: %" PRIu64 "/%" PRIu64,
                    file_rng.file_size, clip.max_file_size_rejected);

                FilecontentsRequestSendBack(
                    RDPECLIP::FILECONTENTS_RANGE,
                    safe_int(stream_id),
                    &receiver);
                Self::stop_validation_before_transfer(self, clip, file_rng);
                NoLockData::D::init_empty(clip.nolock_data, self);
                return false;
            }

            D::send_filecontents_range(
                sender,
                file_rng.stream_id,
                file_rng.lindex,
                file_rng.file_offset,
                file_rng.first_file_size_requested);

            file_rng.file_size_requested = file_rng.first_file_size_requested;
            return true;
        };


        if (not self.can_lock) {
            auto check_stream_id = [&stream_id](StreamId current_stream_id){
                if (current_stream_id != stream_id) {
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                        " invalid stream id %u, expected %u", stream_id, current_stream_id);
                    throw Error(ERR_RDP_PROTOCOL);
                }
            };

            switch (clip.nolock_data)
            {
            case ClipCtx::TransferState::Text:
            case ClipCtx::TransferState::WaitingTextValidator:
            case ClipCtx::TransferState::TextAccumulate:
            case ClipCtx::TransferState::Empty:
            case ClipCtx::TransferState::SyncRange:
            case ClipCtx::TransferState::WaitingFileValidator:
            case ClipCtx::TransferState::RejectedText:
                // return false;

            case ClipCtx::TransferState::WaitingContinuationRange:
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                    " invalid state %d", int(clip.nolock_data.transfer_state));
                throw Error(ERR_RDP_PROTOCOL);

            case ClipCtx::TransferState::Size:
                if (is_ok) {
                    check_stream_id(clip.nolock_data.data.stream_id);
                    update_file_size_or_throw(
                        clip.files, clip.nolock_data.data.lindex,
                        in_stream.remaining_bytes());
                }

                NoLockData::D::init_empty(clip.nolock_data, self);
                break;

            case ClipCtx::TransferState::RequestedRange: {
                if (is_ok) {
                    check_stream_id(clip.nolock_data.data.stream_id);
                    NoLockData::FileData& file_rng = clip.nolock_data.data;

                    auto validator_id = new_file_validator_id(file_rng.file_name);

                    if (clip.verify_file_before_transfer) {
                        LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                            "ClipboardVirtualChannel::process_filecontents_response:"
                            " Wait before transfer");
                        send_message_is_ok = false;

                        OSD::D::osd_prepare_message(self, clip.nolock_data.data.file_name);

                        NoLockData::D::requested_range_to_get_range(
                            clip.nolock_data, validator_id, new_tfl());

                        auto contents = update_file_range_data(
                            file_rng, in_stream.remaining_bytes());
                        file_rng.file_content.append(contents);

                        uint32_t data_len;

                        if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                            if (set_finalize_file_transfer(file_rng)) {
                                self.file_validator->send_eof(file_rng.file_validator_id);
                                NoLockData::D::set_waiting_validator(clip.nolock_data);
                            }
                            else if (not send_file_contents_request(file_rng)) {
                                return false;
                            }

                            data_len = file_rng.file_content.size();
                        }
                        else {
                            data_len = std::min(
                                in_header.dataLen(), file_rng.file_size_requested);
                        }

                        file_rng.response_size = send_filecontents_response_header(
                            receiver, file_rng.stream_id, data_len);
                    }
                    else {
                        NoLockData::D::requested_range_to_range(
                            clip.nolock_data, validator_id, new_tfl());
                        update_file_range_data(clip.nolock_data.data, in_stream.remaining_bytes());

                        if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                            if (finalize_file_transfer(clip.nolock_data.data)) {
                                NoLockData::D::init_empty(clip.nolock_data, self);
                            }
                            else {
                                NoLockData::D::set_waiting_continuation_range(clip.nolock_data);
                            }
                        }
                    }
                }
                else {
                    NoLockData::D::init_empty(clip.nolock_data, self);
                }

                break;
            }

            case ClipCtx::TransferState::Range: {
                if (is_ok) {
                    check_stream_id(clip.nolock_data.data.stream_id);
                    update_file_range_data(clip.nolock_data.data, in_stream.remaining_bytes());

                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                        if (finalize_file_transfer(clip.nolock_data.data)) {
                            NoLockData::D::init_empty(clip.nolock_data, self);
                        }
                        else {
                            NoLockData::D::set_waiting_continuation_range(clip.nolock_data);
                        }
                    }
                }
                else {
                    Self::broken_file_transfer(self, clip, clip.nolock_data.data);
                    NoLockData::D::init_empty(clip.nolock_data, self);
                }

                break;
            }

            case ClipCtx::TransferState::RequestedSyncRange: {
                NoLockData::FileData& file_rng = clip.nolock_data.data;

                if (is_ok) {
                    check_stream_id(file_rng.stream_id);
                    send_message_is_ok = false;

                    uint32_t known_data_len
                        = file_rng.first_file_size_requested - file_rng.file_size_requested;

                    update_file_range_data(file_rng, in_stream.remaining_bytes());

                    if (file_rng.first_file_size_requested) {
                        auto file_contents = file_rng.file_content.read(known_data_len);

                        using Views = bytes_view[];
                        file_rng.response_size = send_response_datas_with_lengths_and_last_flag(
                            receiver, file_rng.stream_id,
                            Views{ file_contents, in_stream.remaining_bytes() },
                            checked_int(total_length + file_contents.size()),
                            checked_int(in_header.dataLen() + file_contents.size()),
                            flags & CHANNELS::CHANNEL_FLAG_LAST);

                        file_rng.first_file_size_requested = 0;
                    }
                    else {
                        receiver(file_rng.response_size, flags, in_stream.remaining_bytes());
                    }

                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                        if (set_finalize_file_transfer(file_rng)) {
                            Self::_close_file_rng_tfl(self, file_rng,
                                Mwrm3::TransferedStatus::Completed);
                            NoLockData::D::init_empty(clip.nolock_data, self);
                        }
                        else {
                            NoLockData::D::set_waiting_continuation_range(clip.nolock_data);
                        }
                    }
                }
                else {
                    Self::stop_valid_transfer(self, clip, file_rng);
                    Self::log_file_info(self, file_rng, (&clip == &self.server_ctx));
                    NoLockData::D::init_empty(clip.nolock_data, self);
                }

                break;
            }

            case ClipCtx::TransferState::GetRange: {
                NoLockData::FileData& file_rng = clip.nolock_data.data;

                auto send_zero_data = [&]{
                    LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                        "ClipboardVirtualChannel::process_filecontents_response:"
                        " Send fake data");
                    filecontents_response_zerodata(receiver, file_rng.response_size);
                    send_message_is_ok = false;
                };

                if (is_ok) {
                    check_stream_id(clip.nolock_data.data.stream_id);

                    send_message_is_ok = false;

                    auto contents = update_file_range_data(
                        file_rng, in_stream.remaining_bytes());
                    file_rng.file_content.append(contents);

                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                        bool is_finalized = set_finalize_file_transfer(file_rng);

                        switch (file_rng.validator_state)
                        {
                            case ValidatorState::Failure:
                                LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                                    "ClipboardVirtualChannel::process_filecontents_response:"
                                    " file is rejected, stop process");
                                if (is_finalized) {
                                    Self::_close_file_rng_tfl(
                                        self, file_rng, Mwrm3::TransferedStatus::Completed);
                                }
                                else {
                                    file_rng.sig.broken();
                                    Self::log_file_info(self, file_rng,
                                        (&clip == &self.server_ctx));
                                    Self::_close_file_rng_tfl(
                                        self, file_rng, Mwrm3::TransferedStatus::Broken);
                                }
                                send_zero_data();
                                NoLockData::D::init_empty(clip.nolock_data, self);
                                break;

                            case ValidatorState::Success:
                                LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                                    "ClipboardVirtualChannel::process_filecontents_response:"
                                    " file is accepted, send data");
                                if (is_finalized) {
                                    Self::_close_file_rng_tfl(
                                        self, file_rng, Mwrm3::TransferedStatus::Completed);
                                }
                                LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                                    "ClipboardVirtualChannel::process_filecontents_response:"
                                    " Send first data");
                                file_rng.file_content.set_read_mode();
                                filecontents_response_data(
                                    receiver, file_rng.response_size, file_rng.file_content);
                                NoLockData::D::set_sync_range(clip.nolock_data);
                                break;

                            case ValidatorState::Wait:
                                if (is_finalized) {
                                    LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                                        "ClipboardVirtualChannel::process_filecontents_response:"
                                        " wait validator");
                                    self.file_validator->send_eof(file_rng.file_validator_id);
                                    NoLockData::D::set_waiting_validator(clip.nolock_data);
                                }
                                else {
                                    LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                                        "ClipboardVirtualChannel::process_filecontents_response:"
                                        " new request_range");
                                    send_file_contents_request(file_rng);
                                    return false;
                                }
                                break;
                        }
                    }
                }
                else {
                    Self::stop_validation_before_transfer(self, clip, file_rng);

                    send_zero_data();
                    NoLockData::D::init_empty(clip.nolock_data, self);
                }

                break;
            }
            }
        }
        else {
            assert(not clip.verify_file_before_transfer);

            auto update_locked_file_range = [&](ClipCtx::LockedData::LockedRange& locked_file){
                auto& file_rng = locked_file.file_contents_range;

                update_file_range_data(file_rng, in_stream.remaining_bytes());

                if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                    if (finalize_file_transfer(file_rng)) {
                        clip.locked_data.remove_locked_file_contents_range(&locked_file);
                    }
                    else {
                        locked_file.state
                            = ClipCtx::LockedData::LockedRange::State::WaitingRequest;
                    }
                }
            };

            if (clip.locked_data.requested_range.is_stream_id(stream_id)) {
                if (is_ok) {
                    auto& req = clip.locked_data.requested_range.file_contents_requested_range;
                    auto& r = clip.locked_data.ranges.emplace_back(ClipCtx::LockedData::LockedRange{
                        clip.locked_data.requested_range.lock_id,
                        ClipCtx::LockedData::LockedRange::State::WaitingResponse,
                        ClipCtx::FileContentsRange{
                            req.stream_id,
                            req.lindex,
                            new_file_validator_id(req.file_name),
                            ValidatorState::Wait,
                            0,
                            req.file_size_requested,
                            checked_int(std::min(
                                uint64_t(req.file_size_requested),
                                req.file_size)),
                            req.file_size,
                            0,
                            std::move(req.file_name),
                            new_tfl()
                        }
                    });
                    r.file_contents_range.sig.reset();

                    update_locked_file_range(r);
                }
                else {
                    clip.locked_data.requested_range.disable();
                }

                assert(not clip.locked_data.requested_range.is_stream_id(stream_id));
            }
            else if (auto* locked_contents_range
                = clip.locked_data.search_range_by_id(stream_id)
            ) {
                if (is_ok) {
                    update_locked_file_range(*locked_contents_range);
                }
                else {
                    Self::broken_file_transfer(
                        self, clip, locked_contents_range->file_contents_range);
                    clip.locked_data.remove_locked_file_contents_range(locked_contents_range);
                }
            }
            else if (auto* locked_contents_size
                = clip.locked_data.search_size_by_id(stream_id)
            ) {
                if (is_ok) {
                    not_null_ptr lock_data
                        = clip.locked_data.search_lock_by_id(locked_contents_size->lock_id);
                    update_file_size_or_throw(
                        lock_data->files, locked_contents_size->file_contents_size.lindex,
                        in_stream.remaining_bytes());
                }
                clip.locked_data.remove_locked_file_contents_size(locked_contents_size);
            }
            else {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response:"
                    " streamId (%u) not found", stream_id);
                throw Error(ERR_RDP_PROTOCOL);
            }
        }

        return send_message_is_ok;
    }

    static uint64_t real_file_size(ClipCtx::FileContentsRange& file_contents_range)
    {
        return (file_contents_range.file_size != CliprdFileInfo::invalid_size)
            ? file_contents_range.file_size
            : file_contents_range.file_offset + file_contents_range.file_size_requested;
    }

    static void log_file_info(
        ClipboardVirtualChannel& self,
        ClipCtx::FileContentsRange& file_contents_range,
        bool from_remote_session)
    {
        const char* type = from_remote_session
            ? "CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION"
            : "CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION";

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
        size_t file_size_len = std::snprintf(file_size, std::size(file_size), "%lu",
            real_file_size(file_contents_range));

        self.session_log.log6(
            from_remote_session
                ? LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION
                : LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION,
         {
            KVLog("file_name"_av, file_contents_range.file_name),
            KVLog("size"_av, {file_size, file_size_len}),
            KVLog("sha256"_av, {digest_s, digest_s_len}),
        });

        LOG_IF(!self.params.dont_log_data_into_syslog, LOG_INFO,
            "type=%s file_name=%s size=%s sha256=%s",
            type, file_contents_range.file_name, file_size, digest_s);
    }

    static void log_siem_info(
        ClipboardVirtualChannel& self,
        Cliprdr::FormatNameInventory const& current_format_list, uint32_t flags,
        const RDPECLIP::CliprdrHeader & in_header, bool is_file_group_id,
        const uint32_t requestedFormatId, bytes_view data_to_dump,
        bool is_client_to_server
    ) {
        if (!(
            bool(flags & CHANNELS::CHANNEL_FLAG_FIRST)
         && bool(in_header.msgFlags() & RDPECLIP::CB_RESPONSE_OK)
        )) {
            return ;
        }

        bytes_view utf8_format;
        bool log_current_activity;

        if (is_file_group_id) {
            utf8_format = Cliprdr::formats::file_group_descriptor_w.ascii_name;
            log_current_activity = true;
        }
        else {
            auto* format_name = current_format_list.find(requestedFormatId);

            utf8_format = (format_name && not format_name->utf8_name().empty() )
                ? format_name->utf8_name()
                : RDPECLIP::get_FormatId_name_av(requestedFormatId);

            log_current_activity
                = !self.params.log_only_relevant_clipboard_activities
               || !format_name
               || !ranges_equal(utf8_format, Cliprdr::formats::preferred_drop_effect.ascii_name);
        }

        if (self.params.dont_log_data_into_syslog && not log_current_activity) {
            return ;
        }

        char format_buf[255];
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wformat-overflow=")
        int format_len = std::sprintf(format_buf, "%.*s(%u)",
            int(utf8_format.size()), utf8_format.as_charp(), requestedFormatId);
        REDEMPTION_DIAGNOSTIC_POP
        array_view format_av{format_buf, checked_cast<std::size_t>(format_len)};

        char size_buf[32];
        int size_len = std::sprintf(size_buf, "%u", in_header.dataLen());
        array_view size_av{size_buf, checked_cast<std::size_t>(size_len)};

        constexpr size_t max_length_of_data_to_dump = 256;
        char data_to_dump_buf[
            max_length_of_data_to_dump * maximum_length_of_utf8_character_in_bytes];
        std::string_view utf8_string{};

        if (log_current_activity || not self.params.dont_log_data_into_syslog) {
            switch (requestedFormatId) {
                /*
                case RDPECLIP::CF_TEXT:
                {
                    const size_t length_of_data_to_dump = std::min(
                        chunk.in_remain(), max_length_of_data_to_dump);
                    data_to_dump.assign(
                        ::char_ptr_cast(chunk.get_current()),
                        length_of_data_to_dump);
                }
                break;
                */
                case RDPECLIP::CF_UNICODETEXT:
                {
                    assert(!(data_to_dump.size() & 1));

                    const size_t length_of_data_to_dump = std::min(
                        data_to_dump.size(), max_length_of_data_to_dump * 2);

                    auto av = ::UTF16toUTF8_buf(
                        data_to_dump.first(length_of_data_to_dump),
                        make_writable_array_view(data_to_dump_buf));
                    utf8_string = {av.as_charp(), av.size()};
                    if (not utf8_string.empty() && not utf8_string.back()) {
                        utf8_string.remove_suffix(1);
                    }
                }
                break;

                case RDPECLIP::CF_LOCALE:
                    if (data_to_dump.size() >= 4) {
                        InStream in_stream(data_to_dump);
                        int len = std::sprintf(data_to_dump_buf,
                            "%u", in_stream.in_uint32_le());
                        utf8_string = {data_to_dump_buf, checked_cast<std::size_t>(len)};
                    }
                break;
            }
        }

        if (log_current_activity) {
            if (utf8_string.empty()) {
                self.session_log.log6(is_client_to_server
                    ? LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION
                    : LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION,
                {
                    KVLog("format"_av, format_av),
                    KVLog("size"_av, size_av),
                });
            }
            else {
                self.session_log.log6(is_client_to_server
                    ? LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX
                    : LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX,
                {
                    KVLog("format"_av, format_av),
                    KVLog("size"_av, size_av),
                    KVLog("partial_data"_av, utf8_string),
                });
            }
        }

        if (not self.params.dont_log_data_into_syslog) {
            const auto type = (is_client_to_server)
                ? (utf8_string.empty()
                    ? "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION"_av
                    : "CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX"_av)
                : (utf8_string.empty()
                    ? "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION"_av
                    : "CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX"_av);

            LOG(LOG_INFO,
                "type=%s format=%.*s size=%.*s %s%.*s",
                type.data(),
                int(format_av.size()), format_av.data(),
                int(size_av.size()), size_av.data(),
                utf8_string.empty() ? "" : " partial_data",
                int(utf8_string.size()), utf8_string.data());
        }
    }
};


ClipboardVirtualChannel::ClipboardVirtualChannel(
    VirtualChannelDataSender* to_client_sender_,
    VirtualChannelDataSender* to_server_sender_,
    EventContainer& events,
    gdi::OsdApi& osd_api,
    const ClipboardVirtualChannelParams & params,
    FileValidatorService * file_validator_service,
    FileStorage file_storage,
    SessionLogApi& session_log,
    RDPVerbose verbose)
: BaseVirtualChannel(to_client_sender_, to_server_sender_)
// TODO decompose (extract validatorparam)
, params([&]{
    auto p = params;
    if (!file_validator_service) {
        p.validator_params.up_target_name.clear();
        p.validator_params.down_target_name.clear();
    }
    else {
        LOG(LOG_INFO, "ClipboardVirtualChannel: enable file validator service:"
            " up=%s  down=%s  block_invalid_file_up=%d block_invalid_file_down=%d",
            p.validator_params.up_target_name,
            p.validator_params.down_target_name,
            params.validator_params.block_invalid_file_up,
            params.validator_params.block_invalid_file_down);
    }
    return p;
}())
, file_validator(file_validator_service)
, fdx_capture(file_storage.fdx_capture)
, tmp_dir(std::move(file_storage.tmp_dir))
, session_log(session_log)
, verbose(verbose)
, always_file_storage(file_storage.always_file_storage)
, proxy_managed(to_client_sender_ == nullptr)
, client_ctx(
    this->params.validator_params.up_target_name,
    this->params.validator_params.block_invalid_file_up,
    this->params.validator_params.block_invalid_text_up,
    this->params.validator_params.max_file_size_rejected,
    this->tmp_dir.empty() ? "/tmp/" : this->tmp_dir.c_str())
, server_ctx(
    this->params.validator_params.down_target_name,
    this->params.validator_params.block_invalid_file_down,
    this->params.validator_params.block_invalid_text_down,
    this->params.validator_params.max_file_size_rejected,
    this->tmp_dir.empty() ? "/tmp/" : this->tmp_dir.c_str())
, osd{
    events,
    osd_api,
    this->params.validator_params.osd_delay,
    (this->params.validator_params.osd_delay.count()
        && (this->params.validator_params.block_invalid_file_down
         || this->params.validator_params.block_invalid_file_up)),
    this->params.validator_params.lang}
{}

ClipboardVirtualChannel::~ClipboardVirtualChannel()
{
    try {
        using namespace std::string_view_literals;

        auto status = "Connexion closed"sv;

        for (auto& text_validator : this->text_validator_list) {
            dlpav_report_text(
                text_validator.file_validator_id,
                this->session_log,
                text_validator.direction, status);
        }

        for (auto& file_validator : this->file_validator_list) {
            if (file_validator.tfl_file_ptr) {
                this->fdx_capture->close_tfl(
                    file_validator.tfl_file_ptr->tfl_file,
                    file_validator.file_name,
                    Mwrm3::TransferedStatus::Broken,
                    Mwrm3::Sha256Signature{file_validator.sig.digest_as_av()});
            }

            dlpav_report_file(
                file_validator.file_name,
                this->session_log,
                file_validator.direction, status);
        }

        auto close_clip = [&](ClipCtx& clip){
            auto direction = ClipCtx::D::to_direction(*this, clip);

            auto close_rng = [&](ClipCtx::FileContentsRange& rng){
                auto& file_name = rng.file_name;

                if (rng.tfl_file_ptr) {
                    rng.sig.broken();
                    this->fdx_capture->close_tfl(
                        rng.tfl_file_ptr->tfl_file,
                        file_name,
                        Mwrm3::TransferedStatus::Broken,
                        Mwrm3::Sha256Signature{rng.sig.digest_as_av()});
                }

                if (bool(rng.file_validator_id)) {
                    dlpav_report_file(
                        file_name,
                        this->session_log,
                        direction, status);
                }
            };

            switch (clip.nolock_data)
            {
                case ClipCtx::TransferState::Empty:
                    break;

                case ClipCtx::TransferState::RequestedRange:
                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::RejectedText:
                    ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                    break;

                case ClipCtx::TransferState::Text:
                case ClipCtx::TransferState::WaitingTextValidator:
                case ClipCtx::TransferState::TextAccumulate:
                    if (bool(clip.nolock_data.data.file_validator_id)) {
                        dlpav_report_text(
                            clip.nolock_data.data.file_validator_id,
                            this->session_log,
                            direction, status);
                    }
                    ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                    break;

                case ClipCtx::TransferState::GetRange:
                case ClipCtx::TransferState::WaitingFileValidator:
                case ClipCtx::TransferState::RequestedSyncRange:
                case ClipCtx::TransferState::SyncRange:
                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::WaitingContinuationRange:
                    close_rng(clip.nolock_data.data);
                    ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                    break;
            }

            for (auto& locked_file : clip.locked_data.ranges) {
                close_rng(locked_file.file_contents_range);
            }
        };

        close_clip(this->client_ctx);
        close_clip(this->server_ctx);
    }
    catch (Error const& err) {
        LOG(LOG_ERR, "ClipboardVirtualChannel: error on close tfls: %s", err.errmsg());
    }
}

void ClipboardVirtualChannel::process_server_message(uint32_t total_length,
    uint32_t flags, bytes_view chunk_data)
{
    if (flags && !(flags &~ (CHANNELS::CHANNEL_FLAG_SUSPEND | CHANNELS::CHANNEL_FLAG_RESUME))) {
        this->send_message_to_client(total_length, flags, chunk_data);
        return;
    }

    InStream chunk(chunk_data);
    RDPECLIP::CliprdrHeader header;
    bool send_message_to_client = true;

    this->server_ctx.message_type = process_header_message(
        this->server_ctx.message_type, total_length, flags, chunk, header,
        Direction::FileFromServer, this->verbose);

    switch (this->server_ctx.message_type)
    {
        case RDPECLIP::CB_CLIP_CAPS: {
            send_message_to_client = ClipCtx::D::clip_caps(
                *this, this->server_ctx, chunk.remaining_bytes(),
                "server", this->proxy_managed ? nullptr : this->to_client_sender_ptr());
            send_message_to_client = send_message_to_client && !this->proxy_managed;
        }
        break;

        case RDPECLIP::CB_MONITOR_READY: {
            if (this->proxy_managed) {
                this->server_ctx.use_long_format_names = true;
                ServerMonitorReadySendBack sender(this->verbose, this->use_long_format_names(), this->to_server_sender_ptr());
            }

            ClipCtx::D::monitor_ready(*this, this->server_ctx, this->to_client_sender_ptr());

            if (this->clipboard_monitor_ready_notifier) {
                if (!this->clipboard_monitor_ready_notifier->on_clipboard_monitor_ready()) {
                    this->clipboard_monitor_ready_notifier = nullptr;
                }
            }

            send_message_to_client = !this->proxy_managed;
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST: {
            send_message_to_client = ClipCtx::D::format_list(
                *this, flags, header,
                this->to_server_sender_ptr(),
                this->to_client_sender_ptr(),
                this->params.clipboard_down_authorized || this->params.clipboard_up_authorized,
                this->server_ctx,
                chunk.remaining_bytes());

            if (this->format_list_notifier) {
                if (!this->format_list_notifier->on_server_format_list()) {
                    this->format_list_notifier = nullptr;
                }
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
            ClipCtx::D::format_list_response(header, flags, this->client_ctx);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            ClipCtx::D::format_data_request(this->client_ctx, chunk.remaining_bytes(), this->verbose);

            if (this->format_data_request_notifier
             && this->client_ctx.requested_format_id == RDPECLIP::CF_TEXT
            ) {
                if (!this->format_data_request_notifier->on_server_format_data_request()) {
                    this->format_data_request_notifier = nullptr;
                }

                send_message_to_client = false;
            }
            else if (!this->params.clipboard_up_authorized) {
                LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::process_server_format_data_request_pdu: "
                        "Client to server Clipboard operation is not allowed.");

                FormatDataRequestSendBack sender(this->to_server_sender_ptr());

                send_message_to_client = false;
            }
        }
        break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            auto* receiver = this->to_client_sender_ptr();
            if (receiver) {
                send_message_to_client = ClipCtx::D::format_data_response(
                    *this, header, this->server_ctx, flags, chunk.remaining_bytes(),
                    total_length, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto* sender = this->to_server_sender_ptr();
            auto* receiver = this->to_client_sender_ptr();
            if (sender && receiver) {
                send_message_to_client = ClipCtx::D::filecontents_request(
                    *this, this->client_ctx, flags, chunk.remaining_bytes(),
                    *sender, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
            auto* sender = this->to_server_sender_ptr();
            auto* receiver = this->to_client_sender_ptr();
            if (sender && receiver) {
                send_message_to_client = ClipCtx::D::filecontents_response(
                    *this, header, this->server_ctx, flags, chunk.remaining_bytes(),
                    total_length, *sender, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_LOCK_CLIPDATA: {
            ClipCtx::D::lock(*this, this->client_ctx, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_UNLOCK_CLIPDATA: {
            ClipCtx::D::unlock(*this, this->client_ctx, chunk.remaining_bytes());
        }
    }   // switch (this->server_message_type)

    if (send_message_to_client) {
        this->send_message_to_client(total_length, flags, chunk_data);
    }   // switch (this->server_message_type)
}

void ClipboardVirtualChannel::process_client_message(
    uint32_t total_length, uint32_t flags, bytes_view chunk_data)
{
    InStream chunk(chunk_data);
    RDPECLIP::CliprdrHeader header;
    bool send_message_to_server = true;

    this->client_ctx.message_type = process_header_message(
        this->client_ctx.message_type, total_length, flags, chunk, header,
        Direction::FileFromClient, this->verbose);

    switch (this->client_ctx.message_type)
    {
        case RDPECLIP::CB_CLIP_CAPS: {
            send_message_to_server = ClipCtx::D::clip_caps(
                *this, this->client_ctx, chunk.remaining_bytes(),
                "client", this->to_server_sender_ptr());
            this->can_lock = this->client_ctx.optional_lock_id.is_enabled()
                          && this->server_ctx.optional_lock_id.is_enabled();
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST: {
            send_message_to_server = ClipCtx::D::format_list(
                *this, flags, header,
                this->to_client_sender_ptr(),
                this->to_server_sender_ptr(),
                this->params.clipboard_down_authorized
                || this->params.clipboard_up_authorized
                || this->format_list_response_notifier,
                this->client_ctx,
                chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            ClipCtx::D::format_list_response(header, flags, this->server_ctx);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            ClipCtx::D::format_data_request(this->server_ctx, chunk.remaining_bytes(), this->verbose);
            send_message_to_server = this->params.clipboard_down_authorized;
            if (!this->params.clipboard_down_authorized) {
                LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "ClipboardVirtualChannel::process_client_format_data_request_pdu: "
                        "Server to client Clipboard operation is not allowed.");

                FormatDataRequestSendBack sender(this->to_client_sender_ptr());
            }
        }
        break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE: {
            auto* receiver = this->to_server_sender_ptr();
            if (receiver) {
                send_message_to_server = ClipCtx::D::format_data_response(
                    *this, header, this->client_ctx, flags, chunk.remaining_bytes(),
                    total_length, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto* sender = this->to_client_sender_ptr();
            auto* receiver = this->to_server_sender_ptr();
            if (sender && receiver) {
                send_message_to_server = ClipCtx::D::filecontents_request(
                    *this, this->server_ctx, flags, chunk.remaining_bytes(),
                    *sender, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
            auto* sender = this->to_client_sender_ptr();
            auto* receiver = this->to_server_sender_ptr();
            if (sender && receiver) {
                send_message_to_server = ClipCtx::D::filecontents_response(
                    *this, header, this->client_ctx, flags, chunk.remaining_bytes(),
                    total_length, *sender, *receiver);
            }
        }
        break;

        case RDPECLIP::CB_LOCK_CLIPDATA: {
            ClipCtx::D::lock(*this, this->server_ctx, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_UNLOCK_CLIPDATA: {
            ClipCtx::D::unlock(*this, this->server_ctx, chunk.remaining_bytes());
        }
        break;
    }   // switch (this->client_message_type)

    if (send_message_to_server) {
        this->send_message_to_server(total_length, flags, chunk_data);
    }
}

void ClipboardVirtualChannel::DLP_antivirus_check_channels_files()
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

        auto process_text = [&](Direction direction){
            if (!is_accepted || this->params.validator_params.log_if_accepted) {
                dlpav_report_text(
                    file_validator_id,
                    this->session_log,
                    direction, result_content);
            }
        };

        auto process_file = [&](Direction direction, std::string_view file_name){
            if (!is_accepted || this->params.validator_params.log_if_accepted) {
                dlpav_report_file(
                    file_name,
                    this->session_log,
                    direction, result_content);
            }

            if (!is_accepted && (direction == Direction::FileFromClient
                ? this->client_ctx.verify_file_before_transfer
                : this->server_ctx.verify_file_before_transfer
            )) {
                session_log.log6(LogId::FILE_BLOCKED, {
                    KVLog("direction"_av, to_dlpav_str_direction(direction)),
                    KVLog("file_name"_av, file_name),
                });
            }
        };

        if (auto* text_validator_data = this->search_text_validator_by_id(file_validator_id)) {
            process_text(text_validator_data->direction);
            text_validator_data->file_validator_id = FileValidatorId();
            this->remove_text_validator(text_validator_data);
        }
        else if (auto* file_validator_data = this->search_file_validator_by_id(file_validator_id)) {
            process_file(file_validator_data->direction, file_validator_data->file_name);

            if (file_validator_data->tfl_file_ptr) {
                if (this->always_file_storage || not is_accepted) {
                    auto status = Mwrm3::TransferedStatus::Completed;
                    if (!file_validator_data->sig.has_digest()) {
                        file_validator_data->sig.broken();
                        status = Mwrm3::TransferedStatus::Broken;
                    }
                    this->fdx_capture->close_tfl(
                        file_validator_data->tfl_file_ptr->tfl_file,
                        file_validator_data->file_name,
                        status, Mwrm3::Sha256Signature{
                            file_validator_data->sig.digest_as_av()
                        });
                }
                else {
                    this->fdx_capture->cancel_tfl(file_validator_data->tfl_file_ptr->tfl_file);
                }
            }

            this->remove_file_validator(file_validator_data);
        }
        else {
            using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;

            auto process_clip = [&](ClipCtx& clip){
                auto dlp_update_file_rng = [&](ClipCtx::FileContentsRange& file_rng){
                    process_file(ClipCtx::D::to_direction(*this, clip), file_rng.file_name);

                    file_rng.file_validator_id = FileValidatorId();
                    file_rng.validator_state = is_accepted
                        ? ValidatorState::Success
                        : ValidatorState::Failure;
                };

                switch (clip.nolock_data)
                {
                case ClipCtx::TransferState::Empty:
                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::RequestedRange:
                case ClipCtx::TransferState::RequestedSyncRange:
                case ClipCtx::TransferState::SyncRange:
                case ClipCtx::TransferState::RejectedText:
                    break;

                case ClipCtx::TransferState::Text:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        process_text(ClipCtx::D::to_direction(*this, clip));
                        ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                        return true;
                    }
                    break;

                case ClipCtx::TransferState::TextAccumulate:
                    process_text(ClipCtx::D::to_direction(*this, clip));
                    OSD::D::osd_dlp_response(*this, clip.nolock_data.data.file_name, is_accepted);
                    if (is_accepted) {
                        ClipCtx::D::send_text_and_reset(
                            ClipCtx::D::get_sender(*this, clip),
                            clip.nolock_data.data.response_size,
                            clip.nolock_data.data.file_content,
                            false);
                        ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                    }
                    else {
                        ClipCtx::NoLockData::D::set_rejected_text(clip.nolock_data);
                    }
                    return true;

                case ClipCtx::TransferState::WaitingTextValidator: {
                    process_text(ClipCtx::D::to_direction(*this, clip));
                    auto& sender = ClipCtx::D::get_sender(*this, clip);
                    if (is_accepted) {
                        ClipCtx::D::send_text_and_reset(
                            sender,
                            clip.nolock_data.data.response_size,
                            clip.nolock_data.data.file_content,
                            true);
                    }
                    else {
                        send_data_response_fail(sender);
                    }
                    OSD::D::osd_dlp_response(*this, clip.nolock_data.data.file_name, is_accepted);
                    ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                    return true;
                }

                case ClipCtx::TransferState::GetRange:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        dlp_update_file_rng(clip.nolock_data.data);
                        return true;
                    }
                    break;

                case ClipCtx::TransferState::WaitingFileValidator:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        auto& file_rng = clip.nolock_data.data;
                        dlp_update_file_rng(file_rng);
                        auto& sender = ClipCtx::D::get_sender(*this, clip);
                        ClipCtx::D::_close_file_rng_tfl(
                            *this, file_rng, Mwrm3::TransferedStatus::Completed);
                        if (is_accepted) {
                            file_rng.file_content.set_read_mode();
                            ClipCtx::D::filecontents_response_data(
                                sender, file_rng.response_size, file_rng.file_content);
                            ClipCtx::NoLockData::D::set_sync_range(clip.nolock_data);
                        }
                        else {
                            filecontents_response_zerodata(sender, file_rng.response_size);
                            ClipCtx::NoLockData::D::init_empty(clip.nolock_data, *this);
                        }
                        return true;
                    }
                    break;

                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::WaitingContinuationRange:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        auto& file_rng = clip.nolock_data.data;
                        dlp_update_file_rng(file_rng);
                        return true;
                    }
                    break;
                }

                if (auto* r = clip.locked_data.search_range_by_validator_id(file_validator_id)) {
                    process_file(ClipCtx::D::to_direction(*this, clip), r->file_contents_range.file_name);
                    r->file_contents_range.file_validator_id = FileValidatorId();
                    return true;
                }

                return false;
            };

            if (process_clip(this->client_ctx)) {
                OSD::D::osd_dlp_response(
                    *this, this->client_ctx.nolock_data.data.file_name, is_accepted);
            }
            else if (process_clip(this->server_ctx)) {
                OSD::D::osd_dlp_response(
                    *this, this->server_ctx.nolock_data.data.file_name, is_accepted);
            }
            else {
                LOG(LOG_INFO, "ClipboardVirtualChannel::DLP_antivirus_check_channels_files:"
                    " unknown validatorId(%u)", file_validator_id);
            }
        }
    }
}

bool ClipboardVirtualChannel::use_long_format_names() const
{
    return (this->client_ctx.use_long_format_names
        && this->server_ctx.use_long_format_names);
}

void ClipboardVirtualChannel::empty_client_clipboard()
{
    LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
        "ClipboardVirtualChannel::empty_client_clipboard");

    RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
        RDPECLIP::CB_RESPONSE__NONE_, 0);

    StaticOutStream<256> out_s;

    clipboard_header.emit(out_s);

    this->send_message_to_server(
        out_s.get_offset(),
        send_first_last_flags,
        out_s.get_produced_bytes());
}

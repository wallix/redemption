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

#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/rdp/channels/cliprdr_channel.hpp"
#include "mod/rdp/channels/cliprdr_channel_send_and_receive.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "core/error.hpp"
#include "core/session_reactor.hpp"
#include "core/log_id.hpp"
#include "utils/log.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/unordered_erase.hpp"

#include <cassert>

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
}

enum class ClipboardVirtualChannel::ClipCtx::Sig::Status : uint8_t
{
    Update,
    Broken,
    Final,
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


void ClipboardVirtualChannel::ClipCtx::
NoLockData::init_empty()
{
    assert(!this->data.tfl_file);
    this->transfer_state = TransferState::Empty;
}

void ClipboardVirtualChannel::ClipCtx::
NoLockData::init_text(FileValidatorId file_validator_id, bool is_unicode)
{
    this->data.file_validator_id = file_validator_id;
    this->is_unicode = is_unicode;
    this->transfer_state = TransferState::Text;
}

void ClipboardVirtualChannel::ClipCtx::
NoLockData::init_size(StreamId stream_id, FileGroupId lindex)
{
    this->data.stream_id = stream_id;
    this->data.lindex = lindex;
    this->transfer_state = TransferState::Size;
}

void ClipboardVirtualChannel::ClipCtx::
NoLockData::init_requested_range(
    StreamId stream_id, FileGroupId lindex,
    uint32_t file_size_requested, uint64_t file_size, std::string_view file_name)
{
    this->data.stream_id = stream_id;
    this->data.lindex = lindex;
    this->data.first_file_size_requested = file_size_requested;
    this->data.file_size = file_size;
    this->data.file_name = file_name;
    this->transfer_state = TransferState::RequestedRange;
}

struct ClipboardVirtualChannel::ClipCtx::NoLockData::D
{
    static void requested_to_rng_base(
        NoLockData& nolock_data,
        FileValidatorId file_validator_id, std::unique_ptr<FdxCapture::TflFile>&& tfl)
    {
        assert(nolock_data.transfer_state == TransferState::RequestedRange);
        nolock_data.data.file_validator_id = file_validator_id;
        nolock_data.data.file_offset = 0;
        nolock_data.data.file_size_requested = std::min(
            uint64_t(nolock_data.data.first_file_size_requested), nolock_data.data.file_size);
        nolock_data.data.tfl_file = std::move(tfl);
        nolock_data.data.file_contents.clear();
        nolock_data.data.sig.reset();
    }
};

void ClipboardVirtualChannel::ClipCtx::
NoLockData::requested_range_to_range(
    FileValidatorId file_validator_id, std::unique_ptr<FdxCapture::TflFile>&& tfl)
{
    D::requested_to_rng_base(*this, file_validator_id, std::move(tfl));
    this->data.validator_state = ValidatorState::Wait;
    this->transfer_state = TransferState::Range;
}

void ClipboardVirtualChannel::ClipCtx::
NoLockData::requested_range_to_get_range(
    FileValidatorId file_validator_id, std::unique_ptr<FdxCapture::TflFile>&& tfl)
{
    D::requested_to_rng_base(*this, file_validator_id, std::move(tfl));
    this->data.validator_state = ValidatorState::WaitValidatorBeforeTransfer;
    this->transfer_state = TransferState::GetRange;
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::set_waiting_continuation_range()
{
    assert(
        this->transfer_state == TransferState::Range
     || this->transfer_state == TransferState::GetRange);
    this->transfer_state = TransferState::WaitingContinuationRange;
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::set_waiting_validator()
{
    assert(this->transfer_state == TransferState::GetRange);
    this->transfer_state = TransferState::WaitingValidator;
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::set_sync_range()
{
    assert(this->transfer_state == TransferState::GetRange);
    this->transfer_state = TransferState::SyncRange;
}

void ClipboardVirtualChannel::ClipCtx::NoLockData::set_range()
{
    assert(this->transfer_state == TransferState::WaitingContinuationRange);
    this->transfer_state = TransferState::Range;
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
    std::string const& target_name, bool verify_before_download)
: verify_before_download(verify_before_download && not target_name.empty())
, validator_target_name(target_name)
{}

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
    std::unique_ptr<FdxCapture::TflFile> tfl_file;
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
    void append(std::vector<uint8_t>& vec, bytes_view av)
    {
        vec.insert(vec.end(), av.begin(), av.end());
    }

    array_view_const_char to_dlpav_str_direction(Direction direction)
    {
        return (direction == Direction::FileFromClient)
            ? "UP"_av
            : "DOWN"_av;
    }

    void dlpav_report_text(
        FileValidatorId file_validator_id,
        ReportMessageApi& report_message,
        timeval time,
        Direction direction,
        std::string_view result_content)
    {
        char buf[24];
        unsigned n = std::snprintf(buf, std::size(buf), "%" PRIu32,
            underlying_cast(file_validator_id));
        report_message.log6(LogId::TEXT_VERIFICATION, time, {
            KVLog("direction"_av, to_dlpav_str_direction(direction)),
            KVLog("copy_id"_av, {buf, n}),
            KVLog("status"_av, result_content),
        });
    }

    void dlpav_report_file(
        std::string_view file_name,
        ReportMessageApi& report_message,
        timeval time,
        Direction direction,
        std::string_view result_content)
    {
        report_message.log6(LogId::FILE_VERIFICATION, time, {
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
                ::msgdump_c(send, from_or_to_client, total_length, flags, chunk.remaining_bytes());
            }
        };

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            /* msgType(2) + msgFlags(2) + dataLen(4) */
            if (!chunk.in_check_rem(8)) {
                LOG(LOG_INFO, "%s: total_length=%u flags=0x%08X chunk_data_length=%zu",
                    funcname, total_length, flags, chunk.in_remain());

                dump();

                LOG(LOG_ERR, "Truncated %s: expected=8 remains=%zu", funcname, chunk.in_remain());
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

    constexpr uint32_t max_len_channel_message = CHANNELS::CHANNEL_CHUNK_LENGTH;
    const char zeros_buf[max_len_channel_message] {};

    constexpr uint32_t size_of_cliprdr_header_with_stream_id
        = RDPECLIP::CliprdrHeader::size() + 4 /* streamId */;

    struct PreSender
    {
        uint32_t offset;
        uint32_t max_pkt_len;
        uint32_t remaining;
    };

    template<class GetView>
    PreSender pre_send(
        VirtualChannelDataSender& sender,
        uint32_t already_used,
        uint32_t total_pkt_size,
        GetView get_view)
    {
        PreSender pre_sender;
        pre_sender.offset = 0;
        pre_sender.max_pkt_len = max_len_channel_message - already_used;
        pre_sender.remaining = total_pkt_size - already_used;

        while (pre_sender.max_pkt_len < pre_sender.remaining) {
            sender(
                total_pkt_size,
                0,
                get_view(pre_sender.offset, pre_sender.max_pkt_len));
            pre_sender.remaining -= pre_sender.max_pkt_len;
            pre_sender.offset += pre_sender.max_pkt_len;
            pre_sender.max_pkt_len = max_len_channel_message;
        }

        return pre_sender;
    };

    template<class GetView>
    PreSender send_until_last2(
        VirtualChannelDataSender& sender,
        uint32_t already_used,
        uint32_t total_pkt_size,
        GetView get_view)
    {
        uint32_t offset = 0;
        uint32_t remaining = total_pkt_size - already_used;
        constexpr auto max_len = max_len_channel_message;
        for (; remaining > max_len; remaining -= max_len, offset += max_len) {
            sender(total_pkt_size, 0, get_view(offset, max_len));
        }
        sender(total_pkt_size, CHANNELS::CHANNEL_FLAG_LAST, get_view(offset, remaining));
    }

    void send_zero_data(
        VirtualChannelDataSender& sender,
        uint32_t already_used,
        uint32_t total_pkt_size)
    {
        auto pre_sender = pre_send(
            sender,
            already_used,
            total_pkt_size,
            [&](uint32_t /*offset*/, uint64_t len){
                return array_view{zeros_buf, len};
            });

        sender(
            total_pkt_size,
            CHANNELS::CHANNEL_FLAG_LAST,
            array_view{zeros_buf, pre_sender.remaining});
    }

    void send_until_last(
        VirtualChannelDataSender& sender,
        uint32_t already_used,
        bytes_view av,
        uint32_t total_pkt_size)
    {
        auto get_view = [&](uint32_t offset, uint32_t len){ return av.subarray(offset, len); };
        auto pre_sender = pre_send(sender, already_used, total_pkt_size, get_view);
        sender(
            total_pkt_size,
            CHANNELS::CHANNEL_FLAG_LAST,
            av.subarray(pre_sender.offset, pre_sender.remaining));
    }

    void send_partial_data(
        VirtualChannelDataSender& sender,
        uint32_t already_used,
        bytes_view av,
        uint32_t total_pkt_size)
    {
        auto get_view = [&](uint32_t offset, uint64_t len){
            return av.subarray(offset, len);
        };

        auto const full_data_len = av.size() + already_used;

        if (full_data_len < total_pkt_size) {
            auto padding = total_pkt_size - full_data_len;
            auto pre_sender = pre_send(
                sender,
                already_used,
                full_data_len,
                get_view);

            char buffer[max_len_channel_message];
            OutStream out_s({buffer, pre_sender.max_pkt_len});
            out_s.out_copy_bytes(av.from_offset(pre_sender.offset));

            if (padding <= out_s.tailroom()) {
                out_s.out_clear_bytes(padding);
                sender(
                    total_pkt_size,
                    CHANNELS::CHANNEL_FLAG_LAST,
                    out_s.get_produced_bytes());
            }
            else {
                out_s.out_clear_bytes(out_s.tailroom());
                sender(total_pkt_size, 0, out_s.get_produced_bytes());
                while (padding > max_len_channel_message) {
                    sender(total_pkt_size, 0, make_array_view(zeros_buf));
                    padding -= max_len_channel_message;
                }
                sender(
                    total_pkt_size,
                    CHANNELS::CHANNEL_FLAG_LAST,
                    make_array_view(zeros_buf).first(padding));
            }
        }
        else {
            send_until_last(
                sender,
                already_used,
                av.first(total_pkt_size - already_used),
                total_pkt_size);
        }
    }

    void send_response_data(
        VirtualChannelDataSender& sender,
        uint32_t stream_id,
        bytes_view data)
    {
        StaticOutStream<max_len_channel_message> out_stream;
        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FILECONTENTS_RESPONSE,
            RDPECLIP::CB_RESPONSE_OK,
            data.size() + 4 /* streamId */);
        header.emit(out_stream);
        out_stream.out_uint32_le(stream_id);

        if (data.size() <= out_stream.tailroom()) {
            out_stream.out_copy_bytes(data);
            sender(
                out_stream.get_offset(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
            return ;
        }

        uint32_t total_pkt_size = out_stream.get_offset() + data.size();

        out_stream.out_copy_bytes(data.first(out_stream.tailroom()));
        sender(
            total_pkt_size,
            CHANNELS::CHANNEL_FLAG_FIRST,
            out_stream.get_produced_bytes());

        send_until_last(
            sender,
            0,
            data.from_offset(out_stream.get_offset()),
            total_pkt_size);
    }
}

struct ClipboardVirtualChannel::D
{
    Direction to_direction(ClipboardVirtualChannel& self, ClipCtx const& clip) const
    {
        return (&clip == &self.server_ctx)
            ? Direction::FileFromServer
            : Direction::FileFromClient;
    }

    void _close_file_rng_tfl(
        ClipboardVirtualChannel& self,
        ClipCtx::FileContentsRange& file_rng,
        Mwrm3::TransferedStatus transfered_status)
    {
        using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;

        if (file_rng.tfl_file) {
            if (self.always_file_storage
             || ValidatorState::Failure == file_rng.validator_state
            ) {
                self.fdx_capture->close_tfl(
                    *file_rng.tfl_file,
                    file_rng.file_name,
                    transfered_status,
                    Mwrm3::Sha256Signature{file_rng.sig.digest_as_av()});
            }
            else {
                self.fdx_capture->cancel_tfl(*file_rng.tfl_file);
            }
            file_rng.tfl_file.reset();
        }
    }

    void _move_tfl_in_validator_list(
        ClipboardVirtualChannel& self,
        // TODO clip only for direction...
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng)
    {
          self.file_validator->send_eof(file_rng.file_validator_id);
          self.file_validator_list.push_back({
              file_rng.file_validator_id,
              this->to_direction(self, clip),
              std::move(file_rng.tfl_file),
              file_rng.file_name,
              file_rng.sig
          });
    }

    void _close_file_rng(
        ClipboardVirtualChannel& self,
        ClipCtx& clip,
        ClipCtx::FileContentsRange& file_rng,
        Mwrm3::TransferedStatus transfered_status)
    {
        if (bool(file_rng.file_validator_id)) {
            this->_move_tfl_in_validator_list(self, clip, file_rng);
            file_rng.file_validator_id = FileValidatorId();
        }
        else {
            this->_close_file_rng_tfl(self, file_rng, transfered_status);
        }
    }

    void broken_file_transfer(
        ClipboardVirtualChannel& self, ClipCtx& clip, ClipCtx::FileContentsRange& file_rng)
    {
        file_rng.sig.broken();
        if (bool(file_rng.file_validator_id)) {
            self.file_validator->send_abort(file_rng.file_validator_id);
        }
        this->_close_file_rng(self, clip, file_rng, Mwrm3::TransferedStatus::Broken);
    }

    void finalize_text_transfer(
        ClipboardVirtualChannel& self, ClipCtx& clip, FileValidatorId file_validator_id)
    {
        if (bool(file_validator_id)) {
            self.file_validator->send_eof(file_validator_id);
            self.text_validator_list.push_back({
                file_validator_id,
                this->to_direction(self, clip),
            });
        }
    }

    void stop_nolock_data(ClipboardVirtualChannel& self, ClipCtx& clip)
    {
        switch (clip.nolock_data)
        {
            case ClipCtx::TransferState::WaitingContinuationRange:
            case ClipCtx::TransferState::Range:
                this->broken_file_transfer(self, clip, clip.nolock_data.data);
                clip.nolock_data.init_empty();
                break;

            case ClipCtx::TransferState::Size:
                clip.nolock_data.init_empty();
                break;

            case ClipCtx::TransferState::Text:
                this->finalize_text_transfer(
                    self, clip, clip.nolock_data.data.file_validator_id);
                clip.nolock_data.init_empty();
                break;

            case ClipCtx::TransferState::RequestedRange:
                clip.nolock_data.init_empty();
                break;

            case ClipCtx::TransferState::Empty:
                break;
        }
    }

    void reset_clip(ClipboardVirtualChannel& self, ClipCtx& clip)
    {
        self.can_lock = false;
        this->stop_nolock_data(self, clip);
        for (ClipCtx::LockedData::LockedRange& locked_rng : clip.locked_data.ranges) {
            this->broken_file_transfer(self, clip, locked_rng.file_contents_range);
        }
        clip.clear();
    }

    void clip_caps(
        ClipboardVirtualChannel& self, ClipCtx& clip, bytes_view chunk_data)
    {
        this->reset_clip(self, clip);

        auto general_flags = RDPECLIP::extract_clipboard_general_flags_capability(
            chunk_data, bool(self.verbose & RDPVerbose::cliprdr));
        clip.optional_lock_id.enable(bool(general_flags & RDPECLIP::CB_CAN_LOCK_CLIPDATA));
        clip.use_long_format_names = bool(general_flags & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    }

    void monitor_ready(ClipboardVirtualChannel& self, ClipCtx& clip)
    {
        bool has_lock_support = self.can_lock;
        this->reset_clip(self, clip);
        clip.optional_lock_id.enable(has_lock_support);
    }

    bool format_list(
        ClipboardVirtualChannel& self,
        uint32_t flags, RDPECLIP::CliprdrHeader const& in_header,
        VirtualChannelDataSender* sender, bool clip_enabled,
        ClipCtx& clip, bytes_view chunk_data)
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

        this->stop_nolock_data(self, clip);

        InStream in_stream(chunk_data);
        FormatListReceive receiver(
            clip.use_long_format_names,
            in_header,
            in_stream,
            clip.current_format_list,
            self.verbose);

        clip.current_file_list_format_id = receiver.file_list_format_id;

        return true;
    }

    void format_list_response(
        RDPECLIP::CliprdrHeader const& in_header, uint32_t flags, ClipCtx& clip)
    {
        if (not check_header_response(in_header, flags)) {
            clip.current_file_list_format_id = 0;
            return ;
        }
    }

    void lock(ClipboardVirtualChannel& self, ClipCtx& clip, bytes_view chunk_data)
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

    void unlock(ClipboardVirtualChannel& self, ClipCtx& clip, bytes_view chunk_data)
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
                        this->broken_file_transfer(self, clip, rng.file_contents_range);
                        return true;
                    }
                    return false;
                });
        }

        if (clip.locked_data.requested_range.lock_id == lock_id) {
            clip.locked_data.requested_range.disable();
        }
    }

    void format_data_request(ClipCtx& clip, bytes_view chunk_data, RDPVerbose verbose)
    {
        RDPECLIP::FormatDataRequestPDU pdu;
        InStream in_stream(chunk_data);
        pdu.recv(in_stream);
        clip.requested_format_id = pdu.requestedFormatId;

        if (bool(verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }

    void format_data_response(
        ClipboardVirtualChannel& self,
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data)
    {
        if (not check_header_response(in_header, flags)) {
            clip.requested_format_id = 0;
            return ;
        }

        bool is_client_to_server = (&clip == &self.client_ctx);

        auto& current_format_list = is_client_to_server
            ? self.client_ctx.current_format_list
            : self.server_ctx.current_format_list;

        if (clip.current_file_list_format_id
         && clip.current_file_list_format_id == clip.requested_format_id
        ) {
            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                switch (clip.nolock_data)
                {
                    case ClipCtx::TransferState::WaitingContinuationRange:
                        this->broken_file_transfer(self, clip, clip.nolock_data.data);
                        clip.nolock_data.init_empty();
                        break;

                    case ClipCtx::TransferState::Size:
                    case ClipCtx::TransferState::Range:
                    case ClipCtx::TransferState::Text:
                    case ClipCtx::TransferState::RequestedRange:
                        LOG(LOG_ERR, "ClipboardVirtualChannel::process_format_data_response_pdu:"
                            " invalid state");
                        throw Error(ERR_RDP_PROTOCOL);

                    case ClipCtx::TransferState::Empty:
                        break;
                }

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

            this->log_siem_info(
                self, current_format_list, flags, in_header, true, clip.requested_format_id,
                LogSiemDataType::NoData, bytes_view{}, is_client_to_server);
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
                        switch (clip.nolock_data)
                        {
                            case ClipCtx::TransferState::WaitingContinuationRange:
                                // TODO stop or/and broken
                                this->broken_file_transfer(self, clip, clip.nolock_data.data);
                                clip.nolock_data.init_empty();
                                [[fallthrough]];

                            case ClipCtx::TransferState::Empty:
                                clip.nolock_data.init_text(
                                    self.file_validator->open_text(
                                        RDPECLIP::CF_TEXT == clip.requested_format_id
                                            ? 0u : clip.clip_text_locale_identifier,
                                        clip.validator_target_name),
                                    RDPECLIP::CF_UNICODETEXT == clip.requested_format_id);
                                break;

                            case ClipCtx::TransferState::Size:
                            case ClipCtx::TransferState::Range:
                            case ClipCtx::TransferState::RequestedRange:
                            case ClipCtx::TransferState::Text:
                                LOG(LOG_ERR, "ClipboardVirtualChannel::process_format_data_response_pdu:"
                                    " invalid state");
                                throw Error(ERR_RDP_PROTOCOL);
                        }
                    }

                    uint8_t utf8_buf[32*1024];
                    auto utf8_av = UTF16toUTF8_buf(chunk_data, make_array_view(utf8_buf));

                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        if (not utf8_av.empty() && utf8_av.back() == '\0') {
                            utf8_av = utf8_av.drop_back(1);
                        }
                    }

                    if (clip.requested_format_id == RDPECLIP::CF_UNICODETEXT) {
                        this->log_siem_info(
                            self, current_format_list, flags, in_header, false,
                            clip.requested_format_id, LogSiemDataType::Utf8, utf8_av,
                            is_client_to_server);
                    }
                    else {
                        this->log_siem_info(
                            self, current_format_list, flags, in_header, false,
                            clip.requested_format_id, LogSiemDataType::NoData, {},
                            is_client_to_server);
                    }

                    if (clip.nolock_data == ClipCtx::TransferState::Text) {
                        self.file_validator->send_data(
                            clip.nolock_data.data.file_validator_id, utf8_av);

                        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                            this->finalize_text_transfer(
                                self, clip, clip.nolock_data.data.file_validator_id);
                            clip.nolock_data.init_empty();
                        }
                    }

                    break;
                }
                case RDPECLIP::CF_LOCALE: {
                    if (flags & CHANNELS::CHANNEL_FLAG_LAST && chunk_data.size() >= 4) {
                        this->log_siem_info(
                            self, current_format_list, flags, in_header, false,
                            clip.requested_format_id, LogSiemDataType::FormatData, chunk_data,
                            is_client_to_server);
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
                    this->log_siem_info(
                        self, current_format_list, flags, in_header, false,
                        clip.requested_format_id, LogSiemDataType::FormatData, chunk_data,
                        is_client_to_server);
                    break;
            }
        }
        else {
            this->log_siem_info(
                self, current_format_list, flags, in_header, false, clip.requested_format_id,
                LogSiemDataType::FormatData, chunk_data, is_client_to_server);
        }

        if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
            clip.requested_format_id = 0;
        }
    }

    bool filecontents_request(
        ClipboardVirtualChannel& self,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data,
        VirtualChannelDataSender& sender)
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

            LOG(LOG_ERR,
                "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " Invalid lindex %u", lindex);
            return send_error();
        };

        // ignore lock if don't have CB_CAN_LOCK_CLIPDATA
        if (not self.can_lock) {
            auto init_contents_size = [&]{
                if (check_valid_lindex(clip.files)) {
                    clip.nolock_data.init_size(stream_id, ifile);
                    return true;
                }
                return false;
            };

            auto init_contents_range = [&]{
                if (check_valid_lindex(clip.files)) {
                    if (file_contents_request_pdu.position() == 0) {
                        clip.nolock_data.init_requested_range(
                            stream_id,
                            ifile,
                            file_contents_request_pdu.cbRequested(),
                            clip.files[lindex].file_size,
                            clip.files[lindex].file_name);
                        return true;
                    }

                    LOG(LOG_ERR,
                        "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                            " Unsupported random access for a FILECONTENTS_RANGE");
                    return send_error();
                }

                return false;
            };

            ClipCtx::FileContentsRange& file_rng = clip.nolock_data.data;

            switch (clip.nolock_data)
            {
                case ClipCtx::TransferState::SyncRange: {
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        if (file_rng.lindex != ifile) {
                            if (not file_rng.is_finalized()) {
                                this->broken_file_transfer(self, clip, file_rng);
                            }
                            clip.nolock_data.init_empty();
                            return init_contents_range();
                        }

                        bytes_view file_contents = file_rng.file_contents;

                        if (file_rng.is_finalized()) {
                            auto offset = std::min<uint64_t>(
                                file_contents_request_pdu.position(),
                                file_contents.size());
                            auto len = std::min<uint64_t>(
                                file_contents.size() - offset,
                                file_contents_request_pdu.cbRequested());

                            send_response_data(
                                sender, safe_int(stream_id),
                                file_contents.subarray(offset, len));
                        }
                        else if (file_contents_request_pdu.position() <= file_rng.file_offset) {
                            // TODO RequestedSyncRang state
                            auto already_known
                                = file_rng.file_offset - file_contents_request_pdu.position();
                            uint32_t offset
                                = file_contents_request_pdu.position() - already_known;
                            if (file_contents_request_pdu.cbRequested() <= already_known) {
                                send_response_data(
                                    sender, safe_int(stream_id),
                                    file_contents.subarray(
                                        offset, file_contents_request_pdu.cbRequested()));
                            }
                            else {
                                file_rng.file_offset = offset;
                            }
                        }
                        else {
                            LOG(LOG_ERR,
                                "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                    " Unsupported random access for a FILECONTENTS_RANGE (2)");
                            return send_error();
                        }

                    }
                    else {
                        if (not file_rng.is_finalized()) {
                            this->broken_file_transfer(self, clip, file_rng);
                        }
                        clip.nolock_data.init_empty();
                        return init_contents_size();
                    }
                    break;
                }

                case ClipCtx::TransferState::WaitingContinuationRange:
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        if (file_rng.file_offset == file_contents_request_pdu.position()
                         && file_rng.lindex == ifile
                        ) {
                            update_continuation_range(file_rng);
                            clip.nolock_data.set_range();
                            break;
                        }
                        else {
                            LOG(LOG_ERR,
                                "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                                    " Unsupported random access for a FILECONTENTS_RANGE");
                            this->broken_file_transfer(self, clip, file_rng);
                            clip.nolock_data.init_empty();
                            return init_contents_range();
                        }
                    }
                    else {
                        this->broken_file_transfer(self, clip, file_rng);
                        clip.nolock_data.init_empty();
                        return init_contents_size();
                    }

                case ClipCtx::TransferState::Empty:
                    if (file_contents_request_pdu.dwFlags() == RDPECLIP::FILECONTENTS_RANGE) {
                        return init_contents_range();
                    }
                    return init_contents_size();

                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::RequestedRange:
                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::Text:
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                        " double request");
                    throw Error(ERR_RDP_PROTOCOL);
            }
        }
        else if (not file_contents_request_pdu.has_optional_clipDataId()) {
            LOG(LOG_ERR, "Not lock_id with CB_CAN_LOCK_CLIPDATA");
            send_error();
            return false;
        }
        else {
            const auto lock_id = LockId(file_contents_request_pdu.clipDataId());

            auto* lock_data = clip.locked_data.search_lock_by_id(lock_id);

            if (not lock_data) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_request_pdu:"
                    " unknown clipDataId (%u)", lock_id);
                return send_error();
            }

            if (clip.locked_data.contains_stream_id(stream_id)) {
                // TODO broken_file_transfer + remove_locked_file_contents_range/size ?
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

    bool filecontents_response(
        ClipboardVirtualChannel& self,
        RDPECLIP::CliprdrHeader const& in_header,
        ClipCtx& clip, uint32_t flags, bytes_view chunk_data,
        VirtualChannelDataSender& sender)
    {
        bool send_message_is_ok = true;
        const bool is_ok = check_header_response(in_header, flags);

        InStream in_stream(chunk_data);

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            ::check_throw(in_stream, 4, "FileContentsResponse::receive", ERR_RDP_DATA_TRUNCATED);
            if (clip.has_current_file_contents_stream_id) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " streamId already defined");
                throw Error(ERR_RDP_PROTOCOL);
            }
            clip.current_file_contents_stream_id = safe_int(in_stream.in_uint32_le());
            clip.has_current_file_contents_stream_id = true;
            LOG_IF(bool(self.verbose & RDPVerbose::cliprdr), LOG_INFO,
                "File Contents Response: streamId=%u", clip.current_file_contents_stream_id);
        }
        else if (not clip.has_current_file_contents_stream_id) {
            LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                " unknown streamId");
            throw Error(ERR_RDP_PROTOCOL);
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
                ? std::unique_ptr<FdxCapture::TflFile>(new FdxCapture::TflFile(
                    self.fdx_capture->new_tfl((&clip == &self.server_ctx)
                        ? Mwrm3::Direction::ServerToClient
                        : Mwrm3::Direction::ClientToServer
                )))
                : std::unique_ptr<FdxCapture::TflFile>();
        };

        using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;

        auto update_file_size_or_throw = [&in_header, flags](
            std::vector<CliprdFileInfo>& files, FileGroupId lindex, bytes_view chunk_data
        ){
            if (!(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " Unsupported partial FILECONTENTS_SIZE packet");
                throw Error(ERR_RDP_UNSUPPORTED);
            }

            if (in_header.msgFlags() != RDPECLIP::CB_RESPONSE_OK) {
                return false;
            }

            InStream in_stream(chunk_data);
            check_throw(in_stream, 8, "process_filecontents_response_pdu", ERR_RDP_DATA_TRUNCATED);
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

            if (bool(file_rng.tfl_file)) {
                file_rng.tfl_file->trans.send(data);
            }

            if (bool(file_rng.file_validator_id)) {
                self.file_validator->send_data(
                    file_rng.file_validator_id, data);
            }

            return data;
        };

        auto set_finalize_file_transfer = [&, this](ClipCtx::FileContentsRange& file_rng){
            if (file_rng.file_offset < file_rng.file_size) {
                if (file_rng.file_size != CliprdFileInfo::invalid_size) {
                    return false;
                }
                else {
                    file_rng.file_size = file_rng.file_offset;
                }
            }

            file_rng.sig.final();
            this->log_file_info(self, file_rng, (&clip == &self.server_ctx));

            return true;
        };

        auto finalize_file_transfer = [&, this](
            ClipCtx::FileContentsRange& file_rng
        ){
            if (set_finalize_file_transfer(file_rng)) {
                this->_close_file_rng(self, clip, file_rng,
                    Mwrm3::TransferedStatus::Completed);

                return true;
            }

            return false;
        };

        auto send_file_contents_request = [&sender](
            ClipCtx::FileContentsRange& file_rng
        ){
            StaticOutStream<128> out_stream;

            RDPECLIP::FileContentsRequestPDU file_contents_request_pdu(
                safe_int(file_rng.stream_id),
                safe_int(file_rng.lindex),
                RDPECLIP::FILECONTENTS_RANGE,
                // TODO big file capabilities
                uint32_t(file_rng.file_offset),
                uint32_t(file_rng.file_offset >> 32),
                file_rng.first_file_size_requested,
                0, false);

            RDPECLIP::CliprdrHeader header(
                RDPECLIP::CB_FILECONTENTS_REQUEST,
                RDPECLIP::CB_RESPONSE_OK,
                file_contents_request_pdu.size());

            header.emit(out_stream);
            file_contents_request_pdu.emit(out_stream);

            // TODO bad sender
            sender(
                out_stream.get_produced_bytes().size(),
                CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
                out_stream.get_produced_bytes());
        };



        if (not self.can_lock) {
            auto check_stream_id = [&stream_id](StreamId current_stream_id){
                if (current_stream_id != stream_id) {
                    LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                        " invalid stream id %u, expected %u", stream_id, current_stream_id);
                    throw Error(ERR_RDP_PROTOCOL);
                }
            };

            switch (clip.nolock_data)
            {
            case ClipCtx::TransferState::Text:
            case ClipCtx::TransferState::Empty:
                break;

            case ClipCtx::TransferState::WaitingContinuationRange:
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
                    " invalid state");
                throw Error(ERR_RDP_PROTOCOL);

            case ClipCtx::TransferState::Size:
                check_stream_id(clip.nolock_data.data.stream_id);

                if (is_ok) {
                    update_file_size_or_throw(
                        clip.files, clip.nolock_data.data.lindex,
                        in_stream.remaining_bytes());
                }

                clip.nolock_data.init_empty();
                clip.has_current_file_contents_stream_id = false;

                break;

            case ClipCtx::TransferState::RequestedRange: {
                check_stream_id(clip.nolock_data.data.stream_id);

                if (is_ok) {
                    ClipCtx::FileContentsRange& file_rng = clip.nolock_data.data;

                    auto validator_id = new_file_validator_id(file_rng.file_name);

                    if (clip.verify_before_download) {
                        send_message_is_ok = false;

                        clip.nolock_data.requested_range_to_get_range(validator_id, new_tfl());

                        auto contents = update_file_range_data(
                            file_rng, in_stream.remaining_bytes());
                        append(file_rng.file_contents, contents);

                        if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                            clip.has_current_file_contents_stream_id = false;
                            if (set_finalize_file_transfer(file_rng)) {
                                self.file_validator->send_eof(file_rng.file_validator_id);
                                this->_close_file_rng_tfl(self, file_rng,
                                    Mwrm3::TransferedStatus::Completed);
                                clip.nolock_data.set_waiting_validator();
                            }
                            else {
                                send_file_contents_request(file_rng);
                            }

                            // send header without data
                            StaticOutStream<128> out_stream;
                            RDPECLIP::CliprdrHeader header(
                                RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                RDPECLIP::CB_RESPONSE_OK,
                                file_rng.file_contents.size() + 4u /* streamId */);
                            header.emit(out_stream);
                            out_stream.out_uint32_le(
                                safe_int(clip.nolock_data.data.stream_id));

                            file_rng.response_size
                                = file_rng.file_contents.size() + out_stream.get_offset();
                            sender(
                                file_rng.response_size,
                                CHANNELS::CHANNEL_FLAG_FIRST,
                                out_stream.get_produced_bytes());
                        }

                        break;
                    }
                    else {
                        clip.nolock_data.requested_range_to_range(validator_id, new_tfl());
                        [[fallthrough]];
                    }
                }
                else {
                    clip.has_current_file_contents_stream_id = false;
                    clip.nolock_data.init_empty();
                    break;
                }
            }

            case ClipCtx::TransferState::Range: {
                check_stream_id(clip.nolock_data.data.stream_id);

                if (is_ok) {
                    update_file_range_data(clip.nolock_data.data, in_stream.remaining_bytes());

                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                        clip.has_current_file_contents_stream_id = false;
                        if (finalize_file_transfer(clip.nolock_data.data)) {
                            clip.nolock_data.init_empty();
                        }
                        else {
                            clip.nolock_data.set_waiting_continuation_range();
                        }
                    }
                }
                else {
                    clip.has_current_file_contents_stream_id = false;
                    this->broken_file_transfer(self, clip, clip.nolock_data.data);
                    clip.nolock_data.init_empty();
                }

                break;
            }

            case ClipCtx::TransferState::GetRange: {
                check_stream_id(clip.nolock_data.data.stream_id);

                ClipCtx::FileContentsRange& file_rng = clip.nolock_data.data;

                auto send_fake_data = [&]{
                    send_until_last2(
                        sender,
                        size_of_cliprdr_header_with_stream_id,
                        file_rng.response_size,
                        [&](uint32_t /*offset*/, uint32_t len){
                            return bytes_view(zeros_buf, len);
                        });
                };

                auto send_file_data = [&]{
                    bytes_view file_contents = file_rng.file_contents;
                    assert(file_rng.response_size - size_of_cliprdr_header_with_stream_id
                        <= file_contents.size());
                    send_until_last2(
                        sender,
                        size_of_cliprdr_header_with_stream_id,
                        file_rng.response_size,
                        [&](uint32_t offset, uint32_t len){
                            return file_contents.subarray(offset, len);
                        });
                };

                auto post_process = [&, this](
                    ClipCtx::FileContentsRange& file_rng,
                    Mwrm3::TransferedStatus transfered_status
                ){
                    switch (file_rng.validator_state)
                    {
                        case ValidatorState::TransferAfterValidation:
                            send_file_data();
                            clip.nolock_data.set_sync_range();
                            break;

                        case ValidatorState::WaitValidatorBeforeTransfer:
                            self.file_validator->send_eof(file_rng.file_validator_id);
                            this->_close_file_rng_tfl(self, file_rng, transfered_status);
                            clip.nolock_data.set_waiting_validator();
                            break;

                        case ValidatorState::Failure:
                            this->_close_file_rng_tfl(self, file_rng, transfered_status);
                            send_fake_data();
                            clip.nolock_data.init_empty();
                            break;

                        case ValidatorState::Wait:
                        case ValidatorState::Success:
                            assert(false);
                            break;
                    }
                };

                if (is_ok) {
                    send_message_is_ok = false;

                    auto contents = update_file_range_data(
                        file_rng, in_stream.remaining_bytes());
                    append(file_rng.file_contents, contents);

                    if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                        clip.has_current_file_contents_stream_id = false;

                        if (set_finalize_file_transfer(file_rng)) {
                            file_rng.sig.final();
                            this->log_file_info(self, file_rng, (&clip == &self.server_ctx));
                            post_process(file_rng, Mwrm3::TransferedStatus::Completed);
                        }
                        else switch (file_rng.validator_state)
                        {
                            case ValidatorState::Failure:
                                this->_close_file_rng_tfl(self, file_rng,
                                    Mwrm3::TransferedStatus::Broken);
                                send_fake_data();
                                clip.nolock_data.init_empty();
                                break;

                            case ValidatorState::TransferAfterValidation:
                                send_file_data();
                                clip.nolock_data.set_sync_range();
                                break;

                            case ValidatorState::WaitValidatorBeforeTransfer:
                                send_file_contents_request(file_rng);
                                break;

                            case ValidatorState::Wait:
                            case ValidatorState::Success:
                                assert(false);
                        }
                    }
                }
                else {
                    clip.has_current_file_contents_stream_id = false;

                    file_rng.sig.broken();
                    this->log_file_info(self, file_rng, (&clip == &self.server_ctx));
                    post_process(file_rng, Mwrm3::TransferedStatus::Broken);
                }

                break;
            }
            }
        }
        else {
            auto update_locked_file_range = [&](ClipCtx::LockedData::LockedRange& locked_file){
                auto& file_rng = locked_file.file_contents_range;

                update_file_range_data(file_rng, in_stream.remaining_bytes());

                if (bool(flags & CHANNELS::CHANNEL_FLAG_LAST)) {
                    clip.has_current_file_contents_stream_id = false;

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
                            clip.verify_before_download
                                ? ValidatorState::WaitValidatorBeforeTransfer
                                : ValidatorState::Wait,
                            0,
                            req.file_size_requested,
                            checked_int(std::min(
                                uint64_t(req.file_size_requested),
                                req.file_size)),
                            req.file_size,
                            0,
                            std::move(req.file_name),
                            new_tfl(),
                            clip.verify_before_download
                                ? [&]{
                                    std::vector<uint8_t> v;
                                    v.reserve(req.file_size);
                                    return v;
                                }()
                                : std::vector<uint8_t>{}
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
            else if (auto* locked_contents_range = clip.locked_data.search_range_by_id(stream_id)) {
                if (is_ok) {
                    update_locked_file_range(*locked_contents_range);
                }
                else {
                    clip.has_current_file_contents_stream_id = false;
                    this->broken_file_transfer(self, clip, locked_contents_range->file_contents_range);
                    clip.locked_data.remove_locked_file_contents_range(locked_contents_range);
                }
            }
            else if (auto* locked_contents_size = clip.locked_data.search_size_by_id(stream_id)) {
                if (is_ok) {
                    clip.has_current_file_contents_stream_id = false;
                    not_null_ptr lock_data
                        = clip.locked_data.search_lock_by_id(locked_contents_size->lock_id);
                    update_file_size_or_throw(
                        lock_data->files, locked_contents_size->file_contents_size.lindex,
                        in_stream.remaining_bytes());
                    clip.locked_data.remove_locked_file_contents_size(locked_contents_size);
                }
                else {
                    clip.has_current_file_contents_stream_id = false;
                    clip.locked_data.remove_locked_file_contents_size(locked_contents_size);
                }
            }
            else {
                LOG(LOG_ERR, "ClipboardVirtualChannel::process_filecontents_response_pdu:"
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

    void log_file_info(
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

        self.report_message.log6(
            from_remote_session
                ? LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION
                : LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION,
            self.session_reactor.get_current_time(), {
            KVLog("file_name"_av, file_contents_range.file_name),
            KVLog("size"_av, {file_size, file_size_len}),
            KVLog("sha256"_av, {digest_s, digest_s_len}),
        });

        LOG_IF(!self.params.dont_log_data_into_syslog, LOG_INFO,
            "type=%s file_name=%s size=%s sha256=%s",
            type, file_contents_range.file_name, file_size, digest_s);
    }

    enum class LogSiemDataType : int8_t { NoData, FormatData, Utf8, };

    void log_siem_info(
        ClipboardVirtualChannel& self,
        Cliprdr::FormatNameInventory const& current_format_list, uint32_t flags,
        const RDPECLIP::CliprdrHeader & in_header, bool is_file_group_id,
        const uint32_t requestedFormatId, LogSiemDataType data_type, bytes_view data_to_dump,
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
        int format_len = std::sprintf(format_buf, "%.*s(%u)",
            int(utf8_format.size()), utf8_format.as_charp(), requestedFormatId);
        array_view format_av{format_buf, checked_cast<std::size_t>(format_len)};

        char size_buf[32];
        int size_len = std::sprintf(size_buf, "%u", in_header.dataLen());
        array_view size_av{size_buf, checked_cast<std::size_t>(size_len)};

        constexpr size_t max_length_of_data_to_dump = 256;
        char data_to_dump_buf[
            max_length_of_data_to_dump * maximum_length_of_utf8_character_in_bytes];
        std::string_view utf8_string{};
        switch (data_type)
        {
            case LogSiemDataType::NoData:
                break;
            case LogSiemDataType::FormatData:
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
                            make_array_view(data_to_dump_buf));
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
                break;
            case LogSiemDataType::Utf8:
                utf8_string = {
                    data_to_dump.as_charp(),
                    std::min(data_to_dump.size(), max_length_of_data_to_dump)
                };
                break;
        }

        if (log_current_activity) {
            if (utf8_string.empty()) {
                self.report_message.log6(is_client_to_server
                    ? LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION
                    : LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION,
                    self.session_reactor.get_current_time(), {
                    KVLog("format"_av, format_av),
                    KVLog("size"_av, size_av),
                });
            }
            else {
                self.report_message.log6(is_client_to_server
                    ? LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX
                    : LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX,
                    self.session_reactor.get_current_time(), {
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
    SessionReactor& session_reactor,
    const BaseVirtualChannel::Params & base_params,
    const ClipboardVirtualChannelParams & params,
    FileValidatorService * file_validator_service,
    FileStorage file_storage)
: BaseVirtualChannel(to_client_sender_, to_server_sender_, base_params)
// TODO decompose (extract validatorparam)
, params([&]{
    auto p = params;
    if (!file_validator_service) {
        p.validator_params.up_target_name.clear();
        p.validator_params.down_target_name.clear();
    }
    return p;
}())
, session_reactor(session_reactor)
, file_validator(file_validator_service)
, fdx_capture(file_storage.fdx_capture)
, always_file_storage(file_storage.always_file_storage)
, proxy_managed(to_client_sender_ == nullptr)
// TODO param verify_before_download
, client_ctx(this->params.validator_params.up_target_name, false)
, server_ctx(this->params.validator_params.down_target_name, false)
{}

ClipboardVirtualChannel::~ClipboardVirtualChannel()
{
    try {
        using namespace std::string_view_literals;

        auto status = "Connexion closed"sv;

        for (auto& text_validator : this->text_validator_list) {
            dlpav_report_text(
                text_validator.file_validator_id,
                this->report_message,
                this->session_reactor.get_current_time(),
                text_validator.direction, status);
        }

        for (auto& file_validator : this->file_validator_list) {
            if (file_validator.tfl_file) {
                this->fdx_capture->close_tfl(
                    *file_validator.tfl_file,
                    file_validator.file_name,
                    Mwrm3::TransferedStatus::Broken,
                    Mwrm3::Sha256Signature{file_validator.sig.digest_as_av()});
            }

            dlpav_report_file(
                file_validator.file_name,
                this->report_message,
                this->session_reactor.get_current_time(),
                file_validator.direction, status);
        }

        auto close_clip = [&](ClipCtx& clip){
            auto direction = D().to_direction(*this, clip);

            auto close_rng = [&](ClipCtx::FileContentsRange& rng){
                auto& file_name = rng.file_name;

                if (rng.tfl_file) {
                    rng.sig.broken();
                    this->fdx_capture->close_tfl(
                        *rng.tfl_file,
                        file_name,
                        Mwrm3::TransferedStatus::Broken,
                        Mwrm3::Sha256Signature{rng.sig.digest_as_av()});
                }

                if (bool(rng.file_validator_id)) {
                    dlpav_report_file(
                        file_name,
                        this->report_message,
                        this->session_reactor.get_current_time(),
                        direction, status);
                }
            };

            switch (clip.nolock_data)
            {
                case ClipCtx::TransferState::Empty:
                    break;

                case ClipCtx::TransferState::Size:
                    clip.nolock_data.init_empty();
                    break;

                case ClipCtx::TransferState::Text:
                    if (bool(clip.nolock_data.data.file_validator_id)) {
                        dlpav_report_text(
                            clip.nolock_data.data.file_validator_id,
                            this->report_message,
                            this->session_reactor.get_current_time(),
                            direction, status);
                    }
                    clip.nolock_data.init_empty();
                    break;

                case ClipCtx::TransferState::RequestedRange:
                    clip.nolock_data.init_empty();
                    break;

                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::WaitingContinuationRange: {
                    close_rng(clip.nolock_data.data);
                    clip.nolock_data.init_empty();
                    break;
                }
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
    uint32_t flags, bytes_view chunk_data,
    std::unique_ptr<AsynchronousTask> & out_asynchronous_task,
    SesmanInterface & /*sesman*/)
{
    (void)out_asynchronous_task;

    InStream chunk(chunk_data);
    RDPECLIP::CliprdrHeader header;
    bool send_message_to_client = true;

    this->server_ctx.message_type = process_header_message(
        this->server_ctx.message_type, total_length, flags, chunk, header,
        Direction::FileFromServer, this->verbose);

    switch (this->server_ctx.message_type)
    {
        case RDPECLIP::CB_CLIP_CAPS: {
            D().clip_caps(*this, this->server_ctx, chunk.remaining_bytes());
            send_message_to_client = !this->proxy_managed;
        }
        break;

        case RDPECLIP::CB_MONITOR_READY: {
            if (this->proxy_managed) {
                this->server_ctx.use_long_format_names = true;
                ServerMonitorReadySendBack sender(this->verbose, this->use_long_format_names(), this->to_server_sender_ptr());
            }

            D().monitor_ready(*this, this->server_ctx);

            if (this->clipboard_monitor_ready_notifier) {
                if (!this->clipboard_monitor_ready_notifier->on_clipboard_monitor_ready()) {
                    this->clipboard_monitor_ready_notifier = nullptr;
                }
            }

            send_message_to_client = !this->proxy_managed;
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST: {
            send_message_to_client = D().format_list(
                *this, flags, header, this->to_server_sender_ptr(),
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
            D().format_list_response(header, flags, this->client_ctx);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            D().format_data_request(this->client_ctx, chunk.remaining_bytes(), this->verbose);

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
            D().format_data_response(
                *this, header, this->server_ctx, flags, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto* sender = this->to_server_sender_ptr();
            if (sender) {
                send_message_to_client = D().filecontents_request(
                    *this, this->client_ctx, flags, chunk.remaining_bytes(), *sender);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
            auto* sender = this->to_server_sender_ptr();
            if (sender) {
                send_message_to_client = D().filecontents_response(
                    *this, header, this->server_ctx, flags, chunk.remaining_bytes(), *sender);
            }
        }
        break;

        case RDPECLIP::CB_LOCK_CLIPDATA: {
            D().lock(*this, this->client_ctx, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_UNLOCK_CLIPDATA: {
            D().unlock(*this, this->client_ctx, chunk.remaining_bytes());
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
            D().clip_caps(*this, this->client_ctx, chunk.remaining_bytes());
            this->can_lock = this->client_ctx.optional_lock_id.is_enabled()
                          && this->server_ctx.optional_lock_id.is_enabled();
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST: {
            send_message_to_server = D().format_list(
                *this, flags, header, this->to_client_sender_ptr(),
                this->params.clipboard_down_authorized
                || this->params.clipboard_up_authorized
                || this->format_list_response_notifier,
                this->client_ctx,
                chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            D().format_list_response(header, flags, this->server_ctx);
        break;

        case RDPECLIP::CB_FORMAT_DATA_REQUEST: {
            D().format_data_request(this->server_ctx, chunk.remaining_bytes(), this->verbose);
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
            D().format_data_response(
                *this, header, this->client_ctx, flags, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_REQUEST: {
            auto* sender = this->to_client_sender_ptr();
            if (sender) {
                send_message_to_server = D().filecontents_request(
                    *this, this->server_ctx, flags, chunk.remaining_bytes(), *sender);
            }
        }
        break;

        case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
            auto* sender = this->to_client_sender_ptr();
            if (sender) {
                send_message_to_server = D().filecontents_response(
                    *this, header, this->client_ctx, flags, chunk.remaining_bytes(), *sender);
            }
        }
        break;

        case RDPECLIP::CB_LOCK_CLIPDATA: {
            D().lock(*this, this->server_ctx, chunk.remaining_bytes());
        }
        break;

        case RDPECLIP::CB_UNLOCK_CLIPDATA: {
            D().unlock(*this, this->server_ctx, chunk.remaining_bytes());
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
                    this->report_message,
                    this->session_reactor.get_current_time(),
                    direction, result_content);
            }
        };

        auto process_file = [&](Direction direction, std::string_view file_name){
            if (!is_accepted || this->params.validator_params.log_if_accepted) {
                dlpav_report_file(
                    file_name,
                    this->report_message,
                    this->session_reactor.get_current_time(),
                    direction, result_content);
            }
        };

        if (auto* text_validator_data = this->search_text_validator_by_id(file_validator_id)) {
            process_text(text_validator_data->direction);
            text_validator_data->file_validator_id = FileValidatorId();
            this->remove_text_validator(text_validator_data);
        }
        else if (auto* file_validator_data = this->search_file_validator_by_id(file_validator_id)) {
            process_file(file_validator_data->direction, file_validator_data->file_name);

            if (file_validator_data->tfl_file) {
                if (this->always_file_storage || not is_accepted) {
                    auto status = Mwrm3::TransferedStatus::Completed;
                    if (!file_validator_data->sig.has_digest()) {
                        file_validator_data->sig.broken();
                        status = Mwrm3::TransferedStatus::Broken;
                    }
                    this->fdx_capture->close_tfl(
                        *file_validator_data->tfl_file,
                        file_validator_data->file_name,
                        status, Mwrm3::Sha256Signature{
                            file_validator_data->sig.digest_as_av()
                        });
                }
                else {
                    this->fdx_capture->cancel_tfl(*file_validator_data->tfl_file);
                }
            }

            this->remove_file_validator(file_validator_data);
        }
        else {
            auto process_clip = [&](ClipCtx& clip){
                switch (clip.nolock_data)
                {
                case ClipCtx::TransferState::Empty:
                case ClipCtx::TransferState::Size:
                case ClipCtx::TransferState::RequestedRange:
                    break;

                case ClipCtx::TransferState::Text:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        process_text(D().to_direction(*this, clip));
                        clip.nolock_data.init_empty();
                        return true;
                    }
                    break;

                case ClipCtx::TransferState::Range:
                case ClipCtx::TransferState::WaitingContinuationRange:
                    if (clip.nolock_data.data.file_validator_id == file_validator_id) {
                        auto& file_rng = clip.nolock_data.data;
                        process_file(D().to_direction(*this, clip), file_rng.file_name);
                        using ValidatorState = ClipCtx::FileContentsRange::ValidatorState;
                        if (is_accepted) {
                            if (file_rng.validator_state
                                == ValidatorState::WaitValidatorBeforeTransfer
                            ) {
                                // TODO send data
                                file_rng.validator_state
                                    = ValidatorState::TransferAfterValidation;
                            }
                            else {
                                file_rng.validator_state = ValidatorState::Success;

                            }
                        }
                        else {
                            file_rng.validator_state = ValidatorState::Failure;
                        }
                        file_rng.file_validator_id = FileValidatorId();
                        return true;
                    }
                    break;
                }

                if (auto* r = clip.locked_data.search_range_by_validator_id(file_validator_id)) {
                    process_file(D().to_direction(*this, clip), r->file_contents_range.file_name);
                    r->file_contents_range.file_validator_id = FileValidatorId();
                    return true;
                }

                return false;
            };

            if (not process_clip(this->client_ctx) && not process_clip(this->server_ctx)) {
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

    const size_t totalLength = out_s.get_offset();

    this->send_message_to_server(
        totalLength,
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        out_s.get_produced_bytes());
}

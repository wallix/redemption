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

#include "capture/fdx_capture.hpp"
#include "core/channel_list.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/file_validator_service.hpp"
#include "utils/stream.hpp"
#include "system/ssl_sha256.hpp"

#include <vector>


struct ClipboardSideData
{
    uint16_t current_message_type = 0;
    bool use_long_format_names = false;
    uint32_t file_list_format_id = 0;
    FileValidatorId clip_text_id {};
    // https://docs.microsoft.com/en-us/windows/win32/intl/language-identifier-constants-and-strings
    uint32_t clip_text_locale_identifier = 0;
    StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

    enum class StreamId : uint32_t;
    enum class FileGroupId : uint32_t;

    StreamId file_contents_stream_id {};

    struct FileContent
    {
        // if not stream_id then file_data has value

        // TODO CLIPRDR
        StreamId stream_id;
        FileGroupId file_group_id;

        enum class Status : uint8_t
        {
            // *WithId: file_data with optional clip_data_id
            // WaitContinuation*: file transfered to several packet
            WaitValidator,
            WaitDataWithId,
            WaitData,
            WaitContinuationWithId,
            WaitContinuation,
            IsSize,
        };

        Status status;

        struct FileData
        {
            FileValidatorId file_validator_id;
            uint32_t clip_data_id;
            bool active_lock;

            std::string file_name;
            uint64_t file_size;
            uint64_t file_offset;
            uint64_t file_size_requested;

            std::unique_ptr<FdxCapture::TflFile> tfl_file;

            struct Sig
            {
                Sig()
                {
                    this->sha256.init();
                }

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

                SslSha256_Delayed sha256;
                uint8_t array[digest_len];
                Status status = Status::Update;
            };

            Sig sig;

            bool on_failure = false;
        };

        FileData file_data;

        [[nodiscard]] bool is_file_range() const
        {
            return !is_file_size();
        }

        [[nodiscard]] bool is_file_size() const
        {
            return this->status == Status::IsSize;
        }

        void set_wait_validator()
        {
            this->status = Status::WaitValidator;
        }

        [[nodiscard]] bool is_wait_validator() const
        {
            return this->status == Status::WaitValidator;
        }

        bytes_view receive_data(bytes_view data)
        {
            assert(this->status == Status::WaitData || this->status == Status::WaitDataWithId);

            if (data.size() >= this->file_data.file_size_requested) {
                data = data.first(this->file_data.file_size_requested);
            }

            this->file_data.sig.update(data);
            this->file_data.file_offset += data.size();
            this->file_data.file_size_requested -= data.size();

            if (!this->file_data.file_size_requested) {
                if (this->file_data.file_offset == this->file_data.file_size) {
                    this->file_data.sig.final();
                    this->status = Status::WaitValidator;
                }
                else {
                    if (this->status == Status::WaitData) {
                        this->status = Status::WaitContinuation;
                    }
                    else if (this->status == Status::WaitDataWithId) {
                        this->status = Status::WaitContinuationWithId;
                    }
                }
            }

            return data;
        }

        void update_requested(uint64_t file_size_requested)
        {
            assert(this->status == Status::WaitContinuation || this->status == Status::WaitContinuationWithId);

            if (this->status == Status::WaitContinuation) {
                this->status = Status::WaitData;
            }
            else if (this->status == Status::WaitContinuationWithId) {
                this->status = Status::WaitDataWithId;
            }
            else {
                assert(false);
            }

            uint64_t offset_end = this->file_data.file_offset + file_size_requested;
            uint64_t size_after_requested = std::min(this->file_data.file_size, offset_end);
            this->file_data.file_size_requested = size_after_requested - this->file_data.file_offset;
        }
    };

    std::vector<FileValidatorId> clip_text_id_list;
    std::vector<FileContent> file_contents_list;
    std::vector<uint32_t> lock_id_list;

private:
    [[nodiscard]] auto _find_lock_id_it(uint32_t id) const
    {
        return std::find(this->lock_id_list.begin(), this->lock_id_list.end(), id);
    }

public:
    bool remove_text_id(FileValidatorId file_validator_id)
    {
        auto it = std::find(this->clip_text_id_list.begin(), this->clip_text_id_list.end(),
            file_validator_id);
        if (it != this->clip_text_id_list.end()) {
            *it = std::move(this->clip_text_id_list.back());
            this->clip_text_id_list.pop_back();
            return true;
        }
        return false;
    }

    void push_clip_text_to_list()
    {
        this->clip_text_id_list.push_back(this->clip_text_id);
        this->clip_text_id = FileValidatorId();
    }

    [[nodiscard]] bool has_lock_id(uint32_t id) const
    {
        return this->_find_lock_id_it(id) != this->lock_id_list.end();
    }

    void push_lock_id(uint32_t id)
    {
        if (!this->has_lock_id(id)) {
            this->lock_id_list.push_back(id);
            for (auto& file : this->file_contents_list) {
                if ((
                    file.status == FileContent::Status::WaitDataWithId
                 || file.status == FileContent::Status::WaitContinuationWithId
                ) && file.file_data.clip_data_id == id) {
                    file.file_data.active_lock = true;
                }
            }
        }
    }

    void remove_lock_id(uint32_t id)
    {
        auto pos = this->_find_lock_id_it(id);
        if (pos != this->lock_id_list.end()) {
            this->lock_id_list.erase(pos);
            for (auto& file : this->file_contents_list) {
                if ((
                    (file.status == FileContent::Status::WaitDataWithId)
                 || file.status == FileContent::Status::WaitContinuationWithId
                ) && file.file_data.clip_data_id == id
                ) {
                    if (file.status == FileContent::Status::WaitDataWithId) {
                        file.status = FileContent::Status::WaitData;
                    }
                    else {
                        file.status = FileContent::Status::WaitContinuation;
                    }
                    file.file_data.active_lock = false;
                }
            }
        }
    }

    void push_file_content_size(StreamId stream_id, FileGroupId file_group_id)
    {
        this->file_contents_list.push_back({stream_id, file_group_id, FileContent::Status::IsSize, {}});
    }

    void push_file_content_range(
        StreamId stream_id, FileGroupId file_group_id,
        bool has_clip_data_id, uint32_t clip_data_id,
        FileValidatorId file_validator_id, std::unique_ptr<FdxCapture::TflFile>&& tfl_file,
        std::string const& filename, uint64_t filesize, uint64_t file_size_requested)
    {
        bool active_lock = (has_clip_data_id && this->has_lock_id(clip_data_id));
        this->file_contents_list.push_back({
            stream_id, file_group_id, has_clip_data_id
                ? FileContent::Status::WaitDataWithId
                : FileContent::Status::WaitData,
            FileContent::FileData{
                file_validator_id, clip_data_id, active_lock, filename, filesize, 0,
                std::min(file_size_requested, filesize), std::move(tfl_file), {}
            }
        });
    }

    void remove_file(FileContent* file)
    {
        assert(file);
        auto n = std::size_t(file - this->file_contents_list.data());
        if (n+1u != this->file_contents_list.size()) {
            this->file_contents_list[n] = std::move(this->file_contents_list.back());
        }
        this->file_contents_list.pop_back();
    }

    FileContent* find_file_by_offset(FileGroupId file_group_id, uint64_t offset)
    {
        for (auto& file : this->file_contents_list) {
            if (file.file_group_id == file_group_id
             && file.file_data.file_offset == offset
             && file.status == FileContent::Status::WaitContinuation
            ) {
                return &file;
            }
        }
        return nullptr;
    }

    FileContent* find_file_by_stream_id(StreamId stream_id)
    {
        for (auto& file : this->file_contents_list) {
            if (file.stream_id == stream_id && (
                file.status == FileContent::Status::WaitDataWithId
             || file.status == FileContent::Status::WaitData
             || file.status == FileContent::Status::IsSize
            )) {
                return &file;
            }
        }
        return nullptr;
    }

    FileContent* find_continuation_stream_id(StreamId stream_id)
    {
        for (auto& file : this->file_contents_list) {
            if (file.stream_id == stream_id && file.file_data.active_lock && (
                file.status == FileContent::Status::WaitContinuationWithId
             || file.status == FileContent::Status::WaitDataWithId
            )) {
                return &file;
            }
        }
        return nullptr;
    }

    FileContent* find_file_by_file_validator_id(FileValidatorId file_validator_id)
    {
        for (auto& file : this->file_contents_list) {
            if (file.file_data.file_validator_id == file_validator_id) {
                return &file;
            }
        }
        return nullptr;
    }

    std::vector<FileContent>& get_file_contents_list() noexcept
    {
        return this->file_contents_list;
    }
};

struct ClipboardData
{
    ClipboardSideData server_data;
    ClipboardSideData client_data;

    uint32_t requestedFormatId = 0;
};

struct ClipboardCapabilitiesReceive
{
    ClipboardCapabilitiesReceive(ClipboardSideData& side_data, InStream& chunk, const RDPVerbose verbose)
    {
        // cCapabilitiesSets(2) +
        // pad1(2)
        check_throw(chunk, 4, "CLIPRDR_CAPS", ERR_RDP_DATA_TRUNCATED);

        const uint16_t cCapabilitiesSets = chunk.in_uint16_le();
        assert(1 == cCapabilitiesSets);

        chunk.in_skip_bytes(2); // pad1(2)

        for (uint16_t i = 0; i < cCapabilitiesSets; ++i) {
            RDPECLIP::CapabilitySetRecvFactory f(chunk);

            if (f.capabilitySetType() == RDPECLIP::CB_CAPSTYPE_GENERAL) {
                RDPECLIP::GeneralCapabilitySet general_caps;

                general_caps.recv(chunk, f);

                if (bool(verbose & RDPVerbose::cliprdr)) {
                    general_caps.log(LOG_INFO);
                }

                side_data.use_long_format_names =
                    bool(general_caps.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
            }
        }
    }
};

struct FilecontentsRequestSendBack
{
    FilecontentsRequestSendBack(uint32_t dwFlags, uint32_t streamID, VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        StaticOutStream<256> out_stream;

        switch (dwFlags)
        {
            case RDPECLIP::FILECONTENTS_RANGE:
            {
                RDPECLIP::FileContentsResponseRange pdu(streamID);
                RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                RDPECLIP::CB_RESPONSE_FAIL,
                                                pdu.packet_size());
                header.emit(out_stream);
                pdu.emit(out_stream);
            }
            break;

            case RDPECLIP::FILECONTENTS_SIZE:
            {
                RDPECLIP::FileContentsResponseSize pdu(streamID, 0);
                RDPECLIP::CliprdrHeader header( RDPECLIP::CB_FILECONTENTS_RESPONSE,
                                                RDPECLIP::CB_RESPONSE_FAIL,
                                                pdu.packet_size());
                header.emit(out_stream);
                pdu.emit(out_stream);
            }
            break;
        }

        sender->operator()(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_bytes());
    }
};

struct FormatDataRequestReceive
{
    FormatDataRequestReceive(ClipboardData & clip_data, const RDPVerbose verbose, InStream& chunk) {
        RDPECLIP::FormatDataRequestPDU pdu;
        pdu.recv(chunk);
        clip_data.requestedFormatId = pdu.requestedFormatId;

        if (bool(verbose & RDPVerbose::cliprdr)) {
            pdu.log();
        }
    }
};

struct FormatDataRequestSendBack
{
    FormatDataRequestSendBack(VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_FAIL, 0);
        header.emit(out_stream);

        sender->operator()(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_bytes());
    }
};

struct CliprdFileInfo
{
    uint64_t file_size;
    std::string file_name;
};

struct FormatDataResponseReceiveFileList
{
    FormatDataResponseReceiveFileList(std::vector<CliprdFileInfo>& files, InStream & chunk, const RDPECLIP::CliprdrHeader & in_header, bool param_dont_log_data_into_syslog, const uint32_t file_list_format_id, const uint32_t flags, OutStream & file_descriptor_stream, const RDPVerbose verbose, char const* direction)
    {
        auto receive_file = [&](InStream& in_stream){
            RDPECLIP::FileDescriptor fd;

            fd.receive(in_stream);

            if (bool(verbose & RDPVerbose::cliprdr)) {
                fd.log(LOG_INFO);
            }

            files.push_back(CliprdFileInfo{fd.file_size(), std::move(fd.file_name)});
        };

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            if (!(in_header.msgFlags() & RDPECLIP::CB_RESPONSE_FAIL) && (in_header.dataLen() >= 4 /* cItems(4) */)) {
                const uint32_t cItems = chunk.in_uint32_le();

                if (!param_dont_log_data_into_syslog) {
                    LOG(LOG_INFO,
                        "Sending %sFileGroupDescriptorW(%u) clipboard data to %s. "
                            "cItems=%u",
                        ((flags & CHANNELS::CHANNEL_FLAG_LAST) ? "" : "(chunked) "),
                        file_list_format_id, direction, cItems);
                }
            }
        }
        else if (file_descriptor_stream.get_offset()) {
            const uint32_t complementary_data_length =
                RDPECLIP::FileDescriptor::size() -
                    file_descriptor_stream.get_offset();

            assert(chunk.in_remain() >= complementary_data_length);

            file_descriptor_stream.out_copy_bytes(chunk.get_current(),
                complementary_data_length);

            chunk.in_skip_bytes(complementary_data_length);

            InStream in_stream(file_descriptor_stream.get_bytes());

            receive_file(in_stream);

            file_descriptor_stream.rewind();
        }

        while (chunk.in_remain() >= RDPECLIP::FileDescriptor::size()) {
            receive_file(chunk);
        }

        if (chunk.in_remain()) {
            file_descriptor_stream.rewind();
            file_descriptor_stream.out_copy_bytes(chunk.in_skip_bytes(chunk.in_remain()));
        }
    }
};

struct FormatDataResponseReceive
{
    std::string data_to_dump;

    FormatDataResponseReceive(const uint32_t requestedFormatId, InStream & chunk, const uint32_t flags)
    {
        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

            constexpr size_t const max_length_of_data_to_dump = 256;

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
                    assert(!(chunk.in_remain() & 1));

                    const size_t length_of_data_to_dump = std::min(
                        chunk.in_remain(), max_length_of_data_to_dump * 2);

                    constexpr size_t size_of_utf8_string =
                        max_length_of_data_to_dump *
                            maximum_length_of_utf8_character_in_bytes;

                    uint8_t utf8_string[size_of_utf8_string + 1] {};
                    const size_t length_of_utf8_string = ::UTF16toUTF8(
                        chunk.get_current(), length_of_data_to_dump / 2,
                        utf8_string, size_of_utf8_string);
                    this->data_to_dump.assign(
                        ::char_ptr_cast(utf8_string),
                        ((length_of_utf8_string && !utf8_string[length_of_utf8_string - 1]) ?
                            length_of_utf8_string - 1 :
                            length_of_utf8_string));
                }
                break;

                case RDPECLIP::CF_LOCALE:
                {
                    const uint32_t locale_identifier = chunk.in_uint32_le();
                    this->data_to_dump = std::to_string(locale_identifier);
                }
                break;
            }
        }
    }
};


struct ServerMonitorReadySendBack
{
    ServerMonitorReadySendBack(const RDPVerbose verbose, const bool use_long_format_names, VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                "Send Clipboard Capabilities PDU.");

        RDPECLIP::GeneralCapabilitySet general_cap_set(
            RDPECLIP::CB_CAPS_VERSION_1,
            RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
        RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1);
        RDPECLIP::CliprdrHeader caps_clipboard_header(RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE__NONE_,
            clipboard_caps_pdu.size() + general_cap_set.size());

        StaticOutStream<1024> caps_stream;

        caps_clipboard_header.emit(caps_stream);
        clipboard_caps_pdu.emit(caps_stream);
        general_cap_set.emit(caps_stream);

        sender->operator()(
            caps_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            caps_stream.get_bytes());

        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "ClipboardVirtualChannel::process_server_monitor_ready_pdu: "
                "Send Format List PDU.");

        StaticOutStream<256> list_stream;
        Cliprdr::format_list_serialize_with_header(
            list_stream, Cliprdr::IsLongFormat(use_long_format_names),
            std::array{Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}}});

        sender->operator()(
            list_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            list_stream.get_bytes());
    }
};


struct FormatListReceive
{
    uint32_t file_list_format_id = 0;

    FormatListReceive(
        const bool use_long_format,
        const RDPECLIP::CliprdrHeader & in_header,
        InStream & chunk,
        Cliprdr::FormatNameInventory& format_name_inventory,
        const RDPVerbose verbose)
    {
        LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
            "%s variant of Format List PDU is used for exchanging updated format names.",
            use_long_format
                ? "Long Format Name"
            : (in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES)
                ? "Short Format Name (ASCII 8)"
                : "Short Format Name"
        );

        auto buf = chunk.remaining_bytes();
        InStream in_stream(buf.first(std::min<size_t>(in_header.dataLen(), buf.size())));

        Cliprdr::format_list_extract(
            in_stream,
            Cliprdr::IsLongFormat(use_long_format),
            Cliprdr::IsAscii(in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES),
            [&](uint32_t format_id, auto name) {
                auto&& format_name = format_name_inventory.push(format_id, name);
                auto&& utf8_name = format_name.utf8_name();

                LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
                    "formatId=%s(%u) wszFormatName=\"%.*s\"",
                    RDPECLIP::get_FormatId_name(format_id),
                    format_id, int(utf8_name.size()), utf8_name.data());

                if (ranges_equal(utf8_name, Cliprdr::formats::file_group_descriptor_w.ascii_name)) {
                    this->file_list_format_id = format_id;
                }
            }
        );

        chunk.in_skip_bytes(in_stream.get_offset());

        // some version of server add "\0\0\0\0" and total_len == chunk_size + 4
        LOG_IF(chunk.in_remain() != 4 && chunk.in_remain() != 0,
            LOG_WARNING, "Truncated CLIPRDR_FORMAT_LIST remains=%zu", chunk.in_remain());
    }
};

struct FormatListSendBack
{
    FormatListSendBack(VirtualChannelDataSender* sender)
    {
        if (!sender) {
            return ;
        }

        RDPECLIP::FormatListResponsePDU pdu;

        RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, pdu.size());

        StaticOutStream<256> out_stream;

        header.emit(out_stream);
        pdu.emit(out_stream);

        sender->operator()(
            out_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
            out_stream.get_bytes());
    }
};

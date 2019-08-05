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
#include "core/RDP/clipboard.hpp"
#include "mod/rdp/channels/base_channel.hpp"
#include "mod/rdp/channels/sespro_launcher.hpp"
#include "mod/icap_files_service.hpp"
#include "utils/stream.hpp"
#include "system/ssl_sha256.hpp"

#include <vector>


struct ClipboardSideData
{
    uint16_t message_type = 0;
    bool use_long_format_names = false;
    // TODO
    uint32_t clipDataId = 0;
    uint32_t file_list_format_id = 0;
    StaticOutStream<RDPECLIP::FileDescriptor::size()> file_descriptor_stream;

    // TODO
    std::string provider_name;

    uint32_t last_stream_id = 0;

    ClipboardSideData(std::string provider_name)
      : provider_name(std::move(provider_name))
    {}

    enum class StreamId : uint32_t;
    enum class FileGroupId : uint32_t;

    struct FileContent
    {
        // if not stream_id then file_data has value

        // TODO CLIPRDR
        StreamId stream_id;
        // TODO ICAPFileId -> FileValidatorId
        FileGroupId file_group_id;

        // TODO type
        enum class Status : uint8_t
        {
            // WithId: file_data with optional clip_data_id
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
            ICAPFileId validator_id;
            uint32_t clip_data_id;
            bool active_lock;

            std::string file_name;
            uint64_t file_size;
            uint64_t file_offset;
            uint64_t file_size_requested;

            SslSha256_Delayed sha256;
        };

        FileData file_data;

        bool is_file_range() const
        {
            return !is_file_size();
        }

        bool is_file_size() const
        {
            return this->status == Status::IsSize;
        }

        void set_wait_validator()
        {
            this->status = Status::WaitValidator;
        }

        bool is_wait_validator() const
        {
            return this->status == Status::WaitValidator;
        }

        const_bytes_view receive_data(const_bytes_view data)
        {
            assert(this->status == Status::WaitData || this->status == Status::WaitDataWithId);

            if (data.size() >= this->file_data.file_size_requested) {
                data = data.first(this->file_data.file_size_requested);
            }

            this->file_data.sha256.update(data);
            this->file_data.file_offset += data.size();
            this->file_data.file_size_requested -= data.size();

            if (!this->file_data.file_size_requested) {
                if (this->file_data.file_offset == this->file_data.file_size) {
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

    std::vector<FileContent> file_contents_list;
    std::vector<uint32_t> lock_id_list;

private:
    auto _find_lock_id_it(uint32_t id)
    {
        return std::find(this->lock_id_list.begin(), this->lock_id_list.end(), id);
    }

public:
    void push_lock_id(uint32_t id)
    {
        if (this->_find_lock_id_it(id) == this->lock_id_list.end()) {
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
        bool has_clip_data_id, uint32_t clip_data_id, ICAPFileId validator_id,
        std::string const& filename, uint64_t filesize, uint64_t file_size_requested)
    {
        bool active_lock = false;
        if (has_clip_data_id) {
            if (this->_find_lock_id_it(clip_data_id) != this->lock_id_list.end()) {
                active_lock = true;
            }
        }
        this->file_contents_list.push_back({
            stream_id, file_group_id, has_clip_data_id
                ? FileContent::Status::WaitDataWithId
                : FileContent::Status::WaitData,
            FileContent::FileData{
                validator_id, clip_data_id, active_lock, filename, filesize, 0,
                std::min(file_size_requested, filesize), {}
            }
        });
        this->file_contents_list.back().file_data.sha256.init();
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

    FileContent* find_file_by_validator_id(ICAPFileId validator_id)
    {
        for (auto& file : this->file_contents_list) {
            if (file.file_data.validator_id == validator_id) {
                return &file;
            }
        }
        return nullptr;
    }
};

struct ClipboardData
{
    ClipboardSideData server_data {"server"};
    ClipboardSideData client_data {"client"};

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

        RDPECLIP::FormatListPDUEx format_list_pdu;
        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

        RDPECLIP::CliprdrHeader list_clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
            format_list_pdu.size(use_long_format_names));

        StaticOutStream<256> list_stream;

        list_clipboard_header.emit(list_stream);
        format_list_pdu.emit(list_stream, use_long_format_names);

        sender->operator()(
            list_stream.get_offset(),
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL,
            list_stream.get_bytes());
    }
};

// TODO copy from /browser_client_JS/src/red_channels/clipboard.cpp
namespace Cliprdr
{
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
    // TODO string_literal<cs...>::view()
    template<class C, C... cs>
    constexpr std::array<uint8_t, sizeof...(cs) * 2> const operator "" _utf16()
    {
        std::array<uint8_t, sizeof...(cs) * 2> a{};
        char s[] {cs...};
        auto p = a.data();
        for (char c : s)
        {
            p[0] = c;
            p[1] = 0;
            p += 2;
        }
        return a;
    }
    REDEMPTION_DIAGNOSTIC_POP

    inline namespace format_name_constants
    {
        constexpr auto file_group_descriptor_w_utf8 = "FileGroupDescriptorW"_av;
        constexpr auto file_group_descriptor_w_utf16 = "FileGroupDescriptorW"_utf16;

        constexpr auto preferred_drop_effect_utf8 = "Preferred DropEffect"_av;
        constexpr auto preferred_drop_effect_utf16 = "Preferred DropEffect"_av;
    }

    enum class Charset : bool
    {
        Ascii,
        Utf16,
    };

    struct FormatListExtractorData
    {
        uint32_t format_id;
        Charset charset;
        cbytes_view av_name;
    };

    enum class IsLongFormat : bool;
    // TODO Charset ?
    enum class IsAscii : bool;

    // formatId(4) + wszFormatName(variable, min = "\x00\x00" => 2)
    constexpr size_t min_long_format_name_data_length = 6;

    // formatId(4) + formatName(32)
    constexpr size_t short_format_name_length = 32;
    constexpr size_t short_format_name_data_length = short_format_name_length + 4;

    constexpr size_t format_name_data_length[] = {
        short_format_name_data_length,
        min_long_format_name_data_length
    };

    enum class ExtractResult : int8_t
    {
        Ok,
        WaitingData,
        LongFormatNameTooLong,
    };

    inline ExtractResult format_list_extract(
        FormatListExtractorData& data, InStream& stream,
        IsLongFormat is_long_format, IsAscii is_ascii) noexcept
    {
        if (stream.in_remain() < format_name_data_length[int(is_long_format)])
        {
            return ExtractResult::WaitingData;
        }

        data.format_id = stream.in_uint32_le();

        if (bool(is_long_format))
        {
            data.charset = Charset::Utf16;
            auto av = stream.remaining_bytes();
            data.av_name = av.first(UTF16ByteLen(av));

            if (data.av_name.size() == av.size())
            {
                return ExtractResult::LongFormatNameTooLong;
            }

            stream.in_skip_bytes(data.av_name.size() + 2);
        }
        else
        {
            auto av = stream.remaining_bytes();
            if (bool(is_ascii))
            {
                data.charset = Charset::Ascii;
                data.av_name = av.first(strnlen(av.as_charp(), short_format_name_length));
            }
            else
            {
                data.charset = Charset::Utf16;
                data.av_name = av.first(UTF16ByteLen(av.first(short_format_name_length)));
            }

            stream.in_skip_bytes(short_format_name_length);
        }

        return ExtractResult::Ok;
    }

    inline bool format_list_serialize(
        OutStream& out_stream, cbytes_view name,
        uint32_t id, IsLongFormat is_long_format, Charset charset)
    {
        constexpr size_t header_length = 4;

        if (bool(is_long_format))
        {
            switch (charset)
            {
            case Charset::Ascii: {
                if (!out_stream.has_room(header_length + name.size() * 2 + 2))
                {
                    return false;
                }
                out_stream.out_uint32_le(id);

                auto data = out_stream.get_tailroom_bytes();
                out_stream.out_skip_bytes(UTF8toUTF16(name, data.first(data.size() - 2u)));
                out_stream.out_uint16_le(0);
                break;
            }
            case Charset::Utf16: {
                if (!out_stream.has_room(header_length + name.size() + 2))
                {
                    return false;
                }
                out_stream.out_uint32_le(id);
                out_stream.out_copy_bytes(name);
                out_stream.out_uint16_le(0);
                break;
            }
            }
        }
        else
        {
            if (!out_stream.has_room(4 + short_format_name_length))
            {
                return false;
            }

            out_stream.out_uint32_le(id);

            switch (charset)
            {
            case Charset::Ascii: {
                auto data = out_stream.get_tailroom_bytes();
                auto len = std::min(data.size(), short_format_name_length-2u);
                out_stream.out_skip_bytes(UTF8toUTF16(name, data.first(len)));
                char zero_data[short_format_name_length]{};
                out_stream.out_copy_bytes(zero_data, short_format_name_length - len);
                break;
            }
            case Charset::Utf16: {
                auto len = std::min(name.size(), short_format_name_length-1u);
                out_stream.out_copy_bytes(name);
                char zero_data[short_format_name_length]{};
                out_stream.out_copy_bytes(zero_data, short_format_name_length - len);
                break;
            }
            }
        }

        return true;
    }

    struct FormatNameInventory
    {
        using FormatId = uint32_t;
        struct FormatName;

        FormatName const& push(FormatId format_id, Charset charset, array_view_const_u8 av_name)
        {
            return this->formats.emplace_back(format_id, charset, av_name);
        }

        FormatName const* find(FormatId format_id) const noexcept
        {
            for (auto const& format : this->formats) {
                if (format.format_id == format_id) {
                    return &format;
                }
            }
            return nullptr;
        }

        void clear() noexcept
        {
            this->formats.clear();
        }

        struct FormatName
        {
            static constexpr size_t utf8_buffer_buf_len = 123;

            uint32_t format_id;
            // TODO static_vector<raw_buf_len>
            uint8_t len;
            uint8_t utf8_buffer[utf8_buffer_buf_len];

            FormatName(FormatId format_id, Charset charset, array_view_const_u8 raw_name) noexcept
            : format_id(format_id)
            , len(std::min(raw_name.size(), std::size(this->utf8_buffer)))
            {
                if (charset == Charset::Ascii)
                {
                    this->len = std::min(raw_name.size(), std::size(this->utf8_buffer));
                    memcpy(this->utf8_buffer, raw_name.data(), this->len);
                }
                else
                {
                    this->len = UTF16toUTF8_buf(
                        raw_name, make_array_view(this->utf8_buffer)).size();
                }
            }

            FormatName(FormatName const& other) noexcept
            : format_id(other.format_id)
            , len(other.len)
            {
                memcpy(this->utf8_buffer, other.utf8_buffer, other.len);
            }

            FormatName& operator=(FormatName const& other) noexcept
            {
                this->format_id = other.format_id;
                this->len = other.len;
                memcpy(this->utf8_buffer, other.utf8_buffer, other.len);
                return *this;
            }

            const_bytes_view utf8_name() const noexcept
            {
                return {this->utf8_buffer, this->len};
            }

            bool utf8_name_equal(const_bytes_view utf8_name_compared) const noexcept
            {
                return utf8_name_compared.size() == this->len
                    && 0 == strncasecmp(utf8_name_compared.as_charp(),
                                        char_ptr_cast(this->utf8_buffer), this->len);
            }
        };

    private:
        std::vector<FormatName> formats;
    };
} // namespace Cliprdr


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

        Cliprdr::FormatListExtractorData extracted_data;

        auto buf = chunk.remaining_bytes();
        InStream in_stream(buf.first(std::min<size_t>(in_header.dataLen(), buf.size())));

        Cliprdr::ExtractResult r;

        while ((r = Cliprdr::format_list_extract(
            extracted_data, in_stream,
            Cliprdr::IsLongFormat(use_long_format),
            Cliprdr::IsAscii(in_header.msgFlags() & RDPECLIP::CB_ASCII_NAMES))
        ) == Cliprdr::ExtractResult::Ok)
        {
            auto&& format_name = format_name_inventory.push(
                extracted_data.format_id, extracted_data.charset, extracted_data.av_name);
            auto&& utf8_name = format_name.utf8_name();

            LOG_IF(bool(verbose & RDPVerbose::cliprdr), LOG_INFO,
                "formatId=%s(%u) wszFormatName=\"%.*s\"",
                RDPECLIP::get_FormatId_name(extracted_data.format_id),
                extracted_data.format_id, int(utf8_name.size()), utf8_name.data());

            if (format_name.utf8_name_equal(Cliprdr::file_group_descriptor_w_utf8)) {
                this->file_list_format_id = extracted_data.format_id;
            }
        }

        chunk.in_skip_bytes(in_stream.get_offset());

        if (r == Cliprdr::ExtractResult::LongFormatNameTooLong) {
            LOG(LOG_ERR, "Truncated CLIPRDR_FORMAT_LIST (Long) FormatName");
            throw Error(ERR_RDP_UNSUPPORTED);
        }

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

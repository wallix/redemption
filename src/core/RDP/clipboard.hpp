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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include "core/FSCC/FileInformation.hpp"
#include "core/WMF/MetaFileFormat.hpp"
#include "core/channel_list.hpp"
#include "core/error.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/overload.hpp"

#include <cinttypes>
#include <sstream>
#include <vector>



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
        // Unicode
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

    struct UnicodeName
    {
        explicit UnicodeName(const_bytes_view name) noexcept
        : name(name)
        {}

        const_bytes_view name;
    };

    struct AsciiName
    {
        explicit AsciiName(const_bytes_view name) noexcept
        : name(name)
        {}

        const_bytes_view name;
    };

    template<class ProcessFormat>
    bool format_list_extract_long_format(InStream& stream, ProcessFormat&& process_format)
    {
        if (stream.in_check_rem(min_long_format_name_data_length))
        {
            uint32_t format_id = stream.in_uint32_le();

            auto name = stream.remaining_bytes();
            auto end_name_pos = UTF16ByteLen(name);
            auto end_format_pos = end_name_pos + 2;

            if (!stream.in_check_rem(end_format_pos))
            {
                stream.rewind(stream.get_offset() - 4);
                return false;
            }

            stream.in_skip_bytes(end_format_pos);
            process_format(format_id, UnicodeName(name.first(end_name_pos)));

            return true;
        }

        return false;
    }

    template<class ProcessFormat>
    bool format_list_extract_ascii_format(InStream& stream, ProcessFormat&& process_format)
    {
        if (stream.in_check_rem(short_format_name_data_length))
        {
            uint32_t format_id = stream.in_uint32_le();

            auto name = stream.remaining_bytes();
            stream.in_skip_bytes(short_format_name_length);
            name = name.first(strnlen(name.as_charp(), short_format_name_length));
            process_format(format_id, AsciiName(name));

            return true;
        }

        return false;
    }

    template<class ProcessFormat>
    bool format_list_extract_unicode_format(InStream& stream, ProcessFormat&& process_format)
    {
        if (stream.in_check_rem(short_format_name_data_length))
        {
            uint32_t format_id = stream.in_uint32_le();

            auto name = stream.remaining_bytes();
            stream.in_skip_bytes(short_format_name_length);
            name = name.first(UTF16ByteLen(name.first(short_format_name_length)));
            process_format(format_id, UnicodeName(name));

            return true;
        }

        return false;
    }

    template<class ProcessFormat>
    void format_list_extract(
        InStream& stream, IsLongFormat is_long_format, IsAscii is_ascii,
        ProcessFormat&& process_format)
    {
        if (bool(is_long_format))
        {
            while (format_list_extract_long_format(stream, process_format))
                ;
        }
        else if (bool(is_ascii))
        {
            while (format_list_extract_ascii_format(stream, process_format))
                ;
        }
        else
        {
            while (format_list_extract_unicode_format(stream, process_format))
                ;
        }
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
                out_stream.out_clear_bytes(short_format_name_length - len);
                break;
            }
            case Charset::Utf16: {
                auto len = std::min(name.size(), short_format_name_length-1u);
                out_stream.out_copy_bytes(name);
                out_stream.out_clear_bytes(short_format_name_length - len);
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


namespace RDPECLIP {

// Predefined Clipboard Formats (WinUser.h)
enum CF
{
      CF_TEXT            = 1
    , CF_BITMAP          = 2
    , CF_METAFILEPICT    = 3
    , CF_SYLK            = 4
    , CF_DIF             = 5
    , CF_TIFF            = 6
    , CF_OEMTEXT         = 7
    , CF_DIB             = 8
    , CF_PALETTE         = 9
    , CF_PENDATA         = 10
    , CF_RIFF            = 11
    , CF_WAVE            = 12
    , CF_UNICODETEXT     = 13
    , CF_ENHMETAFILE     = 14
    , CF_HDROP           = 15
    , CF_LOCALE          = 16
    , CF_DIBV5           = 17
    , CF_OWNERDISPLAY    = 128
    , CF_DSPTEXT         = 129
    , CF_DSPBITMAP       = 130
    , CF_DSPMETAFILEPICT = 131
    , CF_DSPENHMETAFILE  = 142
    , CF_PRIVATEFIRST    = 512
    , CF_PRIVATELAST     = 767
    , CF_GDIOBJFIRST     = 768
    , CF_GDIOBJLAST      = 1023
};

inline array_view_const_char get_FormatId_name_av(uint32_t FormatId) noexcept
{
    switch (FormatId) {
        case CF_TEXT:            return "CF_TEXT"_av;
        case CF_BITMAP:          return "CF_BITMAP"_av;
        case CF_METAFILEPICT:    return "CF_METAFILEPICT"_av;
        case CF_SYLK:            return "CF_SYLK"_av;
        case CF_DIF:             return "CF_DIF"_av;
        case CF_TIFF:            return "CF_TIFF"_av;
        case CF_OEMTEXT:         return "CF_OEMTEXT"_av;
        case CF_DIB:             return "CF_DIB"_av;
        case CF_PALETTE:         return "CF_PALETTE"_av;
        case CF_PENDATA:         return "CF_PENDATA"_av;
        case CF_RIFF:            return "CF_RIFF"_av;
        case CF_WAVE:            return "CF_WAVE"_av;
        case CF_UNICODETEXT:     return "CF_UNICODETEXT"_av;
        case CF_ENHMETAFILE:     return "CF_ENHMETAFILE"_av;
        case CF_HDROP:           return "CF_HDROP"_av;
        case CF_LOCALE:          return "CF_LOCALE"_av;
        case CF_DIBV5:           return "CF_DIBV5"_av;
        case CF_OWNERDISPLAY:    return "CF_OWNERDISPLAY"_av;
        case CF_DSPTEXT:         return "CF_DSPTEXT"_av;
        case CF_DSPBITMAP:       return "CF_DSPBITMAP"_av;
        case CF_DSPMETAFILEPICT: return "CF_DSPMETAFILEPICT"_av;
        case CF_DSPENHMETAFILE:  return "CF_DSPENHMETAFILE"_av;
        case CF_PRIVATEFIRST:    return "CF_PRIVATEFIRST"_av;
        case CF_PRIVATELAST:     return "CF_PRIVATELAST"_av;
        case CF_GDIOBJFIRST:     return "CF_GDIOBJFIRST"_av;
        case CF_GDIOBJLAST:      return "CF_GDIOBJLAST"_av;
    }

    return "<unknown>"_av;
}

inline static const char * get_FormatId_name(uint32_t FormatId) noexcept
{
    return get_FormatId_name_av(FormatId).data();
}

// [MS-RDPECLIP] 2.2.1 Clipboard PDU Header (CLIPRDR_HEADER)
// =========================================================

// The CLIPRDR_HEADER structure is present in all clipboard PDUs. It is used
//  to identify the PDU type, specify the length of the PDU, and convey
//  message flags.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |            msgType            |            msgFlags           |
// +-------------------------------+-------------------------------+
// |                            dataLen                            |
// +---------------------------------------------------------------+

// msgType (2 bytes): An unsigned, 16-bit integer that specifies the type of
// the clipboard PDU that follows the dataLen field.

// +---------------------------------+-----------------------------------------+
// | Value                           | Meaning                                 |
// +---------------------------------+-----------------------------------------+
// | 0x0001 CB_MONITOR_READY         | Monitor Ready PDU                       |
// +---------------------------------+-----------------------------------------+
// | 0x0002 CB_FORMAT_LIST           | Format List PDU                         |
// +---------------------------------+-----------------------------------------+
// | 0x0003 CB_FORMAT_LIST_RESPONSE  | Format List Response PDU                |
// +---------------------------------+-----------------------------------------+
// | 0x0004 CB_FORMAT_DATA_REQUEST   | Format Data Request PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x0005 CB_FORMAT_DATA_RESPONSE  | Format Data Response PDU                |
// +---------------------------------+-----------------------------------------+
// | 0x0006 CB_TEMP_DIRECTORY        | Temporary Directory PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x0007 CB_CLIP_CAPS             | Clipboard Capabilities PDU              |
// +---------------------------------+-----------------------------------------+
// | 0x0008 CB_FILECONTENTS_REQUEST  | File Contents Request PDU               |
// +---------------------------------+-----------------------------------------+
// | 0x0009 CB_FILECONTENTS_RESPONSE | File Contents Response PDU              |
// +---------------------------------+-----------------------------------------+
// | 0x000A CB_LOCK_CLIPDATA         | Lock Clipboard Data PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x000B CB_UNLOCK_CLIPDATA       | Unlock Clipboard Data PDU               |
// +---------------------------------+-----------------------------------------+

enum {
      CB_MONITOR_READY         = 0x0001
    , CB_FORMAT_LIST           = 0x0002
    , CB_FORMAT_LIST_RESPONSE  = 0x0003
    , CB_FORMAT_DATA_REQUEST   = 0x0004
    , CB_FORMAT_DATA_RESPONSE  = 0x0005
    , CB_TEMP_DIRECTORY        = 0x0006
    , CB_CLIP_CAPS             = 0x0007
    , CB_FILECONTENTS_REQUEST  = 0x0008
    , CB_FILECONTENTS_RESPONSE = 0x0009
    , CB_LOCK_CLIPDATA         = 0x000A
    , CB_UNLOCK_CLIPDATA       = 0x000B
};

enum {
      CB_CHUNKED_FORMAT_DATA_RESPONSE = 0xFFFF
};

inline static const char * get_msgType_name(uint16_t msgType) {
    switch (msgType) {
        case CB_MONITOR_READY:         return "CB_MONITOR_READY";
        case CB_FORMAT_LIST:           return "CB_FORMAT_LIST";
        case CB_FORMAT_LIST_RESPONSE:  return "CB_FORMAT_LIST_RESPONSE";
        case CB_FORMAT_DATA_REQUEST:   return "CB_FORMAT_DATA_REQUEST";
        case CB_FORMAT_DATA_RESPONSE:  return "CB_FORMAT_DATA_RESPONSE";
        case CB_TEMP_DIRECTORY:        return "CB_TEMP_DIRECTORY";
        case CB_CLIP_CAPS:             return "CB_CLIP_CAPS";
        case CB_FILECONTENTS_REQUEST:  return "CB_FILECONTENTS_REQUEST";
        case CB_FILECONTENTS_RESPONSE: return "CB_FILECONTENTS_RESPONSE";
        case CB_LOCK_CLIPDATA:         return "CB_LOCK_CLIPDATA";
        case CB_UNLOCK_CLIPDATA:       return "CB_UNLOCK_CLIPDATA";
        case CB_CHUNKED_FORMAT_DATA_RESPONSE: return "CB_CHUNKED_FORMAT_DATA_RESPONSE";
    }

    return "<unknown>";
}

// msgFlags (2 bytes): An unsigned, 16-bit integer that indicates message
//  flags.

// +------------------+--------------------------------------------------------+
// | Value            | Meaning                                                |
// +------------------+--------------------------------------------------------+
// | CB_RESPONSE_OK   | Used by the Format List Response PDU, Format Data      |
// | 0x0001           | Response PDU, and File Contents Response PDU to        |
// |                  | indicate that the associated request Format List PDU,  |
// |                  | Format Data Request PDU, and File Contents Request PDU |
// |                  | were processed successfully.                           |
// +------------------+--------------------------------------------------------+
// | CB_RESPONSE_FAIL | Used by the Format List Response PDU, Format Data      |
// | 0x0002           | Response PDU, and File Contents Response PDU to        |
// |                  | indicate that the associated Format List PDU, Format   |
// |                  | Data Request PDU, and File Contents Request PDU were   |
// |                  | not processed successfully.                            |
// +------------------+--------------------------------------------------------+
// | CB_ASCII_NAMES   | Used by the Short Format Name variant of the Format    |
// | 0x0004           | List Response PDU to indicate the format names are in  |
// |                  | ASCII 8.                                               |
// +------------------+--------------------------------------------------------+

enum {
      CB_RESPONSE__NONE_ = 0x0000

    , CB_RESPONSE_OK     = 0x0001
    , CB_RESPONSE_FAIL   = 0x0002
    , CB_ASCII_NAMES     = 0x0004
};

inline static const char * get_msgFlag_name(uint16_t msgFlag) {
    switch (msgFlag) {
        case CB_RESPONSE__NONE_: return "CB_RESPONSE__NONE_";

        case CB_RESPONSE_OK:     return "CB_RESPONSE_OK";
        case CB_RESPONSE_FAIL:   return "CB_RESPONSE_FAIL";
        case CB_ASCII_NAMES:     return "CB_ASCII_NAMES";
    }

    return "<unknown>";
}

static inline const std::string msgFlags_to_string(uint16_t msgFlags) {
    std::ostringstream stream;

    if (!msgFlags) {
        stream << "0x" << std::hex << 0;
    }
    else {
        auto out_flag = [&stream, msgFlags](uint16_t msgFlag) {
                if (msgFlags & msgFlag) {
                    if (stream.tellp()) { stream << " | "; }
                    stream << get_msgFlag_name(msgFlag) << "(0x" <<
                        std::hex << msgFlag << ")";
                }
            };

        out_flag(CB_RESPONSE_OK);
        out_flag(CB_RESPONSE_FAIL);
        out_flag(CB_ASCII_NAMES);
    }

    return stream.str();
}

// Short Format Name
enum {
    SF_TEXT_HTML = 1
};

inline static const char * get_format_short_name(uint16_t formatID) {
    switch (formatID) {
        case SF_TEXT_HTML: return "text/html";
    }

    return "<unknown>";
}

// dataLen (4 bytes): An unsigned, 32-bit integer that specifies the size, in
//  bytes, of the data which follows the Clipboard PDU Header.

// struct RecvFactory {
//     uint16_t msgType;

//     explicit RecvFactory(InStream & stream) {
//         const unsigned expected = 2;    /* msgType(2) */
//         if (!stream.in_check_rem(expected)) {
//             LOG( LOG_INFO, "RDPECLIP::RecvFactory truncated msgType, need=%u remains=%zu"
//                , expected, stream.in_remain());
//             throw Error(ERR_RDP_DATA_TRUNCATED);
//         }

//         this->msgType = stream.in_uint16_le();
//     }   // RecvFactory(InStream & stream)
// };

class RecvPredictor {
    uint16_t msgType_;

public:
    explicit RecvPredictor(const InStream & stream) {
        InStream s = stream.clone();

        /* msgType(2) */
        ::check_throw(stream, 2, "RDPECLIP::RecvPredictor truncated msgType", ERR_RDP_DATA_TRUNCATED);

        this->msgType_ = s.in_uint16_le();
    }   // RecvFactory(InStream & stream)

    uint16_t msgType() const { return this->msgType_; }
};

class CliprdrHeader {
    uint16_t msgType_{0};
    uint16_t msgFlags_{0};
    uint32_t dataLen_{0};

public:
    uint16_t msgType()  const { return this->msgType_; }

    uint16_t msgFlags() const { return this->msgFlags_; }

    uint32_t dataLen()  const { return this->dataLen_; }

    void dataLen(uint32_t dataLen) {
        this->dataLen_ = dataLen;
    }

    CliprdrHeader() = default; // CliprdrHeader()

    CliprdrHeader(uint16_t msgType, uint16_t msgFlags, uint32_t dataLen)
        : msgType_(msgType)
        , msgFlags_(msgFlags)
        , dataLen_(dataLen) {}

    CliprdrHeader(CliprdrHeader const &) = delete;
    CliprdrHeader& operator=(CliprdrHeader const &) = delete;

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->msgType_);
        stream.out_uint16_le(this->msgFlags_);
        stream.out_uint32_le(this->dataLen_);
    }   // void emit(OutStream & stream)

    void recv(InStream & stream) {

        /* msgType(2) + msgFlags(2) + dataLen(4) */
        ::check_throw(stream, 8, "RDPECLIP::CliprdrHeader::recv truncated data", ERR_RDP_DATA_TRUNCATED);

        this->msgType_  = stream.in_uint16_le();
        this->msgFlags_ = stream.in_uint16_le();
        this->dataLen_  = stream.in_uint32_le();
    }

    static size_t size() {
        return 8;       // 2(msgType) + 2(msgFlags) + 4(dataLen)
    }

    void log(int level) const {
        LOG(level, "CliprdrHeader: msgType=%s(0x%X) msgFlags=<%s> dataLen=%u",
            get_msgType_name(this->msgType_), this->msgType_,
            msgFlags_to_string(this->msgFlags_), this->dataLen_);
    }

    void log() const {
        LOG(LOG_INFO, "     CliprdrHeader: MsgType=0x%x(2 bytes):%s MsgFlags=0x%x(2 bytes):%s DataLen=%uByte(s)(4 bytes)", this->msgType_, get_msgType_name(this->msgType_), this->msgFlags_, get_msgFlag_name(this->msgFlags_), this->dataLen_);
    }
};  // struct CliprdrHeader

// [MS-RDPECLIP] - 2.2.2.1 Clipboard Capabilities PDU (CLIPRDR_CAPS)
// =================================================================

// The Clipboard Capabilities PDU is an optional PDU used to exchange
//  capability information. It is first sent from the server to the client.
//  Upon receipt of the Monitor Ready PDU, the client sends the Clipboard
//  Capabilities PDU to the server.

// If this PDU is not sent by a Remote Desktop Protocol: Clipboard Virtual
//  Channel Extension endpoint, it is assumed that the endpoint is using the
//  default values for each capability field.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |       cCapabilitiesSets       |              pad1             |
// +-------------------------------+-------------------------------+
// |                   capabilitySets (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_CLIP_CAPS (0x0007), while the
//  msgFlags field MUST be set to 0x0000.

// cCapabilitiesSets (2 bytes): An unsigned, 16-bit integer that specifies
//  the number of CLIPRDR_CAPS_SETs, present in the capabilitySets field.

// pad1 (2 bytes): An unsigned, 16-bit integer used for padding. Values in
//  this field are ignored.

// capabilitySets (variable): A variable-sized array of capability sets, each
//  conforming in structure to the CLIPRDR_CAPS_SET.

class ClipboardCapabilitiesPDU
{
    uint16_t cCapabilitiesSets_ = 0;

public:
    ClipboardCapabilitiesPDU() = default;

    ClipboardCapabilitiesPDU(uint16_t cCapabilitiesSets)
        : cCapabilitiesSets_(cCapabilitiesSets)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->cCapabilitiesSets_);
        stream.out_clear_bytes(2);  // pad1(2)
    }   // void emit(OutStream & stream)

    void recv(InStream & stream) {

        // cCapabilitiesSets(2) + pad1(2)
        ::check_throw(stream, 4, "RDPECLIP::ClipboardCapabilitiesPDU:recv(): recv truncated data", ERR_RDP_DATA_TRUNCATED);

        this->cCapabilitiesSets_ = stream.in_uint16_le();

        stream.in_skip_bytes(2);    // pad1(2)
    }

    uint16_t cCapabilitiesSets() const { return this->cCapabilitiesSets_; }

    static constexpr size_t size() {
        return 4;   // cCapabilitiesSets(2) + pad1(2)
    }

    void log(int level) const {
        LOG(level, "ClipboardCapabilitiesPDU: cCapabilitiesSets=%u", this->cCapabilitiesSets_);
    }

    void log() const {
        LOG(LOG_INFO, "     Clipboard Capabilities PDU: cCapabilitiesSets=%d(2 bytes) Padding-(2 byte):NOT USED", this->cCapabilitiesSets_);
    }
};  // struct ClipboardCapabilitiesPDU

// [MS-RDPECLIP] - 2.2.2.1.1 Capability Set (CLIPRDR_CAPS_SET)
// ===========================================================

// The CLIPRDR_CAPS_SET structure is used to wrap capability set data and to
//  specify the type and size of this data exchanged between the client and
//  the server. All capability sets conform to this basic structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       capabilitySetType       |        lengthCapability       |
// +-------------------------------+-------------------------------+
// |                   capabilityData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// capabilitySetType (2 bytes): An unsigned, 16-bit integer used as a type
//  identifier of the capability set.

//  +---------------------+------------------------+
//  | Value               | Meaning                |
//  +---------------------+------------------------+
//  | CB_CAPSTYPE_GENERAL | General Capability Set |
//  | 0x0001              |                        |
//  +---------------------+------------------------+

enum {
    CB_CAPSTYPE_GENERAL = 0x00000001
};

// lengthCapability (2 bytes): An unsigned, 16-bit integer that specifies the
//  combined length, in bytes, of the capabilitySetType, capabilityData and
//  lengthCapability fields.

// capabilityData (variable): Capability set data specified by the type given
//  in the capabilitySetType field. This field is a variable number of bytes.

class CapabilitySetRecvFactory {
    uint16_t capabilitySetType_;

public:
    explicit CapabilitySetRecvFactory(InStream & stream) {

        /* capabilitySetType(2) */
        ::check_throw(stream, 2, "RDPECLIP::CapabilitySetRecvFactory truncated capabilitySetType", ERR_RDP_DATA_TRUNCATED);

        this->capabilitySetType_ = stream.in_uint16_le();
    }   // CapabilitySetRecvFactory(InStream & stream)

    uint16_t capabilitySetType() const { return this->capabilitySetType_; }

    static const char * get_capabilitySetType_name(uint16_t capabilitySetType) {
        switch (capabilitySetType) {
            case CB_CAPSTYPE_GENERAL:  return "CB_CAPSTYPE_GENERAL";
        }

        return "<unknown>";
    }
};

// [MS-RDPECLIP] - 2.2.2.1.1.1 General Capability Set
//  (CLIPRDR_GENERAL_CAPABILITY)
// ==================================================

// The CLIPRDR_GENERAL_CAPABILITY structure is used to advertise general
//  clipboard settings.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       capabilitySetType       |        lengthCapability       |
// +-------------------------------+-------------------------------+
// |                            version                            |
// +---------------------------------------------------------------+
// |                          generalFlags                         |
// +---------------------------------------------------------------+

// capabilitySetType (2 bytes): An unsigned 16-bit integer that specifies the
//  type of the capability set. This field MUST be set to CB_CAPSTYPE_GENERAL
//  (0x0001).

// lengthCapability (2 bytes): An unsigned, 16-bit integer that specifies the
//  length, in bytes, of the capabilitySetType, capability data and
//  lengthCapability fields.

// version (4 bytes): An unsigned, 32-bit integer that specifies the Remote
//  Desktop Protocol: Clipboard Virtual Channel Extension version number.
//  This field is for informational purposes and MUST NOT be used to make
//  protocol capability decisions. The actual features supported are
//  specified in the generalFlags field.

//  +-------------------+-----------+
//  | Value             | Meaning   |
//  +-------------------+-----------+
//  | CB_CAPS_VERSION_1 | Version 1 |
//  | 0x00000001        |           |
//  +-------------------+-----------+
//  | CB_CAPS_VERSION_2 | Version 2 |
//  | 0x00000002        |           |
//  +-------------------+-----------+

enum {
    CB_CAPS_VERSION_1 = 0x00000001,
    CB_CAPS_VERSION_2 = 0x00000002
};

// generalFlags (4 bytes): An unsigned, 32-bit integer that specifies the
//  general capability flags.

//  +------------------------------+------------------------------------------+
//  | Value                        | Meaning                                  |
//  +------------------------------+------------------------------------------+
//  | CB_USE_LONG_FORMAT_NAMES     | The Long Format Name variant of the      |
//  | 0x00000002                   | Format List PDU is supported for         |
//  |                              | exchanging updated format names. If this |
//  |                              | flag is not set, the Short Format Name   |
//  |                              | variant MUST be used. If this flag is    |
//  |                              | set by both protocol endpoints, then the |
//  |                              | Long Format Name variant MUST be used.   |
//  +------------------------------+------------------------------------------+
//  | CB_STREAM_FILECLIP_ENABLED   | File copy and paste using stream-based   |
//  | 0x00000004                   | operations are supported using the File  |
//  |                              | Contents Request PDU and File Contents   |
//  |                              | Response PDU.                            |
//  +------------------------------+------------------------------------------+
//  | CB_FILECLIP_NO_FILE_PATHS    | Indicates that any description of files  |
//  | 0x00000008                   | to copy and paste MUST NOT include the   |
//  |                              | source path of the files.                |
//  +------------------------------+------------------------------------------+
//  | CB_CAN_LOCK_CLIPDATA         | Locking and unlocking of File Stream     |
//  | 0x00000010                   | data on the clipboard is supported using |
//  |                              | the Lock Clipboard Data PDU and Unlock   |
//  |                              | Clipboard Data PDU.                      |
//  +------------------------------+------------------------------------------+
//  | CB_HUGE_FILE_SUPPORT_ENABLED | Indicates support for transferring files |
//  | 0x00000020                   | that are larger than 4,294,967,296 bytes |
//  |                              | in size. If this flag is not set, then   |
//  |                              | only files of size less than or equal to |
//  |                              | 4,294,967,296 bytes can be exchanged     |
//  |                              | using the File Contents Request PDU and  |
//  |                              | File Contents Response PDU.              |
//  +------------------------------+------------------------------------------+

enum {
    CB_USE_LONG_FORMAT_NAMES     = 0x00000002,
    CB_STREAM_FILECLIP_ENABLED   = 0x00000004,
    CB_FILECLIP_NO_FILE_PATHS    = 0x00000008,
    CB_CAN_LOCK_CLIPDATA         = 0x00000010,
    CB_HUGE_FILE_SUPPORT_ENABLED = 0x00000020
};

enum {
    CB__MINIMUM_WINDOWS_CLIENT_GENERAL_CAPABILITY_FLAGS_ =
        (CB_USE_LONG_FORMAT_NAMES | CB_STREAM_FILECLIP_ENABLED |
         CB_FILECLIP_NO_FILE_PATHS | CB_CAN_LOCK_CLIPDATA)
};

inline static const char * get_generalFlag_name(uint16_t msgFlag) {
    switch (msgFlag) {
        case CB_USE_LONG_FORMAT_NAMES:   return "CB_USE_LONG_FORMAT_NAMES";
        case CB_STREAM_FILECLIP_ENABLED: return "CB_STREAM_FILECLIP_ENABLED";
        case CB_FILECLIP_NO_FILE_PATHS:  return "CB_FILECLIP_NO_FILE_PATHS";
        case CB_CAN_LOCK_CLIPDATA:       return "CB_CAN_LOCK_CLIPDATA";
    }

    return "<unknown>";
}

static inline const std::string generalFlags_to_string(uint32_t generalFlags) {
    std::ostringstream stream;

    if (!generalFlags) {
        stream << "0x" << std::hex << 0;
    }
    else {
        auto out_flag = [&stream, generalFlags](uint16_t generalFlag) {
                if (generalFlags & generalFlag) {
                    if (stream.tellp()) { stream << " | "; }
                    stream << get_generalFlag_name(generalFlag) << "(0x" <<
                        std::hex << generalFlag << ")";
                }
            };

        out_flag(CB_USE_LONG_FORMAT_NAMES);
        out_flag(CB_STREAM_FILECLIP_ENABLED);
        out_flag(CB_FILECLIP_NO_FILE_PATHS);
        out_flag(CB_CAN_LOCK_CLIPDATA);
        out_flag(CB_HUGE_FILE_SUPPORT_ENABLED);
    }

    return stream.str();
}

// If the General Capability Set is not present in the Clipboard Capabilities
//  PDU, then the default set of general capabilities MUST be assumed. By
//  definition the default set does not specify any flags in the generalFlags
//  field, that is the generalFlags field is set to 0x00000000.

class GeneralCapabilitySet {
    uint16_t capabilitySetType_ = CB_CAPSTYPE_GENERAL;
    uint16_t lengthCapability  = GeneralCapabilitySet::size();
    uint32_t version_          = CB_CAPS_VERSION_1;
    uint32_t generalFlags_     = 0;

public:
    GeneralCapabilitySet() = default;

    GeneralCapabilitySet(uint32_t version, uint32_t generalFlags) {
        this->version_      = version;
        this->generalFlags_ = generalFlags;
    }

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->capabilitySetType_);
        stream.out_uint16_le(this->lengthCapability);
        stream.out_uint32_le(this->version_);
        stream.out_uint32_le(this->generalFlags_);
    }

    void recv(InStream & stream, const CapabilitySetRecvFactory & recv_factory) {
        this->capabilitySetType_ = recv_factory.capabilitySetType();
        assert(this->capabilitySetType_ == CB_CAPSTYPE_GENERAL);

        ::check_throw(stream, this->size() - 2, "RDPECLIP::GeneralCapabilitySet::recv (1) truncated data", ERR_RDP_DATA_TRUNCATED);
        // capabilitySetType(2)

        this->lengthCapability  = stream.in_uint16_le();
        assert(this->size() == this->lengthCapability);

        this->version_      = stream.in_uint32_le();
        this->generalFlags_ = stream.in_uint32_le();
    }

    void recv(InStream & stream) {

        // capabilitySetType(2) + lengthCapability(2) + version(4) + generalFlags(4)
        ::check_throw(stream, this->size(), "RDPECLIP::GeneralCapabilitySet::recv (2) truncated data", ERR_RDP_DATA_TRUNCATED);

        this->capabilitySetType_ = stream.in_uint16_le();
//         assert(this->capabilitySetType == CB_CAPSTYPE_GENERAL);

        this->lengthCapability  = stream.in_uint16_le();
//         assert(this->size() == this->lengthCapability);

        this->version_      = stream.in_uint32_le();
        this->generalFlags_ = stream.in_uint32_le();
    }

    uint32_t version() const { return this->version_; }

    uint32_t generalFlags() const { return this->generalFlags_; }

    uint32_t capabilitySetType() const { return this->capabilitySetType_;}

    static constexpr size_t size() {
        return 12;  // capabilitySetType(2) + lengthCapability(2) + version(4) +
                    //     generalFlags(4)
    }

private:
    static const char * get_version_name(uint32_t version) {
        switch (version) {
            case CB_CAPS_VERSION_1: return "CB_CAPS_VERSION_1";
            case CB_CAPS_VERSION_2: return "CB_CAPS_VERSION_2";
        }

        return "<unknown>";
    }

    size_t str(char * buffer, size_t size) const {
        std::string const generalFlags_string = generalFlags_to_string(this->generalFlags_);
        size_t length = ::snprintf(buffer, size,
            "RDPECLIP::GeneralCapabilitySet: "
                "capabilitySetType=%s(%d) lengthCapability=%u version=%s(0x%08X) generalFlags=<%s>",
            CapabilitySetRecvFactory::get_capabilitySetType_name(this->capabilitySetType_),
            this->capabilitySetType_,
            unsigned(this->lengthCapability),
            this->get_version_name(this->version_),
            this->version_,
            generalFlags_string.c_str()
            );
        return ((length < size) ? length : size - 1);
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }

    void log() const {
        LOG(LOG_INFO, "GeneralCapabilitySet: capabilitySetType=0x%04x(2 bytes):CB_CAPSTYPE_GENERAL lengthCapability=0x%04x(2 bytes) version=0x%08x(4 bytes) generalFlags=0x%08x(4 bytes):%s", this->capabilitySetType_, this->lengthCapability, this->version_, this->generalFlags_, generalFlags_to_string(this->generalFlags_));
    }
};  // GeneralCapabilitySet

// [MS-RDPECLIP] 2.2.2.2 Server Monitor Ready PDU (CLIPRDR_MONITOR_READY)
// ======================================================================

// The Monitor Ready PDU is sent from the server to the client to indicate
//  that the server is initialized and ready. This PDU is transmitted by the
//  server after it has sent the Clipboard Capabilities PDU to the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_MONITOR_READY (0x0001), while the
//  msgFlags field MUST be set to 0x0000.

class ServerMonitorReadyPDU
{
public:
    void emit(OutStream &/* stream*/) const {}

    void recv(InStream &/* stream*/) {}

    static constexpr size_t size() {
        return 0;
    }

    void log(int level = LOG_INFO) const {
        LOG(level, "ServerMonitorReadyPDU");
    }
};  // struct ServerMonitorReadyPDU

// [MS-RDPECLIP] 2.2.2.3 Client Temporary Directory PDU (CLIPRDR_TEMP_DIRECTORY)
// =============================================================================

// The Temporary Directory PDU is an optional PDU sent from the client to the server. This PDU informs
//  the server of a location on the client file system that MUST be used to deposit files being copied to the
//  client. The location MUST be accessible by the server to be useful. Section 3.1.1.3 specifies how direct
//  file access impacts file copy and paste. This PDU is sent by the client after receiving the Monitor Ready
//  PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                    wszTempDir (520 bytes)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header
//  MUST be set to CB_TEMP_DIRECTORY (0x0006), while the msgFlags field MUST be set to
//  0x0000.

// wszTempDir (520 bytes): A 520-byte block that contains a null-terminated string that represents
//  the directory on the client that MUST be used to store temporary clipboard related information.
//  The supplied path MUST be absolute and relative to the local client system, for example,
//  "c:\temp\clipdata". Any space not used in this field SHOULD be filled with null characters.

class ClientTemporaryDirectoryPDU
{
    std::string temp_dir;

public:
    explicit ClientTemporaryDirectoryPDU() = default;

    explicit ClientTemporaryDirectoryPDU(const char* temp_dir)
    : temp_dir(temp_dir) {}

    void emit(OutStream & stream) const
    {
        uint8_t tempDir_unicode_data[520]; // wszTempDir(520)

        size_t size_of_tempDir_unicode_data = ::UTF8toUTF16(
            this->temp_dir,
            tempDir_unicode_data, sizeof(tempDir_unicode_data));

        stream.out_copy_bytes(tempDir_unicode_data,
            size_of_tempDir_unicode_data);

        stream.out_clear_bytes(520 /* wszTempDir(520) */ -
            size_of_tempDir_unicode_data);
    }

    void recv(InStream & stream)
    {
        // wszTempDir(520)
        ::check_throw(stream, this->size(), "RDPECLIP::ClientTemporaryDirectoryPDU::recv truncated data", ERR_RDP_DATA_TRUNCATED);

        uint8_t const * const tempDir_unicode_data = stream.get_current();
        uint8_t tempDir_utf8_string[520 /* wszTempDir(520) */ / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes];
        ::UTF16toUTF8(tempDir_unicode_data, 520 /* wszTempDir(520) */ / sizeof(uint16_t),
            tempDir_utf8_string, sizeof(tempDir_utf8_string));
        // The null-terminator is included.
        this->temp_dir = ::char_ptr_cast(tempDir_utf8_string);

        stream.in_skip_bytes(520);  // wszTempDir(520)
    }

    const char * wszTempDir() const { return this->temp_dir.c_str(); }

    static constexpr size_t size() {
        return 520;  // wszTempDir(520)
    }

    void log(int level = LOG_INFO) const {
        LOG(level, "ClientTemporaryDirectoryPDU: wszTempDir=\"%s\"", temp_dir);
    }
};  // struct ClientTemporaryDirectoryPDU

// [MS-RDPECLIP] 2.2.3.1 Format List PDU (CLIPRDR_FORMAT_LIST)
// ===========================================================

// The Format List PDU is sent by either the client or the server when its
//  local system clipboard is updated with new clipboard data. This PDU
//  contains the Clipboard Format ID and name pairs of the new Clipboard
//  Formats on the clipboard.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   formatListData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_LIST (0x0002), while the
//  msgFlags field MUST be set to 0x0000 or CB_ASCII_NAMES (0x0004) depending
//  on the type of data present in the formatListData field.

// formatListData (variable): An array consisting solely of either Short
//  Format Names or Long Format Names. The type of structure used in the
//  array is determined by the presence of the CB_USE_LONG_FORMAT_NAMES
//  (0x00000002) flag in the generalFlags field of the General Capability Set
//  (section 2.2.2.1.1.1). Each array holds a list of the Clipboard Format ID
//  and name pairs available on the local system clipboard of the sender. If
//  Short Format Names are being used, and the embedded Clipboard Format
//  names are in ASCII 8 format, then the msgFlags field of the clipHeader
//  must contain the CB_ASCII_NAMES (0x0004) flag.

class FormatName {
    uint32_t    formatId_ = 0;
    std::string format_name_;

public:
    explicit FormatName(uint32_t formatId, std::string format_name) noexcept
    : formatId_(formatId)
    , format_name_(std::move(format_name))
    {}

    uint32_t formatId() const { return this->formatId_; }

    void formatId(uint32_t formatId) { this->formatId_ = formatId; }

    std::string const& format_name() const { return this->format_name_; }

    void format_name(const char * format_name) { this->format_name_ = format_name; }

    size_t format_name_length() const { return this->format_name_.length(); }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "{formatId=%s(%u) formatName=\"%s\"}",
            get_FormatId_name(this->formatId_), this->formatId_, this->format_name_.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class FormatName

class FormatListPDUEx {
    std::vector<FormatName> format_names;

public:
    auto begin() const { return this->format_names.begin(); }
    auto end() const { return this->format_names.end(); }

    void emit(OutStream & stream, bool use_long_format_names) const
    {
        if (use_long_format_names) {
            for (auto & format_name : this->format_names) {
                stream.out_uint32_le(format_name.formatId());

                auto data = stream.get_tailroom_bytes();
                stream.out_skip_bytes(UTF8toUTF16(
                    format_name.format_name(), data.first(data.size() - 2u)));
                stream.out_uint16_le(0);
            }
        }
        else if (!this->will_be_sent_in_ASCII_8(use_long_format_names)) {
            // Unicode
            for (auto & format_name : this->format_names) {
                stream.out_uint32_le(format_name.formatId());

                auto data = stream.get_tailroom_bytes();
                data = data.first(std::min(Cliprdr::short_format_name_length-2u, data.size()));

                data = stream.out_skip_bytes(UTF8toUTF16(format_name.format_name(), data));
                stream.out_clear_bytes(Cliprdr::short_format_name_length - data.size());
            }
        }
        else {
            // ASCII
            for (auto & format_name : this->format_names) {
                stream.out_uint32_le(format_name.formatId());

                auto const& name = format_name.format_name();
                auto len = std::min(name.size(), Cliprdr::short_format_name_length - 1u);

                stream.out_copy_bytes({name.data(), len});
                stream.out_clear_bytes(Cliprdr::short_format_name_length - len);
            }
        }
    }

    void recv(InStream & in_stream, bool use_long_format_names, bool in_ASCII_8)
    {
        this->format_names.clear();

        Cliprdr::format_list_extract(
            in_stream, Cliprdr::IsLongFormat(use_long_format_names),
            Cliprdr::IsAscii(in_ASCII_8), overload{
            [this](uint32_t format_id, Cliprdr::UnicodeName unicode_name) {
                char buf[1024];
                auto name = UTF16toUTF8_buf(unicode_name.name, make_array_view(buf)).as_chars();
                this->format_names.emplace_back(format_id, std::string(name.data(), name.size()));
            },
            [this](uint32_t format_id, Cliprdr::AsciiName ascii_name) {
                auto name = ascii_name.name.as_chars();
                this->format_names.emplace_back(format_id, std::string(name.data(), name.size()));
            }
        });
    }

    void add_format_name(uint32_t formatId, const char * format_name = "") {
        this->format_names.emplace_back(formatId, format_name);
    }

    size_t num_format_names() const { return this->format_names.size(); }

    FormatName const & format_name(size_t idx_format_name) const { return this->format_names[idx_format_name]; }

    size_t size(bool use_long_format_names) const {
        size_t sz = 0;

        if (use_long_format_names) {
            for (auto & format_name : this->format_names) {
                sz += 4;    // formatId(4)
                auto utf8_len = UTF8Len(format_name.format_name().data());
                sz += utf8_len * 2 + 2; // wszFormatName (variable)
            }
        }
        else {
            sz = this->format_names.size() * (
                      4     // formatId(4)
                    + 32    // formatName(32)
                );
        }

        return sz;
    }

    bool will_be_sent_in_ASCII_8(bool use_long_format_names) const
    {
        if (!use_long_format_names) {
            bool   all_format_names_are_ASCII_strings = true;
            size_t max_format_name_len_in_char        = 0;
            for (auto & format_name : this->format_names) {
                if (!::is_ASCII_string(byte_ptr_cast(format_name.format_name().c_str()))) {
                    all_format_names_are_ASCII_strings = false;
                }

                const size_t format_name_len_in_char = ::UTF8StrLenInChar(byte_ptr_cast(format_name.format_name().c_str()));
                if (max_format_name_len_in_char < format_name_len_in_char) {
                    max_format_name_len_in_char = format_name_len_in_char;
                }
            }

            if (all_format_names_are_ASCII_strings &&
                (max_format_name_len_in_char >= (32 /* formatName(32) */ / sizeof(uint16_t)))) {
                return true;
            }
        }

        return false;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "FormatListPDU:");
        length += ((result < size - length) ? result : (size - length - 1));

        for (auto & format_name : this->format_names) {
            size_t result = ::snprintf(buffer + length, size - length, " ");
            length += ((result < size - length) ? result : (size - length - 1));

            result = format_name.str(buffer + length, size - length);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // FormatListPDUEx

inline static bool FormatListPDUEx_contains_data_in_format(const FormatListPDUEx & format_list_pdu, uint32_t formatId) {
    for (FormatName const& format_name : format_list_pdu) {
        if (format_name.formatId() == formatId) {
            return true;
        }
    }

    return false;
}


// TODO zstr_XXXXX
constexpr auto FILEGROUPDESCRIPTORW = cstr_array_view("FileGroupDescriptorW\0");
constexpr auto FILECONTENTS         = cstr_array_view("FileContents\0");
constexpr auto PREFERRED_DROPEFFECT = cstr_array_view("Preferred DropEffect\0");


// [MS-RDPECLIP] 2.2.3.2 Format List Response PDU (FORMAT_LIST_RESPONSE)
// =====================================================================
// The Format List Response PDU is sent as a reply to the Format List PDU. It
//  is used to indicate whether processing of the Format List PDU was
//  successful.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_LIST_RESPONSE (0x0003). The
//  CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set in
//  the msgFlags field of the Clipboard PDU Header.

struct FormatListResponsePDU
{
    void emit(OutStream & /*out_stream*/) const {}

    void recv(InStream & /*in_stream*/) {}

    static constexpr size_t size() {
        return 0;
    }

    void log(int level = LOG_INFO) const {
        LOG(level, "FormatListResponsePDU");
    }

};  // struct FormatListResponsePDU

// [MS-RDPECLIP] 2.2.5.1 Format Data Request PDU (CLIPRDR_FORMAT_DATA_REQUEST)
// ===========================================================================

// The Format Data Request PDU is sent by the recipient of the Format List
//  PDU. It is used to request the data for one of the formats that was listed
//  in the Format List PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       requestedFormatId                       |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_DATA_REQUEST (0x0004), while
//  the msgFlags field MUST be set to 0x0000.

// requestedFormatId (4 bytes): An unsigned, 32-bit integer that specifies the
//  Clipboard Format ID of the clipboard data. The Clipboard Format ID MUST be
//  one listed previously in the Format List PDU.

struct FormatDataRequestPDU
{
    uint32_t requestedFormatId{0};

    FormatDataRequestPDU() = default;

    explicit FormatDataRequestPDU(uint32_t requestedFormatId)
    : requestedFormatId(requestedFormatId)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->requestedFormatId);
    }

    void recv(InStream & stream)
    {
        ::check_throw(stream, 4, "FormatDataRequestPDU::recv", ERR_RDP_DATA_TRUNCATED);

        this->requestedFormatId = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "FormatDataRequestPDU: requestedFormatId=0x%08x(4 bytes):%s", this->requestedFormatId, get_FormatId_name(this->requestedFormatId));
    }

    constexpr static size_t size() {
       return 4;                                            // requestedFormatId(4)
    }
};  // struct FormatDataRequestPDU






//2.2.5.3 File Contents Request PDU (CLIPRDR_FILECONTENTS_REQUEST)

//The File Contents Request PDU is sent by the recipient of the Format List PDU. It is used to request either the size of a remote file copied to the clipboard or a portion of the data in the file.


// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            streamId                           |
// +---------------------------------------------------------------+
// |                             lindex                            |
// +---------------------------------------------------------------+
// |                             dwFlags                           |
// +---------------------------------------------------------------+
// |                          nPositionLow                         |
// +---------------------------------------------------------------+
// |                          nPositionHigh                        |
// +---------------------------------------------------------------+
// |                           cbRequested                         |
// +---------------------------------------------------------------+
// |                       clipDataId (optional)                   |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_FILECONTENTS_REQUEST (0x0008), while the msgFlags field MUST be set to 0x0000.

// streamId (4 bytes): An unsigned, 32-bit format ID used to associate the File Contents Request PDU with the corresponding File Contents Response PDU. The File Contents Response PDU is sent as a reply and contains an identical value in the streamId field.

// lindex (4 bytes): A signed, 32-bit integer that specifies the numeric ID of the remote file that is the target of the File Contents Request PDU. This field is used as an index that identifies a particular file in a File List. This File List SHOULD have been obtained as clipboard data in a prior Format Data Request PDU and Format Data Response PDU exchange.

// dwFlags (4 bytes): An unsigned, 32-bit integer that specifies the type of operation to be performed by the recipient.

//  +----------------------------+--------------------------------------------+
//  | Value                      | Meaning                                    |
//  +----------------------------+--------------------------------------------+
//  | FILECONTENTS_SIZE          | A request for the size of the file         |
//  | 0x00000001                 | identified by the lindex field. The size   |
//  |                            | MUST be returned as a 64-bit, unsigned     |
//  |                            | integer. The cbRequested field MUST be set |
//  |                            | to 0x00000008 and both the nPositionLow and|
//  |                            | nPositionHigh fields MUST be set to        |
//  |                            | 0x00000000.                                |
//  +----------------------------+--------------------------------------------+
//  | FILECONTENTS_RANGE         | A request for the data present in the file |
//  | 0x00000002                 | identified by the lindex field. The data to|
//  |                            | be retrieved is extracted starting from the|
//  |                            | offset given by the nPositionLow and       |
//  |                            | nPositionHigh fields. The maximum number   |
//  |                            | of bytes to extract is specified by the    |
//  |                            | cbRequested field.                         |
//  +----------------------------+--------------------------------------------+

//    The FILECONTENTS_SIZE and FILECONTENTS_RANGE flags MUST NOT be set at the same time.

// nPositionLow (4 bytes): An unsigned, 32-bit integer that specifies the low bytes of the offset into the remote file, identified by the lindex field, from where the data needs to be extracted to satisfy a FILECONTENTS_RANGE operation.

// nPositionHigh (4 bytes): An unsigned, 32-bit integer that specifies the high bytes of the offset into the remote file, identified by the lindex field, from where the data needs to be extracted to satisfy a FILECONTENTS_RANGE operation. This field is currently not used because offsets greater than 4,294,967,295 bytes are not supported, and MUST be set to zero.

// cbRequested (4 bytes): An unsigned, 32-bit integer that specifies the size, in bytes, of the data to retrieve. For a FILECONTENTS_SIZE operation, this field MUST be set to 0x00000008. In the case of a FILECONTENTS_RANGE operation, this field contains the maximum number of bytes to read from the remote file.

// clipDataId (4 bytes): An optional unsigned, 32-bit integer that identifies File Stream data which was tagged in a prior Lock Clipboard Data PDU (section 2.2.4.1).

enum : uint32_t {
    FILECONTENTS_SIZE              = 0x00000001
  , FILECONTENTS_RANGE             = 0x00000002
  , FILECONTENTS_SIZE_CB_REQUESTED = 0x00000008
};


class FileContentsRequestPDU {
    uint32_t streamId_      = 0;
    uint32_t lindex_        = 0;
    uint32_t dwFlags_       = 0;
    uint32_t nPositionLow_  = 0;
    uint32_t nPositionHigh_ = 0;
    uint32_t cbRequested_   = 0;
    uint32_t clipDataId_    = 0;

    bool has_optional_clipDataId_ = false;

public:
    explicit FileContentsRequestPDU() = default;

    FileContentsRequestPDU(uint32_t streamId,
                           uint32_t lindex,
                           uint32_t dwFlags,
                           uint32_t nPositionLow,
                           uint32_t nPositionHigh,
                           uint32_t cbRequested,
                           uint32_t clipDataId,
                           bool has_optional_clipDataId)
        : streamId_(streamId)
        , lindex_(lindex)
        , dwFlags_(dwFlags)
        , nPositionLow_(nPositionLow)
        , nPositionHigh_(nPositionHigh)
        , cbRequested_(cbRequested)
        , clipDataId_(clipDataId)
        , has_optional_clipDataId_(has_optional_clipDataId)
    {}

    FileContentsRequestPDU(uint32_t streamId, uint32_t lindex,
        uint32_t dwFlags, uint32_t nPositionLow, uint32_t nPositionHigh,
        uint32_t cbRequested) :
            FileContentsRequestPDU(streamId, lindex, dwFlags, nPositionLow,
                nPositionHigh, cbRequested, 0, false) {}

    FileContentsRequestPDU(uint32_t streamId, uint32_t lindex,
        uint32_t dwFlags, uint32_t nPositionLow, uint32_t nPositionHigh,
        uint32_t cbRequested, uint32_t clipDataId) :
            FileContentsRequestPDU(streamId, lindex, dwFlags, nPositionLow,
                nPositionHigh, cbRequested, clipDataId, true) {}

    void emit(OutStream& stream) const {
        stream.out_uint32_le(this->streamId_);
        stream.out_uint32_le(this->lindex_);
        stream.out_uint32_le(this->dwFlags_);
        stream.out_uint32_le(this->nPositionLow_);
        stream.out_uint32_le(this->nPositionHigh_);
        stream.out_uint32_le(this->cbRequested_);
        if (this->has_optional_clipDataId_) {
            stream.out_uint32_le(this->clipDataId_);
        }
    }

    void receive(InStream& stream) {
        ::check_throw(stream, this->minimum_size(), "FileContentsRequest", ERR_RDP_DATA_TRUNCATED);

        this->streamId_      = stream.in_uint32_le();
        this->lindex_        = stream.in_uint32_le();
        this->dwFlags_       = stream.in_uint32_le();
        this->nPositionLow_  = stream.in_uint32_le();
        this->nPositionHigh_ = stream.in_uint32_le();
        this->cbRequested_   = stream.in_uint32_le();

        if (stream.in_remain() >= 4 /* clipDataId(4) */) {
            this->clipDataId_ = stream.in_uint32_le();
            this->has_optional_clipDataId_ = true;
        }
        else {
            this->has_optional_clipDataId_ = false;
        }
    }

    uint32_t streamId() const { return this->streamId_; }
    uint32_t lindex() const { return this->lindex_; }
    uint32_t dwFlags() const { return this->dwFlags_; }

    uint64_t position() const {
            return (this->nPositionLow_ | (static_cast<uint64_t>(this->nPositionHigh_) << 32));
        }

    uint32_t cbRequested() const { return this->cbRequested_; }
    uint32_t clipDataId() const { return this->clipDataId_; }

    bool has_optional_clipDataId() const { return this->has_optional_clipDataId_; }

    size_t size() const {
        return this->minimum_size()
             + (  this->has_optional_clipDataId_
                ? 4     /* clipDataId(4) */
                : 0);
    }

    constexpr static size_t minimum_size() {
        return 24;  // streamId(4) + lindex(4) + dwFlags(4) +
                    //     nPositionLow(4) + nPositionHigh(4) +
                    //     cbRequested(4)

    }



private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "FileContentsRequestPDU: streamId=%u(4 bytes) lindex=%u(4 bytes) dwFlags=%u(4 bytes) "
                "nPositionLow=%u(4 bytes) nPositionHigh=%u(4 bytes) cbRequested=%u(4 bytes)",
            this->streamId_, this->lindex_, this->dwFlags_,
            this->nPositionLow_, this->nPositionHigh_, this->cbRequested_);
        length += ((result < size - length) ? result : (size - length - 1));

        if (this->has_optional_clipDataId_) {
            result = ::snprintf(buffer + length, size - length,
                " clipDataId=%u",
                this->clipDataId_);
            length +=
                ((result < size - length) ? result : (size - length - 1));
        }

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};



    // 2.2.5.4 File Contents Response PDU (CLIPRDR_FILECONTENTS_RESPONSE)

    // The File Contents Response PDU is sent as a reply to the File Contents Request PDU.
    // It is used to indicate whether processing of the File Contents Request PDU
    // was successful. If the processing was successful, the File Contents Response PDU
    // includes either a file size or extracted file data, based on the operation
    // requested in the corresponding File Contents Request PDU.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          clipHeader                           |
    // +---------------------------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+
    // |                           streamId                            |
    // +---------------------------------------------------------------+
    // |              requestedFileContentsData (variable)             |
    // +---------------------------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+

    // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
    // Clipboard PDU Header MUST be set to CB_FILECONTENTS_RESPONSE (0x0009).
    // The CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set
    // in the msgFlags field of the Clipboard PDU Header.

    // streamId (4 bytes): An unsigned, 32-bit numeric ID used to associate the
    // File Contents Response PDU with the corresponding File Contents Request PDU.
    // The File Contents Request PDU that triggered the response MUST contain an
    // identical value in the streamId field.

    // requestedFileContentsData (variable): This field contains a variable
    // number of bytes. If the response is to a FILECONTENTS_SIZE (0x00000001)
    // operation, the requestedFileContentsData field holds a 64-bit, unsigned
    // integer containing the size of the file. In the case of a
    // FILECONTENTS_RANGE (0x00000002) operation, the requestedFileContentsData
    // field contains a byte-stream of data extracted from the file.

struct FileContentsResponseSize
{
   uint32_t streamID{0};
   uint64_t size{0};


   FileContentsResponseSize() = default;

   // SIZE (16 bytes)
   explicit FileContentsResponseSize(const uint32_t streamID, const uint64_t size)
   : streamID(streamID)
   , size(size)
   {}

   void receive(InStream & stream) {
       ::check_throw(stream, 12, "FileContentsResponseSize::receive", ERR_RDP_DATA_TRUNCATED);
       this->streamID = stream.in_uint32_le();
       this->size = stream.in_uint64_le();
   }

   void emit(OutStream & stream) const {
       stream.out_uint32_le(this->streamID);
       stream.out_uint64_le(this->size);
   }

   void log() const {
       LOG(LOG_INFO, "     File Contents Response Size: streamID = 0X%08x(4 bytes) size=%" PRIu64 "(8 bytes) Padding - (4 byte) NOT USED", this->streamID, this->size);
   }

   size_t packet_size() {
        return 12;                                          // streamID(4) + size(8)
   }
};


struct FileContentsResponseRange
{
   uint32_t streamID{0};

   FileContentsResponseRange() = default;

   // RANGE (4 + Data_Len Bytes)
   explicit FileContentsResponseRange(const uint32_t streamID)
       : streamID(streamID)
   {}

   void receive(InStream & stream) {
       ::check_throw(stream, 4, "FileContentsResponseRange::receive", ERR_RDP_DATA_TRUNCATED);
       this->streamID = stream.in_uint32_le();
   }

   void emit(OutStream & stream) const {
       stream.out_uint32_le(this->streamID);
   }

   void log() const {
       LOG(LOG_INFO, "     File Contents Response Range: streamID=0X%08x(4 bytes)", this->streamID);
   }

   size_t packet_size() {
        return 4;                                          // streamID(4)
   }
};


struct PacketFileList {
    uint32_t cItems;
    /*variable fileDescriptorArray*/
};

enum : uint64_t {
    TIME64_FILE_LIST = 0x01d1e2a0379fb504
};

// [MS-RDPECLIP] - 2.2.5.2.3.1 File Descriptor (CLIPRDR_FILEDESCRIPTOR)
// ====================================================================

// The CLIPRDR_FILEDESCRIPTOR structure describes the properties of a file.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             flags                             |
// +---------------------------------------------------------------+
// |                      reserved1 (32 bytes)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         fileAttributes                        |
// +---------------------------------------------------------------+
// |                      reserved2 (16 bytes)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         lastWriteTime                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         fileSizeHigh                          |
// +---------------------------------------------------------------+
// |                         fileSizeLow                           |
// +---------------------------------------------------------------+
// |                     fileName (520 bytes)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// flags (4 bytes): An unsigned 32-bit integer that specifies which fields
//  contain valid data and the usage of progress UI during a copy operation.

//  +-------------------+---------------------------------------------------+
//  | Value             | Meaning                                           |
//  +-------------------+---------------------------------------------------+
//  | FD_ATTRIBUTES     | The fileAttributes field contains valid data.     |
//  | 0x00000004        |                                                   |
//  +-------------------+---------------------------------------------------+
//  | FD_FILESIZE       | The fileSizeHigh and fileSizeLow fields contain   |
//  | 0x00000040        | valid data.                                       |
//  +-------------------+---------------------------------------------------+
//  | FD_WRITESTIME     | The lastWriteTime field contains valid data.      |
//  | 0x00000020        |                                                   |
//  +-------------------+---------------------------------------------------+
//  | FD_SHOWPROGRESSUI | A progress indicator SHOULD be shown when copying |
//  | 0x00004000        | the file.                                         |
//  +-------------------+---------------------------------------------------+

enum {
    FD_ATTRIBUTES     = 0x0004,
    FD_FILESIZE       = 0x0040,
    FD_WRITESTIME     = 0x0020,
    FD_SHOWPROGRESSUI = 0x4000
};

// reserved1 (32 bytes): An array of 32 bytes. This field MUST be initialized
//  with zeros when sent and MUST be ignored on receipt.

// fileAttributes (4 bytes): An unsigned 32-bit integer that specifies file
//  attribute flags.

//  +--------------------------+-----------------------------------------------+
//  | Value                    | Meaning                                       |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_READONLY  | A file that is read-only. Applications can    |
//  | 0x00000001               | read the file, but cannot write to it or      |
//  |                          | delete it.                                    |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_HIDDEN    | The file or directory is hidden. It is not    |
//  | 0x00000002               | included in an ordinary directory listing.    |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_SYSTEM    | A file or directory that the operating system |
//  | 0x00000004               | uses a part of, or uses exclusively.          |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_DIRECTORY | Identifies a directory.                       |
//  | 0x00000010               |                                               |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_ARCHIVE   | A file or directory that is an archive file   |
//  | 0x00000020               | or directory. Applications typically use this |
//  |                          | attribute to mark files for backup or         |
//  |                          | removal.                                      |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_NORMAL    | A file that does not have other attributes    |
//  | 0x00000080               | set. This attribute is valid only when used   |
//  |                          | alone.                                        |
//  +--------------------------+-----------------------------------------------+

// reserved2 (16 bytes): An array of 16 bytes. This field MUST be initialized
//  with zeros when sent and MUST be ignored on receipt.

// lastWriteTime (8 bytes): An unsigned 64-bit integer that specifies the
//  number of 100-nanoseconds intervals that have elapsed since 1 January
//  1601 to the time of the last write operation on the file.

// fileSizeHigh (4 bytes): An unsigned 32-bit integer that contains the most
//  significant 4 bytes of the file size.

// fileSizeLow (4 bytes): An unsigned 32-bit integer that contains the least
//  significant 4 bytes of the file size.

// fileName (520 bytes): A null-terminated 260 character Unicode string that
//  contains the name of the file.

class FileDescriptor {

public:
    uint32_t flags = 0;
    uint32_t fileAttributes = 0;
    uint64_t lastWriteTime = 0;
    uint32_t fileSizeHigh = 0;
    uint32_t fileSizeLow = 0;
    std::string file_name;

    FileDescriptor() = default;

    explicit FileDescriptor(std::string name, const uint64_t size, const uint32_t attribute)
      : flags(FD_SHOWPROGRESSUI |FD_FILESIZE | FD_WRITESTIME | FD_ATTRIBUTES)
      , fileAttributes(attribute)
      , lastWriteTime(TIME64_FILE_LIST)
      , fileSizeHigh(size >> 32)
      , fileSizeLow(size)
      , file_name(std::move(name))
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->flags);

        stream.out_clear_bytes(32); // reserved1(32)

        stream.out_uint32_le(this->fileAttributes);

        stream.out_clear_bytes(16); // reserved2(16)

        stream.out_uint64_le(this->lastWriteTime);
        stream.out_uint32_le(this->fileSizeHigh);
        stream.out_uint32_le(this->fileSizeLow);

        // The null-terminator is included.
        uint8_t fileName_unicode_data[520]; // fileName(520)

        size_t size_of_fileName_unicode_data = ::UTF8toUTF16(
            this->file_name,
            fileName_unicode_data, sizeof(fileName_unicode_data));

        stream.out_copy_bytes(fileName_unicode_data,
            size_of_fileName_unicode_data);

        stream.out_clear_bytes(520 /* fileName(520) */ -
            size_of_fileName_unicode_data);
    }

    void receive(InStream & stream)
    {
        // flags(4) + reserved1(32) +
        //     fileAttributes(4) +
        //     reserved2(16) +
        //     lastWriteTime(8) +
        //     fileSizeHigh(4) +
        //     fileSizeLow(4) +
        //     fileName(520)
        check_throw(stream, 592, "FileDescriptor", ERR_RDPDR_PDU_TRUNCATED);

        this->flags = stream.in_uint32_le();

        stream.in_skip_bytes(32);   // reserved1(32)

        this->fileAttributes = stream.in_uint32_le();

        stream.in_skip_bytes(16);   // reserved2(16)

        this->lastWriteTime = stream.in_uint64_le();
        this->fileSizeHigh  = stream.in_uint32_le();
        this->fileSizeLow   = stream.in_uint32_le();

        uint8_t const * const fileName_unicode_data = stream.get_current();
        uint8_t fileName_utf8_string[520 /* fileName(520) */ / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes];
        ::UTF16toUTF8(fileName_unicode_data, 520 /* fileName(520) */ / 2,
            fileName_utf8_string, sizeof(fileName_utf8_string));
        // The null-terminator is included.
        this->file_name = ::char_ptr_cast(fileName_utf8_string);

        stream.in_skip_bytes(520);       // fileName(520)
    }

    const char * fileName() const { return this->file_name.c_str(); }

    uint64_t file_size() const {
        return (static_cast<uint64_t>(this->fileSizeLow)) |
            (static_cast<uint64_t>(this->fileSizeHigh) << 32);
    }

    static constexpr size_t size() {
        return 592; // flags(4) + reserved1(32) + fileAttributes(4) +
                    //     reserved2(16) + lastWriteTime(8) +
                    //     fileSizeHigh(4) + fileSizeLow(4) +
                    //     fileName(520)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileDescriptor: flags=0x%X fileAttributes=0x%X lastWriteTime=%" PRIu64 " "
                "fileSizeHigh=0x%X fileSizeLow=0x%X "
                "fileName=\"%s\"",
            this->flags, this->fileAttributes, this->lastWriteTime,
            this->fileSizeHigh, this->fileSizeLow, this->file_name.c_str());
        return ((length < size) ? length : size - 1);
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class file_descriptor







// 2.2.5.2.1 Packed Metafile Payload (CLIPRDR_MFPICT)

// The CLIPRDR_MFPICT structure is used to transfer a Windows metafile. The Windows metafile format is specified in [MS-WMF] section 2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          mappingMode                          |
// +---------------------------------------------------------------+
// |                             xExt                              |
// +---------------------------------------------------------------+
// |                             yExt                              |
// +---------------------------------------------------------------+
// |                     metaFileData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// mappingMode (4 bytes): An unsigned, 32-bit integer specifying the mapping mode in which the picture is drawn.

enum {
    MM_TEXT        = 0x00000001, // Each logical unit is mapped to one device pixel. Positive x is to the right; positive y is down.
    MM_LOMETRIC    = 0x00000002, // Each logical unit is mapped to 0.1 millimeter. Positive x is to the right; positive y is up.
    MM_HIMETRIC    = 0x00000003, // Each logical unit is mapped to 0.01 millimeter. Positive x is to the right; positive y is up.
    MM_LOENGLISH   = 0x00000004, // Each logical unit is mapped to 0.01 inch. Positive x is to the right; positive y is up.
    MM_HIENGLISH   = 0x00000005, // Each logical unit is mapped to 0.001 inch. Positive x is to the right; positive y is up.
    MM_TWIPS       = 0x00000006, // Each logical unit is mapped to 1/20 of a printer's point (1/1440 of an inch), also called a twip. Positive x is to the right; positive y is up.
    MM_ISOTROPIC   = 0x00000007, // ogical units are mapped to arbitrary units with equally scaled axes; one unit along the x-axis is equal to one unit along the y-axis.
    MM_ANISOTROPIC = 0x00000008  // LoFormatDataResponsePDUgical units are mapped to arbitrary units with arbitrarily scaled axes.
};

//    For MM_ISOTROPIC and MM_ANISOTROPIC modes, which can be scaled, the xExt and yExt fields contain an optional suggested size in MM_HIMETRIC units. For MM_ANISOTROPIC pictures, xExt and yExt SHOULD be zero when no suggested size is given. For MM_ISOTROPIC pictures, an aspect ratio MUST be supplied even when no suggested size is given. If a suggested size is given, the aspect ratio is implied by the size. To give an aspect ratio without implying a suggested size, the xExt and yExt fields are set to negative values whose ratio is the appropriate aspect ratio. The magnitude of the negative xExt and yExt values is ignored; only the ratio is used.

//xExt (4 bytes): An unsigned, 32-bit integer that specifies the width of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

//yExt (4 bytes): An unsigned, 32-bit integer that specifies the height of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

//metaFileData (variable): The variable sized contents of the metafile as specified in [MS-WMF] section 2


// [MS-RDPECLIP] 2.2.5.2 Format Data Response PDU (CLIPRDR_FORMAT_DATA_RESPONSE)
// =============================================================================

// The Format Data Response PDU is sent as a reply to the Format Data Request
//  PDU. It is used to indicate whether processing of the Format Data Request
//  PDU was successful. If the processing was successful, the Format Data
//  Response PDU includes the contents of the requested clipboard data.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 requestedFormatData (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_DATA_RESPONSE (0x0005). The
//  CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set in
//  the msgFlags field of the Clipboard PDU Header structure.

// requestedFormatData (variable): Variable length clipboard format data. The
//  contents of this field MUST be one of the following types: generic, Packed
//  Metafile Payload, or Packed Palette Payload.



enum : uint32_t {
    SRCCOPY = 0x00CC0020
};

enum : uint16_t {
    PLANES_NUMBER = 0x00001
};

enum : int {
      METAFILE_HEADERS_SIZE          = 130
    , META_DIBSTRETCHBLT_HEADER_SIZE = 66
    , METAFILE_WORDS_HEADER_SIZE     = (METAFILE_HEADERS_SIZE/2) -6
    , META_HEADER_SIZE               = 9

    , META_SETMAPMODE_WORDS_SIZE   = 4
    , META_SETWINDOWEXT_WORDS_SIZE = 5
    , META_SETWINDOWORG_WORDS_SIZE = 5
    , BITMAPINFO_HEADER_SIZE       = 40

};

// TODO: We should only create a FormatDataResponsePDU when we have enough data to build it
// this probably implies reading the header and checking if we received enough data

struct FormatDataResponsePDU
{
    void emit(OutStream & stream, const uint8_t * data, size_t data_length) const {
        if (data_length
        // in some case (VNC clipboard) we already have data inplace
        // in these cases no need to copy anything
        && data != stream.get_data())
        {
            stream.out_copy_bytes(data, data_length);
        }
    }

    void log() const {
        LOG(LOG_INFO, "FormatDataResponsePDU:");
    }


}; // struct FormatDataResponsePDU


struct FormatDataResponsePDU_MetaFilePic {

    // 2.2.5.2.1 Packed Metafile Payload

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          mappingMode                          |
    // +---------------------------------------------------------------+
    // |                             xExt                              |
    // +---------------------------------------------------------------+
    // |                             yExt                              |
    // +---------------------------------------------------------------+
    // |                    metaFileData (variable)                    |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // mappingMode (4 bytes): An unsigned, 32-bit integer specifying the mapping mode in which the picture is drawn.

    //  +----------------------------+--------------------------------------------+
    //  | Value                      | Meaning                                    |
    //  +----------------------------+--------------------------------------------+
    //  | MM_TEXT                    | Each logical unit is mapped to one device  |
    //  |                            | pixel. Positive x is to the right; positive|
    //  | 0x00000001                 | y is down.                                 |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_LOMETRIC                | Each logical unit is mapped to 0.1         |
    //  |                            | millimeter. Positive x is to the right;    |
    //  | 0x00000002                 | positive y is up.                          |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_HIMETRIC                | Each logical unit is mapped to 0.01        |
    //  |                            | millimeter. Positive x is to the right;    |
    //  | 0x00000003                 | positive y is up.                          |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_LOENGLISH               | Each logical unit is mapped to 0.01        |
    //  |                            | inch. Positive x is to the right;          |
    //  | 0x00000004                 | positive y is up.                          |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_HIENGLISH               | Each logical unit is mapped to 0.001       |
    //  |                            | inch. Positive x is to the right;          |
    //  | 0x00000005                 | positive y is up.                          |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_TWIPS                   | Each logical unit is mapped to 1/20 of a   |
    //  |                            | printer's point (1/1440 of an inch), also  |
    //  | 0x00000006                 | called a twip. Positive x is to the right; |
    //  |                            | positive y is up.                          |                                                   //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_ISOTROPIC               | Logical units are mapped to arbitrary units|
    //  |                            | with equally scaled axes; one unit along   |
    //  | 0x00000007                 | the x-axis is equal to one unit along the  |
    //  |                            | y-axis.                                    |                                                   //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+
    //  | MM_ANISOTROPIC             | Logical units are mapped to arbitrary units|
    //  |                            | with arbitrarily scaled axes.              |
    //  | 0x00000008                 |                                            |
    //  |                            |                                            |
    //  +----------------------------+--------------------------------------------+

    // For MM_ISOTROPIC and MM_ANISOTROPIC modes, which can be scaled, the xExt and yExt fields contain an optional suggested size in MM_HIMETRIC units. For MM_ANISOTROPIC pictures, xExt and yExt SHOULD be zero when no suggested size is given. For MM_ISOTROPIC pictures, an aspect ratio MUST be supplied even when no suggested size is given. If a suggested size is given, the aspect ratio is implied by the size. To give an aspect ratio without implying a suggested size, the xExt and yExt fields are set to negative values whose ratio is the appropriate aspect ratio. The magnitude of the negative xExt and yExt values is ignored; only the ratio is used.

    // xExt (4 bytes): An unsigned, 32-bit integer that specifies the width of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

    // yExt (4 bytes): An unsigned, 32-bit integer that specifies the height of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

    // metaFileData (variable): The variable sized contents of the metafile as specified in [MS-WMF] section 2.

    uint32_t mappingMode{0};
    uint32_t xExt{0};
    uint32_t yExt{0};

    MFF::MetaHeader metaHeader;
    MFF::MetaSetMapMod metaSetMapMod;
    MFF::MetaSetWindowExt metaSetWindowExt;
    MFF::MetaSetWindowOrg metaSetWindowOrg;
    MFF::DibStretchBLT dibStretchBLT;


    struct Ender {
        enum : uint32_t {
            SIZE = 6
          , FLAG = 0x03
        };

        void emit(uint8_t * chunk, const size_t data_len) {
            chunk[data_len + 1] = FLAG;
            chunk[data_len + 2] = 0x00;
            chunk[data_len + 3] = 0x00;
            chunk[data_len + 4] = 0x00;
            chunk[data_len + 5] = 0x00;
            chunk[data_len + 6] = 0x00;
        }
    };

    void log() const {
//         this->header.log();
        LOG(LOG_INFO, "     Packed Metafile Payload:");
        LOG(LOG_INFO, "          * mappingMode = 0x%08x (4 bytes)", this->mappingMode);
        LOG(LOG_INFO, "          * xExt        = %d (4 bytes)", int(this->xExt));
        LOG(LOG_INFO, "          * yExt        = %d (4 bytes)", int(this->yExt));
        this->metaHeader.log();
        this->metaSetMapMod.log();
        this->metaSetWindowExt.log();
        this->metaSetWindowOrg.log();
        this->dibStretchBLT.log();
    }

    explicit FormatDataResponsePDU_MetaFilePic()
      : metaHeader(0, 0, 0)
      , metaSetMapMod(0)
      , metaSetWindowExt(0, 0)
      , metaSetWindowOrg(0, 0)
      , dibStretchBLT(0, 0, 0, 0, SRCCOPY)
    {}

    explicit FormatDataResponsePDU_MetaFilePic( const std::size_t data_length
                                              , const uint16_t width
                                              , const uint16_t height
                                              , const uint16_t depth
                                              , const double ARBITRARY_SCALE)
//       : FormatDataResponsePDU(data_length + METAFILE_HEADERS_SIZE)
  :  mappingMode(MM_ANISOTROPIC)
      , xExt(int(double(width ) * ARBITRARY_SCALE))
      , yExt(int(double(height) * ARBITRARY_SCALE))
      , metaHeader(MFF::MEMORYMETAFILE, MFF::METAVERSION300, data_length)
      , metaSetMapMod(MM_ANISOTROPIC)
      , metaSetWindowExt(height, width)
      , metaSetWindowOrg(0, 0)
      , dibStretchBLT(data_length, height, width, depth, SRCCOPY)
    {}

    void emit(OutStream & stream) const {
//         this->header.emit(stream);

        // 2.2.5.2.1 Packed Metafile Payload
        // 12 bytes
        stream.out_uint32_le(this->mappingMode);
        stream.out_uint32_le(this->xExt);
        stream.out_uint32_le(this->yExt);

        this->metaHeader.emit(stream);

        this->metaSetMapMod.emit(stream);

        this->metaSetWindowExt.emit(stream);

        this->metaSetWindowOrg.emit(stream);

        this->dibStretchBLT.emit(stream);
    }


    void recv(InStream & stream) {
//         this->header.recv(stream);
        this->mappingMode = stream.in_uint32_le();
        this->xExt = stream.in_uint32_le();
        this->yExt = stream.in_uint32_le();

        this->metaHeader.recv(stream);

        InStream stream_header = stream.clone();
        bool notEOF(true);
        while(notEOF) {

            int recordSize = stream_header.in_uint32_le();
            int type = stream_header.in_uint16_le();

            switch (type) {

                case MFF::META_SETWINDOWEXT:
                    stream_header.in_skip_bytes(4);
                    this->metaSetWindowExt.recv(stream);
                    break;

                case MFF::META_SETWINDOWORG:
                    stream_header.in_skip_bytes(4);
                    this->metaSetWindowOrg.recv(stream);
                    break;

                case MFF::META_SETMAPMODE:
                    this->metaSetMapMod.recv(stream);
                    stream_header.in_skip_bytes(2);
                    assert(this->mappingMode == this->metaSetMapMod.mappingMode);
                    break;

                case MFF::META_DIBSTRETCHBLT:
                    {
                        notEOF = false;
                        this->dibStretchBLT.recv(stream);
                        assert(this->metaHeader.maxRecord == this->dibStretchBLT.recordSize);
                        assert(this->metaSetWindowExt.height == this->dibStretchBLT.destHeight);
                        assert(this->metaSetWindowExt.width == this->dibStretchBLT.destWidth);
                    }
                    break;

                default:
                    size_t len_to_skyp = recordSize*2;
                    LOG(LOG_WARNING, "DEFAULT: unknown record type=%x size=%zu octets", unsigned(type), len_to_skyp);
                    if (len_to_skyp <= stream.in_remain()) {
                        stream.in_skip_bytes(len_to_skyp - 6);
                    } else {
                        notEOF = false;
                    }
                    break;
            }
        }
    }
};



// 2.2.5.2.3 Packed File List (CLIPRDR_FILELIST)

//  The CLIPRDR_FILELIST structure is used to describe a list of files, each file in the list being represented by a File Descriptor (section 2.2.5.2.3.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                            cItems                             |
// +---------------------------------------------------------------+
// |                 fileDescriptorArray (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cItems (4 bytes): An unsigned 32-bit integer that specifies the number of entries in the fileDescriptorArray field.

// fileDescriptorArray (variable): An array of File Descriptors (section 2.2.5.2.3.1). The number of elements in the array is specified by the cItems field.

struct FormatDataResponsePDU_FileList {

    int cItems{0};

    void log() const {
        LOG(LOG_INFO, "FormatDataResponseFileListPDU: cItems=%d(4 bytes)", this->cItems);
    }

    explicit FormatDataResponsePDU_FileList() = default;

    explicit FormatDataResponsePDU_FileList(const std::size_t cItems)
    : cItems(cItems)
    {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->cItems);
    }

    void recv(InStream & stream) {
        this->cItems = stream.in_uint32_le();
    }
};

class MetaFilePicDescriptor
{

public:
    int recordSize{0};
    int height{0};
    int width{0};
    int bpp{0};
    int imageSize{0};


    MetaFilePicDescriptor() = default;

    void receive(InStream & chunk) {
        // Header
        chunk.in_skip_bytes(8);

        // 2.2.5.2.1 Packed Metafile Payload (cliboard.hpp)
        chunk.in_skip_bytes(12);

        // 3.2.1 META_HEADER Example
        chunk.in_skip_bytes(18);

        bool notEOF(true);
        while(notEOF) {

            // 2.3 WMF Records
            int size = chunk.in_uint32_le() * 2;
            int type = chunk.in_uint16_le();

            switch (type) {

                case MFF::META_SETWINDOWEXT:
                    chunk.in_skip_bytes(4);
                break;

                case MFF::META_SETWINDOWORG:
                    chunk.in_skip_bytes(4);
                break;

                case MFF::META_SETMAPMODE:
                    chunk.in_skip_bytes(2);
                break;

                case MFF::META_DIBSTRETCHBLT:
                {
                    notEOF = false;
                    this->recordSize = size;

                    // 2.3.1.3.1 META_DIBSTRETCHBLT With Bitmap
                    chunk.in_skip_bytes(4);

                    this->height = chunk.in_uint16_le();
                    this->width  = chunk.in_uint16_le();
                    chunk.in_skip_bytes(12);

                    // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object

                    // 2.2.2.3 BitmapInfoHeader Object
                    chunk.in_skip_bytes(14);

                    this->bpp     = chunk.in_uint16_le();
                    chunk.in_skip_bytes(4);

                    this->imageSize   = chunk.in_uint32_le();
                    //assert(this->imageSize == ((this->bpp/8) * this->height * this->width));
                    chunk.in_skip_bytes(8);

                    int skip(0);
                    if (chunk.in_uint32_le() == 0) { // if colorUsed == 0
                        skip = 0;
                    }
                    chunk.in_skip_bytes(4);

                        // Colors (variable)
                    chunk.in_skip_bytes(skip);

                        // BitmapBuffer (variable)
                    chunk.in_skip_bytes(0);
                }
                break;

                default: LOG(LOG_INFO, "DEFAULT: unknown record type=%x size=%d octets", unsigned(type), size);
                         chunk.in_skip_bytes(size - 6);

                break;
            }
        }
    }

};  // class MetaFilePicDescriptor


// 2.2.4.1 Lock Clipboard Data PDU (CLIPRDR_LOCK_CLIPDATA)

// The Lock Clipboard Data PDU can be sent at any point in time after the clipboard capabilities and temporary directory have been exchanged in the Clipboard Initialization Sequence (section 1.3.2.1) by a Local Clipboard Owner (section 1.3.2.2.1). The purpose of this PDU is to request that the Shared Clipboard Owner (section 1.3.2.2.1) retain all File Stream (section 1.3.1.1.5) data on the clipboard until the Unlock Clipboard Data PDU (section 2.2.4.2) is received. This ensures that File Stream data can be requested by the Local Owner in a subsequent File Contents Paste Sequence (section 1.3.2.2.3) by using the File Contents Request PDU (section 2.2.5.3) even when the Shared Owner clipboard has changed and the File Stream data is no longer available.

 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
 // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // |                          clipHeader                           |
 // +---------------------------------------------------------------+
 // |                              ...                              |
 // +---------------------------------------------------------------+
 // |                          clipDataId                           |
 // +-------------------------------+-------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_LOCK_CLIPDATA (0x000A), while the msgFlags field MUST be set to 0x0000.

// clipDataId (4 bytes): An unsigned, 32-bit integer that is used to tag File Stream data on the Shared Owner clipboard so that it can be requested in a subsequent File Contents Request PDU (section 2.2.5.3).

struct LockClipboardDataPDU
{
    uint32_t clipDataId;

    explicit LockClipboardDataPDU() = default;

    explicit LockClipboardDataPDU(uint32_t clipDataId)
    : clipDataId(clipDataId)
    {}

    void emit(OutStream & stream) const
    {
        stream.out_uint32_le(this->clipDataId);
    }

    void recv(InStream & stream)
    {
        check_throw(stream, 4, "LockClipboardDataPDU", ERR_RDPDR_PDU_TRUNCATED);
        clipDataId = stream.in_uint32_le();
    }

    void log() const
    {
        LOG(LOG_INFO, "LockClipboardDataPDU: clipDataId=0x%08x(4 bytes)", this->clipDataId);
    }

};



 // 2.2.4.2 Unlock Clipboard Data PDU (CLIPRDR_UNLOCK_CLIPDATA)

 // The Unlock Clipboard Data PDU can be sent at any point in time after the Clipboard Initialization Sequence (section 1.3.2.1) by a Local Clipboard Owner (section 1.3.2.2.1). The purpose of this PDU is to notify the Shared Clipboard Owner (section 1.3.2.2.1) that File Stream data that was locked in response to the Lock Clipboard Data PDU (section 2.2.4.1) can be released.

 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
 // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // |                          clipHeader                           |
 // +---------------------------------------------------------------+
 // |                              ...                              |
 // +---------------------------------------------------------------+
 // |                          clipDataId                           |
 // +-------------------------------+-------------------------------+

 // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_UNLOCK_CLIPDATA (0x000B), while the msgFlags field MUST be set to 0x0000.

 // clipDataId (4 bytes): An unsigned, 32-bit integer that identifies the File Stream data that was locked by the Lock Clipboard Data PDU (section 2.2.4.1) and can now be released.

struct UnlockClipboardDataPDU
{
    uint32_t clipDataId;

    explicit UnlockClipboardDataPDU() = default;

    explicit UnlockClipboardDataPDU(uint32_t clipDataId)
    : clipDataId(clipDataId)
    {}

    void emit(OutStream & stream) const
    {
        stream.out_uint32_le(this->clipDataId);
    }

    void recv(InStream & stream)
    {
        check_throw(stream, 4, "LockClipboardDataPDU", ERR_RDPDR_PDU_TRUNCATED);
        clipDataId = stream.in_uint32_le();
    }

    void log() const
    {
        LOG(LOG_INFO, "UnlockClipboardDataPDU: clipDataId=0x%08x(4 bytes)", this->clipDataId);
    }
};


struct CliprdrLogState
{
    bool use_long_format_names     = true;
    int  file_content_request_flag = 0;
    int  requestedFormatId         = 0;
};


static inline void streamLogCliprdr(InStream & chunk, int flags, CliprdrLogState & state) {
    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
//         InStream chunk =  stream.clone();

        CliprdrHeader header;
        header.recv(chunk);

        switch (header.msgType()) {

            case CB_MONITOR_READY:
            {
                header.log();

                ServerMonitorReadyPDU pdu;
                pdu.recv(chunk);
                pdu.log();
            }
                break;

            case CB_FORMAT_LIST:
            {
                header.log();

                FormatListPDUEx format_list_pdu;
                format_list_pdu.recv(chunk, state.use_long_format_names, (header.msgFlags() & CB_ASCII_NAMES));
                format_list_pdu.log(LOG_INFO);
            }
                break;

            case CB_FORMAT_LIST_RESPONSE:
            {
                header.log();

                FormatListResponsePDU pdu;
                pdu.recv(chunk);
                pdu.log();
            }
                break;

            case CB_FORMAT_DATA_REQUEST:
            {
                FormatDataRequestPDU pdu;
                pdu.recv(chunk);
                pdu.log();

                state.requestedFormatId = pdu.requestedFormatId;
            }
                break;

            case CB_FORMAT_DATA_RESPONSE:

                switch (state.requestedFormatId) {

                    case CF_METAFILEPICT:
                    {
                        FormatDataResponsePDU_MetaFilePic pdu;
                        pdu.recv(chunk);
                        pdu.log();
                    }
                        break;

                    case CF_TEXT:
                    case CF_UNICODETEXT:
                    {
//                         FormatDataResponsePDU_Text pdu;
//                         pdu.recv(stream);
//                         pdu.log();
                    }
                        break;

                    default: LOG(LOG_INFO, "CLIPRDR CB_FORMAT_DATA_RESPONSE format data id = %d %s", state.requestedFormatId, get_FormatId_name(state.requestedFormatId));
                        break;
                }
                break;

            case CB_TEMP_DIRECTORY:
            {
                header.log();

                ClientTemporaryDirectoryPDU pdu;
                pdu.recv(chunk);
                pdu.log();
            }
                break;

            case CB_CLIP_CAPS:
            {
                header.log();

                ClipboardCapabilitiesPDU pdu;
                pdu.recv(chunk);
                pdu.log();

                GeneralCapabilitySet caps;
                caps.recv(chunk);
                caps.log();

                if (state.use_long_format_names) {
                    state.use_long_format_names = bool(caps.generalFlags() & CB_USE_LONG_FORMAT_NAMES);
                }
            }
                break;

            case CB_FILECONTENTS_REQUEST:
            {
                FileContentsRequestPDU pdu;
                pdu.receive(chunk);
                pdu.log(LOG_INFO);

                state.file_content_request_flag = pdu.dwFlags();
            }
                break;

            case CB_FILECONTENTS_RESPONSE:

                switch (state.file_content_request_flag) {

                    case FILECONTENTS_SIZE:
                    {
                        FileContentsResponseSize pdu;
                        pdu.receive(chunk);
                        pdu.log();
                    }
                        break;

                    case FILECONTENTS_RANGE:
                    {
                        FileContentsResponseRange pdu;
                        pdu.receive(chunk);
                        pdu.log();
                    }
                        break;

                    default: LOG(LOG_WARNING, "CLIPRDR Unknown CB_FILECONTENTS_RESPONSE with flag = %d", state.file_content_request_flag);
                        break;
                }
                break;

            case CB_LOCK_CLIPDATA:
            {
                LockClipboardDataPDU pdu;
                pdu.recv(chunk);
                pdu.log();
            }
                break;

            case CB_UNLOCK_CLIPDATA:
            {
                UnlockClipboardDataPDU pdu;
                pdu.recv(chunk);
                pdu.log();
            }
                break;

            default:
                header.log();
                LOG(LOG_WARNING, "CLIPRDR Unknown PDU!");
                break;
        }
    }
}

}   // namespace RDPECLIP


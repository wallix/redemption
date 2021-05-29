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

#include "core/RDP/clipboard/format_list_extract.hpp"
#include "core/RDP/clipboard.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/stream.hpp"

namespace Cliprdr
{
    struct FormatNameRef
    {
        uint32_t _format_id;
        bytes_view _utf8_bytes;

        [[nodiscard]] uint32_t format_id() const noexcept { return this->_format_id; }
        [[nodiscard]] bytes_view utf8_name() const noexcept { return this->_utf8_bytes; }
    };


    template<class Fw, class Senti>
    bool will_be_sent_in_ASCII_8(Fw first, Senti const& end)
    {
        size_t max_format_name_len_in_char = 0;
        for (; first != end; ++first) {
            auto&& format = *first;

            if (!is_ASCII_string(format.utf8_name())) {
                return false;
            }

            const size_t format_name_len_in_char = format.utf8_name().size();
            if (max_format_name_len_in_char < format_name_len_in_char) {
                max_format_name_len_in_char = format_name_len_in_char;
            }
        }

        return (max_format_name_len_in_char >= (32 /* formatName(32) */ / sizeof(uint16_t)));
    }

    template<class Format>
    bool format_list_serialize_long_format(OutStream& out_stream, Format const& format)
    {
        if (!out_stream.has_room(min_long_format_name_data_length)) {
            return false;
        }

        out_stream.out_uint32_le(format.format_id());

        auto data = out_stream.get_tail();
        auto len = UTF8toUTF16(format.utf8_name(), data);

        if (len + 2 > data.size())
        {
            out_stream.rewind(out_stream.get_offset() - 4u);
            return false;
        }

        out_stream.out_skip_bytes(len);
        out_stream.out_uint16_le(0);

        return true;
    }

    template<class Format>
    bool format_list_serialize_ascii_format(OutStream& out_stream, Format const& format)
    {
        if (!out_stream.has_room(short_format_name_data_length)) {
            return false;
        }

        out_stream.out_uint32_le(format.format_id());

        auto const& name = format.utf8_name();
        auto len = std::min(name.size(), Cliprdr::short_format_name_length - 1u);

        out_stream.out_copy_bytes({name.data(), len});
        out_stream.out_clear_bytes(Cliprdr::short_format_name_length - len);

        return true;
    }

    template<class Format>
    bool format_list_serialize_unicode_format(OutStream& out_stream, Format const& format)
    {
        if (!out_stream.has_room(short_format_name_data_length)) {
            return false;
        }

        out_stream.out_uint32_le(format.format_id());

        auto data = out_stream.get_tail();
        data = data.first(std::min(Cliprdr::short_format_name_length - 2u, data.size()));

        data = out_stream.out_skip_bytes(UTF8toUTF16(format.utf8_name(), data));
        out_stream.out_clear_bytes(Cliprdr::short_format_name_length - data.size());

        return true;
    }

    template<class Fw, class Senti>
    bool format_list_serialize_with_header(
        OutStream& out_stream, IsLongFormat is_long_format,
        Fw first, Senti const& end)
    {
        if (!out_stream.has_room(RDPECLIP::CliprdrHeader::size()))
        {
            return false;
        }

        OutStream out_stream_header(out_stream.out_skip_bytes(RDPECLIP::CliprdrHeader::size()));
        auto out_stream_start_pos = out_stream.get_offset();
        auto ascii_flag = RDPECLIP::CB_RESPONSE_NONE;

        if (bool(is_long_format))
        {
            while (first != end && format_list_serialize_long_format(out_stream, *first))
            {
                ++first;
            }
        }
        else if (will_be_sent_in_ASCII_8(first, end))
        {
            ascii_flag = RDPECLIP::CB_ASCII_NAMES;
            while (first != end && format_list_serialize_ascii_format(out_stream, *first))
            {
                ++first;
            }
        }
        else
        {
            while (first != end && format_list_serialize_unicode_format(out_stream, *first))
            {
                ++first;
            }
        }

        RDPECLIP::CliprdrHeader(
            RDPECLIP::CB_FORMAT_LIST, ascii_flag,
            out_stream.get_offset() - out_stream_start_pos)
        .emit(out_stream_header);

        return (first == end);
    }

    template<class Formats>
    bool format_list_serialize_with_header(
        OutStream& out_stream, IsLongFormat is_long_format,
        Formats const& formats)
    {
        using std::begin;
        using std::end;
        return format_list_serialize_with_header(
            out_stream, is_long_format, begin(formats), end(formats));
    }
} // namespace Cliprdr

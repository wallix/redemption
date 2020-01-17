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

#include "core/RDP/clipboard/format_name.hpp"
#include "utils/stream.hpp"
#include "utils/utf.hpp"


namespace Cliprdr
{
    // formatId(4) + wszFormatName(variable, min = "\x00\x00" => 2)
    constexpr size_t min_long_format_name_data_length = 6;

    // formatId(4) + formatName(32)
    constexpr size_t short_format_name_length = 32;
    constexpr size_t short_format_name_data_length = short_format_name_length + 4;

    namespace detail
    {
        /**
        * Used for return true or left value
        * \code
            void foo();
            return foo(), LeftOrTrue{}; // return true
        * \endcode
        * \code
            int foo() { return 42; }
            return foo(), LeftOrTrue{}; // return 42
        * \endcode
        */
        struct LeftOrTrue
        {
            constexpr operator bool () const noexcept { return true; }
        };

        template<class T, class U>
        T&& operator,(T&& x, LeftOrTrue const& /*dummy*/) noexcept
        {
            return static_cast<T&&>(x);
        }
    } // namespace detail

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

    template<class ProcessFormat>
    bool format_list_extract_long_format(InStream& in_stream, ProcessFormat&& process_format)
    {
        if (in_stream.in_check_rem(min_long_format_name_data_length))
        {
            uint32_t format_id = in_stream.in_uint32_le();

            auto name = in_stream.remaining_bytes();
            auto end_name_pos = UTF16ByteLen(name);
            auto end_format_pos = end_name_pos + 2;

            if (!in_stream.in_check_rem(end_format_pos))
            {
                in_stream.rewind(in_stream.get_offset() - 4);
                return false;
            }

            in_stream.in_skip_bytes(end_format_pos);

            return process_format(format_id, UnicodeName(name.first(end_name_pos))), detail::LeftOrTrue{};
        }

        return false;
    }

    template<class ProcessFormat>
    bool format_list_extract_ascii_format(InStream& in_stream, ProcessFormat&& process_format)
    {
        if (in_stream.in_check_rem(short_format_name_data_length))
        {
            uint32_t format_id = in_stream.in_uint32_le();

            auto name = in_stream.remaining_bytes();
            in_stream.in_skip_bytes(short_format_name_length);
            name = name.first(strnlen(name.as_charp(), short_format_name_length));

            return process_format(format_id, AsciiName(name)), detail::LeftOrTrue{};
        }

        return false;
    }

    template<class ProcessFormat>
    bool format_list_extract_unicode_format(InStream& in_stream, ProcessFormat&& process_format)
    {
        if (in_stream.in_check_rem(short_format_name_data_length))
        {
            uint32_t format_id = in_stream.in_uint32_le();

            auto name = in_stream.remaining_bytes();
            in_stream.in_skip_bytes(short_format_name_length);
            name = name.first(UTF16ByteLen(name.first(short_format_name_length)));

            return process_format(format_id, UnicodeName(name)), detail::LeftOrTrue{};
        }

        return false;
    }

    template<class ProcessFormat>
    void format_list_extract(
        InStream& in_stream, IsLongFormat is_long_format, IsAscii is_ascii,
        ProcessFormat&& process_format)
    {
        if (bool(is_long_format))
        {
            while (format_list_extract_long_format(in_stream, process_format))
                ;
        }
        else if (bool(is_ascii))
        {
            while (format_list_extract_ascii_format(in_stream, process_format))
                ;
        }
        else
        {
            while (format_list_extract_unicode_format(in_stream, process_format))
                ;
        }
    }
} // namespace Cliprdr

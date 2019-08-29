/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/utf.hpp"


/// \return size of unicode data
inline size_t put_non_null_terminated_utf16_from_utf8(
    OutStream & out, bytes_view utf8_string,
    const size_t maximum_length_in_bytes,
    const size_t offset_of_data_length
) noexcept {
    uint8_t * const unicode_data = out.get_current();
    const size_t size_of_unicode_data = ::UTF8toUTF16(utf8_string, unicode_data, maximum_length_in_bytes);
    out.out_skip_bytes(size_of_unicode_data);

    out.stream_at(offset_of_data_length).out_uint16_le(size_of_unicode_data);

    return size_of_unicode_data;
}

/// \return size of unicode data
inline size_t put_non_null_terminated_utf16_from_utf8(
    OutStream & out, bytes_view utf8_string,
    const size_t maximum_length_in_bytes
) noexcept {
    const size_t offset_of_data_length = out.get_offset();
    out.out_skip_bytes(2);
    return put_non_null_terminated_utf16_from_utf8(
        out, utf8_string, maximum_length_in_bytes, offset_of_data_length);
}

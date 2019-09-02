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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/limited_array.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/utf.hpp"


template<std::size_t ByteCapacity>
struct utf8_array
{
    explicit utf8_array(bytes_view utf8_string) noexcept
    : array(utf8_string.begin(), utf8_string.end())
    {}

    uint8_t const* data() const noexcept
    {
        return array.data();
    }

    /// \return the size of the sequence in bytes
    std::size_t size() const noexcept
    {
        return array.size();
    }

private:
    limited_array<uint8_t, ByteCapacity> array;
};


template<std::size_t ByteCapacity>
struct utf16_array
{
    explicit utf16_array(bytes_view utf8_string) noexcept
    : array(utf8_string.begin(), utf8_string.end())
    {}

    uint8_t const* data() const noexcept
    {
        return array.data();
    }

    /// \return the size of the sequence in bytes
    std::size_t size() const noexcept
    {
        return array.size();
    }

private:
    limited_array<uint8_t, ByteCapacity> array;
};

// TODO utf8_array = basic_utf<utf8_tag, limited_array<u8, Capacity>>
// TODO utf8_view = basic_utf<utf16_tag, bytes_view>

template<std::size_t Utf16ByteCapacity = 0, std::size_t ByteCapacity>
utf16_array<Utf16ByteCapacity ? Utf16ByteCapacity : ByteCapacity*2>
to_utf16(utf8_array<ByteCapacity> const& utf8) noexcept
{
    utf16_array<Utf16ByteCapacity ? Utf16ByteCapacity : ByteCapacity*2> utf16;
    utf16.transaction([&](writable_bytes_view av) noexcept {
        return UTF8toUTF16(utf8, av);
    });
    return utf16;
}

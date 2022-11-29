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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "cxx/cxx.hpp"


struct UninitDynamicBuffer
{
    UninitDynamicBuffer() noexcept;

    UninitDynamicBuffer(std::size_t n);
    UninitDynamicBuffer(UninitDynamicBuffer&& other) noexcept;

    UninitDynamicBuffer& operator=(UninitDynamicBuffer&& other) noexcept;

    UninitDynamicBuffer(UninitDynamicBuffer const&) = delete;
    UninitDynamicBuffer& operator=(UninitDynamicBuffer const&) = delete;

    ~UninitDynamicBuffer();

    /// \post size() >= new_size
    writable_bytes_view grow(std::size_t new_size, std::size_t copiable_length, std::size_t copiable_offset = 0)
    {
        grow_impl(new_size, copiable_length, copiable_offset);
        return {data(), new_size};
    }

    /// \post size() >= new_size
    writable_bytes_view grow_without_copy(std::size_t new_size)
    {
        grow_without_copy_impl(new_size);
        return {data(), new_size};
    }

    void free() noexcept;

    void swap(UninitDynamicBuffer& other) noexcept;

    writable_bytes_view buffer() const noexcept
    {
        return {data(), len};
    }

    REDEMPTION_ATTRIBUTE_RETURNS_NONNULL
    uint8_t* data() const noexcept
    {
        return p;
    }

    std::size_t size() const noexcept
    {
        return len;
    }

private:
    /// \post size() >= new_size
    void grow_impl(std::size_t new_size, std::size_t copiable_length, std::size_t copiable_offset = 0);

    /// \post size() >= new_size
    void grow_without_copy_impl(std::size_t new_size);

    uint8_t* p;
    std::size_t len = 0;
};

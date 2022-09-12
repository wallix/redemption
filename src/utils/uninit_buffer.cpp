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

#include "utils/uninit_buffer.hpp"

#include <new>
#include <utility>

#include <cstring>


namespace
{
    constexpr char const* uninit_null_buffer = "";

    static uint8_t* uninit_null_pointer() noexcept
    {
        return reinterpret_cast<uint8_t*>(const_cast<char*>(uninit_null_buffer)); /* NOLINT */
    }

    static void uninit_deallocate(void* p) noexcept
    {
        if (p != uninit_null_buffer) {
            operator delete (p);
        }
    }

    static uint8_t* uninit_allocate(std::size_t n)
    {
        return static_cast<uint8_t*>(operator new(n));
    }
} // anonymous namespace

UninitDynamicBuffer::UninitDynamicBuffer() noexcept
: p(uninit_null_pointer())
{}

UninitDynamicBuffer::UninitDynamicBuffer(std::size_t n)
: p(uninit_allocate(n))
, len(n)
{}

UninitDynamicBuffer::UninitDynamicBuffer(UninitDynamicBuffer&& other) noexcept
: UninitDynamicBuffer()
{
    swap(other);
}

UninitDynamicBuffer& UninitDynamicBuffer::operator=(UninitDynamicBuffer&& other) noexcept
{
    p = std::exchange(other.p, uninit_null_pointer());
    len = std::exchange(other.len, 0);
    return *this;
}

UninitDynamicBuffer::~UninitDynamicBuffer()
{
    uninit_deallocate(p);
}

void UninitDynamicBuffer::grow(std::size_t new_size, std::size_t copiable_length, std::size_t copiable_offset)
{
    if (new_size > len) {
        assert(copiable_length + copiable_offset <= len);
        uint8_t* newp = uninit_allocate(new_size);
        std::memcpy(newp + copiable_offset, p + copiable_offset, copiable_length);
        uninit_deallocate(p);
        p = newp;
        len = new_size;
    }
}

void UninitDynamicBuffer::grow_without_copy(std::size_t new_size)
{
    if (new_size < len) {
        uint8_t* newp = uninit_allocate(new_size);
        uninit_deallocate(p);
        p = newp;
        len = new_size;
    }
}

void UninitDynamicBuffer::free() noexcept
{
    uninit_deallocate(p);
    p = uninit_null_pointer();
    len = 0;
}

void UninitDynamicBuffer::swap(UninitDynamicBuffer& other) noexcept
{
    std::swap(p, other.p);
    std::swap(len, other.len);
}

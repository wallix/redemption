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
Author(s): Proxy Team
*/

#pragma once

#include <charconv>
#include <type_traits>

struct int_to_chars_result
{
    char* data() noexcept { return a; }
    char const* data() const noexcept { return a; }
    std::size_t size() const noexcept { return len; }

    void set_size(std::size_t n) noexcept { len = n; }

    static constexpr std::size_t max_capacity() noexcept { return 20; }

private:
    char a[20];
    std::size_t len;
};

struct int_to_zchars_result
{
    char* data() noexcept { return a; }
    char const* data() const noexcept { return a; }
    std::size_t size() const noexcept { return len; }

    void set_size(std::size_t n) noexcept { len = n; }

    char const* c_str() const noexcept { return a; }

    static constexpr std::size_t max_capacity() noexcept
    {
        return int_to_chars_result::max_capacity() + 1;
    }

private:
    char a[int_to_chars_result::max_capacity() + 1];
    std::size_t len;
};


template<class T>
inline int_to_chars_result int_to_chars(T n) noexcept
{
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(T) <= 64);

    int_to_chars_result chars;
    auto r = std::to_chars(chars.data(), chars.data() + chars.max_capacity(), n);
    chars.set_size(std::size_t(r.ptr - chars.data()));

    return chars;
}

template<class T>
inline int_to_chars_result int_to_zchars(T n) noexcept
{
    static_assert(std::is_integral_v<T>);
    static_assert(sizeof(T) <= 64);

    int_to_chars_result chars;
    auto r = std::to_chars(chars.data(), chars.data() + chars.max_capacity(), n);
    r.ptr = '\0';
    chars.set_size(std::size_t(r.ptr - chars.data()) + 1);

    return chars;
}

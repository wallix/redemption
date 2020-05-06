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

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/std_stream_proto.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <cassert>


struct zstring_view
{
    zstring_view() = default;
    zstring_view(zstring_view &&) = default;
    zstring_view(zstring_view const &) = default;
    zstring_view & operator=(zstring_view &&) = default;
    zstring_view & operator=(zstring_view const &) = default;

    constexpr zstring_view(std::nullptr_t) noexcept
    {}

    explicit zstring_view(char const* s) noexcept = delete;

    zstring_view(std::string const& s) noexcept
    : s(s.c_str())
    , len(s.size())
    {}

    class is_zero_terminated {};

    constexpr zstring_view(is_zero_terminated const& /*tag*/, char const* s, std::size_t n) noexcept
    : s(s)
    , len(n)
    {
        assert(s[len] == 0);
    }

    constexpr zstring_view(is_zero_terminated const& /*tag*/, chars_view str) noexcept
    : zstring_view(is_zero_terminated{}, str.data(), str.size())
    {}

    [[nodiscard]] constexpr char const* c_str() const noexcept { return data(); }

    // TODO temporary ?
    constexpr operator char const* () const noexcept { return c_str(); }

    [[nodiscard]] constexpr char const * data() const noexcept { return this->s; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return this->len; }

    constexpr char operator[](std::size_t i) const noexcept
    {
        assert(i < len);
        return s[i];
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return !this->len; }

    [[nodiscard]] constexpr char const * begin() const noexcept { return this->data(); }
    [[nodiscard]] constexpr char const * end() const noexcept { return this->data() + this->size(); }

    [[nodiscard]] constexpr std::string_view to_sv() const noexcept
    {
        return std::string_view(c_str(), size());
    }

    [[nodiscard]] std::string to_string() const
    {
        return std::string(c_str(), size());
    }

private:
    char const* s = nullptr;
    std::size_t len = 0;
};

inline constexpr bool operator==(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() == rhs.to_sv();
}

inline constexpr bool operator==(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) == rhs.to_sv();
}

inline constexpr bool operator==(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() == std::string_view(rhs);
}

inline constexpr bool operator!=(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() != rhs.to_sv();
}

inline constexpr bool operator!=(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) != rhs.to_sv();
}

inline constexpr bool operator!=(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() != std::string_view(rhs);
}

inline constexpr bool operator<(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() < rhs.to_sv();
}

inline constexpr bool operator<(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) < rhs.to_sv();
}

inline constexpr bool operator<(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() < std::string_view(rhs);
}

inline constexpr bool operator<=(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() <= rhs.to_sv();
}

inline constexpr bool operator<=(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) <= rhs.to_sv();
}

inline constexpr bool operator<=(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() <= std::string_view(rhs);
}

inline constexpr bool operator>(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() > rhs.to_sv();
}

inline constexpr bool operator>(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) > rhs.to_sv();
}

inline constexpr bool operator>(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() > std::string_view(rhs);
}

inline constexpr bool operator>=(zstring_view const& lhs, zstring_view const& rhs) noexcept
{
    return lhs.to_sv() >= rhs.to_sv();
}

inline constexpr bool operator>=(char const* lhs, zstring_view const& rhs) noexcept
{
    return std::string_view(lhs) >= rhs.to_sv();
}

inline constexpr bool operator>=(zstring_view const& lhs, char const* rhs) noexcept
{
    return lhs.to_sv() >= std::string_view(rhs);
}

constexpr zstring_view operator "" _zv(char const * s, std::size_t len) noexcept
{
    return {zstring_view::is_zero_terminated{}, s, len};
}

REDEMPTION_OSTREAM(out, zstring_view const& str)
{
    return out << str.to_sv();
}

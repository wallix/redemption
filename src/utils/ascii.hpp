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

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "utils/string_c.hpp"

namespace detail
{
    inline constexpr char const ascii_to_upper_table[] = {
        '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09',
        '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f', '\x10', '\x11', '\x12', '\x13',
        '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d',
        '\x1e', '\x1f', '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27',
        '\x28', '\x29', '\x2a', '\x2b', '\x2c', '\x2d', '\x2e', '\x2f', '\x30', '\x31',
        '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39', '\x3a', '\x3b',
        '\x3c', '\x3d', '\x3e', '\x3f', '\x40', 'A', 'B', 'C', 'D', 'E',
        'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', '\x5b', '\x5c', '\x5d', '\x5e', '\x5f', '\x60', 'A', 'B', 'C',
        'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '\x7b', '\x7c', '\x7d', '\x7e', '\x7f', '\x80', '\x81',
        '\x82', '\x83', '\x84', '\x85', '\x86', '\x87', '\x88', '\x89', '\x8a', '\x8b',
        '\x8c', '\x8d', '\x8e', '\x8f', '\x90', '\x91', '\x92', '\x93', '\x94', '\x95',
        '\x96', '\x97', '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e', '\x9f',
        '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7', '\xa8', '\xa9',
        '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf', '\xb0', '\xb1', '\xb2', '\xb3',
        '\xb4', '\xb5', '\xb6', '\xb7', '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd',
        '\xbe', '\xbf', '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7',
        '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf', '\xd0', '\xd1',
        '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7', '\xd8', '\xd9', '\xda', '\xdb',
        '\xdc', '\xdd', '\xde', '\xdf', '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5',
        '\xe6', '\xe7', '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef',
        '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7', '\xf8', '\xf9',
        '\xfa', '\xfb', '\xfc', '\xfd', '\xfe',
    };
    static_assert(sizeof(ascii_to_upper_table) == 255);

    struct StringArrayAccess;

    template<class>
    struct extract_string_tag
    {};

    template<class T, class Tag, class R>
    using enable_if_same_tag_t = std::enable_if_t<
        std::is_same<Tag, typename extract_string_tag<T>::type
    >::value, R>;

    constexpr std::size_t unsafe_ascii_to_upper(char* dest, chars_view src) noexcept;
} // anonymous namespace


constexpr inline char ascii_to_upper(char c) noexcept
{
    return detail::ascii_to_upper_table[static_cast<unsigned char>(c)];
}

template<std::size_t N, class Tag>
struct StringArray
{
    constexpr std::string_view sv() const noexcept
    {
        return std::string_view(_buffer.data(), _len);
    }

    template<class U>
    friend constexpr detail::enable_if_same_tag_t<U, Tag, bool>
    operator==(StringArray<N, Tag> const& a, U const& b) noexcept
    {
        return a.sv() == b.sv();
    }

private:
    friend detail::StringArrayAccess;
    std::array<char, N> _buffer;
    std::size_t _len = 0;
};

template<std::size_t N, class Tag>
struct ZStringArray
{
    constexpr char const* c_str() const noexcept
    {
        return _buffer;
    }

    constexpr std::string_view sv() const noexcept
    {
        return std::string_view(_buffer.data(), _len);
    }

    constexpr zstring_view zsv() const noexcept
    {
        return zstring_view::from_null_terminated(_buffer.data(), _len);
    }

    template<class U>
    friend constexpr detail::enable_if_same_tag_t<U, Tag, bool>
    operator==(ZStringArray<N, Tag> const& a, U const& b) noexcept
    {
        return a.sv() == b.sv();
    }

private:
    friend detail::StringArrayAccess;
    std::array<char, N+1> _buffer;
    std::size_t _len = 0;
};

template<class Tag>
struct TaggedString
{
    constexpr std::string_view sv() const noexcept
    {
        return av;
    }

    template<class Tag2, class U>
    friend constexpr detail::enable_if_same_tag_t<U, Tag2, bool>
    operator==(TaggedString<Tag2> const& a, U const& b) noexcept
    {
        return a.sv() == b.sv();
    }

    std::string_view av;
};

namespace detail
{
    template<std::size_t N, class Tag>
    struct extract_string_tag<StringArray<N, Tag>>
    {
        using type = Tag;
    };

    template<std::size_t N, class Tag>
    struct extract_string_tag<ZStringArray<N, Tag>>
    {
        using type = Tag;
    };

    template<class Tag>
    struct extract_string_tag<TaggedString<Tag>>
    {
        using type = Tag;
    };
}


namespace detail
{
    constexpr inline std::size_t unsafe_ascii_to_upper(char* dest, chars_view src) noexcept
    {
        auto* start = dest;
        for (char c : src) {
            *dest++ = ascii_to_upper(c);
        }
        return std::size_t(dest - start);
    }

    struct StringArrayAccess
    {
        template<class SA>
        constexpr static std::size_t& len(SA& sa) noexcept
        {
            return sa._len;
        }

        template<class SA>
        constexpr static auto& buffer(SA& sa) noexcept
        {
            return sa._buffer;
        }
    };
} // anonymous namespace


class UpperTag {};
// class LowerTag {};

template<std::size_t N>
constexpr StringArray<N, UpperTag> ascii_to_limited_upper(chars_view str) noexcept
{
    StringArray<N, UpperTag> upper;
    if (str.size() <= N) {
        auto* p = detail::StringArrayAccess::buffer(upper).data();
        detail::StringArrayAccess::len(upper) = detail::unsafe_ascii_to_upper(p, str);
    }
    return upper;
}

template<std::size_t N>
constexpr ZStringArray<N, UpperTag> ascii_to_limited_zupper(chars_view str) noexcept
{
    ZStringArray<N, UpperTag> upper;
    auto* p = detail::StringArrayAccess::buffer(upper).data();
    auto& len = detail::StringArrayAccess::len(upper);
    if (str.size() <= N) {
         len = detail::unsafe_ascii_to_upper(p, str);
    }
    p[len] = '\0';
    return upper;
}

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wpedantic")
template<class C, C... cs>
constexpr TaggedString<UpperTag> operator "" _ascii_upper() noexcept
{
    return {jln::string_c<ascii_to_upper(cs)...>::sv()};
}
REDEMPTION_DIAGNOSTIC_POP()

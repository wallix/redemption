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
#include "utils/static_string.hpp"
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

    inline constexpr char const ascii_to_lower_table[] = {
        '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09',
        '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f', '\x10', '\x11', '\x12', '\x13',
        '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d',
        '\x1e', '\x1f', '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27',
        '\x28', '\x29', '\x2a', '\x2b', '\x2c', '\x2d', '\x2e', '\x2f', '\x30', '\x31',
        '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39', '\x3a', '\x3b',
        '\x3c', '\x3d', '\x3e', '\x3f', '\x40', 'a', 'b', 'c', 'd', 'e',
        'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
        'z', '\x5b', '\x5c', '\x5d', '\x5e', '\x5f', '\x60', 'a', 'b', 'c',
        'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', '\x7b', '\x7c', '\x7d', '\x7e', '\x7f', '\x80', '\x81',
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
    static_assert(sizeof(ascii_to_lower_table) == 255);

    struct StringAsArrayAccess;

    template<class>
    struct extract_string_tag
    {};

    template<class T, class Tag, class R>
    using enable_if_same_tag_t = std::enable_if_t<
        std::is_same<Tag, typename extract_string_tag<T>::type
    >::value, R>;

    struct StringAsArrayAccess
    {
        template<class SA>
        constexpr static auto& internal(SA& sa) noexcept
        {
            return sa.data;
        }
    };

    template<std::size_t N>
    struct StringAsArray
    {
    private:
        friend class StringAsArrayAccess;
        struct Data
        {
            std::array<char, N> buffer;
            std::size_t len = 0;
        };
        Data data;
    };
} // namespace detail


class UpperTag {};
class LowerTag {};

/// \return true value only for the lowercase letters (abcdefghijklmnopqrstuvwxyz).
constexpr bool ascii_is_lower(char c) noexcept
{
    return 'a' <= c && c <= 'z';
}

/// \return true value only for the uppercase letters (ABCDEFGHIJKLMNOPQRSTUVWXYZ).
constexpr bool ascii_is_upper(char c) noexcept
{
    return 'A' <= c && c <= 'Z';
}

constexpr bool ascii_contains_lower(chars_view str) noexcept
{
    for (char c : str) {
        if (ascii_is_lower(c)) {
            return true;
        }
    }
    return false;
}

constexpr bool ascii_contains_upper(chars_view str) noexcept
{
    for (char c : str) {
        if (ascii_is_upper(c)) {
            return true;
        }
    }
    return false;
}

// fast with long string
constexpr struct ascii_to_upper_t {
    using tag_type = UpperTag;
    constexpr inline char operator()(char c) const noexcept
    {
        return 'a' <= c && c <= 'z' ? (c - 'a' + 'A') : c;
    }
} ascii_to_upper;

// fast with long string
constexpr struct ascii_to_lower_t {
    using tag_type = LowerTag;
    constexpr inline char operator()(char c) const noexcept
    {
        return 'A' <= c && c <= 'Z' ? (c - 'A' + 'a') : c;
    }
} ascii_to_lower;

// fast with short string (/!\ cache miss)
constexpr struct ascii_to_upper_with_table_t {
    using tag_type = UpperTag;
    constexpr inline char operator()(char c) const noexcept
    {
        return detail::ascii_to_upper_table[static_cast<unsigned char>(c)];
    }
} ascii_to_upper_with_table;

// fast with short string (/!\ cache miss)
constexpr struct ascii_to_lower_with_table_t {
    using tag_type = LowerTag;
    constexpr inline char operator()(char c) const noexcept
    {
        return detail::ascii_to_lower_table[static_cast<unsigned char>(c)];
    }
} ascii_to_lower_with_table;

template<class String>
struct TaggedStringTraits
{
    constexpr static chars_view av(String const& str) noexcept
    {
        return str;
    }
};

template<std::size_t N>
struct TaggedStringTraits<detail::StringAsArray<N>>
{
    constexpr static chars_view av(detail::StringAsArray<N> const& str) noexcept
    {
        auto& array = detail::StringAsArrayAccess::internal(str);
        return {array.buffer.data(), array.len};
    }
};

template<class T>
struct TagOfConverterTraits
{
    using type = typename T::tag_type;
};


template<class Tag, class String>
struct TaggedString
{
    String str;

    constexpr char const* data() const noexcept
    {
        return chars().data();
    }

    constexpr std::size_t size() const noexcept
    {
        return chars().size();
    }

    constexpr char operator[](std::size_t i) const noexcept
    {
        return chars()[i];
    }

    constexpr chars_view chars() const noexcept
    {
        return TaggedStringTraits<String>::av(str);
    }

    constexpr std::string_view sv() const noexcept
    {
        return chars().template as<std::string_view>();
    }

    constexpr TaggedString<Tag, std::string_view> tagged_sv() const noexcept
    {
        return TaggedString<Tag, std::string_view>{sv()};
    }

    template<class TaggedView = TaggedString<Tag, std::string_view>>
    constexpr operator std::enable_if_t<
        !std::is_same_v<String, std::string_view>,
        TaggedView
    > () const noexcept
    {
        return TaggedString<Tag, std::string_view>{sv()};
    }

    template<class U>
    friend constexpr detail::enable_if_same_tag_t<U, Tag, bool>
    operator==(TaggedString const& a, U const& b) noexcept
    {
        return a.sv() == b.sv();
    }

    template<class U>
    friend constexpr detail::enable_if_same_tag_t<U, Tag, bool>
    operator !=(TaggedString const& a, U const& b) noexcept
    {
        return a.sv() != b.sv();
    }
};

template<class Tag>
using TaggedStringView = TaggedString<Tag, std::string_view>;

using TaggedUpperStringView = TaggedString<UpperTag, std::string_view>;
using TaggedLowerStringView = TaggedString<UpperTag, std::string_view>;

template<class Tag, std::size_t N>
struct TaggedStringArray : TaggedString<Tag, detail::StringAsArray<N>>
{
};

// C++20: merged c_str() / zsv() into TaggedString
template<class Tag, std::size_t N>
struct TaggedZStringArray : TaggedString<Tag, static_string<N>>
{
    constexpr char const* c_str() const noexcept
    {
        return this->str.c_str();
    }

    constexpr zstring_view zsv() const noexcept
    {
        return zstring_view(this->str);
    }
};

template<class Tag, std::size_t N>
struct is_null_terminated<TaggedZStringArray<Tag, N>>
: std::true_type
{};

namespace detail
{
    template<class Tag, std::size_t N>
    struct extract_string_tag<TaggedStringArray<Tag, N>>
    {
        using type = Tag;
    };

    template<class Tag, std::size_t N>
    struct extract_string_tag<TaggedZStringArray<Tag, N>>
    {
        using type = Tag;
    };

    template<class Tag, class String>
    struct extract_string_tag<TaggedString<Tag, String>>
    {
        using type = Tag;
    };

    template<class Transform>
    constexpr inline std::size_t unsafe_ascii_to(char* dest, chars_view src, Transform&& transform) noexcept
    {
        auto* start = dest;
        for (char c : src) {
            *dest++ = transform(c);
        }
        return std::size_t(dest - start);
    }
} // namespace detail

template<std::size_t N, class To,
    class Tag = typename TagOfConverterTraits<std::decay_t<To>>::type>
constexpr TaggedStringArray<Tag, N>
chars_to_tagged_string_array(chars_view str, To&& to)
{
    TaggedStringArray<Tag, N> upper;
    if (str.size() <= N) {
        auto& array = detail::StringAsArrayAccess::internal(upper.str);
        auto* p = array.buffer.data();
        array.len = detail::unsafe_ascii_to(p, str, to);
    }
    return upper;
}

template<std::size_t N, class To,
    class Tag = typename TagOfConverterTraits<std::decay_t<To>>::type>
constexpr TaggedZStringArray<Tag, N>
chars_to_tagged_zstring_array(chars_view str, To&& to)
{
    TaggedZStringArray<Tag, N> upper;
    if (str.size() <= N) {
        upper.str.delayed_build([&](auto& array){
            return detail::unsafe_ascii_to(array.data(), str, to);
        });
    }
    return upper;
}

template<std::size_t N>
constexpr TaggedStringArray<UpperTag, N> ascii_to_limited_upper(chars_view str) noexcept
{
    return chars_to_tagged_string_array<N>(str, ascii_to_upper);
}

template<std::size_t N>
constexpr TaggedZStringArray<UpperTag, N> ascii_to_limited_zupper(chars_view str) noexcept
{
    return chars_to_tagged_zstring_array<N>(str, ascii_to_upper);
}

template<std::size_t N>
constexpr TaggedStringArray<LowerTag, N> ascii_to_limited_lower(chars_view str) noexcept
{
    return chars_to_tagged_string_array<N>(str, ascii_to_lower);
}

template<std::size_t N>
constexpr TaggedZStringArray<LowerTag, N> ascii_to_limited_zlower(chars_view str) noexcept
{
    return chars_to_tagged_zstring_array<N>(str, ascii_to_lower);
}

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wpedantic")
template<class C, C... cs>
constexpr TaggedStringView<UpperTag> operator ""_ascii_upper() noexcept
{
    return {jln::string_c<ascii_to_upper(cs)...>::sv()};
}

template<class C, C... cs>
constexpr TaggedStringView<LowerTag> operator ""_ascii_lower() noexcept
{
    return {jln::string_c<ascii_to_lower(cs)...>::sv()};
}
REDEMPTION_DIAGNOSTIC_POP()

namespace detail
{
    template<class Tag, class To>
    inline bool tagged_string_equal_with_transform(
        chars_view sv, TaggedStringView<Tag> tagged_str, To const& to)
    {
        static_assert(std::is_same_v<typename TagOfConverterTraits<To>::type, Tag>);

        if (sv.size() != tagged_str.sv().size()) {
            return false;
        }

        for (std::size_t i = 0; i < tagged_str.sv().size(); ++i) {
            if (tagged_str.sv()[i] != to(sv[i])) {
                return false;
            }
        }
        return true;
    }
} // namespace detail

template<class To = ascii_to_upper_t>
inline bool insensitive_eq(chars_view sv, TaggedStringView<UpperTag> upper, To&& to = To())
{
    return detail::tagged_string_equal_with_transform(sv, upper, to);
}

template<class To = ascii_to_lower_t>
inline bool insensitive_eq(chars_view sv, TaggedStringView<LowerTag> lower, To&& to = To())
{
    return detail::tagged_string_equal_with_transform(sv, lower, to);
}

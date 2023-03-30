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

#include "utils/sugar/array_view.hpp"
#include "utils/traits/is_null_terminated.hpp"

#include <type_traits>
#include <charconv>
#include <limits>


template<class Int>
struct chars_to_int_result
{
    std::errc ec;
    Int val;
    char const* ptr;

    constexpr friend bool operator == (chars_to_int_result const& x, chars_to_int_result const& y) noexcept
    {
        return x.ec == y.ec && x.val == y.val && x.ptr == y.ptr;
    }
};

template<class Int>
struct parsed_chars_to_int_result
{
    // unspecified when has_value = false
    Int value;
    bool has_value;

    explicit constexpr operator bool() const noexcept
    {
        return has_value;
    }

    constexpr friend bool operator == (parsed_chars_to_int_result const& x, parsed_chars_to_int_result const& y) noexcept
    {
        return x.has_value == y.has_value && (!x.has_value || x.value == y.value);
    }
};

template<class AvOrCharp>
struct unchecked_decimal_chars_to_int_converter
{
    template<class Int>
    constexpr operator Int () const && noexcept;

    AvOrCharp av_or_charp;
};

template<class AvOrCharp>
struct unchecked_hexadecimal_chars_to_int_converter
{
    template<class UInt>
    constexpr operator UInt () const && noexcept;

    AvOrCharp av_or_charp;
};


// from decimal chars
//@{
constexpr bool is_decimal_char(char c) noexcept
{
    return '0' <= c && c <= '9';
}

constexpr bool is_decimal_char(unsigned char c) noexcept
{
    return '0' <= c && c <= '9';
}

constexpr int unchecked_digit_char_to_int(char c) noexcept
{
    return c - '0';
}

constexpr int unchecked_digit_char_to_int(unsigned char c) noexcept
{
    return c - '0';
}

/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
/// @{
template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(char const* s) noexcept;

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(char const* s, Int& value) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr chars_to_int_result<Int> decimal_chars_to_int(View&& av) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr chars_to_int_result<Int> decimal_chars_to_int(View&& av, Int& value) noexcept;

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(chars_view av) noexcept;

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(chars_view av, Int& value) noexcept;
/// @}

/// Same as std::from_chars()
/// @{
template<class Int>
constexpr std::from_chars_result from_decimal_chars(char const* s, Int& value) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr std::from_chars_result from_decimal_chars(View&& av, Int& value) noexcept;

template<class Int>
constexpr std::from_chars_result from_decimal_chars(chars_view av, Int& value) noexcept;
/// @}

/// Converts a string to a number.
/// If \c s contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
/// @{
template<class Int>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(char const* s) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(View&& av) noexcept;

template<class Int>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(chars_view av) noexcept;
/// @}

/// Converts a string to a number.
/// If \c av contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
/// @{
template<class Int>
constexpr Int parse_decimal_chars_or(char const* s, Int default_value) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr Int parse_decimal_chars_or(View&& av, Int default_value) noexcept;

template<class Int>
constexpr Int parse_decimal_chars_or(chars_view av, Int default_value) noexcept;
/// @}

/// fast atoi()
/// @{
unchecked_decimal_chars_to_int_converter<char const*>
constexpr unchecked_decimal_chars_to_int(char const* s) noexcept
{
    return {s};
}

unchecked_decimal_chars_to_int_converter<chars_view>
constexpr unchecked_decimal_chars_to_int(chars_view av) noexcept
{
    return {av};
}

template<class View, class = decltype(chars_view(std::declval<View>()))>
constexpr auto unchecked_decimal_chars_to_int(View&& av) noexcept;
/// @}
//@}


// from hexadecimal chars (without 0x/0X prefix)
//@{
constexpr bool is_hexadecimal_char(unsigned char c) noexcept;

constexpr bool is_hexadecimal_char(char c) noexcept
{
    return is_hexadecimal_char(static_cast<unsigned char>(c));
}

constexpr unsigned char unchecked_hexadecimal_char_to_int(unsigned char c) noexcept;

constexpr unsigned char unchecked_hexadecimal_char_to_int(char c) noexcept
{
    return unchecked_hexadecimal_char_to_int(static_cast<unsigned char>(c));
}

/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
/// @{
template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s) noexcept;

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s, UInt& value) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av, UInt& value) noexcept;

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av) noexcept;

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av, UInt& value) noexcept;
/// @}

/// Same as std::from_chars()
/// @{
template<class UInt>
constexpr std::from_chars_result from_hexadecimal_chars(char const* s, UInt& value) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr std::from_chars_result from_hexadecimal_chars(View&& av, UInt& value) noexcept;

template<class UInt>
constexpr std::from_chars_result from_hexadecimal_chars(chars_view av, UInt& value) noexcept;
/// @}

/// Converts a string to a number.
/// If \c s contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
/// @{
template<class UInt>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(char const* s) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(View&& av) noexcept;

template<class UInt>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(chars_view av) noexcept;
/// @}

/// Converts a string to a number.
/// If \c av contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
/// @{
template<class UInt>
constexpr UInt parse_hexadecimal_chars_or(char const* s, UInt default_value) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr UInt parse_hexadecimal_chars_or(View&& av, UInt default_value) noexcept;

template<class UInt>
constexpr UInt parse_hexadecimal_chars_or(chars_view av, UInt default_value) noexcept;
/// @}


// remove 0x / 0x
//@{
constexpr char const* remove_hexadecimal_prefix(char const* s) noexcept;

constexpr chars_view remove_hexadecimal_prefix(chars_view av) noexcept;

template<class View, class = decltype(chars_view(std::declval<View>()))>
constexpr auto remove_hexadecimal_prefix(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return remove_hexadecimal_prefix(av.c_str());
    }
    else {
        return remove_hexadecimal_prefix(chars_view(av));
    }
}
//@}


/// fast atoi() for hexadecimal string
/// @{
unchecked_hexadecimal_chars_to_int_converter<char const*>
constexpr unchecked_hexadecimal_chars_without_prefix_to_int(char const* s) noexcept
{
    return {s};
}

unchecked_hexadecimal_chars_to_int_converter<chars_view>
constexpr unchecked_hexadecimal_chars_without_prefix_to_int(chars_view av) noexcept
{
    return {av};
}

template<class View, class = decltype(chars_view(std::declval<View>()))>
constexpr auto unchecked_hexadecimal_chars_without_prefix_to_int(View&& av) noexcept;

unchecked_hexadecimal_chars_to_int_converter<char const*>
constexpr unchecked_hexadecimal_chars_with_prefix_to_int(char const* s) noexcept
{
    return unchecked_hexadecimal_chars_without_prefix_to_int(remove_hexadecimal_prefix(s));
}

unchecked_hexadecimal_chars_to_int_converter<chars_view>
constexpr unchecked_hexadecimal_chars_with_prefix_to_int(chars_view av) noexcept
{
    return unchecked_hexadecimal_chars_without_prefix_to_int(remove_hexadecimal_prefix(av));
}

template<class View, class = decltype(chars_view(std::declval<View>()))>
constexpr auto unchecked_hexadecimal_chars_with_prefix_to_int(View&& av) noexcept
{
    return unchecked_hexadecimal_chars_without_prefix_to_int(remove_hexadecimal_prefix(av));
}
/// @}
//@}


/// Read a hexadecimal or a decimal number.
/// result.ptr points at the first character not matching the pattern.
/// Pattern: [ \t\n]* -? ( 0x[a-fA-F0-9]+ | [0-9]+ )
///@{
template<class Int>
constexpr chars_to_int_result<Int> string_to_int(char const* s);

template<class Int>
constexpr chars_to_int_result<Int> string_to_int(chars_view av);

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
constexpr chars_to_int_result<Int> string_to_int(View&& av);
///@}



//
// IMPLEMENTATION
//

namespace detail
{

template<class Int, bool = std::is_enum_v<Int>>
struct chars_to_int_traits;

template<class Int>
struct chars_to_int_traits<Int, false>
{
    using integral_type = Int;
};

template<class Int>
struct chars_to_int_traits<Int, true>
{
    using integral_type = std::underlying_type_t<Int>;
};

struct unchecked_char_iterator
{
    friend constexpr bool operator!=(char const* /*s*/, unchecked_char_iterator const& /*self*/) noexcept
    {
        return true;
    }

    friend constexpr std::ptrdiff_t operator-(unchecked_char_iterator const& /*self*/, char const* /*s*/) noexcept
    {
        return std::numeric_limits<std::ptrdiff_t>::max();
    }
};

template<class Int, class EndIterator>
constexpr chars_to_int_result<Int> decimal_chars_to_int_impl(char const* first, EndIterator last) noexcept
{
    char const* start = first;

    int sign = 1;
    if constexpr (std::is_signed_v<Int>) {
        if (first != last && *first == '-') {
            sign = -1;
            ++first;
        }
    }

    // '+' allows to promote char/short to int
    auto n = +Int();
    constexpr auto risky_value = +std::numeric_limits<Int>::max() / 10;

    if (first != last && is_decimal_char(*first)) {
        do {
            auto c = static_cast<unsigned char>(*first - '0');
            n = (n * 10) + c;
            ++first;
        } while (n < risky_value && first != last && is_decimal_char(*first));
    }
    else {
        return {std::errc::invalid_argument, {}, start};
    }

    // fast-path (noticeable only with clang)
    if (n < risky_value) {
        if constexpr (std::is_signed_v<Int>) {
            return {std::errc(), Int(n * sign), first};
        }
        else {
            return {std::errc(), Int(n), first};
        }
    }

    if (first != last && is_decimal_char(*first)) {
        using Int2 = std::conditional_t<std::is_signed_v<decltype(n)>, int, unsigned>;
        Int2 base = 10;
        Int2 c = Int2(*first - '0');
        Int2 max_last_digit = std::numeric_limits<Int>::max() % 10;

        if constexpr (std::is_signed_v<Int>) {
            static_assert(std::numeric_limits<Int>::max() % 10 + 1 == -(std::numeric_limits<Int>::min() % 10));
            max_last_digit += (sign == -1);
        }

        if (n > risky_value || c > max_last_digit) {
            return {std::errc::result_out_of_range, {}, first};
        }

        ++first;
        if (first != last && is_decimal_char(*first)) {
            return {std::errc::result_out_of_range, {}, first};
        }

        if constexpr (std::is_signed_v<Int>) {
            base *= sign;
            c *= sign;
        }

        return {std::errc(), Int(n * base + c), first};
    }

    if constexpr (std::is_signed_v<Int>) {
        return {std::errc(), Int(n * sign), first};
    }
    else {
        return {std::errc(), Int(n), first};
    }
}

template<class EndIt>
constexpr char const* eat_decimal_char(char const* p, EndIt last) noexcept
{
    while (p != last && is_decimal_char(*p)) {
        ++p;
    }
    return p;
}

template<class Int, class EndIt>
constexpr std::from_chars_result from_decimal_chars_impl(char const* first, EndIt last, Int& i) noexcept
{
    auto r = decimal_chars_to_int_impl<Int>(first, last);
    if (r.ec == std::errc()) {
        i = r.val;
        return std::from_chars_result{r.ptr, r.ec};
    }
    if (r.ec == std::errc::result_out_of_range) {
        return std::from_chars_result{eat_decimal_char(r.ptr, last), r.ec};
    }
    return std::from_chars_result{first, std::errc::invalid_argument};
}

// convert ['0', '9'] ['A', 'F'] ['a', 'f'] to [0, 15], everything else to 255
inline constexpr unsigned char hex_digit_table[] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13,
    14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14,
    15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

constexpr unsigned char hexadecimal_char_to_int(unsigned char c) noexcept
{
    return hex_digit_table[c];
}

constexpr unsigned char hexadecimal_char_to_int(char c) noexcept
{
    return hex_digit_table[static_cast<unsigned char>(c)];
}

template<class UInt, class EndIterator>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int_impl(char const* first, EndIterator last) noexcept
{
    static_assert(std::is_unsigned_v<UInt>);

    // '+' allows to promote char/short to int
    auto x = +UInt();
    constexpr auto risky_value = +std::numeric_limits<UInt>::max() / 16;

    unsigned char c = 0;
    if (first != last && 0xff != (c = hexadecimal_char_to_int(*first))) {
        do {
            x = (x * 16) + c;
            ++first;
        } while (x <= risky_value && first != last && 0xff != (c = hexadecimal_char_to_int(*first)));
    }
    else {
        return {std::errc::invalid_argument, {}, first};
    }

    // fast-path (noticeable only with clang)
    if (x <= risky_value) {
        return {std::errc(), UInt(x), first};
    }

    if (first != last && 0xff != hexadecimal_char_to_int(*first)) {
        return {std::errc::result_out_of_range, {}, first};
    }

    return {std::errc(), UInt(x), first};
}

template<class EndIt>
constexpr char const* eat_hexadecimal_char(char const* p, EndIt last) noexcept
{
    while (p != last && 0xff != hexadecimal_char_to_int(*p)) {
        ++p;
    }
    return p;
}

template<class Int, class EndIt>
constexpr std::from_chars_result from_hexadecimal_chars_impl(char const* first, EndIt last, Int& i) noexcept
{
    auto r = hexadecimal_chars_to_int_impl<Int>(first, last);
    if (r.ec == std::errc()) {
        i = r.val;
        return std::from_chars_result{r.ptr, r.ec};
    }
    if (r.ec == std::errc::result_out_of_range) {
        return std::from_chars_result{eat_hexadecimal_char(r.ptr, last), r.ec};
    }
    return std::from_chars_result{first, std::errc::invalid_argument};
}

template<class Int, class U>
constexpr chars_to_int_result<Int> convert_char_to_int_result(chars_to_int_result<U> r) noexcept
{
    return {r.ec, Int(r.val), r.ptr};
}

constexpr bool is_hexadecimal_prefix(char const* s) noexcept
{
    return s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
}

} // namespace detail

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::convert_char_to_int_result<Int>(
        detail::decimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator()));
}

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(char const* s, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    if (r.ec == std::errc()) {
        value = Int(r.val);
    }
    return detail::convert_char_to_int_result<Int>(r);
}

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::convert_char_to_int_result<Int>(
        detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end()));
}

template<class Int>
constexpr chars_to_int_result<Int> decimal_chars_to_int(chars_view av, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc()) {
        value = r.val;
    }
    return detail::convert_char_to_int_result<Int>(r);
}

template<class Int, class View, class>
constexpr chars_to_int_result<Int> decimal_chars_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return decimal_chars_to_int<Int>(av.c_str());
    }
    else {
        return decimal_chars_to_int<Int>(chars_view(av));
    }
}

template<class Int, class View, class>
constexpr chars_to_int_result<Int> decimal_chars_to_int(View&& av, Int& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return decimal_chars_to_int<Int>(av.c_str(), value);
    }
    else {
        return decimal_chars_to_int<Int>(chars_view(av), value);
    }
}

template<class Int>
constexpr std::from_chars_result from_decimal_chars(char const* s, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::from_decimal_chars_impl<integral_type>(s, detail::unchecked_char_iterator(), value);
}

template<class Int>
constexpr std::from_chars_result from_decimal_chars(chars_view av, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::from_decimal_chars_impl<integral_type>(av.begin(), av.end(), value);
}

template<class Int, class View, class>
constexpr std::from_chars_result from_decimal_chars(View&& av, Int& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return from_decimal_chars<Int>(av.c_str(), value);
    }
    else {
        return from_decimal_chars<Int>(chars_view(av), value);
    }
}

template<class Int>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    if (r.ec == std::errc() && !*r.ptr) {
        return {Int(r.val), true};
    }
    return {};
}

template<class Int>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc() && r.ptr == av.end()) {
        return {Int(r.val), true};
    }
    return {};
}

template<class Int, class View, class>
constexpr parsed_chars_to_int_result<Int> parse_decimal_chars(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_decimal_chars<Int>(av.c_str());
    }
    else {
        return parse_decimal_chars<Int>(chars_view(av));
    }
}

template<class Int>
constexpr Int parse_decimal_chars_or(char const* s, Int default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    return (r.ec == std::errc() && !*r.ptr) ? Int(r.val) : default_value;
}

template<class Int>
constexpr Int parse_decimal_chars_or(chars_view av, Int default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    chars_to_int_result<Int> r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    return (r.ec == std::errc() && r.ptr == av.end()) ? Int(r.val) : default_value;
}

template<class Int, class View, class>
constexpr Int parse_decimal_chars_or(View&& av, Int default_value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_decimal_chars_or(av.c_str(), default_value);
    }
    else {
        return parse_decimal_chars_or(chars_view(av), default_value);
    }
}

template<class View, class>
constexpr auto unchecked_decimal_chars_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return unchecked_decimal_chars_to_int(av.c_str());
    }
    else {
        return unchecked_decimal_chars_to_int(chars_view(av));
    }
}

template<class AvOrCharp>
template<class Int>
constexpr unchecked_decimal_chars_to_int_converter<AvOrCharp>::operator Int () const && noexcept
{
    return decimal_chars_to_int<Int>(av_or_charp).val;
}


constexpr bool is_hexadecimal_char(unsigned char c) noexcept
{
    return detail::hexadecimal_char_to_int(c) != 255;
}

constexpr unsigned char unchecked_hexadecimal_char_to_int(unsigned char c) noexcept
{
    return detail::hexadecimal_char_to_int(c);
}

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::convert_char_to_int_result<UInt>(
        detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator()));
}

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    if (r.ec == std::errc()) {
        value = UInt(r.val);
    }
    return detail::convert_char_to_int_result<UInt>(r);
}

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::convert_char_to_int_result<UInt>(
        detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end()));
}

template<class UInt>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc()) {
        value = UInt(r.val);
    }
    return detail::convert_char_to_int_result<UInt>(r);
}

template<class UInt, class View, class>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return hexadecimal_chars_to_int<UInt>(av.c_str());
    }
    else {
        return hexadecimal_chars_to_int<UInt>(chars_view(av));
    }
}

template<class UInt, class View, class>
constexpr chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av, UInt& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return hexadecimal_chars_to_int<UInt>(av.c_str(), value);
    }
    else {
        return hexadecimal_chars_to_int<UInt>(chars_view(av), value);
    }
}

template<class UInt>
constexpr std::from_chars_result from_hexadecimal_chars(char const* s, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::from_hexadecimal_chars_impl<integral_type>(s, detail::unchecked_char_iterator(), value);
}

template<class UInt>
constexpr std::from_chars_result from_hexadecimal_chars(chars_view av, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::from_hexadecimal_chars_impl<integral_type>(av.begin(), av.end(), value);
}

template<class UInt, class View, class>
constexpr std::from_chars_result from_hexadecimal_chars(View&& av, UInt& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return from_hexadecimal_chars(av.c_str(), value);
    }
    else {
        return from_hexadecimal_chars(chars_view(av), value);
    }
}

template<class UInt>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    if (r.ec == std::errc() && !*r.ptr) {
        return {UInt(r.val), true};
    }
    return {};
}

template<class UInt>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc() && r.ptr == av.end()) {
        return {UInt(r.val), true};
    }
    return {};
}

template<class UInt, class View, class>
constexpr parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_hexadecimal_chars<UInt>(av.c_str());
    }
    else {
        return parse_hexadecimal_chars<UInt>(chars_view(av));
    }
}

template<class UInt>
constexpr UInt parse_hexadecimal_chars_or(char const* s, UInt default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::unchecked_char_iterator());
    return (r.ec == std::errc() && !*r.ptr) ? UInt(r.val) : default_value;
}

template<class UInt>
constexpr UInt parse_hexadecimal_chars_or(chars_view av, UInt default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    chars_to_int_result<UInt> r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    return (r.ec == std::errc() && r.ptr == av.end()) ? UInt(r.val) : default_value;
}

template<class UInt, class View, class>
constexpr UInt parse_hexadecimal_chars_or(View&& av, UInt default_value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_hexadecimal_chars_or(av.c_str(), default_value);
    }
    else {
        return parse_hexadecimal_chars_or(chars_view(av), default_value);
    }
}

constexpr char const* remove_hexadecimal_prefix(char const* s) noexcept
{
    return detail::is_hexadecimal_prefix(s) ? s + 2 : s;
}

constexpr chars_view remove_hexadecimal_prefix(chars_view av) noexcept
{
    return av.size() > 2 && detail::is_hexadecimal_prefix(av.data()) ? av.drop_front(2) : av;
}

template<class View, class>
constexpr auto unchecked_hexadecimal_chars_without_prefix_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return unchecked_hexadecimal_chars_without_prefix_to_int(av.c_str());
    }
    else {
        return unchecked_hexadecimal_chars_without_prefix_to_int(chars_view(av));
    }
}

template<class AvOrCharp>
template<class UInt>
constexpr unchecked_hexadecimal_chars_to_int_converter<AvOrCharp>::operator UInt () const && noexcept
{
    return hexadecimal_chars_to_int<UInt>(av_or_charp).val;
}


namespace detail
{
template<class Int, class EndIt>
constexpr chars_to_int_result<Int> string_to_int_impl(char const* s, EndIt last)
{
    using Result = chars_to_int_result<Int>;
    using UInt = std::make_unsigned_t<Int>;

    // TODO eat_spaces()
    while (s != last && (*s == ' ' || *s == '\t' || *s == '\n')) {
        ++s;
    }

    if constexpr (std::is_signed_v<Int>) {
        // -0x[0-9a-fA-F]
        if (last - s >= 4 && s[0] == '-'
         && is_hexadecimal_prefix(s + 1)
         && 0xff != hexadecimal_char_to_int(s[3])
        ) {
            auto r = hexadecimal_chars_to_int_impl<UInt>(s + 3, last);
            if (r.ec == std::errc()) {
                constexpr auto min = std::numeric_limits<Int>::min();
                constexpr auto abs = UInt(-(min / 16)) * 16 + UInt(-(min % 16));
                if (r.val <= abs) {
                    auto val = -(r.val / 16) * 16;
                    val -= r.val % 16;
                    return Result{std::errc(), Int(val), r.ptr};
                }
                return Result{std::errc::result_out_of_range, {}, r.ptr};
            }
            r.ptr = eat_hexadecimal_char(r.ptr, last);
            return convert_char_to_int_result<Int>(r);
        }

        // 0x[0-9a-fA-F]
        if (last - s >= 3
         && is_hexadecimal_prefix(s)
         && 0xff != hexadecimal_char_to_int(s[2])
        ) {
            auto r = hexadecimal_chars_to_int_impl<UInt>(s + 2, last);
            constexpr auto max = std::numeric_limits<Int>::max();
            if (r.ec == std::errc()) {
                if (r.val <= max) {
                    return convert_char_to_int_result<Int>(r);
                }
                return Result{std::errc::result_out_of_range, {}, r.ptr};
            }
            r.ptr = eat_hexadecimal_char(r.ptr, last);
            return convert_char_to_int_result<Int>(r);
        }
    }
    // 0x[0-9a-fA-F]
    else if (last - s >= 3
          && is_hexadecimal_prefix(s)
          && 0xff != hexadecimal_char_to_int(s[2])
    ) {
        auto r = hexadecimal_chars_to_int_impl<Int>(s + 2, last);
        r.ptr = eat_hexadecimal_char(r.ptr, last);
        return r;
    }

    auto r = decimal_chars_to_int_impl<Int>(s, last);
    r.ptr = eat_decimal_char(r.ptr, last);
    return r;
}
} // namespace detail

template<class Int>
constexpr chars_to_int_result<Int> string_to_int(char const* s)
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::convert_char_to_int_result<Int>(
        detail::string_to_int_impl<integral_type>(s, detail::unchecked_char_iterator()));
}

template<class Int>
constexpr chars_to_int_result<Int> string_to_int(chars_view av)
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::convert_char_to_int_result<Int>(
        detail::string_to_int_impl<integral_type>(av.begin(), av.end()));
}

template<class Int, class View, class>
constexpr chars_to_int_result<Int> string_to_int(View&& av)
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return string_to_int<Int>(av.c_str());
    }
    else {
        return string_to_int<Int>(chars_view(av));
    }
}

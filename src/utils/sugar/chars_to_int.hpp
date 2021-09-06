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

    friend bool operator == (chars_to_int_result const& x, chars_to_int_result const& y) noexcept
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

    explicit operator bool() const noexcept
    {
        return has_value;
    }

    friend bool operator == (parsed_chars_to_int_result const& x, parsed_chars_to_int_result const& y) noexcept
    {
        return x.has_value == y.has_value && (!x.has_value || x.value == y.value);
    }
};

// from decimal chars
//@{
/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
template<class Int>
chars_to_int_result<Int> decimal_chars_to_int(char const* s) noexcept;

template<class Int>
chars_to_int_result<Int> decimal_chars_to_int(char const* s, Int& value) noexcept;

/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
chars_to_int_result<Int> decimal_chars_to_int(View&& av) noexcept;

template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
chars_to_int_result<Int> decimal_chars_to_int(View&& av, Int& value) noexcept;

template<class Int>
chars_to_int_result<Int> decimal_chars_to_int(chars_view av) noexcept;

template<class Int>
chars_to_int_result<Int> decimal_chars_to_int(chars_view av, Int& value) noexcept;

/// Same as std::from_chars()
template<class Int>
std::from_chars_result from_decimal_chars(char const* s, Int& value) noexcept;

/// Same as std::from_chars()
template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
std::from_chars_result from_decimal_chars(View&& av, Int& value) noexcept;

template<class Int>
std::from_chars_result from_decimal_chars(chars_view av, Int& value) noexcept;

/// Converts a string to a number.
/// If \c s contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
template<class Int>
parsed_chars_to_int_result<Int> parse_decimal_chars(char const* s) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
parsed_chars_to_int_result<Int> parse_decimal_chars(View&& av) noexcept;

template<class Int>
parsed_chars_to_int_result<Int> parse_decimal_chars(chars_view av) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
template<class Int>
Int parse_decimal_chars_or(char const* s, Int default_value) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than decimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
template<class Int, class View, class = decltype(chars_view(std::declval<View>()))>
Int parse_decimal_chars_or(View&& av, Int default_value) noexcept;

template<class Int>
Int parse_decimal_chars_or(chars_view av, Int default_value) noexcept;
//@}

// from hexadecimal chars
//@{
/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
template<class UInt>
chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s) noexcept;

template<class UInt>
chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s, UInt& value) noexcept;

/// Same as std::from_chars(), but chars_to_int_result::ptr points at the last
/// character parsed. Which means that unlike std::from_chars(), a
/// std::errc::result_out_of_range error does not consume the whole pattern.
template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av) noexcept;

template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av, UInt& value) noexcept;

template<class UInt>
chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av) noexcept;

template<class UInt>
chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av, UInt& value) noexcept;

/// Same as std::from_chars()
template<class UInt>
std::from_chars_result from_hexadecimal_chars(char const* s, UInt& value) noexcept;

/// Same as std::from_chars()
template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
std::from_chars_result from_hexadecimal_chars(View&& av, UInt& value) noexcept;

template<class UInt>
std::from_chars_result from_hexadecimal_chars(chars_view av, UInt& value) noexcept;

/// Converts a string to a number.
/// If \c s contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
template<class UInt>
parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(char const* s) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then parsed_chars_to_int_result::has_value = false and parsed_chars_to_int_result::value is unspecified.
template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(View&& av) noexcept;

template<class UInt>
parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(chars_view av) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
template<class UInt>
UInt parse_hexadecimal_chars_or(char const* s, UInt default_value) noexcept;

/// Converts a string to a number.
/// If \c av contains anything other than hexadecimal values or if the parsed value is not in the range represented by the value type, then default_value is returned.
template<class UInt, class View, class = decltype(chars_view(std::declval<View>()))>
UInt parse_hexadecimal_chars_or(View&& av, UInt default_value) noexcept;

template<class UInt>
UInt parse_hexadecimal_chars_or(chars_view av, UInt default_value) noexcept;
//@}


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

struct uncheck_char_iterator
{
    friend constexpr bool operator!=(char const* /*s*/, uncheck_char_iterator const& /*self*/) noexcept
    {
        return true;
    }
};

template<class Int, class EndIterator>
chars_to_int_result<Int> decimal_chars_to_int_impl(char const* first, EndIterator last) noexcept
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
    auto x = +Int();
    constexpr auto max_safe = +std::numeric_limits<Int>::max() / 10;

    if (first != last && '0' <= *first && *first <= '9') {
        do {
            auto c = static_cast<unsigned char>(*first - '0');
            x = (x * 10) + c;
            ++first;
        } while (x < max_safe && first != last && '0' <= *first && *first <= '9');
    }
    else {
        return {std::errc::invalid_argument, {}, start};
    }

    // fast-path (noticeable only with clang)
    if (x < max_safe) {
        if constexpr (std::is_signed_v<Int>) {
            return {std::errc(), Int(x * sign), first};
        }
        else {
            return {std::errc(), Int(x), first};
        }
    }

    if (first != last && '0' <= *first && *first <= '9') {
        Int result;
        int base = 10;
        int c = *first - '0';

        if constexpr (std::is_signed_v<Int>) {
            base *= sign;
            c *= sign;
        }

        if (__builtin_mul_overflow(x, base, &result)
         || __builtin_add_overflow(result, c, &result)
        ) {
            return {std::errc::result_out_of_range, {}, first};
        }

        ++first;
        if (first != last && '0' <= *first && *first <= '9') {
            return {std::errc::result_out_of_range, {}, first};
        }

        return {std::errc(), result, first};
    }

    if constexpr (std::is_signed_v<Int>) {
        return {std::errc(), Int(x * sign), first};
    }
    else {
        return {std::errc(), Int(x), first};
    }
}

template<class Int, class EndIt>
inline std::from_chars_result from_decimal_chars_impl(char const* first, EndIt last, Int& i) noexcept
{
    auto r = decimal_chars_to_int_impl<Int>(first, last);
    if (r.ec == std::errc()) {
        i = r.val;
        return std::from_chars_result{r.ptr, r.ec};
    }
    if (r.ec == std::errc::result_out_of_range) {
        auto p = r.ptr;
        while (p != last && '0' <= *p && *p <= '9') {
            ++p;
        }
        return std::from_chars_result{p, r.ec};
    }
    return std::from_chars_result{first, std::errc::invalid_argument};
}

#ifdef __clang__
inline unsigned char hexadecimal_char_to_int(char c) noexcept
{
    if ('0' <= c && c <= '9') return static_cast<unsigned char>(c - '0');
    if ('a' <= c && c <= 'f') return static_cast<unsigned char>(c - 'a' + 10);
    if ('A' <= c && c <= 'F') return static_cast<unsigned char>(c - 'A' + 10);
    return 0xff;
}
#else
inline unsigned char hexadecimal_char_to_int(char c) noexcept
{
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default: return 0xff;
    }
}
#endif

template<class UInt, class EndIterator>
chars_to_int_result<UInt> hexadecimal_chars_to_int_impl(char const* first, EndIterator last) noexcept
{
    static_assert(std::is_unsigned_v<UInt>);

    // '+' allows to promote char/short to int
    auto x = +UInt();
    constexpr auto max_safe = +std::numeric_limits<UInt>::max() / 16;

    unsigned char c;
    if (first != last && 0xff != (c = hexadecimal_char_to_int(*first))) {
        do {
            x = (x * 16) + c;
            ++first;
        } while (x <= max_safe && first != last && 0xff != (c = hexadecimal_char_to_int(*first)));
    }
    else {
        return {std::errc::invalid_argument, {}, first};
    }

    // fast-path (noticeable only with clang)
    if (x <= max_safe) {
        return {std::errc(), UInt(x), first};
    }

    if (first != last && 0xff != hexadecimal_char_to_int(*first)) {
        return {std::errc::result_out_of_range, {}, first};
    }

    return {std::errc(), UInt(x), first};
}

template<class Int, class EndIt>
inline std::from_chars_result from_hexadecimal_chars_impl(char const* first, EndIt last, Int& i) noexcept
{
    auto r = hexadecimal_chars_to_int_impl<Int>(first, last);
    if (r.ec == std::errc()) {
        i = r.val;
        return std::from_chars_result{r.ptr, r.ec};
    }
    if (r.ec == std::errc::result_out_of_range) {
        auto p = r.ptr;
        while (p != last && 0xff != hexadecimal_char_to_int(*p)) {
            ++p;
        }
        return std::from_chars_result{p, r.ec};
    }
    return std::from_chars_result{first, std::errc::invalid_argument};
}

} // namespace detail


template<class Int>
inline chars_to_int_result<Int> decimal_chars_to_int(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::decimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
}

template<class Int>
inline chars_to_int_result<Int> decimal_chars_to_int(char const* s, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    if (r.ec == std::errc()) {
        value = r.val;
    }
    return r;
}

template<class Int>
inline chars_to_int_result<Int> decimal_chars_to_int(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
}

template<class Int>
inline chars_to_int_result<Int> decimal_chars_to_int(chars_view av, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc()) {
        value = r.val;
    }
    return r;
}

template<class Int, class View, class>
inline chars_to_int_result<Int> decimal_chars_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return decimal_chars_to_int<Int>(av.c_str());
    }
    else {
        return decimal_chars_to_int<Int>(chars_view(av));
    }
}

template<class Int, class View, class>
inline chars_to_int_result<Int> decimal_chars_to_int(View&& av, Int& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return decimal_chars_to_int<Int>(av.c_str(), value);
    }
    else {
        return decimal_chars_to_int<Int>(chars_view(av), value);
    }
}

template<class Int>
inline std::from_chars_result from_decimal_chars(char const* s, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::from_decimal_chars_impl<integral_type>(s, detail::uncheck_char_iterator(), value);
}

template<class Int>
inline std::from_chars_result from_decimal_chars(chars_view av, Int& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    return detail::from_decimal_chars_impl<integral_type>(av.begin(), av.end(), value);
}

template<class Int, class View, class>
inline std::from_chars_result from_decimal_chars(View&& av, Int& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return from_decimal_chars<Int>(av.c_str(), value);
    }
    else {
        return from_decimal_chars<Int>(chars_view(av), value);
    }
}

template<class Int>
inline parsed_chars_to_int_result<Int> parse_decimal_chars(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    parsed_chars_to_int_result<Int> result;
    if (r.ec == std::errc() && !*r.ptr) {
        result.value = r.val;
        result.has_value = true;
    }
    else {
        result.has_value = false;
    }
    return result;
}

template<class Int>
inline parsed_chars_to_int_result<Int> parse_decimal_chars(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    parsed_chars_to_int_result<Int> result;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc() && r.ptr == av.end()) {
        result.value = r.val;
        result.has_value = true;
    }
    else {
        result.has_value = false;
    }
    return result;
}

template<class Int, class View, class>
inline parsed_chars_to_int_result<Int> parse_decimal_chars(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_decimal_chars<Int>(av.c_str());
    }
    else {
        return parse_decimal_chars<Int>(chars_view(av));
    }
}

template<class Int>
inline Int parse_decimal_chars_or(char const* s, Int default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    auto r = detail::decimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    return (r.ec == std::errc() && !*r.ptr) ? r.val : default_value;
}

template<class Int>
inline Int parse_decimal_chars_or(chars_view av, Int default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<Int>::integral_type;
    chars_to_int_result<Int> r = detail::decimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    return (r.ec == std::errc() && r.ptr == av.end()) ? r.val : default_value;
}

template<class Int, class View, class>
inline Int parse_decimal_chars_or(View&& av, Int default_value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_decimal_chars_or(av.c_str(), default_value);
    }
    else {
        return parse_decimal_chars_or(chars_view(av), default_value);
    }
}


template<class UInt>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
}

template<class UInt>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(char const* s, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    if (r.ec == std::errc()) {
        value = r.val;
    }
    return r;
}

template<class UInt>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
}

template<class UInt>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(chars_view av, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc()) {
        value = r.val;
    }
    return r;
}

template<class UInt, class View, class>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return hexadecimal_chars_to_int<UInt>(av.c_str());
    }
    else {
        return hexadecimal_chars_to_int<UInt>(chars_view(av));
    }
}

template<class UInt, class View, class>
inline chars_to_int_result<UInt> hexadecimal_chars_to_int(View&& av, UInt& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return hexadecimal_chars_to_int<UInt>(av.c_str(), value);
    }
    else {
        return hexadecimal_chars_to_int<UInt>(chars_view(av), value);
    }
}

template<class UInt>
inline std::from_chars_result from_hexadecimal_chars(char const* s, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::from_hexadecimal_chars_impl<integral_type>(s, detail::uncheck_char_iterator(), value);
}

template<class UInt>
inline std::from_chars_result from_hexadecimal_chars(chars_view av, UInt& value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    return detail::from_hexadecimal_chars_impl<integral_type>(av.begin(), av.end(), value);
}

template<class UInt, class View, class>
inline std::from_chars_result from_hexadecimal_chars(View&& av, UInt& value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return from_hexadecimal_chars(av.c_str(), value);
    }
    else {
        return from_hexadecimal_chars(chars_view(av), value);
    }
}

template<class UInt>
inline parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(char const* s) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    parsed_chars_to_int_result<UInt> result;
    if (r.ec == std::errc() && !*r.ptr) {
        result.value = r.val;
        result.has_value = true;
    }
    else {
        result.has_value = false;
    }
    return result;
}

template<class UInt>
inline parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(chars_view av) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    parsed_chars_to_int_result<UInt> result;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    if (r.ec == std::errc() && r.ptr == av.end()) {
        result.value = r.val;
        result.has_value = true;
    }
    else {
        result.has_value = false;
    }
    return result;
}

template<class UInt, class View, class>
inline parsed_chars_to_int_result<UInt> parse_hexadecimal_chars(View&& av) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_hexadecimal_chars<UInt>(av.c_str());
    }
    else {
        return parse_hexadecimal_chars<UInt>(chars_view(av));
    }
}

template<class UInt>
inline UInt parse_hexadecimal_chars_or(char const* s, UInt default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    auto r = detail::hexadecimal_chars_to_int_impl<integral_type>(s, detail::uncheck_char_iterator());
    return (r.ec == std::errc() && !*r.ptr) ? r.val : default_value;
}

template<class UInt>
inline UInt parse_hexadecimal_chars_or(chars_view av, UInt default_value) noexcept
{
    using integral_type = typename detail::chars_to_int_traits<UInt>::integral_type;
    chars_to_int_result<UInt> r = detail::hexadecimal_chars_to_int_impl<integral_type>(av.begin(), av.end());
    return (r.ec == std::errc() && r.ptr == av.end()) ? r.val : default_value;
}

template<class UInt, class View, class>
inline UInt parse_hexadecimal_chars_or(View&& av, UInt default_value) noexcept
{
    if constexpr (is_null_terminated_v<std::decay_t<View>>) {
        return parse_hexadecimal_chars_or(av.c_str(), default_value);
    }
    else {
        return parse_hexadecimal_chars_or(chars_view(av), default_value);
    }
}

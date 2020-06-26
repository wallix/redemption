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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/zbuffer.hpp"

#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "utils/string_c.hpp"
#include "utils/chex_to_int.hpp"

#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <charconv>

namespace
{

template<class> struct cfg_s_type {};

// assign_zbuf_from_cfg (guarantee with null terminal)
//@{

inline zstring_view assign_zbuf_from_cfg(
    writable_chars_view /*zbuf*/,
    cfg_s_type<::configs::spec_types::directory_path> /*type*/,
    ::configs::spec_types::directory_path const & dir
) {
    return dir.as_string();
}

inline zstring_view assign_zbuf_from_cfg(
    writable_chars_view /*zbuf*/,
    cfg_s_type<std::string> /*type*/,
    std::string const & str
) {
    return str;
}

template<class T>
inline zstring_view assign_zbuf_from_cfg(
    writable_chars_view /*zbuf*/,
    cfg_s_type<::configs::spec_types::list<T>> /*type*/,
    std::string const & str
) {
    return str;
}

template<unsigned n>
inline zstring_view assign_zbuf_from_cfg(
    writable_chars_view /*zbuf*/,
    cfg_s_type<::configs::spec_types::fixed_string> /*type*/,
    char const (&str)[n]
) {
    return zstring_view(zstring_view::is_zero_terminated(), str, strlen(str));
}

inline zstring_view assign_zbuf_from_cfg(
    writable_chars_view /*zbuf*/,
    cfg_s_type<bool> /*type*/,
    bool x
) {
    return x ? "True"_zv : "False"_zv;
}

template<std::size_t N>
zstring_view assign_zbuf_from_cfg(
    writable_chars_view zbuf,
    cfg_s_type<::configs::spec_types::fixed_binary> /*type*/,
    std::array<unsigned char, N> const & arr
) {
    char * p = zbuf.data();
    const char * hex = "0123456789ABCDEF";
    for (int c : arr) {
        *p++ = hex[(c & 0xf0) >> 4];
        *p++ = hex[c & 0xf];
    }
    *p = '\0';
    return zstring_view(zstring_view::is_zero_terminated{}, zbuf.data(), p-zbuf.data());
}

template<class T,
    ::configs::spec_types::underlying_type_for_range_t<T> min,
    ::configs::spec_types::underlying_type_for_range_t<T> max>
zstring_view assign_zbuf_from_cfg(
    writable_chars_view zbuf,
    cfg_s_type<::configs::spec_types::range<T, min, max>> /*type*/,
    T const & rng
) {
    return assign_zbuf_from_cfg(zbuf, cfg_s_type<T>{}, rng);
}

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value, zstring_view>::type
assign_zbuf_from_cfg(writable_chars_view zbuf, cfg_s_type<TInt> /*type*/, TInt const & x)
{
    assert(zbuf.data());

    int sz;
    if constexpr (std::is_signed<TInt>::value) {
        sz = snprintf(zbuf.data(), zbuf.size(), "%lld", static_cast<long long>(x));
    }
    else {
        sz = snprintf(zbuf.data(), zbuf.size(), "%llu", static_cast<unsigned long long>(x));
    }
    return zstring_view(zstring_view::is_zero_terminated{}, zbuf.data(), sz);
}

template<class T, class Ratio>
zstring_view assign_zbuf_from_cfg(
    writable_chars_view zbuf,
    cfg_s_type<std::chrono::duration<T, Ratio>> /*type*/,
    std::chrono::duration<T, Ratio> const & x
) {
    return assign_zbuf_from_cfg(zbuf, cfg_s_type<T>{}, x.count());
}
//@}

//
// parse
//

struct parse_error
{
    constexpr explicit parse_error(char const * err) noexcept : s_err(err) {}

    explicit operator bool () const noexcept { return this->s_err; }
    [[nodiscard]] char const * c_str() const noexcept { return this->s_err; }

private:
    char const * s_err;
};


constexpr parse_error no_parse_error {nullptr};

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<std::string> /*type*/, zstring_view value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse_from_cfg(char (&x)[N], ::configs::spec_type<::configs::spec_types::fixed_string> /*type*/, zstring_view value)
{
    using namespace jln::literals;
    if (value.size() >= N) {
        return parse_error{("out of bounds, limits is "_c + jln::ull_to_string_c_t<N>()).c_str()};
    }
    memcpy(x, value.data(), value.size());
    memset(x + value.size(), 0, N - value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse_from_cfg(
    std::array<unsigned char, N> & key,
    ::configs::spec_type<::configs::spec_types::fixed_binary> /*type*/,
    zstring_view value
) {
    using namespace jln::literals;
    if (value.size() != N*2) {
        return parse_error{(
            "bad length, should be "_c + jln::ull_to_string_c_t<N*2>()
        ).c_str()};
    }

    std::array<unsigned char, N> tmp;
    char const* data = value.begin();
    int err = 0;
    for (std::size_t i = 0; i < N; ++i, data += 2) {
        tmp[i] = (chex_to_int(data[0], err) << 4) | chex_to_int(data[1], err);
    }

    if (err) {
        return parse_error{"bad format, expected hexadecimal value"};
    }

    key = tmp;

    return no_parse_error;
}

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<std::string>> /*type*/, zstring_view value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

inline parse_error parse_from_cfg(
    ::configs::spec_types::directory_path & x,
    ::configs::spec_type<::configs::spec_types::directory_path> /*type*/,
    zstring_view value
) {
    x = std::string(value.data(), value.size());
    return no_parse_error;
}

template<class TInt, TInt min, TInt max>
parse_error make_invalid_range_error()
{
    using namespace jln::literals;

    return parse_error{(
        "invalid range in ["_c
        + jln::integer_to_string_c_t<TInt, min>()
        + ", "_c
        + jln::integer_to_string_c_t<TInt, max>()
        + "]"_c
    ).c_str()};
}

template<bool InList = false, class TInt, TInt min, TInt max>
parse_error parse_integral(
    TInt & x, chars_view value,
    std::integral_constant<TInt, min> /*unused*/,
    std::integral_constant<TInt, max> /*unused*/)
{
    if (REDEMPTION_UNLIKELY(value.empty())) {
        return parse_error{"empty value"};
    }

    TInt tmp;
    int base = 10;
    if (value.size() >= 2 && value[0] == '0' && value[1] == 'x') {
        value = value.drop_front(2);
        base = 16;
    }

    auto r = std::from_chars(value.begin(), value.end(), tmp, base);

    if (r.ec == std::errc() && r.ptr == value.end()) {
        if (tmp < min || max < tmp) {
            return make_invalid_range_error<TInt, min, max>();
        }

        x = tmp;
        return no_parse_error;
    }

    switch (r.ec) {
        case std::errc::value_too_large:
            return make_invalid_range_error<TInt, min, max>();
        default:
            return InList
                ? parse_error{"bad format, expected list of decimal, hexadecimal"
                    " (ex: \"integral[, integral ...]*\")"}
                : parse_error{"bad format, expected decimal, hexadecimal"};
    }
}

template<class T>
using max_integral = std::integral_constant<T, std::numeric_limits<T>::max()>;
template<class T>
using min_integral = std::integral_constant<T, std::numeric_limits<T>::min()>;
template<class T>
using zero_integral = std::integral_constant<T, 0>;

template<class TInt>
std::enable_if_t<
    std::is_integral_v<TInt> && !std::is_same_v<TInt, bool>,
    parse_error>
parse_from_cfg(TInt & x, ::configs::spec_type<TInt> /*type*/, zstring_view value)
{
    return parse_integral(x, value, min_integral<TInt>(), max_integral<TInt>());
}

template<class T,
    ::configs::spec_types::underlying_type_for_range_t<T> min,
    ::configs::spec_types::underlying_type_for_range_t<T> max>
parse_error parse_from_cfg(
    T & x,
    ::configs::spec_type<::configs::spec_types::range<T, min, max>> /*type*/,
    zstring_view value
) {
    using namespace jln::literals;
    using Int = ::configs::spec_types::underlying_type_for_range_t<T>;
    Int y;
    if (auto err = parse_from_cfg(y, ::configs::spec_type<Int>{}, value)) {
        return err;
    }
    if (y < min || max < y) {
        return make_invalid_range_error<Int, min, max>();
    }
    x = T{y};
    return no_parse_error;
}

template<class T, class Ratio>
parse_error parse_from_cfg(
    std::chrono::duration<T, Ratio> & x,
    ::configs::spec_type<std::chrono::duration<T, Ratio>> /*type*/,
    zstring_view value
) {
    T y{}; // create with default value
    if (parse_error err = parse_integral(y, value, zero_integral<T>(), max_integral<T>())) {
        return err;
    }
    x = std::chrono::duration<T, Ratio>{y};
    return no_parse_error;
}

template<class IntOrigin>
parse_error parse_integral_list(std::string & x, zstring_view value) {
    for (auto r : get_split(value, ',')) {
        IntOrigin i;
        auto rng = trim(r);
        if (auto err = parse_integral<true>(
          i, {rng.begin(), rng.size()},
          min_integral<IntOrigin>(),
          max_integral<IntOrigin>())
        ) {
            return err;
        }
    }
    x.assign(value.data(), value.size());
    return no_parse_error;
}

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<int>> /*type*/, zstring_view value)
{ return parse_integral_list<int>(x, value); }

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<unsigned>> /*type*/, zstring_view value)
{ return parse_integral_list<unsigned>(x, value); }

inline bool str_compare(chars_view x, chars_view y)
{
    return x.size() == y.size() && 0 == std::memcmp(x.data(), y.data(), y.size());
}

template<std::size_t N>
struct UpperArray
{
    UpperArray(chars_view s)
    {
        if (s.size() > N) {
            return;
        }

        auto p = arr.data();
        for (char c : s) {
            *p++ = ('a' <= c && c <= 'z')  ? c - 'a' + 'A' : c;
        }
        str = {arr.data(), p};
    }

    chars_view str;

private:
    std::array<char, N> arr;
};

inline constexpr std::pair<chars_view, bool> boolean_strings[]{
    {"1"_av, true},
    {"ON"_av, true},
    {"YES"_av, true},
    {"TRUE"_av, true},
    {"0"_av, false},
    {"OFF"_av, false},
    {"NO"_av, false},
    {"FALSE"_av, false},
};

template<auto& Pairs>
constexpr std::size_t str_value_pairs_max_len()
{
    std::size_t n = 0;
    for (auto& pair : Pairs) {
        n = std::max(n, pair.first.size());
    }
    return n;
}

template<auto& Pairs, class T>
parse_error parse_str_value_pairs(T & x, zstring_view value, char const* error_msg)
{
    UpperArray<str_value_pairs_max_len<Pairs>()> av_value{value};

    for (auto const& pair : Pairs) {
        if (str_compare(pair.first, av_value.str)) {
            x = pair.second;
            return no_parse_error;
        }
    }

    return parse_error{error_msg};
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse_from_cfg(T & x, ::configs::spec_type<bool> /*type*/, zstring_view value)
{
    return parse_str_value_pairs<boolean_strings>(
        x, value, "bad value, expected: 1, on, yes, true, 0, off, no, false");
}

inline parse_error parse_from_cfg(
    uint32_t& x, ::configs::spec_type<::configs::spec_types::file_permission> /*type*/,
    zstring_view value)
{
    uint32_t tmp = 0;

    if (auto [p, ec] = std::from_chars(value.begin(), value.end(), tmp, 8);
        p != value.end() || ec != std::errc()
    ){
        return parse_error{"Cannot parse file permission because it's not an octal number"};
    }

    if (tmp > 0777) {
        return parse_error{"Cannot have file permission higher than 0777 number"};
    }

    x = tmp;
    return no_parse_error;
}

} // anonymous namespace

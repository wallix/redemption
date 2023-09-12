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

#include "configs/spec_types.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/strutils.hpp"
#include "utils/string_c.hpp"
#include "utils/hexadecimal_string_to_buffer.hpp"
#include "utils/colors.hpp"
#include "utils/file_permissions.hpp"
#include "utils/ascii.hpp"
#include "configs/parsers/parse_performance_flags.hpp"

#include <algorithm>
#include <cstring>
#include <charconv>
#include <chrono>
#include <string>
#include <type_traits>

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
    return zstring_view::from_null_terminated(str);
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
    return zstring_view::from_null_terminated(zbuf.data(), p-zbuf.data());
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
    auto r = std::to_chars(zbuf.begin(), zbuf.end(), x);
    assert(r.ec == std::errc());
    *r.ptr = '\0';
    return zstring_view::from_null_terminated(zbuf.data(), std::size_t(r.ptr - zbuf.data()));
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

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<std::string> /*type*/, bytes_view value)
{
    x.assign(value.as_chars().data(), value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse_from_cfg(char (&x)[N], ::configs::spec_type<::configs::spec_types::fixed_string> /*type*/, bytes_view value)
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
    bytes_view value
) {
    using namespace jln::literals;
    if (value.size() != N*2) {
        return parse_error{(
            "bad length, should be "_c + jln::ull_to_string_c_t<N*2>()
        ).c_str()};
    }

    std::array<unsigned char, N> tmp;
    if (!hexadecimal_string_to_buffer(value.as_chars(), make_writable_array_view(tmp))) {
        return parse_error{"bad format, expected hexadecimal value"};
    }

    key = tmp;

    return no_parse_error;
}

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<std::string>> /*type*/, bytes_view value)
{
    x.assign(value.as_chars().data(), value.size());
    return no_parse_error;
}

inline parse_error parse_from_cfg(
    ::configs::spec_types::directory_path & x,
    ::configs::spec_type<::configs::spec_types::directory_path> /*type*/,
    bytes_view value
) {
    x = value.as_chars();
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
    TInt & x, bytes_view value,
    std::integral_constant<TInt, min> /*unused*/,
    std::integral_constant<TInt, max> /*unused*/)
{
    if (REDEMPTION_UNLIKELY(value.empty())) {
        return parse_error{"empty value"};
    }

    TInt tmp;
    int base = 10;
    if (value.size() > 2 && value[0] == '0' && value[1] == 'x') {
        value = value.drop_front(2);
        base = 16;
    }

    auto const chars = value.as_chars();
    auto const r = std::from_chars(chars.begin(), chars.end(), tmp, base);

    if (r.ec == std::errc() && r.ptr == chars.end()) {
        if (tmp < min || max < tmp) {
            return make_invalid_range_error<TInt, min, max>();
        }

        x = tmp;
        return no_parse_error;
    }

    if (r.ec == std::errc::value_too_large) {
        return make_invalid_range_error<TInt, min, max>();
    }

    return InList
        ? parse_error{"bad format, expected list of decimal or hexadecimal number"
            " (ex: \"integral[, integral ...]*\")"}
        : parse_error{"bad format, expected decimal or hexadecimal number"};
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
parse_from_cfg(TInt & x, ::configs::spec_type<TInt> /*type*/, bytes_view value)
{
    return parse_integral(x, value, min_integral<TInt>(), max_integral<TInt>());
}

template<class T,
    ::configs::spec_types::underlying_type_for_range_t<T> min,
    ::configs::spec_types::underlying_type_for_range_t<T> max>
parse_error parse_from_cfg(
    T & x,
    ::configs::spec_type<::configs::spec_types::range<T, min, max>> /*type*/,
    bytes_view value
) {
    using namespace jln::literals;
    using Int = ::configs::spec_types::underlying_type_for_range_t<T>;
    Int y;
    if (auto err = parse_integral(y, value, std::integral_constant<Int, min>(), std::integral_constant<Int, max>())) {
        return err;
    }
    x = T{y};
    return no_parse_error;
}

template<class T, class Ratio>
parse_error parse_from_cfg(
    std::chrono::duration<T, Ratio> & x,
    ::configs::spec_type<std::chrono::duration<T, Ratio>> /*type*/,
    bytes_view value
) {
    T y{}; // create with default value
    if (parse_error err = parse_integral(y, value, zero_integral<T>(), max_integral<T>())) {
        return err;
    }
    x = std::chrono::duration<T, Ratio>{y};
    return no_parse_error;
}

template<class IntOrigin>
parse_error parse_integral_list(std::string & x, bytes_view value) {
    const auto chars = value.as_chars();
    for (auto r : get_lines(chars, ',')) {
        IntOrigin i;
        auto rng = trim(r);
        if (auto err = parse_integral<true>(
            i, rng.as<bytes_view>(),
            min_integral<IntOrigin>(),
            max_integral<IntOrigin>())
        ) {
            return err;
        }
    }
    x.assign(chars.data(), chars.size());
    return no_parse_error;
}

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<int>> /*type*/, bytes_view value)
{ return parse_integral_list<int>(x, value); }

inline parse_error parse_from_cfg(std::string & x, ::configs::spec_type<::configs::spec_types::list<unsigned>> /*type*/, bytes_view value)
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

        auto p = buffer.data();
        for (char c : s) {
            *p++ = ('a' <= c && c <= 'z') ? c - 'a' + 'A' : c;
        }
        str = {buffer.data(), p};
    }

    chars_view str;

private:
    // fix -Wstringop-overflow= with gcc-10 to release (bug?)
    std::array<char, (N <= 9 ? 9 : N)> buffer;
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
parse_error parse_str_value_pairs(T & x, bytes_view value, char const* error_msg)
{
    UpperArray<str_value_pairs_max_len<Pairs>()> av_value{value.as_chars()};

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
parse_from_cfg(T & x, ::configs::spec_type<bool> /*type*/, bytes_view value)
{
    return parse_str_value_pairs<boolean_strings>(
        x, value, "bad value, expected: 1, on, yes, true, 0, off, no, false");
}

inline parse_error parse_from_cfg(
    FilePermissions& x, ::configs::spec_type<FilePermissions> /*type*/,
    bytes_view value)
{
    uint32_t tmp = 0;

    if (value.empty()) {
        return parse_error{"File permission is empty"};
    }

    auto chars = value.as_chars();

    if ('0' <= chars[0] && chars[0] <= '9') {
        if (auto [p, ec] = std::from_chars(chars.begin(), chars.end(), tmp, 8);
            p != chars.end() || ec != std::errc()
        ){
            return parse_error{"Cannot parse file permission because it's not an octal number"};
        }

        if (tmp > 0777) {
            return parse_error{"Cannot have file permission higher than 0777 number"};
        }

        x = FilePermissions(tmp);

        return no_parse_error;
    }

    char const* p = chars.begin();
    char const* e = chars.end();

    auto consume_right = [&p, e]{
        int r = 0;
        while (++p != e) {
            switch (*p) {
                case 'r': r |= 4; break;
                case 'w': r |= 2; break;
                case 'x': r |= 1; break;
                default: return r;
            }
        }
        return r;
    };

    parse_error parsing_error{
        "Cannot parse file permission because it's not an octal number or a symbolic mode format"
    };

    for (;;) {
        int mask;

        switch (*p) {
            case 'u': mask = 0700; ++p; break;
            case 'g': mask = 0070; ++p; break;
            case 'o': mask = 0007; ++p; break;
            case 'a': mask = 0777; ++p; break;
            case '+':
            case '-':
            case '=': mask = 0775; break;
            default: return parsing_error;
        }

        if (p == e) return parsing_error;

        int r;
        switch (*p) {
            case '=':
                r = consume_right();
                tmp = ((r << 6) | (r << 3) | r) & mask;
                break;
            case '+':
                r = consume_right();
                tmp |= ((r << 6) | (r << 3) | r) & mask;
                break;
            case '-':
                r = consume_right();
                tmp &= ~(((r << 6) | (r << 3) | r) & mask);
                break;
            default:
                continue;
        }

        if (p == e) {
            x = FilePermissions(tmp);
            return no_parse_error;
        }

        switch (*p) {
            case ',':
                while (++p != e && *p == ' ') {
                }
                if (p == e) {
                    x = FilePermissions(tmp);
                    return no_parse_error;
                }
                break;

            default: return parsing_error;
        }
    }
}

inline parse_error parse_from_cfg(
    RdpPerformanceFlags& x, ::configs::spec_type<RdpPerformanceFlags> /*type*/,
    bytes_view flags)
{
    return parse_error{parse_performance_flags(x, flags.as_chars())};
}

#define X_COLOR_RGB(f)                  \
    f("aliceblue", 0xf0f8ff)            \
    f("antiquewhite", 0xfaebd7)         \
    f("aqua", 0x00ffff)                 \
    f("aquamarine", 0x7fffd4)           \
    f("azure", 0xf0ffff)                \
    f("beige", 0xf5f5dc)                \
    f("bisque", 0xffe4c4)               \
    f("black", 0x000000)                \
    f("blanchedalmond", 0xffebcd)       \
    f("blue", 0x0000ff)                 \
    f("blueviolet", 0x8a2be2)           \
    f("brown", 0xa52a2a)                \
    f("burlywood", 0xdeb887)            \
    f("cadetblue", 0x5f9ea0)            \
    f("chartreuse", 0x7fff00)           \
    f("chocolate", 0xd2691e)            \
    f("coral", 0xff7f50)                \
    f("cornflowerblue", 0x6495ed)       \
    f("cornsilk", 0xfff8dc)             \
    f("crimson", 0xdc143c)              \
    f("cyan", 0x00ffff)                 \
    f("darkblue", 0x00008b)             \
    f("darkcyan", 0x008b8b)             \
    f("darkgoldenrod", 0xb8860b)        \
    f("darkgray", 0xa9a9a9)             \
    f("darkgreen", 0x006400)            \
    f("darkgrey", 0xa9a9a9)             \
    f("darkkhaki", 0xbdb76b)            \
    f("darkmagenta", 0x8b008b)          \
    f("darkolivegreen", 0x556b2f)       \
    f("darkorange", 0xff8c00)           \
    f("darkorchid", 0x9932cc)           \
    f("darkred", 0x8b0000)              \
    f("darksalmon", 0xe9967a)           \
    f("darkseagreen", 0x8fbc8f)         \
    f("darkslateblue", 0x483d8b)        \
    f("darkslategray", 0x2f4f4f)        \
    f("darkslategrey", 0x2f4f4f)        \
    f("darkturquoise", 0x00ced1)        \
    f("darkviolet", 0x9400d3)           \
    f("deeppink", 0xff1493)             \
    f("deepskyblue", 0x00bfff)          \
    f("dimgray", 0x696969)              \
    f("dimgrey", 0x696969)              \
    f("dodgerblue", 0x1e90ff)           \
    f("firebrick", 0xb22222)            \
    f("floralwhite", 0xfffaf0)          \
    f("forestgreen", 0x228b22)          \
    f("fuchsia", 0xff00ff)              \
    f("gainsboro", 0xdcdcdc)            \
    f("ghostwhite", 0xf8f8ff)           \
    f("gold", 0xffd700)                 \
    f("goldenrod", 0xdaa520)            \
    f("gray", 0x808080)                 \
    f("green", 0x008000)                \
    f("greenyellow", 0xadff2f)          \
    f("grey", 0x808080)                 \
    f("honeydew", 0xf0fff0)             \
    f("hotpink", 0xff69b4)              \
    f("indianred", 0xcd5c5c)            \
    f("indigo", 0x4b0082)               \
    f("ivory", 0xfffff0)                \
    f("khaki", 0xf0e68c)                \
    f("lavender", 0xe6e6fa)             \
    f("lavenderblush", 0xfff0f5)        \
    f("lawngreen", 0x7cfc00)            \
    f("lemonchiffon", 0xfffacd)         \
    f("lightblue", 0xadd8e6)            \
    f("lightcoral", 0xf08080)           \
    f("lightcyan", 0xe0ffff)            \
    f("lightgoldenrodyellow", 0xfafad2) \
    f("lightgray", 0xd3d3d3)            \
    f("lightgreen", 0x90ee90)           \
    f("lightgrey", 0xd3d3d3)            \
    f("lightpink", 0xffb6c1)            \
    f("lightsalmon", 0xffa07a)          \
    f("lightseagreen", 0x20b2aa)        \
    f("lightskyblue", 0x87cefa)         \
    f("lightslategray", 0x778899)       \
    f("lightslategrey", 0x778899)       \
    f("lightsteelblue", 0xb0c4de)       \
    f("lightyellow", 0xffffe0)          \
    f("lime", 0x00ff00)                 \
    f("limegreen", 0x32cd32)            \
    f("linen", 0xfaf0e6)                \
    f("magenta", 0xff00ff)              \
    f("maroon", 0x800000)               \
    f("mediumaquamarine", 0x66cdaa)     \
    f("mediumblue", 0x0000cd)           \
    f("mediumorchid", 0xba55d3)         \
    f("mediumpurple", 0x9370db)         \
    f("mediumseagreen", 0x3cb371)       \
    f("mediumslateblue", 0x7b68ee)      \
    f("mediumspringgreen", 0x00fa9a)    \
    f("mediumturquoise", 0x48d1cc)      \
    f("mediumvioletred", 0xc71585)      \
    f("midnightblue", 0x191970)         \
    f("mintcream", 0xf5fffa)            \
    f("mistyrose", 0xffe4e1)            \
    f("moccasin", 0xffe4b5)             \
    f("navajowhite", 0xffdead)          \
    f("navy", 0x000080)                 \
    f("oldlace", 0xfdf5e6)              \
    f("olive", 0x808000)                \
    f("olivedrab", 0x6b8e23)            \
    f("orange", 0xffa500)               \
    f("orangered", 0xff4500)            \
    f("orchid", 0xda70d6)               \
    f("palegoldenrod", 0xeee8aa)        \
    f("palegreen", 0x98fb98)            \
    f("paleturquoise", 0xafeeee)        \
    f("palevioletred", 0xdb7093)        \
    f("papayawhip", 0xffefd5)           \
    f("peachpuff", 0xffdab9)            \
    f("peru", 0xcd853f)                 \
    f("pink", 0xffc0cb)                 \
    f("plum", 0xdda0dd)                 \
    f("powderblue", 0xb0e0e6)           \
    f("purple", 0x800080)               \
    f("rebeccapurple", 0x663399)        \
    f("red", 0xff0000)                  \
    f("rosybrown", 0xbc8f8f)            \
    f("royalblue", 0x4169e1)            \
    f("saddlebrown", 0x8b4513)          \
    f("salmon", 0xfa8072)               \
    f("sandybrown", 0xf4a460)           \
    f("seagreen", 0x2e8b57)             \
    f("seashell", 0xfff5ee)             \
    f("sienna", 0xa0522d)               \
    f("silver", 0xc0c0c0)               \
    f("skyblue", 0x87ceeb)              \
    f("slateblue", 0x6a5acd)            \
    f("slategray", 0x708090)            \
    f("slategrey", 0x708090)            \
    f("snow", 0xfffafa)                 \
    f("springgreen", 0x00ff7f)          \
    f("steelblue", 0x4682b4)            \
    f("tan", 0xd2b48c)                  \
    f("teal", 0x008080)                 \
    f("thistle", 0xd8bfd8)              \
    f("tomato", 0xff6347)               \
    f("turquoise", 0x40e0d0)            \
    f("violet", 0xee82ee)               \
    f("wheat", 0xf5deb3)                \
    f("white", 0xffffff)                \
    f("whitesmoke", 0xf5f5f5)           \
    f("yellow", 0xffff00)               \
    f("yellowgreen", 0x9acd32)

inline parse_error parse_from_cfg(
    ::configs::spec_types::rgb& x, ::configs::spec_type<::configs::spec_types::rgb> /*type*/,
    bytes_view value)
{
    using Rgb = ::configs::spec_types::rgb;

    const parse_error parsing_error{"invalid color, expected #rgb, #rrggbb, hexadecimal value or web colors"};

    auto sv = std::string_view{value.as_chars().data(), value.size()};

    // #rgb or #rrggbb
    if (((sv.size() == 7 || sv.size() == 4) && sv[0] == '#')
    // hexadecimal
     || (sv.size() > 2 && sv[0] == '0' && sv[1] == 'x')
    ) {
        uint32_t color;
        auto d = sv[0] == '#' ? 1u : 2u;
        auto [p, ec] = std::from_chars(sv.begin() + d, sv.end(), color, 16);
        if (ec == std::errc() && p == sv.end() && color <= 0xffffffu) {
            // rgb -> rrggbb
            if (sv.size() == 4 && sv[0] == '#') {
                uint32_t r = (color >> 8) & 0xf;
                uint32_t g = (color >> 4) & 0xf;
                uint32_t b = (color >> 0) & 0xf;
                color = (((r << 4) | r) << 16) | (((g << 4) | g) << 8) | ((b << 4) | b);
            }
            x = Rgb(color);
            return no_parse_error;
        }
        return parsing_error;
    }

    // named color
    auto lower = ascii_to_limited_lower<20>(sv);
    #define NAMED_COLOR_COND(name, rgb)             \
        if (std::string_view(name) == lower.sv()) { \
            x = Rgb(rgb);                           \
            return no_parse_error;                  \
        }
    X_COLOR_RGB(NAMED_COLOR_COND)
    #undef NAMED_COLOR_COND

    return parsing_error;
}

#undef X_COLOR_RGB

} // anonymous namespace

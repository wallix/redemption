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

#include <limits>
#include <string>
#include <array>
#include <chrono>
#include <algorithm>

#include <cstdlib>
#include <cerrno>

#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/splitter.hpp"

#include "utils/log.hpp"
#include "cxx/diagnostic.hpp"

namespace configs {

template<std::size_t N>
struct zstr_buffer
{
    zstr_buffer() : buf() {}

    static constexpr std::size_t size() { return N; }
    char * get() { return this->buf; }

private:
    char buf[N+1];
};

template<>
struct zstr_buffer<0>
{
    static constexpr std::size_t size() { return 0; }
    char * get() { return nullptr; }
};

namespace detail
{
    template<class T, bool = std::is_integral<T>::value, bool = std::is_enum<T>::value>
    struct zstr_buffer_traits
    { using type = zstr_buffer<0>; };

    template<class TInt>
    constexpr std::size_t integral_buffer_size()
    { return std::numeric_limits<TInt>::digits10 + 1 + std::numeric_limits<TInt>::is_signed; }

    template<class T>
    struct zstr_buffer_traits<T, true, false>
    { using type = zstr_buffer<integral_buffer_size<T>() + 1>; };

    template<class T>
    struct zstr_buffer_traits<T, false, true>
    : zstr_buffer_traits<typename std::underlying_type<T>::type, true, false>
    {};
} // namespace detail

template<class T>
struct zstr_buffer_traits
{ using type = typename detail::zstr_buffer_traits<T>::type; };

template<class T, class Ratio>
struct zstr_buffer_traits<std::chrono::duration<T, Ratio>>
: zstr_buffer_traits<T>
{};

template<> struct zstr_buffer_traits<void> { using type = zstr_buffer<0>; };
template<> struct zstr_buffer_traits<std::string> { using type = zstr_buffer<0>; };
template<std::size_t N> struct zstr_buffer_traits<char[N]> { using type = zstr_buffer<0>; };

template<std::size_t N>
struct zstr_buffer_traits<std::array<unsigned char, N>>
{ using type = zstr_buffer<N*2>; };


template<class T>
using zstr_buffer_from = typename zstr_buffer_traits<T>::type;


template<class> struct spec_type {};

namespace spec_types
{
    class fixed_binary;
    class fixed_string;
    template<class T> class list;
    using ip = std::string;

    template<class T>
    struct underlying_type_for_range
    {
        using type = T;
    };

    template<class Rep, class Period>
    struct underlying_type_for_range<std::chrono::duration<Rep, Period>>
    {
        using type = Rep;
    };

    template<class T>
    using underlying_type_for_range_t = typename underlying_type_for_range<T>::type;

    template<class T, underlying_type_for_range_t<T> min, underlying_type_for_range_t<T> max>
    struct range {};

    struct directory_path
    {
        directory_path()
        : path("./")
        {}

        directory_path(std::string path)
        : path(std::move(path))
        { this->normalize(); }

        directory_path(char const * path)
        : path(path)
        { this->normalize(); }

        directory_path(directory_path &&) = default;
        directory_path(directory_path const &) = default;

        directory_path & operator = (std::string new_path)
        {
            this->path = std::move(new_path);
            this->normalize();
            return *this;
        }

        directory_path & operator = (char const * new_path)
        {
            this->path = new_path;
            this->normalize();
            return *this;
        }

        directory_path & operator = (directory_path &&) = default;
        directory_path & operator = (directory_path const &) = default;

        char const * c_str() const noexcept { return this->path.c_str(); }

        std::string const & to_string() const noexcept { return this->path; }

    private:
        void normalize()
        {
            if (this->path.empty()) {
                this->path = "./";
            }
            else {
                if (this->path.front() != '/') {
                    if (not(this->path.size() >= 2 && this->path[0] == '.' && this->path[1] == '/')) {
                        this->path.insert(0, "./");
                    }
                }
                if (this->path.back() != '/') {
                    this->path += '/';
                }
            }
        }

        std::string path;
    };

    inline bool operator == (directory_path const & x, directory_path const & y) { return x.to_string() == y.to_string(); }
    inline bool operator == (std::string const & x, directory_path const & y) { return x == y.to_string(); }
    inline bool operator == (char const * x, directory_path const & y) { return x == y.to_string(); }
    inline bool operator == (directory_path const & x, std::string const & y) { return x.to_string() == y; }
    inline bool operator == (directory_path const & x, char const * y) { return x.to_string() == y; }

    inline bool operator != (directory_path const & x, directory_path const & y) { return !(x == y); }
    inline bool operator != (std::string const & x, directory_path const & y) { return !(x == y); }
    inline bool operator != (char const * x, directory_path const & y) { return !(x == y); }
    inline bool operator != (directory_path const & x, std::string const & y) { return !(x == y); }
    inline bool operator != (directory_path const & x, char const * y) { return !(x == y); }

    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> & operator << (std::basic_ostream<Ch, Tr> & out, directory_path const & path)
    { return out << path.to_string(); }
} // namespace spec_types



template<class> struct cfg_s_type {};

// assign_zbuf_from_cfg (guarantee with null terminal)
//@{

inline array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<std::string> & /*buf*/,
    cfg_s_type<std::string> /*type*/,
    std::string const & str
) { return {str.c_str(), str.size()}; }

inline array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<std::string> & /*buf*/,
    cfg_s_type<spec_types::list<std::string>> /*type*/,
    std::string const & str
) { return {str.c_str(), str.size()}; }

template<std::size_t N>
array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<char[N]> & /*buf*/,
    cfg_s_type<char[N]> /*type*/,
    char const (&s)[N])
{ return {s, strlen(s)}; }

inline array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<bool> & /*type*/,
    cfg_s_type<bool> /*type*/,
    bool x)
{ return x ? cstr_array_view("True") : cstr_array_view("False"); }

template<std::size_t N>
array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<std::array<unsigned char, N>> & buf,
    cfg_s_type<spec_types::fixed_binary> /*type*/,
    std::array<unsigned char, N> const & arr
) {
    char * p = buf.get();
    for (int c : arr) {
        auto x = (c & 0xf0) >> 4;
        *p++ = x < 10 ? ('0' + x) : ('A' + x - 10);
        x = c & 0xf;
        *p++ = x < 10 ? ('0' + x) : ('A' + x - 10);
    }
    return array_view_const_char(buf.get(), p-buf.get());
}

template<class T,
    spec_types::underlying_type_for_range_t<T> min,
    spec_types::underlying_type_for_range_t<T> max>
array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<T> & zbuf,
    cfg_s_type<spec_types::range<T, min, max>> /*type*/,
    T const & rng
) { return assign_zbuf_from_cfg(zbuf, cfg_s_type<T>{}, rng); }

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value, array_view_const_char>::type
assign_zbuf_from_cfg(zstr_buffer_from<TInt> & buf, cfg_s_type<TInt> /*type*/, TInt const & x)
{
    int sz = (std::is_signed<TInt>::value)
        ? snprintf(buf.get(), buf.size(), "%lld", static_cast<long long>(x))
        : snprintf(buf.get(), buf.size(), "%llu", static_cast<unsigned long long>(x));
    return array_view_const_char(buf.get(), sz);
}

template<class E>
typename std::enable_if<std::is_enum<E>::value, array_view_const_char>::type
assign_zbuf_from_cfg(zstr_buffer_from<E> & buf, cfg_s_type<E> /*type*/, E const & x)
{
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return array_view_const_char(buf.get(), sz);
}


template<class T, class Ratio>
array_view_const_char assign_zbuf_from_cfg(
    zstr_buffer_from<std::chrono::duration<T, Ratio>> & buf,
    cfg_s_type<std::chrono::duration<T, Ratio>> /*type*/,
    std::chrono::duration<T, Ratio> const & x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x.count()));
    return array_view_const_char(buf.get(), sz);
}
//@}


template<class T>
zstr_buffer_from<T> make_zstr_buffer(T const & x)
{
    zstr_buffer_from<T> buf;
    assign_zbuf_from_cfg(buf, cfg_s_type<T>{}, x);
    return buf;
}


//
// parse
//

struct parse_error
{
    constexpr explicit parse_error(char const * err) noexcept : s_err(err) {}

    explicit operator bool () const noexcept { return this->s_err; }
    char const * c_str() const noexcept { return this->s_err; }

private:
    char const * s_err;
};

namespace detail
{
    template<char... c>
    struct string_literal
    {
        template<char... c2>
        string_literal<c..., c2...>
        operator+(string_literal<c2...> const& /*unused*/) const
        { return {}; }

        operator char const * () const
        {
            return c_str;
        }

        static constexpr char c_str[sizeof...(c)+1] = {c..., '\0'};
    };

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
    template<class T, T... c>
    string_literal<c...> operator "" _mp_str()
    { return {}; }
    REDEMPTION_DIAGNOSTIC_POP


    template<long long val, char... c>
    struct int_to_meta_string_impl
      : int_to_meta_string_impl<val/10, (val % 10) + '0', c...>
    {};

    template<char... c>
    struct int_to_meta_string_impl<0, c...>
    {
        using type = string_literal<c...>;
    };

    template<class T, T val, bool negate = (val < 0)>
    struct int_to_meta_string
    {
        using type = decltype("-"_mp_str +
            typename int_to_meta_string_impl<-(val / 10), -(val % 10) + '0'>::type());
    };

    template<class T, T val>
    struct int_to_meta_string<T, val, false>
      : int_to_meta_string_impl<val / 10, val % 10 + '0'>
    {};

    template<class T, T val>
    using mp_to_string = typename int_to_meta_string<T, val>::type;
} // namespace detail


constexpr parse_error no_parse_error {nullptr};

inline parse_error parse(std::string & x, spec_type<std::string> /*type*/, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse(char (&x)[N], spec_type<char[N]> /*type*/, array_view_const_char value)
{
    using namespace detail;
    if (value.size() >= N-1) {
        return parse_error{"out of bounds, limits is "_mp_str + mp_to_string<long, N-1>()};
    }
    memcpy(x, value.data(), value.size());
    x[value.size()] = 0;
    return no_parse_error;
}

template<std::size_t N>
parse_error parse(char (&x)[N], spec_type<spec_types::fixed_string> /*type*/, array_view_const_char value)
{
    using namespace detail;
    if (value.size() >= N) {
        return parse_error{"out of bounds, limits is "_mp_str + mp_to_string<long, N>()};
    }
    memcpy(x, value.data(), value.size());
    memset(x + value.size(), 0, N - value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse(
    std::array<unsigned char, N> & key,
    spec_type<spec_types::fixed_binary> /*type*/,
    array_view_const_char value
) {
    using namespace detail;
    if (value.size() != N*2) {
        return parse_error{"bad length, should be "_mp_str + mp_to_string<long, N*2>()};
    }

    char   hexval[3] = { 0 };
    char * end{};
    for (std::size_t i = 0; i < N; i++) {
        hexval[0] = value[i*2];
        hexval[1] = value[i*2+1];
        key[i] = strtol(hexval, &end, 16);
        if (end != hexval+2) {
            return parse_error{"bad format, expected hexadecimal value"};
        }
    }

    return no_parse_error;
}

inline parse_error parse(std::string & x, spec_type<spec_types::list<std::string>> /*type*/, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

inline parse_error parse(
    spec_types::directory_path & x,
    spec_type<spec_types::directory_path> /*type*/,
    array_view_const_char value
) {
    x = std::string(value.data(), value.size());
    return no_parse_error;
}

template<class T,
    spec_types::underlying_type_for_range_t<T> min,
    spec_types::underlying_type_for_range_t<T> max>
parse_error parse(
    T & x,
    spec_type<spec_types::range<T, min, max>> /*type*/,
    array_view_const_char value
) {
    using namespace detail;
    using Int = spec_types::underlying_type_for_range_t<T>;
    Int y;
    if (auto err = parse(y, spec_type<Int>{}, value)) {
        return err;
    }
    if (y < min || max < y) {
        return parse_error{
            "invalid range in ["_mp_str +
            mp_to_string<Int, min>() + ", "_mp_str + mp_to_string<Int, max>() +
            "]"_mp_str};
    }
    x = T{y};
    return no_parse_error;
}

namespace detail
{
    inline char const * ignore_0(char const * buf, char const * end)
    { return std::find_if_not(buf, end, [](char c){ return c == '0'; }); }

    template<bool InList = false, class TInt, TInt min, TInt max>
    parse_error parse_integral(
        TInt & x, array_view_const_char value,
        std::integral_constant<TInt, min> /*unused*/, std::integral_constant<TInt, max> /*unused*/)
    {
        range<char const *> rng = trim(value);

        TInt val{};
        {
            std::size_t idx = 0;
            int base = 10;
            if (rng.size() >= 2 && rng[0] == '0' && rng[1] == 'x') {
                idx = 2;
                base = 16;
            }
            std::size_t ignored = ignore_0(rng.begin()+idx, rng.end()) - rng.begin();
            std::size_t sz = rng.size() - ignored;

            constexpr std::size_t buf_sz = detail::integral_buffer_size<TInt>();
            if (sz > buf_sz) {
                return parse_error{"too large, limited to"_mp_str +
                    mp_to_string<std::size_t, buf_sz>()};
            }
            if (sz == 0) {
                x = 0;
                return no_parse_error;
            }

            char buf[buf_sz+1];
            memcpy(buf, rng.begin() + ignored, sz);
            buf[sz] = 0;
            char * end{};
            if (std::is_signed<TInt>::value) {
                val = strtoll(buf, &end, base);
            }
            else {
                val = strtoull(buf, &end, base);
            }
            if (errno == ERANGE || std::size_t(end - buf) != sz) {
                return InList
                    ? parse_error{"bad format, expected list of decimal, hexadecimal or octal (ex: \"integral[, integral ...]*\")"}
                    : parse_error{"bad format, expected decimal, hexadecimal or octal"};
            }
        }

        if (val < min || max < val) {
            return parse_error{
                "invalid range in ["_mp_str +
                mp_to_string<TInt, min>() + ", "_mp_str + mp_to_string<TInt, max>() +
                "]"_mp_str};
        }

        x = val;
        return no_parse_error;
    }
} // namespace detail

namespace detail
{
    template<class T>
    using max_integral = std::integral_constant<T, std::numeric_limits<T>::max()>;
    template<class T>
    using min_integral = std::integral_constant<T, std::numeric_limits<T>::min()>;
    template<class T>
    using zero_integral = std::integral_constant<T, 0>;
} // namespace detail

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value && !std::is_same<TInt, bool>::value, parse_error>::type
parse(TInt & x, spec_type<TInt> /*type*/, array_view_const_char value)
{ return detail::parse_integral(x, value, detail::min_integral<TInt>(), detail::max_integral<TInt>()); }

template<class T, class Ratio>
parse_error parse(
    std::chrono::duration<T, Ratio> & x,
    spec_type<std::chrono::duration<T, Ratio>> /*type*/,
    array_view_const_char value
) {
    T y{}; // create with default value
    if (parse_error err = detail::parse_integral(y, value, detail::zero_integral<T>(), detail::max_integral<T>())) {
        return err;
    }
    x = std::chrono::duration<T, Ratio>{y};
    return no_parse_error;
}

namespace detail
{
    template<class IntOrigin>
    parse_error parse_integral_list(std::string & x, array_view_const_char value) {
        for (auto r : get_split(value, ',')) {
            IntOrigin i;
            if (auto err = parse_integral<true>(i, {r.begin(), r.size()}, detail::min_integral<IntOrigin>(), detail::max_integral<IntOrigin>())) {
                return err;
            }
        }
        x.assign(value.data(), value.size());
        return no_parse_error;
    }
} // namespace detail

inline parse_error parse(std::string & x, spec_type<spec_types::list<int>> /*type*/, array_view_const_char value)
{ return detail::parse_integral_list<int>(x, value); }

inline parse_error parse(std::string & x, spec_type<spec_types::list<unsigned>> /*type*/, array_view_const_char value)
{ return detail::parse_integral_list<unsigned>(x, value); }

namespace detail
{
    inline bool icase_equal_view(array_view_const_char x, array_view_const_char y)
    {
        return x.size() == y.size()
            && std::equal(x.begin(), x.end(), y.begin(), [](char c1, char c2) {
                auto to_upper = [](char c) { return ('a' <= c && c <= 'z')  ? c - 'a' + 'A' : c; };
                return to_upper(c1) == to_upper(c2);
            });
    }
} // namespace detail

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse(T & x, spec_type<bool> /*type*/, array_view_const_char value)
{
    range<char const *> rng = trim(value);

    struct SV {
        array_view_const_char s;
        bool val;
    };
    SV cmp_list[]{
        {cstr_array_view("1"), true},
        {cstr_array_view("on"), true},
        {cstr_array_view("yes"), true},
        {cstr_array_view("true"), true},
        {cstr_array_view("0"), false},
        {cstr_array_view("off"), false},
        {cstr_array_view("no"), false},
        {cstr_array_view("false"), false},
    };
    for (SV sv : cmp_list) {
        if (detail::icase_equal_view(sv.s, {rng.begin(), rng.size()})) {
            x = sv.val;
            return no_parse_error;
        }
    }
    return parse_error{"bad format, expected 1, on, yes, true, 0, off, no, false"};
}


template<class T, class U>
parse_error parse_adapter(T & x, U & y)
{
    x = std::move(y);
    return no_parse_error;
}

template<class T, class U>
typename std::enable_if<!std::is_same<T, U>::value>::type
parse(T & x, spec_type<U> ty, array_view_const_char value)
{
    U tmp;
    if (parse_error err = parse(tmp, ty, value)) {
        return err;
    }
    return parse_adapter(x, tmp);
}


template<class E, class Max>
parse_error parse_enum_u(E & x, array_view_const_char value, Max max)
{
    using ul = unsigned long;
    ul xi = 0;
    if (parse_error err = detail::parse_integral(xi, value, detail::zero_integral<ul>(), max)) {
        return err;
    }
    //if (~~static_cast<E>(xi) != static_cast<E>(xi)) {
    //    return parse_error{"bad value"};
    //}
    //x = ~~static_cast<E>(xi);
    x = static_cast<E>(xi);
    return no_parse_error;
}

template<class E>
parse_error parse_enum_str(
    E & x, array_view_const_char value,
    std::initializer_list<std::pair<array_view_const_char, E>> l
) {
    for (auto & p : l) {
        if (detail::icase_equal_view(value, p.first)) {
            x = p.second;
            return no_parse_error;
        }
    }
    return parse_error{"unknown value"};
}

template<class E>
parse_error parse_enum_list(E & x, array_view_const_char value, std::initializer_list<E> l)
{
    using ul = unsigned long;
    ul xi = 0;
    if (parse_error err = detail::parse_integral(xi, value, detail::min_integral<ul>(), detail::max_integral<ul>())) {
        return err;
    }
    for (auto val : l) {
        if (val == static_cast<E>(xi)) {
            x = static_cast<E>(xi);
            return no_parse_error;
        }
    }
    return parse_error{"unknown value"};
}


namespace detail
{
    template<class T, class Spec>
    struct set_value_impl
    {
        template<class U>
        static void impl(T & x, U && new_value)
        { x = std::forward<U>(new_value); }

        template<class U, class... Ts>
        static void impl(T & x, U && param1, Ts && ... other_params)
        { x = {std::forward<U>(param1), std::forward<Ts>(other_params)...}; }
    };

    template<>
    struct set_value_impl<std::array<unsigned char, 32>, spec_types::fixed_binary>
    {
        static constexpr std::size_t N = 32;
        using T = std::array<unsigned char, N>;

        static void impl(T & x, T const & arr)
        { x = arr; }

        static void impl(T & x, unsigned char const (&arr)[N])
        { std::copy(begin(arr), end(arr), begin(x)); }

        static void impl(T & x, char const (&arr)[N+1])
        {
            assert(arr[N] == '\0');
            std::copy(begin(arr), end(arr)-1, begin(x));
        }

        // template<class U>
        // static void impl(T & x, U const * arr, std::size_t n)
        // {
        //     assert(N >= n);
        //     std::copy(arr, arr + n, begin(x));
        // }
    };

    template<std::size_t N>
    struct set_value_impl<char[N], char[N]>
    {
        using T = char[N];

        static void impl(T & x, char const * s, std::size_t n)
        {
            assert(N > n);
            n = std::min(n, N-1);
            memcpy(x, s, n);
            x[n] = 0;
        }

        static void impl(T & x, char const * s)
        { impl(x, s, strnlen(s, N-1)); }

        static void impl(T & x, std::string const & str)
        { impl(x, str.data(), str.size()); }
    };

    template<std::size_t N>
    struct set_value_impl<char[N], spec_types::fixed_string>
    {
        using T = char[N];

        static void impl(T & x, char const * s, std::size_t n)
        {
            assert(N >= n);
            n = std::min(n, N-1);
            memcpy(x, s, n);
            memset(x + n, 0, N - n);
        }

        static void impl(T & x, char const * s)
        { impl(x, s, strnlen(s, N-1)); }

        static void impl(T & x, std::string const & str)
        { impl(x, str.data(), str.size()); }
    };

    template<class T,
        spec_types::underlying_type_for_range_t<T> min,
        spec_types::underlying_type_for_range_t<T> max>
    struct set_value_impl<T, spec_types::range<T, min, max>>
    {
        static void impl(T & x, T new_value)
        {
            assert(T{min} <= new_value || new_value <= T{max});
            x = new_value;
        }
    };
} // namespace detail

template<class T, class Spec, class U>
void set_value(T & x, spec_type<Spec> /*type*/, U && new_value)
{ detail::set_value_impl<T, Spec>::impl(x, std::forward<U>(new_value)); }

template<class T, class Spec, class U, class... Ts>
void set_value(T & x, spec_type<Spec> /*type*/, U && param1, Ts && ... other_params)
{ detail::set_value_impl<T, Spec>::impl(x, std::forward<U>(param1), std::forward<Ts>(other_params)...); }

} // namespace configs

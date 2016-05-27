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
#include <algorithm>

#include <cstdlib>

#include "utils/algostring.hpp"
#include "utils/array_view.hpp"
#include "utils/splitter.hpp"

// TODO
#include "utils/log.hpp"

namespace configs {

template<std::size_t N>
struct szbuffer
{
    static constexpr std::size_t size() { return N; }
    char * get() { return this->buf; }

private:
    char buf[N+1];
};

template<>
struct szbuffer<0>
{
    static constexpr std::size_t size() { return 0; }
    char * get() { return nullptr; }
};

namespace detail
{
    template<class T, bool = std::is_integral<T>::value, bool = std::is_enum<T>::value>
    struct szbuffer_traits
    { using type = szbuffer<0>; };

    template<class TInt>
    constexpr std::size_t integral_buffer_size()
    { return std::numeric_limits<TInt>::digits10 + 1 + std::numeric_limits<TInt>::is_signed; }

    template<class T>
    struct szbuffer_traits<T, true, false>
    { using type = szbuffer<integral_buffer_size<T>()>; };

    template<class T>
    struct szbuffer_traits<T, false, true>
    : szbuffer_traits<typename std::underlying_type<T>::type, true, false>
    {};
}

template<class T>
struct szbuffer_traits
{ using type = typename detail::szbuffer_traits<T>::type; };

template<> struct szbuffer_traits<void> { using type = szbuffer<0>; };
template<> struct szbuffer_traits<std::string> { using type = szbuffer<0>; };
template<std::size_t N> struct szbuffer_traits<char[N]> { using type = szbuffer<0>; };

template<std::size_t N>
struct szbuffer_traits<std::array<unsigned char, N>>
{ using type = szbuffer<N*2>; };


template<class T>
using szbuffer_from = typename szbuffer_traits<T>::type;


struct non_owner_string
{
    constexpr non_owner_string(char const * s, std::size_t sz) noexcept
    : s(s)
    , sz(sz)
    {}

    template<std::size_t N>
    constexpr non_owner_string(char const (&s)[N]) noexcept
    : s(s)
    , sz(N)
    {}

    char const * c_str() const { return this->s; }
    std::size_t size() const { return this->sz; }

private:
    char const * s;
    std::size_t sz;
};


template<class> struct cfg_s_type {};

inline non_owner_string cfg_to_s(
    szbuffer_from<std::string> &,
    cfg_s_type<std::string>,
    std::string const & str
) { return {str.c_str(), str.size()}; }

template<std::size_t N>
non_owner_string cfg_to_s(szbuffer_from<char[N]> &, cfg_s_type<char[N]>,  char const (&s)[N])
{ return {s, strlen(s)}; }

inline non_owner_string cfg_to_s(szbuffer_from<bool>&, cfg_s_type<bool>, bool x)
{ return x ? non_owner_string("True") : non_owner_string("False"); }

template<std::size_t N>
non_owner_string cfg_to_s(
    szbuffer_from<std::array<unsigned char, N>> & buf,
    cfg_s_type<std::array<unsigned char, N>>,
    std::array<unsigned char, N> const & arr
) {
    char * p = buf.get();
    for (int c : arr) {
        auto x = (c & 0xf0) >> 4;
        *p++ = x < 10 ? ('0' + x) : ('A' + x - 10);
        x = c & 0xf;
        *p++ = x < 10 ? ('0' + x) : ('A' + x - 10);
    }
    return {buf.get(), p};
}

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value, non_owner_string>::type
cfg_to_s(szbuffer_from<TInt> & buf, cfg_s_type<TInt>, TInt const & x)
{
    int sz = (std::is_signed<TInt>::value)
        ? snprintf(buf.get(), buf.size(), "%lld", static_cast<long long>(x))
        : snprintf(buf.get(), buf.size(), "%llu", static_cast<unsigned long long>(x));
    return non_owner_string(buf.get(), sz);
}

template<class E>
typename std::enable_if<std::is_enum<E>::value, non_owner_string>::type
cfg_to_s(szbuffer_from<E> & buf, cfg_s_type<E>, E const & x)
{
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}



template<class> struct spec_type {};

namespace spec_types
{
    template<unsigned> class fixed_binary;
    template<class T> class list;
    using ip = std::string;

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
                    this->path.insert(0, "./");
                }
                if (this->path.back() != '/') {
                    this->path += '/';
                }
            }
        }

        std::string path;
    };

    bool operator == (directory_path const & x, directory_path const & y) { return x.to_string() == y.to_string(); }
    bool operator == (std::string const & x, directory_path const & y) { return x == y.to_string(); }
    bool operator == (char const * x, directory_path const & y) { return x == y.to_string(); }
    bool operator == (directory_path const & x, std::string const & y) { return x.to_string() == y; }
    bool operator == (directory_path const & x, char const * y) { return x.to_string() == y; }

    bool operator != (directory_path const & x, directory_path const & y) { return !(x == y); }
    bool operator != (std::string const & x, directory_path const & y) { return !(x == y); }
    bool operator != (char const * x, directory_path const & y) { return !(x == y); }
    bool operator != (directory_path const & x, std::string const & y) { return !(x == y); }
    bool operator != (directory_path const & x, char const * y) { return !(x == y); }

    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> & operator << (std::basic_ostream<Ch, Tr> & out, directory_path const & path)
    { return out << path.to_string(); }
}

struct parse_error
{
    char const * s_err = nullptr;

    explicit operator bool () const { return this->s_err; }
    char const * c_str() const { return this->s_err; }
};

constexpr parse_error no_parse_error {nullptr};


inline parse_error parse(std::string & x, spec_type<std::string>, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

template<std::size_t N>
parse_error parse(char (&x)[N], spec_type<char[N]>, array_view_const_char value)
{
    if (value.size() >= N-1) {
        return {"out of bounds"};
    }
    memcpy(x, value.data(), value.size());
    x[value.size()] = 0;
    return no_parse_error;
}

template<std::size_t N1, unsigned N2>
parse_error parse(
    std::array<unsigned char, N1> & key,
    spec_type<spec_types::fixed_binary<N2>>,
    array_view_const_char value
) {
    static_assert(N1 == N2, "");

    if (value.size() != N1*2) {
        return {"bad length"};
    }

    char   hexval[3] = { 0 };
    char * end{};
    for (std::size_t i = 0; i < N1; i++) {
        hexval[0] = value[i*2];
        hexval[1] = value[i*2+1];
        key[i] = strtol(hexval, &end, 16);
        if (end != hexval+2) {
            return {"bad format"};
        }
    }

    return no_parse_error;
}

inline parse_error parse(std::string & x, spec_type<spec_types::list<std::string>>, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

inline parse_error parse(
    spec_types::directory_path & x,
    spec_type<spec_types::directory_path>,
    array_view_const_char value
) {
    x = std::string(value.data(), value.size());
    return no_parse_error;
}

namespace detail
{
    template<class T>
    inline T * ignore_0(T * buf, std::size_t sz)
    { return std::find_if_not(buf, buf + sz, [](char c){ return c == '0'; }); }

    template<class TInt>
    parse_error parse_integral(TInt & x, array_view_const_char value, TInt min, TInt max)
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
            std::size_t ignored = ignore_0(rng.begin()+idx, rng.size()-idx) - rng.begin();
            std::size_t sz = rng.size() - ignored;

            constexpr std::size_t buf_sz = detail::integral_buffer_size<TInt>();
            if (sz >= buf_sz) {
                return {"too large"};
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
                return {"bad format"};
            }
        }

        if (val > max) {
            return {"too large"};
        }
        if (val < min) {
            return {"too short"};
        }

        x = val;
        return no_parse_error;
    }
}

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value && !std::is_same<TInt, bool>::value, parse_error>::type
parse(TInt & x, spec_type<TInt>, array_view_const_char value)
{ return detail::parse_integral(x, value, std::numeric_limits<TInt>::min(), std::numeric_limits<TInt>::max()); }

namespace detail
{
    template<class IntOrigni>
    parse_error parse_integral_list(std::string & x, array_view_const_char value) {
        for (auto r : get_split(value, ',')) {
            IntOrigni i;
            using limits = std::numeric_limits<IntOrigni>;
            if (auto err = parse_integral(i, {r.begin(), r.size()}, limits::min(), limits::max())) {
                if (strcmp(err.c_str(), "bad format")) {
                    return {"bad format, expected \"integral(, integral)*\""};
                }
            }
        }
        x.assign(value.data(), value.size());
        return no_parse_error;
    }
}

inline parse_error parse(std::string & x, spec_type<spec_types::list<int>>, array_view_const_char value)
{ return detail::parse_integral_list<int>(x, value); }

inline parse_error parse(std::string & x, spec_type<spec_types::list<unsigned>>, array_view_const_char value)
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
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse(T & x, spec_type<bool>, array_view_const_char value)
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
        {cstr_array_view("no"), false},
        {cstr_array_view("false"), false},
    };
    for (SV sv : cmp_list) {
        if (detail::icase_equal_view(sv.s, {rng.begin(), rng.size()})) {
            x = sv.val;
            return no_parse_error;
        }
    }
    return {"bad format, expected 1, on, yes, true, 0, no, false"};
}

template<class T, class U>
parse_error parse_adapter(T & x, U & y)
{
    x = std::move(y);
    return no_parse_error;
}

template<class T, class U>
std::enable_if_t<!std::is_same<T, U>::value>
parse(T & x, spec_type<U> ty, array_view_const_char value)
{
    U tmp;
    if (parse_error err = parse(tmp, ty, value)) {
        return err;
    }
    return parse_adapter(x, tmp);
}


template<class E>
parse_error parse_enum_u(E & x, array_view_const_char value, unsigned long max)
{
    unsigned long xi;
    if (parse_error err = detail::parse_integral(xi, value, 0ul, max)) {
        return err;
    }
    //if (~~static_cast<E>(xi) != static_cast<E>(xi)) {
    //    return {"bad value"};
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
    return {"unknown value"};
}

template<class E>
parse_error parse_enum_list(E & x, array_view_const_char value, std::initializer_list<E> l)
{
    unsigned long xi;
    using limits = std::numeric_limits<unsigned long>;
    if (parse_error err = detail::parse_integral(xi, value, limits::min(), limits::max())) {
        return err;
    }
    for (auto val : l) {
        if (val == static_cast<E>(xi)) {
            x = static_cast<E>(xi);
            return no_parse_error;
        }
    }
    return {"unknown value"};
}


template<class T, class U>
void set_value(T & x, U && new_value)
{ x = std::forward<U>(new_value); }

template<class T, class U, class... Ts>
void set_value(T & x, U && param1, Ts && ... other_params)
{ x = {std::forward<U>(param1), std::forward<Ts>(other_params)...}; }

namespace detail
{
    template<class T, class U>
    using enable_if_no_cv_is_same_t = typename std::enable_if<
        std::is_same<typename std::remove_cv<T>::type, U>::value
    >::type;
}

template<class T>
detail::enable_if_no_cv_is_same_t<T, unsigned char>
set_value(std::array<unsigned char, 32> & x, T (&arr)[32])
{ std::copy(begin(arr), end(arr), begin(x)); }

template<class T>
detail::enable_if_no_cv_is_same_t<T, char>
set_value(std::array<unsigned char, 32> & x, T (&arr)[33])
{ std::copy(begin(arr), end(arr), begin(x)); }

template<class T, class Int>
detail::enable_if_no_cv_is_same_t<T, char>
set_value(std::array<unsigned char, 32> & x, T * arr, Int n)
{
    assert(32 >= n);
    std::copy(arr, arr + n, begin(x));
}

template<std::size_t N, class T, class Int>
detail::enable_if_no_cv_is_same_t<T, char>
set_value(char (&s)[N], T * arr, Int n)
{
    // TODO
}

template<std::size_t N, class T>
detail::enable_if_no_cv_is_same_t<T, char>
set_value(char (&s)[N], T * arr)
{
    // TODO
}

template<std::size_t N>
void set_value(char (&s)[N], std::string & arr)
{
    // TODO
}

template<std::size_t N>
void set_value(char (&s)[N], std::string const & arr)
{
    // TODO
}

template<std::size_t N1, std::size_t N2, class T>
detail::enable_if_no_cv_is_same_t<T, char>
set_value(char (&s1)[N1], T (&s2)[N1])
{
    // TODO
}

}

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


namespace configs {

namespace detail {
    template<class T, bool = std::is_integral<T>::value, bool = std::is_enum<T>::value>
    struct TaggedCStrBuf;

    template<class TInt>
    constexpr std::size_t integral_buffer_size()
    { return std::numeric_limits<TInt>::digits10 + 1 + std::numeric_limits<TInt>::is_signed; }

    template<class T>
    struct TaggedCStrBuf<T, true, false>
    {
        static constexpr std::size_t size() { return integral_buffer_size<T>(); }
        char buf[integral_buffer_size<T>()+1];
        char * get() { return this->buf; }
    };

    template<class T>
    struct TaggedCStrBuf<T, false, true>
    : TaggedCStrBuf<typename std::underlying_type<T>::type, true, false>
    {};

    template<class T>
    struct TaggedCStrBuf<T, false, false>
    {};
}

template<class T>
struct CStrBuf : detail::TaggedCStrBuf<T>
{};

template<class T>
char const * c_str(CStrBuf<T>& , T const &);

template<class T>
CStrBuf<T> make_c_str_buf(T const & x) {
    CStrBuf<T> buf;
    c_str(buf, x);
    return buf;
}


template<class> struct spec_type {};

namespace spec_types
{
    template<unsigned> class fixed_binary;
    template<class T> class list;
    using ip = std::string;
    class path;
}

struct parse_error
{
    char const * s_err = nullptr;

    explicit operator bool () const { return this->s_err; }
    char const * c_str() const { return this->s_err; }
};

constexpr parse_error no_parse_error {nullptr};


parse_error parse(std::string & x, spec_type<std::string>, array_view_const_char value)
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

parse_error parse(std::string & x, spec_type<spec_types::list<std::string>>, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

namespace detail
{
    template<class IntOrigni, class IntStr>
    parse_error parse_list(
        std::string & x,
        array_view_const_char value,
        IntStr(*strtoi)(char const *, char**, int)
    ) {
        constexpr std::size_t sz_buf = detail::integral_buffer_size<IntOrigni>();
        char buf[sz_buf+1];
        for (auto r : get_split(value, ',')) {
            r = trim(r);
            char * end{};
            if (r.size() >= sz_buf) {
                return {"too large"};
            }
            memcpy(buf, r.begin(), r.size());
            buf[r.size()] = 0;
            IntStr val{strtoi(buf, &end, 10)};
            if (val > static_cast<IntStr>(std::numeric_limits<IntOrigni>::max())) {
                return {"too large"};
            }
            char const * cend = end;
            cend = std::find_if_not(cend, r.end(), [](char c) {
                return c == ' ' || c == '\t';
            });
            if (errno == ERANGE || cend != r.end()) {
                return {"bad format, expected \"integral(, integral)*\""};
            }
        }
        x.assign(value.data(), value.size());
        return no_parse_error;
    }
}

parse_error parse(std::string & x, spec_type<spec_types::list<unsigned>>, array_view_const_char value)
{ return detail::parse_list<unsigned>(x, value, strtoul); }

parse_error parse(std::string & x, spec_type<spec_types::list<int>>, array_view_const_char value)
{ return detail::parse_list<int>(x, value, strtoul); }

template<std::size_t N>
parse_error parse(char (&x)[N], spec_type<spec_types::path>, array_view_const_char value)
{
    static_assert(N > 4, "path length is too short");
    switch (value.size()) {
        case 0:
            x[0] = '.';
            x[1] = '/';
            x[2] = 0;
            return no_parse_error;
        case 1:
            x[0] = value[0];
            x[1] = 0;
            if (x[0] == '.') {
                x[1] = '/';
                x[2] = 0;
                return no_parse_error;
            }
            break;
        case 2:
            x[0] = value[0];
            x[1] = value[1];
            x[2] = 0;
            if (x[0] == '.' && x[1] == '.') {
                x[2] = '/';
                x[3] = 0;
                return no_parse_error;
            }
            break;
        default:;
    }

    std::size_t cp_sz = N - (value[0] == '/' ? 2 : 4);
    if (value.size() > cp_sz) {
        return {"too long"};
    }
    char * p = x;
    if (value[0] != '/') {
        *p++ = '.';
        *p++ = '/';
    }
    memcpy(p, value.data(), value.size());
    p += value.size();
    if (*(p-1) != '/') {
        *p++ = '/';
    }
    *p = 0;
    return no_parse_error;
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse(T & x, spec_type<bool>, array_view_const_char value)
{
    range<char const *> rng = trim(value);

    struct P {
        array_view_const_char s;
        bool val;
    };
    P cmp_list[]{
        {cstr_array_view("1"), true},
        {cstr_array_view("on"), true},
        {cstr_array_view("yes"), true},
        {cstr_array_view("true"), true},
        {cstr_array_view("0"), false},
        {cstr_array_view("no"), false},
        {cstr_array_view("false"), false},
    };
    for (P c : cmp_list) {
        if (c.s.size() == rng.size() && strncasecmp(c.s.data(), rng.begin(), c.s.size())) {
            x = c.val;
            return no_parse_error;
        }
    }
    return {"bad format, expected 1, on, yes, true, 0, no, false"};
}

template<class TInt>
typename std::enable_if<std::is_integral<TInt>::value && !std::is_same<TInt, bool>::value, parse_error>::type
parse(TInt & x, spec_type<TInt>, array_view_const_char value)
{
    range<char const *> rng = trim(value);

    constexpr std::size_t buf_sz = detail::integral_buffer_size<TInt>();
    char buf[buf_sz+1];
    if (rng.size() >= buf_sz) {
        return {"too large"};
    }
    if (rng.size() == 0) {
        x = 0;
        return no_parse_error;
    }

    char * end{};
    TInt val{};
    {
        std::size_t idx = 0;
        int base = 10;
        if (rng.size() > 2 && rng[0] == '0' && rng[1] == 'x') {
            idx = 2;
            base = 16;
        }

        memcpy(buf, rng.begin()+idx, rng.size()-idx);
        buf[rng.size()-idx] = 0;
        if (std::is_signed<TInt>::value) {
            val = strtoll(buf, &end, 16);
        }
        else {
            val = strtoull(buf, &end, 16);
        }
    }

    if (val > std::numeric_limits<TInt>::max()) {
        return {"too large"};
    }
    if (val < std::numeric_limits<TInt>::min()) {
        return {"too short"};
    }
    char const * cend = end;
    cend = std::find_if_not(cend, rng.end(), [](char c) {
        return c == ' ' || c == '\t';
    });
    if (errno == ERANGE || cend != rng.end()) {
        return {"bad format"};
    }

    x = val;
    return no_parse_error;
}

template<class T, class U>
parse_error parse_adapter(T & x, U & y)
{
    x = std::move(y);
    return no_parse_error;
}

template<class T, class U>
parse_error parse(T & x, spec_type<U> ty, array_view_const_char value)
{
    U tmp;
    if (parse_error err = parse(tmp, ty, value)) {
        return err;
    }
    return parse_adapter(x, tmp);
}

}

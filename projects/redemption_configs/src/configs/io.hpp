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

#include "utils/array_view.hpp"
#include "utils/splitter.hpp"


namespace configs {

namespace detail {
    template<class T, bool = std::is_integral<T>::value, bool = std::is_enum<T>::value>
    struct TaggedCStrBuf;

    template<class IntT>
    constexpr std::size_t integral_buffer_size()
    { return std::numeric_limits<IntT>::digits10 + 1 + std::numeric_limits<IntT>::is_signed; }

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
inline char const * c_str(CStrBuf<T>& , T const &);

template<class T>
CStrBuf<T> make_c_str_buf(T const & x) {
    CStrBuf<T> buf;
    c_str(buf, x);
    return buf;
}


template<class> struct spec_type {};

namespace spec_types {
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

constexpr parse_error no_parse_error = {nullptr};

parse_error parse(std::string & x, spec_type<std::string>, array_view_const_char value)
{
    x.assign(value.data(), value.size());
    return no_parse_error;
}

template<unsigned N>
parse_error parse(
    std::array<unsigned char, N> & key,
    spec_type<spec_types::fixed_binary<N>>,
    array_view_const_char value
) {
    if (value.size() != N*2) {
        return {"bad length"};
    }

    char   hexval[3] = { 0 };
    char * end{};
    for (std::size_t i = 0; i < N; i++) {
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
    template<class IntOrigni, class IntStr, class StrTo>
    parse_error parse_list(
        std::string & x,
        array_view_const_char value,
        IntStr(*strtoi)(char const *, char**, int)
    ) {
        char buf[detail::integral_buffer_size<IntOrigni>()+1];
        for (auto && r : get_split(value, ',')) {
            char * end{};
            IntStr val{strtoi(value, &end, 10)};
            if (val > static_cast<IntStr>(std::numeric_limits<IntOrigni>::max())) {
                return {"too large"};
            }
            end = std::find_if_not(end, r.end(), [](char c) {
                return c == ' ' || c == '\t';
            });
            if (errno == ERANGE || end != r.end()) {
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
            }
            return no_parse_error;
        case 2:
            x[0] = value[0];
            x[1] = value[1];
            x[2] = 0;
            if (x[0] == '.' && x[1] == '.') {
                x[2] = '/';
                x[3] = 0;
            }
            return no_parse_error;
        default: {
            if (value.size() > std::size_t(N - 2)) {
                return {"too long"};
            }
            memcpy(x, value.data(), value.size());
            x[value.size()+0] = '/';
            x[value.size()+1] = 0;
            return no_parse_error;
        }
    }
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse(T & x, spec_type<bool>, array_view_const_char value)
{
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
        if (c.s.size() == value.size() && strncasecmp(c.s.data(), value, c.s.size())) {
            x = c.val;
            return no_parse_error;
        }
    }
    return {"bad format, expected 1, on, yes, true, 0, no, false"};
}

template<class T>
typename std::enable_if<std::is_integral<T>::value, parse_error>::type
parse(T & x, spec_type<T>, array_view_const_char value)
{
    constexpr std::size_t buf_sz = detail::integral_buffer_size<T>();
    char buf[buf_sz+1];
    if (value.size() >= buf_sz) {
        return {"too large"};
    }
    if (value.size() == 0) {
        x = 0;
        return no_parse_error;
    }

    char * end{};
    T val{};
    {
        std::size_t idx = 0;
        int base = 10;
        if (value.size() > 2 && value[0] == '0' && value[1] == 'x') {
            idx = 2;
            base = 16;
        }

        memcpy(buf, value.data()+idx, value.size()-idx);
        buf[value.size()-idx] = 0;
        if (std::is_signed<T>::value) {
            val = strtoll(buf, end, 16);
        }
        else {
            val = strtoull(buf, end, 16);
        }
    }

    if (val > static_cast<IntStr>(std::numeric_limits<T>::max())) {
        return {"too large"};
    }
    if (val < static_cast<IntStr>(std::numeric_limits<T>::min())) {
        return {"too short"};
    }
    end = std::find_if_not(end, value.end(), [](char c) {
        return c == ' ' || c == '\t';
    });
    if (errno == ERANGE || end != value.end()) {
        return {"bad format"};
    }

    x = val;
    return no_parse_error;
}

template<class T, class U>
void parse_adapter(T & x, U & y)
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

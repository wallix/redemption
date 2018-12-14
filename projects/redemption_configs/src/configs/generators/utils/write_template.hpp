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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <iostream>
#include <utility>

#include <cstring>


namespace cfg_generators
{

template<class T>
struct binder
{
    char c;
    T x;
};

template<class Ints, class... Binders>
struct mapper_impl;

template<std::size_t, class Binder>
struct map_binder
{
    Binder binder;
};

template<std::size_t i, class Binder>
Binder const & get_binder(map_binder<i, Binder> const & mbinder)
{ return mbinder.binder; }

template<std::size_t... Ints, class... Binders>
struct mapper_impl<std::integer_sequence<std::size_t, Ints...>, Binders...>
: map_binder<Ints, Binders>...
{
    mapper_impl(Binders... binders) : map_binder<Ints, Binders>{binders}... {}
    mapper_impl(mapper_impl const &) = default;
    mapper_impl(mapper_impl &&) = default;

    template<class... Binders2>
    mapper_impl<
        std::make_index_sequence<sizeof...(Binders) + sizeof...(Binders2)>,
        Binders..., Binders2...
    >
    extends(Binders2 const & ... binders) const
    {
        return mapper_impl<
            std::make_index_sequence<sizeof...(Binders) + sizeof...(Binders2)>,
            Binders..., Binders2...
        >(get_binder<Ints>(*this)..., binders...);
    }

    template<std::size_t... Ints2, class... Binders2>
    mapper_impl<
        std::make_index_sequence<sizeof...(Binders) + sizeof...(Binders2)>,
        Binders..., Binders2...
    >
    merge(mapper_impl<std::integer_sequence<std::size_t, Ints2...>, Binders2...> const & other) const
    { return this->extends(get_binder<Ints2>(other)...); }
};

template<class... Binders>
using mapper = mapper_impl<std::make_index_sequence<sizeof...(Binders)>, Binders...>;


template<std::size_t... Ints, class... Binders>
void write_template(
    std::ostream & out,
    mapper_impl<std::integer_sequence<std::size_t, Ints...>, Binders...> const & map,
    char const * fmt)
{
    auto fmt_point = fmt;
    for (; *fmt; ++fmt) {
        if ('%' == *fmt && *(fmt+1)) {
            ++fmt;
            bool is_executed = false;
            (void)std::initializer_list<int>{(
                !is_executed && get_binder<Ints>(map).c == *fmt
                ? ((out.write(fmt_point, fmt - fmt_point - 1) << get_binder<Ints>(map).x),
                    fmt_point = fmt+1,
                    is_executed = true
                ) : 1
            )...};
            if (!is_executed) {
                out.write(fmt_point, fmt - fmt_point - 1);
                out.put(*fmt);
                fmt_point = fmt+1;
            }
        }
    }
    out.write(fmt_point, fmt - fmt_point);
}

template<class T>
binder<T const &> bind(char c, T const & x)
{ return {c, x}; }

template<class T>
binder<T> bind(char c, T && x)
{ return {c, x}; }

template<class... Binders>
mapper<Binders...> map(Binders... binders)
{ return {binders...}; }


struct io_prefix_lines
{
    char const * s;
    char const * prefix;
    char const * suffix;
    unsigned space;

    friend std::ostream & operator<<(std::ostream & out, io_prefix_lines const & comment)
    {
        if (auto s = comment.s) {
            auto const prefix_size = strlen(comment.prefix);
            while (*s) {
                auto s_point = s;
                for (; *s && '\n' != *s; ++s) {
                }
                out << std::setw(int(comment.space + prefix_size)) << comment.prefix;
                out.write(s_point, s - s_point);
                if (*s == '\n') {
                    ++s;
                }
                out << comment.suffix << '\n';
            }
        }
        return out;
    }
};

inline io_prefix_lines cpp_comment(char const * s, unsigned space) {
    return io_prefix_lines{s, "// ", "", space};
}

inline io_prefix_lines cpp_comment(std::string const & str, unsigned space) {
    return cpp_comment(str.c_str(), space);
}

inline io_prefix_lines cpp_doxygen_comment(char const * s, unsigned space) {
    return io_prefix_lines{s, "/// ", " <br/>", space};
}

inline io_prefix_lines cpp_doxygen_comment(std::string const & str, unsigned space) {
    return cpp_doxygen_comment(str.c_str(), space);
}

inline io_prefix_lines python_comment(char const * s, unsigned space) {
    return io_prefix_lines{s, "# ", "", space};
}

inline io_prefix_lines python_comment(std::string const & str, unsigned space) {
    return python_comment(str.c_str(), space);
}


struct io_chex
{
    char c;

    friend std::ostream & operator<<(std::ostream & out, io_chex const & h)
    {
        int
        c = (h.c >> 4);
        c += (c > 9) ? 'A' - 10 : '0';
        out << char(c);
        c = (h.c & 0xf);
        c += (c > 9) ? 'A' - 10 : '0';
        out << char(c);
        return out;
    }
};


struct io_hexkey
{
    char const * s;
    std::size_t n;
    char const * prefix = "";
    char const * suffix = "";

    friend std::ostream & operator<<(std::ostream & out, io_hexkey const & hk)
    {
        for (const char * k = hk.s, * e = k + hk.n; k != e; ++k) {
            out << hk.prefix << io_chex{*k} << hk.suffix;
        }
        return out;
    }
};


struct io_quoted2
{
    char const * s;

    io_quoted2(char const * s) : s(s) {}
    io_quoted2(std::string const & str) : s(str.c_str()) {}

    friend std::ostream & operator<<(std::ostream & out, io_quoted2 const & q)
    {
        if (auto s = q.s) {
            for (; *s; ++s) {
                if ('\'' == *s || '"' == *s || '\\' == *s) {
                    out << '\\';
                }
                out << *s;
            }
        }
        return out;
    }
};


struct io_replace
{
    char const * s;
    char from;
    char to;

    io_replace(char const * s, char from, char to) : s(s), from(from), to(to) {}
    io_replace(std::string const & str, char from, char to) : io_replace(str.c_str(), from, to) {}

    friend std::ostream & operator<<(std::ostream & out, io_replace const & r)
    {
        if (auto s = r.s) {
            for (; *s; ++s) {
                out << (r.from == *s ? r.to : *s);
            }
        }
        return out;
    }
};


struct io_four_string
{
    char const * s1;
    char const * s2;
    char const * s3;
    char const * s4;

    friend std::ostream & operator<<(std::ostream & out, io_four_string const & str4)
    { return out << str4.s1 << str4.s2 << str4.s3 << str4.s4; }
};


inline char const * string_or(char const * s, char const * default_)
{ return s ? s : default_; }


}

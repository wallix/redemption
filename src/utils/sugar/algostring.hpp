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

#include <iterator>
#include <algorithm>
#include <string>

#include "utils/sugar/range.hpp"
#include "utils/sugar/array_view.hpp"

struct is_blank_fn
{
    bool operator()(char c) const noexcept
    { return c == ' ' || c == '\t'; }
};

template<class FwIt, class Pred = is_blank_fn>
FwIt ltrim(FwIt first, FwIt last, Pred pred = Pred()) {
    return std::find_if_not(first, last, pred);
}

template<class BiIt, class Pred = is_blank_fn>
BiIt rtrim(BiIt first, BiIt last, Pred pred = Pred()) {
    using reverse_iterator = std::reverse_iterator<BiIt>;
    return std::find_if_not(reverse_iterator(last), reverse_iterator(first), pred).base();
}

template<class BiIt, class Pred = is_blank_fn>
range<BiIt> trim(BiIt first, BiIt last, Pred pred = Pred()) {
    first = ltrim(first, last, pred);
    return {first, rtrim(first, last, pred)};
}

template<class R, class Pred = is_blank_fn>
auto trim(R & r, Pred pred = Pred()) -> range<decltype(r.begin())> {
    using std::begin;
    using std::end;
    return trim(begin(r), end(r), pred);
}


namespace algostring
{
    struct is_ascii_double_quote_escapable_fn
    {
        constexpr bool operator()(char c) const noexcept
        {
            return c == '\\' || c == '"';
        }
    };


    template<class Pred>
    void append_escaped(std::string & escaped_subject, array_view_const_char subject, Pred && pred, char esc)
    {
        auto first = subject.begin();
        auto last = subject.end();

        auto p = first;
        while ((p = std::find_if(first, last, pred)) != last) {
            escaped_subject.append(first, p);
            escaped_subject += esc;
            escaped_subject += *p;
            first = p + 1;
        }

        escaped_subject.append(first, last);
    }

    template<class Pred>
    void append_escaped(std::string & escaped_subject, char const * subject, Pred && pred, char esc)
    {
        for (; *subject; ++subject) {
            if (pred(*subject)) {
                escaped_subject += esc;
            }
            escaped_subject += *subject;
        }
    }

    template<class Pred>
    std::pair<char *, char const *>
    append_escaped(
        char * escaped_subject, std::size_t size_max,
        char const * subject, Pred && pred, char esc
    ) {
        auto end = escaped_subject + size_max;
        for (; escaped_subject != end && *subject; ++subject) {
            if (pred(*subject)) {
                if (escaped_subject + 1 == end) {
                    break;
                }
                *escaped_subject++ = esc;
            }
            *escaped_subject++ = *subject;
        }
        return {escaped_subject, subject};
    }
}  // namespace algostring

inline void append_escaped_delimiters(std::string & escaped_subject, array_view_const_char subject)
{
    algostring::append_escaped(
        escaped_subject, subject,
        algostring::is_ascii_double_quote_escapable_fn{}, '\\');
}

inline void append_escaped_delimiters(std::string & escaped_subject, char const * subject)
{
    algostring::append_escaped(
        escaped_subject, subject,
        algostring::is_ascii_double_quote_escapable_fn{}, '\\');
}

inline std::pair<char *, char const *>
append_escaped_delimiters(char * escaped_subject, std::size_t size_max, char const * subject)
{
    return algostring::append_escaped(
        escaped_subject, size_max, subject,
        algostring::is_ascii_double_quote_escapable_fn{}, '\\'
    );
}

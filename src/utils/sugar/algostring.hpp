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

#include <cstring>

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


namespace detail
{
    inline array_view_const_char to_string_view_or_char(array_view_const_char av) noexcept
    {
        return av;
    }

    inline array_view_const_char to_string_view_or_char(char const* s) noexcept
    {
        return {s, ::strlen(s)};
    }

    inline char to_string_view_or_char(char c) noexcept
    {
        return c;
    }


    inline std::size_t len_from_av_or_char(array_view_const_char av) noexcept
    {
        return av.size();
    }

    inline std::size_t len_from_av_or_char(char c) noexcept
    {
        return sizeof(c);
    }


    inline void append_from_av_or_char(std::string& s, array_view_const_char av)
    {
        s.append(av.data(), av.size());
    }

    inline void append_from_av_or_char(std::string& s, char c)
    {
        s += c;
    }


    template<class... StringsOrChars>
    void str_concat_view(std::string& str, StringsOrChars&&... strs)
    {
        str.reserve(str.size() + (len_from_av_or_char(strs) + ...));
        (append_from_av_or_char(str, strs), ...);
    }
} // namespace detail


template<class String, class... Strings>
[[nodiscard]] std::string str_concat(String&& str, Strings const&... strs)
{
    std::string s;
    detail::str_concat_view(s, detail::to_string_view_or_char(str),
                               detail::to_string_view_or_char(strs)...);
    return s;
}

template<class... Strings>
[[nodiscard]] std::string str_concat(std::string&& str, Strings const&... strs)
{
    detail::str_concat_view(str, detail::to_string_view_or_char(strs)...);
    return std::move(str);
}


template<class... Strings>
void str_append(std::string& str, Strings const&... strs)
{
    detail::str_concat_view(str, detail::to_string_view_or_char(strs)...);
}


template<class... Strings>
void str_assign(std::string& str, Strings const&... strs)
{
    str.clear();
    detail::str_concat_view(str, detail::to_string_view_or_char(strs)...);
}

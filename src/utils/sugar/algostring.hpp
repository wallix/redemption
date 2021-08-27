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

#include "utils/sugar/array_view.hpp"

struct is_blank_fn
{
    bool operator()(char c) const noexcept
    {
        return c == ' ' || c == '\t';
    }
};

namespace detail
{
    template<class Pred>
    const char* ltrim(const char* first, const char* last, Pred&& pred)
    {
        while (first != last && pred(*first)) {
            ++first;
        }
        return first;
    }

    template<class Pred>
    const char* rtrim(const char* first, const char* last, Pred&& pred)
    {
        while (first != last && pred(*(last - 1))) {
            --last;
        }
        return last;
    }

    inline writable_chars_view chars_view_to_writable_chars_view(chars_view av) noexcept
    {
        char* first = const_cast<char*>(av.begin()); /*NOLINT*/
        char* last = const_cast<char*>(av.end()); /*NOLINT*/
        return writable_chars_view{first, last};
    }
}

template<class Pred = is_blank_fn>
chars_view ltrim(chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    return {detail::ltrim(chars.begin(), chars.end(), pred), chars.end()};
}

template<class Pred = is_blank_fn>
chars_view rtrim(chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    return {chars.begin(), detail::rtrim(chars.begin(), chars.end(), pred)};
}

template<class Pred = is_blank_fn>
chars_view trim(chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    auto first = detail::ltrim(chars.begin(), chars.end(), pred);
    auto last = detail::rtrim(first, chars.end(), pred);
    return {first, last};
}

template<class Pred = is_blank_fn>
writable_chars_view ltrim(writable_chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    return detail::chars_view_to_writable_chars_view(ltrim(chars_view(chars), pred));
}

template<class Pred = is_blank_fn>
writable_chars_view rtrim(writable_chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    return detail::chars_view_to_writable_chars_view(rtrim(chars_view(chars), pred));
}

template<class Pred = is_blank_fn>
writable_chars_view trim(writable_chars_view chars, Pred&& pred = Pred()) /*NOLINT*/
{
    return detail::chars_view_to_writable_chars_view(trim(chars_view(chars), pred));
}

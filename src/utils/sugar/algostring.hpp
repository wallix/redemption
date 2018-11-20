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

/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include <algorithm>

#include "utils/sugar/array.hpp"


template<class R1, class R2>
constexpr bool ranges_equal(R1&& lhs, R2&& rhs)
{
    return std::equal(utils::begin(lhs), utils::end(lhs), utils::begin(rhs), utils::end(rhs));
}

template<class R1, class R2, class Cmp>
constexpr bool ranges_equal(R1&& lhs, R2&& rhs, Cmp&& cmp)
{
    return std::equal(utils::begin(lhs), utils::end(lhs), utils::begin(rhs), utils::end(rhs), cmp);
}

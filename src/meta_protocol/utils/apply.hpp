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

#ifndef REDEMPTION_SRC_META_PROTOCOL_UTILS_APPLY_HPP
#define REDEMPTION_SRC_META_PROTOCOL_UTILS_APPLY_HPP

#include "../meta/integer_sequence.hpp"

#include <tuple>

namespace meta_protocol
{
    template<class T>
    std::make_index_sequence<std::tuple_size<T>::value>
    tuple_to_index_sequence(T const &) {
        return {};
    }

#if __cplusplus > 201103L
    template<class Tuple, class Fn, size_t... Ints>
    auto apply(std::index_sequence<Ints...>, Tuple && t, Fn fn) {
        using std::get;
        return fn(get<Ints>(t)...);
    }

    template<class Tuple, class Fn>
    auto apply(Tuple && t, Fn fn) {
        using std::get;
        return apply(tuple_to_index_sequence(t), std::forward<Tuple>(t), fn);
    }
#endif
}

#endif

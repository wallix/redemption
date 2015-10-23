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

#ifndef REDEMPTION_SRC_META_PROTOCOL_META_INTEGER_SEQUENCE_HPP
#define REDEMPTION_SRC_META_PROTOCOL_META_INTEGER_SEQUENCE_HPP


#if __cplusplus <= 201103L
#include <cstddef>
namespace meta_ {
    template<class T, T... Ints>
    struct integer_sequence {};

    template<std::size_t... Ints>
    using index_sequence = integer_sequence<std::size_t, Ints...>;

    namespace detail_ {
        template<std::size_t... Indexes>
        struct integer_sequence_ext
        {
            using next = integer_sequence_ext<Indexes..., sizeof...(Indexes)>;
            using seq = integer_sequence<std::size_t, Indexes...>;
        };

        template<std::size_t Num>
        struct make_integer_sequence_ext
        { using type = typename make_integer_sequence_ext<Num-1>::type::next; };

        template<>
        struct make_integer_sequence_ext<0>
        { using type = integer_sequence_ext<>; };

        /// Builds an parameter_index<0, 1, 2, ..., Num-1>.
        template<std::size_t Num>
        struct build_parameter_index
        { using type = typename make_integer_sequence_ext<Num>::type::seq; };

        template<class T, class Seq>
        struct type_sequence_converter;

        template<class T, class U, U... Ints>
        struct type_sequence_converter<T, integer_sequence<U, Ints...>>
        { using type = integer_sequence<T, T(Ints)...>; };
    }

    template<std::size_t N>
    using make_index_sequence = typename detail_::make_integer_sequence_ext<N>::type::seq;

    template<class T, T N>
    using make_integer_sequence = typename detail_::type_sequence_converter<
        T, make_index_sequence<std::size_t(N)>
    >::type;

    template<class... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}
namespace std {
    using ::meta_::integer_sequence;
    using ::meta_::index_sequence;
    using ::meta_::make_index_sequence;
    using ::meta_::make_integer_sequence;
    using ::meta_::index_sequence_for;
}
#else
#include <type_traits>
#endif

#endif

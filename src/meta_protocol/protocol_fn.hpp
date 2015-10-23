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

#ifndef REDEMPTION_SRC_META_PROTOCOL_PROTOCOL_FN_HPP
#define REDEMPTION_SRC_META_PROTOCOL_PROTOCOL_FN_HPP

#include "utils/apply.hpp"
#include "meta/static_const.hpp"

#include <tuple>
#include <type_traits>

namespace meta_protocol
{
    template<class Fn, class... T>
    struct lazy_fn
    {
        std::tuple<T...> t;
        Fn fn;

#if __cplusplus <= 201103L
        template<class Layout>
        void operator()(Layout && layout) const {
            this->apply(layout, tuple_to_index_sequence(this->t));
        }

    private:
        template<class Layout, size_t... Ints>
        void apply(Layout && layout, std::integer_sequence<size_t, Ints>) const {
            using std::get;
            this->fn(layout, get<Ints>(this->t)...);
        }
#else
        template<class Layout>
        void operator()(Layout && layout) const {
            apply(this->t, [&](auto && ... elems) {
                this->fn(layout, elems...);
            });
        }
#endif
    };

    template<class Fn, class... T>
    lazy_fn<Fn, T...> lazy(Fn fn, T && ... args) {
        return lazy_fn<Fn, T...>{std::tuple<T...>{std::forward<T>(args)...}, fn};
    }

    namespace detail_
    {
        template<class T, class U = void>
        struct enable_type
        { using type = U; };

        template<class T, class = void>
        struct is_layout_impl : std::false_type
        {};

        template<class T>
        struct is_layout_impl<T, typename enable_type<typename T::is_layout>::type> : T::is_layout
        {};
    }

    template<class T>
    using is_layout = detail_::is_layout_impl<typename std::remove_reference<T>::type>;

    template<class FnClass>
    struct protocol_wrapper
    {
        FnClass operator()() const {
            return {};
        }

        template<class T, class... Ts>
        typename std::enable_if<is_layout<T>::value>::type
        operator()(T && arg, Ts && ... args) const {
            FnClass()(arg, args...);
        }

        template<class T, class... Ts>
        typename std::enable_if<!is_layout<T>::value, lazy_fn<FnClass, T, Ts...>>::type
        operator()(T && arg, Ts && ... args) const {
            return {std::tuple<T, Ts...>{arg, args...}, FnClass{}};
        }
    };

    template<class FnClass>
    using protocol_fn = meta::static_const<protocol_wrapper<FnClass>>;
}

#endif

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

#ifndef REDEMPTION_GDI_PROXY_HPP
#define REDEMPTION_GDI_PROXY_HPP

#include <utility>
#include <type_traits>
#include <vector>
#include <functional>
#include "meta/meta.hpp"

namespace gdi {

template<class Api, class Proxy>
struct DispatcherProxy : Proxy
{
    std::vector<std::reference_wrapper<Api>> apis;

    using Proxy::Proxy;

    template<class T, class... Ts>
    void operator()(T &, Ts && ... args) {
        for (Api & api : this->apis) {
            Proxy::operator()(api, std::forward<Ts>(args)...);
        }
    }
};

struct DummyProxy {
    template<class ... Ts>
    void operator()(Ts const & ...) {
    }
};


template<class Proxy>
struct SkipBaseProxy : Proxy
{
    using Proxy::Proxy;

    template<class Base, class... Ts>
    void operator()(Base const &, Ts && ... args) {
        Proxy::operator()(std::forward<Ts>(args)...);
    }
};


namespace detail_
{
    template<class T, class = void>
    struct get_ref
    {
        using result_type = T &;
        static result_type impl(T & x) {
            return x;
        }
    };

    template<class Ptr>
    struct get_ref<Ptr, meta::void_t<decltype(*std::declval<Ptr&>())>>
    {
        using result_type = decltype(*std::declval<Ptr&>());
        static result_type impl(Ptr & x) {
            return *x;
        }
    };
}

template<class T, class Proxy>
struct ContextProxy : private Proxy
{
    ContextProxy() = default;
    ContextProxy(ContextProxy &&) = default;
    ContextProxy(ContextProxy const &) = default;

    template<class... Ts>
    ContextProxy(Ts && ... args)
    : val_{std::forward<Ts>(args)...}
    {}

    template<class... Ts>
    ContextProxy(Proxy && proxy, Ts && ... args)
    : Proxy(std::move(proxy))
    , val_{std::forward<Ts>(args)...}
    {}

    template<class... Ts>
    ContextProxy(Proxy const & proxy, Ts && ... args)
    : Proxy(proxy)
    , val_{std::forward<Ts>(args)...}
    {}

    template<class Base, class... Ts>
    void operator()(Base const &, Ts && ... args) {
        Proxy::operator()(detail_::get_ref<T>::impl(this->val_), std::forward<Ts>(args)...);
    }

private:
    T val_;
};

}

#endif

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


namespace gdi {

template<class Proxy, class InterfaceBase>
struct AdaptorBase : InterfaceBase, private Proxy
{
    using proxy_type = Proxy;
    using interface_base = InterfaceBase;

    AdaptorBase() = default;
    AdaptorBase(AdaptorBase const &) = delete;

    template<class... ProxyArgs>
    AdaptorBase(ProxyArgs && ... args)
    : Proxy{std::forward<ProxyArgs>(args)...}
    {}

    template<class... Ts>
    AdaptorBase(Proxy && proxy, Ts && ... args)
    : InterfaceBase{std::forward<Ts>(args)...}
    , Proxy(std::move(proxy))
    {}

    template<class... Ts>
    AdaptorBase(Proxy const & proxy, Ts && ... args)
    : InterfaceBase{std::forward<Ts>(args)...}
    , Proxy(proxy)
    {}

    proxy_type & get_proxy() noexcept { return static_cast<proxy_type&>(*this); }
    proxy_type const & get_proxy() const noexcept { return static_cast<proxy_type const&>(*this); }

protected:
    proxy_type & prox() noexcept { return static_cast<proxy_type&>(*this); }
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

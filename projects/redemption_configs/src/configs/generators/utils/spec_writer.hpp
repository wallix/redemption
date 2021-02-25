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

#include "configs/attributes/spec.hpp"
#include "configs/enumeration.hpp"
#include "utils/sugar/algostring.hpp"

#include <type_traits>
#include <string>
#include <memory>


namespace cfg_generators {

template<class... Ts>
struct pack_type : Ts...
{};


namespace detail
{
    template<class From, class To, class
      = decltype(static_cast<void(*)(To)>(nullptr)(*static_cast<From*>(nullptr)))>
    std::true_type is_convertible(int)
    { return {}; }

    template<class From, class To>
    std::false_type is_convertible(...)
    { return {}; }

    template<template<class...> class To, class... Ts>
    std::true_type is_t_convertible_impl_aux(To<Ts...> const&)
    { return {}; }

    template<template<class...> class To, class From>
    auto is_t_convertible_impl(From const& pack, int)
    -> decltype(is_t_convertible_impl_aux<To>(pack))
    { return {}; }

    template<template<class...> class To, class From>
    std::false_type is_t_convertible_impl(From const&, char)
    { return {}; }
}

template<class From, class To>
using is_convertible = decltype(detail::is_convertible<From, To>(0));

template<class From, class To>
constexpr bool is_convertible_v = is_convertible<From, To>::value;

template<class From, template<class...> class To>
using is_t_convertible = decltype(detail::is_t_convertible_impl<To>(*static_cast<From*>(nullptr), 1));

template<class From, template<class...> class To>
constexpr bool is_t_convertible_v = is_t_convertible<From, To>::value;

template<template<class...> class T, class... Ts>
T<Ts...> const& get_t_elem(T<Ts...> const& x)
{
    return x;
}

namespace detail_
{
    template<class T, class U>
    U const & get_default(cfg_attributes::type_<T>, cfg_attributes::default_<U> const * d)
    { return d->value; }

    template<class T>
    T const & get_default(cfg_attributes::type_<T>, ...)
    { static T r{}; return r; }
}

template<class T, class Pack>
auto const & get_default(cfg_attributes::type_<T> t, Pack const & infos)
{ return detail_::get_default<T>(t, &infos); }

template<template<class> class Default, class T, class Pack>
auto const & get_default(cfg_attributes::type_<T> t, Pack const & infos)
{
    if constexpr (is_t_convertible_v<Pack, Default>) {
        (void)t;
        return get_t_elem<Default>(infos).value;
    }
    else {
        return detail_::get_default<T>(t, &infos);
    }
}


template<class T, class Pack, class D>
decltype(auto) value_or(Pack const& pack, D&& default_)
{
    if constexpr (is_convertible_v<Pack, T>) {
        return static_cast<T const&>(pack);
    }
    else {
        return D(static_cast<D&&>(default_));
    }
}

template<template<class...> class T, class Pack>
auto get_type(Pack const & pack)
{
    if constexpr (is_t_convertible_v<Pack, T>) {
        return get_t_elem<T>(pack).to_type();
    }
    else {
        return get_t_elem<cfg_attributes::type_>(pack);
    }
}

template<class Pack>
std::string const& get_desc(Pack const & pack)
{
    static cfg_attributes::desc d{};
    return value_or<cfg_attributes::desc>(pack, d).value;
}


inline bool is_empty(char const * s) { return !*s; }
inline bool is_empty(std::string const & str) { return str.empty(); }
template<class T> bool is_empty(cfg_attributes::types::list<T> const &) { return true; }


struct no_spec_attr_t {};
struct no_sesman_io_t {};

struct spec_attr_t { cfg_attributes::spec::internal::attr value; };
struct sesman_io_t { cfg_attributes::sesman::internal::io value; };
struct log_policy_t { cfg_attributes::spec::log_policy value; };
struct connection_policy_t : sesman_io_t
{
    // Should be `std::vector<std::string> files`, but does not work with g++ + debug
    //   internal compiler error: in assign_temp, at function.c:984
    // 287 |         const infos_type infos{detail_::normalize_info_arg(args)...};
    std::vector<std::string>* files;
    cfg_attributes::connpolicy::internal::attr spec;

    connection_policy_t(cfg_attributes::sesman::connection_policy const& conn)
    : sesman_io_t{::cfg_attributes::sesman::internal::io::sesman_to_proxy}
    , files(new std::vector<std::string>{conn.files.begin(), conn.files.end()})
    , spec(conn.spec)
    {}

    // internal compiler error too...
    // connection_policy_t(connection_policy_t const&) = delete;

    // ~connection_policy_t()
    // {
    //     delete files;
    // }
};

namespace detail_
{
    template<class T>
    auto normalize_info_arg(T const& x)
    {
        if constexpr (is_convertible_v<T, cfg_attributes::spec::internal::attr>) {
            static_assert(!std::is_same<cfg_attributes::spec::internal::attr, T>::value, "Has a direct spec::attr value");
            if constexpr (T::value == cfg_attributes::spec::internal::attr::no_ini_no_gui) {
                return no_spec_attr_t{};
            }
            else {
                return spec_attr_t{T::value};
            }
        }
        else if constexpr (is_convertible_v<T, cfg_attributes::sesman::internal::io>) {
            static_assert(!std::is_same<cfg_attributes::sesman::internal::io, T>::value, "Has a direct sesman::io value");
            if constexpr (T::value == cfg_attributes::sesman::internal::io::none) {
                return no_sesman_io_t{};
            }
            else {
                return sesman_io_t{T::value};
            }
        }
        else if constexpr (is_convertible_v<T, cfg_attributes::spec::log_policy>) {
            return log_policy_t{x};
        }
        else if constexpr (is_convertible_v<T, cfg_attributes::sesman::connection_policy>) {
            return connection_policy_t{x};
        }
        else {
            return T(x);
        }
    }

    inline void check_names(
        cfg_attributes::names const& names,
        bool has_ini, bool has_sesman, bool has_connpolicy)
    {
        if (!has_ini && !names.ini.empty())
            throw std::runtime_error("names.ini without ini for " + names.cpp);

        if (!has_sesman && !names.sesman.empty())
            throw std::runtime_error("names.sesman without sesman for " + names.cpp);

        if (has_sesman && !names.connpolicy.empty())
            throw std::runtime_error("names.connpolicy with sesman for " + names.cpp);

        if (!has_connpolicy && !names.connpolicy.empty())
            throw std::runtime_error("names.connpolicy without connection_policy for " + names.cpp);

        if (has_connpolicy && !names.sesman.empty())
            throw std::runtime_error("names.sesman with connection_policy for " + names.cpp);
    }
}

using Names = cfg_attributes::names;

template<class Dispatch>
struct ConfigSpecWrapper
{
    type_enumerations enums;
    Dispatch dispatch;

    ConfigSpecWrapper(Dispatch dispatch)
    : dispatch(dispatch)
    {}

private:
    Names current_section_names {};

public:
    template<class Fn>
    void section(Names&& names, Fn fn)
    {
        current_section_names = std::move(names);
        assert(current_section_names.sesman.empty());

        dispatch([&](auto&... writer){
            (..., writer.do_start_section(current_section_names));
        });

        fn();

        dispatch([&](auto&... writer){
            (..., writer.do_stop_section(current_section_names));
        });
    }

    template<class Fn>
    void section(char const* name, Fn fn)
    {
        section(Names{name}, fn);
    }

    template<class... Ts>
    void member(Ts const& ... args)
    {
        static_assert((is_convertible_v<Ts, cfg_attributes::spec::internal::attr> || ...),
            "spec::attr is missing");
        constexpr bool has_conn_policy = (is_convertible_v<Ts, cfg_attributes::sesman::connection_policy> || ...);
        constexpr bool has_sesman_io = (is_convertible_v<Ts, cfg_attributes::sesman::internal::io> || ...);
        static_assert(has_conn_policy || has_sesman_io,
            "sesman::io or connection_policy are missing");
        static_assert(!(has_conn_policy && has_sesman_io),
            "has sesman::io with connection_policy");
        static_assert((std::is_same_v<Ts, cfg_attributes::spec::log_policy> || ...),
            "spec::log_policy is missing");
        static_assert(
            !has_conn_policy || ((
                is_convertible_v<Ts, decltype(cfg_attributes::spec::constants::no_ini_no_gui)>
             || is_convertible_v<Ts, decltype(cfg_attributes::spec::constants::hidden_in_gui)>
            ) || ...),
            "sesman::connection_policy only with:\n- spec::constants::no_ini_no_gui\n- spec::constants::hidden_in_gui\n- connpolicy::allow_connpolicy_and_gui");
        constexpr bool is_target_context = (is_convertible_v<Ts, cfg_attributes::sesman::internal::is_target_context> || ...);
        static_assert(has_conn_policy ? !is_target_context : is_target_context == !((
            is_convertible_v<Ts, decltype(cfg_attributes::sesman::constants::no_sesman)>
         )|| ...), "sesman::is_target_context is missing or specified with no_sesman");

        using infos_type = pack_type<decltype(detail_::normalize_info_arg(args))...>;
        const infos_type infos{detail_::normalize_info_arg(args)...};

        detail_::check_names(infos,
            !(... || is_convertible_v<Ts, decltype(cfg_attributes::spec::constants::no_ini_no_gui)>),
            has_sesman_io, has_conn_policy);

        dispatch([&](auto&... writer){
            (..., writer.evaluate_member(
                current_section_names,
                infos,
                this->enums
            ));
        });
    }
};

}

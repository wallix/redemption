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

#include <stdexcept>
#include <type_traits>
#include <string>
#include <unordered_map>
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

template<class T>
T const& get_elem(T const& x)
{
    return x;
}

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
        return get_elem<T>(pack);
    }
    else {
        return D(static_cast<D&&>(default_));
    }
}

template<class T, class Pack>
std::string const& get_name(Pack const & pack)
{
    if constexpr (is_convertible_v<Pack, T>) {
        return get_elem<T>(pack).name;
    }
    else {
        return get_elem<cfg_attributes::name_>(pack).name;
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
struct connection_policy_t : sesman_io_t {
    char const* file;
    cfg_attributes::connpolicy::internal::attr spec;
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
            return connection_policy_t{
                {cfg_attributes::sesman::internal::io::sesman_to_proxy},
                x.file,
                x.spec
            };
        }
        else if constexpr (is_convertible_v<T, char const*>) {
            return cfg_attributes::name_{x};
        }
        else {
            return T(x);
        }
    }

    template<class... Ts>
    struct check_name_type
    {
        template<class T>
        static constexpr bool is_name = (std::is_same_v<Ts, T> || ...);
    };

    using name_type_checker = cfg_attributes::names::f<check_name_type>;

    template<class T>
    auto normalize_name(T const& x)
    {
        if constexpr (is_convertible_v<T, char const*>) {
            return cfg_attributes::name_{x};
        }
        else {
            static_assert(name_type_checker::template is_name<T>, "not a name type");
            return T(x);
        }
    }

    template<class BaseName, class... Name>
    struct Names_
    {
        template<class Pack>
        explicit Names_(Pack const& pack)
        : names{
            static_cast<BaseName const&>(pack).name,
            value_or<Name>(pack, Name{}).name...
        }
        {}

        void merge(Names_ const& other)
        {
            for (std::size_t i = 0; i < std::size(names); ++i) {
                if (!other.names[i].empty()) {
                    if (names[i].empty()) {
                        names[i] = other.names[i];
                    }
                    else if (names[i] != other.names[i]) {
                        std::string msg = "name redefined: '";
                        msg += names[i];
                        msg += "' and '";
                        msg += other.names[i];
                        msg += "'";
                        throw std::runtime_error(msg);
                    }
                }
            }
        }

        template<class T>
        std::string const& name() const
        {
            std::string const* p = &names[0];
            std::size_t i = 1;
            (void)(((std::is_same_v<T, Name> && ((void)(
                !names[i].empty() && ((void)(p = &names[i]), true)
            ), true)) || !++i) || ...);
            return *p;
        }

        std::array<std::string, 1+sizeof...(Name)> names;
    };
}


template<class... Writers>
struct ConfigSpecWrapper
{
    type_enumerations enums;

private:
    struct InfosBase
    {
        virtual void apply(type_enumerations& enums, std::pair<Writers&, std::string>&... pws) = 0;
        virtual ~InfosBase() = default;
    };

    template<class... Ts>
    struct Infos final : InfosBase
    {
        template<class... Us>
        Infos(Us&& ... args)
        : infos{static_cast<Us&&>(args)...}
        {}

        void apply(type_enumerations& enums, std::pair<Writers&, std::string>&... pws) override
        {
            (pws.first.evaluate_member(pws.second, this->infos, enums), ...);
        }

        pack_type<Ts...> infos;
    };

    using Names = cfg_attributes::names::f<detail_::Names_>;

    struct Sections
    {
        Names names;
        std::vector<std::string> members_ordered {};
        std::unordered_map<std::string, std::unique_ptr<InfosBase>> members {};
    };
    std::unordered_map<std::string, Sections> sections;
    std::vector<std::string> sections_ordered;

    Sections* section_;


    template<class... Ts>
    static Names names_impl(Ts&&... s)
    {
        return Names{pack_type<Ts...>{static_cast<Ts&&>(s)...}};
    }

public:
    template<class... Ts>
    static Names names(Ts const&... s)
    {
        return names_impl(detail_::normalize_name(s)...);
    }

    template<class Fn>
    void section(Names names, Fn fn)
    {
        auto const& name = names.names[0];
        auto it = this->sections.find(name);
        if (it == this->sections.end()) {
            this->sections_ordered.push_back(name);
            it = this->sections.emplace(name, Sections{names}).first;
        }
        else {
            it->second.names.merge(names);
        }
        this->section_ = &it->second;
        fn();
    }

    template<class Fn>
    void section(char const* name, Fn fn)
    {
        section(names(name), fn);
    }

    template<class... Ts>
    void member(Ts const& ... args)
    {
        static_assert((is_convertible_v<Ts, cfg_attributes::spec::internal::attr> || ...),
            "spec::attr is missing");
        static_assert((
            (is_convertible_v<Ts, cfg_attributes::sesman::internal::io>
          || is_convertible_v<Ts, cfg_attributes::sesman::connection_policy>) || ...),
            "sesman::io or connection_policy are missing");
        static_assert((
            !((!is_convertible_v<Ts, cfg_attributes::sesman::internal::io>
            && !is_convertible_v<Ts, cfg_attributes::sesman::connection_policy>) && ...)),
            "has sesman::io and connection_policy");
        static_assert((std::is_same_v<Ts, cfg_attributes::spec::log_policy> || ...),
            "spec::log_policy is missing");
        constexpr bool has_conn_policy = (is_convertible_v<Ts, cfg_attributes::sesman::connection_policy> || ...);
        static_assert(
            !has_conn_policy || ((
                is_convertible_v<Ts, decltype(cfg_attributes::spec::constants::no_ini_no_gui)>
             || is_convertible_v<Ts, decltype(cfg_attributes::spec::constants::hidden_in_gui)>
            ) || ...),
            "connection_policy only with no_ini_no_gui or hidden_in_gui");

        using infos_type = Infos<decltype(detail_::normalize_info_arg(args))...>;
        std::unique_ptr<infos_type> u(new infos_type{detail_::normalize_info_arg(args)...});

        std::string const& varname = get_elem<cfg_attributes::name_>(u->infos).name;
        auto it = this->section_->members.find(varname);
        if (it != this->section_->members.end()) {
            throw std::runtime_error("duplicates member '" + varname +
                "' in section '" + this->section_->names.names[0] + "'");
        }
        this->section_->members_ordered.push_back(varname);
        this->section_->members.emplace(varname, std::move(u));
    }

    int evaluate(Writers&... ws)
    {
        (ws.do_init(), ...);
        for (std::string const & global_section_name : this->sections_ordered) {
            auto const & section = this->sections.find(global_section_name)->second;

            pack_type<std::pair<Writers&, std::string>...> pws{
                {ws, section.names.template name<
                    typename std::remove_reference_t<decltype(ws)>::attribute_name_type>()
                }...
            };

            (ws.do_start_section(static_cast<std::pair<Writers&, std::string>&>(pws).second), ...);
            for (std::string const & member_name : section.members_ordered) {
                section.members.find(member_name)->second
                    ->apply(this->enums, static_cast<std::pair<Writers&, std::string>&>(pws)...);
            }
            (ws.do_stop_section(static_cast<std::pair<Writers&, std::string>&>(pws).second), ...);
        }

        auto error_code = 0;
        auto check_final = [&](auto& w){
            if (int err = w.do_finish()) {
                error_code = error_code ? error_code : err;
            }
        };
        (check_final(ws), ...);
        return error_code;
    }
};


template<class Inherit>
struct EvaluatorBase
{
    Inherit & inherit() { return static_cast<Inherit&>(*this); }


    template<class T>
    void write(T const & x) { this->inherit().do_write(x); }

    template<class T, class... Ts>
    void write(T const & x, pack_type<Ts...> const & pack)
    { this->inherit().do_write(x, pack); }

private:
    void do_start_section(std::string const & section_name) { (void)section_name; }
    void do_stop_section(std::string const & section_name) { (void)section_name; }
    void do_init() {}
    void do_finish() {}
};

}

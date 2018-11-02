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

#include <type_traits>
#include <string>
#include <unordered_map>
#include <memory>


namespace cfg_generators {

template<class T>
struct val
{
    T const x;
    operator T const & () const { return this->x; }
};

template<class... Ts>
struct pack_type : val<Ts>...
{
    explicit pack_type(Ts const &... x)
    : val<Ts>{x}...
    {}
};


namespace detail_
{
    template<class...> using void_t = void;

    template<class T>
    T const & unwrap(T const & val)
    { return val; }

    template<class Tag, class T>
    T const & unwrap(cfg_attributes::bind_<Tag, T> const & val)
    { return val.binded; }


    template<class T>
    auto pack_get(val<T> const & val, int)
    -> decltype(unwrap(val.x))
    { return unwrap(val.x); }

    template<class T, class Pack>
    std::enable_if_t<!std::is_base_of<val<T>, Pack>::value, typename T::bind_type const &>
    pack_get(Pack const & pack, char)
    { return static_cast<typename T::bind_type const &>(pack); }


    template<template<class> class Tpl, class T>
    auto pack_get_tpl_val(val<Tpl<T>> const & val)
    -> decltype(unwrap(val.x))
    { return unwrap(val.x); }

    template<template<class> class Tpl, class T>
    auto pack_get_tpl(T const & val, int)
    -> decltype(pack_get_tpl_val<Tpl>(val))
    { return pack_get_tpl_val<Tpl>(val); }

    template<class T> struct pack_get_binded_tpl;

    template<class Tag, template<class> class Tpl, class T>
    struct pack_get_binded_tpl<cfg_attributes::bind_<Tag, Tpl<T>>>
    {
        template<class U>
        static Tpl<U> const & pack_get(val<Tpl<U>> const & val)
        { return val.x; }
    };

    template<template<class> class Tpl, class T>
    auto pack_get_tpl(T const & val, char)
    -> decltype(pack_get_binded_tpl<Tpl<int>>::pack_get(val))
    { return pack_get_binded_tpl<Tpl<int>>::pack_get(val); }
}


template<class T, class...Ts>
auto pack_get(pack_type<Ts...> const & pack)
-> decltype(detail_::pack_get<T>(pack, 1))
{ return detail_::pack_get<T>(pack, 1); }

template<template<class> class Tpl, class...Ts>
auto pack_get(pack_type<Ts...> const & pack)
-> decltype(detail_::pack_get_tpl<Tpl>(pack, 1))
{ return detail_::pack_get_tpl<Tpl>(pack, 1); }


namespace detail_
{
    template<class T, class Pack>
    auto pack_contains(Pack const & pack, int)
    -> decltype(void(pack_get<T>(pack)), std::true_type{})
    { return {}; }

    template<class T, class Pack>
    std::false_type pack_contains(Pack const &, char)
    { return {}; }

    template<template<class> class Tpl, class Pack>
    auto pack_contains_tpl(Pack const & pack, int)
    -> decltype(void(pack_get<Tpl>(pack)), std::true_type{})
    { return {}; }

    template<template<class> class Tpl, class Pack>
    std::false_type pack_contains_tpl(Pack const &, char)
    { return {}; }
}

template<class T, class...Ts>
auto pack_contains(pack_type<Ts...> const & pack)
-> decltype(detail_::pack_contains<T>(pack, 1))
{ return detail_::pack_contains<T>(pack, 1); }

template<template<class> class Tpl, class...Ts>
auto pack_contains(pack_type<Ts...> const & pack)
-> decltype(detail_::pack_contains_tpl<Tpl>(pack, 1))
{ return detail_::pack_contains_tpl<Tpl>(pack, 1); }


inline bool is_empty(char const * s) { return !*s; }
inline bool is_empty(std::string const & str) { return str.empty(); }
template<class T> bool is_empty(cfg_attributes::types::list<T> const &) { return true; }


namespace detail_
{
    template<class T, class U>
    U const & get_default(cfg_attributes::type_<T>, val<cfg_attributes::default_<U>> const * d)
    { return d->x.value; }

    template<class T>
    T const & get_default(cfg_attributes::type_<T>, ...)
    { static T r{}; return r; }
}

template<class T, class Pack>
auto const & get_default(cfg_attributes::type_<T> t, Pack const & infos)
{ return detail_::get_default<T>(t, &infos); }


namespace detail_
{
    template<class To, class Pack, class Default>
    To const & value_or(std::true_type, Pack const & pack, Default const &)
    { return pack_get<To>(pack); }

    template<class To, class Pack, class Default>
    To const & value_or(std::false_type, Pack const &, Default const & default_)
    { return default_; }

    template<template<class> class To, class Pack, class Default>
    auto const & value_or(std::true_type, Pack const & pack, Default const &)
    { return pack_get<To>(pack); }

    template<template<class> class To, class Pack, class Default>
    Default const & value_or(std::false_type, Pack const &, Default const & default_)
    { return default_; }
}

template<class T, class...Ts>
T const & value_or(pack_type<Ts...> const & pack, T const & default_)
{ return detail_::value_or<T>(pack_contains<T>(pack), pack, default_); }

template<class T, class...Ts, class Default>
T const & value_or(pack_type<Ts...> const & pack, Default const & default_)
{ return detail_::value_or<T>(pack_contains<T>(pack), pack, default_); }

template<template<class> class T, class...Ts, class Default>
auto const & value_or(pack_type<Ts...> const & pack, Default const & default_)
{ return detail_::value_or<T>(pack_contains<T>(pack), pack, default_); }


namespace detail_
{
    template<class To, class Pack, class Fn, class... Args>
    void apply_if_contains(std::true_type, Pack const & pack, Fn & fn, Args const & ... args)
    { fn(pack_get<To>(pack), args...); }

    template<class To, class Pack, class Fn, class... Args>
    void apply_if_contains(std::false_type, Pack const &, Fn &, Args const & ...)
    {}

    template<template<class> class To, class Pack, class Fn, class... Args>
    void apply_if_contains(std::true_type, Pack const & pack, Fn & fn, Args const & ... args)
    { fn(pack_get<To>(pack), args...); }

    template<template<class> class To, class Pack, class Fn, class... Args>
    void apply_if_contains(std::false_type, Pack const &, Fn &, Args const & ...)
    {}
}

template<class T, class... Ts, class Fn, class... Args>
void apply_if_contains(pack_type<Ts...> const & pack, Fn fn, Args const & ... args)
{ detail_::apply_if_contains<T>(pack_contains<T>(pack), pack, fn, args...); }

template<template<class> class T, class... Ts, class Fn, class... Args>
void apply_if_contains(pack_type<Ts...> const & pack, Fn fn, Args const & ... args)
{ detail_::apply_if_contains<T>(pack_contains<T>(pack), pack, fn, args...); }


namespace detail_
{
    struct no_spec_attr_t {};
    struct no_sesman_io_t {};

    template<class T>
    inline auto normalize_info_arg(T const& x)
    {
        if constexpr (std::is_convertible_v<T, cfg_attributes::spec::internal::attr>) {
            static_assert(!std::is_same<cfg_attributes::spec::internal::attr, T>::value, "Has a direct spec::attr value");
            if constexpr (T::value == cfg_attributes::spec::internal::attr::no_ini_no_gui) {
                return no_spec_attr_t{};
            }
            else {
                return T::value;
            }
        }
        else if constexpr (std::is_convertible_v<T, cfg_attributes::sesman::internal::io>) {
            static_assert(!std::is_same<cfg_attributes::sesman::internal::io, T>::value, "Has a direct sesman::io value");
            if constexpr (T::value == cfg_attributes::sesman::internal::io::none) {
                return no_sesman_io_t{};
            }
            else {
                return T::value;
            }
        }
        else if constexpr (std::is_convertible_v<T, char const*>) {
            return cfg_attributes::name_{x};
        }
        else {
            return T(x);
        }
    }
}


template<class T, class U = void>
using enable_if_enum_t = typename std::enable_if<std::is_enum<T>::value, U>::type;
template<class T, class U = void>
using disable_if_enum_t = typename std::enable_if<!std::is_enum<T>::value, U>::type;


template<class Inherit, class AttributeName>
struct ConfigSpecWriterBase
{
    type_enumerations enums;

    unsigned depth = 0;

private:
    struct InfosBase
    {
        virtual void apply(
            std::string const & section_name,
            std::string const & member_name,
            Inherit & x
        ) = 0;
        virtual ~InfosBase() = default;
    };

    template<class... Ts>
    struct Infos final : InfosBase
    {
        Infos(Ts const & ... args)
        : infos(args...)
        {}

        void apply(
            std::string const & section_name,
            std::string const & member_name,
            Inherit & x
        ) override
        { x.do_member(section_name, member_name, this->infos); }

        pack_type<Ts...> infos;
    };

    struct Sep final : InfosBase
    {
        void apply(std::string const &, std::string const &, Inherit & x) override
        { x.do_sep(); }
    };

    struct Members
    {
        std::vector<std::string> members_ordered;
        std::unordered_map<std::string, std::unique_ptr<InfosBase>> members;
    };
    std::unordered_map<std::string, Members> sections;
    std::vector<std::string> sections_ordered;

    Members * section_;

public:
    Inherit & inherit() { return static_cast<Inherit&>(*this); }
    void sep() { this->section_->members_ordered.emplace_back(); }

    template<class Fn>
    void section(std::string name, Fn fn)
    {
        auto it = this->sections.find(std::move(name));
        if (it == this->sections.end()) {
            this->sections_ordered.push_back(name);
            it = this->sections.emplace(std::move(name), Members{}).first;
        }
        this->section_ = &it->second;
        fn();
    }

    template<class... Ts>
    void member(Ts const& ... args)
    {
        static_assert((std::is_convertible_v<Ts, cfg_attributes::spec::internal::attr> || ...),
            "spec::attr is missing");
        static_assert((std::is_convertible_v<Ts, cfg_attributes::sesman::internal::io> || ...),
            "sesman::io is missing");
        static_assert((std::is_same_v<Ts, cfg_attributes::spec::log_policy> || ...),
            "spec::log_policy is missing");

        using infos_type = Infos<decltype(detail_::normalize_info_arg(args))...>;
        std::unique_ptr<infos_type> u(new infos_type{detail_::normalize_info_arg(args)...});

        std::string varname = pack_get<AttributeName>(u->infos);
        auto it = this->section_->members.find(varname);
        if (it == this->section_->members.end()) {
            this->section_->members_ordered.push_back(varname);
            this->section_->members.emplace(std::move(varname), std::move(u));
        }
        else {
            it->second = std::move(u);
        }
    }

    void evaluate()
    {
        this->inherit().do_init();
        for (std::string const & section_name : this->sections_ordered) {
            auto const & section = this->sections.find(section_name)->second;
            this->inherit().do_start_section(section_name);
            for (std::string const & member_name : section.members_ordered) {
                if (member_name.empty()) {
                    this->inherit().do_sep();
                }
                else {
                    section.members.find(member_name)->second
                    ->apply(section_name, member_name, this->inherit());
                }
            }
            this->inherit().do_stop_section(section_name);
        }
        this->inherit().do_finish();
    }

    template<class T>
    void write(T const & x) { this->inherit().do_write(x); }

    template<class T, class... Ts>
    void write(T const & x, pack_type<Ts...> const & pack)
    { this->inherit().do_write(x, pack); }

private:
    void do_start_section(std::string const & section_name) { (void)section_name; }
    void do_stop_section(std::string const & section_name) { (void)section_name; }
    void do_sep() {}
    void do_init() {}
    void do_finish() {}
};

}

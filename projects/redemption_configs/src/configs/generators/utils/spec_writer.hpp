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
#include <sstream>
#include <string>
#include <map>


namespace cfg_generators {

template<class T>
struct ref
{
    T const & x;
    operator T const & () const { return this->x; }
};

template<class T>
struct ref<T*>
{
    T const * const x;
    operator T const * const & () const { return this->x; }
};

template<class... Ts>
struct pack_type : ref<std::decay_t<Ts>>...
{
    explicit pack_type(Ts const &... x)
    : ref<std::decay_t<Ts>>{x}...
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
    auto pack_get(ref<T> const & val, int)
    -> decltype(unwrap(val.x))
    { return unwrap(val.x); }

    template<class T, class Pack>
    std::enable_if_t<!std::is_base_of<ref<T>, Pack>::value, typename T::bind_type const &>
    pack_get(Pack const & pack, char)
    { return static_cast<typename T::bind_type const &>(pack); }


    template<template<class> class Tpl, class T>
    auto pack_get_tpl_val(ref<Tpl<T>> const & val)
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
        static Tpl<U> const & pack_get(ref<Tpl<U>> const & val)
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


bool is_empty(char const * s) { return !*s; }
bool is_empty(std::string const & str) { return str.empty(); }
template<class T> bool is_empty(cfg_attributes::types::list<T> const &) { return true; }


template<class T, class U>
U const & get_default(cfg_attributes::type_<T>, ref<cfg_attributes::default_<U>> const & d)
{ return d.x.value; }

template<class T>
T const & get_default(cfg_attributes::type_<T>, ...)
{ static T r; return r; }


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


template<class T, class U = void>
using enable_if_enum_t = typename std::enable_if<std::is_enum<T>::value, U>::type;
template<class T, class U = void>
using disable_if_enum_t = typename std::enable_if<!std::is_enum<T>::value, U>::type;


template<class Inherit>
struct ConfigSpecWriterBase
{
    std::string section_name;
    unsigned depth = 0;
    std::ostringstream out_member_;
    std::ostream * out_;
    type_enumerations enums;

    std::map<std::string, std::string> sections_member;

    std::ostream & out() {
        return *this->out_;
    }

    template<class Fn>
    void section(std::string name, Fn fn) {
        this->out_ = &this->out_member_;
        this->section_name = std::move(name);
        if (!this->section_name.empty()) {
            ++this->depth;
        }
        this->inherit().do_start_section();

        fn();

        this->out_ = &this->out_member_;
        if (!this->section_name.empty()) {
            --this->depth;
        }
        this->sections_member[this->section_name] += this->out_member_.str();
        this->out_member_.str("");
        this->inherit().do_stop_section();
    }

    void sep() { this->inherit().do_sep(); }
    void tab() { this->inherit().do_tab(); }

    template<class T>
    void write(T const & x) { this->inherit().do_write(x); }

    template<class T, class... Ts>
    void write(T const & x, pack_type<Ts...> const & pack)
    { this->inherit().do_write(x, pack); }

    template<class To, class... Ts>
    void write_if_contains(pack_type<Ts...> const & pack)
    { apply_if_contains<To>(pack, [this, &pack](auto&& val){ this->write(val, pack); }); }

    template<template<class> class To, class... Ts>
    void write_if_contains(pack_type<Ts...> const & pack)
    { apply_if_contains<To>(pack, [this, &pack](auto&& val){ this->write(val, pack); }); }

public:
    void write_key(char const * k, std::size_t n, char const * prefix = "") {
        int c;
        for (const char * e = k + n; k != e; ++k) {
            this->out() << prefix;
            c = (*k >> 4);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
            c = (*k & 0xf);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
        }
    }

protected:
    Inherit & inherit() {
        return static_cast<Inherit&>(*this);
    }

private:
    void do_start_section() {}
    void do_stop_section() {}
    void do_sep() {}
    void do_tab() {}
};

}

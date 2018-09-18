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

#include <cstddef>

#include <string>

#if __has_include(<linux/limits.h>)
# include <linux/limits.h>
namespace cfg_attributes {
namespace globals {
    constexpr std::size_t path_max = PATH_MAX;
}
}
#else
namespace cfg_attributes {
namespace globals {
    constexpr std::size_t path_max = 4096;
}
}
#endif


namespace cfg_attributes
{

using name_ = char const *;

template<class T>
struct type_ { using type = T; };

template<class T>
struct default_
{
    using type = T;
    T value;
};

template<class T>
default_<T> set(T const & x)
{ return {x}; }

template<std::size_t N>
default_<std::string> set(char const (&x)[N])
{ return {std::string(x+0, x+N-1)}; }

struct desc { std::string value; };

struct prefix_value { char const * value; };



template<class Tag, class T>
struct bind_
{
    using tag_type = Tag;
    using bind_type = T;
    T binded;
};

template<template<class> class>
struct tpl_t {};


namespace types
{
    struct integer_base {};
    struct signed_base : integer_base {};
    struct unsigned_base : integer_base {};

    struct u32 : unsigned_base { u32(long long = 0) {} };
    struct u64 : unsigned_base { u64(long long = 0) {} };

    template<unsigned Len> struct fixed_string {};
    template<unsigned Len> struct fixed_binary {};

    template<class T>
    struct list {};

    struct ip_string {};

    struct dirpath { using fixed_type = fixed_string<globals::path_max>; };

    template<class T, long min, long max> struct range {};
}


namespace cpp
{
    using name = bind_<class name_tag, ::cfg_attributes::name_>;

    template<class T>
    using type_ = bind_<class type_tag, ::cfg_attributes::type_<T>>;

    struct expr { char const * value; };
    #define CPP_EXPR(name) ::cfg_attributes::cpp::expr{#name}
}


namespace spec
{
    using name = bind_<class name_tag, ::cfg_attributes::name_>;

    template<class T>
    using type_ = bind_<class type_tag, ::cfg_attributes::type_<T>>;

    enum class attr {
        no_ini_no_gui   = 1 << 0,
        ini_and_gui     = 1 << 1,
        hidden_in_gui   = 1 << 2,
        hex_in_gui      = 1 << 3,
        advanced_in_gui = 1 << 4,
        iptables_in_gui = 1 << 5,
        password_in_gui = 1 << 6,

        connection_policy_with_ini    = hidden_in_gui,
        connection_policy_without_ini = no_ini_no_gui
    };

    constexpr attr operator | (attr x, attr y) {
        return static_cast<attr>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
    }

    constexpr attr operator & (attr x, attr y) {
        return static_cast<attr>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
    }
}


namespace sesman
{
    using name = bind_<class name_tag, ::cfg_attributes::name_>;

    template<class T>
    using type_ = bind_<class type_tag, ::cfg_attributes::type_<T>>;

    enum class io {
        none,
        sesman_to_proxy = 1 << 0,
        proxy_to_sesman = 1 << 1,
        rw = sesman_to_proxy | proxy_to_sesman,
    };

    constexpr io operator | (io x, io y) {
        return static_cast<io>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
    }

    constexpr io operator & (io x, io y) {
        return static_cast<io>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
    }
}

}

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
#include <type_traits>

#include <string>
#include <vector>

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

struct name_ { std::string name; };

template<class T>
struct type_ { using type = T; };

template<class T>
struct default_
{
    using type = T;
    T value;
};

class novalue {};

using nodefault = default_<novalue>;


template<class T>
default_<T> set(T const & x)
{ return {x}; }

template<std::size_t N>
default_<std::string> set(char const (&x)[N])
{ return {std::string(x+0, x+N-1)}; }

struct desc { std::string value; };

struct prefix_value { char const * value; };


template<template<class> class>
struct tpl_t {};


namespace types
{
    struct integer_base {};
    struct signed_base : integer_base {};
    struct unsigned_base : integer_base {};

    struct u16 : unsigned_base { u16(long long = 0) {} };
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
    struct name { std::string name; };

    template<class T>
    struct type_
    {
        ::cfg_attributes::type_<T> to_type() const
        {
            return {};
        }
    };

    struct expr { char const * value; };
    #define CPP_EXPR(expression) ::cfg_attributes::cpp::expr{#expression}
}


namespace spec
{
    struct name { std::string name; };

    template<class T>
    struct type_
    {
        ::cfg_attributes::type_<T> to_type() const
        {
            return {};
        }
    };

    namespace internal
    {
        enum class attr {
            no_ini_no_gui   = 1 << 0,
            ini_and_gui     = 1 << 1,
            hidden_in_gui   = 1 << 2,
            hex_in_gui      = 1 << 3,
            advanced_in_gui = 1 << 4,
            iptables_in_gui = 1 << 5,
            password_in_gui = 1 << 6,
        };

        constexpr attr operator | (attr x, attr y) {
            return static_cast<attr>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
        }

        constexpr attr operator & (attr x, attr y) {
            return static_cast<attr>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
        }

        template<attr value>
        using spec_attr_t = std::integral_constant<attr, value>;

        template<internal::attr v1, internal::attr v2>
        spec_attr_t<v1 | v2>
        operator | (spec_attr_t<v1>, spec_attr_t<v2>)
        {
            static_assert(!bool(v1 & attr::no_ini_no_gui), "no_ini_no_gui is incompatible with other values");
            static_assert(!bool(v2 & attr::no_ini_no_gui), "no_ini_no_gui is incompatible with other values");
            constexpr auto in_gui
                = attr::hidden_in_gui
                | attr::hex_in_gui
                | attr::advanced_in_gui
                | attr::iptables_in_gui
                | attr::password_in_gui
            ;
            static_assert(!(bool(v1 & attr::hidden_in_gui) && bool(v2 & in_gui)), "hidden_in_gui is incompatible with *_in_gui values");
            static_assert(!(bool(v2 & attr::hidden_in_gui) && bool(v1 & in_gui)), "hidden_in_gui is incompatible with *_in_gui values");
            return {};
        }
    }

    enum class log_policy : unsigned char
    {
        loggable,
        unloggable,
        unloggable_if_value_contains_password,
    };

    inline namespace constants
    {
        inline internal::spec_attr_t<internal::attr::no_ini_no_gui>   no_ini_no_gui{};
        inline internal::spec_attr_t<internal::attr::ini_and_gui>     ini_and_gui{};
        inline internal::spec_attr_t<internal::attr::hidden_in_gui>   hidden_in_gui{};
        inline internal::spec_attr_t<internal::attr::hex_in_gui>      hex_in_gui{};
        inline internal::spec_attr_t<internal::attr::advanced_in_gui> advanced_in_gui{};
        inline internal::spec_attr_t<internal::attr::iptables_in_gui> iptables_in_gui{};
        inline internal::spec_attr_t<internal::attr::password_in_gui> password_in_gui{};

        inline auto loggable = log_policy::loggable;
        inline auto unloggable = log_policy::unloggable;
        inline auto unloggable_if_value_contains_password = log_policy::unloggable_if_value_contains_password;
    }
}


namespace connpolicy
{
    struct name { std::string name; };

    struct section { char const* name; };

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
    { return {{std::string(x+0, x+N-1)}}; }

    namespace internal
    {
        enum class attr {
            unspecified            = 1 << 0,
            hex_in_connpolicy      = 1 << 1,
            advanced_in_connpolicy = 1 << 2,
        };

        constexpr attr operator | (attr x, attr y)
        {
            return static_cast<attr>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
        }

        constexpr attr operator & (attr x, attr y)
        {
            return static_cast<attr>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
        }
    }

    namespace constants
    {
        constexpr auto hex_in_connpolicy = internal::attr::hex_in_connpolicy;
        constexpr auto advanced_in_connpolicy = internal::attr::advanced_in_connpolicy;
    }
}


namespace sesman
{
    struct name { std::string name; };


    template<class T>
    struct type_
    {
        ::cfg_attributes::type_<T> to_type() const
        {
            return {};
        }
    };

    namespace internal
    {
        enum class io {
            none,
            sesman_to_proxy   = 1 << 0,
            proxy_to_sesman   = 1 << 1,
            rw = sesman_to_proxy | proxy_to_sesman,
        };

        inline io operator | (io x, io y) {
            return static_cast<io>(static_cast<unsigned>(x) | static_cast<unsigned>(y));
        }

        inline io operator & (io x, io y) {
            return static_cast<io>(static_cast<unsigned>(x) & static_cast<unsigned>(y));
        }

        template<io value>
        using sesman_io_t = std::integral_constant<io, value>;
    }

    inline namespace constants
    {
        inline internal::sesman_io_t<internal::io::none>              no_sesman{};
        inline internal::sesman_io_t<internal::io::proxy_to_sesman>   proxy_to_sesman{};
        inline internal::sesman_io_t<internal::io::sesman_to_proxy>   sesman_to_proxy{};
        inline internal::sesman_io_t<internal::io::rw>                sesman_rw{};
    }

    struct connection_policy
    {
        char const* file;
        connpolicy::internal::attr spec;

        explicit connection_policy(char const* file, connpolicy::internal::attr spec = {})
        : file(file)
        , spec(spec)
        {}
    };

    inline connection_policy operator | (connection_policy const& x, connpolicy::internal::attr y)
    {
        return connection_policy{x.file, x.spec | y};
    }

    struct deprecated_names
    {
        std::vector<std::string> names;

        deprecated_names(std::initializer_list<char const*> l)
        : names(l.begin(), l.end())
        {}
    };
}


namespace detail
{
    template<class... Ts>
    struct names
    {
        template<template<class...> class F>
        using f = F<Ts...>;
    };
}

using names = detail::names<
    name_,
    cpp::name,
    spec::name,
    sesman::name,
    connpolicy::name
>;

}

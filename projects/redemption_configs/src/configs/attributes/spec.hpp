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

#include <cassert>

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

#define TYPE_REQUIEREMENT(T)                                                   \
    static_assert(!std::is_integral<T>::value || std::is_same<T, bool>::value, \
        "T cannot be a integral type, "                                        \
        "use types::u8, types::u16, types::s16, etc instead, "                 \
        "eventually types::int_ or types::unsigned_")

template<class T>
struct type_
{
    TYPE_REQUIEREMENT(T);
    using type = T;
};

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

struct names
{
    std::string cpp;
    std::string ini {};
    std::string sesman {};
    std::string connpolicy {};

    std::string const& cpp_name() const { assert(!cpp.empty()); return cpp; }
    std::string const& ini_name() const { return ini.empty() ? cpp : ini; }
    std::string const& sesman_name() const { return sesman.empty() ? cpp : sesman; }
    std::string const& connpolicy_name() const { return connpolicy.empty() ? cpp : connpolicy; }
};


namespace types
{
    struct integer_base {};
    struct signed_base : integer_base {};
    struct unsigned_base : integer_base {};

    struct u8 : unsigned_base { u8(long long = 0) {} };
    struct u16 : unsigned_base { u16(long long = 0) {} };
    struct u32 : unsigned_base { u32(long long = 0) {} };
    struct u64 : unsigned_base { u64(long long = 0) {} };

    struct s8 : signed_base { s8(long long = 0) {} };
    struct s16 : signed_base { s16(long long = 0) {} };
    struct s32 : signed_base { s32(long long = 0) {} };
    struct s64 : signed_base { s64(long long = 0) {} };

    struct unsigned_ : unsigned_base { unsigned_(unsigned = 0) {} };
    struct int_ : signed_base { int_(int = 0) {} };

    struct rgb
    {
        rgb(int = 0) {}
        rgb(char const* = nullptr) {}
    };

    template<unsigned Len> struct fixed_string {};
    template<unsigned Len> struct fixed_binary {};

    template<class T>
    struct list
    {
        TYPE_REQUIEREMENT(T);
    };

    struct ip_string {};

    struct dirpath {};

    template<class T, long min, long max>
    struct range
    {
        TYPE_REQUIEREMENT(T);
    };
}

namespace traits
{
    template<class T>
    constexpr bool is_integer_v = std::is_base_of_v<types::integer_base, T>;

    template<class T>
    constexpr bool is_signed_v = std::is_base_of_v<types::signed_base, T>;

    template<class T>
    constexpr bool is_unsigned_v = std::is_base_of_v<types::unsigned_base, T>;
}

namespace cpp
{
    struct expr { char const * value; };
    #define CPP_EXPR(expression) ::cfg_attributes::cpp::expr{#expression}
}


namespace spec
{
    template<class T>
    struct type_
    {
        TYPE_REQUIEREMENT(T);

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
            image_in_gui    = 1 << 7,
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
                | attr::image_in_gui
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
        inline constexpr internal::spec_attr_t<internal::attr::no_ini_no_gui>   no_ini_no_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::ini_and_gui>     ini_and_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::hidden_in_gui>   hidden_in_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::hex_in_gui>      hex_in_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::advanced_in_gui> advanced_in_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::iptables_in_gui> iptables_in_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::password_in_gui> password_in_gui{};
        inline constexpr internal::spec_attr_t<internal::attr::image_in_gui>    image_in_gui{};

        inline constexpr auto loggable = log_policy::loggable;
        inline constexpr auto unloggable = log_policy::unloggable;
        inline constexpr auto unloggable_if_value_contains_password = log_policy::unloggable_if_value_contains_password;
    }
}


namespace connpolicy
{
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
        inline constexpr internal::sesman_io_t<internal::io::none>              no_sesman{};
        inline constexpr internal::sesman_io_t<internal::io::proxy_to_sesman>   proxy_to_sesman{};
        inline constexpr internal::sesman_io_t<internal::io::sesman_to_proxy>   sesman_to_proxy{};
        inline constexpr internal::sesman_io_t<internal::io::rw>                sesman_rw{};
    }

    struct connection_policy
    {
        std::vector<std::string> files;
        connpolicy::internal::attr spec = {};

        explicit connection_policy(std::string file)
        : files(1u, std::move(file))
        {}
    };

    inline connection_policy operator | (connection_policy const& x, connpolicy::internal::attr y)
    {
        connection_policy conn{x};
        conn.spec = conn.spec | y;
        return conn;
    }

    inline connection_policy operator | (connection_policy const& x, connection_policy const& y)
    {
        assert(x.spec == y.spec);
        connection_policy conn{x};
        conn.files.insert(conn.files.end(), y.files.begin(), y.files.end());
        return conn;
    }

    namespace internal
    {
        enum class is_target_context : bool;

        template<is_target_context value>
        using is_target_context_t = std::integral_constant<is_target_context, value>;
    }

    inline namespace constants
    {
        inline constexpr internal::is_target_context_t<internal::is_target_context(true)> is_target_ctx{};
        inline constexpr internal::is_target_context_t<internal::is_target_context(false)> not_target_ctx{};
    }
}

}

#undef TYPE_REQUIEREMENT
